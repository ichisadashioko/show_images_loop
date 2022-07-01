#include <windows.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "utils.h"
#include "termcolor.h"
#include "virtualkeycodes.h"

// constants for toggling fullscreen mode
#define DEFAULT_WIDTH           1280
#define DEFAULT_HEIGHT          720
#define DEFAULT_NORMAL_WINDOW_X 0
#define DEFAULT_NORMAL_WINDOW_Y 0

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[]       = _T("window title");

struct APPLICATION_STATE
{
  int is_fullscreen;
  int should_toggle_fullscreen;  // key down event will fire rapidly while key is held down. the event handler only toggle the fullscreen state once and set this flag to 0. when we receive key up event, we will set this flag to 1.
                                 // if this flag is 1, we will toggle the fullscreen state. if this flag is 0, we will not toggle the fullscreen state.
  RECT previous_window_rect;     // save normal window client rect while in fullscreen
};

struct APPLICATION_STATE app_state;

void mHandlePaintMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  // clock_t ts = clock();
  HDC hdc;
  PAINTSTRUCT ps;
  RECT rc;

  GetClientRect(hwnd, &rc);

  if (rc.bottom == 0)
  {
    return;
  }

  hdc = BeginPaint(hwnd, &ps);

  // draw black background
  HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
  FillRect(hdc, &rc, bgBrush);

  // TODO

  EndPaint(hwnd, &ps);
}

int is_window_rect_valid(RECT rect)
{
  // get the virtual screen rect
  RECT virtual_screen_rect;
  virtual_screen_rect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
  virtual_screen_rect.top  = GetSystemMetrics(SM_YVIRTUALSCREEN);

  int virtual_screen_width   = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  int virtual_screen_height  = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  virtual_screen_rect.right  = virtual_screen_rect.left + virtual_screen_width;
  virtual_screen_rect.bottom = virtual_screen_rect.top + virtual_screen_height;

  int window_width  = rect.right - rect.left;
  int window_height = rect.bottom - rect.top;

  // for clang-format to format one line if statement per line
  // clang-format off
  if (rect.left < virtual_screen_rect.left) { return 0; }
  if (rect.left > virtual_screen_rect.right) { return 0; }
  if (rect.top < virtual_screen_rect.top) { return 0; }
  if (rect.top > virtual_screen_rect.bottom) { return 0; }
  if (window_width < 0) { return 0; }
  if (window_width > virtual_screen_width) { return 0; }
  if (window_height < 0) { return 0; }
  if (window_height > virtual_screen_height) { return 0; }
  // clang-format on
  return 1;
}

