#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "font.h"
#else
#include <VGUI_Font.h>
#endif
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>
#include <cxxabi.h>

static SDL_Window *s_window;
static SDL_Renderer *s_renderer;

// host.scale: integer nearest-neighbour render scale (-scale N). VGUI keeps
// drawing in its logical coordinate space; the window is N times larger and
// SDL_SetRenderScale upscales. Input coords are divided back down. Clamped >=1.
void Sys_SetRenderScale( int scale )
{
	host.scale = scale > 1 ? scale : 1;
}

// host.headless (-headless): create the window hidden so test runs don't flash
// windows or steal focus. Rendering, screenshots (SDL_RenderReadPixels) and
// injected input all still work; pair with SDL_VIDEODRIVER=offscreen to run
// with no display at all. Real mouse input can't reach a hidden window, so
// headless also forces host.ignoreMouse (GetMousePos returns the injected pos).
void Sys_SetHeadless( bool headless )
{
	host.headless = headless;
	if( headless )
		host.ignoreMouse = true;
}

SDL_FRect SDLSurface::rect( int x0, int y0, int x1, int y1 )
{
	SDL_FRect frect = { (float)( x0 + origin[0] ), (float)( y0 + origin[1] ), (float)( x1 - x0 ), (float)( y1 - y0 ) };
	return frect;
}

SDLSurface::~SDLSurface()
{
	for( int i = 0; i < sizeof( cursors ) / sizeof( cursors[0] ); i++ )
	{
		SDL_DestroyCursor( cursors[i] );
		cursors[i] = nullptr;
	}

	for( int i = 0; i < sizeof( textures ) / sizeof( textures[0] ); i++ )
	{
		SDL_DestroyTexture( textures[i] );
		textures[i] = nullptr;
	}

	fonts.clear();

	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
}

SDL_Window *Sys_GetWindow( void )
{
	return s_window;
}

SDL_Renderer *Sys_GetRenderer( void )
{
	return s_renderer;
}

// debug overlay: outline every visible panel, coloured by tree depth, drawn
// straight on the SDL_Renderer in swapBuffers so VGUI never sees it. host.drawBounds
// shows the rects, host.drawLabels the class-name labels; toggled in sdl_app.cpp
static void DrawPanelBounds( SDL_Renderer *renderer, Panel *panel, int depth )
{
	static const struct { Uint8 r, g, b; } colors[] =
	{
		{ 255,  64,  64 }, {  64, 255,  64 }, {  96, 160, 255 }, { 255, 255,  64 },
		{ 255,  96, 255 }, {  64, 255, 255 }, { 255, 160,  64 }, { 160,  96, 255 },
	};

	// an invisible panel hides its whole subtree, so prune here
	if( !panel->isVisible())
		return;

	// Panel::_paintEnabled has no public getter, so read it by its offset. The
	// 186 offset is from the 32-bit (ILP32) ABI this testbed is built against;
	// on any other data model the layout differs, so fall back to outlining
	// everything. Skipping paint-disabled panels drops Frame's invisible grips,
	// but we still recurse: paint-disabled containers such as the desktop
	// foreground still host visible children
#if defined( __ILP32__ ) || __SIZEOF_POINTER__ == 4
	bool paintEnabled = *(const bool *)( (const char *)panel + 186 );
#else
	bool paintEnabled = true;
#endif

	if( paintEnabled )
	{
		int x0, y0, x1, y1;
		panel->getAbsExtents( x0, y0, x1, y1 );

		int i = depth % ( sizeof( colors ) / sizeof( colors[0] ));
		SDL_SetRenderDrawColor( renderer, colors[i].r, colors[i].g, colors[i].b, 255 );

		SDL_FRect frect = { (float)x0, (float)y0, (float)( x1 - x0 ), (float)( y1 - y0 ) };
		SDL_RenderRect( renderer, &frect );

		if( host.drawLabels )
		{
			int status;
			char *demangled = abi::__cxa_demangle( typeid( *panel ).name(), NULL, NULL, &status );
			const char *name = ( status == 0 && demangled ) ? demangled : typeid( *panel ).name();

			if( !strncmp( name, "vgui::", 6 ))
				name += 6;

			SDL_RenderDebugText( renderer, x0 + 2, y0 + 2, name );
			free( demangled );
		}
	}

	for( int c = 0; c < panel->getChildCount(); c++ )
		DrawPanelBounds( renderer, panel->getChild( c ), depth + 1 );
}

