using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;
using System.Security.Principal;

namespace ContainerPublicConfigurator
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

            if (e.Args.Length == 11)
            {
                if (e.Args[0] == "-p")
                {
                    try
                    {
                        Config.IconSize = Convert.ToInt32(e.Args[1]);
                        Config.GridMaximumCols = Convert.ToInt32(e.Args[2]);
                        Config.GridMaximumRows = Convert.ToInt32(e.Args[3]);
                        Config.HideDelay = Convert.ToInt32(e.Args[4]);
                        Config.PopupDelay = Convert.ToInt32(e.Args[5]);
                        Config.HoverEnabled = Convert.ToBoolean(e.Args[6]);
                        Config.HoverHideDelay = Convert.ToInt32(e.Args[7]);
                        Config.HoverMoveDealy = Convert.ToInt32(e.Args[8]);
                        Config.HoverPopupDelay = Convert.ToInt32(e.Args[9]);
                        Config.HoverTextEnabled = Convert.ToBoolean(e.Args[10]);

                        var id = WindowsIdentity.GetCurrent();
                        var wPrincipal = new WindowsPrincipal(id);
                        if (wPrincipal.IsInRole(WindowsBuiltInRole.Administrator))
                        {
                            Config.Save();
                        }
                    }
                    catch
                    {
                    }
                }
            }
        }
    }
}
