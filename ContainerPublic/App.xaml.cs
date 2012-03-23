using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;
using System.Windows.Threading;
using System.IO;
using XWindowsDock;
using System.Runtime.InteropServices;
using System.Windows.Media;

namespace ContainerPublic
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        #region SH File Operations

        public enum FO_Func : uint
        {
            FO_MOVE = 0x0001,
            FO_COPY = 0x0002,
        }

        [StructLayout(LayoutKind.Sequential)]
        struct SHFileOpStruct
        {
            public IntPtr hwnd;
            public FO_Func wFunc;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string pFrom;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string pTo;
            public ushort fFlags;
            public Int32 fAnyOperationsAborted;
            public IntPtr hNameMappings;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string lpszProgressTitle;
        }

        [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
        private static extern int SHFileOperation(ref SHFileOpStruct lpFileOp);

        private string StringArrayToMultiString(string[] Strings)
        {
            var result = string.Empty;
            foreach (var str in Strings)
            {
                result += str + '\0';
            }
            return result + '\0';
        }

        #endregion

        public enum ViewModeEnum { Grid, Fan, List }

        public static ViewModeEnum ViewMode;
        public static string Path;
        public static string StartupPath;
        public static string[] Directories;
        public static string[] Files;

        public static Size MeasureTextSize(string text, FontFamily fontFamily, FontStyle fontStyle, FontWeight fontWeight, FontStretch fontStretch, double fontSize)
        {
            var ft = new FormattedText(text, System.Globalization.CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                new Typeface(fontFamily, fontStyle, fontWeight, fontStretch), fontSize, Brushes.Black);
            return new Size(ft.Width, ft.Height);
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

            StartupPath = System.Windows.Forms.Application.StartupPath + '\\';

            DockIcon.Register(StartupPath + "Container-Empty.png", null, false);
            if (!DockIcon.IsRegistered)
            {
                Shutdown();
                return;
            }

            DockIcon.KeepInDock = true;
            DockIcon.Exposable = false;
            DockIcon.Activatable = true;

            Settings.Initialize(DockIcon.SettingsPath + "settings.xml");

            #region Get Previous Icon

            var iconName = DockIcon.IconName;
            if (string.IsNullOrEmpty(iconName))
            {
                iconName = StartupPath + "Container-Empty.png";
            }
            if (string.IsNullOrEmpty(Settings.Icon) || ((iconName != StartupPath + "Container-Empty.png") && (iconName != StartupPath + "Container-Opened.png")))
            {
                Settings.Icon = iconName;
                Settings.Save();
            }

            #endregion

            #region Init Path

            Path = Settings.Path;
            if (!Directory.Exists(Path))
            {
                Path = StartupPath;
            }
            if (Path.LastIndexOf('\\') < Path.Length - 1)
            {
                Path += '\\';
            }

            #endregion

            #region Start With View Mode

            if ((e.Args.Length >= 3) && (e.Args[0] == "-viewMode"))
            {
                Path = e.Args[2];
                if (Path.LastIndexOf('\\') < Path.Length - 1)
                {
                    Path += '\\';
                }
                if (!((e.Args.Length == 4) && (e.Args[3] == "-notSetPath")))
                {
                    Settings.Path = Path;
                }

                Directories = Directory.GetDirectories(Path);
                Files = Directory.GetFiles(Path);

                DockIcon.Title = null;
                DockIcon.AddFolderWatcher(1, DockIcon.FolderWatcherActions.FileAdded | DockIcon.FolderWatcherActions.FileModified | DockIcon.FolderWatcherActions.FolderAdded, Path);

                switch (e.Args[1])
                {
                    case "fan":
                        ViewMode = ViewModeEnum.Fan;
                        MainWindow = new ContainerPublic.FanView();
                        break;

                    case "list":
                        ViewMode = ViewModeEnum.List;
                        Shutdown();
                        return;
                    //break;

                    default:
                        ViewMode = ViewModeEnum.Grid;
                        MainWindow = new ContainerPublic.GridView();
                        break;
                }

                MainWindow.Show();
                return;
            }

            #endregion

            #region Dropped files & directories into the stack

            if ((e.Args.Length > 0) && !((e.Args.Length == 1) && Directory.Exists(e.Args[0])))
            {
                var droppedFiles = true;
                foreach (var file in e.Args)
                {
                    if (!File.Exists(file) && !Directory.Exists(file))
                    {
                        droppedFiles = false;
                        break;
                    }
                }
                if (droppedFiles)
                {
                    var fileOperation = new SHFileOpStruct();
                    fileOperation.hwnd = IntPtr.Zero;

                    fileOperation.wFunc = FO_Func.FO_MOVE;
                    if ((System.Windows.Forms.Control.ModifierKeys & System.Windows.Forms.Keys.Control) == System.Windows.Forms.Keys.Control)
                    {
                        fileOperation.wFunc = FO_Func.FO_COPY;
                    }

                    fileOperation.pFrom = StringArrayToMultiString(e.Args);
                    fileOperation.pTo = StringArrayToMultiString(new string[] { Path });
                    fileOperation.hNameMappings = IntPtr.Zero;
                    fileOperation.fAnyOperationsAborted = 0;
                    fileOperation.fFlags = 0;
                    fileOperation.lpszProgressTitle = null;

                    SHFileOperation(ref fileOperation);

                    Shutdown();
                    return;
                }
            }

            #endregion

            #region Start With Directory

            if ((e.Args.Length > 0) && Directory.Exists(e.Args[0]))
            {
                Path = e.Args[0];
                if (Path.LastIndexOf('\\') < Path.Length - 1)
                {
                    Path += '\\';
                }
                if (!((e.Args.Length == 2) && (e.Args[1] == "-notSetPath")))
                {
                    Settings.Path = Path;
                }
            }

            #endregion

            #region Default

            Directories = Directory.GetDirectories(Path);
            Files = Directory.GetFiles(Path);

            if (Directories.Length + Files.Length <= FanView.MaxItems)
            {
                ViewMode = ViewModeEnum.Fan;
            }
            else if (Directories.Length + Files.Length <= GridView.MaxItems)
            {
                ViewMode = ViewModeEnum.Grid;
            }
            else
            {
                //ViewMode = ViewModeEnum.List;
                ViewMode = ViewModeEnum.Grid;
            }

            DockIcon.Title = null;
            DockIcon.AddFolderWatcher(1, DockIcon.FolderWatcherActions.FileAdded | DockIcon.FolderWatcherActions.FileModified | DockIcon.FolderWatcherActions.FolderAdded, Path);

            switch (ViewMode)
            {
                case ViewModeEnum.Fan:
                    MainWindow = new ContainerPublic.FanView();
                    break;

                case ViewModeEnum.List:
                    Shutdown();
                    return;
                    //break;

                default:
                    MainWindow = new ContainerPublic.GridView();
                    break;
            }

            MainWindow.Show();

            #endregion
        }

        protected override void OnExit(ExitEventArgs e)
        {
            if (DockIcon.IsRegistered)
            {
                Settings.Save();
            }

            //Config.Save();

            // Update title
            var i = Settings.Path.LastIndexOf('\\');
            var title = Settings.Path;
            if (i >= 0)
            {
                if (i == title.Length - 1)
                {
                    title = title.Remove(i);
                    i = title.LastIndexOf('\\');
                    if (i >= 0)
                    {
                        title = title.Remove(0, i + 1);
                    }
                }
                else
                {
                    title = title.Remove(0, i + 1);
                }
            }
            DockIcon.Title = title;

            base.OnExit(e);
        }

        #region Do Events

        private static DispatcherOperationCallback exitFrameCallback = new System.Windows.Threading.DispatcherOperationCallback(ExitFrame);

        public static void DoEvents()
        {
            var nestedFrame = new DispatcherFrame();
            var exitOperation = Dispatcher.CurrentDispatcher.BeginInvoke(DispatcherPriority.Background, exitFrameCallback, nestedFrame);
            Dispatcher.PushFrame(nestedFrame);
            if (exitOperation.Status != DispatcherOperationStatus.Completed)
            {
                exitOperation.Abort();
            }
        }

        private static Object ExitFrame(Object state)
        {
            var frame = state as DispatcherFrame;
            frame.Continue = false;
            return null;
        }

        #endregion
    }
}
