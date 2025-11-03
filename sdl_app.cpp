// SPDX-License-Identifier: LGPL-3.0-or-later

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <VGUI.h>
#include <VGUI_App.h>
#include <VGUI_Panel.h>
#include <VGUI_Surface.h>
#include <VGUI_SurfaceBase.h>
#include <VGUI_Cursor.h>
#include <VGUI_Desktop.h>
#include <VGUI_DesktopIcon.h>
#include <VGUI_MiniApp.h>
#include <VGUI_Frame.h>
#include <VGUI_Font.h>
#include <VGUI_MouseCode.h>
#include <VGUI_KeyCode.h>
#include <stdio.h>
#include <ctype.h>
#include <vector>

#define MAX_TEXTURES 4096

static inline const char *bool_to_str( bool value )
{
	return value ? "true" : "false";
}

static inline SDL_FRect rect( int x0, int y0, int x1, int y1 )
{
	SDL_FRect frect = { (float)x0, (float)y0, (float)(x1 - x0), (float)(y1 - y0) };
	return frect;
}

class FontTexture
{
public:
	int texture;
	int font_id;
	SDL_FRect rect[256];
};

class SDL_VGUISurface : public vgui::SurfaceBase
{
public:
	SDL_VGUISurface( vgui::Panel *panel ) : vgui::SurfaceBase( panel ) {}

	bool CreateWindow()
	{
		int wide, tall;
		getPanel()->getSize( wide, tall );

		if( !SDL_CreateWindowAndRenderer( "test", wide, tall, 0, &window, &renderer ))
		{
			printf( "Can't create SDL window and surface: %s\n", SDL_GetError());
			return false;
		}

		return true;
	}

	void setTitle( const char *title ) override
	{
		SDL_SetWindowTitle( window, title );
	}

	bool setFullscreenMode( int wide, int tall, int bpp ) override
	{
		printf( "%s( %d, %d, %d ): unimplemented\n", __PRETTY_FUNCTION__, wide, tall, bpp  );
		return false;
	}

	void setWindowedMode() override
	{
		printf( "%s(): unimplemented\n", __PRETTY_FUNCTION__ );
	}

	void setAsTopMost( bool state ) override
	{
		printf( "%s( %s ): unimplemented\n", __PRETTY_FUNCTION__, bool_to_str( state ));
	}

	void createPopup( vgui::Panel *embeddedPanel ) override
	{
		printf( "%s( %p ): unimplemented\n", __PRETTY_FUNCTION__, embeddedPanel );
	}

	bool hasFocus() override
	{
		return SDL_GetWindowFlags( window ) & (SDL_WINDOW_INPUT_FOCUS|SDL_WINDOW_MOUSE_FOCUS);
	}

	bool isWithin( int x, int y ) override
	{
		return SDL_GetWindowFlags( window ) & SDL_WINDOW_MOUSE_FOCUS;
	}

	int createNewTextureID( void ) override
	{
		return last_texture++;
	}

	void GetMousePos( int &x, int &y ) override
	{
		float fx, fy;

		SDL_GetGlobalMouseState( &fx, &fy );

		x = (int)fx;
		y = (int)fy;
	}

protected:
	void addModeInfo( int wide, int tall, int bpp )
	{
		printf( "%s( %d, %d, %d ): unimplemented\n", __PRETTY_FUNCTION__, wide, tall, bpp );
	}

	void drawSetColor( int r, int g, int b, int a )
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		color[3] = 255 - a;