bool SDLSurface::CreateWindow()
{
	int wide, tall;
	getPanel()->getSize( wide, tall );

	// the window is host.scale times the logical size; VGUI still draws at the
	// logical size and SDL_SetRenderScale stretches it to fill the window
	SDL_WindowFlags flags = host.headless ? SDL_WINDOW_HIDDEN : 0;

	if( !SDL_CreateWindowAndRenderer( "test", wide * host.scale, tall * host.scale, flags, &window, &renderer ))
	{
		printf( "Can't create SDL window and surface: %s\n", SDL_GetError());
		return false;
	}

	if( host.scale > 1 )
		SDL_SetRenderScale( renderer, (float)host.scale, (float)host.scale );

	// nearest keeps the bitmap fonts and icons crisp under any scaling
	SDL_SetDefaultTextureScaleMode( renderer, SDL_SCALEMODE_NEAREST );

	s_window = window;
	s_renderer = renderer;

	return true;
}

void SDLSurface::setTitle( const char *title )
{
	SDL_SetWindowTitle( window, title );
}

bool SDLSurface::setFullscreenMode( int wide, int tall, int bpp )
{
	printf( "%s( %d, %d, %d ): unimplemented\n", __PRETTY_FUNCTION__, wide, tall, bpp  );
	return false;
}

void SDLSurface::setWindowedMode()
{
	printf( "%s(): unimplemented\n", __PRETTY_FUNCTION__ );
}

void SDLSurface::setAsTopMost( bool state )
{
	printf( "%s( %s ): unimplemented\n", __PRETTY_FUNCTION__, bool_to_str( state ));
}

void SDLSurface::createPopup( Panel *embeddedPanel )
{
	printf( "%s( %p ): unimplemented\n", __PRETTY_FUNCTION__, embeddedPanel );
}

bool SDLSurface::hasFocus()
{
	// a hidden (headless) window never gets real input/mouse focus, but VGUI
	// gates focus acquisition and keyboard routing on this -- report focused so
	// scripted runs behave identically to a normal, focused window
	if( host.headless )
		return true;

	return SDL_GetWindowFlags( window ) & (SDL_WINDOW_INPUT_FOCUS|SDL_WINDOW_MOUSE_FOCUS);
}

bool SDLSurface::isWithin( int x, int y )
{
	int w, h;

	SDL_GetWindowSizeInPixels( window, &w, &h );

	// x, y are logical; the window is host.scale times larger
	w /= host.scale;
	h /= host.scale;

	return x >= 0 && x < w && y >= 0 && y < h;
}

int SDLSurface::createNewTextureID( void )
{
	return last_texture++;
}

void SDLSurface::GetMousePos( int &x, int &y )
{
	// return the last injected cursor position instead of the real pointer, so
	// scripted drags (frames, sliders) work; also the only sane source when the
	// window is hidden and the real pointer can't be over it
	if( host.ignoreMouse )
	{
		x = host.injMouseX;
		y = host.injMouseY;
		return;
	}

	float fx, fy;
	int wx, wy;

	// must be in the same coordinate space as isWithin
	SDL_GetGlobalMouseState( &fx, &fy );
	SDL_GetWindowPosition( window, &wx, &wy );

	// window-local pixels down to the logical coordinate space
	x = ( (int)fx - wx ) / host.scale;
	y = ( (int)fy - wy ) / host.scale;
}

void SDLSurface::addModeInfo( int wide, int tall, int bpp )
{
	printf( "%s( %d, %d, %d ): unimplemented\n", __PRETTY_FUNCTION__, wide, tall, bpp );
}

void SDLSurface::drawSetColor( int r, int g, int b, int a )
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = 255 - a;

	SDL_SetRenderDrawColor( renderer, r, g, b, a );
}

void SDLSurface::drawFilledRect( int x0, int y0, int x1, int y1 )
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

void SDLSurface::drawOutlinedRect( int x0, int y0, int x1, int y1 )
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

