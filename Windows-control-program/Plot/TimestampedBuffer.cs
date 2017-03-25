using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MightyWatt
{
    class TimestampedBuffer<T>
    {
        private int capacity;
        private Tuple<DateTime, T>[] values;
        private int iterator;

        public TimestampedBuffer(int capacity)
        {
            if (capacity > 0)
            {
                this.capacity = capacity;
                iterator = 0;
                values = new Tuple<DateTime, T>[capacity];
            }
            else
            {
                throw new ArgumentException("Capacity must be greater than zero");
            }
        }

        public void Add(T value)
        {
            values[iterator] = new Tuple<DateTime, T>(DateTime.Now, value);
            if (++iterator >= capacity)
            {
                iterator = 0;
            }
        }

        public Tuple<DateTime, T>[] GetValues(DateTime from, DateTime to)
        {
            int startIndex = -1;
            int endIndex = -1;
            DateTime oldestValid = DateTime.Now;
            DateTime newestValid = DateTime.MinValue;
            Tuple<DateTime, T>[] result;

            for (int i = 0; i < capacity; i++)
            {
                if (values[i] == null)
                {
                    break;
                }

                if (values[i].Item1.CompareTo(oldestValid) < 0 && values[i].Item1.CompareTo(from) >= 0)
                {
                    oldestValid = values[i].Item1;
                    startIndex = i;
                }

                if (values[i].Item1.CompareTo(newestValid) >= 0 && values[i].Item1.CompareTo(to) <= 0)
                {
                    newestValid = values[i].Item1;
                    endIndex = i;
                }
            }

            if (startIndex < 0 || endIndex < 0)
            {
                result = null;
            }
            else
            {
                if (startIndex > endIndex)
                {
                    result = new Tuple<DateTime, T>[capacity - startIndex + endIndex + 1];
                    for (int i = startIndex; i < capacity; i++)
                    {
                        if (values[i] == null)
                        {
                            break;
                        }

                        result[i - startIndex] = values[i];
                    }
                    for (int i = 0; i <= endIndex; i++)
                    {
                        if (values[i] == null)
                        {
                            break;
                        }

                        result[i + capacity - startIndex] = values[i];
                    }
                }
                else
                {
                    result = new Tuple<DateTime, T>[endIndex - startIndex + 1];
                    for (int i = startIndex; i <= endIndex; i++)
                    {
                        if (values[i] == null)
                        {
                            break;
                        }

                        result[i - startIndex] = values[i];
                    }
                }
            }

            return result;
        }

        public Tuple<DateTime, T> this[int i]
        {
            get
            {
                if (i >= 0 && i < capacity)
                {                    
                    return values[i];
                }
                else
                {
                    throw new ArgumentOutOfRangeException(i > 0 ? "Index is outside of array bounds" : "Index cannot be negative");
                }
            }
            set
            {
                if (i >= 0 && i < capacity)
                {
                    values[i] = value;
                }
                else
                {
                    throw new ArgumentOutOfRangeException(i > 0 ? "Index is outside of array bounds" : "Index cannot be negative");
                }
            }
        }
    }
}
