// SPDX-License-Identifier: LGPL-3.0-or-later

#include <SDL3/SDL.h>
#include <VGUI.h>
#include <VGUI_App.h>
#include <VGUI_Font.h>
#include <VGUI_Panel.h>
#include <VGUI_SurfaceBase.h>
#include <vector>
#include <stdio.h>

using namespace vgui;

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
	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

	SDL_Texture *textures[MAX_TEXTURES] = {};
	int last_texture = 0;
	int bound_texture = 0;

	int color[4] = {};
	int text_color[4] = {};
	int text_pos[2] = {};

	SDL_Cursor *cursors[SDL_SYSTEM_CURSOR_COUNT] = {};

	std::vector<FontTexture> fonts;
	Font *font = nullptr;
	FontTexture *font_texture = nullptr;
};

void CreateViewport( Panel *rootpanel );
void DestroyViewport();
