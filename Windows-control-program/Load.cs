using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows;
using System.IO;
using System.ComponentModel;

namespace MightyWatt
{
    public delegate void GuiUpdateDelegate();
    public delegate void WatchdogStopDelegate();
    public delegate void ErrorDelegate(string error);        

    public class Load : INotifyPropertyChanged
    {
        // events
        public event PropertyChangedEventHandler PropertyChanged;

        // connection
        private Communication device;
        private bool isConnected = false;        

        // program
        public BindingList<ProgramItem> ProgramItems = new BindingList<ProgramItem>();
        private List<DateTime> programItemsStartTime;
        private BackgroundWorker worker;       
        private bool isManual = true; // indicates whether the load control is manual or programmatic       
        private bool cancel = false; // this cancels execution of a single program item
        private int currentItemNumber; // number of currently executed program item

        // loops
        private int currentLoop; // current loop number
        public int TotalLoops { get; set; } // total number of loops, 0 for infinite

        // GUI
        //public static readonly string[] ModeNames = { "Current", "Voltage", "Power (CC)", "Power (CV)", "Resistance (CC)", "Resistance (CV)", "SW controlled voltage", "Max power point tracker", "Simple ammeter", "Temperature" };
        //public static readonly string[] ModeNamesWatchdogAndSkip = { "Current", "Voltage", "Power", "Power", "Resistance", "Resistance", "Voltage", "Voltage", "Current", "Temperature" };
        //public static readonly string[] UnitSymbols = { "A", "V", "W", "W", "Ω", "Ω", "V", "V", "", "°C"};
        public static readonly string[] Pins = {"Arduino pin 2", "Arduino pin 6", "Arduino pin 7", "Arduino pin 10", "Arduino pin 13"};
        DateTime lastGuiUpdate = DateTime.Now;
        private double guiUpdatePeriod = 0.2;
        public event GuiUpdateDelegate GuiUpdateEvent;
        public event GuiUpdateDelegate ProgramStartedEvent; // occurs when program starts
        public event GuiUpdateDelegate ProgramStoppedEvent; // occurs when program finishes
        public event ConnectionUpdateDelegate ConnectionUpdateEvent; // occurs when connection of load changes
        public event DataUpdateDelegate DataUpdated; // pass dataupdate event from connection

        // error management
        public event ErrorDelegate Error;

        // watchdog
        public event WatchdogStopDelegate WatchdogStop; // event that is raised when watchdog has stopped the load
        public bool WatchdogEnabled { get; set; }
        public WDandSkipMode WatchdogMode { get; set; }
        public Comparison WatchdogCompare { get; set; }
        public string WatchdogValue { get; set; }

        // logging
        private File file;    
        private double loggingPeriod = 1; // logging period in seconds
        public TimeUnits LoggingTimeUnit { get; set; } // time units for logging
        private DateTime lastManualLog = DateTime.MinValue;
        private DateTime lastProgramLog = DateTime.MinValue;
        private double lastLogSecondDifference = 0;

        // minimum firmware version
        public static readonly int[] MinimumFWVersion = new int[] { 3, 1, 4 };

        // LED, fan, measurements filter and autoranging settings
        public const LEDBrightnesses DefaultLEDBrightness = LEDBrightnesses.Medium;
        public const byte DefaultLEDRule = (byte)LEDRules.I1;
        public const FanRules DefaultFanRule = FanRules.AlwaysOn;
        public const MeasurementFilters DefaultMeasurementFilter = MeasurementFilters.Filtered;
        public const bool DefaultAutorangingCurrent = true;
        public const bool DefaultAutorangingVoltage = true;

        private LEDBrightnesses _LEDBrightness = DefaultLEDBrightness;
        private byte _LEDRule = DefaultLEDRule;
        private FanRules _FanRule = DefaultFanRule;
        private MeasurementFilters _MeasurementFilter = DefaultMeasurementFilter;
        private bool _AutorangingCurrent = DefaultAutorangingCurrent;
        private bool _AutorangingVoltage = DefaultAutorangingVoltage;

