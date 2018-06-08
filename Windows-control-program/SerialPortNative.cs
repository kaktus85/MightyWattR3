using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.IO;
using System.Threading.Tasks;

namespace MightyWatt
{
    class SerialPortNative
    {
        private System.IO.Ports.SerialPort port;
        private string newLine = "\r\n";
        private byte portNumber;
        private uint baudRate = 0;
        private Parity parity = Parity.None;
        private bool dtrControl = false;
        private bool rtsControl = false;
        private byte dataBits = 8;
        private StopBits stopBits = StopBits.One;
        private uint readTimeout = 500;
        private uint writeTimeout = 500;
        private int writeBufferSize = 16384;
        private int readBufferSize = 16384;

        public SerialPortNative()
        {

        }

        public void Open()
        {
            Close();
            port = new System.IO.Ports.SerialPort();
            if (PortName == null)
            {
                throw new IOException("Serial port number not set");
            }
            if (baudRate == 0)
            {
                throw new IOException("Baud rate not set");
            }

            port.PortName = PortName;
            port.BaudRate = (int)baudRate;
            port.DataBits = dataBits;
            port.DtrEnable = dtrControl;
            port.NewLine = newLine;
            port.Parity = parity;
            port.ReadTimeout = (int)readTimeout;
            port.RtsEnable = rtsControl;
            port.StopBits = stopBits;
            port.WriteTimeout = (int)writeTimeout;
            port.WriteBufferSize = writeBufferSize;
            port.ReadBufferSize = readBufferSize;

            port.Open();
            while (port.IsOpen == false) { }
            IsOpen = true;
        }

        public void Close()
        {
            if (port != null)
            {
                if (port.IsOpen)
                {
                    port.Close();
                }
                port.Dispose();
                port = null;
            }
            IsOpen = false;            
        }

        public byte[] ReadBytes(int count)
        {
            DateTime start = DateTime.Now;

            if (port.BytesToRead < count)
            {
                return null;
            }
            else
            {
                byte[] b = new byte[count];
                port.Read(b, 0, count);
                return b;
            }          
        }

        public int ReadByte()
        {
            if (port != null)
            {                
                return port.ReadByte();
            }
            else
            {
                return -1;
            }            
        }

        public string ReadLine()
        {
            return port?.ReadLine();          
        }

        public void Flush()
        {
            port.ReadExisting();
            port.DiscardInBuffer();
            //port.DiscardOutBuffer();
        }

        public void Write(byte[] buffer)
        {
            if (port != null)
            {
                if (port.IsOpen)
                {                    
                    port.BaseStream.Write(buffer, 0, buffer.Length);
                    //DateTime now = DateTime.Now;
                    //if (buffer.Length > 3)
                    //{
                    //    Console.WriteLine("Written {0} bytes at {1}.{2}:{3}:{4}", buffer.Length, now.Hour.ToString("00"), now.Minute.ToString("00"), now.Second.ToString("00"), now.Millisecond.ToString("000"));
                    //}
                }
            }            
        }

        public int BytesToWrite
        {
            get
            {
                if (port != null)
                {
                    return port.BytesToWrite;
                }
                else
                {
                    return -1;
                }
            }
        }

        public byte PortNumber
        {
            get
            {
                return portNumber;
            }
            set
            {
                if (value == 0)
                {
                    throw new ArgumentOutOfRangeException("Port number cannot be zero");
                }
                portNumber = value;
            }
        }

        public string PortName
        {
            get
            {
                if (portNumber > 0)
                {
                    return "COM" + portNumber.ToString();
                }
                else
                {
                    return null;
                }
            }
        }

        public uint BaudRate
        {
            get
            {
                return baudRate;
            }
            set
            {
                if (IsOpen)
                {
                    throw new IOException("Baud rate cannot be changed while the port is open");
                }

                if (value == 0)
                {
                    throw new ArgumentOutOfRangeException("Baud rate cannot be zero");
                }

                baudRate = value;
            }
        }

        public Parity Parity
        {
            get { return parity; }
            set
            {
                if (IsOpen)
                {
                    throw new IOException("Parity cannot be changed while the port is open");
                }

                parity = value;
            }
        }

        public bool DtrControl
        {
            get { return dtrControl; }
            set
            {
                if (IsOpen)
                {
                    throw new IOException("DTR control cannot be changed while the port is open");
                }

                dtrControl = value;
            }
        }

        public bool RtsControl
        {
            get { return rtsControl; }
            set
            {
                if (IsOpen)
                {
                    throw new IOException("RTS control cannot be changed while the port is open");
                }

                rtsControl = value;
            }
        }

        public byte DataBits
        {
            get { return dataBits; }
            set
            {
                if (IsOpen)
                {
                    throw new IOException("Data bits cannot be changed while the port is open");
                }

                if ((value < 5) || (value > 8))
                {
                    throw new ArgumentOutOfRangeException("Data bits can only be 5, 6, 7 or 8");
                }

                dataBits = value;
            }
        }

        public StopBits StopBits
        {
            get { return stopBits; }
            set
            {
                if (IsOpen)
                {
                    throw new IOException("Stop bits cannot be changed while the port is open");
                }

                stopBits = value;
            }
        }

        public uint ReadTimeout
        {
            get { return readTimeout; }
            set
            {
                if (value == 0)
                {
                    throw new ArgumentOutOfRangeException("Read timeout cannot be zero");
                }

                readTimeout = value;
                if (port != null)
                {
                    port.ReadTimeout = (int)value;
                }
            }
        }

        public uint WriteTimeout
        {
            get { return writeTimeout; }
            set
            {
                if (value == 0)
                {
                    throw new ArgumentOutOfRangeException("Write timeout cannot be zero");
                }

                writeTimeout = value;
                if (port != null)
                {
                    port.WriteTimeout = (int)value;
                }
            }
        }

        public string NewLine
        {
            get { return NewLine; }
            set
            {
                if (string.IsNullOrEmpty(value))
                {
                    throw new ArgumentException("New line character(s) cannot be an empty string");
                }

                newLine = value;
            }
        }

        public bool IsOpen { get; private set; } = false;
    }
}
