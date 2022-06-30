#include <windows.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[]       = _T("window title");

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

void mHandleKeyUpMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
  // TODO
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

int main()
{
  return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
}
