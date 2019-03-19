// PS2KeyboardTogglerTray.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "tray.h"

#include <shellapi.h>
#include <cguid.h>

#include "Keybitor.h"

#define MAX_LOADSTRING 100
#define MY_TRAY_ICON_ID 1<<3;
#define MY_TRAY_ICON_MESSAGE WM_APP+1
#define CTX_MENU_QUIT 1
#define CTX_MENU_TOGGLE 2
#define CTX_MENU_CONFIG 3

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HWND hWndMain;
NOTIFYICONDATA niData;
HMENU hwndMenu;
HICON iconOn;
HICON iconOff;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

void toggleDevices();
void updateIcon();
void ShowContextMenu();
void launchConfigApp();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PS2KEYBOARDTOGGLERTRAY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PS2KEYBOARDTOGGLERTRAY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PS2KEYBOARDTOGGLERTRAY);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   

   hInst = hInstance; // Store instance handle in our global variable

   hWndMain = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWndMain)
   {
      return FALSE;
   }

   iconOn = (HICON) LoadImage(hInstance,
     MAKEINTRESOURCE(IDI_PS2KEYBOARDTOGGLERTRAY),
     IMAGE_ICON,
     GetSystemMetrics(SM_CXSMICON),
     GetSystemMetrics(SM_CYSMICON),
     LR_DEFAULTCOLOR);

   iconOff = (HICON) LoadImage(hInstance,
     MAKEINTRESOURCE(IDI_PS2KEYBOARDTOGGLERTRAYOFF),
     IMAGE_ICON,
     GetSystemMetrics(SM_CXSMICON),
     GetSystemMetrics(SM_CYSMICON),
     LR_DEFAULTCOLOR);

   /*
   notify_icon example code courtesy of Abraxas23:
   https://www.codeproject.com/Articles/4768/Basic-use-of-Shell-NotifyIcon-in-Win32
   */



   // zero the structure - note: Some Windows funtions
   // require this but I can't be bothered to remember
   // which ones do and which ones don't.


   
   ZeroMemory(&niData, sizeof(NOTIFYICONDATA));

   niData.cbSize = sizeof(NOTIFYICONDATA);


   // the ID number can be any UINT you choose and will
   // be used to identify your icon in later calls to
   // Shell_NotifyIcon


   niData.uID = MY_TRAY_ICON_ID;


   // state which structure members are valid
   // here you can also choose the style of tooltip
   // window if any - specifying a balloon window:
   // NIF_INFO is a little more complicated 


   niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;


   // load the icon note: you should destroy the icon
   // after the call to Shell_NotifyIcon


   niData.hIcon = iconOn;

   // set the window you want to recieve event messages


   niData.hWnd = hWndMain;


   // set the message to send
   // note: the message value should be in the
   // range of WM_APP through 0xBFFF


   niData.uCallbackMessage = MY_TRAY_ICON_MESSAGE;

   // NIM_ADD adds a new tray icon
   Shell_NotifyIcon(NIM_ADD, &niData);
   
   updateIcon();

   hwndMenu = CreatePopupMenu();
   InsertMenu(hwndMenu, 3, MF_BYPOSITION|MF_STRING, CTX_MENU_QUIT, L"Quit");
   InsertMenu(hwndMenu, 2, MF_STRING, CTX_MENU_TOGGLE, L"Toggle");
   InsertMenu(hwndMenu, 1, MF_STRING, CTX_MENU_CONFIG, L"Config");

   //ShowWindow(hWndMain, nCmdShow);
   UpdateWindow(hWndMain);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWndMain, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
    case CTX_MENU_QUIT:
      Shell_NotifyIcon(NIM_DELETE, &niData);
      PostMessage(hWndMain, WM_QUIT, 0, 0);
      break;
    case CTX_MENU_CONFIG:
      launchConfigApp();
      break;
    case CTX_MENU_TOGGLE:
      toggleDevices();
      break;
		default:
			return DefWindowProc(hWndMain, message, wParam, lParam);
		}
		break;
  case MY_TRAY_ICON_MESSAGE:
    switch (lParam){
    case WM_LBUTTONDBLCLK:
      toggleDevices();
      break;
    case WM_RBUTTONDOWN:
      ShowContextMenu();
      break;
    }
    break;
	case WM_PAINT:
		hdc = BeginPaint(hWndMain, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWndMain, &ps);
		break;
	case WM_DESTROY:
    Shell_NotifyIcon(NIM_DELETE, &niData);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWndMain, message, wParam, lParam);
	}
	return 0;
}


