Start program window in specified position and size

Runin {program to start} {1 or more options}
{} means required arguemnts
[] means optional arguments

Options can be specified in any postion after target program
-pos	x y
-size	sizeX sizeY
-title
	remove title bar (not working for every program)
-wait	milliseconds (default 5000ms)
	delay milliseconds to change target program window


usage example

start firefox in (200, 300) width 1600 heigh 900
Runin.exe firefox.exe -pos 200 300 -size 1600 900

start chrome in (200, 300), wait 5000ms
Runin.exe chrome.exe -pos 200 300 -wait 5000