void SDLSurface::drawSetTextFont( Font *font )
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
			new_texture.rects[i].x = x;
			new_texture.rects[i].y = y;
			new_texture.rects[i].w = width;
			new_texture.rects[i].h = height;

			x += width + 1;
		}

		drawSetTextureRGBA( new_texture.texture, (char*)rgba, texture_width, texture_height );
		free( rgba );
		fonts.push_back( new_texture );

		font_texture = &fonts[fonts.size() - 1];
	}
}

void SDLSurface::drawSetTextColor( int r, int g, int b, int a )
{
	text_color[0] = r;
	text_color[1] = g;
	text_color[2] = b;
	text_color[3] = 255 - a;
}

void SDLSurface::drawSetTextPos( int x, int y )
{
	text_pos[0] = x;
	text_pos[1] = y;
}

void SDLSurface::drawPrintText( const char *text, int textLen )
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

		SDL_FRect srcrect = font_texture->rects[ch];

		SDL_FRect dstrect = {
		    (float)( text_pos[0] + a + origin[0] ),
		    (float)( text_pos[1] + origin[1] ),
		    (float)b,
		    (float)height
		};

		SDL_RenderTexture( renderer, tex, &srcrect, &dstrect );

		text_pos[0] += a + b + c;
	}
}

void SDLSurface::drawSetTextureRGBA( int id, const char *rgba, int wide, int tall )
{
	if( id < 0 || id >= MAX_TEXTURES )
		return;

	if( textures[id] )
		SDL_DestroyTexture( textures[id] );

	SDL_Surface *surf = SDL_CreateSurfaceFrom( wide, tall, SDL_PIXELFORMAT_RGBA32, (void *)rgba, wide * 4 );

#if 0
	char path[256];
	snprintf( path, sizeof( path ), "tex-%i.png", id );
	SDL_SavePNG( surf, path );
#endif

	textures[id] = SDL_CreateTextureFromSurface( renderer, surf );

	if( textures[id] )
		SDL_SetTextureBlendMode( textures[id], SDL_BLENDMODE_BLEND );

	SDL_DestroySurface( surf );
}

void SDLSurface::drawSetTexture( int id )
{
	bound_texture = id;
}

void SDLSurface::drawTexturedRect( int x0, int y0, int x1, int y1 )
{
	SDL_FRect frect = rect( x0, y0, x1, y1 );

	if( bound_texture < 0 || bound_texture >= MAX_TEXTURES || !textures[bound_texture] )
		return;

	SDL_RenderTexture( renderer, textures[bound_texture], NULL, &frect );
}

void SDLSurface::invalidate( Panel *panel )
{
	// printf( "%s( %p ): unimplemeted\n", __PRETTY_FUNCTION__, panel );
}

void SDLSurface::enableMouseCapture( bool state )
{
	printf( "%s( %s ): unimplemeted\n", __PRETTY_FUNCTION__, bool_to_str( state ));
}

