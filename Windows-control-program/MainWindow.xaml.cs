using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Input;
using Microsoft.Win32;
using System.Threading.Tasks;
using System.Globalization;

namespace MightyWatt
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        // main components
        private Load load;
        private Xml xmlSettings;

        // GUI
        private delegate void GuiDispatcherDelegate();
        private GuiDispatcherDelegate guiDispatcherDelegate;
        private bool errorShowingEnabled = true;
        private bool watchdogMessageShowingEnabled = true, seriesResistanceWatchdogMessageShowingEnabled = true;
        private string statusBarConnectionString = "Not connected";
        private Statistics statisticsWindow;

        // bindings
        List<BindingExpression> bindingExpressions;
        private Binding logManualBinding, logProgramBinding, temperatureBinding, voltageBinding, currentBinding, powerBinding, resistanceBinding,
                        manualUnitBinding, mainButtonBinding, filePathBinding, deviceInfoBinding, settingsBinding,
                        programConstantUnitBinding, programConstantSkipUnitBinding, programRampUnitBinding, programRampUnitSkipBinding, groupBoxManualControlBinding,
                        watchdogEnabledBinding, watchdogUnitBinding, watchdogModeBinding, watchdogComparisonBinding, watchdogValueBinding,
                        localBinding, remoteBinding, localRemoteEnabledBinding, listBoxSelectedIndexBinding;

        // plot
        private const int replotTime = 1000; // ms
        private DateTime lastPlotTime = DateTime.MinValue;
        private Plot gnuPlot = null;
        private PlotData m1, m15, h4, d1, d7;
        private PlotTimeSpan plotTimeSpan;
        private const string gnuPlotDefaultPath = @"C:\Program Files (x86)\gnuplot\bin\gnuplot.exe";
        string gnuPlotPath;

        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;
            load = new Load();
            createBindings();
            fillCombos();
            statisticsWindow = null;

            // events and delegates
            load.Error += showErrors;
            load.ConnectionUpdateEvent += connectionUpdated;
            guiDispatcherDelegate += updateGui;
            load.GuiUpdateEvent += () => Dispatcher.Invoke(guiDispatcherDelegate, null); // invoking GUI update from different thread
            // gui change for program start/stop and single item finish
            load.ProgramStartedEvent += programStartChangeGui;
            load.ProgramStoppedEvent += programStopChangeGui;
            // watchdog
            load.WatchdogStop += watchdogStop;
            comboBoxWatchdogComparator.SelectedIndex = 1;
            // series resistance watchdog
            load.SeriesResistanceWatchdogStop += seriesResistanceWatchdogStop;

            // plot
            m1 = new PlotData(100, 1000);
            m15 = new PlotData(1000, 1000);
            h4 = new PlotData(10000, 2000);
            d1 = new PlotData(100000, 1000);
            d7 = new PlotData(700000, 1000);

            menuItemPlot1m.Tag = PlotTimeSpan.m1;
            menuItemPlot15m.Tag = PlotTimeSpan.m15;
            menuItemPlot4h.Tag = PlotTimeSpan.h4;
            menuItemPlot1d.Tag = PlotTimeSpan.d1;
            menuItemPlot7d.Tag = PlotTimeSpan.d7;            
        }

        // disconnects load when the program exits
        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            statisticsWindow?.Close();
            load.Disconnect();
            base.OnClosing(e);
        }

        // opens resource page with newest software, application notes etc.
        public static void OpenResourcesInBrowser()
        {
            System.Diagnostics.Process.Start("http://kaktuscircuits.blogspot.cz/2017/03/mightywatt-r3-resource-page.html");
        }

        // bindings for various GUI elements
        private void createBindings()
        {
            // list box program binding
            listBoxProgram.ItemsSource = load.ProgramItems;

            bindingExpressions = new List<BindingExpression>();

            // status bar filepath binding
            filePathBinding = new Binding();
            filePathBinding.Path = new PropertyPath("FilePath");
            filePathBinding.Source = this.load;
            filePathBinding.Mode = BindingMode.OneWay;
            this.statusBarFilePath.SetBinding(StatusBarItem.ContentProperty, this.filePathBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.statusBarFilePath, StatusBarItem.ContentProperty));

            // status bar temperature binding
            temperatureBinding = new Binding();
            temperatureBinding.Path = new PropertyPath("Temperature");
            temperatureBinding.Source = this.load;
            temperatureBinding.Mode = BindingMode.OneWay;
            temperatureBinding.Converter = new Converters.TemperatureConverter();
            temperatureBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.statusBarTemperature.SetBinding(StatusBarItem.ContentProperty, this.temperatureBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.statusBarTemperature, StatusBarItem.ContentProperty));

            // status bar connection binding expression
            bindingExpressions.Add(BindingOperations.GetBindingExpression(statusBarConnection, ContentProperty));

            // current text box binding
            currentBinding = new Binding();
            currentBinding.Path = new PropertyPath("Current");
            currentBinding.Source = this.load;
            currentBinding.Mode = BindingMode.OneWay;
            currentBinding.Converter = new Converters.SignificantDigitsConverter();
            currentBinding.ConverterParameter = 5;
            currentBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.textBoxValuesCurrent.SetBinding(TextBox.TextProperty, this.currentBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.textBoxValuesCurrent, TextBox.TextProperty));

            // voltage text box binding
            voltageBinding = new Binding();
            voltageBinding.Path = new PropertyPath("Voltage");
            voltageBinding.Source = this.load;
            voltageBinding.Mode = BindingMode.OneWay;
            voltageBinding.Converter = new Converters.SignificantDigitsConverter();
            voltageBinding.ConverterParameter = 5;
            voltageBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.textBoxValuesVoltage.SetBinding(TextBox.TextProperty, this.voltageBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.textBoxValuesVoltage, TextBox.TextProperty));

            // power text box binding
            powerBinding = new Binding();
            powerBinding.Path = new PropertyPath("Power");
            powerBinding.Source = this.load;
            powerBinding.Mode = BindingMode.OneWay;
            powerBinding.Converter = new Converters.SignificantDigitsConverter();
            powerBinding.ConverterParameter = 5;
            powerBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.textBoxValuesPower.SetBinding(TextBox.TextProperty, this.powerBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.textBoxValuesPower, TextBox.TextProperty));

            // resistance text box binding
            resistanceBinding = new Binding();
            resistanceBinding.Path = new PropertyPath("Resistance");
            resistanceBinding.Source = this.load;
            resistanceBinding.Mode = BindingMode.OneWay;
            resistanceBinding.Converter = new Converters.SignificantDigitsConverter();
            resistanceBinding.ConverterParameter = 5;
            resistanceBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.textBoxValuesResistance.SetBinding(TextBox.TextProperty, this.resistanceBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.textBoxValuesResistance, TextBox.TextProperty));

            // manual unit label binding
            manualUnitBinding = new Binding();
            manualUnitBinding.Path = new PropertyPath("SelectedIndex");
            manualUnitBinding.Source = this.comboBoxManualQuantity;
            manualUnitBinding.Mode = BindingMode.OneWay;
            manualUnitBinding.Converter = new Converters.UnitEnumConverter();
            manualUnitBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.labelManualUnit.SetBinding(Label.ContentProperty, this.manualUnitBinding);

            // manual set/stop, program start buttons binding
            mainButtonBinding = new Binding();
            mainButtonBinding.Path = new PropertyPath("IsConnected");
            mainButtonBinding.Source = this.load;
            mainButtonBinding.Mode = BindingMode.OneWay;
            this.buttonManualStop.SetBinding(Button.IsEnabledProperty, this.mainButtonBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.buttonManualStop, Button.IsEnabledProperty));
            this.buttonManualSet.SetBinding(Button.IsEnabledProperty, this.mainButtonBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.buttonManualSet, Button.IsEnabledProperty));
            this.buttonProgramStart.SetBinding(Button.IsEnabledProperty, this.mainButtonBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.buttonProgramStart, Button.IsEnabledProperty));

            // device info binding
            deviceInfoBinding = new Binding();
            deviceInfoBinding.Path = new PropertyPath("IsConnected");
            deviceInfoBinding.Source = this.load;
            deviceInfoBinding.Mode = BindingMode.OneWay;
            this.menuItemDeviceInfo.SetBinding(MenuItem.IsEnabledProperty, this.deviceInfoBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.menuItemDeviceInfo, MenuItem.IsEnabledProperty));

            // settings binding
            settingsBinding = new Binding();
            settingsBinding.Path = new PropertyPath("IsConnected");
            settingsBinding.Source = load;
            settingsBinding.Mode = BindingMode.OneWay;
            menuItemSettingsLED.SetBinding(MenuItem.IsEnabledProperty, settingsBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(menuItemSettingsLED, MenuItem.IsEnabledProperty));
            menuItemSettingsFan.SetBinding(MenuItem.IsEnabledProperty, settingsBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(menuItemSettingsFan, MenuItem.IsEnabledProperty));
            menuItemSettingsMeasurementFilter.SetBinding(MenuItem.IsEnabledProperty, settingsBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(menuItemSettingsMeasurementFilter, MenuItem.IsEnabledProperty));
            menuItemSettingsSeriesResistance.SetBinding(MenuItem.IsEnabledProperty, settingsBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(menuItemSettingsSeriesResistance, MenuItem.IsEnabledProperty));
            menuItemToolsIntegratorsAndStatistics.SetBinding(MenuItem.IsEnabledProperty, settingsBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(menuItemToolsIntegratorsAndStatistics, MenuItem.IsEnabledProperty));

            // manual log checkbox binding
            logManualBinding = new Binding();
            logManualBinding.Path = new PropertyPath("IsLoggingManual");
            logManualBinding.Source = this.load;
            logManualBinding.Mode = BindingMode.OneWayToSource;
            this.menuItemLogManual.SetBinding(MenuItem.IsCheckedProperty, this.logManualBinding);

            // program log checkbox binding
            logProgramBinding = new Binding();
            logProgramBinding.Path = new PropertyPath("IsLoggingProgram");
            logProgramBinding.Source = this.load;
            logProgramBinding.Mode = BindingMode.OneWayToSource;
            this.menuItemLogProgram.SetBinding(MenuItem.IsCheckedProperty, this.logProgramBinding);

            // program constant unit label binding
            programConstantUnitBinding = new Binding();
            programConstantUnitBinding.Path = new PropertyPath("SelectedIndex");
            programConstantUnitBinding.Source = this.comboBoxProgramConstantQuantity;
            programConstantUnitBinding.Mode = BindingMode.OneWay;
            programConstantUnitBinding.Converter = new Converters.UnitEnumConverter();
            programConstantUnitBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.labelProgramConstantUnit.SetBinding(Label.ContentProperty, this.programConstantUnitBinding);

            // program constant skip unit label binding
            programConstantSkipUnitBinding = new Binding();
            programConstantSkipUnitBinding.Path = new PropertyPath("SelectedIndex");
            programConstantSkipUnitBinding.Source = this.comboBoxProgramConstantSkipQuantity;
            programConstantSkipUnitBinding.Mode = BindingMode.OneWay;
            programConstantSkipUnitBinding.Converter = new Converters.UnitEnumConverter();
            programConstantSkipUnitBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.labelProgramConstantSkipUnit.SetBinding(Label.ContentProperty, this.programConstantSkipUnitBinding);

            // program ramp unit label binding
            programRampUnitBinding = new Binding();
            programRampUnitBinding.Path = new PropertyPath("SelectedIndex");
            programRampUnitBinding.Source = this.comboBoxProgramRampQuantity;
            programRampUnitBinding.Mode = BindingMode.OneWay;
            programRampUnitBinding.Converter = new Converters.UnitEnumConverter();
            programRampUnitBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.labelProgramRampInitialValueUnit.SetBinding(Label.ContentProperty, this.programRampUnitBinding);
            this.labelProgramRampFinalValueUnit.SetBinding(Label.ContentProperty, this.programRampUnitBinding);

            // program ramp skip unit label binding
            programRampUnitSkipBinding = new Binding();
            programRampUnitSkipBinding.Path = new PropertyPath("SelectedIndex");
            programRampUnitSkipBinding.Source = this.comboBoxProgramRampSkipQuantity;
            programRampUnitSkipBinding.Mode = BindingMode.OneWay;
            programRampUnitSkipBinding.Converter = new Converters.UnitEnumConverter();
            programRampUnitSkipBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.labelProgramRampSkipUnit.SetBinding(Label.ContentProperty, this.programRampUnitSkipBinding);

            // manual groupbox enabled binding
            groupBoxManualControlBinding = new Binding();
            groupBoxManualControlBinding.Path = new PropertyPath("isManual");
            groupBoxManualControlBinding.Source = this;
            groupBoxManualControlBinding.Mode = BindingMode.OneWay;
            this.groupBoxManualControl.SetBinding(GroupBox.IsEnabledProperty, this.groupBoxManualControlBinding);

            // watchdog enabled binding
            watchdogEnabledBinding = new Binding();
            watchdogEnabledBinding.Path = new PropertyPath("WatchdogEnabled");
            watchdogEnabledBinding.Source = this.load;
            watchdogEnabledBinding.Mode = BindingMode.TwoWay;
            this.checkBoxWatchdogEnable.SetBinding(CheckBox.IsCheckedProperty, this.watchdogEnabledBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.checkBoxWatchdogEnable, CheckBox.IsCheckedProperty));

            // watchdog unit binding
            watchdogUnitBinding = new Binding();
            watchdogUnitBinding.Path = new PropertyPath("SelectedIndex");
            watchdogUnitBinding.Source = this.comboBoxWatchdogQuantity;
            watchdogUnitBinding.Mode = BindingMode.OneWay;
            watchdogUnitBinding.Converter = new Converters.UnitEnumConverter();
            watchdogUnitBinding.ConverterCulture = System.Globalization.CultureInfo.CurrentCulture;
            this.labelWatchdogUnit.SetBinding(Label.ContentProperty, this.watchdogUnitBinding);

            // watchdog mode binding
            watchdogModeBinding = new Binding();
            watchdogModeBinding.Path = new PropertyPath("WatchdogMode");
            watchdogModeBinding.Source = this.load;
            watchdogModeBinding.Mode = BindingMode.OneWayToSource;
            this.comboBoxWatchdogQuantity.SetBinding(ComboBox.SelectedItemProperty, this.watchdogModeBinding);

            // watchdog comparison binding
            watchdogComparisonBinding = new Binding();
            watchdogComparisonBinding.Path = new PropertyPath("WatchdogCompare");
            watchdogComparisonBinding.Source = this.load;
            watchdogComparisonBinding.Mode = BindingMode.OneWayToSource;
            this.comboBoxWatchdogComparator.SetBinding(ComboBox.SelectedIndexProperty, this.watchdogComparisonBinding);

            // watchdog value binding (string)
            watchdogValueBinding = new Binding();
            watchdogValueBinding.Path = new PropertyPath("WatchdogValue");
            watchdogValueBinding.Source = this.load;
            watchdogValueBinding.Mode = BindingMode.OneWayToSource;
            this.textBoxWatchdogValue.SetBinding(TextBox.TextProperty, this.watchdogValueBinding);

            // local binding
            localBinding = new Binding();
            localBinding.Path = new PropertyPath("Local");
            localBinding.Source = this.load;
            localBinding.Mode = BindingMode.OneWay;
            this.radioButtonLocal.SetBinding(RadioButton.IsCheckedProperty, this.localBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.radioButtonLocal, RadioButton.IsCheckedProperty));

            // remote binding
            remoteBinding = new Binding();
            remoteBinding.Path = new PropertyPath("Remote");
            remoteBinding.Source = this.load;
            remoteBinding.Mode = BindingMode.OneWay;
            this.radioButtonRemote.SetBinding(RadioButton.IsCheckedProperty, this.remoteBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.radioButtonRemote, RadioButton.IsCheckedProperty));

            // local/remote enabled binding
            localRemoteEnabledBinding = new Binding();
            localRemoteEnabledBinding.Path = new PropertyPath("IsConnected");
            localRemoteEnabledBinding.Source = this.load;
            localRemoteEnabledBinding.Mode = BindingMode.OneWay;
            this.radioButtonLocal.SetBinding(RadioButton.IsEnabledProperty, this.localRemoteEnabledBinding);
            this.radioButtonRemote.SetBinding(RadioButton.IsEnabledProperty, this.localRemoteEnabledBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.radioButtonLocal, RadioButton.IsEnabledProperty));
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.radioButtonRemote, RadioButton.IsEnabledProperty));

            // disconnect menu item
            bindingExpressions.Add(BindingOperations.GetBindingExpression(menuItemDisconnect, MenuItem.IsEnabledProperty));
        }

        // fill combos with enum names (voltage, current etc.)
        private void fillCombos()
        {
            comboBoxManualQuantity.Items.Clear();
            comboBoxProgramConstantQuantity.Items.Clear();
            comboBoxProgramRampQuantity.Items.Clear();
            comboBoxWatchdogQuantity.Items.Clear();
            comboBoxProgramConstantSkipQuantity.Items.Clear();
            comboBoxProgramRampSkipQuantity.Items.Clear();
            for (int i = 0; i < Enum.GetValues(typeof(Modes)).Length; i++) // add the modes
            {
                comboBoxManualQuantity.Items.Add(Load.ModeNames[i]);
                comboBoxProgramConstantQuantity.Items.Add(Load.ModeNames[i]);
                if (i != (int)Modes.MPPT) // do not add MPPT to ramp
                {
                    comboBoxProgramRampQuantity.Items.Add(Load.ModeNames[i]);
                }
                if (i != (int)Modes.VoltageSoftware && i != (int)Modes.MPPT) // do not add the voltage phase inversion and MPPT to watchdog and skip, use regular voltage or power
                {
                    comboBoxWatchdogQuantity.Items.Add(Load.ModeNames[i]);
                    comboBoxProgramConstantSkipQuantity.Items.Add(Load.ModeNames[i]);
                    comboBoxProgramRampSkipQuantity.Items.Add(Load.ModeNames[i]);
                }
            }

            comboBoxManualQuantity.SelectedIndex = 0;
            comboBoxProgramConstantQuantity.SelectedIndex = 0;
            comboBoxProgramRampQuantity.SelectedIndex = 0;
            comboBoxWatchdogQuantity.SelectedIndex = 0;
            comboBoxProgramConstantSkipQuantity.SelectedIndex = 0;
            comboBoxProgramRampSkipQuantity.SelectedIndex = 0;

            // fill combos with enum names (time units)
            comboBoxProgramConstantUnit.Items.Clear();
            comboBoxProgramRampDurationUnit.Items.Clear();
            for (byte i = 0; i < (Enum.GetValues(typeof(TimeUnits))).Length; i++)
            {
                comboBoxProgramConstantUnit.Items.Add((TimeUnits)i);
                comboBoxProgramRampDurationUnit.Items.Add((TimeUnits)i);
            }
            comboBoxProgramConstantUnit.SelectedIndex = 1;
            comboBoxProgramRampDurationUnit.SelectedIndex = 1;
        }

        // updates loop settings according to user input
        private void loops()
        {
            if (this.load != null)
            {
                if (radioButtonProgramLoopFinite.IsChecked == true)
                {
                    int numberOfLoops;
                    if (int.TryParse(textBoxProgramLoopCount.Text, out numberOfLoops))
                    {
                        if (numberOfLoops > 0)
                        {
                            this.load.TotalLoops = numberOfLoops; // new value of number of loops
                        }
                        else
                        {
                            textBoxProgramLoopCount.Text = this.load.TotalLoops.ToString(); // replace faulty text with old (valid) value
                        }
                    }
                    else
                    {
                        textBoxProgramLoopCount.Text = this.load.TotalLoops.ToString(); // replace faulty text with old (valid) value
                    }
                }
                else if (radioButtonProgramLoopInfinite.IsChecked == true) // infinite loop
                {
                    this.load.TotalLoops = 0;
                }
            }
        }

        // finds the currently connected port and matches it with its text description from WMI query; then updates the status bar with connection information
        private void connectionUpdated()
        {
            if (load.IsConnected == false)
            {
                // status bar
                statusBarConnectionString = "Not connected";
            }
            else
            {
                foreach (COMPortInfo comPortInfo in COMPortInfo.GetCOMPortsInfo())
                {
                    if (comPortInfo.Name == load.PortName)
                    {
                        statusBarConnectionString = "MightyWatt R3 on " + comPortInfo.Description;
                        break;
                    }
                }

                menuItemPlotClear_Click(null, null);
            }
        }

        // changes GUI at program start event
        private void programStartChangeGui()
        {
            // listbox selected index binding
            listBoxSelectedIndexBinding = new Binding();
            listBoxSelectedIndexBinding.Path = new PropertyPath("CurrentItemNumber");
            listBoxSelectedIndexBinding.Source = this.load;
            listBoxSelectedIndexBinding.Mode = BindingMode.OneWay;
            this.listBoxProgram.SetBinding(ListBox.SelectedIndexProperty, this.listBoxSelectedIndexBinding);
            bindingExpressions.Add(BindingOperations.GetBindingExpression(this.listBoxProgram, ListBox.SelectedIndexProperty));

            this.buttonProgramStart.Content = "Stop";
            this.buttonProgramAdd.Content = "Skip";
            this.buttonProgramDown.IsEnabled = false;
            this.buttonProgramRemove.IsEnabled = false;
            this.buttonProgramUp.IsEnabled = false;
            this.tabControlProgram.IsEnabled = false;
            this.menuItemExperimentAdd.IsEnabled = false;
            this.menuItemExperimentReplace.IsEnabled = false;
            this.labelElapsed.Visibility = Visibility.Visible;
        }

        // changes GUI at program stop event
        private void programStopChangeGui()
        {
            // listbox remove selected index binding
            bindingExpressions.Remove(BindingOperations.GetBindingExpression(this.listBoxProgram, ListBox.SelectedIndexProperty));
            BindingOperations.ClearBinding(listBoxProgram, ListBox.SelectedIndexProperty);

            this.buttonProgramStart.Content = "Start";
            this.buttonProgramAdd.Content = "Add";
            this.buttonProgramDown.IsEnabled = true;
            this.buttonProgramRemove.IsEnabled = true;
            this.buttonProgramUp.IsEnabled = true;
            this.tabControlProgram.IsEnabled = true;
            this.menuItemExperimentAdd.IsEnabled = true;
            this.menuItemExperimentReplace.IsEnabled = true;
            this.labelElapsed.Visibility = Visibility.Hidden;
        }

        // in case of watchdog action, shows error message and prompts user to disable watchdog
        private async void watchdogStop()
        {
            load.WatchdogEnabled = false;
            if (watchdogMessageShowingEnabled)
            {
                watchdogMessageShowingEnabled = false;
                await Task.Run(() =>
                {
                    MessageBoxResult result = MessageBox.Show("Watchdog stopped the load.\nDo you want to keep the watchdog on?", "Watchdog", MessageBoxButton.YesNo, MessageBoxImage.Information);
                    if (result == MessageBoxResult.Yes)
                    {
                        load.WatchdogEnabled = true;
                    }
                }
                );
                watchdogMessageShowingEnabled = true;
            }
        }

        // in case of series resistance watchdog action, shows error message
        private async void seriesResistanceWatchdogStop()
        {
            if (seriesResistanceWatchdogMessageShowingEnabled)
            {
                seriesResistanceWatchdogMessageShowingEnabled = false;
                await Task.Run(() =>
                {
                    MessageBox.Show("Series resistance power limit has been exceeded.\nLoad has been stopped.", "Series resistance power limit exceeded", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                });
                seriesResistanceWatchdogMessageShowingEnabled = true;
            }
        }

        // in case of load reported error shows message
        private async void showErrors(string errorList) // shows error list in case there are some
        {
            if (errorShowingEnabled)
            {
                errorShowingEnabled = false;
                await showErrorMessage(errorList);
                errorShowingEnabled = true;
            }
        }

        // show error message
        private Task showErrorMessage(string message)
        {
            return Task.Run(() => MessageBox.Show(message + "\nThe load has been stopped", "Error", MessageBoxButton.OK, MessageBoxImage.Error));
        }

        // updates all bound GUI elements
        private void updateGui()
        {
            if (bindingExpressions != null)
            {
                foreach (BindingExpression be in bindingExpressions)
                {
                    be.UpdateTarget();
                }
            }

            if (this.load != null)
            {
                if (this.load.IsManual == false) // remaining time and loops label
                {
                    TimeSpan itemRemainingTimeSpan = TimeSpan.FromSeconds(this.load.ItemRemainingTime);
                    TimeSpan totalRemainingTimeSpan = TimeSpan.FromSeconds(this.load.TotalRemainingTime);
                    string loopInfo = "";
                    if (this.load.TotalLoops != 1)
                    {
                        if (this.load.TotalLoops == 0)
                        {
                            loopInfo = "Loop " + (this.load.CurrentLoop + 1).ToString() + " (infinite).";
                        }
                        else
                        {
                            loopInfo = "Loop " + (this.load.CurrentLoop + 1).ToString() + " of " + this.load.TotalLoops.ToString() + ".";
                        }
                    }
                    labelElapsed.Content = string.Format("Item: {0}h:{1:D2}m:{2:D2}s, total: {3}h:{4:D2}m:{5:D2}s. {6}", Math.Floor(itemRemainingTimeSpan.TotalHours), itemRemainingTimeSpan.Minutes, itemRemainingTimeSpan.Seconds, Math.Floor(totalRemainingTimeSpan.TotalHours), totalRemainingTimeSpan.Minutes, totalRemainingTimeSpan.Seconds, loopInfo);
                }
            }
        }

        private void AddPlotData()
        {
            m1.Add(load.Voltage, load.Current, load.Power, load.Resistance);
            m15.Add(load.Voltage, load.Current, load.Power, load.Resistance);
            h4.Add(load.Voltage, load.Current, load.Power, load.Resistance);
            d1.Add(load.Voltage, load.Current, load.Power, load.Resistance);
            d7.Add(load.Voltage, load.Current, load.Power, load.Resistance);
        }

        private void UpdatePlot()
        {
            if (gnuPlot != null)
            {
                DateTime now = DateTime.Now;
                if ((now - lastPlotTime).TotalMilliseconds > replotTime)
                {
                    lastPlotTime = now;

                    switch (plotTimeSpan)
                    {
                        case PlotTimeSpan.m1:
                            gnuPlot.PlotData(PlotTimeUnit.s, m1, new TimeSpan(0, 1, 0));
                            break;
                        case PlotTimeSpan.m15:
                            gnuPlot.PlotData(PlotTimeUnit.min, m15, new TimeSpan(0, 15, 0));
                            break;
                        case PlotTimeSpan.h4:
                            gnuPlot.PlotData(PlotTimeUnit.h, h4, new TimeSpan(4, 0, 0));
                            break;
                        case PlotTimeSpan.d1:
                            gnuPlot.PlotData(PlotTimeUnit.h, d1, new TimeSpan(24, 0, 0));
                            break;
                        case PlotTimeSpan.d7:
                            gnuPlot.PlotData(PlotTimeUnit.d, d7, new TimeSpan(168, 0, 0));
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        public bool LEDBrightnessHigh
        {
            get
            {
                if (load != null)
                {
                    return (load.LEDBrightness == LEDBrightnesses.High);
                }
                else
                {
                    return false;
                }
            }
            set
            {
                if (value)
                {
                    load.LEDBrightness = LEDBrightnesses.High;
                }
                else
                {
                    load.LEDBrightness = LEDBrightnesses.Off;
                }
            }
        }

        public bool LEDBrightnessMedium
        {
            get
            {
                if (load != null)
                {
                    return (load.LEDBrightness == LEDBrightnesses.Medium);
                }
                else
                {
                    return false;
                }
            }
            set
            {
                if (value)
                {
                    load.LEDBrightness = LEDBrightnesses.Medium;
                }
                else
                {
                    load.LEDBrightness = LEDBrightnesses.Off;
                }
            }
        }

        public bool LEDBrightnessLow
        {
            get
            {
                if (load != null)
                {
                    return (load.LEDBrightness == LEDBrightnesses.Low);
                }
                else
                {
                    return false;
                }
            }
            set
            {
                if (value)
                {
                    load.LEDBrightness = LEDBrightnesses.Low;
                }
                else
                {
                    load.LEDBrightness = LEDBrightnesses.Off;
                }
            }
        }

        public bool LEDBrightnessOff
        {
            get
            {
                if (load != null)
                {
                    return (load.LEDBrightness == LEDBrightnesses.Off);
                }
                else
                {
                    return false;
                }
            }
            set
            {
                if (value)
                {
                    load.LEDBrightness = LEDBrightnesses.Off;
                }
            }
        }

        public bool LEDRulesI1
        {
            get
            {
                return load == null ? false : ((load.LEDRule & (byte)LEDRules.I1) > 0);
            }
            set
            {
                if (value)
                {
                    load.LEDRule |= (byte)LEDRules.I1;
                }
                else
                {
                    load.LEDRule &= Convert.ToByte((~((uint)LEDRules.I1)) & 0xFF);
                }
            }
        }

        public bool LEDRulesP1
        {
            get
            {
                return load == null ? false : ((load.LEDRule & (byte)LEDRules.P1) > 0);
            }
            set
            {
                if (value)
                {
                    load.LEDRule |= (byte)LEDRules.P1;
                }
                else
                {
                    load.LEDRule &= Convert.ToByte((~((uint)LEDRules.P1)) & 0xFF);
                }
            }
        }

        public bool LEDRulesV1
        {
            get
            {
                return load == null ? false : ((load.LEDRule & (byte)LEDRules.V1) > 0);
            }
            set
            {
                if (value)
                {
                    load.LEDRule |= (byte)LEDRules.V1;
                }
                else
                {
                    load.LEDRule &= Convert.ToByte((~((uint)LEDRules.V1)) & 0xFF);
                }
            }
        }

        public bool LEDRulesI10
        {
            get
            {
                return load == null ? false : ((load.LEDRule & (byte)LEDRules.I10) > 0);
            }
            set
            {
                if (value)
                {
                    load.LEDRule |= (byte)LEDRules.I10;
                }
                else
                {
                    load.LEDRule &= Convert.ToByte((~((uint)LEDRules.I10)) & 0xFF);
                }
            }
        }

        public bool LEDRulesP10
        {
            get
            {
                return load == null ? false : ((load.LEDRule & (byte)LEDRules.P10) > 0);
            }
            set
            {
                if (value)
                {
                    load.LEDRule |= (byte)LEDRules.P10;
                }
                else
                {
                    load.LEDRule &= Convert.ToByte((~((uint)LEDRules.P10)) & 0xFF);
                }
            }
        }

        public bool LEDRulesV10
        {
            get
            {
                return load == null ? false : ((load.LEDRule & (byte)LEDRules.V10) > 0);
            }
            set
            {
                if (value)
                {
                    load.LEDRule |= (byte)LEDRules.V10;
                }
                else
                {
                    load.LEDRule &= Convert.ToByte((~((uint)LEDRules.V10)) & 0xFF);
                }
            }
        }

        public bool LEDRulesT50
        {
            get
            {
                return load == null ? false : ((load.LEDRule & (byte)LEDRules.T50) > 0);
            }
            set
            {
                if (value)
                {
                    load.LEDRule |= (byte)LEDRules.T50;
                }
                else
                {
                    load.LEDRule &= Convert.ToByte((~((uint)LEDRules.T50)) & 0xFF);
                }
            }
        }

        public bool LEDRulesAlwaysOn
        {
            get
            {
                return load == null ? false : ((load.LEDRule & (byte)LEDRules.AlwaysOn) > 0);
            }
            set
            {
                if (value)
                {
                    load.LEDRule |= (byte)LEDRules.AlwaysOn;
                }
                else
                {
                    load.LEDRule &= Convert.ToByte((~((uint)LEDRules.AlwaysOn)) & 0xFF);
                }
            }
        }

        public bool LEDRulesAlwaysOff
        {
            get
            {
                return load == null ? false : load.LEDRule == (byte)LEDRules.AlwaysOff;
            }
            set
            {
                if (value)
                {
                    load.LEDRule = (byte)LEDRules.AlwaysOff;
                }
            }
        }

        public bool FanRulesAlwaysOn
        {
            get
            {
                return load == null ? false : load.FanRule == FanRules.AlwaysOn;
            }
            set
            {
                if (value)
                {
                    load.FanRule = FanRules.AlwaysOn;
                }
            }
        }

        public bool FanRulesAutoCool
        {
            get
            {
                return load == null ? false : load.FanRule == FanRules.AutoCool;
            }
            set
            {
                if (value)
                {
                    load.FanRule = FanRules.AutoCool;
                }
            }
        }

        public bool FanRulesAutoQuiet
        {
            get
            {
                return load == null ? false : load.FanRule == FanRules.AutoQuiet;
            }
            set
            {
                if (value)
                {
                    load.FanRule = FanRules.AutoQuiet;
                }
            }
        }

        public bool MeasurementFilterUnfilteredNoADCAutoranging
        {
            get
            {
                return load == null ? false : load.MeasurementFilter == MeasurementFilters.UnfilteredNoADCAutoranging;
            }
            set
            {
                if (value)
                {
                    load.MeasurementFilter = MeasurementFilters.UnfilteredNoADCAutoranging;
                }
            }            
        }

        public bool MeasurementFilterUnfiltered
        {
            get
            {
                return load == null ? false : load.MeasurementFilter == MeasurementFilters.Unfiltered;
            }
            set
            {
                if (value)
                {
                    load.MeasurementFilter = MeasurementFilters.Unfiltered;
                }
            }
        }

        public bool MeasurementFilterFiltered
        {
            get
            {
                return load == null ? false : load.MeasurementFilter == MeasurementFilters.Filtered;
            }
            set
            {
                if (value)
                {
                    load.MeasurementFilter = MeasurementFilters.Filtered;
                }
            }
        }

        public string StatusBarConnectionString
        {
            get
            {
                return statusBarConnectionString;
            }
        }

        public bool IsConnected
        {
            get
            {
                return load.IsConnected;
            }
        }

        // GUI element interaction:

        // shows all available COM ports; invokes WMI
        private void menuItemConnection_MouseEnter(object sender, MouseEventArgs e)
        {
            menuItemConnectionUno.Items.Clear(); // clears the menu
            menuItemConnectionZero.Items.Clear(); // clears the menu
            MenuItem[] item;
            foreach (COMPortInfo comPort in COMPortInfo.GetCOMPortsInfo())
            {
                item = new MenuItem[2];
                item[0] = new MenuItem();
                item[1] = new MenuItem();
                item[0].Header = comPort.Description;
                item[1].Header = comPort.Description;
                item[0].Tag = Boards.Zero;
                item[1].Tag = Boards.Uno;
                item[0].IsCheckable = true;
                item[1].IsCheckable = true;
                if (comPort.Name == load.PortName)
                {                    
                    item[0].IsChecked = true;
                    item[1].IsChecked = true;
                }
                else
                {
                    item[0].IsChecked = false;
                    item[1].IsChecked = false;
                }
                menuItemConnectionZero.Items.Add(item[0]);
                menuItemConnectionUno.Items.Add(item[1]);                
                item[0].AddHandler(MenuItem.ClickEvent, new RoutedEventHandler(menuItemConnection_Click));
                item[1].AddHandler(MenuItem.ClickEvent, new RoutedEventHandler(menuItemConnection_Click));
            }
        }

        // connect to or disconnect from selected COM port, without invoking WMI
        private void menuItemConnection_Click(object sender, RoutedEventArgs e)
        {
            string selectedPort = string.Empty;
            selectedPort = ((MenuItem)sender).Header.ToString().Substring(((MenuItem)sender).Header.ToString().LastIndexOf("(COM")).Replace("(", string.Empty).Replace(")", string.Empty).Trim();
            Boards selectedBoard = (Boards)(((MenuItem)sender).Tag);

            //foreach (MenuItem menuItem in menuItemConnection.Items)
            //{
            //    if (menuItem.Equals(sender))
            //    {
            //        selectedPort = menuItem.Header.ToString().Substring(menuItem.Header.ToString().LastIndexOf("(COM")).Replace("(", string.Empty).Replace(")", string.Empty).Trim();
            //        break;
            //    }
            //}

            if (selectedPort == load.PortName)
            {
                load.Disconnect();
            }
            else
            {
                load.Connect(selectedPort, selectedBoard);
            }
        }

        // stop load - set load to zero
        private void buttonManualStop_Click(object sender, RoutedEventArgs e)
        {
            this.load.Stop();
        }

        // manually set to load
        private void buttonManualSet_Click(object sender, RoutedEventArgs e)
        {
            if (this.load.IsManual)
            {
                try
                {
                    this.load.Set((Modes)(comboBoxManualQuantity.SelectedIndex), Double.Parse(textBoxManualValue.Text));
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
            else
            {
                MessageBox.Show("Stop the programmatic control before doing manual setting", "Cannot set to load", MessageBoxButton.OK, MessageBoxImage.Asterisk);
            }
        }

        // execute program
        private void buttonProgramStart_Click(object sender, RoutedEventArgs e)
        {
            if (this.load.IsManual) // start programmatic control
            {
                this.load.Start();
            }
            else
            {
                this.load.Stop();
            }
        }

        // add new program item (or skip currently executed item)
        private void buttonProgramAdd_Click(object sender, RoutedEventArgs e)
        {
            if (this.load.IsManual) // adds new program item (Add button)
            {
                ProgramItem programItem;
                try
                {
                    double? initialValue = null;
                    if (tabControlProgram.SelectedItem == tabItemProgramConstant) // adds constant program item
                    {
                        if (checkBoxProgramConstantPrevious.IsChecked == false) // use previously measured value as the starting point if true, parse text box if false
                        {
                            initialValue = Double.Parse(textBoxProgramConstantValue.Text);
                        }

                        if (checkBoxProgramConstantSkip.IsChecked == true) // skip conditions apply
                        {
                            programItem = new ProgramItem((Modes)(comboBoxProgramConstantQuantity.SelectedIndex), initialValue, textBoxProgramConstantDuration.Text, (TimeUnits)(comboBoxProgramConstantUnit.SelectedIndex), (Modes)(comboBoxProgramConstantSkipQuantity.SelectedIndex), (Comparison)(comboBoxProgramConstantSkipComparator.SelectedIndex), Double.Parse(textBoxProgramConstantSkipValue.Text));
                        }
                        else
                        {
                            programItem = new ProgramItem((Modes)(comboBoxProgramConstantQuantity.SelectedIndex), initialValue, textBoxProgramConstantDuration.Text, (TimeUnits)(comboBoxProgramConstantUnit.SelectedIndex));
                        }
                    }
                    else // adds ramp program item
                    {
                        if (checkBoxProgramRampPrevious.IsChecked == false) // use previously measured value as the starting point if true, parse text box if false
                        {
                            initialValue = Double.Parse(textBoxProgramRampInitialValue.Text);
                        }

                        if (checkBoxProgramRampSkip.IsChecked == true) // skip conditions apply
                        {
                            programItem = new ProgramItem((Modes)(comboBoxProgramRampQuantity.SelectedIndex), initialValue, Double.Parse(textBoxProgramRampFinalValue.Text), textBoxProgramRampDuration.Text, (TimeUnits)(comboBoxProgramRampDurationUnit.SelectedIndex), (Modes)(comboBoxProgramRampSkipQuantity.SelectedIndex), (Comparison)(comboBoxProgramRampSkipComparator.SelectedIndex), Double.Parse(textBoxProgramRampSkipValue.Text));
                        }
                        else
                        {
                            programItem = new ProgramItem((Modes)(comboBoxProgramRampQuantity.SelectedIndex), initialValue, Double.Parse(textBoxProgramRampFinalValue.Text), textBoxProgramRampDuration.Text, (TimeUnits)(comboBoxProgramRampDurationUnit.SelectedIndex));
                        }
                    }
                    load.ProgramItems.Add(programItem); // add to program item list                         
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
            else  // skips current program item (Skip button)
            {
                this.load.Skip();
            }
        }

        // remove program item
        private void buttonProgramRemove_Click(object sender, RoutedEventArgs e) // n
        {
            int index = listBoxProgram.SelectedIndex;
            if (index >= 0)
            {
                load.ProgramItems.RemoveAt(index);
                if (load.ProgramItems.Count > index)
                {
                    listBoxProgram.SelectedIndex = index; // select item at the same position
                }
                else if (load.ProgramItems.Count > 0)
                {
                    listBoxProgram.SelectedIndex = load.ProgramItems.Count - 1; // select last item
                }
            }
        }

        // update submenu bindings
        private void menuItem_SubmenuOpened(object sender, RoutedEventArgs e)
        {
            foreach (MenuItem item in ((MenuItem)sender).Items)
            {
                item.GetBindingExpression(MenuItem.IsCheckedProperty).UpdateTarget();
            }
        }

        // disconnect load
        private void menuItemDisconnect_Click(object sender, RoutedEventArgs e)
        {
            load.Disconnect();
        }

        // show integrators and statistics window
        private void menuItemToolsIntegratorsAndStatistics_Click(object sender, RoutedEventArgs e)
        {
            if (statisticsWindow == null) // only open new window if the old one is closed
            {
                statisticsWindow = new Statistics(load.PresentValues, load.DvmInputResistance, load.LogFile);
                load.DataUpdated += statisticsWindow.Update;
                statisticsWindow.Closing += StatisticsWindow_Closing;
                statisticsWindow.Closed += StatisticsWindow_Closed;
                statisticsWindow.Show();
            }            
        }

        private void StatisticsWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            load.DataUpdated -= statisticsWindow.Update;
        }

        // marks the window null
        private void StatisticsWindow_Closed(object sender, EventArgs e)
        {
            statisticsWindow = null;            
        }

        // manage plots
        private void menuItemPlot_Click(object sender, RoutedEventArgs e)
        {            
            MenuItem sourceItem = sender as MenuItem;
            bool check = sourceItem.IsChecked;
            foreach (object item in (sourceItem.Parent as MenuItem).Items)
            {
                if (item is MenuItem)
                {
                    if ((item as MenuItem).IsCheckable)
                    {
                        (item as MenuItem).IsChecked = false;
                    }
                }
            }

            // open plot or change time base
            if (check)
            {                      
                if (gnuPlot == null)
                {
                    if (!string.IsNullOrEmpty(gnuPlotPath))
                    {
                        // path already known
                    }
                    else if (System.IO.File.Exists(gnuPlotDefaultPath))
                    {
                        // gnuplot is in default location
                        gnuPlotPath = gnuPlotDefaultPath;
                    }
                    else if (System.IO.File.Exists("gnuplotpath.txt"))
                    {
                        // gnuplot folder is referenced in gnuplotpath file
                        string path;
                        using (System.IO.StreamReader pathFile = new System.IO.StreamReader("gnuplotpath.txt"))
                        {
                            path = pathFile.ReadLine();
                            if (path.EndsWith("gnuplot.exe"))
                            {

                            }
                            else if (path.EndsWith("\\"))
                            {
                                path += "gnuplot.exe";
                            }
                            else
                            {
                                path += "\\gnuplot.exe";
                            }
                        }

                        if (System.IO.File.Exists(path))
                        {
                            // gnuplot found in referenced path
                            gnuPlotPath = path;
                        }
                    }

                    if (!System.IO.File.Exists(gnuPlotPath))
                    {
                        MessageBoxResult msgResult = MessageBox.Show("Make sure gnuplot is installed (http://www.gnuplot.info).\nDo you want to locate the file manually?", "Gnuplot not found", MessageBoxButton.YesNo, MessageBoxImage.Error);
                        if (msgResult == MessageBoxResult.Yes)
                        {
                            // open file dialog and locate the file manually
                            OpenFileDialog ofd = new OpenFileDialog();
                            ofd.AddExtension = true;
                            ofd.CheckFileExists = true;
                            ofd.CheckPathExists = true;
                            ofd.DefaultExt = "exe";
                            ofd.Filter = "gnuplot.exe|gnuplot.exe";
                            ofd.Multiselect = false;
                            ofd.Title = "Select gnuplot.exe";

                            if (ofd.ShowDialog() == true)
                            {
                                gnuPlotPath = ofd.FileName;
                            }
                        }
                    }

                    if (System.IO.File.Exists(gnuPlotPath))
                    {      
                        if (gnuPlotPath != gnuPlotDefaultPath)
                        {
                            // save file path if not in the default location

                            using (System.IO.FileStream stream = System.IO.File.Open("gnuplotpath.txt", System.IO.FileMode.Create))
                            {
                                byte[] path = System.Text.Encoding.ASCII.GetBytes(gnuPlotPath.ToCharArray());
                                stream.Write(path, 0, path.Length);
                                stream.Flush();
                                stream.Close();
                            }

                        }
                    }
                    else
                    {
                        // file not found after all attempts - do nothing
                        sourceItem.IsChecked = false;
                        return;
                    }

                    gnuPlot = new Plot(gnuPlotPath);
                    load.DataUpdated += AddPlotData;
                    load.DataUpdated += UpdatePlot;
                }

                sourceItem.IsChecked = true;
                plotTimeSpan = (PlotTimeSpan)sourceItem.Tag;
                UpdatePlot();
            }
            else
            {
                if (gnuPlot != null)
                {
                    load.DataUpdated -= UpdatePlot;
                    load.DataUpdated -= AddPlotData;
                    gnuPlot.Dispose();
                    gnuPlot = null;
                }
            }
        }

        // clear data
        private void menuItemPlotClear_Click(object sender, RoutedEventArgs e)
        {
            // clear all plot data
            m1.Clear();
            m15.Clear();
            h4.Clear();
            d1.Clear();
            d7.Clear();
        }

        // close plot
        private void menuItemPlotClose_Click(object sender, RoutedEventArgs e)
        {
            if (gnuPlot != null)
            {
                load.DataUpdated -= UpdatePlot;
                load.DataUpdated -= AddPlotData;
                gnuPlot.Dispose();
                gnuPlot = null;
            }

            MenuItem sourceItem = sender as MenuItem;
            foreach (object item in (sourceItem.Parent as MenuItem).Items)
            {
                if (item is MenuItem)
                {
                    if ((item as MenuItem).IsCheckable)
                    {
                        (item as MenuItem).IsChecked = false;
                    }
                }
            }
        }

        // remove all program items
        private void listBoxProgramContextMenuRemoveAll_Click(object sender, RoutedEventArgs e)
        {
            load.ProgramItems.Clear();
        }

        // move selected program item one position up
        private void buttonProgramUp_Click(object sender, RoutedEventArgs e) // n
        {
            int index = listBoxProgram.SelectedIndex;
            if (index >= 1)
            {
                ProgramItem temp = load.ProgramItems[index];
                load.ProgramItems.RemoveAt(index);
                load.ProgramItems.Insert(index - 1, temp);
                listBoxProgram.SelectedIndex = index - 1;
            }
        }

        // move selected program item one position down
        private void buttonProgramDown_Click(object sender, RoutedEventArgs e) // n
        {
            int index = listBoxProgram.SelectedIndex;
            if ((index >= 0) && (index < load.ProgramItems.Count - 1))
            {
                ProgramItem temp = load.ProgramItems[index];
                load.ProgramItems.RemoveAt(index);
                load.ProgramItems.Insert(index + 1, temp);
                listBoxProgram.SelectedIndex = index + 1;
            }
        }

        // initial values for program items
        private void checkBoxProgramConstantPrevious_Checked(object sender, RoutedEventArgs e)
        {
            textBoxProgramConstantValue.IsEnabled = false;
        }

        // initial values for program items
        private void checkBoxProgramConstantPrevious_Unchecked(object sender, RoutedEventArgs e)
        {
            textBoxProgramConstantValue.IsEnabled = true;
        }

        // initial values for program items
        private void checkBoxProgramRampPrevious_Checked(object sender, RoutedEventArgs e)
        {
            textBoxProgramRampInitialValue.IsEnabled = false;
        }

        // initial values for program items
        private void checkBoxProgramRampPrevious_Unchecked(object sender, RoutedEventArgs e)
        {
            textBoxProgramRampInitialValue.IsEnabled = true;
        }

        // program loop control
        private void checkBoxProgramLoop_Checked(object sender, RoutedEventArgs e)
        {
            this.textBoxProgramLoopCount.IsEnabled = true;
            this.radioButtonProgramLoopFinite.IsEnabled = true;
            this.radioButtonProgramLoopInfinite.IsEnabled = true;
            loops(); // update number of loops
        }

        // program loop control
        private void checkBoxProgramLoop_Unchecked(object sender, RoutedEventArgs e)
        {
            this.textBoxProgramLoopCount.IsEnabled = false;
            this.radioButtonProgramLoopFinite.IsEnabled = false;
            this.radioButtonProgramLoopInfinite.IsEnabled = false;
            this.load.TotalLoops = 1; // single loop
        }

        // program loop control
        private void radioButtonProgramLoopInfinite_Checked(object sender, RoutedEventArgs e)
        {
            loops();
        }

        // program loop control
        private void radioButtonProgramLoopFinite_Checked(object sender, RoutedEventArgs e)
        {
            loops();
        }

        // program loop control
        private void textBoxProgramLoopCount_LostFocus(object sender, RoutedEventArgs e)
        {
            loops();
        }

        // program loop control
        private void textBoxProgramLoopCount_TextChanged(object sender, TextChangedEventArgs e)
        {
            loops();
        }

        // sets sensing to 2W (local)
        private void radioButtonLocal_Click(object sender, RoutedEventArgs e)
        {
            load.SetRemote(false);
        }

        // sets sensing to 4W (remote)
        private void radioButtonRemote_Click(object sender, RoutedEventArgs e)
        {
            load.SetRemote(true);
        }

        // show logging settings
        private void menuItemLoggingSettings_Click(object sender, RoutedEventArgs e)
        {
            LogSettings window = new LogSettings(this.load);
            window.ShowDialog();
        }

        // create new file for logging
        private void menuItemLoggingNewFile_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.AddExtension = true;
            dialog.DefaultExt = ".txt";
            dialog.Filter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
            dialog.Title = "Create new log";
            dialog.ValidateNames = true;
            dialog.CreatePrompt = false;
            dialog.OverwritePrompt = false;
            if (dialog.ShowDialog() == true)
            {
                load.NewFile(dialog.FileName);
                statisticsWindow?.SetFile(load.LogFile);
                updateGui();
            }
        }

        // close file for logging
        private void menuItemLoggingCloseFile_Click(object sender, RoutedEventArgs e)
        {
            load.CloseFile();
            statisticsWindow?.SetFile(null);
            updateGui();
        }

        // saves current program items and loop settings to XML file
        private void menuItemExperimentSave_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.AddExtension = true;
            dialog.DefaultExt = ".xml";
            dialog.Filter = "XML files (*.xml)|*.xml|All Files (*.*)|*.*";
            dialog.Title = "Save program items";
            dialog.ValidateNames = true;
            dialog.CreatePrompt = false;
            dialog.OverwritePrompt = true;
            if (dialog.ShowDialog() == true)
            {
                xmlSettings = new Xml(dialog.FileName);
                bool isLoopEnabled = false;
                if (this.checkBoxProgramLoop.IsChecked != null)
                {
                    isLoopEnabled = (bool)this.checkBoxProgramLoop.IsChecked;
                }
                xmlSettings.SaveItems(this.load.ProgramItems, isLoopEnabled, this.load.TotalLoops, this.load.Remote,
                    this.load.LoggingPeriod, this.load.LoggingTimeUnit, load.LEDBrightness, load.LEDRule, load.FanRule, load.MeasurementFilter);
            }
        }

        // adds program items from XML file to the end of the currentlist
        private void menuItemExperimentAdd_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.AddExtension = true;
            dialog.DefaultExt = ".xml";
            dialog.Filter = "XML files (*.xml)|*.xml|All Files (*.*)|*.*";
            dialog.Title = "Add program items";
            dialog.ValidateNames = true;
            if (dialog.ShowDialog() == true)
            {
                xmlSettings = new Xml(dialog.FileName);
                try
                {
                    xmlSettings.AddItems(this.load.ProgramItems);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Cannot read XML file", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                }
            }
        }

        // replaces the current list of program items with content of XML file, also overwrites loop settings
        private void menuItemExperimentReplace_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.AddExtension = true;
            dialog.DefaultExt = ".xml";
            dialog.Filter = "XML files (*.xml)|*.xml|All Files (*.*)|*.*";
            dialog.Title = "Replace program items";
            dialog.ValidateNames = true;
            if (dialog.ShowDialog() == true)
            {
                xmlSettings = new Xml(dialog.FileName);
                bool enableLoop;
                int totalLoops;
                bool isRemote;
                double periodSeconds;
                TimeUnits periodTimeUnits;
                LEDBrightnesses LEDBrightness;
                byte LEDRule;
                FanRules FanRule;
                MeasurementFilters MeasurementFilter;
                try
                {
                    xmlSettings.ReplaceItems(this.load.ProgramItems, out enableLoop, out totalLoops, out isRemote, 
                        out periodSeconds, out periodTimeUnits, out LEDBrightness, out LEDRule, out FanRule, out MeasurementFilter);

                    // loop
                    checkBoxProgramLoop.IsChecked = enableLoop;
                    if (totalLoops > 0)
                    {
                        textBoxProgramLoopCount.Text = totalLoops.ToString();
                        radioButtonProgramLoopFinite.IsChecked = true;
                        radioButtonProgramLoopInfinite.IsChecked = false;
                    }
                    else
                    {
                        radioButtonProgramLoopFinite.IsChecked = false;
                        radioButtonProgramLoopInfinite.IsChecked = true;
                    }

                    // remote
                    if (load.IsConnected)
                    {
                        load.SetRemote(isRemote);
                    }

                    // LED
                    load.LEDBrightness = LEDBrightness;
                    load.LEDRule = LEDRule;

                    // Fan
                    load.FanRule = FanRule;

                    // Measurement filter
                    load.MeasurementFilter = MeasurementFilter;

                    // logging period
                    load.LoggingPeriod = periodSeconds;
                    load.LoggingTimeUnit = periodTimeUnits;
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Cannot read XML file", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                }
            }
        }

        // opens a dialog window for series resistance
        private void menuItemSeriesResistance_Click(object sender, RoutedEventArgs e)
        {
            SeriesResistance window = new SeriesResistance(this.load);
            window.ShowDialog();
        }

        // opens device info window
        private void menuItemDeviceInfo_Click(object sender, RoutedEventArgs e)
        {
            load.ShowDeviceInfo();
        }

        // opens about window
        private void menuItemAbout_Click(object sender, RoutedEventArgs e)
        {
            About about = new About();
            about.ShowDialog();
        }

        // open resource page in browser
        private void menuItemResources_Click(object sender, RoutedEventArgs e)
        {
            MainWindow.OpenResourcesInBrowser();
        }
    }
}
