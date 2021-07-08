//
// App:         UPS Monitoring Service
// Author:      Mirco Caramori
// Copyright:   (c) 2021 Mirco Caramori
// Repository:  https://github.com/padus/vacation
//
// Description: application source
//

// Includes --------------------------------------------------------------------------------------------------------------------

#include <system.hpp>

using namespace std;

#define FORMAT_BUFFER_SIZE    512

// Source ----------------------------------------------------------------------------------------------------------------------

int FormatString(string& str, const char* format, ...) {
  //
  // Format a string similarly to sprintf
  // Return 0 if successful, a negative error otherwise
  //
  char buffer[FORMAT_BUFFER_SIZE];
  va_list args;

  va_start(args, format);
  int ret = vsnprintf(buffer, FORMAT_BUFFER_SIZE, format, args);
  va_end(args);  

  if (ret < 0) str.clear();
  else {
    str.assign(buffer, (size_t)ret);
    ret = 0;
  }

  return (ret);
}

// -------------------------------------------------------------

int FormatStringW(wstring& str, const wchar_t* format, ...) {
  //
  // Format a string similarly to sprintf
  // Return 0 if successful, a negative error otherwise
  //
  wchar_t buffer[FORMAT_BUFFER_SIZE];
  va_list args;

  va_start(args, format);
  int ret = vswprintf(buffer, FORMAT_BUFFER_SIZE, format, args);
  va_end(args);  

  if (ret < 0) str.clear();
  else {
    str.assign(buffer, (size_t)ret);
    ret = 0;
  }

  return (ret);
}

// -------------------------------------------------------------

int SendPostRequest(wstring& msg, const wchar_t* host, INTERNET_PORT port, string& json) {
  //
  // Send a POST request to Hubitat
  // Return a Windows error, -1 if we get an invalid http status, 0 if successful 
  //
  int ret = 0;

  HINTERNET session = nullptr;
  HINTERNET connect = nullptr;
  HINTERNET request = nullptr;

  // Open a session
  session = WinHttpOpen(L"UPS Monitoring Service/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (session == nullptr) {
    ret = (int)GetLastError();
    FormatStringW(msg, L"WinHttpOpen() error: %d", ret); 
  }
  else {
    // Specify an HTTP server
    connect = WinHttpConnect(session, host, port, 0);
    if (connect == nullptr) {
      ret = (int)GetLastError();
      FormatStringW(msg, L"WinHttpConnect() error: %d", ret); 
    }
    else {
      // Create an HTTP request handle.
      request = WinHttpOpenRequest(connect, L"POST", nullptr, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
      if (request == nullptr) {
        ret = (int)GetLastError();
        FormatStringW(msg, L"WinHttpOpenRequest() error: %d", ret); 
      }
      else {
        // Send a request
        if (WinHttpSendRequest(request, L"Content-Type: application/json", (DWORD)-1, (LPVOID)json.c_str(), (DWORD)json.length(), (DWORD)json.length(), 0) == false) {
          ret = (int)GetLastError();
          FormatStringW(msg, L"WinHttpSendRequest() error: %d", ret); 
        }
        else {
          // Wait for response
          if (WinHttpReceiveResponse(request, nullptr) == false) {
            ret = (int)GetLastError();
            FormatStringW(msg, L"WinHttpReceiveResponse() error: %d", ret); 
          }
          else {
            // Query return status code
            unsigned long code;
            unsigned long length = sizeof(code);
            if (WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, nullptr, &code, &length, nullptr) == false) {
              ret = (int)GetLastError();
              FormatStringW(msg, L"WinHttpQueryHeaders() error: %d", ret); 
            }
            else if (code != HTTP_STATUS_OK) {
              ret = -1;
              FormatStringW(msg, L"SendPostRequest() code: %d", code);               
            }
          }
        }
      }
    }
  }

  // Close any open handles.
  if (request) WinHttpCloseHandle(request);
  if (connect) WinHttpCloseHandle(connect);
  if (session) WinHttpCloseHandle(session);

  return (ret);
}

// -------------------------------------------------------------

struct AppContext {
  wstring hubitatAddress;
  INTERNET_PORT hubitatPort;
};

