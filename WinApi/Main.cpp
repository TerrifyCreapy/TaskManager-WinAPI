#include <Windows.h>
#include "Definition.h" //All defines of func and vars
#include <Strsafe.h> //StringCchCat - does not allow you to overflow the buffer 
#include "TlHelp32.h" //Library for reading proccesses
#include <memory>
#include "stdafx.h"
#include "resource.h"
using namespace std;

static THREADS Thread;

int WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{

	MSG msg;
	HACCEL hAccelTable;

	TaskManagerClass(hInstance);


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
	TM.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON10));
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
	//require title and nameofWindow from resources
	LoadString(hInstance, TitleProg, title, 100);
	LoadString(hInstance, MainWindowName, mwName, 100);

	hWnd = CreateWindow(mwName, title, WS_OVERLAPPEDWINDOW,
		300, 200, 0, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}


	HANDLE processHandle, currentProcess;
	// structure which describe procces
	PROCESSENTRY32 processes;
	WCHAR buffer[255];
	// create processes screen
	processHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	processes.dwSize = sizeof(PROCESSENTRY32);

	HWND cpuUsageGraph = CreateWindow(L"static", L"", WS_CHILD | WS_VISIBLE,
		118, 130, 207, 200, hWnd, (HMENU)CPU_ID, hInstance, NULL);
	HWND memoryUsageGraph = CreateWindow(L"static", L"", WS_CHILD | WS_VISIBLE,
		218, 130, 310, 200, hWnd, (HMENU)RAM_ID, hInstance, NULL);
	HWND edit = CreateWindow(L"static", L"", WS_CHILD | WS_VISIBLE,
		110, 200, 400, 200, hWnd, (HMENU)ID_EDIT, hInstance, NULL);
	HWND info = CreateWindow(L"static", L"        ID                 Thread                                 Name", WS_CHILD | WS_VISIBLE,
		390, 10, 450, 20, hWnd, (HMENU)ID_EDIT, hInstance, NULL);
	HWND Refresh = CreateWindow(L"button", L"Refresh",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		740, 385,
		100, 30,
		hWnd,
		(HMENU)Refreshed,
		hInstance, NULL);
	HWND hListBox = CreateWindow(L"listbox", L"",
		WS_CHILD | WS_VISIBLE | LBS_STANDARD |
		LBS_WANTKEYBOARDINPUT,
		390, 30, 450, 360,
		hWnd, (HMENU)LIST1, hInst, NULL);


	if (Process32First(processHandle, &processes))
	{
		do
		{
			wsprintfW(buffer, L" ID [%05d]      Threads [%03d] %30s", processes.th32ProcessID, processes.cntThreads, processes.szExeFile);
			SendMessageW(hListBox, LB_ADDSTRING, 0, (WPARAM)buffer);
		} while (Process32Next(processHandle, &processes));
	}
	CloseHandle(processHandle);

	//Creating a multi-thread for autoupdating window
	Thread.handleThread = NULL;
	Thread.ThreadFunction = Thread_InfoSystem;
	Thread.handleDialog = hWnd;
	Thread.time = 0;
	Thread.handleThread = CreateThread(NULL, 0, Thread.ThreadFunction, NULL, NULL, &Thread.threadId);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK CourseWorkProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	HDC hdc;
	PAINTSTRUCT ps;
	HPEN hpen, hpen_cpu_graph, hpen_mem_graph;
	RECT rect, rect_cpu, rect_cpu_graph, rect_mem, rect_mem_update, rect_mem_graph, rect_window;
	int CommandID = LOWORD(wp);
	POINT Min;  //Min size of main window
	POINT Max;	//Max size of main window
	MINMAXINFO* pInfo;



	switch (msg) {
	case WM_GETMINMAXINFO:

		pInfo = (MINMAXINFO*)lp;
		Min = { 890, 480 };
		Max = Min;
		pInfo->ptMinTrackSize = Min;
		pInfo->ptMaxTrackSize = Max;
		return 0;
	case WM_COMMAND:
		switch (CommandID) {
		case OnMenuAbout: {
			MessageBoxA(hWnd, "This task manager was developed as course work BMSTU IPS-41B", "ABOUT", MB_OK);
			break;
		}
		case Refreshed: {

			SendDlgItemMessage(hWnd, LIST1, LB_RESETCONTENT, 0, 0);

			HANDLE processHandle, currentProcess;
			PROCESSENTRY32 processes;
			WCHAR buffer[255];
			processHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			processes.dwSize = sizeof(PROCESSENTRY32);

			if (Process32First(processHandle, &processes))
			{
				do
				{
					wsprintfW(buffer, L" ID [%05d]      Threads [%03d] %30s", processes.th32ProcessID, processes.cntThreads, processes.szExeFile);
					SendDlgItemMessage(hWnd, LIST1, LB_ADDSTRING, 0, (WPARAM)buffer);
				} while (Process32Next(processHandle, &processes));
			}
			CloseHandle(processHandle);
			break;
		}
		case LIST1: {
			CHAR str[255];
			iSelect = SendDlgItemMessage(hWnd, LIST1, LB_GETCURSEL, 0, 0);
			SendDlgItemMessage(hWnd, LIST1, LB_GETTEXT, iSelect, (LPARAM)str);

			int idProc = (((int)str[10] - 48) * 10000 + ((int)str[12] - 48) * 1000 + ((int)str[14] - 48) * 100 + ((int)str[16] - 48) * 10 + ((int)str[18] - 48));

			if (iSelect >= 0)
			{
				GetCursorPos(&p);

				hPopupMenu = CreatePopupMenu();

				InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1, L"Terminate");
				InsertMenu(hPopupMenu, 1, MF_BYPOSITION | MF_STRING, 2, L"Close");

				int choice = TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, p.x, p.y + 23, 0, hWnd, NULL);

				switch (choice) {
				case 1: {
					_killProcess(idProc);
					SendDlgItemMessage(hWnd, LIST1, LB_DELETESTRING, (WPARAM)iSelect, 0);
					break;
				}
				case 2: {
					DestroyMenu(hPopupMenu);
				}
				default: break;
				}

				DestroyMenu(hPopupMenu);
			}
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
	case WM_PAINT:
		GetClientRect(hWnd, &rect_window);
		hdc = BeginPaint(hWnd, &ps);

		SetRect(&rect, 110, 10, 200, 130);
		if (cpu >= 0)
		{
			SetRect(&rect_cpu, 113, 120 - cpu, 197, 127);
		}


		SetRect(&rect_mem, 210, 10, 300, 130);
		SetRect(&rect_mem_update, 213, rect_mem.bottom - memory, 297, rect_mem.bottom - 3);

		FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 0, 0)));
		FillRect(hdc, &rect_mem, CreateSolidBrush(RGB(0, 0, 0)));
		FillRect(hdc, &rect_mem_update, CreateSolidBrush(RGB(0, 255, 0)));
		FillRect(hdc, &rect_cpu, CreateSolidBrush(RGB(0, 255, 0)));

		EndPaint(hWnd, &ps);

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
	AppendMenu(RootMenu, MF_STRING, OnMenuAbout, L"About");
	AppendMenu(RootMenu, MF_STRING, IDM_EXIT, L"Exit");

	SetMenu(hwnd, RootMenu);
}