void toggleDeviceInternal(const wchar_t* device, void* data){
  UNREFERENCED_PARAMETER(data);

  toggleDevice(device);
}

void toggleDevices(){
  foreachDevice(toggleDeviceInternal, NULL);
  updateIcon();
}


void checkDeviceToggledOff(const wchar_t* dev, void* data){
  bool& flag = *((bool*) data);
  flag |= !isDeviceEnable(dev);
}

bool checkDevicesToggledOff(){
  bool toggledOff = false;
  foreachDevice(checkDeviceToggledOff, (void*) &toggledOff);
  return toggledOff;
}

void updateIcon(){
  if (checkDevicesToggledOff()){
    niData.hIcon = iconOff;
    bool res = Shell_NotifyIcon(NIM_MODIFY, &niData);
  }
  else {
    niData.hIcon = iconOn;
    Shell_NotifyIcon(NIM_MODIFY, &niData);
  }
  UpdateWindow(hWndMain);
}

void ShowContextMenu(){
  SetForegroundWindow(hWndMain);
  NOTIFYICONIDENTIFIER iconId;
  iconId.cbSize = sizeof(NOTIFYICONIDENTIFIER);
  iconId.hWnd = hWndMain;
  iconId.uID = MY_TRAY_ICON_ID;
  iconId.guidItem = GUID_NULL;
  RECT iconPosition;
  HRESULT res = Shell_NotifyIconGetRect(&iconId, &iconPosition);
  TrackPopupMenu(hwndMenu, TPM_CENTERALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON | TPM_NOANIMATION, iconPosition.left, iconPosition.top, 0, hWndMain, NULL);
  Shell_NotifyIcon(NIM_SETFOCUS, &niData);
}

const wchar_t ConfigAppName[] = L"KeybitorConfig.exe";

void launchConfigApp(){
  TCHAR appFileName[MAX_PATH + 1];

  if (GetModuleFileName(NULL, appFileName, MAX_PATH + 1) > (MAX_PATH + 1)){
    return;
  }

  DWORD pathSize = MAX_PATH + sizeof(ConfigAppName);
  TCHAR* appPath = NULL;
  LPWSTR filePart = NULL;
  DWORD written = 0;

  appPath = (PTCHAR) malloc(pathSize * sizeof(TCHAR));
  memset(appPath, '\0', pathSize * sizeof(TCHAR));
  written = GetFullPathName(appFileName, pathSize, appPath, &filePart);
  if (written > pathSize){
    free(appPath);
    pathSize = written + sizeof(ConfigAppName) + 2;
    appPath = (PTCHAR) malloc(pathSize * sizeof(TCHAR));
    memset(appPath, '\0', pathSize * sizeof(TCHAR));
    written = GetFullPathName(appFileName, pathSize - 2, appPath, &filePart);
  }

  memset(filePart, 0, wcslen(filePart));
  memcpy(filePart, ConfigAppName, sizeof(ConfigAppName));
  STARTUPINFO startupInfo;
  ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
  startupInfo.cb = sizeof(STARTUPINFO);
  PROCESS_INFORMATION procInfo;
  if (CreateProcess(appPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &procInfo) == 0){
    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);

    MessageBox(hWndMain, messageBuffer, L"Error", MB_OK);

    LocalFree(messageBuffer);
  }

}