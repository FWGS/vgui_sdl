#include "vgui_sdl.h"
#include <VGUI_Font.h>
#include <ctype.h>

static inline SDL_FRect rect( int x0, int y0, int x1, int y1 )
{
	SDL_FRect frect = { (float)x0, (float)y0, (float)(x1 - x0), (float)(y1 - y0) };
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

bool SDLSurface::CreateWindow()
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
	return SDL_GetWindowFlags( window ) & (SDL_WINDOW_INPUT_FOCUS|SDL_WINDOW_MOUSE_FOCUS);
}

bool SDLSurface::isWithin( int x, int y )
{
	return SDL_GetWindowFlags( window ) & SDL_WINDOW_MOUSE_FOCUS;
}

int SDLSurface::createNewTextureID( void )
{
	return last_texture++;
}

void SDLSurface::GetMousePos( int &x, int &y )
{
	float fx, fy;

	SDL_GetGlobalMouseState( &fx, &fy );

	x = (int)fx;
	y = (int)fy;
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
		    (float)text_pos[0] + (float)a,
		    (float)text_pos[1],
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
	snprintf( path, sizeof( path ), "tex/%i.bmp", id );
	SDL_SaveBMP( surf, path );
#endif

	textures[id] = SDL_CreateTextureFromSurface( renderer, surf );

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

	if( dc == Cursor::dc_user )
	{
		printf( "%s( bitmap cursor ): unimplemented\n", __PRETTY_FUNCTION__ );
		return;
	}

	if( dc == Cursor::dc_none )
	{
		SDL_HideCursor();
		return;
	}

	SDL_SystemCursor sc;
	switch( dc )
	{
	case Cursor::dc_arrow:
	case Cursor::dc_last:
		sc = SDL_SYSTEM_CURSOR_DEFAULT;
		break;
	case Cursor::dc_ibeam:
		sc = SDL_SYSTEM_CURSOR_TEXT;
		break;
	case Cursor::dc_hourglass:
		sc = SDL_SYSTEM_CURSOR_WAIT;
		break;
	case Cursor::dc_crosshair:
		sc = SDL_SYSTEM_CURSOR_CROSSHAIR;
		break;
	case Cursor::dc_up:
		sc = SDL_SYSTEM_CURSOR_DEFAULT; // wtf is UP???
		break;
	case Cursor::dc_sizenwse:
		sc = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
		break;
	case Cursor::dc_sizenesw:
		sc = SDL_SYSTEM_CURSOR_NESW_RESIZE;
		break;
	case Cursor::dc_sizewe:
		sc = SDL_SYSTEM_CURSOR_EW_RESIZE;
		break;
	case Cursor::dc_sizens:
		sc = SDL_SYSTEM_CURSOR_NS_RESIZE;
		break;
	case Cursor::dc_sizeall:
		sc = SDL_SYSTEM_CURSOR_MOVE;
		break;
	case Cursor::dc_no:
		sc = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
		break;
	case Cursor::dc_hand:
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

void SDLSurface::swapBuffers()
{
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

	SDL_Rect viewport;

	int w, h;

	SDL_GetWindowSizeInPixels( window, &w, &h );

	viewport.x = insets[0] + absExtents[0];
	viewport.y = insets[1] + absExtents[1];
	viewport.w = w - viewport.x;
	viewport.h = h - viewport.y;

	SDL_SetRenderViewport( renderer, &viewport );
}

void SDLSurface::popMakeCurrent( Panel *panel )
{
	SDL_SetRenderViewport( renderer, NULL );
}

void SDLSurface::applyChanges()
{
	// printf( "%s: unimplemented\n", __PRETTY_FUNCTION__ );
}