void SDLSurface::setCursor( Cursor *cursor )
{
	Cursor::DefaultCursor dc = cursor->getDefaultCursor();

#if USE_FREEVGUI_HEADERS
	if( dc == Cursor::DC_USER )
#else
	if( dc == Cursor::dc_user )
#endif
	{
		printf( "%s( bitmap cursor ): unimplemented\n", __PRETTY_FUNCTION__ );
		return;
	}

#if USE_FREEVGUI_HEADERS
	if( dc == Cursor::DC_NONE )
#else
	if( dc == Cursor::dc_none )
#endif
	{
		SDL_HideCursor();
		return;
	}

	SDL_SystemCursor sc;
	switch( dc )
	{
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_ARROW:
#else
	case Cursor::dc_arrow:
#endif
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_LAST:
#else
	case Cursor::dc_last:
#endif
		sc = SDL_SYSTEM_CURSOR_DEFAULT;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_IBEAM:
#else
	case Cursor::dc_ibeam:
#endif
		sc = SDL_SYSTEM_CURSOR_TEXT;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_HOURGLASS:
#else
	case Cursor::dc_hourglass:
#endif
		sc = SDL_SYSTEM_CURSOR_WAIT;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_CROSSHAIR:
#else
	case Cursor::dc_crosshair:
#endif
		sc = SDL_SYSTEM_CURSOR_CROSSHAIR;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_UP:
#else
	case Cursor::dc_up:
#endif
		sc = SDL_SYSTEM_CURSOR_DEFAULT; // wtf is UP???
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_SIZENWSE:
#else
	case Cursor::dc_sizenwse:
#endif
		sc = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_SIZENESW:
#else
	case Cursor::dc_sizenesw:
#endif
		sc = SDL_SYSTEM_CURSOR_NESW_RESIZE;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_SIZEWE:
#else
	case Cursor::dc_sizewe:
#endif
		sc = SDL_SYSTEM_CURSOR_EW_RESIZE;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_SIZENS:
#else
	case Cursor::dc_sizens:
#endif
		sc = SDL_SYSTEM_CURSOR_NS_RESIZE;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_SIZEALL:
#else
	case Cursor::dc_sizeall:
#endif
		sc = SDL_SYSTEM_CURSOR_MOVE;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_NO:
#else
	case Cursor::dc_no:
#endif
		sc = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
		break;
#if USE_FREEVGUI_HEADERS
	case Cursor::DC_HAND:
#else
	case Cursor::dc_hand:
#endif
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

static void TakeScreenshot( SDL_Renderer *renderer )
{
	SDL_Surface *surf = SDL_RenderReadPixels( renderer, nullptr );

	if( !surf )
	{
		printf( "Can't read pixels for screenshot: %s\n", SDL_GetError());
		return;
	}

	static int counter = 0;
	char timestamp[32];
	char path[256];

	if( host.screenshotPath[0] )
	{
		// caller supplied an explicit filename (e.g. from a test script)
		snprintf( path, sizeof( path ), "%s", host.screenshotPath );
		host.screenshotPath[0] = '\0';
	}
	else
	{
		time_t now = time( nullptr );
		strftime( timestamp, sizeof( timestamp ), "%Y%m%d-%H%M%S", localtime( &now ));

		snprintf( path, sizeof( path ), "screenshot-%s-%03d.png", timestamp, counter++ );
	}

	if( SDL_SavePNG( surf, path ))
		printf( "Saved screenshot to %s\n", path );
	else
		printf( "Can't save screenshot to %s: %s\n", path, SDL_GetError());

	SDL_DestroySurface( surf );
}

void SDLSurface::swapBuffers()
{
	// overlay the debug bounds before the screenshot so they are captured too.
	// reset the clip first, otherwise the last panel's scissor would crop them
	if( host.drawBounds )
	{
		SDL_SetRenderClipRect( renderer, NULL );
		DrawPanelBounds( renderer, getPanel(), 0 );
	}

	// must happen before present: backbuffer contents are undefined afterwards.
	// clear the flag only after the file is written, so the event server can
	// wait on it going back to 0 and reply once the screenshot is on disk
	if( host.screenshotRequested )
	{
		TakeScreenshot( renderer );
		host.screenshotRequested = 0;
	}

	SDL_RenderPresent( renderer );
}

void SDLSurface::pushMakeCurrent( Panel *panel, bool useInsets )
{
	int insets[4] = { 0 };
	int absExtents[4];
	int cliprects[4];

	if( useInsets )
		panel->getInset( insets[0], insets[1], insets[2], insets[3] );

	panel->getAbsExtents( absExtents[0], absExtents[1], absExtents[2], absExtents[3] );
	panel->getClipRect( cliprects[0], cliprects[1], cliprects[2], cliprects[3] );

	origin[0] = insets[0] + absExtents[0];
	origin[1] = insets[1] + absExtents[1];

	// scissor to the panel's clip rect, otherwise scrolled content paints
	// outside its clipping parent, e.g. ScrollPanel's client over the
	// frame decorations
	SDL_Rect clip;

	clip.x = cliprects[0];
	clip.y = cliprects[1];
	clip.w = cliprects[2] - cliprects[0];
	clip.h = cliprects[3] - cliprects[1];

	SDL_SetRenderClipRect( renderer, &clip );
}

void SDLSurface::popMakeCurrent( Panel *panel )
{
	SDL_SetRenderClipRect( renderer, NULL );
	origin[0] = origin[1] = 0;
}

void SDLSurface::applyChanges()
{
	// printf( "%s: unimplemented\n", __PRETTY_FUNCTION__ );
}
