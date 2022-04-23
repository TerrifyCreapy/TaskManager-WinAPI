#pragma once
#define DIV 1048576	// = 1MB
#define ID_EDIT 2002
#define SIZE 400
#define GB 1073741824
#define ID_CPU 12

TCHAR mwName[100];
TCHAR title[100];


HINSTANCE hInst;	

FILETIME idleTime;
FILETIME kernelTime;
FILETIME userTime;

FILETIME last_idleTime;
FILETIME last_kernelTime;
FILETIME last_userTime;



WCHAR cpu_char[6] = { ' ' };
WCHAR mem_char[8] = { ' ' };
WCHAR buffer[8068] = { ' ' };
WCHAR temp[8068] = { ' ' };
double cpu = -1, memory = -1;
int cpu_mas[SIZE] = { -1 }, mem_mas[SIZE] = { -1 };

int iSelect;
POINT p;
HMENU hPopupMenu;


LRESULT CALLBACK CourseWorkProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
BOOL InitializationInstance(HINSTANCE hInstance, int nCmdShow);
ATOM TaskManagerClass(HINSTANCE hInstance);
BOOL _killProcess(int procID);

void MainMenuAdd(HWND hwnd);


DWORD WINAPI Thread_InfoSystem(LPVOID lpParam);