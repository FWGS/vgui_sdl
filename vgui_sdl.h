// SPDX-License-Identifier: LGPL-3.0-or-later

#include <SDL3/SDL.h>
#include <VGUI.h>
#include <VGUI_App.h>
#include <VGUI_Desktop.h>
#include <VGUI_DesktopIcon.h>
#include <VGUI_Font.h>
#include <VGUI_FrameSignal.h>
#include <VGUI_Panel.h>
#include <VGUI_SurfaceBase.h>
#include <VGUI_TaskBar.h>
#include <vector>
#include <stdio.h>
#include <signal.h>

using namespace vgui;

// set from SIGUSR1 handler, serviced in SDLSurface::swapBuffers
extern volatile sig_atomic_t g_screenshot_requested;

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
// controls/taskbar.cpp
//
class TaskBarEx : public TaskBar
{
public:
	TaskBarEx( int x, int y, int wide, int tall );

	void removeFrame( Frame *frame );
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
void EventServer_Start( void );

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
