using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Serialization;
using System.IO;

namespace PluginManager
{
    public class PluginsList
    {
        [Serializable]
        [XmlRoot("plugin")]
        public class Plugin
        {
            [XmlElement("name")]
            public string Name;

            [XmlElement("author")]
            public string Author;

            [XmlElement("url")]
            public string Url;

            [XmlElement("description")]
            public string Description;

            [XmlElement("path")]
            public string Path;

            [XmlElement("icon")]
            public string Icon;
        }

        private static PluginsList instance;
        public static PluginsList Instance
        {
            get
            {
                if (instance == null)
                {
                    instance = new PluginsList();
                }
                return instance;
            }
        }

        public List<Plugin> Items = new List<Plugin>();

        private void ScanDirectory(string Path)
        {
            try
            {
                using (var reader = new StringReader(File.ReadAllText(Path + "\\plugin.xml")))
                {
                    var plugin = (Plugin)new XmlSerializer(typeof(Plugin)).Deserialize(reader);
                    plugin.Path = Path + "\\" + plugin.Path;
                    plugin.Icon = Path + "\\" + plugin.Icon;
                    Items.Add(plugin);
                }
            }
            catch
            {
            }
            var dirs = Directory.GetDirectories(Path);
            foreach (var dir in dirs)
            {
                ScanDirectory(dir);
            }
        }

        public void Scan(string Path)
        {
            Items.Clear();
            ScanDirectory(Path);
        }
    }
}
