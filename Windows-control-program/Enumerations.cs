using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MightyWatt
{
    public enum RunMode : byte { Current, Voltage, Power_CC, Power_CV, Resistance_CC, Resistance_CV, VoltageSoftware, MPPT, SimpleAmmeter };
    public enum RampMode : byte { Current, Voltage, Power_CC, Power_CV, Resistance_CC, Resistance_CV, VoltageSoftware };
    public enum WDandSkipMode : byte { Current, Voltage, Power, Resistance, Temperature };    

    //public enum Modes : byte { Current, Voltage, Power_CC, Power_CV, Resistance_CC, Resistance_CV, VoltageSoftware, MPPT, SimpleAmmeter, Temperature };
    public enum TimeUnits : byte { ms, s, min, h }
    public enum Comparison { LessThan, MoreThan }
    public enum ProgramModes : byte { Constant, Ramp, Pin };
    public enum Boards : byte { Zero, Uno };
    public enum LEDBrightnesses : byte { Off = 0, Low = 23, Medium = 105, High = 255 }; // Pre-defined brightness levels using 2.2 gamma correction
    public enum LEDRules : byte { AlwaysOff = 0, P1 = 1, V1 = 2, I1 = 4, P10 = 8, V10 = 16, I10 = 32, T50 = 64, AlwaysOn = 128 };
    public enum FanRules : byte { AlwaysOn, AutoCool, AutoQuiet };
    public enum MeasurementFilters : byte { UnfilteredNoADCAutoranging, Unfiltered, Filtered };

    static class EnumExtensions
    {
        public static readonly string[] RunModeNames = { "Current", "Voltage", "Power (CC)", "Power (CV)", "Resistance (CC)", "Resistance (CV)", "SW controlled voltage", "Max power point tracker", "Simple ammeter" };
        public static readonly string[] RunModeSymbols = { "A", "V", "W", "W", "Ω", "Ω", "V", "V", ""};

        public static readonly string[] RampModeNames = { "Current", "Voltage", "Power (CC)", "Power (CV)", "Resistance (CC)", "Resistance (CV)", "SW controlled voltage" };
        public static readonly string[] RampModeSymbols = { "A", "V", "W", "W", "Ω", "Ω", "V" };

        public static readonly string[] WDandSkipModeNames = { "Current", "Voltage", "Power", "Resistance", "Temperature" };
        public static readonly string[] WDandSkipModeSymbols = { "A", "V", "W", "Ω", "°C" };

        public static string Name(this RunMode mode)
        {
            return RunModeNames[(int)mode];
        }

        public static string Symbol(this RunMode mode)
        {
            return RunModeSymbols[(int)mode];
        }


        public static string Name(this RampMode mode)
        {
            return RampModeNames[(int)mode];
        }

        public static string Symbol(this RampMode mode)
        {
            return RampModeSymbols[(int)mode];
        }

        public static RunMode ToRunMode(this RampMode mode)
        {
            return (RunMode)((int)mode);
        }


        public static string Name(this WDandSkipMode mode)
        {
            return WDandSkipModeNames[(int)mode];
        }

        public static string Symbol(this WDandSkipMode mode)
        {
            return WDandSkipModeSymbols[(int)mode];
        }
    }
}
