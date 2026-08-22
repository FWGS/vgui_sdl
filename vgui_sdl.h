// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef VGUI_SDL_H
#define VGUI_SDL_H

#include <SDL3/SDL.h>
#if USE_FREEVGUI_HEADERS
#include "vgui.h"
#include "app.h"
#include "buildgroup.h"
#include "controls/desktop.h"
#include "font.h"
#include "signals.h"
#include "panel.h"
#include "surface.h"
#else
#include <VGUI.h>
#include <VGUI_App.h>
#include <VGUI_BuildGroup.h>
#include <VGUI_Desktop.h>
#include <VGUI_DesktopIcon.h>
#include <VGUI_Font.h>
#include <VGUI_FrameSignal.h>
#include <VGUI_Panel.h>
#include <VGUI_SurfaceBase.h>
#include <VGUI_TaskBar.h>
#endif
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

using namespace vgui;

// global runtime state, xash3d-fwgs style: read and written directly. state
// that needs side effects on change keeps a Sys_* setter (Sys_SetHeadless,
// Sys_SetRenderScale); everything else is a plain field.
struct host_s
{
	int scale = 1;                              // -scale N nearest render scale, always >=1
	bool headless = false;                      // -headless: window created hidden
	bool ignoreMouse = false;                   // GetMousePos returns the injected cursor, not the real one
	int injMouseX = 0;                          // last injected cursor, logical coords
	int injMouseY = 0;
	bool drawBounds = false;                    // Ctrl+R panel-bounds overlay
	bool drawLabels = false;                    // Ctrl+E class-name labels
	volatile unsigned int frameCount = 0;       // completed frames; the event server waits on this
	volatile sig_atomic_t screenshotRequested = 0; // set by SIGUSR1/event server, serviced in swapBuffers
	char screenshotPath[256] = "";              // explicit screenshot filename, or empty for a timestamp
};

extern host_s host;

constexpr int MAX_TEXTURES = 4096;

static inline const char *bool_to_str( bool value )
{
	return value ? "true" : "false";
}

struct FontTexture
{
	int texture;
	int font_id;
	SDL_FRect rects[256];
};

class SDLSurface : public SurfaceBase
{
public:
	SDLSurface( Panel *panel ) : SurfaceBase( panel ) {}
	virtual ~SDLSurface();

	bool CreateWindow();

	void setTitle( const char * ) override;
	bool setFullscreenMode( int, int, int ) override;
	void setWindowedMode() override;
	void setAsTopMost( bool ) override;
	void createPopup( Panel * ) override;
	bool hasFocus() override;
	bool isWithin( int, int ) override;
	int createNewTextureID() override;
	void GetMousePos( int &, int & ) override;

protected:
	void addModeInfo( int, int, int ) override;
	void drawSetColor( int, int, int, int ) override;
	void drawFilledRect( int, int, int, int ) override;
	void drawOutlinedRect( int, int, int, int ) override;
	void drawSetTextFont( Font *font ) override;
	void drawSetTextColor( int, int, int, int ) override;
	void drawSetTextPos( int, int ) override;
	void drawPrintText( const char *, int ) override;
	void drawSetTextureRGBA( int, const char *, int, int ) override;
	void drawSetTexture( int ) override;
	void drawTexturedRect( int, int, int, int ) override;
	void invalidate( Panel * ) override;
	void enableMouseCapture( bool ) override;
	void setCursor( Cursor * ) override;
	void swapBuffers() override;
	void pushMakeCurrent( Panel *, bool ) override;
	void popMakeCurrent( Panel * ) override;
	void applyChanges() override;

private:
	SDL_FRect rect( int x0, int y0, int x1, int y1 );

	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

	SDL_Texture *textures[MAX_TEXTURES] = {};
	int last_texture = 1; // id 0 means "no texture" in vgui
	int bound_texture = 0;

	int color[4] = {};
	int text_color[4] = {};
	int text_pos[2] = {};
	int origin[2] = {};

	SDL_Cursor *cursors[SDL_SYSTEM_CURSOR_COUNT] = {};

	std::vector<FontTexture> fonts;
	Font *font = nullptr;
	FontTexture *font_texture = nullptr;
};

//
// sdl_viewport.cpp
//
void CreateViewport( Panel *rootpanel );
void DestroyViewport();
Image *LoadTGA( const char *path );
void DestroyFrame( Frame *frame );

