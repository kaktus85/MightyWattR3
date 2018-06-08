using System;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Runtime.ConstrainedExecution;
using System.Security;
using System.IO;
using System.IO.Ports;
using Microsoft.Win32.SafeHandles;
using System.Management;
using System.Collections.Generic;

namespace MightyWatt
{
    public enum DTR_CONTROL : byte { DISABLE, ENABLE, HANDSHAKE };
    public enum RTS_CONTROL : byte { DISABLE, ENABLE, HANDSHAKE, TOGGLE };

    class SerialPort
    {
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        static extern IntPtr CreateFile(
        string lpFileName,
        [MarshalAs(UnmanagedType.U4)] FileAccess dwDesiredAccess,
        [MarshalAs(UnmanagedType.U4)] FileShare dwShareMode,
        IntPtr lpSecurityAttributes,
        [MarshalAs(UnmanagedType.U4)] FileMode dwCreationDisposition,
        [MarshalAs(UnmanagedType.U4)] FileAttributes dwFlagsAndAttributes,
        IntPtr hTemplateFile);

        [DllImport("kernel32.dll")]
        static extern bool SetCommState(IntPtr hFile, [In] ref DCB lpDCB);

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool SetCommTimeouts(IntPtr hFile, [In] ref CommTimeouts lpCommTimeouts);

