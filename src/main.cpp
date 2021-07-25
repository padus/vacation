//
// App:         UPS Monitoring Service
// Author:      Mirco Caramori
// Copyright:   (c) 2021 Mirco Caramori
// Repository:  https://github.com/padus/vacation
//
// Description: Application source
//

// Includes --------------------------------------------------------------------------------------------------------------------

#include <system.h>

using namespace std;

#define BUFFER_SIZE           256

#define countof(T)            (sizeof(T) / sizeof(T[0]))

#define SERVICE_NAME          L"Vacation"
#define PROVIDER_NAME         L"UPS Monitoring Service"
#define AGENT_NAME            L"UPS Monitoring Service/1.0"

// Source ----------------------------------------------------------------------------------------------------------------------

unsigned long FormatString(string& str, const char* format, ...) {
  //
  // Format a string similarly to sprintf
  // Return NO_ERROR if successful, a Windows error otherwise
  //
  unsigned long err = NO_ERROR;

  char buffer[BUFFER_SIZE];
  va_list args;

  va_start(args, format);
  int len = vsnprintf(buffer, countof(buffer), format, args);
  va_end(args);  

  if (len >= 0 || len < countof(buffer)) str = buffer;
  else {
    str.clear();
    err = ERROR_INVALID_PARAMETER;
  }

  return (err);
}

// -------------------------------------------------------------

unsigned long FormatStringW(wstring& str, const wchar_t* format, ...) {
  //
  // Format a string similarly to sprintf
  // Return NO_ERROR if successful, a Windows error otherwise
  //
  unsigned long err = NO_ERROR;

  wchar_t buffer[BUFFER_SIZE];
  va_list args;

  va_start(args, format);
  int len = vswprintf(buffer, countof(buffer), format, args);
  va_end(args);  

  if (len >= 0 || len < countof(buffer)) str = buffer;
  else {
    str.clear();
    err = ERROR_INVALID_PARAMETER;
  }

  return (err);
}

// -------------------------------------------------------------

