using System;
using System.Windows.Data;
using System.Globalization;
using System.Text;

namespace MightyWatt
{
    public class Converters
    {
        // temperature in status bar
        [ValueConversion(typeof(double), typeof(string))]
        public class TemperatureConverter : IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
            {
                // temperature to full text
                if ((double)value == 0)
                {
                    return "Temperature: N/A";
                }
                return "Temperature: " + ((double)value).ToString("N0") + " °C";
            }
            public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
            {
                // not needed
                string s;
                s = ((string)value).Remove(0, 13); // remove "Temperature: "
                if (s == "N/A")
                {
                    return 0;
                }
                s = s.TrimEnd('°', ' ', 'C');
                return Double.Parse(s);
            }
        }       

        // double to string conversion
        [ValueConversion(typeof(double), typeof(string))]
        public class ValueConverter : IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, CultureInfo culture) // parameter is number format
            {
                // double to string
                return ((double)value).ToString((string)parameter, culture);                
            }
            public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
            {
                // string to double                
                if (String.IsNullOrEmpty((string)value))
                {
                    return 0;
                }
                return Double.Parse((string)value, NumberStyles.Float);
            }
        }

        [ValueConversion(typeof(double), typeof(string))]
        public class SignificantDigitsConverter : IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, CultureInfo culture) // parameter is number format
            {
                // double to string
                if (value == null)
                {
                    return string.Empty;
                }
                else
                {
                    return ToStringSignificant((double)value, (int)parameter, culture);
                }
            }
            public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
            {
                // string to double
                return Double.Parse((string)value);
            }
        }

        // int to unit (enum name) conversion
        [ValueConversion(typeof(byte), typeof(string))]
        public class UnitEnumConverter : IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
            {
                // byte to string
                try
                {
                    if ((Type)parameter == typeof(RunMode))
                    {
                        return ((RunMode)System.Convert.ToByte((int)value)).Symbol();
                    }
                    else if ((Type)parameter == typeof(RampMode))
                    {
                        return ((RampMode)System.Convert.ToByte((int)value)).Symbol();
                    }
                    else if ((Type)parameter == typeof(WDandSkipMode))
                    {
                        return ((WDandSkipMode)System.Convert.ToByte((int)value)).Symbol();
                    }

                    throw new NotImplementedException();
                }
                catch (OverflowException)
                {
                    return null;
                }
            }
            public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
            {
                // Back conversion is not possible
                throw new NotImplementedException();
            }
        }

        // double seconds to string representation in parameter-units
        [ValueConversion(typeof(double), typeof(string))]
        public class TimeConverter : IValueConverter
        {
            public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
            {
                double returnedValue = (double)value;
                switch ((TimeUnits)parameter)
                {
                    case TimeUnits.ms:
                    {
                        returnedValue *= 1000;
                        break;
                    }
                    case TimeUnits.min:
                    {
                        returnedValue /= 60;
                        break;
                    }
                    case TimeUnits.h:
                    {
                        returnedValue /= 3600;
                        break;
                    }
                }
                return returnedValue.ToString("g4", culture);
            }
            public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
            {
                double returnedValue = Double.Parse((string)value, culture);
                switch ((TimeUnits)parameter)
                {
                    case TimeUnits.ms:
                    {
                        returnedValue /= 1000;
                        break;
                    }
                    case TimeUnits.min:
                    {
                        returnedValue *= 60;
                        break;
                    }
                    case TimeUnits.h:
                    {
                        returnedValue *= 3600;
                        break;
                    }
                }
                return returnedValue;
            }
        }

        public static string ToStringSignificant(double value, int significantDigits, CultureInfo culture)
        {
            if (value == 0)
            {
                return "0";
            }
            else
            {                
                string format;
                double v = value;
                double absoluteValue = Math.Abs(value);
                int magnitude = Convert.ToInt32(Math.Ceiling(Math.Log10(absoluteValue)));          
                if (absoluteValue < 0.01)
                {
                    StringBuilder sb = new StringBuilder();
                    long multiplier = Convert.ToInt64(Math.Pow(10, 1 - magnitude));
                    v = Math.Round(value * multiplier, significantDigits - 1);
                    sb.Append(v.ToString("f" + (significantDigits - 1).ToString(), culture));
                    sb.Append("E");                    
                    sb.Append((magnitude - 1).ToString("D2"));
                    return sb.ToString();
                    //return v.ToString("f" + (significantDigits - 1).ToString(), culture) + "E" + magnitude.ToString("D2");
                }
                else
                {
                    if (magnitude < significantDigits)
                    {
                        format = "f" + (significantDigits - magnitude).ToString();
                    }
                    else
                    {
                        format = "f0";
                        double divisor = Math.Pow(10, magnitude - significantDigits);
                        v = Math.Round(value / divisor) * divisor;
                    }
                    return v.ToString(format, culture);
                }
            }
        }
    }
}
