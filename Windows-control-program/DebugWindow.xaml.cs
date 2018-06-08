using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.ComponentModel;
using System.IO;

namespace MightyWatt
{
    /// <summary>
    /// Interakční logika pro DebugWindow.xaml
    /// </summary>
    public partial class DebugWindow : Window, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private StringBuilder stringBuilder;

        public DebugWindow()
        {
            InitializeComponent();     
            DataContext = this;
            Writer.UpdatedEvent += Writer_UpdatedEvent;
            stringBuilder = Writer.GetStringBuilder();
        }

        private void Writer_UpdatedEvent()
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Text)));
        }

        public string Text
        {
            get
            {
                return stringBuilder.ToString();
            }
        }

        public DebugWriter Writer { get; private set; } = new DebugWriter();

        private void TextBox_MouseUp(object sender, MouseButtonEventArgs e)
        {
            Clipboard.SetText(Text);
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (sender is TextBox)
            {
                (sender as TextBox).CaretIndex = int.MaxValue;
            }
        }
    }

    public delegate void DebugWriterUpdated();

    public class DebugWriter : StringWriter
    {
        public event DebugWriterUpdated UpdatedEvent;

        public override void WriteLine()
        {
            UpdatedEvent?.Invoke();
            base.WriteLine();
        }

        public override void WriteLine(string value)
        {
            UpdatedEvent?.Invoke();
            base.WriteLine(value);
        }

        public override void WriteLine(string format, object arg0)
        {
            UpdatedEvent?.Invoke();
            base.WriteLine(format, arg0);
        }

        public override void WriteLine(string format, object arg0, object arg1, object arg2)
        {
            UpdatedEvent?.Invoke();
            base.WriteLine(format, arg0, arg1, arg2);
        }

        public override void WriteLine(string format, params object[] arg)
        {
            UpdatedEvent?.Invoke();
            base.WriteLine(format, arg);
        }
    }
}
