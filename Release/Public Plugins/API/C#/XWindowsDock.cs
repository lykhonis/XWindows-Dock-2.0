using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Diagnostics;
using System.Drawing;
namespace XWindowsDock
{
    public class DockIcon
    {
        private class NativeMethods
        {
            [DllImport("user32.dll")]
            public static extern IntPtr SendMessage(IntPtr hWnd, int Msg, IntPtr wParam, IntPtr lParam);

            [DllImport("user32.dll")]
            public static extern uint RegisterWindowMessage(string lpString);

            [DllImport("user32.dll")]
            public static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

            [DllImport("user32.dll")]
            public static extern IntPtr GetProp(IntPtr hWnd, string lpString);

            public const int WM_COPYDATA = 0x4A;

            [StructLayout(LayoutKind.Sequential)]
            public struct CopyDataStruct
            {
                public uint dwData;
                public int cbData;
                public IntPtr data;
            }
        }

        private class Parameters : IDisposable
        {
            private NativeMethods.CopyDataStruct data;

            private IntPtr pointer = IntPtr.Zero;
            public IntPtr Pointer
            {
                get
                {
                    if (pointer == IntPtr.Zero)
                    {
                        var dataSize = Marshal.SizeOf(typeof(NativeMethods.CopyDataStruct));
                        pointer = Marshal.AllocHGlobal(dataSize + data.cbData);
                        Marshal.StructureToPtr(data, pointer, false);
                        for (var i = 0; i < data.cbData; i++)
                        {
                            Marshal.WriteByte(pointer, dataSize + i, Marshal.ReadByte(data.data, i));
                        }
                    }
                    return pointer;
                }
            }

            private int Offset;

            public static uint Identificator
            {
                get
                {
                    return NativeMethods.RegisterWindowMessage("WM_XWDPUBLICAPICALL");
                }
            }

            public bool IsReady
            {
                get
                {
                    return data.dwData == Identificator;
                }
            }

            public Parameters()
            {
                data = new NativeMethods.CopyDataStruct();
                data.dwData = Identificator;
                data.cbData = 0;
                data.data = IntPtr.Zero;
            }

            public Parameters(IntPtr lParam)
            {
                data = (NativeMethods.CopyDataStruct)Marshal.PtrToStructure(lParam, typeof(NativeMethods.CopyDataStruct));
            }

            public Parameters AddString(string Value)
            {
                var offset = data.cbData;
                var length = 2 * 256;

                data.cbData += length;
                if (data.data == IntPtr.Zero)
                {
                    data.data = Marshal.AllocHGlobal(data.cbData);
                }
                else
                {
                    data.data = Marshal.ReAllocHGlobal(data.data, (IntPtr)data.cbData);
                }

                var ptrLength = 0;
                if (!string.IsNullOrEmpty(Value))
                {
                    var ptr = Encoding.Unicode.GetBytes(Value);
                    Marshal.Copy(ptr, 0, (IntPtr)(data.data.ToInt32() + offset), ptr.Length);
                    ptrLength = ptr.Length;
                }

                // fill in by 0x0
                var pointer = (IntPtr)(data.data.ToInt32() + offset + ptrLength);
                for (var i = 0; i < length - ptrLength; i++)
                {
                    Marshal.WriteByte(pointer, i, 0);
                }
                return this;
            }

            public Parameters AddInt(int Value)
            {
                var offset = data.cbData;
                var length = 4;

                data.cbData += length;
                if (data.data == IntPtr.Zero)
                {
                    data.data = Marshal.AllocHGlobal(data.cbData);
                }
                else
                {
                    data.data = Marshal.ReAllocHGlobal(data.data, (IntPtr)data.cbData);
                }
                Marshal.WriteInt32(data.data, offset, Value);
                return this;
            }

            public Parameters AddStructure(object structure)
            {
                var offset = data.cbData;
                var length = Marshal.SizeOf(structure);

                data.cbData += length;
                if (data.data == IntPtr.Zero)
                {
                    data.data = Marshal.AllocHGlobal(data.cbData);
                }
                else
                {
                    data.data = Marshal.ReAllocHGlobal(data.data, (IntPtr)data.cbData);
                }

                var ptr = Marshal.AllocHGlobal(length);
                Marshal.StructureToPtr(structure, ptr, false);

                for (var i = 0; i < length; i++)
                {
                    Marshal.WriteByte(data.data, offset + i, Marshal.ReadByte(ptr, i));
                }

                Marshal.FreeHGlobal(ptr);
                return this;
            }

