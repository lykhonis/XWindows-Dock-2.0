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
using System.Windows.Interop;
using System.Diagnostics;
using System.IO;
using System.Windows.Media.Animation;
using XWindowsDock;

namespace ContainerPublic
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class GridView : Window
    {
        public enum AnimationPathEnum { FromBottomToTop, FromLeftToRight, FromTopToBottom, FromRightToLeft }
        public AnimationPathEnum AnimationPath { get; private set; }

        public bool IsClosing { get; private set; }
        public bool IsOpened { get; private set; }

        public const int MaxItems = 80;

        private double OffsetX;
        private double OffsetY;

        public GridView()
        {
            InitializeComponent();
        }

        private void NormalizeOffsets()
        {
            var bounds = System.Windows.Forms.Screen.PrimaryScreen.Bounds;
            OffsetX = 0;
            OffsetY = 0;
            if (Left < bounds.Left)
            {
                OffsetX += Left - bounds.Left;
                Left = bounds.Left;
            }
            if (Top < bounds.Top)
            {
                OffsetY += Top - bounds.Top;
                Top = bounds.Top;
            }
            if (Left + Width > bounds.Right)
            {
                OffsetX += Left + Width - bounds.Right;
                Left = bounds.Right - Width;
            }
            if (Top + Height > bounds.Bottom)
            {
                OffsetY += Top + Height - bounds.Bottom;
                Top = bounds.Bottom - Height;
            }
        }

        public void MoveHover(GridIconControl icon)
        {
            if (Config.HoverEnabled)
            {
                if (Hover.Opacity == 0)
                {
                    var anim = new DoubleAnimation(0, 1, new Duration(TimeSpan.FromMilliseconds(Config.HoverPopupDelay)));
                    Hover.BeginAnimation(OpacityProperty, anim);
                }
                else
                {
                    Hover.BeginAnimation(OpacityProperty, null);
                    Hover.Opacity = 1;
                }
                var bounds = icon.TransformToAncestor(wpContent).TransformBounds(new Rect(0, 0, icon.ActualWidth, icon.ActualHeight));
                var anim2 = new ThicknessAnimation(Hover.Margin, new Thickness(bounds.Left - 3, bounds.Top - 3, 0, 0), new Duration(TimeSpan.FromMilliseconds(Config.HoverMoveDealy)));
                anim2.AccelerationRatio = 0.2;
                Hover.BeginAnimation(MarginProperty, null);
                Hover.BeginAnimation(MarginProperty, anim2);
            }
        }

        protected override void OnInitialized(EventArgs e)
        {
            base.OnInitialized(e);

            Hover.Width = Config.IconSize + 16 * 2 + 6;
            Hover.Height = Config.IconSize + 20 + 6;

            wpContent.Children.Clear();

            btnBack.Visibility = Visibility.Collapsed;
            if (App.Path.LastIndexOf('\\') > 2)
            {
                btnBack.Visibility = Visibility.Visible;
            }

            var i = App.Path.LastIndexOf('\\');
            lbTitle.Text = App.Path;
            if (i >= 0)
            {
                if (i == App.Path.Length - 1)
                {
                    lbTitle.Text = lbTitle.Text.Remove(i);
                    i = lbTitle.Text.LastIndexOf('\\');
                    if (i >= 0)
                    {
                        lbTitle.Text = lbTitle.Text.Remove(0, i + 1);
                    }
                }
                else
                {
                    lbTitle.Text = lbTitle.Text.Remove(0, i + 1);
                }
            }

            foreach (var dir in App.Directories)
            {
                var attr = File.GetAttributes(dir);
                if ((attr & (FileAttributes.Hidden | FileAttributes.System)) == 0)
                {
                    var ctrl = new GridIconControl();
                    ctrl.IconSize = Config.IconSize;
                    ctrl.Margin = new Thickness(0, 10, 0, 10);
                    ctrl.Filename = dir;
                    wpContent.Children.Add(ctrl);
                }
            }

            foreach (var file in App.Files)
            {
                var attr = File.GetAttributes(file);
                if ((attr & (FileAttributes.Hidden | FileAttributes.System)) == 0)
                {
                    var ctrl = new GridIconControl();
                    ctrl.IconSize = Config.IconSize;
                    ctrl.Margin = new Thickness(0, 10, 0, 10);
                    ctrl.Filename = file;
                    wpContent.Children.Add(ctrl);
                }
            }

            var bounds = System.Windows.Forms.Screen.PrimaryScreen.Bounds;
            var monitorSize = Math.Min(bounds.Width, bounds.Height);

            var cellWidth = Config.IconSize + 16 * 2;
            var cellHeight = Config.IconSize + 20 + 10 + 10;

            var cols = Config.GridMaximumCols;
            var rows = Config.GridMaximumRows;

            if (cols * rows > Math.Max(wpContent.Children.Count, 1))
            {
                var fileCount = Math.Max(wpContent.Children.Count, 1);
                if (fileCount == 1)
                {
                    cols = 1;
                    rows = 1;
                }
                else
                {
                    cols = (int)Math.Sqrt(fileCount) + 1;
                    rows = fileCount / cols;
                    if (cols * rows < fileCount)
                    {
                        cols++;
                    }
                    if (cols * rows < fileCount)
                    {
                        rows++;
                    }
                }
            }

            var rect = DockIcon.IconRect;
            switch (DockIcon.DockEdge)
            {
                case DockIcon.DockEdgeEnum.Bottom:
                    Width = 14 * 2 + 5 * 2 + cellWidth * cols + 10;
                    Height = 24 + 14 * 2 + 20 + 5 * 2 + cellHeight * rows;

                    Left = rect.Left + (rect.Width - Width) / 2;
                    Top = rect.Top - Height + 10;

                    NormalizeOffsets();

                    ArrowBackground.HorizontalAlignment = HorizontalAlignment.Center;
                    ArrowBackground.VerticalAlignment = VerticalAlignment.Bottom;
                    ArrowBackground.Margin = new Thickness(10 + OffsetX * 2, 0, 10, 0);

                    ContentBackground.Margin = new Thickness(0, 0, 0, 20);

                    AnimationPath = AnimationPathEnum.FromBottomToTop;
                    break;

                case DockIcon.DockEdgeEnum.Top:
                    Width = 14 * 2 + 5 * 2 + cellWidth * cols + 10;
                    Height = 24 + 14 * 2 + 20 + 5 * 2 + cellHeight * rows;

                    Left = rect.Left + (rect.Width - Width) / 2;
                    Top = rect.Top + rect.Height - 10;

                    NormalizeOffsets();

                    ArrowBackground.HorizontalAlignment = HorizontalAlignment.Center;
                    ArrowBackground.VerticalAlignment = VerticalAlignment.Top;
                    ArrowBackground.LayoutTransform = new RotateTransform(180);
                    ArrowBackground.Margin = new Thickness(10 + OffsetX * 2, 0, 10, 0);

                    ContentBackground.Margin = new Thickness(0, 20, 0, 0);

                    AnimationPath = AnimationPathEnum.FromTopToBottom;
                    break;

                case DockIcon.DockEdgeEnum.Left:
                    Width = 14 * 2 + 5 * 2 + 20 + cellWidth * cols + 10;
                    Height = 24 + 14 * 2 + 5 * 2 + cellHeight * rows;

                    Left = rect.Left + rect.Width - 10;
                    Top = rect.Top + (rect.Height - Height) / 2;

                    NormalizeOffsets();

                    ArrowBackground.HorizontalAlignment = HorizontalAlignment.Left;
                    ArrowBackground.VerticalAlignment = VerticalAlignment.Center;
                    ArrowBackground.LayoutTransform = new RotateTransform(90);
                    ArrowBackground.Margin = new Thickness(0, 10 + OffsetY * 2, 0, 10);

                    ContentBackground.Margin = new Thickness(20, 0, 0, 0);

                    AnimationPath = AnimationPathEnum.FromLeftToRight;
                    break;

                case DockIcon.DockEdgeEnum.Right:
                    Width = 14 * 2 + 5 * 2 + 20 + cellWidth * cols + 10;
                    Height = 24 + 14 * 2 + 5 * 2 + cellHeight * rows;

                    Left = rect.Left - Width + 10;
                    Top = rect.Top + (rect.Height - Height) / 2;

                    NormalizeOffsets();

                    ArrowBackground.HorizontalAlignment = HorizontalAlignment.Right;
                    ArrowBackground.VerticalAlignment = VerticalAlignment.Center;
                    ArrowBackground.LayoutTransform = new RotateTransform(-90);
                    ArrowBackground.Margin = new Thickness(0, 10 + OffsetY * 2, 0, 10);

                    ContentBackground.Margin = new Thickness(0, 0, 20, 0);

                    AnimationPath = AnimationPathEnum.FromRightToLeft;
                    break;
            }
        }

        private bool IsUserVisible(FrameworkElement element, FrameworkElement container)
        {
            if (!element.IsVisible)
            {
                return false;
            }
            var bounds = element.TransformToAncestor(container).TransformBounds(new Rect(0.0, 0.0, element.ActualWidth, element.ActualHeight));
            var rect = new Rect(0.0, 0.0, container.ActualWidth, container.ActualHeight);
            return rect.Contains(bounds.TopLeft) || rect.Contains(bounds.BottomRight);
        }

        private void svScroller_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            if (IsActive && (svScroller.ScrollableHeight > 0))
            {
                (App.Current.MainWindow as GridView).PopupText.Hide();
                foreach (var elem in wpContent.Children)
                {
                    if ((elem is GridIconControl) && !(elem as GridIconControl).IsRendered && IsUserVisible((elem as GridIconControl), svScroller))
                    {
                        (elem as GridIconControl).Render();
                    }
                }
            }
        }

        private void Window_Deactivated(object sender, EventArgs e)
        {
            if (!IsClosing)
            {
                DockIcon.IconName = Settings.Icon;
                Close();
            }
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            IsClosing = true;

            PopupText.Hide();

            var bmp = new RenderTargetBitmap((int)Width, (int)Height, 96, 96, PixelFormats.Pbgra32);
            bmp.Render(this);

            ImageAnimation.Width = Width;
            ImageAnimation.Height = Height;
            ImageAnimation.Opacity = 1;
            ImageAnimation.Source = bmp;
            ImageAnimation.Visibility = Visibility.Visible;
            ContentBackground.Visibility = Visibility.Hidden;

            var delay = Config.HideDelay;
            if ((System.Windows.Forms.Control.ModifierKeys & System.Windows.Forms.Keys.Shift) == System.Windows.Forms.Keys.Shift)
            {
                delay = 5000;
            }

            var tickcount = System.Environment.TickCount;
            for (; ; )
            {
                var k = 1 - (System.Environment.TickCount - tickcount) / (double)delay;
                if (k < 0)
                {
                    k = 0;
                }
                k = Math.Sin(k * Math.PI / 2);

                switch (AnimationPath)
                {
                    case AnimationPathEnum.FromLeftToRight:
                    case AnimationPathEnum.FromRightToLeft:
                        ImageAnimation.Margin = new Thickness(0, OffsetY * 2 * (1 - k), 0, 0);
                        break;

                    case AnimationPathEnum.FromBottomToTop:
                    case AnimationPathEnum.FromTopToBottom:
                        ImageAnimation.Margin = new Thickness(OffsetX * 2 * (1 - k), 0, 0, 0);
                        break;
                }

                ImageAnimation.Width = Width * k;
                ImageAnimation.Height = Height * k;
                ImageAnimation.Opacity = k;

                App.DoEvents();

                if (k == 0)
                {
                    break;
                }
            }

            base.OnClosing(e);
        }

        private void Window_Activated(object sender, EventArgs e)
        {
            if (!IsOpened)
            {
                IsOpened = true;
                DockIcon.IconName = App.StartupPath + "Container-Opened.png";

                var bmp = new RenderTargetBitmap((int)Width, (int)Height, 96, 96, PixelFormats.Pbgra32);
                bmp.Render(this);

                ImageAnimation.Width = 0;
                ImageAnimation.Height = 0;
                ImageAnimation.Opacity = 0;
                ImageAnimation.Source = bmp;
                ImageAnimation.Visibility = Visibility.Visible;
                ContentBackground.Visibility = Visibility.Hidden;

                var delay = Config.PopupDelay;
                if ((System.Windows.Forms.Control.ModifierKeys & System.Windows.Forms.Keys.Shift) == System.Windows.Forms.Keys.Shift)
                {
                    delay = 5000;
                }

                switch (AnimationPath)
                {
                    case AnimationPathEnum.FromBottomToTop:
                        ImageAnimation.HorizontalAlignment = HorizontalAlignment.Center;
                        ImageAnimation.VerticalAlignment = VerticalAlignment.Bottom;
                        ImageAnimation.Margin = new Thickness(OffsetX * 2, 0, 0, 0);
                        break;

                    case AnimationPathEnum.FromLeftToRight:
                        ImageAnimation.HorizontalAlignment = HorizontalAlignment.Left;
                        ImageAnimation.VerticalAlignment = VerticalAlignment.Center;
                        ImageAnimation.Margin = new Thickness(0, OffsetY * 2, 0, 0);
                        break;

                    case AnimationPathEnum.FromRightToLeft:
                        ImageAnimation.HorizontalAlignment = HorizontalAlignment.Right;
                        ImageAnimation.VerticalAlignment = VerticalAlignment.Center;
                        ImageAnimation.Margin = new Thickness(0, OffsetY * 2, 0, 0);
                        break;

                    case AnimationPathEnum.FromTopToBottom:
                        ImageAnimation.HorizontalAlignment = HorizontalAlignment.Center;
                        ImageAnimation.VerticalAlignment = VerticalAlignment.Top;
                        ImageAnimation.Margin = new Thickness(OffsetX * 2, 0, 0, 0);
                        break;
                }

                var tickcount = System.Environment.TickCount;
                for (; ; )
                {
                    var k = (System.Environment.TickCount - tickcount) / (double)delay;
                    if (k > 1)
                    {
                        k = 1;
                    }
                    k = Math.Sin(k * Math.PI / 2);

                    switch (AnimationPath)
                    {
                        case AnimationPathEnum.FromLeftToRight:
                        case AnimationPathEnum.FromRightToLeft:
                            ImageAnimation.Margin = new Thickness(0, OffsetY * 2 * (1 - k), 0, 0);
                            break;

                        case AnimationPathEnum.FromBottomToTop:
                        case AnimationPathEnum.FromTopToBottom:
                            ImageAnimation.Margin = new Thickness(OffsetX * 2 * (1 - k), 0, 0, 0);
                            break;
                    }

                    ImageAnimation.Width = Width * k;
                    ImageAnimation.Height = Height * k;
                    ImageAnimation.Opacity = k;

                    App.DoEvents();

                    if (k == 1)
                    {
                        break;
                    }
                }

                ContentBackground.Visibility = Visibility.Visible;
                ImageAnimation.Visibility = Visibility.Hidden;
                ImageAnimation.Source = null;

                foreach (var elem in wpContent.Children)
                {
                    if ((elem is GridIconControl) && !(elem as GridIconControl).IsRendered && IsUserVisible((elem as GridIconControl), svScroller))
                    {
                        (elem as GridIconControl).Render();
                    }
                }
            }
            Activate();
        }

        private void btnBack_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var i = App.Path.LastIndexOf('\\');
                var path = App.Path;
                if (i >= 0)
                {
                    if (i == path.Length - 1)
                    {
                        path = path.Remove(i);
                        i = path.LastIndexOf('\\');
                        if (i >= 0)
                        {
                            path = path.Remove(i);
                        }
                    }
                    else
                    {
                        path = path.Remove(i);
                    }
                }
                var info = new ProcessStartInfo(Process.GetCurrentProcess().MainModule.FileName);
                info.Arguments = string.Format("-viewMode grid \"{0}\" -notSetPath", path);
                Process.Start(info);

                if (!IsClosing)
                {
                    Close();
                }
            }
            catch
            {
            }
        }

        private void btnOpen_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var info = new ProcessStartInfo(App.Path);
                Process.Start(info);
                if (!IsClosing)
                {
                    DockIcon.IconName = Settings.Icon;
                    Close();
                }
            }
            catch
            {
            }
        }

        private void GridContent_MouseLeave(object sender, MouseEventArgs e)
        {
            if (Config.HoverEnabled)
            {
                var anim = new DoubleAnimation(0, new Duration(TimeSpan.FromMilliseconds(Config.HoverHideDelay)));
                Hover.BeginAnimation(OpacityProperty, anim);
            }
        }
    }
}
