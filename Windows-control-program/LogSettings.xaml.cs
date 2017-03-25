using System;
using System.Windows;

namespace MightyWatt
{
    /// <summary>
    /// Interaction logic for LogSettings.xaml
    /// </summary>
    public partial class LogSettings : Window
    {
        private Load program; // reference to program instance from which settings are read and changes are made
        Converters.TimeConverter timeConverter;

        internal LogSettings(Load program)
        {
            InitializeComponent();
            this.program = program;
            timeConverter = new Converters.TimeConverter();      
      
            // fill with current values
            textBoxPeriod.Text = (string)(timeConverter.Convert(program.LoggingPeriod, typeof(string), program.LoggingTimeUnit, System.Globalization.CultureInfo.CurrentCulture));
            comboBoxUnit.Items.Clear();
            for (byte i = 0; i < (Enum.GetValues(typeof(TimeUnits))).Length; i++)
            {
                comboBoxUnit.Items.Add((TimeUnits)i);
                if ((byte)(program.LoggingTimeUnit) == i)
                {
                    comboBoxUnit.SelectedIndex = i;
                }
            }
        }

        private void buttonOK_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                // assign new values
                this.program.LoggingPeriod = (double)(this.timeConverter.ConvertBack(textBoxPeriod.Text, typeof(double), (TimeUnits)comboBoxUnit.SelectedItem, System.Globalization.CultureInfo.CurrentCulture));
                this.program.LoggingTimeUnit = (TimeUnits)(comboBoxUnit.SelectedItem);            
                DialogResult = true;
            }
            catch (ArgumentOutOfRangeException ex)
            {
                MessageBox.Show(ex.Message, "Period out of range", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error parsing data", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
    }
}
