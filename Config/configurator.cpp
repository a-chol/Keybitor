// PS2KeyboardTogglerConfig.cpp : Defines the entry point for the application.
//

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <initguid.h>

#include "resource.h"

#include "Keybitor.h"

#include "difxapi.h"
#include "newdev.h"
#include <windowsx.h>

#include <vector>
#include <sstream>

//enable visual styles for common controls
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static wchar_t DriverInfName [] = L"Keybitor.inf";
static wchar_t TrayExeName [] = L"KeybitorTray.exe";


DEFINE_GUID(GUID_DEVINTERFACE_KBFILTER,
  0x3fb7299d, 0x6847, 0x4490, 0xb0, 0xc9, 0x99, 0xe0, 0x98, 0x6a, 0xb8, 0x86);
// {3FB7299D-6847-4490-B0C9-99E0986AB886}

#define MAX_LOADSTRING 100

#define CHECK_ELEV WM_USER+1

#ifdef _WIN64
#define GWL_HINSTANCE_PTF GWLP_HINSTANCE
#else
#define GWL_HINSTANCE_PTF GWL_HINSTANCE
#endif
// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

HWND hwndMainWindow;
HWND hwndInstallButton;
HWND hwndToggleButton;
HWND hwndPersistButton;
HWND hwndTrayButton;


void updateKeyboardToggleGUI();
void updateDriverInstallGUI();
void updatePersistenceGUI();
void updateTraybarInstallGUI();