// shared by the test miniapps: reacting to the close button is app
// responsibility (the stock taskbar handler's closing() is a no-op),
// and destroying frames exists purely for testing convenience
class TestFrameSignal : public FrameSignal
{
public:
	void closing( Frame *frame ) override
	{
		DestroyFrame( frame );
	}

	void minimizing( Frame *frame, bool toTray ) override
	{
	}
};

//
// sdl_surface.cpp: live SDL window/renderer, for apps/ to introspect
//
SDL_Window *Sys_GetWindow( void );
SDL_Renderer *Sys_GetRenderer( void );

// these two keep setters because changing them has side effects beyond the
// field; everything else (host.scale read, host.headless, host.ignoreMouse,
// host.injMouseX/Y, host.drawBounds/drawLabels, ...) is accessed directly.
void Sys_SetRenderScale( int scale );  // clamps to >=1
void Sys_SetHeadless( bool headless ); // also forces ignoreMouse on

//
// controls/taskbar.cpp
//
class TaskBarEx : public TaskBar
{
public:
	TaskBarEx( int x, int y, int wide, int tall );

	void removeFrame( Frame *frame );
	Button *getStartButton() { return start; }

protected:
	void performLayout() override;

private:
	Button *start;
};

class DesktopEx : public Desktop
{
public:
	DesktopEx( int x, int y, int wide, int tall );

	TaskBarEx *getTaskBar();

private:
	TaskBarEx *taskbar;
};

//
// sdl_eventserver.cpp
//
bool EventServer_Start( void );

//
// sdl_app.cpp: Ctrl+B toggles every registered BuildGroup (VGUI's
// built-in edit mode). App::enableBuildMode is a different, one-way
// input-freeze flag
//
void BuildMode_Register( BuildGroup *bg );
void BuildMode_Toggle( void );

// returns FreeVGUI version or NULL if we're running against proprietary VGUI
const char *Sys_FreeVGUIVersion( void );

//
// controls/button_test.cpp
//
DesktopIcon *CreateButtonTest();

//
// controls/label_test.cpp
//
DesktopIcon *CreateLabelTest();

//
// controls/scrollpanel_test.cpp
//
DesktopIcon *CreateScrollPanelTest();

//
// controls/tabpanel_test.cpp
//
DesktopIcon *CreateTabPanelTest();

//
// controls/textentry_test.cpp
//
DesktopIcon *CreateTextEntryTest();

//
// controls/slider_test.cpp
//
DesktopIcon *CreateSliderTest();

//
// controls/progressbar_test.cpp
//
DesktopIcon *CreateProgressBarTest();

//
// controls/imagepanel_test.cpp
//
DesktopIcon *CreateImagePanelTest();

//
// controls/messagebox_test.cpp
//
DesktopIcon *CreateMessageBoxTest();

//
// controls/menu_test.cpp
//
DesktopIcon *CreateMenuTest();

//
// controls/listpanel_test.cpp
//
DesktopIcon *CreateListPanelTest();

//
// controls/tablepanel_test.cpp
//
DesktopIcon *CreateTablePanelTest();

//
// controls/wizard_test.cpp
//
DesktopIcon *CreateWizardTest();

//
// controls/configwizard_test.cpp
//
DesktopIcon *CreateConfigWizardTest();

//
// controls/border_test.cpp
//
DesktopIcon *CreateBorderTest();

//
// controls/buildgroup_test.cpp
//
DesktopIcon *CreateBuildGroupTest();

//
// controls/focusnav_test.cpp
//
DesktopIcon *CreateFocusNavTest();

//
// controls/frame_test.cpp
//
DesktopIcon *CreateFrameTest();

//
// controls/headerpanel_test.cpp
//
DesktopIcon *CreateHeaderPanelTest();

//
// controls/scrollbar_test.cpp
//
DesktopIcon *CreateScrollBarTest();

//
// controls/treefolder_test.cpp
//
DesktopIcon *CreateTreeFolderTest();

//
// controls/statematrix_test.cpp
//
DesktopIcon *CreateStateMatrixTest();

//
// controls/propertypanel_test.cpp
//
DesktopIcon *CreatePropertyPanelTest();

#endif // VGUI_SDL_H
