#include "scripts\products.iss"
#include "scripts\products\winversion.iss"
#include "scripts\products\fileversion.iss"
#include "scripts\products\msi20.iss"
#include "scripts\products\msi31.iss"
#include "scripts\products\dotnetfx35sp1.iss"
#include "scripts\products\dotnetfx40.iss"

[Setup]
AppName=XWindows Dock
AppVersion=2.0.3.0
AppCopyright=© 2008-2010 Lichonos Vladimir
AppVerName=XWindows Dock Addon 1.0
AppPublisher=Lichonos Vladimir
AppPublisherURL=http://xwdock.aqua-soft.org
AppSupportURL=http://xwdock.aqua-soft.org
AppUpdatesURL=http://xwdock.aqua-soft.org
VersionInfoProductName=XWindows Dock Addon
VersionInfoDescription=XWindows Dock Addon
VersionInfoVersion=1.0
VersionInfoCompany=Lichonos Vladimir
VersionInfoCopyright=© 2008-2010 Lichonos Vladimir
DefaultDirName={pf}\XWindows Dock
MinVersion=4.1,5.0
AllowNoIcons=yes
CreateAppDir=true
OutputBaseFilename=XWindows Dock Addon
SolidCompression=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x86 x64 ia64
InternalCompressLevel=ultra64
Compression=lzma2/ultra64
SetupIconFile=addon.ico
WizardSmallImageFile=xwd.bmp
Uninstallable=false

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"

[Dirs]
Name: "{app}\Public Plugins"
Name: "{app}\Public Plugins\API"
Name: "{app}\Public Plugins\API\C#"
Name: "{app}\Public Plugins\Container"
Name: "{app}\Public Plugins\GMail Checker"

[Files]
Source: "..\PluginManager\bin\Release\PluginManager.exe"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\Public Plugins\API\C#\XWindowsDock.cs"; DestDir: "{app}\Public Plugins\API\C#"; Flags: ignoreversion

Source: "..\Release\Public Plugins\Container\Container-Empty.png"; DestDir: "{app}\Public Plugins\Container"; Flags: ignoreversion
Source: "..\Release\Public Plugins\Container\Container-Opened.png"; DestDir: "{app}\Public Plugins\Container"; Flags: ignoreversion
Source: "..\Release\Public Plugins\Container\ContainerPublic.exe"; DestDir: "{app}\Public Plugins\Container"; Flags: ignoreversion
Source: "..\Release\Public Plugins\Container\ContainerPublicConfigurator.exe"; DestDir: "{app}\Public Plugins\Container"; Flags: ignoreversion
Source: "..\Release\Public Plugins\Container\Interop.video.dll"; DestDir: "{app}\Public Plugins\Container"; Flags: ignoreversion
Source: "..\Release\Public Plugins\Container\plugin.xml"; DestDir: "{app}\Public Plugins\Container"; Flags: ignoreversion

Source: "..\Release\Public Plugins\GMail Checker\GMail Checker.exe"; DestDir: "{app}\Public Plugins\GMail Checker"; Flags: ignoreversion
Source: "..\Release\Public Plugins\GMail Checker\icon.png"; DestDir: "{app}\Public Plugins\GMail Checker"; Flags: ignoreversion
Source: "..\Release\Public Plugins\GMail Checker\plugin.xml"; DestDir: "{app}\Public Plugins\GMail Checker"; Flags: ignoreversion
Source: "..\Release\Public Plugins\GMail Checker\Microsoft.Expression.Drawing.dll"; DestDir: "{app}\Public Plugins\GMail Checker"; Flags: ignoreversion
Source: "..\Release\Public Plugins\GMail Checker\Microsoft.Expression.Effects.dll"; DestDir: "{app}\Public Plugins\GMail Checker"; Flags: ignoreversion
Source: "..\Release\Public Plugins\GMail Checker\Microsoft.Expression.Interactions.dll"; DestDir: "{app}\Public Plugins\GMail Checker"; Flags: ignoreversion

[Icons]
Name: "{group}\Plugin Manager"; Filename: "{app}\PluginManager.exe"

[Code]
function InitializeSetup(): Boolean;
begin
	initwinversion();

	msi20('2.0');
	msi31('3.1');

	dotnetfx35sp1();
  dotnetfx40();
	
	Result := true;
end;
