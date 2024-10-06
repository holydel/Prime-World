using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace PW_MiniLauncher
{
  static class Program
  {
    /// <summary>
    /// The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main( string[] args )
    {
      Application.EnableVisualStyles();
      Application.SetCompatibleTextRenderingDefault(false);
      string protocolArgs = "";
      if ( args.Length > 0 )
        protocolArgs = args[0];

      Application.Run(new LoginForm(protocolArgs));
    }
  }
}
