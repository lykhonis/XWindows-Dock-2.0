[CustomMessages]
ie8_title=Internet Explorer 8

en.ie8_size=1 MB - 77.5 MB

[Code]
const
	ie8_url = 'http://download.microsoft.com/download/C/C/0/CC0BD555-33DD-411E-936B-73AC6F95AE11/IE8-WindowsXP-x86-ENU.exe';

procedure ie8(MinVersion: string);
var
	version: string;
begin
	RegQueryStringValue(HKLM, 'Software\Microsoft\Internet Explorer', 'Version', version);
	if version < MinVersion then
		AddProduct('ie8.exe',
			'/q:a /C:"setup /QNT"',
			CustomMessage('ie8_title'),
			CustomMessage('ie8_size'),
			ie8_url);
end;