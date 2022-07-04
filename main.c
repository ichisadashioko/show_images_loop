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

struct WINDOWS_PATH_STRING_DATA
{
  TCHAR* buffer;
  size_t character_size_in_bytes;
  DWORD number_of_characters_with_null_terminator;
};

int find_test_images_directory_path(struct WINDOWS_PATH_STRING_DATA* path_string_data)
{
  // test-images
  // 11 characters
  // 12 characters with null terminator
  TCHAR test_images_directory_name[] = {'t', 'e', 's', 't', '-', 'i', 'm', 'a', 'g', 'e', 's', 0};

  /*
  TCHAR* test_images_directory_name                           = TEXT("test-images");
  DWORD test_images_number_of_characters_with_null_terminator = sizeof(test_images_directory_name);
  printf("test_images_number_of_characters_with_null_terminator: %d\n", test_images_number_of_characters_with_null_terminator);
  printf("sizeof(TCHAR): %d\n", sizeof(TCHAR));
  printf("string length: %d\n", test_images_number_of_characters_with_null_terminator / sizeof(TCHAR));

  printf("test_images_directory_name\n");
  wprintf("%s\n", test_images_directory_name);
  for (int i = 0; i < test_images_number_of_characters_with_null_terminator; i++)
  {
    printf("%d", (i % 10));
  }
  printf("\n");
  */

  DWORD dword_retval;

  DWORD path_number_of_characters = 0;

  dword_retval = GetCurrentDirectory(path_number_of_characters, NULL);

  if (dword_retval == 0)
  {
    printf("GetCurrentDirectory failed at %s:%d\n", __FILE__, __LINE__);
    mGetLastError();
    return -1;
  }

  path_number_of_characters        = dword_retval;
  size_t path_buffer_size_in_bytes = path_number_of_characters * sizeof(TCHAR);
  TCHAR* path_buffer               = (TCHAR*)mMalloc(path_buffer_size_in_bytes, __FILE__, __LINE__);

  dword_retval = GetCurrentDirectory(path_number_of_characters, path_buffer);

  if (dword_retval == 0)
  {
    printf("GetCurrentDirectory failed at %s:%d\n", __FILE__, __LINE__);
    mGetLastError();

    free(path_buffer);
    return -1;
  }

  if (dword_retval != (path_number_of_characters - 1))
  {
    printf("warning: the number of characters written is not equal to the reported number\n");
    printf("written: %d, reported: %d\n", dword_retval, path_number_of_characters);
    printf("%s:%d\n", __FILE__, __LINE__);
  }

  if (path_buffer[path_number_of_characters - 1] != 0)
  {
    printf("warning: the last character in the path buffer is not 0\n");
    printf("%s:%d\n", __FILE__, __LINE__);
  }

  path_string_data->buffer                                    = path_buffer;
  path_string_data->number_of_characters_with_null_terminator = path_number_of_characters;
  path_string_data->character_size_in_bytes                   = sizeof(TCHAR);

  return 0;
}

int testmain()
{
  struct WINDOWS_PATH_STRING_DATA path_string_data;
  int result = find_test_images_directory_path(&path_string_data);

  if (result != 0)
  {
    return result;
  }

  printf("number of characters: %d\n", path_string_data.number_of_characters_with_null_terminator);
  printf("character size in bytes: %d\n", path_string_data.character_size_in_bytes);

  if (path_string_data.buffer[path_string_data.number_of_characters_with_null_terminator - 1] != 0)
  {
    printf("warning: the last character in the path buffer is not 0\n");
    printf("%s:%d\n", __FILE__, __LINE__);

    path_string_data.buffer[path_string_data.number_of_characters_with_null_terminator - 1] = 0;
  }

  wprintf(L"%s\n", path_string_data.buffer);
  free(path_string_data.buffer);

  for (int i = 0; i < path_string_data.number_of_characters_with_null_terminator; i++)
  {
    printf("%d", (i % 10));
  }
  printf("\n");
  return 0;
}

int main()
{
  return testmain();
  initialize_application_state();
  return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
}