unsigned long LogSys(const wchar_t* file, unsigned long line, unsigned short type, const wchar_t* format, ...) {
  //
  // Format a string similarly to sprintf
  // Return NO_ERROR if successful, a Windows error otherwise
  //
  unsigned long err = ERROR_INVALID_PARAMETER;

  wchar_t data[BUFFER_SIZE];
  int dataLen = swprintf(data,  countof(data), L"file: %s, line: %lu", file, line);
  if (dataLen >= 0 && dataLen < countof(data)) { 

    wchar_t str[BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    int strLen = vswprintf(str,  countof(str), format, args);
    va_end(args);  
    if (strLen >= 0 && strLen < countof(str)) {

      HANDLE handle = RegisterEventSourceW(nullptr, PROVIDER_NAME);
      if (!handle) return (GetLastError());

      unsigned long message = MSG_INFO;
      if (type == EVENTLOG_ERROR_TYPE) message = MSG_ERROR;
      else if (type == EVENTLOG_WARNING_TYPE) message = MSG_WARNING;
      else type = EVENTLOG_INFORMATION_TYPE;

      const wchar_t* pstr = str;
      err = ReportEventW(handle, type, TYPE_SERVICE, message, nullptr, 1, (dataLen + 1) * sizeof(data[0]), &pstr, data)? NO_ERROR: GetLastError();

      DeregisterEventSource(handle);
    }
  }

  return (err);
}

#define LogError(format, ...)     LogSys(__FILEW__, __LINE__, EVENTLOG_ERROR_TYPE, format, __VA_ARGS__)
#define LogWarning(format, ...)   LogSys(__FILEW__, __LINE__, EVENTLOG_WARNING_TYPE, format, __VA_ARGS__)
#define LogInfo(format, ...)      LogSys(__FILEW__, __LINE__, EVENTLOG_INFORMATION_TYPE, format, __VA_ARGS__)

#ifdef _DEBUG
#define LogDebug(format, ...)     LogSys(__FILEW__, __LINE__, EVENTLOG_WARNING_TYPE, format, __VA_ARGS__)
#else
#define LogDebug(format, ...)
#endif

// -------------------------------------------------------------

unsigned long  SendPostRequest(const wchar_t* host, INTERNET_PORT port, string& json) {
  //
  // Send a POST request to Hubitat
  // Return NO_ERROR if successful, a Windows error otherwise
  //
  unsigned long err = NO_ERROR;

  HINTERNET session = nullptr;
  HINTERNET connect = nullptr;
  HINTERNET request = nullptr;

  // Open a session
  session = WinHttpOpen(AGENT_NAME, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (session == nullptr) {
    err = GetLastError();
    LogError(L"WinHttpOpen() error: 0x%08lX", err); 
  }
  else {
    // Specify an HTTP server
    connect = WinHttpConnect(session, host, port, 0);
    if (connect == nullptr) {
      err = GetLastError();
      LogError(L"WinHttpConnect() error: 0x%08lX", err); 
    }
    else {
      // Create an HTTP request handle.
      request = WinHttpOpenRequest(connect, L"POST", nullptr, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
      if (request == nullptr) {
        err = GetLastError();
        LogError(L"WinHttpOpenRequest() error: 0x%08lX", err); 
      }
      else {
        // Send a request
        if (WinHttpSendRequest(request, L"Content-Type: application/json", (DWORD)-1, (LPVOID)json.c_str(), (DWORD)json.length(), (DWORD)json.length(), 0) == false) {
          err = GetLastError();
          LogError(L"WinHttpSendRequest() error: 0x%08lX", err); 
        }
        else {
          // Wait for response
          if (WinHttpReceiveResponse(request, nullptr) == false) {
            err = GetLastError();
            LogError(L"WinHttpReceiveResponse() error: 0x%08lX", err); 
          }
          else {
            // Query return status code
            unsigned long code;
            unsigned long length = sizeof(code);
            if (WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, nullptr, &code, &length, nullptr) == false) {
              err = GetLastError();
              LogError(L"WinHttpQueryHeaders() error: 0x%08lX", err); 
            }
            else if (code != HTTP_STATUS_OK) {
              err = ERROR_INVALID_PARAMETER;
              LogError(L"SendPostRequest() code: %lu", code);               
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

  return (err);
}

// -------------------------------------------------------------

struct Context {
  SERVICE_STATUS_HANDLE statusHandle;
  SERVICE_STATUS status;

  wchar_t hubitatAddress[BUFFER_SIZE];
  INTERNET_PORT hubitatPort;

  HANDLE stopHandle;
  HANDLE powerHandle;
  HANDLE batteryHandle;

  Context() {
    LogDebug(L"Context() clearing %zu bytes", sizeof(*this));

    memset(this, 0, sizeof(*this));
    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  }
  
  virtual ~Context() {
    LogDebug(L"~Context()");

    if (stopHandle) CloseHandle(stopHandle);
    if (powerHandle) UnregisterPowerSettingNotification(powerHandle);
    if (batteryHandle) UnregisterPowerSettingNotification(batteryHandle);
  }
};

// -------------------------------------------------------------

unsigned long ServiceStatus(Context& ctx, unsigned long currentState, unsigned long exitCode, unsigned long waitHint) {
  //
  // Sets the current service status and reports it to the SCM
  // Return NO_ERROR if successful, a Windows error otherwise
  //
  static unsigned long checkPoint = 1;

  // Fill in the SERVICE_STATUS structure
  ctx.status.dwCurrentState = currentState;
  ctx.status.dwWin32ExitCode = exitCode;
  ctx.status.dwWaitHint = waitHint;

  ctx.status.dwControlsAccepted = (currentState == SERVICE_START_PENDING)? 0UL: SERVICE_ACCEPT_STOP;
  ctx.status.dwCheckPoint = (currentState == SERVICE_RUNNING || currentState == SERVICE_STOPPED)? 0UL: checkPoint++;

  // Report the status of the service to the SCM
  return (SetServiceStatus(ctx.statusHandle, &ctx.status)? NO_ERROR: GetLastError());
}

// -------------------------------------------------------------

unsigned long WINAPI ServiceHandler(unsigned long ctrl, unsigned long evt, void* data, void* context) {
  //
  // Handle the requested control code
  //
  unsigned long err = NO_ERROR;   
  Context& ctx = *static_cast<Context*>(context);

  switch(ctrl) {  
  case SERVICE_CONTROL_STOP: 
    ServiceStatus(ctx, SERVICE_STOP_PENDING, err, 0);

    // Signal the service to stop.
    SetEvent(ctx.stopHandle);
    ServiceStatus(ctx, ctx.status.dwCurrentState, err, 0);
    break;
 
  case SERVICE_CONTROL_INTERROGATE: 
    break; 
 
  case SERVICE_CONTROL_POWEREVENT:
    if (evt == PBT_POWERSETTINGCHANGE) {
      //
      // GUID_ACDC_POWER_SOURCE: DWORD
      //   0: AC power source
      //   1: Onboard battery power source
      //   2: UPS device
      //
      // GUID_BATTERY_PERCENTAGE_REMAINING: DWORD
      //   0-100: percentage remaining
      //
      unsigned long ret = NO_ERROR;
      wstring log;
      wstring msg;
      string json;
      POWERBROADCAST_SETTING& pwr = *static_cast<POWERBROADCAST_SETTING*>(data);
      data = static_cast<void*>(pwr.Data);

      if (pwr.PowerSetting == GUID_ACDC_POWER_SOURCE) {
        bool present = !(*static_cast<unsigned long*>(data));

        ret = FormatString(json, "{\"mains\": \"%d\"}", present); 
        if (ret == NO_ERROR) FormatStringW(log, L"Mains: %d", present);
        else LogError(L"FormatString(mains) error: 0x%08lX", ret);
      }
      else if (pwr.PowerSetting == GUID_BATTERY_PERCENTAGE_REMAINING) {
        unsigned long percentage = *static_cast<unsigned long*>(data);

        ret = FormatString(json, "{\"battery\": \"%lu\"}", percentage); 
        if (ret == NO_ERROR)  FormatStringW(log, L"Battery: %lu%%%%", percentage);
        else LogError(L"FormatString(battery) error: 0x%08lX", ret);
      }
      else {
        // Unrecognized GUID
        ret = ERROR_INVALID_PARAMETER;
        LogError(L"PowerSettingChange() error: unrecognized GUID");
      }

      // Send POST to Hubitat
      if (ret == NO_ERROR) ret = SendPostRequest(ctx.hubitatAddress, ctx.hubitatPort, json);

      // Log event if no error      
      if (ret == NO_ERROR) LogInfo(log.c_str());
    }
    break;

  default: 
    break;
  } 

  return (err);
}

// -------------------------------------------------------------

void WINAPI ServiceMain(unsigned long /*argc*/, wchar_t** /*argv*/) {
  //
  // Service main entry point
  //
  LogDebug(L"ServiceMain() entry: %s", GetCommandLineW());

  Context ctx;

  // Register the handler function for the service
  ctx.statusHandle = RegisterServiceCtrlHandlerExW(SERVICE_NAME, ServiceHandler, &ctx);
  if (ctx.statusHandle == nullptr) LogError(L"RegisterServiceCtrlHandlerEx() error: 0x%08lX", GetLastError());
  else {
    unsigned long err = NO_ERROR; 

    // Report initial status to the SCM
    ServiceStatus(ctx, SERVICE_START_PENDING, err, 3000);

    // Process commandline arguments
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc != 2) {
      err = ERROR_INVALID_PARAMETER;
      LogError(L"ServiceMain() error: 0x%08lX, argc = %d, argv = %s", err, argc, GetCommandLineW()); 
    }
    else {
      StringCchCopyW(ctx.hubitatAddress, countof(ctx.hubitatAddress), argv[1]);
      ctx.hubitatPort = 39501;

      // Create an event: the control handler function signals this event when it receives the stop control code
      if ((ctx.stopHandle = CreateEventW(nullptr, true, false, nullptr)) == nullptr ||
          (ctx.powerHandle = RegisterPowerSettingNotification(ctx.statusHandle, &GUID_ACDC_POWER_SOURCE, DEVICE_NOTIFY_SERVICE_HANDLE)) == nullptr ||
          (ctx.batteryHandle = RegisterPowerSettingNotification(ctx.statusHandle, &GUID_BATTERY_PERCENTAGE_REMAINING, DEVICE_NOTIFY_SERVICE_HANDLE)) == nullptr) {
        err = GetLastError();
        LogError(L"CreateEvent()/RegisterEvent() error: 0x%08lX", err); 
      }
      else {
        // Report running status when initialization is complete.
        ServiceStatus(ctx, SERVICE_RUNNING, err, 0);

        // Wait until service stops
        WaitForSingleObject(ctx.stopHandle, INFINITE);
      }
    }

    ServiceStatus(ctx, SERVICE_STOPPED, err, 0);
  }

  LogDebug(L"ServiceMain() exit");

  return;
}

// -------------------------------------------------------------

int wmain(int /*argc*/, wchar_t** /*argv*/) { 

  LogDebug(L"wmain() entry");

  unsigned long err = NO_ERROR; 

  SERVICE_TABLE_ENTRY dispatchTable[] = { 
    { SERVICE_NAME, ServiceMain }, 
    { nullptr, nullptr } 
  }; 
 
  if (StartServiceCtrlDispatcherW(dispatchTable) == false) {
    err = GetLastError();    
    LogError(L"StartServiceCtrlDispatcher() error: %lu", err);
  } 

  LogDebug(L"wmain() exit");

  return(static_cast<int>(err));
} 

// Recycle Bin -----------------------------------------------------------------------------------------------------------------

/*

*/

// EOF -------------------------------------------------------------------------------------------------------------------------
