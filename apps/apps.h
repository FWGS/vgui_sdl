// SPDX-License-Identifier: LGPL-3.0-or-later

// apps/ holds actually-useful little programs (unlike the widget demos
// in controls/), each a stock MiniApp driven by real SDL3 APIs and
// launched from the taskbar Start menu instead of a desktop icon.

#include "vgui_sdl.h"

//
// sdlinfo.cpp
//
DesktopIcon *CreateSDLInfoApp();

//
// vguiinfo.cpp
//
DesktopIcon *CreateVGUIInfoApp();

//
// webcam.cpp
//
DesktopIcon *CreateWebcamApp();

//
// eventtester.cpp
//
DesktopIcon *CreateEventTesterApp();

//
// startmenu.cpp
//
void CreateStartMenu( DesktopEx *desktop );

