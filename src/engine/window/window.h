#ifndef COSMORIA_WINDOW
#define COSMORIA_WINDOW

#include "windowDefines.h"


b8 createWindow(WindowInfo *info);

b8 processMessage(void);

void destroyWindow(void);

Vec2u getWindowSize(void);

WindowAPICore* getWindowHandlePtr(void);

#endif