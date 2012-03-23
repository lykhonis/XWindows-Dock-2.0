using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Serialization;
using System.IO;
using System.ComponentModel;

namespace ContainerPublic
{
    public class Settings
    {
        [Serializable]
        [XmlRoot("settings")]
        public class Data
        {
            [XmlElement("path")]
            public string Path;

            [XmlElement("icon")]
            public string Icon;
        }

        private static Data data = new Data();
        private static string filename;

        public static void Initialize(string Filename)
        {
            try
            {
                filename = Filename;
                using (var fs = new StreamReader(filename))
                {
                    var xml = fs.ReadToEnd();
                    var xs = new XmlSerializer(typeof(Data));
                    var reader = new StringReader(xml);
                    data = (Data)xs.Deserialize(reader);
                }
            }
            catch
            {
                data = new Data();
            }
        }

        public static void Save()
        {
            try
            {
                using (var fs = new StreamWriter(filename))
                {
                    var xs = new XmlSerializer(typeof(Data));
                    var writer = new StringWriter();
                    xs.Serialize(writer, data);
                    var xml = writer.ToString();
                    fs.Write(xml);
                }
            }
            catch
            {
            }
        }

        public static string Icon
        {
            get
            {
                if (data.Icon == null)
                {
                    data.Icon = string.Empty;
                }
                return data.Icon;
            }
            set
            {
                data.Icon = value;
            }
        }

        public static string Path
        {
            get
            {
                if (data.Path == null)
                {
                    data.Path = string.Empty;
                }
                return data.Path;
            }
            set
            {
                data.Path = value;
            }
        }
    }

    public class Config
    {
        [Serializable]
        [XmlRoot("configuration")]
        public class Data
        {
            [XmlElement("icon-size")]
            public int IconSize;

            [XmlElement("grid-maximum-cols")]
            public int GridMaximumCols;

            [XmlElement("grid-maximum-rows")]
            public int GridMaximumRows;

            [XmlElement("popup-delay")]
            public int PopupDelay;

            [XmlElement("hide-delay")]
            public int HideDelay;

            [XmlElement("hover-text-enabled")]
            public bool HoverTextEnabled;

            [XmlElement("hover-enabled")]
            public bool HoverEnabled;

            [XmlElement("hover-move-delay")]
            public int HoverMoveDealy;

            [XmlElement("hover-popup-delay")]
            public int HoverPopupDelay;

            [XmlElement("hover-hide-delay")]
            public int HoverHideDelay;

            public Data()
            {
                IconSize = 80;
                PopupDelay = 250;
                HideDelay = 150;
                GridMaximumCols = 6;
                GridMaximumRows = 5;
                HoverEnabled = false;
                HoverPopupDelay = 100;
                HoverHideDelay = 150;
                HoverMoveDealy = 100;
                HoverTextEnabled = true;
            }
        }

        private static Data data;

        public static bool IsInitialized { get; private set; }

        private static void Initialize()
        {
            if (!IsInitialized)
            {
                try
                {
                    using (var fs = new StreamReader(App.StartupPath + "config.xml"))
                    {
                        var xml = fs.ReadToEnd();
                        var xs = new XmlSerializer(typeof(Data));
                        var reader = new StringReader(xml);
                        data = (Data)xs.Deserialize(reader);
                    }
                    IsInitialized = true;
                }
                catch
                {
                }
                if (data == null)
                {
                    data = new Data();
                }
            }
        }

        public static void Save()
        {
            try
            {
                using (var fs = new StreamWriter(App.StartupPath + "config.xml"))
                {
                    var xs = new XmlSerializer(typeof(Data));
                    var writer = new StringWriter();
                    xs.Serialize(writer, data);
                    var xml = writer.ToString();
                    fs.Write(xml);
                }
            }
            catch
            {
            }
        }

        public static int IconSize
        {
            get
            {
                Initialize();
                return data.IconSize;
            }
        }

        public static int GridMaximumCols
        {
            get
            {
                Initialize();
                return data.GridMaximumCols;
            }
        }

        public static int GridMaximumRows
        {
            get
            {
                Initialize();
                return data.GridMaximumRows;
            }
        }

        public static int PopupDelay
        {
            get
            {
                Initialize();
                return data.PopupDelay;
            }
        }

        public static int HideDelay
        {
            get
            {
                Initialize();
                return data.HideDelay;
            }
        }

        public static bool HoverTextEnabled
        {
            get
            {
                Initialize();
                return data.HoverTextEnabled;
            }
        }

        public static bool HoverEnabled
        {
            get
            {
                Initialize();
                return data.HoverEnabled;
            }
        }

        public static int HoverHideDelay
        {
            get
            {
                Initialize();
                return data.HoverHideDelay;
            }
        }

        public static int HoverMoveDealy
        {
            get
            {
                Initialize();
                return data.HoverMoveDealy;
            }
        }

        public static int HoverPopupDelay
        {
            get
            {
                Initialize();
                return data.HoverPopupDelay;
            }
        }
    }
}
