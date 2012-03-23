[CustomMessages]
dotnetfx40_title=.NET Framework 4.0

en.dotnetfx40_size=3 MB - 49 MB

[Code]
const
  dotnetfx40_url = 'http://download.microsoft.com/download/1/B/E/1BE39E79-7E39-46A3-96FF-047F95396215/dotNetFx40_Full_setup.exe';

procedure dotnetfx40();
var
	version: cardinal;
begin
	RegQueryDWordValue(HKLM, 'Software\Microsoft\NET Framework Setup\NDP\v4\Full', 'Install', version);
	if version <> 1 then
  AddProduct('dotNetFx40_Full_setup.exe',
			'/qb /norestart',
			CustomMessage('dotnetfx40_title'),
			CustomMessage('dotnetfx40_size'),
			dotnetfx40_url);
end;