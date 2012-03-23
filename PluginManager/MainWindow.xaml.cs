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
using Microsoft.Win32;

namespace PluginManager
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public string XWindowsDockPath;

        public MainWindow()
        {
            InitializeComponent();
            RefreshList();
        }

        private void RefreshList()
        {
            try
            {
                spContent.Children.Clear();

                using (var key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\XWindows Dock_is1"))
                {
                    XWindowsDockPath = (string)key.GetValue("InstallLocation") + "\\Public Plugins";
                }

                PluginsList.Instance.Scan(XWindowsDockPath);

                foreach (var plugin in PluginsList.Instance.Items)
                {
                    var ctrl = new PluginItem();
                    ctrl.Plugin = plugin;
                    spContent.Children.Add(ctrl);

                    var line = new Rectangle();
                    line.Margin = new Thickness(8, 2, 8, 2);
                    line.Height = 1;
                    line.HorizontalAlignment = HorizontalAlignment.Stretch;
                    line.VerticalAlignment = VerticalAlignment.Top;
                    line.StrokeThickness = 0;
                    line.Fill = new SolidColorBrush(Color.FromRgb(0xE0, 0xE0, 0xE0));
                    spContent.Children.Add(line);
                }
            }
            catch
            {
            }
        }

        private void btnClose_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void btnRefresh_Click(object sender, RoutedEventArgs e)
        {
            RefreshList();
        }

        private void Title_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (!btnClose.IsMouseOver)
            {
                this.DragMove();
            }
        }
    }
}
