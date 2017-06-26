// Decompiled with JetBrains decompiler
// Type: overlapped_namepipe.duplex.overlappedNamedPipeDuplexAppMutex
// Assembly: OverlappedNamedPipeDuplexNet, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: CF7E2C59-CDA1-40A9-AF1E-1FD8DFCEE479
// Assembly location: E:\workspace\OverlappedNamedPipeDuplex\bin\OverlappedNamedPipeDuplexNet.dll

using System;
using System.Runtime.InteropServices;
using System.Text;

namespace overlapped_namepipe.duplex
{
  public class overlappedNamedPipeDuplexAppMutex : IDisposable
  {
    private readonly overlappedNamedPipeDuplexAppMutex.ovNamedPipeDuplexServerCb _resp_cb_server;
    private readonly overlappedNamedPipeDuplexAppMutex.oVNamedPipeDuplexClientCb _resp_cb_client;
    private bool _disposed;

    public event EventHandler onServerResponseEvent;

    public event EventHandler onClientResponseEvent;

    public overlappedNamedPipeDuplexAppMutex(string appName, EventHandler serverResponseHandler, EventHandler clientResponseHandler)
    {
      this._resp_cb_server = new overlappedNamedPipeDuplexAppMutex.ovNamedPipeDuplexServerCb(this.native_server_resp_cb_func);
      this._resp_cb_client = new overlappedNamedPipeDuplexAppMutex.oVNamedPipeDuplexClientCb(this.native_client_resp_cb_func);
      this.onServerResponseEvent += serverResponseHandler;
      this.onClientResponseEvent += clientResponseHandler;
      overlappedNamedPipeDuplexAppMutex.OverlappedNamedPipeDuplexInit(appName, this._resp_cb_server, this._resp_cb_client);
    }

    ~overlappedNamedPipeDuplexAppMutex()
    {
      this.Dispose(false);
    }

    [DllImport("OverlappedNamedPipeDuplex.dll", CallingConvention = CallingConvention.StdCall)]
    private static extern void OverlappedNamedPipeDuplexGetPipeName(StringBuilder sbBuffer, uint size);

    [DllImport("OverlappedNamedPipeDuplex.dll", CallingConvention = CallingConvention.StdCall)]
    private static extern int OverlappedNamedPipeDuplexInit([MarshalAs(UnmanagedType.LPStr)] string strAppName, [MarshalAs(UnmanagedType.FunctionPtr)] overlappedNamedPipeDuplexAppMutex.ovNamedPipeDuplexServerCb cbServer, [MarshalAs(UnmanagedType.FunctionPtr)] overlappedNamedPipeDuplexAppMutex.oVNamedPipeDuplexClientCb cbClient);

    [DllImport("OverlappedNamedPipeDuplex.dll", CallingConvention = CallingConvention.StdCall)]
    private static extern int OverlappedNamedPipeDuplexDeInit();

    [DllImport("OverlappedNamedPipeDuplex.dll", CallingConvention = CallingConvention.StdCall)]
    private static extern int OverlappedNamedPipeDuplexStartServer();

    [DllImport("OverlappedNamedPipeDuplex.dll", CallingConvention = CallingConvention.StdCall)]
    private static extern int OverlappedNamedPipeDuplexStopServer();

    [DllImport("OverlappedNamedPipeDuplex.dll", CallingConvention = CallingConvention.StdCall)]
    private static extern int OverlappedNamedPipeDuplexStartClient();

    [DllImport("OverlappedNamedPipeDuplex.dll", CallingConvention = CallingConvention.StdCall)]
    private static extern int OverlappedNamedPipeDuplexStopClient();

    protected void server_response_notify(overlappedNamedPipeDuplexAppMutex.serverResponseCbEventArgs args)
    {
      if (this.onServerResponseEvent == null)
        return;
      foreach (EventHandler invocation in this.onServerResponseEvent.GetInvocationList())
        invocation.BeginInvoke((object) this, (EventArgs) args, (AsyncCallback) null, (object) null);
    }

    protected void client_response_notify(overlappedNamedPipeDuplexAppMutex.clientResponseCbEventArgs args)
    {
      if (this.onClientResponseEvent == null)
        return;
      foreach (EventHandler invocation in this.onClientResponseEvent.GetInvocationList())
        invocation.BeginInvoke((object) this, (EventArgs) args, (AsyncCallback) null, (object) null);
    }

