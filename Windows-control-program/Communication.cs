using System;
using System.IO.Ports;
using System.Threading;
using System.Threading.Tasks;
using System.ComponentModel;
using System.Management;
using System.Collections.Generic;
using System.Text;

namespace MightyWatt
{
    public delegate void DataUpdateDelegate();
    public delegate void ConnectionUpdateDelegate();
    public enum ReadCommands : byte { Measurement = 1, IDN = 2, QDC = 3, ErrorMessages = 4 };
    public enum WriteCommands : byte { ConstantCurrent = 1, ConstantVoltage = 2, ConstantPower = 3, ConstantResistance = 4, ConstantVoltageSoftware = 5, MPPT = 6,
                                       SeriesResistance = 10, FourWire = 11, MeasurementFilter = 12, FanRules = 13, LEDRules = 14, LEDBrightness = 15, CurrentRangeAuto = 16, VoltageRangeAuto = 17};

    class Communication
    {
        // COM port
        private SerialPort port;
        private const Parity parity = Parity.None;
        private const StopBits stopBits = StopBits.One;
        private const int readTimeout = 500;
        private const int writeTimeout = 400;        
        public const int LoadDelay = readTimeout + writeTimeout;
        private const int baudRate = 500000;
        private const int dataBits = 8;
        private readonly char[] newLine = new char[] {'\r', '\n'};
        //private readonly string newLine = "\r\n";
        private string activePortName;
        public event ConnectionUpdateDelegate ConnectionUpdatedEvent;

        // communication        
        private const UInt16 COMMUNICATION_CRC_POLYNOMIAL_VALUE = 0x1021;
        private const byte measurementMessageLength = 16; // 14 bytes of data + 2 bytes CRC
        private const byte COMMUNICATION_READ = (0 << 7);
        private const byte COMMUNICATION_WRITE = (1 << 7);
        private readonly byte[] dataStageLength = new byte[] { 0, 1, 2, 4 }; // Length of payload
        private const string IdentificationString = "MightyWatt R3";
        private const UInt32 errorMask = 0x1BF832; /*0b110111111100000110010;*/ /*0x1FF8F2;*/  /*0b111111111100011110010*/ // Report only selected errors

        // communication (data in/out) loop
        private const int loopDelay = 1; // delay between read/write attempts
        private BackgroundWorker comLoop;
        public event DataUpdateDelegate DataUpdatedEvent;
        private byte[] dataItem;
        private string[] errorMessages;
        private Queue<byte[]> dataToWrite;

        // device capabilities
        private string calibrationDate, firmwareVersion, boardRevision;
        private double maxIdac, maxIadc, maxVdac, maxVadc, maxPower, dvmInputResistance;
        private int temperatureThreshold;

        // recent values
        private double current;
        private double voltage;
        private MeasurementValues values;
        private double temperature;
        private byte status;
        UInt32 errorFlags;
        private double seriesResistance;
        private bool remote;

        // DEBUG
        private int crcFails = 0;

        public Communication()
        {
            values = new MeasurementValues(voltage, current);
            // creates new serialport object and sets it
            port = new SerialPort();
            port.BaudRate = baudRate;
            port.DataBits = dataBits;
            port.Parity = parity;
            port.ReadTimeout = readTimeout;
            port.StopBits = stopBits;
            port.WriteTimeout = writeTimeout;
            port.NewLine = newLine;

            dataToWrite = new Queue<byte[]>();

            comLoop = new BackgroundWorker(); // background worker for the main communication loop
            comLoop.WorkerReportsProgress = false;
            comLoop.WorkerSupportsCancellation = true;
            comLoop.DoWork += new DoWorkEventHandler(comLoop_DoWork);
        }

