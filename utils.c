#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "termcolor.h"

void mGetLastError()
{
  DWORD errorCode      = GetLastError();
  LPWSTR messageBuffer = NULL;
  size_t size          = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

  printf(FG_RED);
  printf("Error Code: %d\n", errorCode);
  wprintf(L"%s\n", messageBuffer);
  printf(RESET_COLOR);
  // TODO check to see if we don't free the messageBuffer pointer, will it lead to memory leaking?
}

void* mMalloc(size_t size, char* filePath, int lineNumber)
{
  void* retval = malloc(size);
  if (retval == NULL)
  {
    printf(FG_RED);
    printf("malloc failed to allocate %zx byte(s) at %s:%d\n", size, filePath, lineNumber);
    printf(RESET_COLOR);
    exit(-1);
  }

  return retval;
}