		SDL_SetRenderDrawColor( renderer, r, g, b, a );
	}

	void drawFilledRect( int x0, int y0, int x1, int y1 )
	{
		SDL_FRect frect = rect( x0, y0, x1, y1 );

		if( !color[3] )
			return;

		if( color[3] == 255 )
			SDL_SetRenderDrawBlendMode( renderer, SDL_BLENDMODE_NONE );
		else
			SDL_SetRenderDrawBlendMode( renderer, SDL_BLENDMODE_BLEND );

		SDL_SetRenderDrawColor( renderer, color[0], color[1], color[2], color[3] );
		SDL_RenderFillRect( renderer, &frect );
	}

	void drawOutlinedRect( int x0, int y0, int x1, int y1 )
	{
		SDL_FRect frect = rect( x0, y0, x1, y1 );

		if( !color[3] )
			return;

		if( color[3] == 255 )
			SDL_SetRenderDrawBlendMode( renderer, SDL_BLENDMODE_NONE );
		else
			SDL_SetRenderDrawBlendMode( renderer, SDL_BLENDMODE_BLEND );

		SDL_SetRenderDrawColor( renderer, color[0], color[1], color[2], color[3] );
		SDL_RenderRect( renderer, &frect );
	}

	void drawSetTextFont( vgui::Font *font )
	{
		// printf( "%s( %p ): unimplemented\n", __PRETTY_FUNCTION__, font );
		if( !font )
		{
			font_texture = nullptr;
			return;
		}

		int font_id = font->getId();
		bool found = false;

		this->font = font;

		for( int i = 0; i < fonts.size(); i++ )
		{
			if( fonts[i].font_id == font_id )
			{
				font_texture = &fonts[i];
				found = true;
				break;
			}
		}

		if( !found )
		{
			FontTexture new_texture;

			new_texture.font_id = font_id;
			new_texture.texture = createNewTextureID();

			const int texture_width = 256;
			int texture_height = 256;
			unsigned char *rgba = (unsigned char *)malloc( texture_width * texture_height * 4 );

			int x = 0, y = 0;

			for( int i = 0; i < 256; i++ )
			{
				int a, b, c;

				if( isspace( i ))
					continue;

				font->getCharABCwide( i, a, b, c );

				int width = b;
				int height = font->getTall();

				if( x + width + 1 > texture_width )
				{
					x = 0;

					if( y + height + 1 > texture_height )
					{
						texture_height += 256;
						rgba = (unsigned char *)realloc( rgba, texture_width * texture_height * 4 );
					}

					y += height + 1;
				}

				font->getCharRGBA( i, x, y, texture_width, texture_height, rgba );
				new_texture.rect[i].x = x;
				new_texture.rect[i].y = y;
				new_texture.rect[i].w = width;
				new_texture.rect[i].h = height;

				x += width + 1;
			}

			drawSetTextureRGBA( new_texture.texture, (char*)rgba, texture_width, texture_height );
			free( rgba );
			fonts.push_back( new_texture );

			font_texture = &fonts[fonts.size() - 1];
		}
	}

	void drawSetTextColor( int r, int g, int b, int a )
	{
		text_color[0] = r;
		text_color[1] = g;
		text_color[2] = b;
		text_color[3] = 255 - a;
	}

	void drawSetTextPos( int x, int y )
	{
		text_pos[0] = x;
		text_pos[1] = y;
	}

	void drawPrintText( const char *text, int textLen )
	{
		// printf( "%s( %s ): unimplemented\n", __PRETTY_FUNCTION__, text );

		if( !font_texture )
			return;

		const int height = font->getTall();

		SDL_Texture *tex = textures[font_texture->texture];

		if( text_color[3] == 255 )
			SDL_SetRenderDrawBlendMode( renderer, SDL_BLENDMODE_NONE );
		else
			SDL_SetRenderDrawBlendMode( renderer, SDL_BLENDMODE_BLEND );

		SDL_SetTextureColorMod( tex, text_color[0], text_color[1], text_color[2] );
		SDL_SetTextureAlphaMod( tex, text_color[3] );
		SDL_SetRenderDrawColor( renderer, text_color[0], text_color[1], text_color[2], text_color[3] );

		for( int i = 0; i < textLen; i++ )
		{
			int ch = text[i];
			int a = 0, b = 0, c = 0;

			font->getCharABCwide( ch, a, b, c );

			if( isspace( ch ))
			{
				text_pos[0] += a + b + c;
				continue;
			}

			SDL_FRect srcrect = font_texture->rect[ch];

			SDL_FRect dstrect = {
			    (float)text_pos[0] + (float)a,
			    (float)text_pos[1],
			    (float)b,
			    (float)height
			};


			SDL_RenderTexture( renderer, tex, &srcrect, &dstrect );

#if 0
			printf( "drawing %c src (%g %g %g %g) dst (%g %g %g %g)\n", ch,
			    srcrect.x, srcrect.y, srcrect.w, srcrect.h,
			    dstrect.x, dstrect.y, dstrect.w, dstrect.h );
#endif

			text_pos[0] += a + b + c;
		}
	}

	void drawSetTextureRGBA( int id, const char *rgba, int wide, int tall )
	{
		if( id < 0 || id >= MAX_TEXTURES )
			return;

		if( textures[id] )
			SDL_DestroyTexture( textures[id] );

		SDL_Surface *surf = SDL_CreateSurfaceFrom( wide, tall, SDL_PIXELFORMAT_RGBA32, (void *)rgba, wide * 4 );

#if 1
		char path[256];
		snprintf( path, sizeof( path ), "tex/%i.bmp", id );
		SDL_SaveBMP( surf, path );
#endif

		textures[id] = SDL_CreateTextureFromSurface( renderer, surf );

		SDL_DestroySurface( surf );
	}

	void drawSetTexture( int id )
	{
		bound_texture = id;
	}

	void drawTexturedRect( int x0, int y0, int x1, int y1 )
	{
		SDL_FRect frect = rect( x0, y0, x1, y1 );

		if( bound_texture < 0 || bound_texture >= MAX_TEXTURES || !textures[bound_texture] )
			return;

		SDL_RenderTexture( renderer, textures[bound_texture], NULL, &frect );
	}

	void invalidate( vgui::Panel *panel )
	{
		// printf( "%s( %p ): unimplemeted\n", __PRETTY_FUNCTION__, panel );
	}

	void enableMouseCapture( bool state )
	{
		printf( "%s( %s ): unimplemeted\n", __PRETTY_FUNCTION__, bool_to_str( state ));
	}

	void setCursor( vgui::Cursor *cursor )
	{
		vgui::Cursor::DefaultCursor dc = cursor->getDefaultCursor();

		if( dc == vgui::Cursor::dc_user )
		{
			printf( "%s( bitmap cursor ): unimplemented\n", __PRETTY_FUNCTION__ );
			return;
		}

		if( dc == vgui::Cursor::dc_none )
		{
			SDL_HideCursor();
			return;
		}

		SDL_SystemCursor sc;
		switch( dc )
		{
		case vgui::Cursor::dc_arrow:
		case vgui::Cursor::dc_last:
			sc = SDL_SYSTEM_CURSOR_DEFAULT;
			break;
		case vgui::Cursor::dc_ibeam:
			sc = SDL_SYSTEM_CURSOR_TEXT;
			break;
		case vgui::Cursor::dc_hourglass:
			sc = SDL_SYSTEM_CURSOR_WAIT;
			break;
		case vgui::Cursor::dc_crosshair:
			sc = SDL_SYSTEM_CURSOR_CROSSHAIR;
			break;
		case vgui::Cursor::dc_up:
			sc = SDL_SYSTEM_CURSOR_DEFAULT; // wtf is UP???
			break;
		case vgui::Cursor::dc_sizenwse:
			sc = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
			break;
		case vgui::Cursor::dc_sizenesw:
			sc = SDL_SYSTEM_CURSOR_NESW_RESIZE;
			break;
		case vgui::Cursor::dc_sizewe:
			sc = SDL_SYSTEM_CURSOR_EW_RESIZE;
			break;
		case vgui::Cursor::dc_sizens:
			sc = SDL_SYSTEM_CURSOR_NS_RESIZE;
			break;
		case vgui::Cursor::dc_sizeall:
			sc = SDL_SYSTEM_CURSOR_MOVE;
			break;
		case vgui::Cursor::dc_no:
			sc = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
			break;
		case vgui::Cursor::dc_hand:
			sc = SDL_SYSTEM_CURSOR_POINTER;
			break;
		default:
			sc = SDL_SYSTEM_CURSOR_DEFAULT;
			break;
		}

		cursors[sc] = SDL_CreateSystemCursor( sc );

		if( cursors[sc] )
		{
			SDL_SetCursor( cursors[sc] );
		}
		else
		{
			SDL_SetCursor( SDL_GetDefaultCursor());
		}

		SDL_ShowCursor();
	}

	void swapBuffers()
	{
		SDL_RenderPresent( renderer );
	}

	void pushMakeCurrent( vgui::Panel *panel, bool useInsets )
	{
		int insets[4] = { 0 };
		int absExtents[4];
		int clipRect[4];

		if( useInsets )
			panel->getInset( insets[0], insets[1], insets[2], insets[3] );

		panel->getAbsExtents( absExtents[0], absExtents[1], absExtents[2], absExtents[3] );
		panel->getClipRect( clipRect[0], clipRect[1], clipRect[2], clipRect[3] );

		SDL_Rect viewport;

		int w, h;

		SDL_GetWindowSizeInPixels( window, &w, &h );

		viewport.x = insets[0] + absExtents[0];
		viewport.y = insets[1] + absExtents[1];
		viewport.w = w - viewport.x;
		viewport.h = h - viewport.y;

		SDL_SetRenderViewport( renderer, &viewport );

	}

	void popMakeCurrent( vgui::Panel *panel )
	{
		SDL_SetRenderViewport( renderer, NULL );
	}

	void applyChanges()
	{
		// printf( "%s: unimplemented\n", __PRETTY_FUNCTION__ );
	}

