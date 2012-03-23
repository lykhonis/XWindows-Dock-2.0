#include "scripts\products.iss"
#include "scripts\products\winversion.iss"
#include "scripts\products\fileversion.iss"
#include "scripts\products\msi20.iss"
#include "scripts\products\msi31.iss"

[Setup]
AppName=XWindows Dock
AppVersion=2.0.3.0
AppCopyright=© 2008-2010 Lichonos Vladimir
AppVerName=XWindows Dock 2.0.3.0
AppPublisher=Lichonos Vladimir
AppPublisherURL=http://xwdock.aqua-soft.org
AppSupportURL=http://xwdock.aqua-soft.org
AppUpdatesURL=http://xwdock.aqua-soft.org
VersionInfoProductName=XWindows Dock
VersionInfoDescription=XWindows Dock
VersionInfoVersion=2.0.3.0
VersionInfoCompany=Lichonos Vladimir
VersionInfoCopyright=© 2008-2010 Lichonos Vladimir
DefaultDirName={pf}\XWindows Dock
DefaultGroupName=XWindows Dock
MinVersion=4.1,5.0
AllowNoIcons=yes
CreateAppDir=true
OutputBaseFilename=XWindows Dock
SolidCompression=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x86 x64 ia64
InternalCompressLevel=ultra64
Compression=lzma2/ultra64
UninstallDisplayIcon={app}\XWD.exe
UninstallDisplayName=XWindows Dock
Uninstallable=true
SetupIconFile=xwd.ico
WizardSmallImageFile=xwd.bmp

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Dirs]
Name: "{app}\Docklets"
Name: "{app}\Icons"
Name: "{app}\Images"
Name: "{app}\Plugins"
Name: "{app}\Plugins\Container"
Name: "{app}\Plugins\ExampleDocklet"
Name: "{app}\Plugins\ExampleDocklet\Source"
Name: "{app}\Skins"
Name: "{app}\Skins\Default"

[Files]
Source: "..\Release\XWD.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\XWDCore.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\XWDCore64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\XWDCore64.exe"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\CURL_LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\curllib.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\libsasl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\openldap.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\Release\Icons\Box2.png"; DestDir: "{app}\Icons"; Flags: ignoreversion
Source: "..\Release\Icons\Finder.png"; DestDir: "{app}\Icons"; Flags: ignoreversion
Source: "..\Release\Icons\Folder.png"; DestDir: "{app}\Icons"; Flags: ignoreversion
Source: "..\Release\Icons\Preference.png"; DestDir: "{app}\Icons"; Flags: ignoreversion
Source: "..\Release\Icons\Stack-Empty.png"; DestDir: "{app}\Icons"; Flags: ignoreversion
Source: "..\Release\Icons\Stack-Opened.png"; DestDir: "{app}\Icons"; Flags: ignoreversion
Source: "..\Release\Icons\Trash-Empty.png"; DestDir: "{app}\Icons"; Flags: ignoreversion
Source: "..\Release\Icons\Trash-Full.png"; DestDir: "{app}\Icons"; Flags: ignoreversion

Source: "..\Release\Images\animation-poof.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\button-large.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\button-trackbar-bckg.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\button-trackbar-button.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\button-window-close.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\icon-blank.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\icon-docklet.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\icon-tab-general.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\icon-tab-plugins.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\icon-tab-skins.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\icon-update.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\scroll-bottom.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\scroll-button.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\scroll-middle.png"; DestDir: "{app}\Images"; Flags: ignoreversion
Source: "..\Release\Images\scroll-top.png"; DestDir: "{app}\Images"; Flags: ignoreversion

Source: "..\Release\Plugins\Container\Container.dll"; DestDir: "{app}\Plugins\Container"; Flags: ignoreversion
Source: "..\Release\Plugins\Container\Container-Empty.png"; DestDir: "{app}\Plugins\Container"; Flags: ignoreversion
Source: "..\Release\Plugins\Container\Container-Opened.png"; DestDir: "{app}\Plugins\Container"; Flags: ignoreversion

Source: "..\Release\Plugins\ExampleDocklet\ExampleDocklet.dll"; DestDir: "{app}\Plugins\ExampleDocklet"; Flags: ignoreversion
Source: "..\Release\Plugins\ExampleDocklet\ExampleDocklet.ico"; DestDir: "{app}\Plugins\ExampleDocklet"; Flags: ignoreversion
Source: "..\Release\Plugins\ExampleDocklet\Source\ExampleDocklet.vcproj"; DestDir: "{app}\Plugins\ExampleDocklet\Source"; Flags: ignoreversion
Source: "..\Release\Plugins\ExampleDocklet\Source\ExampleDocklet.def"; DestDir: "{app}\Plugins\ExampleDocklet\Source"; Flags: ignoreversion
Source: "..\Release\Plugins\ExampleDocklet\Source\main.cpp"; DestDir: "{app}\Plugins\ExampleDocklet\Source"; Flags: ignoreversion
Source: "..\Release\Plugins\ExampleDocklet\Source\XWDAPI.cpp"; DestDir: "{app}\Plugins\ExampleDocklet\Source"; Flags: ignoreversion
Source: "..\Release\Plugins\ExampleDocklet\Source\XWDAPI.h"; DestDir: "{app}\Plugins\ExampleDocklet\Source"; Flags: ignoreversion

Source: "..\Release\Skins\Default\background-2d.png"; DestDir: "{app}\Skins\Default"; Flags: ignoreversion
Source: "..\Release\Skins\Default\background-3d.png"; DestDir: "{app}\Skins\Default"; Flags: ignoreversion
Source: "..\Release\Skins\Default\background-edge-3d.png"; DestDir: "{app}\Skins\Default"; Flags: ignoreversion
Source: "..\Release\Skins\Default\indicator-2d.png"; DestDir: "{app}\Skins\Default"; Flags: ignoreversion
Source: "..\Release\Skins\Default\indicator-3d.png"; DestDir: "{app}\Skins\Default"; Flags: ignoreversion
Source: "..\Release\Skins\Default\separator-2d.png"; DestDir: "{app}\Skins\Default"; Flags: ignoreversion
Source: "..\Release\Skins\Default\separator-3d.png"; DestDir: "{app}\Skins\Default"; Flags: ignoreversion
Source: "..\Release\Skins\Default\skin.ini"; DestDir: "{app}\Skins\Default"; Flags: ignoreversion

[Icons]
Name: "{group}\XWindows Dock"; Filename: "{app}\XWD.exe"
Name: "{group}\{cm:ProgramOnTheWeb,XWindows Dock}"; Filename: "http://xwdock.aqua-soft.org"
Name: "{group}\{cm:UninstallProgram,XWindows Dock}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\XWindows Dock"; Filename: "{app}\XWD.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\XWD.exe"; Description: "{cm:LaunchProgram,XWindows Dock}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{userstartup}\XWindows Dock.lnk"
Type: files; Name: "{group}\Plugin Manager.lnk"

[Code]
function InitializeSetup(): Boolean;
begin
	initwinversion();

	msi20('2.0');
	msi31('3.1');
	
	Result := true;
end;
