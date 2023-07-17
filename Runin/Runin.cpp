#include "pch.h"
#include <Windows.h>
#include <tchar.h>

#define WSTRCMP_CONST(str, const_str) \
	wmemcmp(str, const_str, sizeof(const_str) / sizeof(*const_str))
void test();

HWND new_proc_hwnd = NULL;
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
	const BOOL result = new_proc_pid == pid && IsWindowVisible(hwnd);
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
	DWORD wait_time = 5000;
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
			}
			else if (!WSTRCMP_CONST(*arg, L"-top")) {
				hWndInsertAfter = HWND_TOPMOST;
			} else if (!WSTRCMP_CONST(*arg, L"-wait")) {
				changeSize = true;
				wait_time = _tcstoul(*++arg, nullptr, 10);
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
	CreateProcess(NULL, argv[1], NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP | INHERIT_PARENT_AFFINITY, NULL, workingDirectory, &si, &pi);
	if (wait_time) {
		Sleep(wait_time);
		EnumWindows(EnumWindowsProc, (LPARAM)&pi.dwProcessId);
		SetWindowPos(new_proc_hwnd, hWndInsertAfter, si.dwX, si.dwY, si.dwXSize, si.dwYSize, uFlags| SWP_SHOWWINDOW);
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
