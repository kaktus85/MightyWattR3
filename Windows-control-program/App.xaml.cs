using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;
using System.Windows.Threading;
using System.Diagnostics;

namespace MightyWatt
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
#if DEBUG
        private static DebugWindow debugWindow = new DebugWindow();
        public static ConsoleTraceListener DebugOutput;

        public App()
        {
            debugWindow.Show();
            DebugOutput = new ConsoleTraceListener
            {
                Writer = debugWindow.Writer
            };
            this.DispatcherUnhandledException += App_DispatcherUnhandledException;
        }

        public static void CloseDebugWindow()
        {
            try
            {
                debugWindow.Close();
            }
            catch (Exception) { }
        }

        void App_DispatcherUnhandledException(object sender, DispatcherUnhandledExceptionEventArgs e)
        {
            Clipboard.SetText(e.Exception.Message);
            DebugOutput.WriteLine(string.Empty);
            DebugOutput.WriteLine("Unhandled exception occured, please, restart the application. Exception message has been copied to the clipboard.");
            DebugOutput.WriteLine(e.Exception.Message);
            e.Handled = true;
        }
#endif
    }
}