            public string PopString()
            {
                var length = 256 * 2;
                var result = string.Empty;
                if (Offset <= data.cbData - length)
                {
                    var ptr = new byte[length];
                    for (var i = 0; i < length; i += 2)
                    {
                        ptr[i] = Marshal.ReadByte(data.data, Offset + i);
                        ptr[i + 1] = Marshal.ReadByte(data.data, Offset + i + 1);
                    }
                    result = Encoding.Unicode.GetString(ptr);
                    result = result.Remove(result.IndexOf('\0'));
                    Offset += length;
                }
                return result;
            }

            public int PopInt()
            {
                var length = 4;
                var result = 0;
                if (Offset <= data.cbData - length)
                {
                    result = Marshal.ReadInt32(data.data, Offset);
                    Offset += length;
                }
                return result;
            }

            public object PopStructure(Type type)
            {
                var length = Marshal.SizeOf(type);
                object result = null;
                if (Offset <= data.cbData - length)
                {
                    var ptr = new byte[length];
                    for (var i = 0; i < length; i += 2)
                    {
                        ptr[i] = Marshal.ReadByte(data.data, Offset + i);
                        ptr[i + 1] = Marshal.ReadByte(data.data, Offset + i + 1);
                    }

                    var buffer = Marshal.AllocHGlobal(length);
                    Marshal.Copy(ptr, 0, buffer, length);
                    result = Marshal.PtrToStructure(buffer, type);
                    Marshal.FreeHGlobal(buffer);

                    Offset += length;
                }
                return result;
            }

            public void DeallocPointer()
            {
                if (pointer != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(pointer);
                    pointer = IntPtr.Zero;
                }
            }

            public void Dispose()
            {
                DeallocPointer();
                if (data.data != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(data.data);
                    data.data = IntPtr.Zero;
                }
            }
        }

