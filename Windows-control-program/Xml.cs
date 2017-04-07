using System;
using System.Xml;
using System.ComponentModel;

namespace MightyWatt
{
    class Xml
    {
        private string path;
        private XmlDocument document;
        private Converters.TimeConverter timeConverter;

        public Xml(string path) // creates a new instance of Xml file (either existing or to-be-created)
        {
            this.path = path;
            document = new XmlDocument();
        }

        // creates Xml file with program items, loop info, local/remote info and period settings
        public void SaveItems(BindingList<ProgramItem> programItems, bool loopIsEnabled, int loopCount, bool isRemote,
            double logPeriod, TimeUnits logTimeUnit, LEDBrightnesses LEDBrightness, byte LEDRule, FanRules FanRule, MeasurementFilters MeasurementFilter, bool AutorangingCurrent, bool AutorangingVoltage) 
        {
            XmlNode declaration = document.CreateXmlDeclaration("1.0", "UTF-8", null);
            document.AppendChild(declaration);
            XmlNode comment = document.CreateComment("MightyWatt R3 experiment settings");
            document.AppendChild(comment);
            XmlElement main = document.CreateElement("settings");
            document.AppendChild(main);
           
            if (loopIsEnabled == true)
            {
                // loop settings
                XmlNode loop = document.CreateElement("loop");
                
                // duration
                XmlAttribute count = document.CreateAttribute("count");
                if (loopCount < 0)
                {
                    count.Value = "infinite";
                }
                else
                {
                    count.Value = loopCount.ToString();
                }
                loop.Attributes.Append(count);
                main.AppendChild(loop);
            }

            // local/remote settings
            XmlNode remote = document.CreateElement("voltageSenseMode");
            main.AppendChild(remote);
            // is remote
            XmlAttribute isrem = document.CreateAttribute("isRemote");
            isrem.Value = isRemote.ToString();
            remote.Attributes.Append(isrem);

            // LED settings
            XmlNode LED = document.CreateElement("LED");
            main.AppendChild(LED);
            // brightness
            XmlAttribute brightness = document.CreateAttribute("brightness");
            brightness.Value = Enum.GetName(typeof(LEDBrightnesses), LEDBrightness);
            LED.Attributes.Append(brightness);
            // rules
            XmlAttribute LEDrule = document.CreateAttribute("rules");
            LEDrule.Value = "0x" + LEDRule.ToString("X2");            
            LED.Attributes.Append(LEDrule);

            // Fan
            XmlNode FanRules = document.CreateElement("fan");
            main.AppendChild(FanRules);
            // rules
            XmlAttribute AFanRule = document.CreateAttribute("rules");
            AFanRule.Value = Enum.GetName(typeof(FanRules), FanRule);
            FanRules.Attributes.Append(AFanRule);

            // Measurement filter
            XmlNode MeasurementFilters = document.CreateElement("measurement");
            main.AppendChild(MeasurementFilters);
            // filter
            XmlAttribute AMeasurementFilter = document.CreateAttribute("filter");
            AMeasurementFilter.Value = Enum.GetName(typeof(MeasurementFilters), MeasurementFilter);
            MeasurementFilters.Attributes.Append(AMeasurementFilter);

            // Autoranging
            XmlNode Autoranging = document.CreateElement("autoranging");
            main.AppendChild(Autoranging);
            // current
            XmlAttribute CurrentAutoranging = document.CreateAttribute("current");
            CurrentAutoranging.Value = AutorangingCurrent.ToString();
            Autoranging.Attributes.Append(CurrentAutoranging);
            // voltage
            XmlAttribute VoltageAutoranging = document.CreateAttribute("voltage");
            VoltageAutoranging.Value = AutorangingVoltage.ToString();
            Autoranging.Attributes.Append(VoltageAutoranging);

            // logging settings
            XmlNode loggingPeriod = document.CreateElement("logging");
            main.AppendChild(loggingPeriod);
            XmlAttribute periodValue = document.CreateAttribute("period");
            timeConverter = new Converters.TimeConverter(); // convert seconds to representation in time-unit
            periodValue.Value = (string)(timeConverter.Convert(logPeriod, typeof(string), logTimeUnit, System.Globalization.CultureInfo.CurrentCulture));
            loggingPeriod.Attributes.Append(periodValue);

            XmlAttribute periodTimeUnit = document.CreateAttribute("timeUnit");
            periodTimeUnit.Value = logTimeUnit.ToString();
            loggingPeriod.Attributes.Append(periodTimeUnit);

            // items
            XmlNode items = document.CreateElement("items");
            main.AppendChild(items);

            foreach (ProgramItem programItem in programItems)
            {
                XmlNode item = document.CreateElement("item");
                
                XmlAttribute programMode = document.CreateAttribute("programMode");
                programMode.Value = programItem.ProgramMode.ToString();
                item.Attributes.Append(programMode);

                // parameters                
                XmlAttribute mode = document.CreateAttribute("mode");
                mode.Value = programItem.Mode.ToString();
                item.Attributes.Append(mode);

                switch (programItem.ProgramMode)
                {
                    case ProgramModes.Constant:                    
                        {
                            XmlAttribute value = document.CreateAttribute("value");
                            if (programItem.Value == null)
                            {
                                value.Value = "previous";
                            }
                            else
                            {
                                value.Value = programItem.Value.ToString();
                            }
                            item.Attributes.Append(value);
                            break;
                        }
                    case ProgramModes.Ramp:
                        {
                            XmlAttribute startingValue = document.CreateAttribute("startingValue");
                            if (programItem.StartingValue == null)
                            {
                                startingValue.Value = "previous";
                            }
                            else
                            {
                                startingValue.Value = programItem.StartingValue.ToString();
                            }
                            item.Attributes.Append(startingValue);

                            XmlAttribute finalValue = document.CreateAttribute("finalValue");
                            finalValue.Value = programItem.FinalValue.ToString();
                            item.Attributes.Append(finalValue);
                            break;
                        }
                }

                XmlAttribute duration = document.CreateAttribute("duration");
                duration.Value = programItem.DurationString;
                item.Attributes.Append(duration);

                XmlAttribute timeUnit = document.CreateAttribute("timeUnit");
                timeUnit.Value = programItem.TimeUnit.ToString();
                item.Attributes.Append(timeUnit);

                // skip attributes                
                if (programItem.SkipEnabled)
                {
                    XmlNode skip = document.CreateElement("skip");
                    XmlAttribute skipMode = document.CreateAttribute("mode");
                    skipMode.Value = programItem.SkipMode.ToString();
                    skip.Attributes.Append(skipMode);
                    XmlAttribute skipComparator = document.CreateAttribute("comparator");
                    skipComparator.Value = programItem.SkipComparator.ToString();
                    skip.Attributes.Append(skipComparator);
                    XmlAttribute skipValue = document.CreateAttribute("value");
                    skipValue.Value = programItem.SkipValue.ToString();
                    skip.Attributes.Append(skipValue);
                    item.AppendChild(skip);
                }
                
                items.AppendChild(item);
            }

            document.Save(path);
        }

