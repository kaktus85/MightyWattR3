using System;
using System.Windows;

namespace MightyWatt
{
    /// <summary>
    /// Interaction logic for About.xaml
    /// </summary>
    public partial class About : Window
    {
        public About()
        {
            InitializeComponent();
            labelVersion.Content = "Version: " + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
            labelMinimumFWVersion.Content = "Minimum firmware version: " + Load.MinimumFWVersion[0].ToString() + "." + Load.MinimumFWVersion[1].ToString() + "." + Load.MinimumFWVersion[2].ToString();
        }

        private void image1_MouseUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            MainWindow.OpenResourcesInBrowser();            
        }

        private void button_Click(object sender, RoutedEventArgs e)
        {
            MainWindow.OpenResourcesInBrowser();
        }
    }
}
