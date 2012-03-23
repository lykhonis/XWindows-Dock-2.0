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
using System.Security.Permissions;
using System.Security.Principal;
using System.Security;
using System.Diagnostics;
using System.Threading;

namespace ContainerPublicConfigurator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            slIconSize.Value = Config.IconSize;
            slGridCols.Value = Config.GridMaximumCols;
            slGridRows.Value = Config.GridMaximumRows;
            slHideDelay.Value = Config.HideDelay;
            slPopupDelay.Value = Config.PopupDelay;
            slHoverEnabled.Value = Config.HoverEnabled ? 1 : 0;
            slHoverHideDelay.Value = Config.HoverHideDelay;
            slHoverMoveDelay.Value = Config.HoverMoveDealy;
            slHoverPopupDelay.Value = Config.HoverPopupDelay;
            slHoverTextEnabled.Value = Config.HoverTextEnabled ? 1 : 0;
        }

        private void Close_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void Title_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            e.Handled = true;
            this.DragMove();
        }

        private void Default_Click(object sender, RoutedEventArgs e)
        {
            Config.Reset();

            slIconSize.Value = Config.IconSize;
            slGridCols.Value = Config.GridMaximumCols;
            slGridRows.Value = Config.GridMaximumRows;
            slHideDelay.Value = Config.HideDelay;
            slPopupDelay.Value = Config.PopupDelay;
            slHoverEnabled.Value = Config.HoverEnabled ? 1 : 0;
            slHoverHideDelay.Value = Config.HoverHideDelay;
            slHoverMoveDelay.Value = Config.HoverMoveDealy;
            slHoverPopupDelay.Value = Config.HoverPopupDelay;
            slHoverTextEnabled.Value = Config.HoverTextEnabled ? 1 : 0;
        }

        private void Apply_Click(object sender, RoutedEventArgs e)
        {
            Config.IconSize = (int)Math.Round(slIconSize.Value);
            Config.GridMaximumCols = (int)Math.Round(slGridCols.Value);
            Config.GridMaximumRows = (int)Math.Round(slGridRows.Value);
            Config.HideDelay = (int)Math.Round(slHideDelay.Value);
            Config.PopupDelay = (int)Math.Round(slPopupDelay.Value);
            Config.HoverEnabled = (int)Math.Round(slHoverEnabled.Value) != 0;
            Config.HoverTextEnabled = (int)Math.Round(slHoverTextEnabled.Value) != 0;
            Config.HoverHideDelay = (int)Math.Round(slHoverHideDelay.Value);
            Config.HoverMoveDealy = (int)Math.Round(slHoverMoveDelay.Value);
            Config.HoverPopupDelay = (int)Math.Round(slHoverPopupDelay.Value);

            var id = WindowsIdentity.GetCurrent();
            var wPrincipal = new WindowsPrincipal(id);
            if (wPrincipal.IsInRole(WindowsBuiltInRole.Administrator))
            {
                Config.Save();
            }
            else
            {
                var myProcess = new ProcessStartInfo(System.Windows.Forms.Application.ExecutablePath);
                myProcess.Verb = "runas";
                myProcess.Arguments = string.Format("-p {0} {1} {2} {3} {4} {5} {6} {7} {8} {9}",
                    Config.IconSize, Config.GridMaximumCols, Config.GridMaximumRows, Config.HideDelay, Config.PopupDelay,
                    Config.HoverEnabled, Config.HoverHideDelay, Config.HoverMoveDealy, Config.HoverPopupDelay, Config.HoverTextEnabled);
                Process.Start(myProcess);
                Close();
            }
        }
    }
}
