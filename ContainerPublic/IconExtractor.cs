using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.Windows.Media.Imaging;
using System.Drawing;

namespace Utils
{
    public class IconExtractor
    {
        #region Declarations

        [StructLayout(LayoutKind.Sequential)]
        private struct IconDirEntry
        {
            public byte bWidth;
            public byte bHeight;
            public byte bColorCount;
            public byte bReserved;
            public short wPlanes;
            public short wBitCount;
            public uint dwBytesInRes;
            public uint dwImageOffset;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct IconDir
        {
            public short idReserved;
            public short idType;
            public short idCount;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct SHFileInfo
        {
            public IntPtr hIcon;
            public int iIcon;
            public uint dwAttributes;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string szDisplayName;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
            public string szTypeName;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct OSVersionInfo
        {
            public int dwOSVersionInfoSize;
            public uint dwMajorVersion;
            public uint dwMinorVersion;
            public uint dwBuildNumber;
            public uint dwPlatformId;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string szCSDVersion;
            public Int16 wServicePackMajor;
            public Int16 wServicePackMinor;
            public Int16 wSuiteMask;
            public Byte wProductType;
            public Byte wReserved;
        }

        private enum SystemMetric
        {
            SM_CXICON = 11,
        }

        #region Private ImageList COM Interop

        [StructLayout(LayoutKind.Sequential)]
        private struct RECT
        {
            int left;
            int top;
            int right;
            int bottom;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct POINT
        {
            int x;
            int y;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct IMAGELISTDRAWPARAMS
        {
            public int cbSize;
            public IntPtr himl;
            public int i;
            public IntPtr hdcDst;
            public int x;
            public int y;
            public int cx;
            public int cy;
            public int xBitmap;        // x offest from the upperleft of bitmap
            public int yBitmap;        // y offset from the upperleft of bitmap
            public int rgbBk;
            public int rgbFg;
            public int fStyle;
            public int dwRop;
            public int fState;
            public int Frame;
            public int crEffect;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct IMAGEINFO
        {
            public IntPtr hbmImage;
            public IntPtr hbmMask;
            public int Unused1;
            public int Unused2;
            public RECT rcImage;
        }

        [ComImportAttribute()]
        [GuidAttribute("46EB5926-582E-4017-9FDF-E8998DAA0950")]
        [InterfaceTypeAttribute(ComInterfaceType.InterfaceIsIUnknown)]
        interface IImageList
        {
            [PreserveSig]
            int Add(IntPtr hbmImage, IntPtr hbmMask, ref int pi);

            [PreserveSig]
            int ReplaceIcon(int i, IntPtr hicon, ref int pi);

            [PreserveSig]
            int SetOverlayImage(int iImage, int iOverlay);

            [PreserveSig]
            int Replace(int i, IntPtr hbmImage, IntPtr hbmMask);

            [PreserveSig]
            int AddMasked(IntPtr hbmImage, int crMask, ref int pi);

            [PreserveSig]
            int Draw(ref IMAGELISTDRAWPARAMS pimldp);

            [PreserveSig]
            int Remove(int i);

            [PreserveSig]
            int GetIcon(int i, int flags, ref IntPtr picon);

            [PreserveSig]
            int GetImageInfo(int i, ref IMAGEINFO pImageInfo);

            [PreserveSig]
            int Copy(int iDst, IImageList punkSrc, int iSrc, int uFlags);

            [PreserveSig]
            int Merge(int i1, IImageList punk2, int i2, int dx, int dy, ref Guid riid, ref IntPtr ppv);

            [PreserveSig]
            int Clone(ref Guid riid, ref IntPtr ppv);

            [PreserveSig]
            int GetImageRect(int i, ref RECT prc);

            [PreserveSig]
            int GetIconSize(ref int cx, ref int cy);

            [PreserveSig]
            int SetIconSize(int cx, int cy);

            [PreserveSig]
            int GetImageCount(ref int pi);

            [PreserveSig]
            int SetImageCount(int uNewCount);

            [PreserveSig]
            int SetBkColor(int clrBk, ref int pclr);

            [PreserveSig]
            int GetBkColor(ref int pclr);

            [PreserveSig]
            int BeginDrag(int iTrack, int dxHotspot, int dyHotspot);

            [PreserveSig]
            int EndDrag();

            [PreserveSig]
            int DragEnter(IntPtr hwndLock, int x, int y);

            [PreserveSig]
            int DragLeave(IntPtr hwndLock);

            [PreserveSig]
            int DragMove(int x, int y);

            [PreserveSig]
            int SetDragCursorImage(ref IImageList punk, int iDrag, int dxHotspot, int dyHotspot);

            [PreserveSig]
            int DragShowNolock(int fShow);

            [PreserveSig]
            int GetDragImage(ref POINT ppt, ref POINT pptHotspot, ref Guid riid, ref IntPtr ppv);

            [PreserveSig]
            int GetItemFlags(int i, ref int dwFlags);

            [PreserveSig]
            int GetOverlayImage(int iOverlay, ref int piIndex);
        }

        #endregion

        #endregion

        #region Native Methods

        [DllImport("user32.dll")]
        private static extern int GetSystemMetrics(SystemMetric smIndex);

        [DllImport("kernel32.dll")]
        private static extern IntPtr FindResource(IntPtr hModule, int lpName, int lpType);

        [DllImport("kernel32.dll")]
        private static extern IntPtr FindResource(IntPtr hModule, string lpName, int lpType);

        [DllImport("kernel32.dll")]
        private static extern IntPtr FindResource(IntPtr hModule, string lpName, string lpType);

        [DllImport("kernel32.dll")]
        private static extern IntPtr LoadResource(IntPtr hModule, IntPtr hResInfo);

        [DllImport("kernel32.dll")]
        private static extern IntPtr LockResource(IntPtr hResData);

        [DllImport("user32.dll")]
        private static extern int LookupIconIdFromDirectoryEx(IntPtr presbits, bool fIcon, int cxDesired, int cyDesired, uint Flags);

        [DllImport("user32.dll")]
        private static extern IntPtr CreateIconFromResourceEx(IntPtr pbIconBits, uint cbIconBits, bool fIcon, uint dwVersion, int cxDesired, int cyDesired, uint uFlags);

        [DllImport("kernel32.dll")]
        private static extern uint SizeofResource(IntPtr hModule, IntPtr hResInfo);

        [DllImport("kernel32.dll")]
        private static extern IntPtr LoadLibraryEx(string lpFileName, IntPtr hFile, uint dwFlags);

        [DllImport("kernel32.dll")]
        private static extern bool FreeLibrary(IntPtr hModule);

        [DllImport("shell32.dll")]
        private static extern IntPtr SHSimpleIDListFromPath(string szPath);

        [DllImport("shell32.dll")]
        private static extern IntPtr SHGetFileInfo(IntPtr pidl, uint dwFileAttributes, ref SHFileInfo psfi, int cbFileInfo, uint uFlags);

        [DllImport("shell32.dll")]
        private static extern IntPtr SHGetFileInfo(string pszPath, uint dwFileAttributes, ref SHFileInfo psfi, int cbFileInfo, uint uFlags);

        [DllImport("kernel32.dll")]
        private static extern bool EnumResourceNames(IntPtr hModule, int lpszType, EnumResNameProcDelegate lpEnumFunc, IntPtr lParam);

        private delegate bool EnumResNameProcDelegate(IntPtr hModule, IntPtr lpszType, IntPtr lpszName, IntPtr lParam);

        private static bool IsIntResource(IntPtr value)
        {
            if (((uint)value) > ushort.MaxValue)
            {
                return false;
            }
            return true;
        }

        private static string GetResourceName(IntPtr value)
        {
            if (IsIntResource(value))
            {
                return value.ToString();
            }
            return Marshal.PtrToStringUni((IntPtr)value);
        }

        [DllImport("user32.dll")]
        private static extern IntPtr LoadImage(IntPtr hinst, string lpszName, uint uType, int cxDesired, int cyDesired, uint fuLoad);

        [DllImport("kernel32")]
        private static extern bool GetVersionEx(ref OSVersionInfo osvi);

        [DllImport("shell32.dll", EntryPoint = "#727")]
        private extern static int SHGetImageList(int iImageList, ref Guid riid, ref IImageList ppv);

        [DllImport("user32.dll")]
        private static extern bool DestroyIcon(IntPtr hIcon);

        #endregion

        private static object StructureFromByte(byte[] bytes, int offset, Type objectType)
        {
            var size = Marshal.SizeOf(objectType);
            if (size + offset > bytes.Length)
            {
                return null;
            }
            var buffer = Marshal.AllocHGlobal(size);
            Marshal.Copy(bytes, offset, buffer, size);
            var obj = Marshal.PtrToStructure(buffer, objectType);
            Marshal.FreeHGlobal(buffer);
            return obj;
        }

        public static int GetIconSize(string FileName)
        {
            var iconSize = GetSystemMetrics(SystemMetric.SM_CXICON);
            try
            {
                using (var fs = File.OpenRead(FileName))
                {
                    var buffer = new byte[Marshal.SizeOf(typeof(IconDir))];
                    fs.Read(buffer, 0, buffer.Length);
                    var iconDir = (IconDir)StructureFromByte(buffer, 0, typeof(IconDir));

                    if ((iconDir.idReserved == 0) && (iconDir.idType == 1))
                    {
                        var sizeOfEntry = Marshal.SizeOf(typeof(IconDirEntry));
                        buffer = new byte[iconDir.idCount * sizeOfEntry];
                        fs.Read(buffer, 0, buffer.Length);

                        for (var i = 0; i < iconDir.idCount; i++)
                        {
                            var iconEntry = (IconDirEntry)StructureFromByte(buffer, i * sizeOfEntry, typeof(IconDirEntry));
                            if (iconEntry.bWidth > iconSize)
                            {
                                iconSize = iconEntry.bWidth;
                            }
                        }
                    }
                }
            }
            catch
            {
            }
            return iconSize;
        }

        public static IntPtr GetIcon(IntPtr hModule, string ResourceName)
        {
            var icon = IntPtr.Zero;
            var hResource = FindResource(hModule, ResourceName, 3 + 11/*DIFFERENCE*/);
            if (hResource != IntPtr.Zero)
            {
                var hMem = LoadResource(hModule, hResource);
                var lpResource = LockResource(hMem);

                if ((hMem != IntPtr.Zero) && (lpResource != IntPtr.Zero))
                {
                    var nId = LookupIconIdFromDirectoryEx(lpResource, true, 0x200, 0x200, 0);
                    hResource = FindResource(hModule, nId, 3);

                    if (hResource != IntPtr.Zero)
                    {
                        hMem = LoadResource(hModule, hResource);
                        lpResource = LockResource(hMem);

                        if ((hMem != IntPtr.Zero) && (lpResource != IntPtr.Zero))
                        {
                            icon = CreateIconFromResourceEx(lpResource, SizeofResource(hModule, hResource), true, 0x30000, 0, 0, 0);
                        }
                    }
                }
            }
            return icon;
        }

        public static IntPtr GetIcon(string FileName, string ResourceName)
        {
            var icon = IntPtr.Zero;
            var hModule = LoadLibraryEx(FileName, IntPtr.Zero, 2/*LOAD_LIBRARY_AS_DATAFILE*/);
            if (hModule != IntPtr.Zero)
            {
                icon = GetIcon(hModule, ResourceName);
                FreeLibrary(hModule);
            }
            return icon;
        }

        private static string EnumResourceName;
        private static int EnumResourceIndex;
        private static int EnumResourceCounter;
        private static bool EnumResNameProc(IntPtr hModule, IntPtr lpszType, IntPtr lpszName, IntPtr lParam)
        {
            if (EnumResourceCounter == EnumResourceIndex)
            {
                if (IsIntResource(lpszName))
                {
                    EnumResourceName = string.Format("#{0}", GetResourceName(lpszName));
                }
                else
                {
                    EnumResourceName = GetResourceName(lpszName);
                }
                return false;
            }
            EnumResourceCounter++;
            return true;
        }

        public static IntPtr GetIcon(string FileName)
        {
            var icon = IntPtr.Zero;
            var sfi = new SHFileInfo();

            var pidl = SHSimpleIDListFromPath(FileName);
            if ((pidl != IntPtr.Zero) && (FileName.Substring(0, 3) == "::{"))
            {
                SHGetFileInfo(pidl, 0, ref sfi, Marshal.SizeOf(typeof(SHFileInfo)), 0x1000 | 0x8); // SHGFI_ICONLOCATION | SHGFI_PIDL
                if (File.Exists(sfi.szDisplayName))
                {
                    FileName = sfi.szDisplayName;
                }
            }

            var ext = string.Empty;
            var i = FileName.LastIndexOf('.');
            if (i >= 0)
            {
                ext = FileName.Substring(i).ToLower();
            }
            if ((ext == ".exe") || (ext == ".ocx") || (ext == ".dll") || (ext == ".scr") || (ext == ".bin") || (ext == ".cpl"))
            {
                IntPtr hModule = LoadLibraryEx(FileName, IntPtr.Zero, 2/*LOAD_LIBRARY_AS_DATAFILE*/);
                if (hModule != IntPtr.Zero)
                {
                    if (sfi.iIcon >= 0)
                    {
                        EnumResourceName = string.Empty;
                        EnumResourceIndex = sfi.iIcon;
                        EnumResourceCounter = 0;
                        EnumResourceNames(hModule, 3 + 11/*DIFFERENCE*/, new EnumResNameProcDelegate(EnumResNameProc), IntPtr.Zero);
                    }
                    else
                    {
                        EnumResourceName = string.Format("#{0}", -sfi.iIcon);
                    }
                    icon = GetIcon(hModule, EnumResourceName);
                    FreeLibrary(hModule);
                }
            }
            else if ((ext == ".ico") || (ext == ".icon"))
            {
                var iconSize = GetIconSize(FileName);
                icon = LoadImage(IntPtr.Zero, FileName, 1, iconSize, iconSize, 0x10 | 0x2); // LR_LOADFROMFILE | LR_COLOR
            }

            if (icon == IntPtr.Zero)
            {
                var osInfo = new OSVersionInfo();
                osInfo.dwOSVersionInfoSize = Marshal.SizeOf(typeof(OSVersionInfo));
                GetVersionEx(ref osInfo);

                var iidImageList = new Guid("46EB5926-582E-4017-9FDF-E8998DAA0950");
                IImageList imageList = null;

                if ((osInfo.dwMajorVersion >= 6) && (SHGetImageList(4/*SHIL_JUMBO*/, ref iidImageList, ref imageList) >= 0))
                {
                    SHGetFileInfo(FileName, 0, ref sfi, Marshal.SizeOf(typeof(SHFileInfo)), 0x4000/*SHGFI_SYSICONINDEX*/);
                    imageList.GetIcon(sfi.iIcon, 0x20 | 0x1, ref icon); // ILD_IMAGE | ILD_TRANSPARENT
                    if(!AnalizeIcon256(icon))
			        {
				        DestroyIcon(icon);
				        icon = IntPtr.Zero;
			        }
                }

                if ((icon == IntPtr.Zero) && (SHGetImageList(2/*SHIL_EXTRALARGE*/, ref iidImageList, ref imageList) >= 0))
                {
                    SHGetFileInfo(FileName, 0, ref sfi, Marshal.SizeOf(typeof(SHFileInfo)), 0x4000/*SHGFI_SYSICONINDEX*/);
                    imageList.GetIcon(sfi.iIcon, 0x20 | 0x1, ref icon); // ILD_IMAGE | ILD_TRANSPARENT
                    if (icon == (IntPtr)(-1))
                    {
                        icon = IntPtr.Zero;
                    }
                }
            }

            if ((icon == IntPtr.Zero) && (pidl != IntPtr.Zero) && (SHGetFileInfo(pidl, 0, ref sfi, Marshal.SizeOf(typeof(SHFileInfo)),
                0x100 | 0x0 | 0x8) != IntPtr.Zero)) // SHGFI_ICON | SHGFI_LARGEICON | SHGFI_PIDL
            {
                icon = sfi.hIcon;
            }

            return icon;
        }

        public static BitmapImage IconToBitmap(IntPtr Icon)
        {
            var icon = System.Drawing.Icon.FromHandle(Icon);
            var bmp = icon.ToBitmap();

            var ms = new MemoryStream();
            bmp.Save(ms, System.Drawing.Imaging.ImageFormat.Png);

            var bmpImage = new BitmapImage();
            bmpImage.BeginInit();
            bmpImage.StreamSource = ms;
            bmpImage.EndInit();

            DestroyIcon(Icon);

            return bmpImage;
        }

        public static bool AnalizeIcon256(IntPtr Icon)
        {
            var icon = System.Drawing.Icon.FromHandle(Icon);
            var bmp = icon.ToBitmap();
            var bmpData = bmp.LockBits(new Rectangle(0, 0, bmp.Width, bmp.Height), System.Drawing.Imaging.ImageLockMode.ReadOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            var size = (bmpData.Width - 48) * (bmpData.Height - 48);
            var offset = (48 * bmpData.Width + 48) * 4;
            for (var i = 0; i < size; i++)
            {
                var pixel = Marshal.ReadInt32(bmpData.Scan0, offset + i * 4);
                if (pixel != 0)
                {
                    return true;
                }
            }
            bmp.UnlockBits(bmpData);
            return false;
        }
    }
}
