#include <Windows.h>
#include "Definition.h"
#include <stdio.h>
#include <string>
#include <WinUser.h>
#include "Commdlg.h"
#include<Strsafe.h>
#include "TlHelp32.h"
#include <string.h>
#include <stdint.h>
#include <memory>
#include "stdafx.h"
#include "resource.h"
using namespace std;


static THREADS Thread;



INT_PTR CALLBACK ProcessList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG: {
		HWND hList = GetDlgItem(hDlg, LIST1);
		HANDLE processHandle, currentProcess;
		PROCESSENTRY32 processes;
		WCHAR buffer[255];
		processHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		processes.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(processHandle, &processes))
		{
			do
			{
				wsprintfW(buffer, L" ID [%05d]           Threads [%03d] %30s", processes.th32ProcessID, processes.cntThreads, processes.szExeFile);

				SendMessageW(hList, LB_ADDSTRING, 0, (WPARAM)buffer);
			} while (Process32Next(processHandle, &processes));
		}
		CloseHandle(processHandle);

		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


int WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	TaskManagerClass(hInstance);

	// Perform application initialization:
	if (!InitializationInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}


ATOM TaskManagerClass(HINSTANCE hInstance)
{
	WNDCLASSEX TM;

	TM.cbSize = sizeof(WNDCLASSEX);

	TM.style = CS_HREDRAW | CS_VREDRAW;
	TM.lpfnWndProc = CourseWorkProc;
	TM.cbClsExtra = 0;
	TM.cbWndExtra = 0;
	TM.hInstance = hInstance;
	TM.hIcon = NULL;
		//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	TM.hCursor = LoadCursor(NULL, IDC_ARROW);
	TM.hbrBackground = (HBRUSH)(COLOR_WINDOW);


	TM.lpszMenuName = MAKEINTRESOURCE(TitleProg);
	TM.lpszClassName = L"Task manager";
	TM.hIconSm = NULL;

	return RegisterClassEx(&TM);
}

BOOL InitializationInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance;

	LoadString(hInstance, TitleProg, title, 100);
	LoadString(hInstance, MainWindowName, mwName, 100);

	hWnd = CreateWindow(mwName, title, WS_OVERLAPPEDWINDOW,
		300, 200, 0, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	HWND edit = CreateWindow(L"static", L"", WS_CHILD | WS_VISIBLE,
		110, 260, 400, 200, hWnd, (HMENU)ID_EDIT, hInstance, NULL);

	Thread.handleThread = NULL;
	Thread.ThreadFunction = Thread_InfoSystem;
	Thread.handleDialog = hWnd;
	Thread.time = 0;
	Thread.handleThread = CreateThread(NULL, 0, Thread.ThreadFunction, NULL, NULL, &Thread.threadId);

	for (int i = SIZE - 1; i >= 0; i--) {
		cpu_mas[i] = -1;
		mem_mas[i] = -1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK CourseWorkProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	int CommandID = LOWORD(wp);
	POINT Min;
	MINMAXINFO* pInfo;
	switch (msg) {
	case WM_GETMINMAXINFO:

		pInfo = (MINMAXINFO*)lp;
		Min = { 650, 450 };
		pInfo->ptMinTrackSize = Min;
		return 0;
	case WM_COMMAND:
		switch (CommandID) {
		case ID_PROCESSES_OPEN: {
			DialogBox(hInst, MAKEINTRESOURCE(ID_PROCESSES_OPEN), hWnd, ProcessList);
			break;
		}
		case IDM_EXIT: {
			PostQuitMessage(0);
		}
		break;
		}
		break;
	case WM_CREATE:
		MainMenuAdd(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wp, lp);
	}
}


void MainMenuAdd(HWND hwnd) {
	HMENU RootMenu = CreateMenu();
	HMENU SubMenu = CreateMenu();
	AppendMenu(SubMenu, MF_STRING, ID_PROCESSES_OPEN, L"Main");
	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"File");
	AppendMenu(RootMenu, MF_STRING, IDM_EXIT, L"Exit");

	SetMenu(hwnd, RootMenu);
}

DWORD WINAPI Thread_InfoSystem(LPVOID lpParam)
{
	THREADS* ti = (THREADS*)lpParam;
	GetSystemTimes(&last_idleTime, &last_kernelTime, &last_userTime);
	for (;;)
	{
		if (GetSystemTimes(&idleTime, &kernelTime, &userTime) != 0)
		{

			double usr = userTime.dwLowDateTime - last_userTime.dwLowDateTime;
			double ker = kernelTime.dwLowDateTime - last_kernelTime.dwLowDateTime;
			double idl = idleTime.dwLowDateTime - last_idleTime.dwLowDateTime;

			double sys = ker + usr;

			last_idleTime = idleTime;
			last_userTime = userTime;
			last_kernelTime = kernelTime;

			if (sys != 0) {
				cpu = (sys - idl) / sys * 100;

				for (int i = SIZE - 1; i > 0; i--) {
					cpu_mas[i] = cpu_mas[i - 1];
				}
				cpu_mas[0] = cpu;
			}
		}

		MEMORYSTATUSEX statex;

		statex.dwLength = sizeof(statex);

		GlobalMemoryStatusEx(&statex);
		
		wsprintfW(temp, L"%d %% - CPU usage\r\n", (int)cpu);
		StringCchCat(buffer, sizeof(temp), temp);

		wsprintfW(buffer, L"%ld %% - RAM uses\r\n", statex.dwMemoryLoad);
		memory = statex.dwMemoryLoad;

		for (int i = SIZE - 1; i > 0; i--) {
			mem_mas[i] = mem_mas[i - 1];
		}
		mem_mas[0] = memory;

		
		wsprintfW(temp, L"%ld - total physical memory (RAM)\r\n", statex.ullTotalPhys / DIV);
		StringCchCat(buffer, sizeof(temp), temp);
		wsprintfW(temp, L"%ld - free physical memory (RAM)\r\n", statex.ullAvailPhys / DIV);
		StringCchCat(buffer, sizeof(temp), temp);
		wsprintfW(temp, L"%ld - total virtual memory\r\n", statex.ullTotalVirtual / DIV);
		StringCchCat(buffer, sizeof(temp), temp);
		wsprintfW(temp, L"%ld - free virtual memory.\r\n", statex.ullAvailVirtual / DIV);
		StringCchCat(buffer, sizeof(temp), temp);

		SetDlgItemText(Thread.handleDialog, ID_EDIT, buffer);

		InvalidateRect(Thread.handleDialog, NULL, TRUE);
		Sleep(1000);
	}

}