        // external resistance watchdog
        public event WatchdogStopDelegate SeriesResistanceWatchdogStop; // event that is raised when series resistance watchdog has stopped the load
        private double seriesResistancePowerLimit = 100;
        private DateTime lastSeriesResistancePowerLimitOKTime = DateTime.Now;

        public Load()
        {
            this.device = new Communication(); // load over COM port         
            device.PropertyChanged += Device_PropertyChanged;   

            device.ConnectionUpdatedEvent += connectionUpdated; // pass connection updated event
            device.DataUpdatedEvent += log; // data logging    
            device.DataUpdatedEvent += updateGui; // updates 
            device.DataUpdatedEvent += checkError; // errors
            device.DataUpdatedEvent += watchdog; // watchdog
            device.DataUpdatedEvent += seriesResistanceWatchdog; // series resistance watchdog                 

            device.ConnectionUpdatedEvent += SetDefault;// LED brightness, LED rules, Fan rules, measurement filter;

            TotalLoops = 1; // standard single loop
            LoggingTimeUnit = TimeUnits.s; // default unit second
        }

        private void Device_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(Communication.UserPins))
            {
                NotifyPropertyChanged(nameof(UserPin0));
                NotifyPropertyChanged(nameof(UserPin1));
                NotifyPropertyChanged(nameof(UserPin2));
                NotifyPropertyChanged(nameof(UserPin3));
                NotifyPropertyChanged(nameof(UserPin4));
            }
        }

        // connect to selected COM port
        public void Connect(string portName, Boards board)
        {
            if (IsConnected)
            {
                Disconnect();
            }

            try
            {
                device.Connect(byte.Parse(portName.Replace("COM", string.Empty)), board == Boards.Zero, 2 /* two attempts */);
                // write current settings
                device.SetValue(WriteCommands.LEDBrightness, (byte)LEDBrightness);
                device.SetValue(WriteCommands.LEDRules, LEDRule);
                device.SetValue(WriteCommands.FanRules, (byte)FanRule);
                device.SetValue(WriteCommands.MeasurementFilter, (byte)MeasurementFilter);
                device.SetValue(WriteCommands.CurrentRangeAuto, Convert.ToByte(AutorangingCurrent));
                device.SetValue(WriteCommands.VoltageRangeAuto, Convert.ToByte(AutorangingVoltage));
                device.Set(RunMode.Current, 0);
            }
            catch (IOException ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        // disconnect from COM port
        public void Disconnect()
        {
            try
            {
                ImmediateStop();
                //device.Set(Modes.Current, 0);
                System.Threading.Thread.Sleep(Communication.LoadDelay); // wait for setting the load to zero current
                this.device.Disconnect();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        // pass connection updated event and update isConnected property
        private void connectionUpdated()
        {
            if (device.PortName == null)
            {
                this.isConnected = false;
            }
            else
            {
                this.isConnected = true;                
            }
            ConnectionUpdateEvent();
        }

        // passes the set method
        public void Set(RunMode mode, double value)
        {
            device.Set(mode, value);
        }

        // passes the set remote method
        public void SetRemote(bool remoteEnabled)
        {
            device.Remote = remoteEnabled;
        }

        // starts execution of program
        public void Start()
        {
            if (worker != null)
            {
                if (worker.IsBusy)
                {
                    cancel = true;
                    worker.CancelAsync();
                }
            }

            while (!isManual) // wait for the previous worker to finish
            {                
                System.Threading.Thread.Sleep(1);
            }
            isManual = false;
            ProgramStartedEvent?.Invoke(); // raise program started event

            // worker for program mode
            worker = new BackgroundWorker();
            worker.DoWork += worker_DoWork;
            worker.RunWorkerCompleted += worker_Finished;
            worker.WorkerSupportsCancellation = true;
            worker.WorkerReportsProgress = false;
            worker.RunWorkerAsync(); // starts execution in different thread
        }

        // cancels worker, stopping the execution of program or stops the load in manual mode
        public void Stop()
        {
            if (worker != null)
            {
                if (worker.IsBusy)
                {
                    cancel = true;
                    worker.CancelAsync();                    
                }                                
            }
            ImmediateStop();
        }

        // immediately stops the load, called in emergency (watchdog, overloads)
        public void ImmediateStop()
        {
            device.ImmediateStop();
        }

        // stops the load but sends any data already in the queue
        public void FinishAndStop()
        {
            device.Stop();
        }

        // skips execution of single program line
        public void Skip()
        {
            if (worker != null)
            {
                if (worker.IsBusy)
                {
                    cancel = true;
                }
            }
        }

        // runs constant mode program item
        private void Constant(int programItemNumber)
        {
            double setValue;

            if (ProgramItems[programItemNumber].Value != null)
            {
                setValue = (double)(ProgramItems[programItemNumber].Value);
            }
            else
            {
                setValue = device.GetValue(ProgramItems[programItemNumber].Mode);
            }
            programItemsStartTime.Add(DateTime.Now);
            // loop
            while (((DateTime.Now - programItemsStartTime[programItemNumber]).TotalSeconds < ProgramItems[programItemNumber].Duration) && (cancel == false))
            {
                if (ProgramItems[programItemNumber].SkipEnabled) // check skip condition
                {
                    if (valueComparer(ProgramItems[programItemNumber].SkipValue, ProgramItems[programItemNumber].SkipMode, ProgramItems[programItemNumber].SkipComparator))
                    {
                        break;
                    }
                }

                while (device.QueueCount > 2)
                {
                    System.Threading.Thread.Sleep(10); // decrease CPU load
                }

                device.Set(ProgramItems[programItemNumber].Mode, setValue);
            }
            cancel = false;
        }

        // runs ramp mode program item
        private void Ramp(int programItemNumber)
        {
            double startValue, currentValue;
            if (ProgramItems[programItemNumber].StartingValue != null)
            {
                startValue = (double)(ProgramItems[programItemNumber].StartingValue);
            }
            else
            {
                startValue = device.GetValue(ProgramItems[programItemNumber].Mode);
            }
            programItemsStartTime.Add(DateTime.Now);
            bool start = true;
            // loop
            while ((cancel == false) && ((DateTime.Now - programItemsStartTime[programItemNumber]).TotalSeconds < ProgramItems[programItemNumber].Duration))
            {
                if (ProgramItems[programItemNumber].SkipEnabled) // check skip condition
                {
                    if (valueComparer(ProgramItems[programItemNumber].SkipValue, ProgramItems[programItemNumber].SkipMode, ProgramItems[programItemNumber].SkipComparator))
                    {
                        break;
                    }
                }

                while (device.QueueCount > 2)
                {
                    System.Threading.Thread.Sleep(1); // decrease CPU load
                }

                if (start)
                {
                    device.Set(ProgramItems[programItemNumber].Mode, startValue);
                    start = false;
                }
                else
                {
                    currentValue = startValue + (ProgramItems[programItemNumber].FinalValue - startValue) / ProgramItems[programItemNumber].Duration * ((DateTime.Now - programItemsStartTime[programItemNumber]).TotalSeconds);
                    if (ProgramItems[programItemNumber].StartingValue <= ProgramItems[programItemNumber].FinalValue)
                    {
                        currentValue = Math.Min(currentValue, ProgramItems[programItemNumber].FinalValue);
                    }
                    else
                    {
                        currentValue = Math.Max(currentValue, ProgramItems[programItemNumber].FinalValue);
                    }

                    device.Set(ProgramItems[programItemNumber].Mode, currentValue);
                }
            }
            cancel = false;            
        }

        // sets or resets pin(s) in program mode
        private void Pin(int programItemNumber)
        {
            byte pinNumber = ProgramItems[programItemNumber].Pin;
            programItemsStartTime.Add(DateTime.Now);

            if (ProgramItems[programItemNumber].SetUserPin)
            {
                if (pinNumber < Pins.Length)
                {
                    device.SetValue(WriteCommands.UserPins, Convert.ToByte((1 << pinNumber) | 0x80));
                }
                else
                {
                    device.SetValue(WriteCommands.UserPins, Convert.ToByte(((1 << Pins.Length) - 1) | 0x80));
                }
            }
            else
            {
                if (pinNumber < Pins.Length)
                {
                    device.SetValue(WriteCommands.UserPins, Convert.ToByte((1 << pinNumber)));
                }
                else
                {
                    device.SetValue(WriteCommands.UserPins, Convert.ToByte(((1 << Pins.Length) - 1)));
                }
            }
        }

        // program execution
        private void worker_DoWork(object sender, DoWorkEventArgs e)
        {
            currentLoop = 0;
            while ((currentLoop < TotalLoops) || (TotalLoops == 0)) // loop while current number of loop is less than the total number of loops, or there is an infinite loop (TotalLoops = 0)
            {
                programItemsStartTime = new List<DateTime>(); // creates fresh list with start times

                for (currentItemNumber = 0; currentItemNumber < ProgramItems.Count; currentItemNumber++)
                {
                    if (((BackgroundWorker)sender).CancellationPending)
                    {
                        break;
                    }

                    if (ProgramItems[currentItemNumber].ProgramMode == ProgramModes.Constant)
                    {
                        Constant(currentItemNumber);
                    }
                    else if (ProgramItems[currentItemNumber].ProgramMode == ProgramModes.Ramp)
                    {
                        Ramp(currentItemNumber);
                    }
                    else if (ProgramItems[currentItemNumber].ProgramMode == ProgramModes.Pin)
                    {
                        Pin(currentItemNumber);
                    }
                }

                if (((BackgroundWorker)sender).CancellationPending)
                {
                    break;
                }

                currentLoop++;
            }
        }

        // final cleanup after program is finished or stopped
        private void worker_Finished(object sender, RunWorkerCompletedEventArgs e)
        {            
            FinishAndStop();
            isManual = true;
            file?.Flush();
            ProgramStoppedEvent?.Invoke(); // raise program started event
        }
        
        // returns time elapsed since the start of a single program item
        private double elapsed(int programItemNumber)
        {
            if (programItemsStartTime != null)
            {
                if (programItemsStartTime.Count > programItemNumber)
                {
                    return (DateTime.Now - programItemsStartTime[programItemNumber]).TotalSeconds;
                }                
            }            
            return 0;
        }

        // checks for errors from the load
        private void checkError()
        {
            if (string.IsNullOrEmpty(device.ErrorList) == false)
            {
                Stop(); // stops the load for safety
                this.Error(this.device.ErrorList); // raise error event in case the error list is not empty
                device.ClearErrors();
            }
        }

        // can disable load based on values in the watchdog group box
        private void watchdog()
        {
            if (WatchdogEnabled)
            {
                double value;
                if (double.TryParse(this.WatchdogValue, out value))
                {
                    if (valueComparer(value, this.WatchdogMode, this.WatchdogCompare))
                    {
                        watchdogStop();
                    }                    
                }
            }
        }

        // can disable load based on series resistance power limit
        private void seriesResistanceWatchdog()
        {
            // limiting is enabled and the present power limit of the series resistance is exceeded…
            if (SeriesResistancePowerLimitIsEnabled && (SeriesResistancePowerLimit < SeriesResistance * Math.Pow(Current, 2)))
            {
                // …for more than one second, stop the load
                if ((DateTime.Now - lastSeriesResistancePowerLimitOKTime).TotalSeconds > 1)
                {
                    seriesResistanceWatchdogStop();
                }
            }
            else
            {
                // update timer
                lastSeriesResistancePowerLimitOKTime = DateTime.Now;
            }            
        }

        // compares the current measured quantity to a given value
        // returns true if the measured quantity is less than / more than [comparator] the given value
        private bool valueComparer(double value, RunMode mode, Comparison comparator)
        {
            switch (mode)
            {
                case RunMode.Current:
                case RunMode.SimpleAmmeter:
                    {
                        return (comparator == Comparison.LessThan) == (Current < value);
                    }
                case RunMode.MPPT:
                case RunMode.Power_CC:
                case RunMode.Power_CV:
                    {
                        return (comparator == Comparison.LessThan) == (Power < value);
                    }
                case RunMode.Resistance_CC:
                case RunMode.Resistance_CV:
                    {
                        return (comparator == Comparison.LessThan) == (Resistance < value);
                    }
                case RunMode.Voltage:
                case RunMode.VoltageSoftware:
                    {
                        return (comparator == Comparison.LessThan) == (Voltage < value);
                    }
            }

            throw new NotImplementedException();
        }

        // compares the current measured quantity to a given value
        // returns true if the measured quantity is less than / more than [comparator] the given value
        private bool valueComparer(double value, WDandSkipMode mode, Comparison comparator)
        {
            switch (mode)
            {
                case WDandSkipMode.Current:
                    {
                        return (comparator == Comparison.LessThan) == (Current < value);
                    }
                case WDandSkipMode.Voltage:
                    {
                        return (comparator == Comparison.LessThan) == (Voltage < value);
                    }
                case WDandSkipMode.Power:
                    {
                        return (comparator == Comparison.LessThan) == (Power < value);
                    }
                case WDandSkipMode.Resistance:
                    {
                        return (comparator == Comparison.LessThan) == (Resistance < value);
                    }
                case WDandSkipMode.Temperature:
                    {
                        return (comparator == Comparison.LessThan) == (Temperature < value);
                    }
            }

            throw new NotImplementedException();
        }

        // compares the current measured quantity to a given value
        // returns true if the measured quantity is less than / more than [comparator] the given value
        private bool valueComparer(double value, RampMode mode, Comparison comparator)
        {
            switch (mode)
            {
                case RampMode.Current:
                    {
                        return (comparator == Comparison.LessThan) == (Current < value);
                    }
                case RampMode.Power_CC:
                case RampMode.Power_CV:
                    {
                        return (comparator == Comparison.LessThan) == (Power < value);
                    }
                case RampMode.Resistance_CC:
                case RampMode.Resistance_CV:
                    {
                        return (comparator == Comparison.LessThan) == (Resistance < value);
                    }
                case RampMode.Voltage:
                case RampMode.VoltageSoftware:
                    {
                        return (comparator == Comparison.LessThan) == (Voltage < value);
                    }
            }

            throw new NotImplementedException();
        }

        // stops the load and raises WatchdogStop event
        private void watchdogStop()
        {
            if (worker != null)
            {
                if (worker.IsBusy)
                {
                    cancel = true;
                    worker.CancelAsync();
                }
            }
            ImmediateStop(); // immediately stop the load            
            WatchdogStop?.Invoke();
        }

        // stops the load and raises SeriesResistanceWatchdogStop event
        private void seriesResistanceWatchdogStop()
        {
            if (worker != null)
            {
                if (worker.IsBusy)
                {
                    cancel = true;
                    worker.CancelAsync();
                }
            }
            ImmediateStop(); // immediately stop the load            
            SeriesResistanceWatchdogStop?.Invoke();
        }

        // creates a new file for logging
        public void NewFile(string filePath) 
        {
            CloseFile();
            try
            {
                file = new File(filePath);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        // closes current file
        public void CloseFile() 
        {
            if (file != null)
            {
                file.Close();
            }
        }

        // manages data logging        
        private void log()
        {
            if (file?.FilePath != null)
            {
                DateTime now = DateTime.Now;
                if (lastLogSecondDifference > 1)
                {
                    // Reset last log difference if the value seems not to be valid
                    lastLogSecondDifference = 0;
                }

                // manual control logging
                if (IsLoggingManual && isManual)
                {                        
                    if ((now - lastManualLog).TotalSeconds >= (LoggingPeriod - lastLogSecondDifference))
                    {
                        lastLogSecondDifference += (now - lastManualLog).TotalSeconds - LoggingPeriod;
                        file.WriteData(Current, Voltage, Temperature, Remote);
                        lastManualLog = now;
                    }
                }
                // program control logging
                else if (IsLoggingProgram && !isManual)
                {
                    if ((now - lastProgramLog).TotalSeconds >= (LoggingPeriod - lastLogSecondDifference))
                    {
                        lastLogSecondDifference += (now - lastProgramLog).TotalSeconds - LoggingPeriod;
                        //Console.WriteLine("Logging period: {0}, difference: {1}", LoggingPeriod, lastLogSecondDifference);
                        file.WriteData(Current, Voltage, Temperature, Remote);
                        lastProgramLog = now;
                    }
                }
                else
                {
                    // Reset last log difference
                    lastLogSecondDifference = 0;
                }
            }
            else
            {
                // Reset last log difference
                lastLogSecondDifference = 0;
            }
        }     
        
        // periodically raise gui update event
        private void updateGui()
        {
            if ((DateTime.Now - this.lastGuiUpdate).TotalSeconds >= guiUpdatePeriod)
            {
                lastGuiUpdate = DateTime.Now;
                GuiUpdateEvent?.Invoke();
            }
            DataUpdated?.Invoke();
        }

        // window with load capabilities
        public void ShowDeviceInfo()
        {
            if (this.IsConnected == true)
            {
                DeviceInfo deviceInfo = new DeviceInfo(this.device); 
                deviceInfo.ShowDialog();
            }
        }

        // set default values upon connecting
        private void SetDefault()
        {
            LEDBrightness = _LEDBrightness;
            LEDRule = _LEDRule;
            FanRule = _FanRule;
        }

        // reset all user pins to zero
        public void ResetUserPins()
        {
            if (isConnected)
            {
                device.SetValue(WriteCommands.UserPins, 0x7F);
            }
        }

        // signals that property has changed
        private void NotifyPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public MeasurementValues PresentValues
        {
            get
            {
                return device.PresentValues;
            }
        }

        public double DvmInputResistance
        {
            get
            {
                return device.DvmInputResistance;
            }
        }

        public string PortName
        {
            get
            {
                return this.device.PortName;
            }
        }

        public string DeviceIdentification
        {
            get
            {
                return device.DeviceIdentification;
            }
        }            

        public double Voltage
        {
            get
            {
                return this.device.GetValue(RunMode.Voltage);
            }
        }

        public double Current
        {
            get
            {
                return this.device.GetValue(RunMode.Current);
            }
        }

        public double Power
        {
            get
            {
                return this.device.GetValue(RunMode.Power_CC);
            }
        }

        public double Resistance
        {
            get
            {
                return this.device.GetValue(RunMode.Resistance_CC);
            }
        }

        public double Temperature
        {
            get
            {
                return this.device.Temperature;
            }
        }

        public double SeriesResistance
        {
            get
            {
                return this.device.SeriesResistance;
            }
            set
            {                
                this.device.SeriesResistance = value;
            }
        }

        public double SeriesResistancePowerLimit
        {
            get
            {
                return seriesResistancePowerLimit;
            }
            set
            {
                if (value > 0.1) // minimum 0.1 W of series resistance power handling
                {
                    seriesResistancePowerLimit = value;
                }
            }
        }

        public bool SeriesResistancePowerLimitIsEnabled { get; set; }

        public bool IsConnected
        {
            get
            {
                return this.isConnected;
            }
        }

        public bool Local
        {
            get
            {
                return !this.device.Remote;
            }
        }

        public bool Remote
        {
            get
            {
                return this.device.Remote;
            }
        }

        public bool IsManual
        {
            get
            {
                return this.isManual;
            }
        }

        public int CurrentItemNumber
        {
            get
            {
                return this.currentItemNumber;
            }
        }

        public double TotalRemainingTime
        {
            get
            {
                double time = 0;
                for (int i = currentItemNumber; i < ProgramItems.Count; i++)
                {
                    time += ProgramItems[i].Duration;
                }
                time -= elapsed(currentItemNumber);
                return time;
            }
        }

        public double ItemRemainingTime
        {
            get
            {
                if (ProgramItems.Count > 0 && currentItemNumber < ProgramItems.Count)
                {
                    return ProgramItems[currentItemNumber].Duration - elapsed(currentItemNumber);
                }
                else
                {
                    return 0;
                }
            }
        }

        public int CurrentLoop
        {
            get
            {
                return this.currentLoop;
            }
        }

        public string FilePath
        {
            get
            {
                if (file != null)
                {
                    if (file.FilePath != null)
                    {
                        return file.FilePath;
                    }
                }
                return "No file";
            }
        }

        public File LogFile
        {
            get
            {
                return file;
            }            
        }

        public bool IsLoggingManual { get; set; } = false;

        public bool IsLoggingProgram { get; set; } = false;

        public double LoggingPeriod // logging period in seconds
        {
            get
            {
                return this.loggingPeriod;
            }
            set
            {                
                if (value >= 0) // value must be non-negative
                {
                    loggingPeriod = value;
                }
                else
                {
                    loggingPeriod = 0;
                }
            }
        }

        public static string MinimumFirmwareVersion
        {
            get
            {
                return MinimumFWVersion[0].ToString() + "." + MinimumFWVersion[1].ToString() + "." + MinimumFWVersion[2].ToString();
            }
        }

        public LEDBrightnesses LEDBrightness
        {
            get
            {
                return _LEDBrightness;
            }
            set
            {
                if (isConnected)
                {
                    _LEDBrightness = value;
                    device.SetValue(WriteCommands.LEDBrightness, (byte)value);
                }
            }
        }

        public byte LEDRule
        {
            get
            {
                return _LEDRule;
            }
            set
            {
                if (isConnected)
                {
                    _LEDRule = value;
                    device.SetValue(WriteCommands.LEDRules, value);
                }
            }
        }

        public FanRules FanRule
        {
            get
            {
                return _FanRule;
            }
            set
            {
                if (isConnected)
                {
                    _FanRule = value;
                    device.SetValue(WriteCommands.FanRules, (byte)value);
                }
            }
        }

        public MeasurementFilters MeasurementFilter
        {
            get
            {
                return _MeasurementFilter;
            }
            set
            {
                if (isConnected)
                {
                    _MeasurementFilter = value;
                    device.SetValue(WriteCommands.MeasurementFilter, (byte)value);
                }
            }
        }

        public bool AutorangingCurrent
        {
            get
            {
                return _AutorangingCurrent;
            }
            set
            {
                if (isConnected)
                {
                    _AutorangingCurrent = value;
                    device.SetValue(WriteCommands.CurrentRangeAuto, Convert.ToByte(value));
                }
            }
        }

        public bool AutorangingVoltage
        {
            get
            {
                return _AutorangingVoltage;
            }
            set
            {
                if (isConnected)
                {
                    _AutorangingVoltage = value;
                    device.SetValue(WriteCommands.VoltageRangeAuto, Convert.ToByte(value));
                }
            }
        }

        public bool UserPin0
        {
            get
            {
                return (device.UserPins & (0x1 << 0)) > 0;
            }
            set
            {
                if (isConnected)
                {
                    if (value)
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte((1 << 0) | 0x80));
                    }
                    else
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte((1 << 0)));
                    }
                }                
            }
        }

        public bool UserPin1
        {
            get
            {
                return (device.UserPins & (0x1 << 1)) > 0;
            }
            set
            {
                if (isConnected)
                {
                    if (value)
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte((1 << 1) | 0x80));
                    }
                    else
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte((1 << 1)));
                    }
                }
            }
        }

        public bool UserPin2
        {
            get
            {
                return (device.UserPins & (0x1 << 2)) > 0;
            }
            set
            {
                if (isConnected)
                {
                    if (value)
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte((1 << 2) | 0x80));
                    }
                    else
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte(1 << 2));
                    }
                }
            }
        }

        public bool UserPin3
        {
            get
            {
                return (device.UserPins & (0x1 << 3)) > 0;
            }
            set
            {
                if (isConnected)
                {
                    if (value)
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte((1 << 3) | 0x80));
                    }
                    else
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte(1 << 3));
                    }
                }
            }
        }

        public bool UserPin4
        {
            get
            {
                return (device.UserPins & (0x1 << 4)) > 0;
            }
            set
            {
                if (isConnected)
                {
                    if (value)
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte((1 << 4) | 0x80));
                    }
                    else
                    {
                        device.SetValue(WriteCommands.UserPins, Convert.ToByte(1 << 4));
                    }
                }
            }
        }
    }
}
