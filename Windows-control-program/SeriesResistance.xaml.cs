using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace MightyWatt
{
    /// <summary>
    /// Interaction logic for SeriesResistance.xaml
    /// </summary>
    public partial class SeriesResistance : Window
    {
        private Load program; // reference to program instance from which settings are read and changes are made
        private Converters.ValueConverter vc;

        internal SeriesResistance(Load program)
        {
            InitializeComponent();
            this.program = program;
            vc = new Converters.ValueConverter();
            textBoxResistance.Text = (string)(vc.Convert(this.program.SeriesResistance, typeof(string), "f3", System.Globalization.CultureInfo.CurrentCulture));            
        }

        private void buttonOK_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                this.program.SeriesResistance = (double)(vc.ConvertBack(textBoxResistance.Text, typeof(double), null, System.Globalization.CultureInfo.CurrentCulture));
                DialogResult = true;
            }
            catch (OverflowException)
            {
                MessageBox.Show("Allowed resistance range: 0 to 65.535 Ω", "Resistance out of range", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error parsing data", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
    }
}