// -------------------------------------------------------------

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  
  //
  // Get the app context
  //
  AppContext* ctx;
  if (uMsg == WM_CREATE) {
    CREATESTRUCT *create = reinterpret_cast<CREATESTRUCT*>(lParam);
    ctx = reinterpret_cast<AppContext*>(create->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ctx);
  }
  else {
    LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    ctx = reinterpret_cast<AppContext*>(ptr);
  }

  switch (uMsg) {
  case WM_POWERBROADCAST:
    if (wParam == PBT_POWERSETTINGCHANGE) {
      //
      // GUID_ACDC_POWER_SOURCE: DWORD
      //   0: AC power source
      //   1: Onboard battery power source
      //   2: UPS device
      //
      // GUID_BATTERY_PERCENTAGE_REMAINING: DWORD
      //   0-100: percentage remaining
      //
      int ret = 0;
      wstring msg;
      string json;
      POWERBROADCAST_SETTING& pwr = *reinterpret_cast<POWERBROADCAST_SETTING*>(lParam);

      if (pwr.PowerSetting == GUID_ACDC_POWER_SOURCE) {
        ret = FormatString(json, "{\"mains\": \"%d\"}", !(*(DWORD*)(pwr.Data))); 
        if (ret) FormatStringW(msg, L"FormatString(mains) error: %d", ret);
      }
      else if (pwr.PowerSetting == GUID_BATTERY_PERCENTAGE_REMAINING) {
        ret = FormatString(json, "{\"battery\": \"%d\"}", *(DWORD*)(pwr.Data)); 
        if (ret) FormatStringW(msg, L"FormatString(battery) error: %d", ret);
      }
      else {
        // Unrecognized GUID
        FormatStringW(msg, L"PowerSettingChange() unrecognized GUID");
        ret = -1;
      }

      // Send POST to Hubitat
      if (!ret) ret = SendPostRequest(msg,  ctx->hubitatAddress.c_str(), ctx->hubitatPort, json);
      
      if (ret) {
        //
        // Log msg
        //
      }
    }
    return (0);

  case WM_DESTROY:
    PostQuitMessage(0);
    return (0);

  case WM_PAINT:
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

    EndPaint(hwnd, &ps);
    return (0);
  }

  return (DefWindowProcW(hwnd, uMsg, wParam, lParam));
}

// -------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */, wchar_t* pCmdLine, int nCmdShow) {

  const wchar_t CLASS_NAME[]  = L"UPS Monitoring Window Class";
  const wchar_t WINDOW_NAME[]  = L"UPS Monitoring Service";

  //
  // Parse Command line
  //
  int nArgs;
  LPWSTR* szArglist = CommandLineToArgvW(pCmdLine, &nArgs);
  if (!pCmdLine[0] || nArgs < 1) {
    MessageBoxW(nullptr, L"Hubitat hostname or IP is missing. Please add it to the command line.", WINDOW_NAME, MB_OK | MB_ICONERROR);
    return (0);
  }

  // Create end initialize app context
  AppContext *ctx = new (nothrow)AppContext;
  ctx->hubitatAddress = szArglist[0];
  ctx->hubitatPort = 39501;

  // Free memory allocated for CommandLineToArgvW arguments.
  LocalFree(szArglist);

  //
  // Register the window class
  //
  WNDCLASS wc = {};

  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  RegisterClassW(&wc);

  //
  // Create the window
  //
  HWND hWnd = CreateWindowExW(
    0,                              // Optional window styles
    CLASS_NAME,                     // Window class
    WINDOW_NAME,                    // Window text
    WS_OVERLAPPEDWINDOW,            // Window style
    CW_USEDEFAULT,                  // X
    CW_USEDEFAULT,                  // Y
    CW_USEDEFAULT,                  // W
    CW_USEDEFAULT,                  // H
    nullptr,                        // Parent window    
    nullptr,                        // Menu
    hInstance,                      // Instance handle
    ctx                             // Additional application data
  );

  if (hWnd == nullptr) {
    return 0;
  }

  ShowWindow(hWnd, nCmdShow);

  //
  // Register power events
  //
  HANDLE hNotify;
  hNotify  = RegisterPowerSettingNotification(hWnd, &GUID_ACDC_POWER_SOURCE, DEVICE_NOTIFY_WINDOW_HANDLE);
  hNotify  = RegisterPowerSettingNotification(hWnd, &GUID_BATTERY_PERCENTAGE_REMAINING, DEVICE_NOTIFY_WINDOW_HANDLE);

  //
  // Run the message loop
  //
  MSG msg = {};
  while (GetMessageW(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  return (0);
}

// Recycle Bin -----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF -------------------------------------------------------------------------------------------------------------------------
