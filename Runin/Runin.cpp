#include "pch.h"
#include <Windows.h>
#include <tchar.h>
#include <time.h>

#define WSTRCMP_CONST(str, const_str) \
	wmemcmp(str, const_str, sizeof(const_str) / sizeof(*const_str))
void test();

volatile HWND new_proc_hwnd = NULL;
//bool g_removeTitle = false;
bool g_removeMenu = false;
LONG_PTR g_removeWindowStyle = 0;

// reference
// https://stackoverflow.com/questions/6041615/c-finding-the-gui-thread-from-a-list-of-thread-ids
BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
	const DWORD new_proc_pid = *(DWORD *)lParam;
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	// filter more window? not child window and not owned window
	BOOL result = ((new_proc_pid == pid) && IsWindowVisible(hwnd));
	if (result) {
		new_proc_hwnd = hwnd;

		if (g_removeWindowStyle) {
			LONG_PTR windowLongPtr = GetWindowLongPtr(hwnd, GWL_STYLE);
			windowLongPtr &= ~g_removeWindowStyle;
			SetWindowLongPtr(hwnd, GWL_STYLE, windowLongPtr);
		}
		if (g_removeMenu) {
			HMENU hwndMenu = GetMenu(hwnd);
			SetMenu(hwnd, NULL);
			DestroyMenu(hwndMenu);
		}
	}

	return !result;
}

void wmain(int argc, wchar_t* __restrict argv[])
{
	if (argc <= 1)
		return;

	bool changePos = false;
	bool changeSize = false;

	wchar_t workingDirectory[MAX_PATH];
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(STARTUPINFO);
	DWORD find_wait = 30;
	DWORD init_wait = 10000;
	DWORD delay_time = 5000;
	HWND hWndInsertAfter = HWND_TOP;
	UINT uFlags = SWP_NOMOVE | SWP_NOSIZE;

	// later arg override early ones
	{
		wchar_t** const end = argv + argc;
		for (wchar_t* __restrict* __restrict arg = argv + 1; arg < end; ++arg) {
			if (!WSTRCMP_CONST(*arg, L"-pos")) {
				si.dwFlags |= STARTF_USEPOSITION;
				uFlags &= ~SWP_NOMOVE;
				si.dwX = _tcstoul(*++arg, nullptr, 10);
				si.dwY = _tcstoul(*++arg, nullptr, 10);
			} else if (!WSTRCMP_CONST(*arg, L"-size")) {
				si.dwFlags |= STARTF_USESIZE;
				uFlags &= ~SWP_NOSIZE;
				si.dwXSize = _tcstoul(*++arg, nullptr, 10);
				si.dwYSize = _tcstoul(*++arg, nullptr, 10);
			} else if (!WSTRCMP_CONST(*arg, L"-top")) {
				hWndInsertAfter = HWND_TOPMOST;
			} else if (!WSTRCMP_CONST(*arg, L"-find_wait")) {
				changeSize = true;
				find_wait = _tcstoul(*++arg, nullptr, 10);
			} else if (!WSTRCMP_CONST(*arg, L"-init_wait")) {
				changeSize = true;
				init_wait = _tcstoul(*++arg, nullptr, 10);
			} else if (!WSTRCMP_CONST(*arg, L"-delay")) {
				changeSize = true;
				delay_time = _tcstoul(*++arg, nullptr, 10);
			} else if (!WSTRCMP_CONST(*arg, L"-title"))
				//g_removeTitle = true;
				g_removeWindowStyle |= WS_CAPTION;
			else if (!WSTRCMP_CONST(*arg, L"-menu")) {
				g_removeMenu = true;
				g_removeWindowStyle |= WS_SYSMENU;
			}
		}
	}

	// set working directory to program directory or some may work incorrectly
	wchar_t *ptr = wcsrchr(argv[1], L'\\');
	const size_t length = ptr ? ptr - argv[1] : 0;
	wmemcpy(workingDirectory, argv[1], length);
	workingDirectory[length] = 0;
	// don't check CreateProcess return (assume create successful, no crash if fail)
	// SetWindowPos won't crash if new_proc_hwnd is null
	#pragma warning( disable : 6335) // handle will be released after Runin ends, don't need to CloseHandle()
	BOOL result = CreateProcess(NULL, argv[1], NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP | INHERIT_PARENT_AFFINITY, NULL, workingDirectory, &si, &pi);
	if (result) {
		const time_t end_time = time(NULL) + find_wait;
		if (!WaitForInputIdle(pi.hProcess, find_wait * 1000) && init_wait)
			Sleep(init_wait); // some program may need extra sleep to wait?
		do {
			EnumWindows(EnumWindowsProc, (LPARAM)&pi.dwProcessId);
			if (new_proc_hwnd) {
				result = SetWindowPos(new_proc_hwnd, hWndInsertAfter, si.dwX, si.dwY, si.dwXSize, si.dwYSize, uFlags | SWP_SHOWWINDOW);
				if (!result)
					DWORD error = GetLastError();
				else
					break;
			} else
			Sleep(delay_time);
		} while (time(NULL) < end_time);
	}
}

// for dev test
void test() {
	HWND hwnd = HWND(0x0000000000390D0E); // 0x0000000000080928
	LONG_PTR windowLongPtr = GetWindowLongPtr(hwnd, GWL_STYLE);
	//windowLongPtr &= ~WS_CAPTION windowLongPtr &= ~WS_CAPTION& ~WS_SYSMENU & ~WS_OVERLAPPED;
	windowLongPtr &= ~WS_CAPTION;
	windowLongPtr &= ~WS_SYSMENU;
	//windowLongPtr &= ~WS_OVERLAPPED;
	SetWindowLongPtr(hwnd, GWL_STYLE, windowLongPtr);

	windowLongPtr = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	windowLongPtr &= ~WS_EX_WINDOWEDGE;
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, windowLongPtr);
	HMENU hwndMenu = GetMenu(hwnd);
	SetMenu(hwnd, NULL);
	BOOL bmenu = DestroyMenu(hwndMenu);
	SetMenu(hwnd, hwndMenu);
}
