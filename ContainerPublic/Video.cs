using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Windows.Media.Imaging;
using System.IO;

namespace ContainerPublic
{
    public class Video
    {
        [DllImport("Interop.video.dll", CharSet = CharSet.Unicode)]
        private static extern bool GetVideoFrame(string FileName, double Delta, ref IntPtr Bits, ref int Width, ref int Height);

        [DllImport("Kernel32.dll", EntryPoint = "RtlMoveMemory")]
        public static extern void MoveMemory(IntPtr dest, IntPtr src, int size);

        public static BitmapImage GetVideoFrame(string FileName, double Delta)
        {
            IntPtr Bits = IntPtr.Zero;
            int Width = 0;
            int Height = 0;
            if (!GetVideoFrame(FileName, Delta, ref Bits, ref Width, ref Height))
            {
                throw new Exception("GetVideoFrame called failed");
            }

            var bmp = new System.Drawing.Bitmap(Width, Height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
            var bmpData = bmp.LockBits(new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height), System.Drawing.Imaging.ImageLockMode.ReadWrite,
                System.Drawing.Imaging.PixelFormat.Format24bppRgb);

            MoveMemory(bmpData.Scan0, Bits, bmpData.Stride * bmpData.Height);

            bmp.UnlockBits(bmpData);
            Marshal.FreeHGlobal(Bits);

            var ms = new MemoryStream();
            bmp.Save(ms, System.Drawing.Imaging.ImageFormat.Bmp);

            var bmpImage = new BitmapImage();
            bmpImage.BeginInit();
            bmpImage.StreamSource = ms;
            bmpImage.EndInit();
 
            return bmpImage;
        }
    }
}