private:
	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;

	SDL_Texture *textures[MAX_TEXTURES] = { };
	int last_texture = 0;
	int bound_texture = 0;

	int color[4] = { 0 };
	int text_color[4] = { 0 };
	int text_pos[2] = { 0 };

	SDL_Cursor *cursors[SDL_SYSTEM_CURSOR_COUNT] = { 0 };
	SDL_Cursor *user_cursor = nullptr;

	std::vector<FontTexture> fonts;

	vgui::Font *font;
	FontTexture *font_texture = nullptr;
};

class SDL_App : public vgui::App
{
public:
	SDL_App() : vgui::App( true ) { }

	bool InitSDL()
	{
		if( !SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS ))
		{
			printf( "Failed to init SDL: %s\n", SDL_GetError( ));
			return false;
		}

		return true;
	}

	bool CreateRootPanel( int width, int height )
	{
		rootpanel = new vgui::Panel;

		if( !rootpanel )
		{
			printf( "No memory to allocate Panel\n" );
			return false;
		}

		rootpanel->setSize( width, height );
		rootpanel->setPaintBackgroundEnabled( false );
		rootpanel->setPaintBorderEnabled( false );
		rootpanel->setVisible( true );
		rootpanel->setCursor( new vgui::Cursor( vgui::Cursor::dc_none ));

		return true;
	}

	bool CreateSurface()
	{
		if( !rootpanel )
			return false;

		surface = new SDL_VGUISurface( rootpanel );
		if( !surface )
		{
			printf( "No memory to allocate surface\n" );
			return false;
		}

		if( !surface->CreateWindow())
			return false;

		rootpanel->setSurfaceBaseTraverse( surface );

		return true;

	}

	vgui::Panel *RootPanel()
	{
		return rootpanel;
	}

	SDL_VGUISurface *Surface()
	{
		return surface;
	}

	void platTick() override
	{
		SDL_Event ev;

		while( SDL_PollEvent( &ev ))
		{
			switch( ev.type )
			{
			case SDL_EVENT_MOUSE_MOTION:
				internalCursorMoved( (int)ev.motion.x, (int)ev.motion.y, surface );
				break;
			case SDL_EVENT_QUIT:
				SDL_Quit();
				exit( 0 );
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				vgui::MouseCode mc;
				switch( ev.button.button )
				{
				case SDL_BUTTON_LEFT:
					mc = vgui::MouseCode::MOUSE_LEFT;
					break;
				case SDL_BUTTON_RIGHT:
					mc = vgui::MouseCode::MOUSE_RIGHT;
					break;
				case SDL_BUTTON_MIDDLE:
					mc = vgui::MouseCode::MOUSE_MIDDLE;
					break;
				default:
					mc = vgui::MouseCode::MOUSE_LAST;
					break;
				}

				if( mc != vgui::MouseCode::MOUSE_LAST )
				{
					if( ev.button.down )
						internalMousePressed( mc, surface );
					else
						internalMouseReleased( mc, surface );
				}
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				internalMouseWheeled( (int)ev.wheel.x, surface );
				break;
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
				vgui::KeyCode kc = vgui::KeyCode::KEY_LAST;

#define DECLARE_KEY_RANGE( min, max, repl ) if( ev.key.scancode >= (min) && ev.key.scancode <= (max) ) \
				{ \
	                kc = (vgui::KeyCode)(ev.key.scancode - (min) + (int)(repl)); \
			    }

				DECLARE_KEY_RANGE( SDL_SCANCODE_0, SDL_SCANCODE_9, vgui::KeyCode::KEY_0 )
				else DECLARE_KEY_RANGE( SDL_SCANCODE_F1, SDL_SCANCODE_F12, vgui::KeyCode::KEY_F1 )
				else DECLARE_KEY_RANGE( SDL_SCANCODE_A, SDL_SCANCODE_Z, vgui::KeyCode::KEY_A )
				else switch( ev.key.scancode )
				{
					// kinda lazy
			        case SDL_SCANCODE_BACKSPACE:
						kc = vgui::KeyCode::KEY_BACKSPACE;
						break;
					case SDL_SCANCODE_TAB:
						kc = vgui::KeyCode::KEY_TAB;
						break;
					case SDL_SCANCODE_RETURN:
						kc = vgui::KeyCode::KEY_ENTER;
						break;
					case SDL_SCANCODE_SPACE:
						kc = vgui::KeyCode::KEY_SPACE;
						break;
				}

				if( kc != vgui::KeyCode::KEY_LAST )
				{
					if( ev.key.down )
					{
						internalKeyPressed( kc, surface );
						internalKeyTyped( kc, surface );
					}
					else
						internalKeyReleased( kc, surface );
				}
				break;

			}
		}
	}

	void Loop( void )
	{
		while( true )
		{
			externalTick();

			rootpanel->repaint();
			rootpanel->paintTraverse();
		}
	}

	virtual void main( int argc, char *argv[] )
	{

	}
