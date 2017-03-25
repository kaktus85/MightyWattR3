using System;
using System.Windows;

namespace MightyWatt
{
    /// <summary>
    /// Interaction logic for DeviceInfo.xaml
    /// </summary>
    public partial class DeviceInfo : Window
    {
        private const string numberFormat = "0.000";

        internal DeviceInfo(Communication device)
        {
            InitializeComponent();
        
            this.textBoxFirmwareVersion.Text = device.FirmwareVersion;
            this.textBoxBoardRevision.Text = device.BoardRevision;
            this.textBoxCalibrationDate.Text = device.CalibrationDate;            
           
            if (device.MaxIadc > 0)
            {
                this.textBoxMaxIadc.Text = device.MaxIadc.ToString(numberFormat);
            }
            else
            {
                this.textBoxMaxIadc.Text = "–";
            }

            if (device.MaxIdac > 0)
            {
                this.textBoxMaxIdac.Text = device.MaxIdac.ToString(numberFormat);
            }
            else
            {
                this.textBoxMaxIdac.Text = "–";
            }

            if (device.MaxVadc > 0)
            {
                this.textBoxMaxVadc.Text = device.MaxVadc.ToString(numberFormat);
            }
            else
            {
                this.textBoxMaxVadc.Text = "–";
            }

            if (device.MaxVdac > 0)
            {
                this.textBoxMaxVdac.Text = device.MaxVdac.ToString(numberFormat);
            }
            else
            {
                this.textBoxMaxVdac.Text = "–";
            }

            this.textBoxMaxPower.Text = device.MaxPower.ToString(numberFormat);
            this.textBoxDvmInputResistance.Text = device.DvmInputResistance.ToString("0");
            this.textBoxMaximumTemperature.Text = device.TemperatureThreshold.ToString("0");
        }
    }
}