void checkNeedsElev();
void toggleDeviceInternal(const wchar_t* device, void*);
void checkDeviceReachable(const wchar_t* dev, void* data);
void checkDeviceToggledOff(const wchar_t* dev, void* data);
bool checkDevicesToggledOff();
bool isDriverInstalled();
bool installDriver();
bool uninstallDriver();
void enablePersistence();
void disablePersistence();
bool isPersistenceEnabled();
bool isTrayInstalled();
void uninstallTray();
void installTray();
wchar_t* getLocalDriverInfFile();
wchar_t* getLocalTrayExe();
void notifyErrorMessage(DWORD);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PS2KEYBOARDTOGGLERCONFIG, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PS2KEYBOARDTOGGLERCONFIG));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PS2KEYBOARDTOGGLERCONFIG));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PS2KEYBOARDTOGGLERCONFIG);
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 300, 330, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   hwndMainWindow = hWnd;

   hwndToggleButton = CreateWindow(
     L"BUTTON", L"Toggle keyboard", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
     10, 30, 260, 80,
     hWnd, NULL, (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE_PTF), NULL);

   HWND hwndConfigFrame = CreateWindow(L"BUTTON", L"Configuration", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_GROUPBOX ,
     5, 120, 270, 166,
     hWnd, NULL, (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE_PTF), NULL);

   /*HWND hwndConfigLabel = CreateWindow(L"STATIC", L"Configuration", WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP,
     10, 115, 100, 20,
     hWnd, NULL, (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE_PTF), NULL);*/
   SetWindowText(hwndConfigFrame, L"Configuration");

   hwndInstallButton = CreateWindow(
     L"BUTTON", L"Install driver", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
     10, 140, 260, 40,
     hWnd, NULL, (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE_PTF), NULL);

   hwndPersistButton = CreateWindow(
     L"BUTTON", L"Enable persistence", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
     10, 190, 260, 40,
     hWnd, NULL, (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE_PTF), NULL);
   
   hwndTrayButton = CreateWindow(
     L"BUTTON", L"Launch tray widget at startup", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
     10, 240, 260, 40,
     hWnd, NULL, (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE_PTF), NULL);

   if (!(hwndToggleButton && hwndPersistButton && hwndTrayButton))
   {
     return FALSE;
   }

   updateKeyboardToggleGUI();
   updateDriverInstallGUI();
   updatePersistenceGUI();
   updateTraybarInstallGUI();

   PostMessage(hWnd, CHECK_ELEV, 0, 0);

   

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

    if (((HWND) lParam) == hwndToggleButton){
      if (wmEvent == BN_CLICKED){
        foreachDevice(toggleDeviceInternal, NULL);
      }
      updateKeyboardToggleGUI();
    }
    else if (((HWND) lParam) == hwndPersistButton){
      if (isPersistenceEnabled()){
        disablePersistence();
      }
      else {
        enablePersistence();
      }
    }
    else if (((HWND) lParam) == hwndInstallButton){
      if (isDriverInstalled()){
        uninstallDriver();
      }
      else {
        installDriver();
      }
    }
    else if (((HWND) lParam) == hwndTrayButton){
      if (isTrayInstalled()){
        uninstallTray();
      }
      else {
        installTray();
      }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
  case CHECK_ELEV:
    checkNeedsElev();
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void updateKeyboardToggleGUI(){
  if (!isDriverInstalled()){
    Button_Enable(hwndToggleButton, FALSE);
  }
  else {
    Button_Enable(hwndToggleButton, TRUE);
  }
  if (checkDevicesToggledOff()){
    Button_SetText(hwndToggleButton, L"Enable PS2 keyboards");
  }
  else {
    Button_SetText(hwndToggleButton, L"Disable PS2 keyboards");
  }
}

void updateDriverInstallGUI(){
  if (isDriverInstalled()){
    Button_SetText(hwndInstallButton, L"Uninstall driver");
  }
  else {
    Button_SetText(hwndInstallButton, L"Install driver");
  }
}

void updatePersistenceGUI(){
  if (!isDriverInstalled()){
    Button_Enable(hwndPersistButton, FALSE);
  }
  else {
    Button_Enable(hwndPersistButton, TRUE);
  }
  if (isPersistenceEnabled()){
    Button_SetCheck(hwndPersistButton, BST_CHECKED);
  }
  else {
    Button_SetCheck(hwndPersistButton, BST_UNCHECKED);
  }
}

void updateTraybarInstallGUI(){
  if (!isDriverInstalled()){
    Button_Enable(hwndTrayButton, FALSE);
  }
  else {
    Button_Enable(hwndTrayButton, TRUE);
  }
  if (isTrayInstalled()){
    Button_SetCheck(hwndTrayButton, BST_CHECKED);
  }
  else {
    Button_SetCheck(hwndTrayButton, BST_UNCHECKED);
  }
}


void toggleDeviceInternal(const wchar_t* device, void* data){
  UNREFERENCED_PARAMETER(data);

  if (toggleDevice(device) < 0){
    MessageBox(hwndMainWindow, L"Device toggle didn't properly work.", L"Error", MB_OK);
    
  }
}

void checkNeedsElev(){
  bool is_error = 0;
  foreachDevice(checkDeviceReachable, (void*) &is_error);
  if (is_error){
    MessageBox(hwndMainWindow, L"Could not read the device. Either the driver is not installed, or this software (and related) have to be \"Run as administrator\".", L"Error", MB_OK);
  }
}

void checkDeviceReachable(const wchar_t* dev, void* data){
  bool& error_flag = *((bool*) data);
  error_flag |= isDeviceEnable(dev) < 0;
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

bool isDriverInstalled(){

  wchar_t* InfPath = getLocalDriverInfFile();

  DWORD NumOfChars = 0;
  DWORD ReturnCode = ERROR_SUCCESS;

  ReturnCode = DriverPackageGetPath(InfPath, NULL, &NumOfChars);

  PTCHAR DriverStoreInfPath = (PTCHAR) LocalAlloc(LPTR, NumOfChars * sizeof(TCHAR));
  ReturnCode = DriverPackageGetPath(InfPath, DriverStoreInfPath, &NumOfChars);
  
  LocalFree(InfPath);

  if (ReturnCode == ERROR_SUCCESS){
    return true;
  }
  
  return false;
}

bool installDriver(){
  wchar_t* infFile = getLocalDriverInfFile();
  if (infFile == NULL){
    return false;
  }
  DWORD res = UpdateDriverForPlugAndPlayDevices(hwndMainWindow, L"*MSF0001", infFile, INSTALLFLAG_FORCE, NULL);
  
  if (res != ERROR_SUCCESS){
    notifyErrorMessage(res);
    updateDriverInstallGUI();
    return false;
  }

  updateDriverInstallGUI();

  return true;
}

bool uninstallDriver(){
  BOOL needReboot = false;
  HDEVINFO dis = SetupDiGetClassDevs((LPGUID) &GUID_DEVINTERFACE_KBFILTER, NULL, NULL, (DIGCF_PRESENT |DIGCF_DEVICEINTERFACE));
  if (INVALID_HANDLE_VALUE == dis){
    notifyErrorMessage(GetLastError());
    return false;
  }

  SP_DEVINFO_DATA did;
  did.cbSize = sizeof(did);
  int devIndex;
  for (devIndex = 0; SetupDiEnumDeviceInfo(dis, devIndex, &did) && GetLastError() != ERROR_NO_MORE_ITEMS; ++devIndex){

    BOOL localNeedReboot = false;
    DWORD res = DiRollbackDriver(dis, &did, hwndMainWindow, 0, &localNeedReboot);

    if (!res){
      if (GetLastError() != ERROR_NO_MORE_ITEMS){
        notifyErrorMessage(GetLastError());
      }
      else {
        DWORD res = UpdateDriverForPlugAndPlayDevices(hwndMainWindow, L"*MSF0001", L"C:\\Windows\\inf\\keyboard.inf", INSTALLFLAG_FORCE, &needReboot);
      }
    }

    needReboot |= localNeedReboot;
  }

  if (devIndex == 0){//no device enumerated
    notifyErrorMessage(GetLastError());
  }

  wchar_t* InfPath = getLocalDriverInfFile();

  DWORD NumOfChars = 0;
  DWORD ReturnCode = ERROR_SUCCESS;

  ReturnCode = DriverPackageGetPath(InfPath, NULL, &NumOfChars);

  PTCHAR DriverStoreInfPath = (PTCHAR) LocalAlloc(LPTR, NumOfChars * sizeof(TCHAR));
  ReturnCode = DriverPackageGetPath(InfPath, DriverStoreInfPath, &NumOfChars);

  DWORD reqSize = 0;
  SetupGetInfPublishedName(DriverStoreInfPath, NULL, 0, &reqSize);
  WCHAR * buffer = (PTCHAR) LocalAlloc(LPTR, reqSize * sizeof(TCHAR));
  SetupGetInfPublishedName(DriverStoreInfPath, buffer, reqSize, NULL);
  SetupUninstallOEMInf(buffer, 0, NULL);

  LocalFree(buffer);
  LocalFree(DriverStoreInfPath);
  LocalFree(InfPath);
  if (needReboot){
    MessageBox(hwndMainWindow, L"The Keybitor driver has been removed.\nA reboot is required to load the previous driver back.", L"Reboot required", MB_OK);
  }

  //clean registry
  LSTATUS res = RegDeleteKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\services\\Keybitor\\persist", KEY_WOW64_64KEY, 0);
  if (res != 0){
    notifyErrorMessage(res);
  }
  
  uninstallTray();

  updateKeyboardToggleGUI();
  updateDriverInstallGUI();
  updatePersistenceGUI();
  updateTraybarInstallGUI();
  
  return true;
}

bool setPersistenceRegistry(bool enable, bool value){
  HKEY persistKeyHandle;
  DWORD dispo;
  LSTATUS res = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\services\\Keybitor\\persist", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &persistKeyHandle, &dispo);

  if (res != ERROR_SUCCESS){
    notifyErrorMessage(res);
    updatePersistenceGUI();
    return false;
  }

  DWORD enableEntry = enable;
  res = RegSetValueEx(persistKeyHandle, L"enable", 0, REG_DWORD, (BYTE*) &enableEntry, sizeof(DWORD));
  if (res != ERROR_SUCCESS){
    notifyErrorMessage(res);
    updatePersistenceGUI();
    return false;
  }

  DWORD valueEntry = value;
  res = RegSetValueEx(persistKeyHandle, L"value", 0, REG_DWORD, (BYTE*) &valueEntry, sizeof(DWORD));
  if (res != ERROR_SUCCESS){
    notifyErrorMessage(res);
    updatePersistenceGUI();
    return false;
  }

  return true;
}

void enablePersistence(){
  setPersistenceRegistry(true, checkDevicesToggledOff());
  updatePersistenceGUI();
}

void disablePersistence(){
  setPersistenceRegistry(false, false);
  updatePersistenceGUI();
}

bool isPersistenceEnabled(){
  DWORD persistEnabled = 0;
  DWORD valSize = sizeof(DWORD);
  DWORD type;
  LSTATUS res = RegGetValue(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\services\\Keybitor\\persist", L"enable", RRF_RT_REG_DWORD, &type, &persistEnabled, &valSize);

  if (res == ERROR_FILE_NOT_FOUND){
    return false;
  }

  if (res != ERROR_SUCCESS){
    notifyErrorMessage(res);
    return false;
  }

  return persistEnabled==1;
}

bool isTrayInstalled(){
  DWORD type;
  std::vector<char*> data(300, '\0');
  DWORD valSize = (DWORD)data.size();
  LSTATUS res = RegGetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"KibitorTray", RRF_RT_REG_SZ, &type, (void*) &data[0], &valSize);
  
  if (res == ERROR_FILE_NOT_FOUND){
    return false;
  }

  while (res == ERROR_MORE_DATA){
    data.insert(data.end(), data.size(), '\0');
    valSize = (DWORD)data.size();
    res = RegGetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"KibitorTray", RRF_RT_REG_SZ, &type, (void*) &data[0], &valSize);
  }

  if (res != ERROR_SUCCESS){
    notifyErrorMessage(res);
    return false;
  }

  return true;
}

void uninstallTray(){
  LSTATUS res = RegDeleteKeyValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"KibitorTray");

  if (res != ERROR_SUCCESS){
    notifyErrorMessage(res);
  }
  updateTraybarInstallGUI();
}

