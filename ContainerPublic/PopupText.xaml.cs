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
using System.Windows.Shapes;
using System.Windows.Media.Animation;

namespace ContainerPublic
{
    /// <summary>
    /// Interaction logic for PopupText.xaml
    /// </summary>
    public partial class PopupText : UserControl
    {
        public GridIconControl IconControl { get; private set; }

        public PopupText()
        {
            InitializeComponent();
        }

        public void Popup(GridIconControl icon)
        {
            if ((!Config.HoverTextEnabled) || (App.ViewMode != App.ViewModeEnum.Grid))
            {
                return;
            }

            (Resources["HideAnimation"] as Storyboard).Stop();
            if (IsVisible)
            {
                (Resources["KeepPopupAnimation"] as Storyboard).Begin();
                if (IconControl == icon)
                {
                    return;
                }
            }
            else
            {
                (Resources["PopupAnimation"] as Storyboard).Begin();
            }

            IconControl = icon;

            lbTitle.Text = IconControl.Title;
            
            var size = App.MeasureTextSize(lbTitle.Text, lbTitle.FontFamily, lbTitle.FontStyle, lbTitle.FontWeight, lbTitle.FontStretch, lbTitle.FontSize);
            size.Width += Border.Padding.Left + Border.Padding.Right;
            size.Height += Border.Padding.Top + Border.Padding.Bottom;

            var bounds = IconControl.TransformToAncestor((App.Current.MainWindow as GridView).GridContent).TransformBounds(new Rect(0, 0, IconControl.ActualWidth, IconControl.ActualHeight));

            var top = bounds.Top + IconControl.IconSize + 2 - Border.Padding.Top;
            var left = bounds.Left + (bounds.Width - size.Width) / 2;
            if (left + size.Width > (App.Current.MainWindow as GridView).wpContent.ActualWidth)
            {
                left -= left + size.Width - (App.Current.MainWindow as GridView).wpContent.ActualWidth;
            }
            if (left < 0)
            {
                left = 0;
            }
            Margin = new Thickness(left, top, 0, 0);
        }

        public void Hide()
        {
            if (IconControl is GridIconControl)
            {
                IconControl = null;
                (Resources["HideAnimation"] as Storyboard).Begin();
            }
        }

        private void UserControl_PreviewMouseMove(object sender, MouseEventArgs e)
        {
            if (IconControl is GridIconControl)
            {
                var ev = new MouseEventArgs(Mouse.PrimaryDevice, 0);
                ev.RoutedEvent = MouseLeaveEvent;
                IconControl.RaiseEvent(ev);
            }
        }

        private void UserControl_PreviewMouseRightButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (IconControl is GridIconControl)
            {
                var ev = new MouseButtonEventArgs(Mouse.PrimaryDevice, 0, MouseButton.Right);
                ev.RoutedEvent = PreviewMouseRightButtonUpEvent;
                IconControl.RaiseEvent(ev);
            }
        }

        private void UserControl_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (IconControl is GridIconControl)
            {
                var ev = new MouseButtonEventArgs(Mouse.PrimaryDevice, 0, MouseButton.Left);
                ev.RoutedEvent = PreviewMouseLeftButtonDownEvent;
                IconControl.RaiseEvent(ev);
            }
        }

        private void UserControl_PreviewMouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (IconControl is GridIconControl)
            {
                var ev = new MouseButtonEventArgs(Mouse.PrimaryDevice, 0, MouseButton.Left);
                ev.RoutedEvent = PreviewMouseLeftButtonUpEvent;
                IconControl.RaiseEvent(ev);
            }
        }
    }
}