        // adds items from xml file to already existing list of program items; ignores loop settings
        public void AddItems(BindingList<ProgramItem> programItems)
        {
            XmlElement main;
            XmlNode items;
            XmlNode parameter;
            document.Load(this.path);
            main = document.DocumentElement;
            items = main.SelectSingleNode("items");

            foreach (XmlNode item in items)
            {
                ProgramModes programMode;
                bool result = (Enum.TryParse(item.Attributes.GetNamedItem("programMode").Value, out programMode));
                if (result)
                {
                    switch (programMode)
                    {
                        case ProgramModes.Constant:
                            {
                                // mode
                                Modes mode = (Modes)(Enum.Parse(typeof(Modes), item.Attributes.GetNamedItem("mode").Value));

                                // value
                                parameter = item.Attributes.GetNamedItem("value");
                                double? value = null;
                                if (parameter.Value != "previous")
                                {
                                    value = Double.Parse(parameter.Value);
                                }

                                // duration
                                string durationString = item.Attributes.GetNamedItem("duration").Value;

                                // timeunits
                                TimeUnits timeunit = (TimeUnits)(Enum.Parse(typeof(TimeUnits), item.Attributes.GetNamedItem("timeUnit").Value));

                                // skip
                                bool skipEnabled = false;
                                Modes skipMode;
                                Comparison skipComparator;
                                double skipValue;
                                foreach (XmlNode node in item.ChildNodes)
                                {
                                    if (node.Name == "skip")
                                    {
                                        skipEnabled = true;
                                        skipMode = (Modes)(Enum.Parse(typeof(Modes), node.Attributes.GetNamedItem("mode").Value));
                                        skipComparator = (Comparison)(Enum.Parse(typeof(Comparison), node.Attributes.GetNamedItem("comparator").Value));
                                        skipValue = Double.Parse(node.Attributes.GetNamedItem("value").Value);
                                        programItems.Add(new ProgramItem(mode, value, durationString, timeunit, skipMode, skipComparator, skipValue));
                                        break;
                                    }
                                }
                                if (!skipEnabled)
                                {
                                    programItems.Add(new ProgramItem(mode, value, durationString, timeunit));
                                }
                                break;
                            }
                        case ProgramModes.Ramp:
                            {
                                // mode
                                Modes mode = (Modes)(Enum.Parse(typeof(Modes), item.Attributes.GetNamedItem("mode").Value));

                                // starting value
                                parameter = item.Attributes.GetNamedItem("startingValue");
                                double? startingValue = null;
                                if (parameter.Value != "previous")
                                {
                                    startingValue = Double.Parse(parameter.Value);
                                }

                                // final value
                                parameter = item.Attributes.GetNamedItem("finalValue");
                                double finalValue;
                                finalValue = Double.Parse(parameter.Value);

                                // duration
                                string durationString = item.Attributes.GetNamedItem("duration").Value;

                                // timeunits
                                TimeUnits timeunit = (TimeUnits)(Enum.Parse(typeof(TimeUnits), item.Attributes.GetNamedItem("timeUnit").Value));

                                // skip
                                bool skipEnabled = false;
                                Modes skipMode;
                                Comparison skipComparator;
                                double skipValue;
                                foreach (XmlNode node in item.ChildNodes)
                                {
                                    if (node.Name == "skip")
                                    {
                                        skipEnabled = true;
                                        skipMode = (Modes)(Enum.Parse(typeof(Modes), node.Attributes.GetNamedItem("mode").Value));
                                        skipComparator = (Comparison)(Enum.Parse(typeof(Comparison), node.Attributes.GetNamedItem("comparator").Value));
                                        skipValue = Double.Parse(node.Attributes.GetNamedItem("value").Value);
                                        programItems.Add(new ProgramItem(mode, startingValue, finalValue, durationString, timeunit, skipMode, skipComparator, skipValue));
                                        break;
                                    }
                                }
                                if (!skipEnabled)
                                {
                                    programItems.Add(new ProgramItem(mode, startingValue, finalValue, durationString, timeunit));
                                }
                                break;
                            }
                    }
                }
            }
        }