        // connects to a specific COM port
        public void Connect(byte portNumber, bool rtsDtrEnable, int attempts)
        {
            if (attempts > 0)
            {
                Disconnect();
                port.PortNumber = portNumber;
                if (rtsDtrEnable)
                {
                    port.DtrControl = DTR_CONTROL.ENABLE;
                    port.RtsControl = RTS_CONTROL.ENABLE;
                    //port.DtrControl = true;
                    //port.RtsControl = true;
                }
                else
                {
                    port.DtrControl = DTR_CONTROL.DISABLE;
                    port.RtsControl = RTS_CONTROL.DISABLE;
                    //port.DtrControl = false;
                    //port.RtsControl = false;
                }

                port.Open();                
                while (port.IsOpen == false) { } // wait for port to open   
                Thread.Sleep(300); // give time to Arduinos that reset upon port opening     
                port.Flush();

                if (identify())
                {
                    queryCapabilities();
                    queryErrorMessages();
                    activePortName = port.PortName;
                    ConnectionUpdatedEvent?.Invoke(); // raise connection updated event
                    dataToWrite.Clear();
                    comLoop.RunWorkerAsync();
                }
                else
                {                    
                    port.Close();
                    while (port.IsOpen) { } // wait for port to close
                    Connect(portNumber, rtsDtrEnable, attempts - 1); // recursively call this function with one less attempt to try
                }
            }
            else
            {
                throw new System.IO.IOException("Wrong device");
            }
        }

