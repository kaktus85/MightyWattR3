using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MightyWatt
{
    public enum PlotDataValues { Voltage, Current, Power, Resistance };

    class PlotData
    {
        int interval; // interval in milliseconds before accepting next data addition
        int capacity; // buffer data capacity
        DateTime lastWrite = DateTime.MinValue; // timestamp of the last write
        private TimestampedBuffer<double[]> values; // circular data buffers

        public PlotData(int interval, int capacity)
        {
            this.interval = interval;
            this.capacity = capacity;
            values = new TimestampedBuffer<double[]>(capacity);
        }

        public void Add(double voltage, double current, double power, double resistance)
        {
            DateTime now = DateTime.Now;

            if ((now - lastWrite).TotalMilliseconds >= interval)
            {
                lastWrite = now;
                values.Add(new double[] { voltage, current, power, resistance });
            }            
        }

        public void Clear()
        {
            values = new TimestampedBuffer<double[]>(capacity);
            lastWrite = DateTime.MinValue;
        }

        public TimestampedBuffer<double[]> Values { get { return values; } }
    }
}
