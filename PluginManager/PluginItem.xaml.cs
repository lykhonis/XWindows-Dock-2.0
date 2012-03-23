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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace PluginManager
{
    /// <summary>
    /// Interaction logic for PluginItem.xaml
    /// </summary>
    public partial class PluginItem : UserControl
    {
        private PluginsList.Plugin plugin;
        public PluginsList.Plugin Plugin
        {
            get
            {
                return plugin;
            }
            set
            {
                plugin = value;
                try
                {
                    lbAuthor.Text = plugin.Author;
                    lbName.Text = plugin.Name;
                    lbDescription.Text = plugin.Description;
                    imgIcon.Source = new BitmapImage(new Uri(plugin.Icon, UriKind.Absolute));
                }
                catch
                {
                    imgIcon.Source = null;
                }
            }
        }

        public PluginItem()
        {
            InitializeComponent();
        }

        private void lbAuthor_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            try
            {
                System.Diagnostics.Process.Start(Plugin.Url);
            }
            catch
            {
            }
        }

        private void btnRun_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                System.Diagnostics.Process.Start(Plugin.Path);
            }
            catch
            {
            }
        }

        private void btnFolder_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                System.Diagnostics.Process.Start(Plugin.Path.Substring(0, Plugin.Path.LastIndexOf('\\')));
            }
            catch
            {
            }
        }
    }
}
