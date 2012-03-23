#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#pragma comment(lib, "user32.lib")

static const TCHAR szWindowClass[] = _T("XWindowsDockCore64");

HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static HMODULE hModule = NULL;
static HOOKPROC MouseProc = NULL;
static HOOKPROC WindowProc = NULL;
static HOOKPROC CBTProc = NULL;
static HHOOK MouseHook = NULL;
static HHOOK WindowHook = NULL;
static HHOOK CBTHook = NULL;

static HANDLE hMutex = NULL;

void Start()
{
	hModule = LoadLibrary(L"XWDCore64.dll");
	if (!hModule)
	{
		return;
	}

	MouseProc = (HOOKPROC)GetProcAddress(hModule, "MouseProc");
	WindowProc = (HOOKPROC)GetProcAddress(hModule, "WindowProc");
	CBTProc = (HOOKPROC)GetProcAddress(hModule, "CBTProc");

	MouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc, hModule, NULL);
	WindowHook = SetWindowsHookEx(WH_CALLWNDPROCRET, WindowProc, hModule, NULL);
	CBTHook = SetWindowsHookEx(WH_CBT, CBTProc, hModule, NULL);
}

void Stop()
{
	if (!hModule)
	{
		return;
	}

	UnhookWindowsHookEx(MouseHook);
	UnhookWindowsHookEx(WindowHook);
	UnhookWindowsHookEx(CBTHook);

	FreeLibrary(hModule);
	hModule = NULL;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int/* nCmdShow*/)
{
	hMutex = CreateMutex(NULL, FALSE, L"XWDCore64AlreadyRunning");
	if((hMutex == 0) || (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		return 1;
	}

    WNDCLASSEX wcex;
    wcex.cbSize			= sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&wcex))
    {
        return 1;
    }

    hInst = hInstance;

    HWND hWnd = CreateWindow(szWindowClass, NULL, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return 1;
    }

	Start();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	Stop();

	CloseHandle(hMutex);
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}