        public void LoadSettings(out bool loopIsEnabled, out int loopCount, out bool isRemote, out double periodSeconds, out TimeUnits periodTimeUnits, out LEDBrightnesses LEDBrightness, out byte LEDRule, out FanRules FanRule, out MeasurementFilters MeasurementFilter, out bool CurrentAutoranging, out bool VoltageAutoranging)
        {
            loopIsEnabled = false;
            loopCount = 1;
            isRemote = false;
            periodSeconds = 1;
            periodTimeUnits = TimeUnits.s;

            document.Load(this.path);
            XmlElement main;
            main = document.DocumentElement;

            // loop
            foreach (XmlNode node in main.ChildNodes)
            {
                if (node.Name == "loop")
                {
                    loopIsEnabled = true;
                    if (node.Attributes.GetNamedItem("count").Value == "infinite")
                    {
                        loopCount = 0;
                    }
                    else
                    {
                        loopCount = Int32.Parse(node.Attributes.GetNamedItem("count").Value);
                    }
                    break;
                }
            }

            // remote/local
            XmlNode remote;
            remote = main.SelectSingleNode("voltageSenseMode");
            isRemote = Boolean.Parse(remote.Attributes.GetNamedItem("isRemote").Value);

            // LED settings
            XmlNode LED;
            LED = main.SelectSingleNode("LED");
            // brightness
            LEDBrightnesses lb;
            if (Enum.TryParse(LED.Attributes.GetNamedItem("brightness").Value, out lb))
            {
                LEDBrightness = lb;
            }
            else
            {
                LEDBrightness = Load.DefaultLEDBrightness;
            }
            // rules
            byte ledRule;
            bool result;
            string hex = LED.Attributes.GetNamedItem("rules").Value;
            if (hex.StartsWith("0x", StringComparison.InvariantCultureIgnoreCase))
            {
                result = byte.TryParse(hex.Substring(2), System.Globalization.NumberStyles.HexNumber, System.Globalization.CultureInfo.InvariantCulture, out ledRule);
            }
            else
            {
                result = byte.TryParse(hex, out ledRule);
            }
            if (result)
            {
                LEDRule = ledRule;
            }
            else
            {
                LEDRule = Load.DefaultLEDRule;
            }

            // Fan
            XmlNode Fan;
            Fan = main.SelectSingleNode("fan");
            // rule
            FanRules fr;
            if (Enum.TryParse(Fan.Attributes.GetNamedItem("rules").Value, out fr))
            {
                FanRule = fr;
            }
            else
            {
                FanRule = Load.DefaultFanRule;
            }

            // Measurement
            XmlNode Measurement;
            Measurement = main.SelectSingleNode("measurement");
            // filter
            MeasurementFilters ms;
            if (Enum.TryParse(Measurement.Attributes.GetNamedItem("filter").Value, out ms))
            {
                MeasurementFilter = ms;
            }
            else
            {
                MeasurementFilter = Load.DefaultMeasurementFilter;
            }

            // Autoranging
            XmlNode Autoranging;
            Autoranging = main.SelectSingleNode("autoranging");
            // current
            if (bool.TryParse(Autoranging.Attributes.GetNamedItem("current").Value, out CurrentAutoranging) == false)
            {
                CurrentAutoranging = Load.DefaultAutorangingCurrent;
            }
            // voltage
            if (bool.TryParse(Autoranging.Attributes.GetNamedItem("voltage").Value, out VoltageAutoranging) == false)
            {
                VoltageAutoranging = Load.DefaultAutorangingVoltage;
            }

            // period settings
            XmlNode loggingPeriod;
            loggingPeriod = main.SelectSingleNode("logging");
            periodTimeUnits = (TimeUnits)Enum.Parse(typeof(TimeUnits), loggingPeriod.Attributes.GetNamedItem("timeUnit").Value);
            timeConverter = new Converters.TimeConverter(); // convert representation in time-unit to seconds
            periodSeconds = (double)(timeConverter.ConvertBack(loggingPeriod.Attributes.GetNamedItem("period").Value, typeof(string), periodTimeUnits, System.Globalization.CultureInfo.CurrentCulture));
        }

        // clears the existing list of program items and replaces them with the items from xml file; changes loop settings, local/remote and period settings according to the xml file
        public void ReplaceItems(BindingList<ProgramItem> programItems, out bool loopIsEnabled, out int loopCount, out bool isRemote,
            out double periodSeconds, out TimeUnits periodTimeUnits, out LEDBrightnesses LEDBrightness, out byte LEDRule, out FanRules FanRule, out MeasurementFilters MeasurementFilter, out bool CurrentAutoranging, out bool VoltageAutoranging)
        {
            // load settings
            LoadSettings(out loopIsEnabled, out loopCount, out isRemote, out periodSeconds, out periodTimeUnits, out LEDBrightness, out LEDRule, out FanRule, out MeasurementFilter, out CurrentAutoranging, out VoltageAutoranging);
            
            // clear program items list
            programItems.Clear();       
            
            // add elements
            AddItems(programItems);
        }
    }
}