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
using System.IO;
using System.Windows.Media.Effects;
using System.Windows.Media.Animation;
using XWindowsDock;

namespace ContainerPublic
{
    /// <summary>
    /// Interaction logic for FanView.xaml
    /// </summary>
    public partial class FanView : Window
    {
        public bool IsClosing { get; private set; }
        public bool IsOpened { get; private set; }
        private bool AllowClose;

        public const int MaxItems = 8;
        private const int Angle = 9;
        private int IconSize;

        private DropShadowEffect ShadowEffect;
        private Storyboard PopupBoard;
        private Storyboard HideBoard;

        public FanView()
        {
            InitializeComponent();
        }

        protected override void OnInitialized(EventArgs e)
        {
            base.OnInitialized(e);

            var bounds = System.Windows.Forms.Screen.PrimaryScreen.Bounds;
            var monitorSize = Math.Min(bounds.Width, bounds.Height);
            IconSize = (int)(monitorSize * 0.09);

            ShadowEffect = new DropShadowEffect();
            ShadowEffect.Direction = -90;
            ShadowEffect.ShadowDepth = IconSize * 0.3;
            ShadowEffect.BlurRadius = IconSize * 0.3;
            ShadowEffect.Opacity = 0;

            spContent.Margin = new Thickness(0, 0, IconSize, ShadowEffect.ShadowDepth);
            //spContent.Margin = new Thickness(0, 0, IconSize, 0);
            spContent.Children.Clear();

            foreach (var file in App.Files)
            {
                var attr = File.GetAttributes(file);
                if ((attr & (FileAttributes.Hidden | FileAttributes.System)) == 0)
                {
                    if (spContent.Children.Count < MaxItems)
                    {
                        var ctrl = new FanIconControl();
                        ctrl.IconSize = IconSize;
                        ctrl.Filename = file;
                        ctrl.Name = "Icon" + Convert.ToString(spContent.Children.Count);
                        this.RegisterName(ctrl.Name, ctrl);
                        ctrl.GridContent.Tag = "GridContent" + Convert.ToString(spContent.Children.Count);
                        this.RegisterName((string)ctrl.GridContent.Tag, ctrl.GridContent);
                        ctrl.BorderTitle.Tag = "BorderTitle" + Convert.ToString(spContent.Children.Count);
                        this.RegisterName((string)ctrl.BorderTitle.Tag, ctrl.BorderTitle);
                        spContent.Children.Add(ctrl);
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if (spContent.Children.Count < MaxItems)
            {
                foreach (var dir in App.Directories)
                {
                    var attr = File.GetAttributes(dir);
                    if ((attr & (FileAttributes.Hidden | FileAttributes.System)) == 0)
                    {
                        if (spContent.Children.Count < MaxItems)
                        {
                            var ctrl = new FanIconControl();
                            ctrl.IconSize = IconSize;
                            ctrl.Filename = dir;
                            ctrl.Name = "Icon" + Convert.ToString(spContent.Children.Count);
                            this.RegisterName(ctrl.Name, ctrl);
                            ctrl.GridContent.Tag = "GridContent" + Convert.ToString(spContent.Children.Count);
                            this.RegisterName((string)ctrl.GridContent.Tag, ctrl.GridContent);
                            ctrl.BorderTitle.Tag = "BorderTitle" + Convert.ToString(spContent.Children.Count);
                            this.RegisterName((string)ctrl.BorderTitle.Tag, ctrl.BorderTitle);
                            spContent.Children.Add(ctrl);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            var rect = DockIcon.IconRect;

            Width = 350 + 5 + IconSize + spContent.Margin.Right + 10;
            Left = rect.Right - Width + spContent.Margin.Right;

            Height = IconSize * spContent.Children.Count + spContent.Margin.Top + spContent.Margin.Bottom + 350 * Math.Sin(Angle) / Math.Cos((90 - Angle));

            Top = rect.Bottom - Height + spContent.Margin.Bottom - 4;
        }

        private void PopupAnimation_Completed(object sender, EventArgs e)
        {
            if (HideBoard == null)
            {
                var board = new Storyboard();

                foreach (FanIconControl ctrl in spContent.Children)
                {
                    if (spContent.Children.IndexOf(ctrl) < spContent.Children.Count - 1)
                    {
                        ctrl.GridContent.Effect = ShadowEffect;

                        var anim = new DoubleAnimationUsingKeyFrames();
                        anim.SetValue(Storyboard.TargetNameProperty, ctrl.GridContent.Tag);
                        anim.SetValue(Storyboard.TargetPropertyProperty, new PropertyPath("(FrameworkElement.Effect).(Opacity)"));
                        anim.KeyFrames.Add(new SplineDoubleKeyFrame(0, KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(0))));
                        anim.KeyFrames.Add(new SplineDoubleKeyFrame(0.3, KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(800))));

                        board.Children.Add(anim);
                    }
                    if (!ctrl.IsRendered)
                    {
                        ctrl.Render();
                    }
                }

                board.Begin(this);

                Activate();
            }
        }

        private void Window_Activated(object sender, EventArgs e)
        {
            if (IsClosing || IsOpened)
            {
                return;
            }
            IsOpened = true;

            DockIcon.IconName = App.StartupPath + "Container-Opened.png";

            PopupBoard = new Storyboard();
            PopupBoard.Completed += new EventHandler(PopupAnimation_Completed);

            var rect = DockIcon.IconRect;

            var delay = Config.PopupDelay;
            if ((System.Windows.Forms.Control.ModifierKeys & System.Windows.Forms.Keys.Shift) == System.Windows.Forms.Keys.Shift)
            {
                delay = 5000;
            }

            for (var i = 0; i < spContent.Children.Count; i++)
            {
                var ctrl = spContent.Children[i] as FanIconControl;

                var k = (double)(spContent.Children.Count - 1 - i) / MaxItems;

                var animRotate = new DoubleAnimationUsingKeyFrames();
                animRotate.SetValue(Storyboard.TargetNameProperty, ctrl.Name);
                animRotate.SetValue(Storyboard.TargetPropertyProperty, new PropertyPath("(UIElement.RenderTransform).(TransformGroup.Children)[0].(RotateTransform.Angle)"));
                animRotate.KeyFrames.Add(new SplineDoubleKeyFrame(k * Angle, KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(delay))));
                animRotate.DecelerationRatio = 0.8;

                var animMargin = new ThicknessAnimationUsingKeyFrames();
                animMargin.SetValue(Storyboard.TargetNameProperty, ctrl.Name);
                animMargin.SetValue(Storyboard.TargetPropertyProperty, new PropertyPath("(FrameworkElement.Margin)"));
                animMargin.KeyFrames.Add(new SplineThicknessKeyFrame(
                    new Thickness(0, 0, -IconSize / 2 * (1 - Math.Sin((1 - k) * Math.PI / 2)) - (IconSize - rect.Width) / 2,
                        rect.Height + IconSize * (spContent.Children.Count - 1 - i)),
                    KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(delay))));
                animMargin.DecelerationRatio = 0.8;

                var animOpacity = new DoubleAnimationUsingKeyFrames();
                animOpacity.SetValue(Storyboard.TargetNameProperty, ctrl.BorderTitle.Tag);
                animOpacity.SetValue(Storyboard.TargetPropertyProperty, new PropertyPath("(FrameworkElement.Opacity)"));
                animOpacity.KeyFrames.Add(new SplineDoubleKeyFrame(1, KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(delay))));
                animOpacity.DecelerationRatio = 1;

                var animIconSize = new Int32AnimationUsingKeyFrames();
                animIconSize.SetValue(Storyboard.TargetNameProperty, ctrl.Name);
                animIconSize.SetValue(Storyboard.TargetPropertyProperty, new PropertyPath("(FrameworkElement.IconSize)"));
                animIconSize.KeyFrames.Add(new SplineInt32KeyFrame(rect.Width, KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(0))));
                animIconSize.KeyFrames.Add(new SplineInt32KeyFrame(IconSize, KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(delay))));
                animIconSize.DecelerationRatio = 1;

                PopupBoard.Children.Add(animRotate);
                PopupBoard.Children.Add(animMargin);
                PopupBoard.Children.Add(animOpacity);
                PopupBoard.Children.Add(animIconSize);
            }

            PopupBoard.Begin(this);
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            if (IsClosing)
            {
                e.Cancel = !AllowClose;
            }
            else
            {
                IsClosing = true;
                AllowClose = false;

                e.Cancel = true;

                if (PopupBoard is Storyboard)
                {
                    PopupBoard.Stop(this);
                    PopupBoard = null;
                }
                HideBoard = new Storyboard();
                HideBoard.Completed += new EventHandler((od, ed) =>
                {
                    AllowClose = true;
                    Close();
                });

                var rect = DockIcon.IconRect;

                var delay = Config.HideDelay;
                if ((System.Windows.Forms.Control.ModifierKeys & System.Windows.Forms.Keys.Shift) == System.Windows.Forms.Keys.Shift)
                {
                    delay = 5000;
                }

                for (var i = 0; i < spContent.Children.Count; i++)
                {
                    var ctrl = spContent.Children[i] as FanIconControl;

                    ctrl.GridContent.Effect = null;

                    var animRotate = new DoubleAnimationUsingKeyFrames();
                    animRotate.SetValue(Storyboard.TargetNameProperty, ctrl.Name);
                    animRotate.SetValue(Storyboard.TargetPropertyProperty, new PropertyPath("(UIElement.RenderTransform).(TransformGroup.Children)[0].(RotateTransform.Angle)"));
                    animRotate.KeyFrames.Add(new SplineDoubleKeyFrame(0, KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(delay))));
                    animRotate.DecelerationRatio = 0.8;

                    var animMargin = new ThicknessAnimationUsingKeyFrames();
                    animMargin.SetValue(Storyboard.TargetNameProperty, ctrl.Name);
                    animMargin.SetValue(Storyboard.TargetPropertyProperty, new PropertyPath("(FrameworkElement.Margin)"));
                    animMargin.KeyFrames.Add(new SplineThicknessKeyFrame(
                        new Thickness(0),
                        KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(delay))));
                    animMargin.DecelerationRatio = 0.8;

                    var animOpacity = new DoubleAnimationUsingKeyFrames();
                    animOpacity.SetValue(Storyboard.TargetNameProperty, ctrl.BorderTitle.Tag);
                    animOpacity.SetValue(Storyboard.TargetPropertyProperty, new PropertyPath("(FrameworkElement.Opacity)"));
                    animOpacity.KeyFrames.Add(new SplineDoubleKeyFrame(0, KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(delay))));
                    animOpacity.DecelerationRatio = 1;

                    var animIconSize = new Int32AnimationUsingKeyFrames();
                    animIconSize.SetValue(Storyboard.TargetNameProperty, ctrl.Name);
                    animIconSize.SetValue(Storyboard.TargetPropertyProperty, new PropertyPath("(FrameworkElement.IconSize)"));
                    animIconSize.KeyFrames.Add(new SplineInt32KeyFrame(rect.Width, KeyTime.FromTimeSpan(TimeSpan.FromMilliseconds(delay))));
                    animIconSize.DecelerationRatio = 1;

                    HideBoard.Children.Add(animRotate);
                    HideBoard.Children.Add(animMargin);
                    HideBoard.Children.Add(animOpacity);
                    HideBoard.Children.Add(animIconSize);
                }

                HideBoard.Begin(this);
            }
            base.OnClosing(e);
        }

        private void Window_Deactivated(object sender, EventArgs e)
        {
            if (!IsClosing)
            {
                DockIcon.IconName = Settings.Icon;
                Close();
            }
        }
    }
}