        public enum BounceType { Normal = 0, Attention }
        private enum Function
        {
            Register = 1, Remove, SetConfig, GetConfig, SetIcon, GetIcon, SetTitle, GetTitle, Bounce, BounceStop,
            SetIndicator, GetIndicator, GetSettingsPath, GetUId, SetNotification, GetDockEdge, GetIconRect,
            AddFolderWatcher, RemoveFolderWatcher, GetEvent, GetMenu
        }
        private enum Event
        {
            LButtonClick = 0, MenuSelect
        }
        public enum NotificationPosition { LeftTop = 0, TopMiddle, RightTop, RightMiddle, RightBottom, BottomMiddle, LeftBottom, LeftMiddle }
        public enum DockEdgeEnum { Left = 0, Top, Right, Bottom }
        public enum FolderWatcherActions : int
        {
            FileNone = 0,
            FileAdded = 1 << 0,
            FileRemoved = 1 << 1,
            FileModified = 1 << 2,
            FileRenamedOldName = 1 << 3,
            FileRenamedNewName = 1 << 4,
            FolderAdded = 1 << 5,
            FolderRemoved = 1 << 6,
            FolderModified = 1 << 7,
            FolderRenamedOldName = 1 << 8,
            FolderRenamedNewName = 1 << 9
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct Rect
        {
            public int left;
            public int top;
            public int width;
            public int height;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct Menu
        {
            public int id;
            public int parentId;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string text;
            public int isCheckbox;
            public int isChecked;
        }

        private static IntPtr Id;
        private static HwndSource HwndSource;
        private static IntPtr UId;
        private static string LastStringResult;
        private static Rectangle LastRectangleResult;
        private static string iconName;
        private static string title;
        private static bool indicator;
        private static bool keepInDock;
        private static bool activatable;
        private static bool exposable;
        private static bool bounceable;

        public static bool IsRegistered
        {
            get
            {
                return Id != IntPtr.Zero;
            }
        }

        public static EventHandler LeftButtonClick;

        public delegate void EventGetMenu(List<Menu> Menu);
        public static EventGetMenu GetMenu;

        public delegate void EventMenuSelect(int nId);
        public static EventMenuSelect MenuSelect;

        private static void Initilize()
        {
            if (HwndSource == null)
            {
                HwndSource = new HwndSource(new HwndSourceParameters());
                HwndSource.AddHook(new HwndSourceHook(HwndSourceHook));
            }
        }

        private static IntPtr HwndSourceHook(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            if (msg == NativeMethods.WM_COPYDATA)
            {
                var paramaters = new Parameters(lParam);
                if (paramaters.IsReady)
                {
                    var function = (Function)paramaters.PopInt();
                    switch (function)
                    {
                        case Function.GetTitle:
                        case Function.GetIcon:
                        case Function.GetSettingsPath:
                            LastStringResult = paramaters.PopString();
                            break;

                        case Function.GetIconRect:
                            var rect = (Rect)paramaters.PopStructure(typeof(Rect));
                            LastRectangleResult = new Rectangle(rect.left, rect.top, rect.width, rect.height);
                            break;

                        case Function.GetConfig:
                            keepInDock = paramaters.PopInt() != 0;
                            activatable = paramaters.PopInt() != 0;
                            exposable = paramaters.PopInt() != 0;
                            bounceable = paramaters.PopInt() != 0;
                            break;

                        case Function.GetEvent:
                            switch ((Event)paramaters.PopInt())
                            {
                                case Event.LButtonClick:
                                    OnEventLButtonClick();
                                    break;

                                case Event.MenuSelect:
                                    int nId = paramaters.PopInt();
                                    OnEventMenuSelect(nId);
                                    break;
                            }
                            break;

                        case Function.GetMenu:
                            OnEventGetMenu();
                            break;
                    }
                    handled = true;
                    return (IntPtr)1;
                }
            }

            if (msg == Parameters.Identificator)
            {
                if (wParam == (IntPtr)1)
                {
                    Id = IntPtr.Zero;
                    Register(iconName, title, bounceable);
                    SetConfig();
                    Indicator = indicator;

                    handled = true;
                    return (IntPtr)1;
                }
            }

            handled = false;
            return IntPtr.Zero;
        }

        #region API

        private static IntPtr APICall(Parameters parameters)
        {
            var handle = NativeMethods.FindWindow("XWindowsDockClass", null);
            var result = IntPtr.Zero;
            if (handle != IntPtr.Zero)
            {
                NativeMethods.SendMessage(handle, NativeMethods.WM_COPYDATA, HwndSource.Handle, parameters.Pointer);
                parameters.DeallocPointer();
                return NativeMethods.GetProp(HwndSource.Handle, "XWDResult");
            }
            return result;
        }

        public static void Register(string DefaultIcon, string Title, bool Bounceable)
        {
            if (!IsRegistered)
            {
                Initilize();

                iconName = DefaultIcon;
                title = Title;
                bounceable = Bounceable;

                using (var parameters = new Parameters())
                {
                    Id = APICall(parameters
                        .AddInt((int)Function.Register)
                        .AddString(Process.GetCurrentProcess().MainModule.FileName)
                        .AddInt(HwndSource.Handle.ToInt32())
                        .AddString(DefaultIcon)
                        .AddString(Title)
                        .AddInt((int)UId)
                        .AddInt(Bounceable ? 1 : 0));
                }

                using (var parameters = new Parameters())
                {
                    UId = APICall(parameters
                        .AddInt((int)Function.GetUId)
                        .AddInt((int)Id));
                }
            }
        }

        public static void Remove()
        {
            if (IsRegistered)
            {
                using (var parameters = new Parameters())
                {
                    APICall(parameters
                        .AddInt((int)Function.Remove)
                        .AddInt((int)Id));
                }
                Id = IntPtr.Zero;
                UId = IntPtr.Zero;
            }
        }

        public static string IconName
        {
            get
            {
                LastStringResult = string.Empty;
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        APICall(parameters
                            .AddInt((int)Function.GetIcon)
                            .AddInt((int)Id));
                    }
                }
                return LastStringResult;
            }
            set
            {
                iconName = value;
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        APICall(parameters
                            .AddInt((int)Function.SetIcon)
                            .AddInt((int)Id)
                            .AddString(value));
                    }
                }
            }
        }

        public static string Title
        {
            get
            {
                LastStringResult = string.Empty;
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        APICall(parameters
                            .AddInt((int)Function.GetTitle)
                            .AddInt((int)Id));
                    }
                }
                return LastStringResult;
            }
            set
            {
                title = value;
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        APICall(parameters
                            .AddInt((int)Function.SetTitle)
                            .AddInt((int)Id)
                            .AddString(value));
                    }
                }
            }
        }

        public static void Bounce(BounceType Type, int Count)
        {
            if (IsRegistered)
            {
                using (var parameters = new Parameters())
                {
                    APICall(parameters
                        .AddInt((int)Function.Bounce)
                        .AddInt((int)Id)
                        .AddInt((int)Type)
                        .AddInt((int)Count));
                }
            }
        }

        public static void BounceStop()
        {
            if (IsRegistered)
            {
                using (var parameters = new Parameters())
                {
                    APICall(parameters
                        .AddInt((int)Function.BounceStop)
                        .AddInt((int)Id));
                }
            }
        }

        public static bool Indicator
        {
            get
            {
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        var result = APICall(parameters
                            .AddInt((int)Function.GetTitle)
                            .AddInt((int)Id));
                        return result != IntPtr.Zero;
                    }
                }
                return indicator;
            }
            set
            {
                indicator = value;
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        APICall(parameters
                            .AddInt((int)Function.SetIndicator)
                            .AddInt((int)Id)
                            .AddInt((int)(value ? 1 : 0)));
                    }
                }
            }
        }

        public static string SettingsPath
        {
            get
            {
                LastStringResult = string.Empty;
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        APICall(parameters
                            .AddInt((int)Function.GetSettingsPath)
                            .AddInt((int)Id));
                    }

                }
                return LastStringResult;
            }
        }

        private static void SetConfig()
        {
            if (IsRegistered)
            {
                using (var parameters = new Parameters())
                {
                    APICall(parameters
                        .AddInt((int)Function.SetConfig)
                        .AddInt((int)Id)
                        .AddInt((int)(keepInDock ? 1 : 0))
                        .AddInt((int)(activatable ? 1 : 0))
                        .AddInt((int)(exposable ? 1 : 0))
                        .AddInt((int)(bounceable ? 1 : 0)));
                }
            }
        }

        private static void GetConfig()
        {
            if (IsRegistered)
            {
                using (var parameters = new Parameters())
                {
                    APICall(parameters
                        .AddInt((int)Function.GetConfig)
                        .AddInt((int)Id));
                }
            }
        }

        public static bool KeepInDock
        {
            get
            {

                GetConfig();
                return keepInDock;
            }
            set
            {
                keepInDock = value;
                SetConfig();
            }
        }

        public static bool Activatable
        {
            get
            {

                GetConfig();
                return activatable;
            }
            set
            {
                activatable = value;
                SetConfig();
            }
        }

        public static bool Exposable
        {
            get
            {

                GetConfig();
                return exposable;
            }
            set
            {
                exposable = value;
                SetConfig();
            }
        }

        public static bool Bounceable
        {
            get
            {

                GetConfig();
                return bounceable;
            }
            set
            {
                bounceable = value;
                SetConfig();
            }
        }

        public static void SetNotification(bool Visible, string Text, NotificationPosition Position)
        {
            if (IsRegistered)
            {
                using (var parameters = new Parameters())
                {
                    APICall(parameters
                        .AddInt((int)Function.SetNotification)
                        .AddInt((int)Id)
                        .AddInt((int)(Visible ? 1 : 0))
                        .AddString(Text)
                        .AddInt((int)Position));
                }
            }
        }

        public static void ShowNotification(string Text, NotificationPosition Position)
        {
            SetNotification(true, Text, Position);
        }

        public static void HideNotification(NotificationPosition Position)
        {
            SetNotification(false, null, Position);
        }

        public static DockEdgeEnum DockEdge
        {
            get
            {
                var edge = DockEdgeEnum.Left;
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        edge = (DockEdgeEnum)APICall(parameters
                            .AddInt((int)Function.GetDockEdge)
                            .AddInt((int)Id));
                    }
                }
                return edge;
            }
        }

        public static Rectangle IconRect
        {
            get
            {
                LastRectangleResult = Rectangle.Empty;
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        APICall(parameters
                            .AddInt((int)Function.GetIconRect)
                            .AddInt((int)Id));
                    }

                }
                return LastRectangleResult;
            }
        }

        public static void AddFolderWatcher(int FolderId, FolderWatcherActions Actions, string Folder)
        {
            if (IsRegistered)
            {
                using (var parameters = new Parameters())
                {
                    APICall(parameters
                        .AddInt((int)Function.AddFolderWatcher)
                        .AddInt((int)Id)
                        .AddInt(FolderId)
                        .AddInt((int)Actions)
                        .AddString(Folder));
                }
            }
        }

        public static void RemoveFolderWatcher(int FolderId)
        {
            if (IsRegistered)
            {
                using (var parameters = new Parameters())
                {
                    APICall(parameters
                        .AddInt((int)Function.RemoveFolderWatcher)
                        .AddInt((int)Id)
                        .AddInt(FolderId));
                }
            }
        }

        #endregion

        #region Events

        private static void OnEventLButtonClick()
        {
            if (LeftButtonClick is EventHandler)
            {
                HwndSource.Dispatcher.BeginInvoke(new EventHandler(LeftButtonClick), null, new EventArgs());
            }
        }

        private static void OnEventGetMenu()
        {
            if (GetMenu is EventGetMenu)
            {
                var menu = new List<Menu>();
                GetMenu(menu);
                if (IsRegistered)
                {
                    using (var parameters = new Parameters())
                    {
                        parameters.AddInt((int)Function.GetMenu).AddInt((int)Id).AddInt(menu.Count);
                        for (int i = menu.Count - 1; i >= 0; i--)
                        {
                            parameters.AddStructure(menu[i]);
                        }
                        APICall(parameters);
                    }
                }
            }
        }

        private static void OnEventMenuSelect(int nId)
        {
            if (MenuSelect is EventMenuSelect)
            {
                HwndSource.Dispatcher.BeginInvoke(new EventMenuSelect(MenuSelect), nId);
            }
        }

        #endregion
    }
}
