Start program in specified in specified properties

Runin {program to start} {1 or more options}
{} means required arguemnts
[] means optional arguments

Options can be specified in any postion after target program
-pos	x y
	left top position of screen is (0, 0)
-size	sizeX sizeY
-wait	milliseconds (default 5000ms)
	delay milliseconds to change target program window
-title
	remove title bar (not working for every program)
-menu
	remove menu


usage example

start firefox in (200, 300) width 1600 heigh 900
Runin.exe "C:\Program Files\Mozilla Firefox\firefox.exe" -pos 200 300 -size 1600 900

start chrome in (200, 300), wait 5000ms
Runin.exe chrome.exe -pos 200 300 -wait 5000

start Telegram with "always on top"
Runin.exe "%appdata%\Telegram Desktop\Telegram.exe" -top