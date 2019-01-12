Start program in specified position and size

Runin {program to start} {posX} {posY} [SizeX SizeY] [SleepTime]
{} means required arguemnts
[] means optional arguments
sleeps SleepTime ms
When SleepTime = 0, disable "wait and SetWindowPos"

It can take 4~6 arguments.
4 arguments, 4th argument is parsed as SleepTime
5+ arguments, 4~5th arguments are parsed as width and heigh
6 arguments, 6th argument is parsed as SleepTime

usage example

start firefox in (200, 300) width 1600 heigh 900
Runin.exe firefox.exe 200 300 1600 900

start chrome in (200, 300), sleep 5000ms
Runin.exe chrome.exe 200 300 5000