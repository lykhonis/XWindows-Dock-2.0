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
using System.Threading;
using System.Windows.Media.Animation;
using System.ComponentModel;
using System.Collections;
using System.Diagnostics;
using System.IO;
using XWindowsDock;

namespace ContainerPublic
{
    /// <summary>
    /// Interaction logic for IconControl.xaml
    /// </summary>
    public partial class GridIconControl : UserControl
    {
        public enum ContentTypeEnum { Icon, Image, Video }

        public ContentTypeEnum ContentType { get; private set; }

        private double VideoDelta;

        private string filename;
        public string Filename
        {
            get
            {
                return filename;
            }
            set
            {
                filename = value;
                if (string.IsNullOrEmpty(Title))
                {
                    Title = Filename.Remove(0, Filename.LastIndexOf('\\') + 1);
                }
            }
        }

        public string Title
        {
            get
            {
                return lbTitle.Text;
            }
            set
            {
                lbTitle.Text = value;
            }
        }

        public bool IsRendered { get; private set; }

        public int IconSize
        {
            get
            {
                return (int)Image.Width;
            }
            set
            {
                Image.Width = value;
                Image.MaxHeight = value;

                Width = IconSize + 16 * 2;
                Height = IconSize + 20;
            }
        }

        private static Queue Queue = new Queue();
        private static Thread RenderThread;

        public GridIconControl()
        {
            InitializeComponent();

            if (RenderThread == null)
            {
                RenderThread = new Thread(new ThreadStart(RenderInProcess));
                RenderThread.IsBackground = true;
                RenderThread.Start();
            }
        }

        public void Render()
        {
            if (!Queue.Contains(this))
            {
                Queue.Enqueue(this);
            }
        }

        private static void RenderInProcess()
        {
            while (RenderThread.IsAlive)
            {
                if (Queue.Count > 0)
                {
                    var item = Queue.Dequeue() as GridIconControl;
                    if (!item.IsRendered)
                    {
                        item.IsRendered = true;
                        BitmapImage bmp = null;
                        try
                        {
                            var ext = string.Empty;
                            var i = item.Filename.LastIndexOf('.');
                            if (i >= 0)
                            {
                                ext = item.Filename.Substring(i).ToLower();
                            }
                            if ((ext != ".ico") && (ext != ".icon"))
                            {
                                bmp = new BitmapImage(new Uri(item.Filename, UriKind.Absolute));
                                bmp.Freeze();

                                item.ContentType = ContentTypeEnum.Image;

                                item.Dispatcher.BeginInvoke(new RenderCompletedDelegate(item.RenderCompleted), bmp);
                                Thread.Sleep(150);
                            }
                        }
                        catch
                        {
                        }
                        if (bmp == null)
                        {
                            try
                            {
                                item.VideoDelta = 0.5;

                                bmp = Video.GetVideoFrame(item.Filename, item.VideoDelta);
                                bmp.Freeze();

                                item.ContentType = ContentTypeEnum.Video;

                                item.Dispatcher.BeginInvoke(new RenderCompletedDelegate(item.RenderCompleted), bmp);
                                Thread.Sleep(150);
                            }
                            catch
                            {
                            }
                        }
                        if (bmp == null)
                        {
                            try
                            {
                                bmp = Utils.IconExtractor.IconToBitmap(Utils.IconExtractor.GetIcon(item.Filename));
                                bmp.Freeze();

                                item.ContentType = ContentTypeEnum.Icon;

                                item.Dispatcher.BeginInvoke(new RenderCompletedDelegate(item.RenderCompleted), bmp);
                            }
                            catch
                            {
                            }
                        }
                    }
                    else
                    {
                        switch (item.ContentType)
                        {
                            case ContentTypeEnum.Video:
                                try
                                {
                                    var bmp = Video.GetVideoFrame(item.Filename, item.VideoDelta);
                                    bmp.Freeze();

                                    item.Dispatcher.BeginInvoke(new RenderCompletedDelegate(item.RenderVideoNewFrameCompleted), bmp);
                                    Thread.Sleep(150);
                                }
                                catch
                                {
                                }
                                break;

                            default:
                                Thread.Sleep(10);
                                break;
                        }
                    }
                }
                else
                {
                    Thread.Sleep(10);
                }
            }
        }

        private delegate void RenderCompletedDelegate(BitmapImage Bitmap);
        private void RenderCompleted(BitmapImage Bitmap)
        {
            Image.Source = null;
            Image.Source = Bitmap;

            switch (ContentType)
            {
                case ContentTypeEnum.Video:
                    VideoCover.Visibility = Visibility.Visible;
                    break;
            }

            (Resources["ShowImageAnimation"] as Storyboard).Begin();
        }

        private void RenderVideoNewFrameCompleted(BitmapImage Bitmap)
        {
            ImageFade.BeginAnimation(Image.SourceProperty, null);
            ImageFade.Source = Image.Source;

            (Resources["FadeImageAnimation"] as Storyboard).Begin();
            Image.Source = Bitmap;
        }

        private void UserControl_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            Image.BeginAnimation(OpacityProperty, null);
            Image.Effect = new FadeInEffect() { Delta = 0.6 };

            CaptureMouse();
        }

        private void UserControl_PreviewMouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            Image.BeginAnimation(OpacityProperty, null);
            Image.Effect = null;

            ReleaseMouseCapture();

            if (IsMouseOver || ((App.Current.MainWindow as GridView).PopupText.IsMouseOver && ((App.Current.MainWindow as GridView).PopupText.IconControl == this)))
            {
                try
                {
                    var info = new ProcessStartInfo(Filename);
                    if (Directory.Exists(Filename) &&
                        ((System.Windows.Forms.Control.ModifierKeys & System.Windows.Forms.Keys.Control) != System.Windows.Forms.Keys.Control))
                    {
                        info.FileName = Process.GetCurrentProcess().MainModule.FileName;
                        info.Arguments = string.Format("-viewMode grid \"{0}\" -notSetPath", Filename);
                    }
                    Process.Start(info);

                    if (!(App.Current.MainWindow as GridView).IsClosing)
                    {
                        DockIcon.IconName = Settings.Icon;
                        App.Current.MainWindow.Close();
                    }
                }
                catch
                {
                }
            }
        }

        private void UserControl_PreviewMouseRightButtonUp(object sender, MouseButtonEventArgs e)
        {
            switch (ContentType)
            {
                case ContentTypeEnum.Video:
                    VideoDelta += 0.025;
                    if (VideoDelta >= 1 - 0.025)
                    {
                        VideoDelta = 0.025;
                    }
                    Render();
                    break;
            }
        }

        private void UserControl_MouseEnter(object sender, MouseEventArgs e)
        {
            var size = App.MeasureTextSize(Title, lbTitle.FontFamily, lbTitle.FontStyle, lbTitle.FontWeight, lbTitle.FontStretch, lbTitle.FontSize);
            if (size.Width > lbTitle.ActualWidth)
            {
                (App.Current.MainWindow as GridView).PopupText.Popup(this);
            }
            (App.Current.MainWindow as GridView).MoveHover(this);
        }

        private void UserControl_MouseLeave(object sender, MouseEventArgs e)
        {
            var bounds = TransformToAncestor(Parent as UIElement).TransformBounds(new Rect(0, 0, ActualWidth, ActualHeight));
            var pt = e.GetPosition(Parent as UIElement);
            if (!bounds.Contains(pt))
            {
                (App.Current.MainWindow as GridView).PopupText.Hide();
            }
        }
    }
}
