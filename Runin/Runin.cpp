#include "pch.h"
#include <Windows.h>
#include <tchar.h>

HWND new_proc_hwnd = NULL;

// reference
// https://stackoverflow.com/questions/6041615/c-finding-the-gui-thread-from-a-list-of-thread-ids
BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	const DWORD new_proc_pid = *(DWORD *)lParam;
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	const BOOL result = new_proc_pid == pid && IsWindowVisible(hwnd);
	if (result) {
		new_proc_hwnd = hwnd;

		//HWND hwnd = HWND(0x0000000000080928); // 0x0000000000080928
		LONG_PTR windowLongPtr = GetWindowLongPtr(hwnd, GWL_STYLE);
		//windowLongPtr &= ~WS_CAPTION windowLongPtr &= ~WS_CAPTION& ~WS_SYSMENU & ~WS_OVERLAPPED;
		windowLongPtr &= ~WS_CAPTION;
		SetWindowLongPtr(hwnd, GWL_STYLE, windowLongPtr);
	}

	return !result;
}

void _tmain(int argc, TCHAR *argv[])
{
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USEPOSITION;

	DWORD sleep_time = 5000;
	UINT uFlags = SWP_NOSIZE;

	if (argc <= 3)
		return;

	si.dwX = _tcstoul(argv[2], nullptr, 10);
	si.dwY = _tcstoul(argv[3], nullptr, 10);

	// sleepTime could be 4/6th argument
	if (argc > 5) { // 5+ arguments
		si.dwFlags |= STARTF_USESIZE;
		si.dwXSize = _tcstoul(argv[4], nullptr, 10);
		si.dwYSize = _tcstoul(argv[5], nullptr, 10);
		uFlags = 0;
		if (argc > 6) // 6th argument
			sleep_time = _tcstoul(argv[6], nullptr, 10);
	} else if (argc == 5) { // 4 arguments
		sleep_time = _tcstoul(argv[4], nullptr, 10);
	}

	// don't check CreateProcess return (assume create successful, no crash if fail)
	// SetWindowPos won't crash if new_proc_hwnd is null
	CreateProcess(NULL, argv[1], NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi);
	if (sleep_time) {
		Sleep(sleep_time);
		EnumWindows(EnumWindowsProc, (LPARAM)&pi.dwProcessId);
		SetWindowPos(new_proc_hwnd, HWND_TOP, si.dwX, si.dwY, si.dwXSize, si.dwYSize, uFlags);
	}
}