        // disconnect from a COM port
        public void Disconnect()
        {
            if (port.IsOpen)
            {
                try
                {
                    comLoop.CancelAsync(); // stops monitoring for new data    
                    port.Close();
                    while (port.IsOpen) { } // wait for port to close
                }
                catch (System.IO.IOException ex)
                {
                    System.Windows.MessageBox.Show(ex.Message, "Error", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
                }
            }

            // resets all read data
            current = 0;
            voltage = 0;            
            temperature = 0;
            status = 0;
            errorFlags = 0;
            seriesResistance = 0;           

            // resets all port and device information
            activePortName = null;
            firmwareVersion = null;
            boardRevision = null;
            maxIdac = 0;
            maxIadc = 0;
            maxVdac = 0;
            maxVadc = 0;
            maxPower = 0;
            dvmInputResistance = 0;
            temperatureThreshold = 0;
            errorMessages = null;
            dataToWrite.Clear();

            // raise connection updated event
            ConnectionUpdatedEvent?.Invoke();

            // last data update (with reset data)
            DataUpdatedEvent?.Invoke(); // event for data update complete
        }

        // reads data from load and then raises update event 
        private void comLoop_DoWork(object sender, DoWorkEventArgs e)
        {
            //uint cyclecounter = 0;
            //DateTime start = DateTime.Now;
            while (!comLoop.CancellationPending)
            {
                try
                {
                    //if ((DateTime.Now - start).TotalMilliseconds > 10000)
                    //{
                    //    Console.WriteLine("Cycles per second: {0:f1}", Convert.ToDouble(cyclecounter) / 10.0);
                    //    cyclecounter = 0;
                    //    start = DateTime.Now;
                    //}

                    dataToWrite.Enqueue(new byte[] { COMMUNICATION_READ | ((byte)ReadCommands.Measurement & 0x7F) });
                    setToLoad();
                    if (readMeasurement())
                    {
                        DataUpdatedEvent?.Invoke(); // event for data update complete
                        //cyclecounter++;
                    }                    
                }
                catch (Exception ex)
                {
                    if (ex is TimeoutException || ex is System.IO.IOException)
                    {
                        Disconnect();
                        System.Windows.MessageBox.Show(ex.Message + "\nTo continue, please reconnect load.", "Error", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
                        return;
                    }
                    if (ex is InvalidOperationException)
                    {
                        System.Windows.MessageBox.Show(ex.Message, "Error", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
                        return;
                    }
                    throw;
                }
               // Thread.Sleep(loopDelay);
            }
            e.Cancel = true;
        }

        // sends the queued commands to the load
        private void setToLoad()
        {
            while (dataToWrite.Count > 0)
            {
                byte[] data = dataToWrite.Dequeue();
                if (data != null)
                {
                    byte[] dataWithCRC = new byte[data.Length + 2];
                    ushort crc = CRC16(COMMUNICATION_CRC_POLYNOMIAL_VALUE, data, data.Length);
                    int i;
                    // copy data
                    for (i = 0; i < data.Length; i++)
                    {
                        dataWithCRC[i] = data[i];
                    }
                    // append CRC
                    dataWithCRC[i] = Convert.ToByte(crc & 0xFF);
                    dataWithCRC[i + 1] = Convert.ToByte((crc >> 8) & 0xFF);
                //if (dataWithCRC.Length > 3)
                //{
                //    Console.WriteLine("Data length: {0}", dataWithCRC.Length);
                //    foreach (byte b in dataWithCRC)
                //    {
                //        Console.Write("0x{0:X}\t", b);
                //    }
                //}
                port.Write(dataWithCRC);
                }
            }     
        }

        // reads available measurement from the load, returns true if successful
        private bool readMeasurement()
        {            
            byte[] newData = port.ReadBytes(measurementMessageLength);
            if (newData != null)
            {
                if (newData.Length == measurementMessageLength) // data have been received in full
                {
                    // check CRC
                    ushort crc = CRC16(COMMUNICATION_CRC_POLYNOMIAL_VALUE, newData, measurementMessageLength - 2);
                    if (crc != Convert.ToUInt16((newData[14] | (newData[15] << 8)) & 0xFFFF))
                    {
                        // CRC check failed, drop data
                        //port.Flush(); // clear stream
                        Console.WriteLine("CRC check failed. Fails so far: {0}.", ++crcFails);
                        return false;
                    }
                    else
                    {
                        UInt32 current = (UInt32)newData[0] | ((UInt32)newData[1] << 8) | ((UInt32)newData[2] << 16) | ((UInt32)newData[3] << 24);
                        UInt32 voltage = (UInt32)newData[4] | ((UInt32)newData[5] << 8) | ((UInt32)newData[6] << 16) | ((UInt32)newData[7] << 24);

                        this.current = Convert.ToDouble(current) / 1e6;
                        this.voltage = Convert.ToDouble(voltage) / 1e6;
                        values.current = this.current;
                        values.voltage = this.voltage;

                        temperature = Convert.ToDouble(newData[8]);
                        status = newData[9];
                        remote = Flag(status, 5);

                        errorFlags |= ((UInt32)newData[10] | ((UInt32)newData[11] << 8) | ((UInt32)newData[12] << 16) | ((UInt32)newData[13] << 24)) & errorMask; // only add to error flags

                        return true;
                    }
                }                
            }

            return false;
        }

        // tries to read identification string from the load, return true if successful
        private bool identify()
        {         
            try
            {
                Query((byte)ReadCommands.IDN);
                string response = port.ReadLine();
                return response.Contains(IdentificationString);
            }
            catch (Exception)
            {
                return false;
            }
        }

        // reads device parameters
        private void queryCapabilities()
        {
            Query((byte)ReadCommands.QDC);
            calibrationDate = port.ReadLine();
            firmwareVersion = port.ReadLine();
            boardRevision = port.ReadLine();
            maxIdac = Double.Parse(port.ReadLine()) / 1e6;
            maxIadc = Double.Parse(port.ReadLine()) / 1e6;
            maxVdac = Double.Parse(port.ReadLine()) / 1e6;
            maxVadc = Double.Parse(port.ReadLine()) / 1e6;
            maxPower = Double.Parse(port.ReadLine()) / 1e6;
            dvmInputResistance = Double.Parse(port.ReadLine()) / 1000; /* Differential input resistance */
            temperatureThreshold = int.Parse(port.ReadLine());

            // check firmware version
            string[] firmware = firmwareVersion.Split('.');
            bool firmwareVersionOK = false;
            if (firmware.Length >= 3)
            {
                int[] fw = new int[3];
                if (int.TryParse(firmware[0], out fw[0]) && int.TryParse(firmware[1], out fw[1]) && int.TryParse(firmware[2], out fw[2]))
                {
                    if (fw[0] > Load.MinimumFWVersion[0])
                    {
                        firmwareVersionOK = true;
                    }
                    else if (fw[0] == Load.MinimumFWVersion[0])
                    {
                        if (fw[1] > Load.MinimumFWVersion[1])
                        {
                            firmwareVersionOK = true;
                        }
                        else if (fw[1] == Load.MinimumFWVersion[1])
                        {
                            if (fw[2] >= Load.MinimumFWVersion[2])
                            {
                                firmwareVersionOK = true;
                            }
                        }
                    }
                }
                if (!firmwareVersionOK)
                {
                    System.Windows.MessageBox.Show("Firmware version is lower than minimum required version for this software\nMinimum firmware version: " + Load.MinimumFirmwareVersion + "\nThis load firmware version: " + firmwareVersion, "Firmware version error", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Exclamation);
                }                
            }  
            else
            {
                System.Windows.MessageBox.Show("The load did not report its firmware version", "Unknown firmware version", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Exclamation);
            }             
        }

        // Gets the list of error messages from load (not the present errors, only the string representations of possible errors)
        private void queryErrorMessages() 
        {
            Query((byte)ReadCommands.ErrorMessages);
            int length = port.ReadByte();
            if (length > 0)
            {
                errorMessages = new string[length];
                for (int i = 0; i < errorMessages.Length; i++)
                {
                    errorMessages[i] = port.ReadLine();
                }
            }
        }

        // this method handles the communication protocol of sending the data to the load
        public void Set(Modes mode, double value)
        {
            if (port.IsOpen)
            {
                validateValues(mode, value); // validate input
                dataItem = new byte[5];

                dataItem[0] = COMMUNICATION_WRITE;
                for (byte i = 0; i < dataStageLength.Length; i++)
                {
                    if (dataStageLength[i] == dataItem.Length - 1)
                    {
                        dataItem[0] |= (byte)(i << 5);
                        break;
                    }
                }

                UInt32 val;
                switch (mode)
                {
                    case Modes.Current:
                        val = Convert.ToUInt32(value * 1e6);
                        dataItem[0] |= (byte)WriteCommands.ConstantCurrent;
                        break;
                    case Modes.Voltage:
                        val = Convert.ToUInt32(value * 1e6);
                        dataItem[0] |= (byte)WriteCommands.ConstantVoltage;
                        break;
                    case Modes.VoltageSoftware:
                        val = Convert.ToUInt32(value * 1e6);
                        dataItem[0] |= (byte)WriteCommands.ConstantVoltageSoftware;
                        break;
                    case Modes.MPPT:
                        val = Convert.ToUInt32(value * 1e6);
                        dataItem[0] |= (byte)WriteCommands.MPPT;
                        break;
                    case Modes.Power:
                        val = Convert.ToUInt32(value * 1e6);
                        dataItem[0] |= (byte)WriteCommands.ConstantPower;
                        break;
                    case Modes.Resistance:
                        val = Convert.ToUInt32(value * 1000);
                        dataItem[0] |= (byte)WriteCommands.ConstantResistance;
                        break;
                    default:
                        return;
                }

                dataItem[1] = Convert.ToByte(val & 0xFF);
                dataItem[2] = Convert.ToByte((val >> 8) & 0xFF);
                dataItem[3] = Convert.ToByte((val >> 16) & 0xFF);
                dataItem[4] = Convert.ToByte((val >> 24) & 0xFF);

                dataToWrite.Enqueue(dataItem);
            }
        }

        // immediately stops the load by setting the current to zero
        public void ImmediateStop()
        {
            dataToWrite.Clear();
            Set(Modes.Current, 0);
            try
            {
                setToLoad();
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.Message, "Error", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
            }
        }

        // checks values for validity
        private void validateValues(Modes mode, double value)
        {
            if (activePortName != null) // values are validated only when a device is connected
            {
                switch (mode)
                {
                    case Modes.Current:
                    /*case Modes.MPPT:*/
                        {
                            if ((value > MaxIadc) || (value > MaxIdac) || (value < 0))
                            {
                                throw new ArgumentOutOfRangeException("Set current out of range.", (Exception)null);
                            }
                            break;
                        }
                    case Modes.Power:
                        {
                            if ((value > MaxPower) || (value < 0))
                            {
                                throw new ArgumentOutOfRangeException("Set power out of range.", (Exception)null);
                            }
                            break;
                        }
                    case Modes.Resistance:
                        {
                            if ((value > DvmInputResistance) || (value < 0))
                            {
                                throw new ArgumentOutOfRangeException("Set resistance out of range.", (Exception)null); ;
                            }
                            break;
                        }
                    case Modes.Voltage:
                    case Modes.MPPT:
                        {
                            if ((value > MaxVadc) || (value > MaxVdac) || (value < 0))
                            {
                                throw new ArgumentOutOfRangeException("Set voltage out of range.", (Exception)null);
                            }
                            break;
                        }
                    case Modes.VoltageSoftware:
                        {
                            if ((value > MaxVadc) || (value < 0))
                            {
                                throw new ArgumentOutOfRangeException("Set voltage out of range.", (Exception)null);
                            }
                            break;
                        }
                    default:
                        {
                            throw new System.IO.InvalidDataException("Invalid input");
                        }
                }
            }           
        }

        // reset error flags
        public void ClearErrors()
        {
            errorFlags = 0;
        }

        // gets selected value of I, P, R or V
        public double GetValue(Modes mode)
        {
            switch (mode)
            {
                case Modes.Current:
                /*case Modes.MPPT:*/
                    {
                        return current;
                    }
                case Modes.Power:                
                    {
                        return voltage * current;
                    }
                case Modes.Resistance:
                    {
                        if (current == 0)
                        {
                            return DvmInputResistance; // zero current reflects to voltmeter input resistance
                        }
                        else
                        {
                            return voltage / current;
                        }
                    }
                case Modes.Voltage:
                case Modes.VoltageSoftware:
                case Modes.MPPT:
                    {
                        return voltage;
                    }
                default:
                    return 0;
            }            
        }

        // gets or sets series resistance to the load
        public double SeriesResistance
        {
            get
            {
                return seriesResistance;
            }
            set
            {
                UInt32 val = Convert.ToUInt32(value * 1000);
                byte[] data = new byte[5];
                data[0] = COMMUNICATION_WRITE;
                data[0] |= (byte)WriteCommands.SeriesResistance;
                for (byte i = 0; i < dataStageLength.Length; i++)
                {
                    if (dataStageLength[i] == data.Length - 1)
                    {
                        data[0] |= (byte)(i << 5);
                        break;
                    }
                }
                data[1] = Convert.ToByte(val & 0xFF);
                data[2] = Convert.ToByte((val >> 8) & 0xFF);
                data[3] = Convert.ToByte((val >> 16) & 0xFF);
                data[4] = Convert.ToByte((val >> 24) & 0xFF);

                dataToWrite.Enqueue(data);
                seriesResistance = value;
            }
        }

        // set any command to device
        public void SetValue(WriteCommands command, byte payload)
        {
            byte[] data = new byte[2];
            data[0] = COMMUNICATION_WRITE;
            data[0] |= (byte)command;
            data[0] |= (1 << 5);
            data[1] = payload;

            dataToWrite.Enqueue(data);
        }

        // set any command to device
        public void SetValue(WriteCommands command, UInt16 payload)
        {
            byte[] data = new byte[3];
            data[0] = COMMUNICATION_WRITE;
            data[0] |= (byte)command;
            data[0] |= (2 << 5);
            data[1] = Convert.ToByte(payload & 0xFF);
            data[2] = Convert.ToByte((payload >> 8) & 0xFF);

            dataToWrite.Enqueue(data);
        }

        // set any command to device
        public void SetValue(WriteCommands command, UInt32 payload)
        {
            byte[] data = new byte[5];
            data[0] = COMMUNICATION_WRITE;
            data[0] |= (byte)command;
            data[0] |= (2 << 5);
            data[1] = Convert.ToByte(payload & 0xFF);
            data[2] = Convert.ToByte((payload >> 8) & 0xFF);
            data[3] = Convert.ToByte((payload >> 16) & 0xFF);
            data[4] = Convert.ToByte((payload >> 24) & 0xFF);
            dataToWrite.Enqueue(data);
        }

        // checks whether flag is on
        private bool Flag(UInt32 flagWord, byte position)
        {
            return Convert.ToBoolean((flagWord >> position) & 1);
        }

        // Query device without waiting for response
        private void Query(byte command)
        {
            byte[] dataWithCRC = new byte[3];
            // copy data
            dataWithCRC[0] = (byte)(COMMUNICATION_READ | (command & 0x7F));
            // calculate CRC
            ushort crc = CRC16(COMMUNICATION_CRC_POLYNOMIAL_VALUE, dataWithCRC, 1);            
            // append CRC
            dataWithCRC[1] = Convert.ToByte(crc & 0xFF);
            dataWithCRC[2] = Convert.ToByte((crc >> 8) & 0xFF);
            port.Write(dataWithCRC);
        }

        // Computes 16-bit cyclic redundancy check on array of binary data
        public static ushort CRC16(ushort polynomial, byte[] data, int length)
        {
            ushort crc = 0;
            for (byte i = 0; i < Math.Min(data.Length, length); i++)
            {
                crc ^= Convert.ToUInt16(Convert.ToUInt16(data[i]) << 8);
                for (byte j = 0; j < 8; j++)
                {
                    if ((crc & 0x8000U) != 0)
                    {
                        crc = Convert.ToUInt16(Convert.ToUInt16((crc << 1) & 0xFFFFU) ^ polynomial);
                    }
                    else
                    {
                        crc = Convert.ToUInt16((crc << 1) & 0xFFFFU);
                    }
                }
            }
            return crc;
        }

        public MeasurementValues PresentValues
        {
            get
            {                
                return values;             
            }
        }

        public string PortName
        {
            get
            {
                return activePortName;
            }
        }

        public string CalibrationDate
        {
            get
            {
                return calibrationDate;
            }
        }

        public string FirmwareVersion
        {
            get
            {
                return firmwareVersion;
            }
        }

        public string BoardRevision
        {
            get
            {
                return boardRevision;
            }
        }

        public double MaxIdac
        {
            get
            {
                return maxIdac;
            }
        }

        public double MaxIadc
        {
            get
            {
                return maxIadc;
            }
        }

        public double MaxVdac
        {
            get
            {
                return maxVdac;
            }
        }

        public double MaxVadc
        {
            get
            {
                return maxVadc;
            }
        }

        public double MaxPower
        {
            get
            {
                return maxPower;
            }
        }

        public double DvmInputResistance
        {
            get
            {
                return dvmInputResistance;
            }
        }

        public int TemperatureThreshold
        {
            get
            {
                return temperatureThreshold;
            }

        }

        public double Temperature
        {
            get
            {
                return temperature;
            }
        }

        public bool Remote
        {
            get
            {
                return remote;
            }
            set
            {                
                byte[] data = new byte[2];
                data[0] = COMMUNICATION_WRITE;
                data[0] |= (byte)WriteCommands.FourWire;
                for (byte i = 0; i < dataStageLength.Length; i++)
                {
                    if (dataStageLength[i] == data.Length - 1)
                    {
                        data[0] |= (byte)(i << 5);
                        break;
                    }
                }
                data[1] = Convert.ToByte(value);
                dataToWrite.Enqueue(data);
                remote = value;
            }
        }

        // persistent list, use ClearErrors to clear this list
        public string ErrorList
        {
            get
            {
                StringBuilder sb = new StringBuilder();
                if (errorFlags > 0)
                {
                    sb.Append("Following errors were detected:");
                    for (byte i = 0; i < errorMessages.Length; i++)
                    {
                        if (Flag(errorFlags, i))
                        {
                            sb.Append("\n");
                            sb.Append(errorMessages[i]);
                        }
                    }
                }
                return sb.ToString();
            }
        }
    }    
}