private:
	vgui::Panel *rootpanel = nullptr;
	SDL_VGUISurface *surface = nullptr;
};

class Viewport
{
public:
	Viewport( vgui::Panel *rootpanel ) :
	            rootpanel( rootpanel )
	{
		int x, y, w, h;

		size_t size;
		void *file;

		// vgui can't load system fonts, so grab temporary bitmap fonts
		file = SDL_LoadFile( "fonts/primary1.tga", &size );
		rootpanel->getApp()->getScheme()->setFont( vgui::Scheme::sf_primary1,
		        new vgui::Font( "Arial", file, size, 23, 0, 0, 0, false, false, false, false ));
		SDL_free( file );

		file = SDL_LoadFile( "fonts/primary2.tga", &size );
		rootpanel->getApp()->getScheme()->setFont( vgui::Scheme::sf_primary2,
		        new vgui::Font( "Arial", file, size, 18, 0, 0, 0, false, false, false, false ));
		SDL_free( file );

		file = SDL_LoadFile( "fonts/primary3.tga", &size );
		rootpanel->getApp()->getScheme()->setFont( vgui::Scheme::sf_primary3,
		        new vgui::Font( "Arial", file, size, 21, 0, 0, 0, false, false, false, false ));
		SDL_free( file );


		rootpanel->getPos( x, y );
		rootpanel->getSize( w, h );

		desktop = new vgui::Desktop( x, y, w, h );

		rootpanel->addChild( desktop );
	}

private:
	vgui::Panel *rootpanel;
	vgui::Desktop *desktop;
};

int main( int argc, char *argv[] )
{
	static SDL_App app;

	if( !app.InitSDL())
		return EXIT_FAILURE;

	if( !app.CreateRootPanel( 1024, 768 ))
		return EXIT_FAILURE;

	if( !app.CreateSurface())
		return EXIT_FAILURE;

	app.start();
	app.setMinimumTickMillisInterval( 0 );

	Viewport viewport( app.RootPanel( ));

	app.Loop();

	return EXIT_SUCCESS;
}