        [DllImport("kernel32.dll", SetLastError = true)]
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
        [SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool CloseHandle(IntPtr hObject);

        [StructLayout(LayoutKind.Sequential)]
        struct DCB
        {
            internal Int32 DCBlength;
            internal UInt32 BaudRate;
            internal UInt32 f;
            internal Int16 wReserved;
            internal Int16 XonLim;
            internal Int16 XoffLim;
            internal byte ByteSize;
            internal byte Parity;
            internal byte StopBits;
            internal char XonChar;
            internal char XoffChar;
            internal char ErrorChar;
            internal char EofChar;
            internal char EvtChar;
            internal Int16 wReserved1;

            internal bool fBinary
            {
                get { return ((f & 1U) > 0); }
                set { f = value ? f | 1U : f & ~1U; }
            }

            internal bool fParity
            {
                get { return ((f & 2U) > 0); }
                set { f = value ? f | 2U : f & ~2U; }
            }

            internal bool fOutxCtsFlow
            {
                get { return ((f & 4U) > 0); }
                set { f = value ? f | 4U : f & ~4U; }
            }

            internal bool fOutxDsrFlow
            {
                get { return ((f & 8U) > 0); }
                set { f = value ? f | 8U : f & ~8U; }
            }

            internal DTR_CONTROL fDtrControl
            {
                get { return (DTR_CONTROL)((f & 48U) >> 4); }
                set
                {
                    f &= ~48U;
                    f |= ((UInt32)value) << 4;
                }
            }

            internal bool fDsrSensitivity
            {
                get { return ((f & 64U) > 0); }
                set { f = value ? f | 64U : f & ~64U; }
            }

            internal bool fTXContinueOnXoff
            {
                get { return ((f & 128U) > 0); }
                set { f = value ? f | 128U : f & ~128U; }
            }

            internal bool fOutX
            {
                get { return ((f & 256U) > 0); }
                set { f = value ? f | 256U : f & ~256U; }
            }

            internal bool fInX
            {
                get { return ((f & 512U) > 0); }
                set { f = value ? f | 512U : f & ~512U; }
            }

            internal bool fErrorChar
            {
                get { return ((f & 1024U) > 0); }
                set { f = value ? f | 1024U : f & ~1024U; }
            }

            internal bool fNull
            {
                get { return ((f & 2048U) > 0); }
                set { f = value ? f | 2048U : f & ~2048U; }
            }

            internal RTS_CONTROL fRtsControl
            {
                get { return (RTS_CONTROL)((f & 12288U) >> 12); }
                set
                {
                    f &= ~12288U;
                    f |= ((UInt32)value) << 12;
                }
            }

            internal bool fAbortOnError
            {
                get { return ((f & 16384U) > 0); }
                set { f = value ? f | 16384U : f & ~16384U; }
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        struct CommTimeouts
        {
            internal UInt32 ReadIntervalTimeout;
            internal UInt32 ReadTotalTimeoutMultiplier;
            internal UInt32 ReadTotalTimeoutConstant;
            internal UInt32 WriteTotalTimeoutMultiplier;
            internal UInt32 WriteTotalTimeoutConstant;
        }

        private IntPtr comPortHandle = IntPtr.Zero;
        private FileStream stream;
        private bool isOpen = false;
        private char[] newLine = new char[] { '\r', '\n' };

        private byte portNumber;

        private uint baudRate = 0;
        private Parity parity = Parity.None;
        private DTR_CONTROL dtrControl;
        private RTS_CONTROL rtsControl;
        private byte dataBits = 8;
        private StopBits stopBits = StopBits.One;
        private uint readTimeout = 500;
        private uint writeTimeout = 500;

        public SerialPort()
        {
        }

        public void Open()
        {
            Close();

            comPortHandle = CreateFile("\\\\.\\COM" + portNumber.ToString(), FileAccess.ReadWrite, 0, IntPtr.Zero, FileMode.Open, 0, IntPtr.Zero);
            if (comPortHandle == IntPtr.Zero)
            {
                throw new IOException("Cannot open port COM" + portNumber.ToString());
            }

            DCB dcb = new DCB();
            dcb.DCBlength = Marshal.SizeOf(typeof(DCB));
            dcb.BaudRate = baudRate;
            dcb.fBinary = true;
            dcb.fParity = false;
            dcb.fOutxCtsFlow = false;
            dcb.fOutxDsrFlow = false;
            dcb.fDtrControl = dtrControl;
            dcb.fDsrSensitivity = false;
            dcb.fTXContinueOnXoff = false;
            dcb.fOutX = false;
            dcb.fInX = false;
            dcb.fErrorChar = false;
            dcb.fNull = false;
            dcb.fRtsControl = rtsControl;
            dcb.fAbortOnError = true;
            dcb.wReserved = 0;
            dcb.XonLim = 0;
            dcb.XoffLim = 0;
            dcb.ByteSize = dataBits;
            dcb.Parity = (byte)parity;
            dcb.StopBits = (byte)stopBits;
            dcb.XonChar = Convert.ToChar(0);
            dcb.XoffChar = Convert.ToChar(0);
            dcb.ErrorChar = Convert.ToChar(0);
            dcb.EofChar = Convert.ToChar(0);
            dcb.EvtChar = Convert.ToChar(0);
            dcb.wReserved1 = 0;

            if (!SetCommState(comPortHandle, ref dcb))
            {
                Close();
                throw new IOException("Cannot open port COM" + portNumber.ToString());
            }

            isOpen = true;

            SetTimeouts();

            stream = new FileStream(new SafeFileHandle(comPortHandle, true), FileAccess.ReadWrite);
            //Flush();
        }

        public void Close()
        {
            stream?.Dispose();
            isOpen = false;
        }

        private void SetTimeouts()
        {
            if (IsOpen)
            {
                CommTimeouts ct = new CommTimeouts();
                ct.ReadIntervalTimeout = readTimeout;
                ct.ReadTotalTimeoutMultiplier = 1;
                ct.ReadTotalTimeoutConstant = 0;
                ct.WriteTotalTimeoutConstant = writeTimeout;
                ct.WriteTotalTimeoutConstant = 0;

                if (!SetCommTimeouts(comPortHandle, ref ct))
                {
                    throw new IOException("Error setting timeouts");
                }
            }
        }

        public Task<byte[]> ReadBytesAsync(int count)
        {
            if (count < 1)
            {
                return null;
            }
            byte[] buffer = new byte[count];
            int i = 0, readBytes = 0, timeout = 0;

            return Task.Run(async () =>
            {
                while (i < count)
                {
                    if (!stream.CanRead)
                    {
                        return null;
                    }

                    readBytes = await stream.ReadAsync(buffer, i, count - i);
                    if (readBytes > 0)
                    {
                        i += readBytes;
                    }
                    else
                    {
                        System.Threading.Thread.Sleep(1);
                        timeout++;
                        if (timeout > readTimeout)
                        {
                            throw new TimeoutException("Read operation timeout");
                        }
                    }
                }
                return buffer;
            });
        }

        public byte[] ReadBytes(int count)
        {
            byte[] buffer = new byte[count];
            if (stream.CanRead)
            {
                if (stream.Read(buffer, 0, count) == count)
                {
                    return buffer;
                }
            }
            return null;
        }

        public async Task<byte> ReadByteAsync()
        {
            byte[] buffer = await ReadBytesAsync(1);
            return buffer[0];
        }

        public int ReadByte()
        {
            return stream.ReadByte();
        }

        public Task<string> ReadLineAsync()
        {
            char[] buffer = new char[16384];
            int length = 0;
            int readBytes;
            byte[] b = new byte[1];
            int timeout = 0;
            bool nl = false;

            return Task.Run(() =>
            {
                while (!nl)
                {
                    if (!stream.CanRead)
                    {
                        break;
                    }
                    readBytes = stream.Read(b, 0, 1);
                    if (readBytes > 0)
                    {
                        buffer[length] = Convert.ToChar(b[0]);
                        length++;
                    }
                    else
                    {
                        System.Threading.Thread.Sleep(1);
                        timeout++;
                        if (timeout > readTimeout)
                        {
                            throw new TimeoutException("Read operation timeout");
                        }
                    }

                    // detect new line
                    if (length >= newLine.Length)
                    {
                        nl = true;
                        for (int i = 0; i < newLine.Length; i++)
                        {
                            if (buffer[length - newLine.Length + i] != newLine[i])
                            {
                                nl = false;
                                break;
                            }
                        }
                    }
                }
                if (length >= newLine.Length)
                {
                    return new string(buffer, 0, length - newLine.Length);
                }
                else
                {
                    return null;
                }

            });
        }

        public string ReadLine()
        {
            char[] buffer = new char[16384];
            int length = 0;
            int readBytes;
            byte[] b = new byte[1];
            int timeout = 0;
            bool nl = false;
            while (!nl)
            {
                if (!stream.CanRead)
                {
                    break;
                }
                readBytes = stream.Read(b, 0, 1);
                if (readBytes > 0)
                {
                    buffer[length] = Convert.ToChar(b[0]);
                    length++;
                }
                else
                {
                    System.Threading.Thread.Sleep(1);
                    timeout++;
                    if (timeout > readTimeout)
                    {
                        throw new TimeoutException("Read operation timeout");
                    }
                }

                // detect new line
                if (length >= newLine.Length)
                {
                    nl = true;
                    for (int i = 0; i < newLine.Length; i++)
                    {
                        if (buffer[length - newLine.Length + i] != newLine[i])
                        {
                            nl = false;
                            break;
                        }
                    }
                }
            }
            return new string(buffer, 0, length - newLine.Length);
        }

        public void Flush()
        {
            ReadExisting();
            stream.Flush();
        }

        public byte[] ReadExisting()
        {
            byte[] buffer = new byte[16384];
            int readBytes = 0, length = 0;
            do
            {
                if (!stream.CanRead)
                {
                    break;
                }
                readBytes = stream.Read(buffer, length, 1);
                length += readBytes;
            } while (readBytes > 0);

            byte[] returnedBytes = new byte[length];
            for (int i = 0; i < length; i++)
            {
                returnedBytes[i] = buffer[i];
            }

            return returnedBytes;
        }

        public async void WriteAsync(byte[] buffer)
        {
            if (stream.CanWrite)
            {
                try
                {
                    await stream.WriteAsync(buffer, 0, buffer.Length);
                }
                catch (Exception) { }
            }
        }

        public void Write(byte[] buffer)
        {            
            if (stream.CanWrite)
            {
                stream.Write(buffer, 0, buffer.Length);
                stream.Flush();
            }
        }

        public async void WriteLineAsync(string line)
        {
            byte[] buffer;
            if (line.EndsWith(new string(newLine))) // end of line already present at the end of the string
            {
                buffer = System.Text.Encoding.Default.GetBytes(line);
            }
            else
            {
                buffer = System.Text.Encoding.Default.GetBytes(line + new string(newLine));
            }

            if (stream.CanWrite)
            {
                await stream.WriteAsync(buffer, 0, buffer.Length);
            }
        }        

        public FileStream BaseStream
        {
            get { return stream; }
        }

        public string PortName
        {
            get { return "COM" + portNumber.ToString(); }
        }

        public byte PortNumber
        {
            get { return portNumber; }
            set
            {
                if (value == 0)
                {
                    throw new ArgumentOutOfRangeException("Port number cannot be zero");
                }
                portNumber = value;
            }
        }

        public uint BaudRate
        {
            get { return baudRate; }
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

        public DTR_CONTROL DtrControl
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

        public RTS_CONTROL RtsControl
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
                SetTimeouts();
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
                SetTimeouts();
            }
        }

        public char[] NewLine
        {
            get { return NewLine; }
            set
            {
                if (value.Length == 0)
                {
                    throw new ArgumentException("New line character(s) cannot be an empty array");
                }

                newLine = value;
            }
        }

        public bool IsOpen
        {
            get { return isOpen; }
        }
    }

    // Following code for retrieving COM port friendly names is based on code by Dario Santarelli https://dariosantarelli.wordpress.com/2010/10/18/c-how-to-programmatically-find-a-com-port-by-friendly-name/
    // WMI calls to the more obvious Win32_SerialPort run very slowly on certain machines, hence the code below which calls to Win32_PnPEntity. Thank you Microsoft again :/

    internal class ProcessConnection
    {
        public static ConnectionOptions ProcessConnectionOptions()
        {
            ConnectionOptions options = new ConnectionOptions();
            options.Impersonation = ImpersonationLevel.Impersonate;
            options.Authentication = AuthenticationLevel.Default;
            options.EnablePrivileges = true;
            return options;
        }

        public static ManagementScope ConnectionScope(string machineName, ConnectionOptions options, string path)
        {
            ManagementScope connectScope = new ManagementScope();
            connectScope.Path = new ManagementPath(@"\\" + machineName + path);
            connectScope.Options = options;
            connectScope.Connect();
            return connectScope;
        }
    }

    public class COMPortInfo
    {
        public string Name { get; set; }
        public string Description { get; set; }

        public COMPortInfo() { }

        public static Task<List<COMPortInfo>> GetCOMPortsInfoAsync()
        {
            return Task.Run(() =>
            {
                List<COMPortInfo> comPortInfoList = new List<COMPortInfo>();

                ConnectionOptions options = ProcessConnection.ProcessConnectionOptions();
                ManagementScope connectionScope = ProcessConnection.ConnectionScope(Environment.MachineName, options, @"\root\CIMV2");

                ObjectQuery objectQuery = new ObjectQuery("SELECT * FROM Win32_PnPEntity WHERE ConfigManagerErrorCode = 0");
                ManagementObjectSearcher comPortSearcher = new ManagementObjectSearcher(connectionScope, objectQuery);

                using (comPortSearcher)
                {
                    string caption = null;
                    foreach (ManagementObject obj in comPortSearcher.Get())
                    {
                        if (obj != null)
                        {
                            object captionObj = obj["Caption"];
                            if (captionObj != null)
                            {
                                caption = captionObj.ToString();
                                if (caption.Contains("(COM"))
                                {
                                    COMPortInfo comPortInfo = new COMPortInfo();
                                    comPortInfo.Name = caption.Substring(caption.LastIndexOf("(COM")).Replace("(", string.Empty).Replace(")", string.Empty).Trim();
                                    comPortInfo.Description = caption.Trim();
                                    comPortInfoList.Add(comPortInfo);
                                }
                            }
                        }
                    }
                }
                return comPortInfoList;
            });
        }
    }
}