BOOL _killProcess(int procID)
{
	HANDLE hProcessSnap;
	HANDLE* hProcess;


	PROCESSENTRY32 pe32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	pe32.dwSize = sizeof(PROCESSENTRY32);

	Process32First(hProcessSnap, &pe32);

	while (Process32Next(hProcessSnap, &pe32))
	{
		if (procID == pe32.th32ProcessID)
		{
			hProcess = (HANDLE*)OpenProcess(PROCESS_TERMINATE, 0, pe32.th32ProcessID);

			TerminateProcess(hProcess, 0);
		}
	}
	CloseHandle(hProcessSnap);

	return 0;
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
			}
			if (cpu >= 0) {
				wsprintfW(cpu_char, L"      %d %%\nCPU Usage", (int)cpu);
				SetDlgItemText(Thread.handleDialog, CPU_ID, cpu_char);
			}
		}


		MEMORYSTATUSEX statex;

		statex.dwLength = sizeof(statex);

		GlobalMemoryStatusEx(&statex);

		wsprintf(mem_char, L"      %d %%\nRAM Usage\n  %d MB", (int)memory, (int)((statex.ullTotalPhys - statex.ullAvailPhys) / DIV));
		SetDlgItemText(Thread.handleDialog, RAM_ID, mem_char);

		wsprintfW(temp, L"%d %% - CPU usage\r\n", (int)cpu);
		StringCchCat(buffer, sizeof(temp), temp);

		wsprintfW(buffer, L"%ld %% - RAM uses\r\n", statex.dwMemoryLoad);
		memory = statex.dwMemoryLoad;


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