void toggle_fullscreen_mode(_In_ HWND hwnd)
{
  BOOL winapi_bool_result;
  if (app_state.is_fullscreen == 0)
  {
    // store the current window rect
    RECT rect;
    winapi_bool_result = GetWindowRect(hwnd, &rect);
    if (winapi_bool_result == 0)
    {
      printf("GetWindowRect failed at %s:%d\n", __FILE__, __LINE__);
      mGetLastError();
      return;
    }

    app_state.previous_window_rect = rect;

    // get current monitor screen size to enter full screen mode in the current monitor only
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFO);
    winapi_bool_result = GetMonitorInfo(hMonitor, &monitorInfo);
    if (winapi_bool_result == 0)
    {
      printf("GetMonitorInfo failed at %s:%d\n", __FILE__, __LINE__);
      mGetLastError();
      return;
    }

    // monitorInfo.rcMonitor.
    int monitor_width  = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
    int monitor_height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

    // set the window to full screen
    if (SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE) == 0)
    {
      printf("SetWindowLongPtr failed at %s:%d\n", __FILE__, __LINE__);
      mGetLastError();
      return;
    }

    winapi_bool_result = SetWindowPos(hwnd, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitor_width, monitor_height, SWP_FRAMECHANGED);

    if (winapi_bool_result == 0)
    {
      printf("SetWindowPos failed at %s:%d\n", __FILE__, __LINE__);
      mGetLastError();
    }

    app_state.is_fullscreen = 1;
  }
  else
  {
    // restore the window to normal mode
    if (SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE) == 0)
    {
      printf("SetWindowLongPtr failed at %s:%d\n", __FILE__, __LINE__);
      mGetLastError();
      return;
    }

    // check the previous_window_rect
    RECT normal_window_rect = app_state.previous_window_rect;
    if (is_window_rect_valid(normal_window_rect) == 0)
    {
      printf("invalid window rect (%d, %d, %d, %d) at %s:%d\n", normal_window_rect.left, normal_window_rect.top, normal_window_rect.right, normal_window_rect.bottom, __FILE__, __LINE__);
      normal_window_rect.left   = DEFAULT_NORMAL_WINDOW_X;
      normal_window_rect.top    = DEFAULT_NORMAL_WINDOW_Y;
      normal_window_rect.right  = normal_window_rect.left + DEFAULT_WIDTH;
      normal_window_rect.bottom = normal_window_rect.top + DEFAULT_HEIGHT;
    }

    winapi_bool_result = SetWindowPos(hwnd, HWND_TOP, normal_window_rect.left, normal_window_rect.top, normal_window_rect.right - normal_window_rect.left, normal_window_rect.bottom - normal_window_rect.top, SWP_FRAMECHANGED);
    if (winapi_bool_result == 0)
    {
      printf("SetWindowPos failed at %s:%d\n", __FILE__, __LINE__);
      mGetLastError();
    }

    app_state.is_fullscreen = 0;
  }
  // TODO
}

void mHandleKeyDownMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  int virtual_key_code = (int)wParam;
  if (virtual_key_code == VK_F_KEY)
  {
    // TODO this is not working
    if (app_state.should_toggle_fullscreen == 0)
    {
      return;
    }

    app_state.should_toggle_fullscreen = 0;
    // toggle full screen and windowed mode
    toggle_fullscreen_mode(hwnd);
  }
  else if (virtual_key_code == VK_Q_KEY)
  {
    PostQuitMessage(0);
  }
  // TODO detect the key being released before execute the event handler to prevent rapid firing key down events while holding the key down
}

void mHandleKeyUpMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  // TODO
  int virtual_key_code = (int)wParam;
  if (virtual_key_code == VK_F_KEY)
  {
    app_state.should_toggle_fullscreen = 1;
    // toggle full screen and windowed mode
  }
}

void mHandleResizeMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  InvalidateRect(hwnd, NULL, FALSE);
}

LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_PAINT:
    {
      mHandlePaintMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_SIZE:
    {
      mHandleResizeMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_KEYDOWN:
    {
      mHandleKeyDownMessage(hwnd, uMsg, wParam, lParam);
    }
    case WM_KEYUP:
    {
      mHandleKeyUpMessage(hwnd, uMsg, wParam, lParam);
      break;
    }
    case WM_DESTROY:
    {
      PostQuitMessage(0);
      break;
    }
    default:
    {
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
  }

  return 0;
}

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  WNDCLASSEX wcex;

  wcex.cbSize        = sizeof(WNDCLASSEX);
  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = hInstance;
  wcex.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(0);
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm       = LoadIcon(wcex.hInstance, IDI_APPLICATION);

  if (!RegisterClassEx(&wcex))
  {
    printf("RegisterClassEx failed at %s:%d\n", __FILE__, __LINE__);
    mGetLastError();
    return -1;
  }

  HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_EX_LAYERED, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, hInstance, NULL);

  if (!hwnd)
  {
    printf("CreateWindow failed at %s:%d\n", __FILE__, __LINE__);
    mGetLastError();
    return -1;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

void initialize_application_state()
{
  app_state.is_fullscreen            = 0;
  app_state.should_toggle_fullscreen = 1;
}

int main()
{
  initialize_application_state();
  return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
}
