using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using overlapped_namepipe.duplex;
using DomenicDenicola.Extensions;

namespace WindowsFormsApplication1
{
	public partial class Form1 : Form
	{
		private overlappedNamedPipeDuplexAppMutex _ov_named_pipe_duplex_app_mutex;

		public Form1() {
			InitializeComponent();
		}

		private void Form1_Load( object sender, EventArgs e ) {
			string appname = System.Reflection.Assembly.GetExecutingAssembly().GetName().Name;
			_ov_named_pipe_duplex_app_mutex = new overlappedNamedPipeDuplexAppMutex(appname, on_app_mutex_server_handler, null);
			_ov_named_pipe_duplex_app_mutex.startServer();
			Debug.WriteLine("form load done...");
		}

		private void on_app_mutex_server_handler(object sender, EventArgs e) {
			var args = e as overlappedNamedPipeDuplexAppMutex.serverResponseCbEventArgs;
			Debug.WriteLine("on_app_mutex_server_handler: " + args );
		}

		private void on_app_mutex_client_handler(object sender, EventArgs e) {
			var args = e as overlappedNamedPipeDuplexAppMutex.clientResponseCbEventArgs;
			this.Invoke(delegate {
				Debug.WriteLine("on_app_mutex_client_handler: " + args);
			});
		}

	}
}
