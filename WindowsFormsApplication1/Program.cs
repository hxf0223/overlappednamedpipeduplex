using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using overlapped_namepipe.duplex;

namespace WindowsFormsApplication1
{
	static class Program
	{
		private static ManualResetEvent _mre;

		[STAThread]
		static void Main() {
			_mre = new ManualResetEvent(false);
			string appname = System.Reflection.Assembly.GetExecutingAssembly().GetName().Name;

			var ov_named_pipe_duplex_app_mutex =
				new OverlappedNamedPipeDuplexAppMutex(appname, null, on_app_mutex_client_handler);
			ov_named_pipe_duplex_app_mutex.startClient();
			bool bresult = _mre.WaitOne(200);

			if (bresult) {
				System.Threading.Thread.Sleep(60);
				ov_named_pipe_duplex_app_mutex.stopClient();
				ov_named_pipe_duplex_app_mutex.Dispose();
				Environment.Exit( 0 );
			}
			else {
				ov_named_pipe_duplex_app_mutex.stopClient();
				ov_named_pipe_duplex_app_mutex.Dispose();

				Application.EnableVisualStyles();
				Application.SetCompatibleTextRenderingDefault(false);
				Application.Run(new Form1());
			}
			
		}

		private static void on_app_mutex_client_handler(object sender, EventArgs e) {
			var args = e as OverlappedNamedPipeDuplexAppMutex.clientResponseCbEventArgs;
			_mre.Set();
		}

	}
}
