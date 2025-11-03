// SPDX-License-Identifier: LGPL-3.0-or-later

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <VGUI.h>
#include <VGUI_App.h>
#include <VGUI_Panel.h>
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
#include "vgui_sdl.h"

class SDLApp : public App
{
public:
	SDLApp() : App( true ) { }

	virtual ~SDLApp()
	{
		delete surface;
		delete rootpanel;

		surface = nullptr;
		rootpanel = nullptr;

		SDL_Quit();
	}

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
		rootpanel = new Panel;

		if( !rootpanel )
		{
			printf( "No memory to allocate Panel\n" );
			return false;
		}

		rootpanel->setSize( width, height );
		rootpanel->setPaintBackgroundEnabled( false );
		rootpanel->setPaintBorderEnabled( false );
		rootpanel->setVisible( true );
		rootpanel->setCursor( new Cursor( Cursor::dc_none ));

		return true;
	}

	bool CreateSurface()
	{
		if( !rootpanel )
			return false;

		surface = new SDLSurface( rootpanel );
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

	Panel *RootPanel()
	{
		return rootpanel;
	}

	SDLSurface *Surface()
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
				MouseCode mc;
				switch( ev.button.button )
				{
				case SDL_BUTTON_LEFT:
					mc = MouseCode::MOUSE_LEFT;
					break;
				case SDL_BUTTON_RIGHT:
					mc = MouseCode::MOUSE_RIGHT;
					break;
				case SDL_BUTTON_MIDDLE:
					mc = MouseCode::MOUSE_MIDDLE;
					break;
				default:
					mc = MouseCode::MOUSE_LAST;
					break;
				}

				if( mc != MouseCode::MOUSE_LAST )
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
				KeyCode kc = KeyCode::KEY_LAST;

#define DECLARE_KEY_RANGE( min, max, repl ) if( ev.key.scancode >= (min) && ev.key.scancode <= (max) ) \
				{ \
	                kc = (KeyCode)(ev.key.scancode - (min) + (int)(repl)); \
			    }

				DECLARE_KEY_RANGE( SDL_SCANCODE_0, SDL_SCANCODE_9, KeyCode::KEY_0 )
				else DECLARE_KEY_RANGE( SDL_SCANCODE_F1, SDL_SCANCODE_F12, KeyCode::KEY_F1 )
				else DECLARE_KEY_RANGE( SDL_SCANCODE_A, SDL_SCANCODE_Z, KeyCode::KEY_A )
				else switch( ev.key.scancode )
				{
					// kinda lazy
			        case SDL_SCANCODE_BACKSPACE:
						kc = KeyCode::KEY_BACKSPACE;
						break;
					case SDL_SCANCODE_TAB:
						kc = KeyCode::KEY_TAB;
						break;
					case SDL_SCANCODE_RETURN:
						kc = KeyCode::KEY_ENTER;
						break;
					case SDL_SCANCODE_SPACE:
						kc = KeyCode::KEY_SPACE;
						break;
				}

				if( kc != KeyCode::KEY_LAST )
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
	Panel *rootpanel = nullptr;
	SDLSurface *surface = nullptr;
};

int main( int argc, char *argv[] )
{
	static SDLApp app;

	if( !app.InitSDL())
		return EXIT_FAILURE;

	if( !app.CreateRootPanel( 1024, 768 ))
		return EXIT_FAILURE;

	if( !app.CreateSurface())
		return EXIT_FAILURE;

	app.start();
	app.setMinimumTickMillisInterval( 0 );

	CreateViewport( app.RootPanel());

	app.Loop();

	return EXIT_SUCCESS;
}
