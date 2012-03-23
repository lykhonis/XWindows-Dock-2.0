using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Serialization;
using System.IO;
using System.ComponentModel;

namespace ContainerPublicConfigurator
{
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
                Reset();
            }

            public void Reset()
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
                    using (var fs = new StreamReader(System.Windows.Forms.Application.StartupPath + "\\config.xml"))
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
                    Save();
                }
            }
        }

        public static void Save()
        {
            try
            {
                using (var fs = new StreamWriter(System.Windows.Forms.Application.StartupPath + "\\config.xml"))
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

        public static void Reset()
        {
            Initialize();
            data.Reset();
        }

        public static int IconSize
        {
            get
            {
                Initialize();
                return data.IconSize;
            }
            set
            {
                Initialize();
                data.IconSize = value;
            }
        }

        public static int GridMaximumCols
        {
            get
            {
                Initialize();
                return data.GridMaximumCols;
            }
            set
            {
                Initialize();
                data.GridMaximumCols = value;
            }
        }

        public static int GridMaximumRows
        {
            get
            {
                Initialize();
                return data.GridMaximumRows;
            }
            set
            {
                Initialize();
                data.GridMaximumRows = value;
            }
        }

        public static int PopupDelay
        {
            get
            {
                Initialize();
                return data.PopupDelay;
            }
            set
            {
                Initialize();
                data.PopupDelay = value;
            }
        }

        public static int HideDelay
        {
            get
            {
                Initialize();
                return data.HideDelay;
            }
            set
            {
                Initialize();
                data.HideDelay = value;
            }
        }

        public static bool HoverEnabled
        {
            get
            {
                Initialize();
                return data.HoverEnabled;
            }
            set
            {
                Initialize();
                data.HoverEnabled = value;
            }
        }

        public static bool HoverTextEnabled
        {
            get
            {
                Initialize();
                return data.HoverTextEnabled;
            }
            set
            {
                Initialize();
                data.HoverTextEnabled = value;
            }
        }

        public static int HoverPopupDelay
        {
            get
            {
                Initialize();
                return data.HoverPopupDelay;
            }
            set
            {
                Initialize();
                data.HoverPopupDelay = value;
            }
        }

        public static int HoverMoveDealy
        {
            get
            {
                Initialize();
                return data.HoverMoveDealy;
            }
            set
            {
                Initialize();
                data.HoverMoveDealy = value;
            }
        }

        public static int HoverHideDelay
        {
            get
            {
                Initialize();
                return data.HoverHideDelay;
            }
            set
            {
                Initialize();
                data.HoverHideDelay = value;
            }
        }
    }
}
