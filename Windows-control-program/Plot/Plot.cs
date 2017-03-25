using System;
using System.Threading.Tasks;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;

namespace MightyWatt
{
    public enum PlotTimeUnit { ms = 1, s = 1000, min = 60000, h = 3600000, d = 86400000};    
    public enum PlotTimeSpan { m1, m15, h4, d1, d7}

    class Plot : IDisposable
    {            
        // GNUplot
        private static StreamWriter gnuPlotStream;

        public Plot(string gnuPlotPath)
        {
            Process gnuPlot = new Process();
            gnuPlot.StartInfo.FileName = gnuPlotPath;
            gnuPlot.StartInfo.UseShellExecute = false;
            gnuPlot.StartInfo.RedirectStandardInput = true;
            gnuPlot.StartInfo.CreateNoWindow = true;
            gnuPlot.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            gnuPlot.Start();

            gnuPlotStream = gnuPlot.StandardInput;
            gnuPlotStream.WriteLine("set terminal wxt noraise");
            gnuPlotStream.Flush();
        }

        public void Dispose()
        {
            gnuPlotStream.Close();
        }        

        public void PlotData(PlotTimeUnit timeUnit, PlotData data, TimeSpan span)
        {
            try
            {
                DateTime now = DateTime.Now;
                Tuple<DateTime, double[]>[] trimmedValues = data.Values.GetValues(now.Subtract(span), now);
                if (trimmedValues == null)
                {
                    // nothing to plot
                    return;
                }
                string[] streamData = new string[trimmedValues.Length];
                StringBuilder sb = new StringBuilder();

                if (trimmedValues != null)
                {
                    gnuPlotStream.WriteLine("set multiplot layout 2,2 columnsfirst");

                    // voltage
                    gnuPlotStream.WriteLine("set xlabel 'Time [{0}]'", timeUnit.ToString());
                    gnuPlotStream.WriteLine("set ylabel \"Voltage [V]\"");
                    gnuPlotStream.WriteLine("unset key");
                    gnuPlotStream.WriteLine("plot \"-\" using 1:2 with lines lt rgb \"#75C15F\"");

                    for (int i = 0; i < trimmedValues.Length; i++)
                    {
                        sb.Clear();
                        sb.Append((Convert.ToDouble((trimmedValues[i].Item1 - now).TotalMilliseconds) / (int)timeUnit).ToString("F3", System.Globalization.CultureInfo.InvariantCulture));
                        sb.Append(" ");
                        sb.Append(trimmedValues[i].Item2[(int)PlotDataValues.Voltage].ToString("F6", System.Globalization.CultureInfo.InvariantCulture));
                        streamData[i] = sb.ToString();
                    }
                    for (int i = 0; i < trimmedValues.Length; i++)
                    {
                        gnuPlotStream.WriteLine(streamData[i]);
                    }

                    gnuPlotStream.WriteLine("e");
                    gnuPlotStream.Flush();

                    // current
                    gnuPlotStream.WriteLine("set xlabel 'Time [{0}]'", timeUnit.ToString());
                    gnuPlotStream.WriteLine("set ylabel \"Current [A]\"");
                    gnuPlotStream.WriteLine("unset key");
                    gnuPlotStream.WriteLine("plot \"-\" using 1:2 with lines lt rgb \"#DE6D26\"");

                    for (int i = 0; i < trimmedValues.Length; i++)
                    {
                        sb.Clear();
                        sb.Append((Convert.ToDouble((trimmedValues[i].Item1 - now).TotalMilliseconds) / (int)timeUnit).ToString("F3", System.Globalization.CultureInfo.InvariantCulture));
                        sb.Append(" ");
                        sb.Append(trimmedValues[i].Item2[(int)PlotDataValues.Current].ToString("F6", System.Globalization.CultureInfo.InvariantCulture));
                        streamData[i] = sb.ToString();
                    }
                    for (int i = 0; i < trimmedValues.Length; i++)
                    {
                        gnuPlotStream.WriteLine(streamData[i]);
                    }
                    gnuPlotStream.WriteLine("e");
                    gnuPlotStream.Flush();

                    // power
                    gnuPlotStream.WriteLine("set xlabel 'Time [{0}]'", timeUnit.ToString());
                    gnuPlotStream.WriteLine("set ylabel \"Power [W]\"");
                    gnuPlotStream.WriteLine("unset key");
                    gnuPlotStream.WriteLine("plot \"-\" using 1:2 with lines lt rgb \"#3D4DA1\"");

                    for (int i = 0; i < trimmedValues.Length; i++)
                    {
                        sb.Clear();
                        sb.Append((Convert.ToDouble((trimmedValues[i].Item1 - now).TotalMilliseconds) / (int)timeUnit).ToString("F3", System.Globalization.CultureInfo.InvariantCulture));
                        sb.Append(" ");
                        sb.Append(trimmedValues[i].Item2[(int)PlotDataValues.Power].ToString("F6", System.Globalization.CultureInfo.InvariantCulture));
                        streamData[i] = sb.ToString();
                    }
                    for (int i = 0; i < trimmedValues.Length; i++)
                    {
                        gnuPlotStream.WriteLine(streamData[i]);
                    }
                    gnuPlotStream.WriteLine("e");
                    gnuPlotStream.Flush();

                    // resistance
                    gnuPlotStream.WriteLine("set xlabel 'Time [{0}]'", timeUnit.ToString());
                    gnuPlotStream.WriteLine("set ylabel \"Resistance [Ohm]\"");
                    gnuPlotStream.WriteLine("unset key");
                    gnuPlotStream.WriteLine("plot \"-\" using 1:2 with lines lt rgb \"#59525C\"");

                    for (int i = 0; i < trimmedValues.Length; i++)
                    {
                        sb.Clear();
                        sb.Append((Convert.ToDouble((trimmedValues[i].Item1 - now).TotalMilliseconds) / (int)timeUnit).ToString("F3", System.Globalization.CultureInfo.InvariantCulture));
                        sb.Append(" ");
                        sb.Append(trimmedValues[i].Item2[(int)PlotDataValues.Resistance].ToString("F3", System.Globalization.CultureInfo.InvariantCulture));
                        streamData[i] = sb.ToString();
                    }
                    for (int i = 0; i < trimmedValues.Length; i++)
                    {
                        gnuPlotStream.WriteLine(streamData[i]);
                    }
                    gnuPlotStream.WriteLine("e");
                    gnuPlotStream.Flush();
                }
            }
            catch (Exception)
            {

            }
        }
    }
}
