[CustomMessages]
flashplayer_title=Adobe Flash Player

en.flashplayer_size=2,69 MB

[Code]	
const
	flashplayer_url = 'http://fpdownload.adobe.com/get/flashplayer/current/install_flash_player_ax.exe';

procedure flashplayer(MinVersion: string);
var
	version: string;
begin
  if maxwinversion(5, 2) then
  begin
    RegQueryStringValue(HKLM, 'Software\Macromedia\FlashPlayer', 'CurrentVersion', version);
	  if version < MinVersion then
    begin
      AddProduct('install_flash_player_ax.exe',
			   '-install',
			   CustomMessage('flashplayer_title'),
			   CustomMessage('flashplayer_size'),
			   flashplayer_url);
    end;
  end
  else
  if maxwinversion(6, 1) then
  begin
    RegQueryStringValue(HKCU, 'Software\Macromedia\FlashPlayer', 'FlashPlayerVersion', version);
	  if version < MinVersion then
    begin
		  AddProduct('install_flash_player_ax.exe',
			   '-install',
			   CustomMessage('flashplayer_title'),
			   CustomMessage('flashplayer_size'),
			   flashplayer_url);
    end;
  end;
end;