    protected void remove_all_server_response_event_handler()
    {
      if (this.onServerResponseEvent == null)
        return;
      Delegate[] invocationList = this.onServerResponseEvent.GetInvocationList();
      if (invocationList.Length <= 0)
        return;
      foreach (Delegate @delegate in invocationList)
        this.onServerResponseEvent -= @delegate as EventHandler;
    }

    protected void remove_all_client_response_event_handler()
    {
      if (this.onClientResponseEvent == null)
        return;
      Delegate[] invocationList = this.onClientResponseEvent.GetInvocationList();
      if (invocationList.Length <= 0)
        return;
      foreach (Delegate @delegate in invocationList)
        this.onClientResponseEvent -= @delegate as EventHandler;
    }

    public void Dispose()
    {
      this.Dispose(true);
      GC.SuppressFinalize((object) this);
    }

    protected virtual void Dispose(bool disposing)
    {
      if (this._disposed)
        return;
      int num = disposing ? 1 : 0;
      overlappedNamedPipeDuplexAppMutex.OverlappedNamedPipeDuplexStopServer();
      overlappedNamedPipeDuplexAppMutex.OverlappedNamedPipeDuplexStopClient();
      overlappedNamedPipeDuplexAppMutex.OverlappedNamedPipeDuplexDeInit();
      this.remove_all_server_response_event_handler();
      this.remove_all_client_response_event_handler();
      this._disposed = true;
    }

    private void native_server_resp_cb_func(IntPtr pReceive)
    {
      this.server_response_notify(new overlappedNamedPipeDuplexAppMutex.serverResponseCbEventArgs((overlappedNamedPipeDuplexAppMutex.pipeCommDataType) Marshal.PtrToStructure(pReceive, typeof (overlappedNamedPipeDuplexAppMutex.pipeCommDataType))));
    }

    private void native_client_resp_cb_func(IntPtr pReceive)
    {
      this.client_response_notify(new overlappedNamedPipeDuplexAppMutex.clientResponseCbEventArgs((overlappedNamedPipeDuplexAppMutex.pipeCommDataType) Marshal.PtrToStructure(pReceive, typeof (overlappedNamedPipeDuplexAppMutex.pipeCommDataType))));
    }

    public int startServer()
    {
      return overlappedNamedPipeDuplexAppMutex.OverlappedNamedPipeDuplexStartServer();
    }

    public int stopServer()
    {
      return overlappedNamedPipeDuplexAppMutex.OverlappedNamedPipeDuplexStopServer();
    }

    public int startClient()
    {
      return overlappedNamedPipeDuplexAppMutex.OverlappedNamedPipeDuplexStartClient();
    }

    public int stopClient()
    {
      return overlappedNamedPipeDuplexAppMutex.OverlappedNamedPipeDuplexStopClient();
    }

    public enum pipeCommand : byte
    {
      request = 1,
      response = 16,
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct pipeCommDataType
    {
      public overlappedNamedPipeDuplexAppMutex.pipeCommand _cmd;
      [MarshalAs(UnmanagedType.ByValArray, SizeConst = 256)]
      public byte[] _name;
    }

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void ovNamedPipeDuplexServerCb(IntPtr pReceive);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate void oVNamedPipeDuplexClientCb(IntPtr sbReceive);

    public class serverResponseCbEventArgs : EventArgs
    {
      public overlappedNamedPipeDuplexAppMutex.pipeCommDataType data { get; private set; }

      public serverResponseCbEventArgs(overlappedNamedPipeDuplexAppMutex.pipeCommDataType data)
      {
        this.data = data;
      }

      public override string ToString()
      {
        return string.Format("cmd {0}, name {1}", (object) this.data._cmd, (object) Encoding.ASCII.GetString(this.data._name));
      }
    }

    public class clientResponseCbEventArgs : EventArgs
    {
      public overlappedNamedPipeDuplexAppMutex.pipeCommDataType data { get; private set; }

      public clientResponseCbEventArgs(overlappedNamedPipeDuplexAppMutex.pipeCommDataType data)
      {
        this.data = data;
      }

      public override string ToString()
      {
        return string.Format("cmd {0}, name {1}", (object) this.data._cmd, (object) Encoding.ASCII.GetString(this.data._name));
      }
    }
  }
}