void installTray(){
  LPCWSTR trayExeW = getLocalTrayExe();
  LSTATUS res = RegSetKeyValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"KibitorTray", REG_SZ, trayExeW, (DWORD)(wcslen(trayExeW)*sizeof(wchar_t)));
  if (res != ERROR_SUCCESS){
    notifyErrorMessage(res);
  }
  updateTraybarInstallGUI();
}

template <size_t Size>
wchar_t* getLocalFile(const wchar_t (&file)[Size]){
  TCHAR appFileName[MAX_PATH + 1];

  if (GetModuleFileName(NULL, appFileName, MAX_PATH + 1) > (MAX_PATH + 1)){
    return false;
  }

  DWORD pathSize = MAX_PATH + Size;
  TCHAR* appPath = NULL;
  LPWSTR filePart = NULL;
  DWORD written = 0;

  appPath = (PTCHAR) malloc(pathSize * sizeof(TCHAR));
  memset(appPath, '\0', pathSize * sizeof(TCHAR));
  written = GetFullPathName(appFileName, pathSize, appPath, &filePart);
  if (written > pathSize){
    free(appPath);
    pathSize = written + Size + 2;
    appPath = (PTCHAR) malloc(pathSize * sizeof(TCHAR));
    memset(appPath, '\0', pathSize * sizeof(TCHAR));
    written = GetFullPathName(appFileName, pathSize - 2, appPath, &filePart);
  }

  memset(filePart, 0, wcslen(filePart)*sizeof(wchar_t));
  memcpy(filePart, file, Size*sizeof(wchar_t));

  return appPath;
}

wchar_t* getLocalDriverInfFile(){

  return getLocalFile(DriverInfName);
}

wchar_t* getLocalTrayExe(){

  return getLocalFile(TrayExeName);
}

void notifyErrorMessage(DWORD code = ERROR_SUCCESS){

  if (code == ERROR_SUCCESS){
    code = GetLastError();
    if (code == 0)
      return;
  }

  LPWSTR messageBuffer = nullptr;
  size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &messageBuffer, 0, NULL);

  MessageBox(hwndMainWindow, messageBuffer, L"Error", MB_OK);

  LocalFree(messageBuffer);
}