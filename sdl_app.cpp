// SPDX-License-Identifier: LGPL-3.0-or-later

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#if USE_FREEVGUI_HEADERS
#include "vgui.h"
#include "app.h"
#include "panel.h"
#include "input.h"
#include "controls/desktop.h"
#include "controls/frame.h"
#include "font.h"
#else
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
#endif
#include <stdio.h>
#include <ctype.h>
#include <vector>
#include "vgui_sdl.h"

static int sys_argc;
static char **sys_argv;
static std::vector<BuildGroup *> g_buildgroups;

// the one global host state instance (declared in vgui_sdl.h)
host_s host;

void BuildMode_Register( BuildGroup *bg )
{
	g_buildgroups.push_back( bg );
}

void BuildMode_Toggle( void )
{
	static bool enabled = false;

	for( BuildGroup *bg : g_buildgroups )
		bg->setEnabled( !enabled );

	if( g_buildgroups.size() > 0 )
	{
		enabled = !enabled;
		printf( "build mode %s for %d group(s)\n", enabled ? "off" : "on", (int)g_buildgroups.size());
	}
	else
	{
		printf( "no build groups registered\n" );
	}
}

// freevgui_version() is a FreeVGUI-only extension export
typedef const char *( *freevgui_version_t )( void );

#ifdef _WIN32
#include <windows.h>
const char *Sys_FreeVGUIVersion( void )
{
	freevgui_version_t fn = (freevgui_version_t)GetProcAddress( GetModuleHandleA( "vgui.dll" ), "freevgui_version" );

	return fn ? fn() : nullptr;
}
#else
#include <dlfcn.h>
const char *Sys_FreeVGUIVersion( void )
{
	// dlopen(NULL) hands back the global symbol scope, which includes the
	// linked vgui.so, so dlsym finds the export there when present
	void *self = dlopen( nullptr, RTLD_LAZY );
	freevgui_version_t fn = self ? (freevgui_version_t)dlsym( self, "freevgui_version" ) : nullptr;

	return fn ? fn() : nullptr;
}
#endif

static int Sys_CheckParm( const char *parm )
{
	for( int i = 1; i < sys_argc; i++ )
	{
		if( !sys_argv[i] )
			continue;

		if( !SDL_strcasecmp( parm, sys_argv[i] ))
			return i;
	}

	return 0;
}

static bool Sys_GetIntFromCmdLine( const char *parm, int *out )
{
	int argIndex = Sys_CheckParm( parm );

	if( argIndex < 1 || argIndex + 1 >= sys_argc || !sys_argv[argIndex + 1] )
	{
		*out = 0;
		return false;
	}

	*out = SDL_atoi( sys_argv[argIndex + 1] );
	return true;
}

static KeyCode ScancodeToKeyCode( SDL_Scancode sc )
{
	if( sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z )
		return (KeyCode)( KeyCode::KEY_A + sc - SDL_SCANCODE_A );

	if( sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9 )
		return (KeyCode)( KeyCode::KEY_1 + sc - SDL_SCANCODE_1 );

	if( sc >= SDL_SCANCODE_F1 && sc <= SDL_SCANCODE_F12 )
		return (KeyCode)( KeyCode::KEY_F1 + sc - SDL_SCANCODE_F1 );

	if( sc >= SDL_SCANCODE_KP_1 && sc <= SDL_SCANCODE_KP_9 )
		return (KeyCode)( KeyCode::KEY_PAD_1 + sc - SDL_SCANCODE_KP_1 );

	switch( sc )
	{
	case SDL_SCANCODE_0:            return KeyCode::KEY_0;
	case SDL_SCANCODE_KP_0:         return KeyCode::KEY_PAD_0;
	case SDL_SCANCODE_KP_DIVIDE:    return KeyCode::KEY_PAD_DIVIDE;
	case SDL_SCANCODE_KP_MULTIPLY:  return KeyCode::KEY_PAD_MULTIPLY;
	case SDL_SCANCODE_KP_MINUS:     return KeyCode::KEY_PAD_MINUS;
	case SDL_SCANCODE_KP_PLUS:      return KeyCode::KEY_PAD_PLUS;
	case SDL_SCANCODE_KP_ENTER:     return KeyCode::KEY_PAD_ENTER;
	case SDL_SCANCODE_KP_PERIOD:    return KeyCode::KEY_PAD_DECIMAL;
	case SDL_SCANCODE_LEFTBRACKET:  return KeyCode::KEY_LBRACKET;
	case SDL_SCANCODE_RIGHTBRACKET: return KeyCode::KEY_RBRACKET;
	case SDL_SCANCODE_SEMICOLON:    return KeyCode::KEY_SEMICOLON;
	case SDL_SCANCODE_APOSTROPHE:   return KeyCode::KEY_APOSTROPHE;
	case SDL_SCANCODE_GRAVE:        return KeyCode::KEY_BACKQUOTE;
	case SDL_SCANCODE_COMMA:        return KeyCode::KEY_COMMA;
	case SDL_SCANCODE_PERIOD:       return KeyCode::KEY_PERIOD;
	case SDL_SCANCODE_SLASH:        return KeyCode::KEY_SLASH;
	case SDL_SCANCODE_BACKSLASH:    return KeyCode::KEY_BACKSLASH;
	case SDL_SCANCODE_MINUS:        return KeyCode::KEY_MINUS;
	case SDL_SCANCODE_EQUALS:       return KeyCode::KEY_EQUAL;
	case SDL_SCANCODE_RETURN:       return KeyCode::KEY_ENTER;
	case SDL_SCANCODE_SPACE:        return KeyCode::KEY_SPACE;
	case SDL_SCANCODE_BACKSPACE:    return KeyCode::KEY_BACKSPACE;
	case SDL_SCANCODE_TAB:          return KeyCode::KEY_TAB;
	case SDL_SCANCODE_CAPSLOCK:     return KeyCode::KEY_CAPSLOCK;
	case SDL_SCANCODE_NUMLOCKCLEAR: return KeyCode::KEY_NUMLOCK;
	case SDL_SCANCODE_ESCAPE:       return KeyCode::KEY_ESCAPE;
	case SDL_SCANCODE_SCROLLLOCK:   return KeyCode::KEY_SCROLLLOCK;
	case SDL_SCANCODE_INSERT:       return KeyCode::KEY_INSERT;
	case SDL_SCANCODE_DELETE:       return KeyCode::KEY_DELETE;
	case SDL_SCANCODE_HOME:         return KeyCode::KEY_HOME;
	case SDL_SCANCODE_END:          return KeyCode::KEY_END;
	case SDL_SCANCODE_PAGEUP:       return KeyCode::KEY_PAGEUP;
	case SDL_SCANCODE_PAGEDOWN:     return KeyCode::KEY_PAGEDOWN;
	case SDL_SCANCODE_PAUSE:        return KeyCode::KEY_BREAK;
	case SDL_SCANCODE_LSHIFT:       return KeyCode::KEY_LSHIFT;
	case SDL_SCANCODE_RSHIFT:       return KeyCode::KEY_RSHIFT;
	case SDL_SCANCODE_LALT:         return KeyCode::KEY_LALT;
	case SDL_SCANCODE_RALT:         return KeyCode::KEY_RALT;
	case SDL_SCANCODE_LCTRL:        return KeyCode::KEY_LCONTROL;
	case SDL_SCANCODE_RCTRL:        return KeyCode::KEY_RCONTROL;
	case SDL_SCANCODE_LGUI:         return KeyCode::KEY_LWIN;
	case SDL_SCANCODE_RGUI:         return KeyCode::KEY_RWIN;
	case SDL_SCANCODE_APPLICATION:  return KeyCode::KEY_APP;
	case SDL_SCANCODE_UP:           return KeyCode::KEY_UP;
	case SDL_SCANCODE_LEFT:         return KeyCode::KEY_LEFT;
	case SDL_SCANCODE_DOWN:         return KeyCode::KEY_DOWN;
	case SDL_SCANCODE_RIGHT:        return KeyCode::KEY_RIGHT;
	default:                        return KeyCode::KEY_LAST;
	}
}

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
#if USE_FREEVGUI_HEADERS
		rootpanel->setCursor( new Cursor( Cursor::DC_NONE ));
#else
		rootpanel->setCursor( new Cursor( Cursor::dc_none ));
#endif

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

	// implement these methods to avoid crash in linux vgui.so 
	char getKeyCodeChar( KeyCode code, bool shifted ) override
	{
		if( code >= KeyCode::KEY_A && code <= KeyCode::KEY_Z )
			return ( shifted ? 'A' : 'a' ) + ( code - KeyCode::KEY_A );

		if( code >= KeyCode::KEY_0 && code <= KeyCode::KEY_9 )
			return shifted ? ")!@#$%^&*("[code - KeyCode::KEY_0] : '0' + ( code - KeyCode::KEY_0 );

		if( code >= KeyCode::KEY_PAD_0 && code <= KeyCode::KEY_PAD_9 )
			return '0' + ( code - KeyCode::KEY_PAD_0 );

		switch( code )
		{
		case KeyCode::KEY_PAD_DIVIDE:   return '/';
		case KeyCode::KEY_PAD_MULTIPLY: return '*';
		case KeyCode::KEY_PAD_MINUS:    return '-';
		case KeyCode::KEY_PAD_PLUS:     return '+';
		case KeyCode::KEY_PAD_DECIMAL:  return '.';
		case KeyCode::KEY_LBRACKET:     return shifted ? '{' : '[';
		case KeyCode::KEY_RBRACKET:     return shifted ? '}' : ']';
		case KeyCode::KEY_SEMICOLON:    return shifted ? ':' : ';';
		case KeyCode::KEY_APOSTROPHE:   return shifted ? '"' : '\'';
		case KeyCode::KEY_BACKQUOTE:    return shifted ? '~' : '`';
		case KeyCode::KEY_COMMA:        return shifted ? '<' : ',';
		case KeyCode::KEY_PERIOD:       return shifted ? '>' : '.';
		case KeyCode::KEY_SLASH:        return shifted ? '?' : '/';
		case KeyCode::KEY_BACKSLASH:    return shifted ? '|' : '\\';
		case KeyCode::KEY_MINUS:        return shifted ? '_' : '-';
		case KeyCode::KEY_EQUAL:        return shifted ? '+' : '=';
		case KeyCode::KEY_SPACE:        return ' ';
		default:                        return 0;
		}
	}

	void getKeyCodeText( KeyCode code, char *buf, int buflen ) override
	{
		if( !buf || buflen < 2 )
			return;

		char ch = getKeyCodeChar( code, false );

		buf[0] = ch;
		buf[1] = '\0';

		if( ch )
			return;

		switch( code )
		{
		case KeyCode::KEY_ENTER:     snprintf( buf, buflen, "Enter" ); break;
		case KeyCode::KEY_ESCAPE:    snprintf( buf, buflen, "Escape" ); break;
		case KeyCode::KEY_BACKSPACE: snprintf( buf, buflen, "Backspace" ); break;
		case KeyCode::KEY_TAB:       snprintf( buf, buflen, "Tab" ); break;
		default:                     buf[0] = '\0'; break;
		}
	}

	void setClipboardText( const char *text, int textLen ) override
	{
		char *copy = (char *)SDL_malloc( textLen + 1 );

		SDL_memcpy( copy, text, textLen );
		copy[textLen] = '\0';
		SDL_SetClipboardText( copy );
		SDL_free( copy );
	}

	int getClipboardTextCount() override
	{
		char *text = SDL_GetClipboardText();
		int count = (int)SDL_strlen( text );

		SDL_free( text );

		return count;
	}

	int getClipboardText( int offset, char *buf, int bufLen ) override
	{
		char *text = SDL_GetClipboardText();
		int len = (int)SDL_strlen( text );
		int copied = 0;

		if( offset < len && bufLen > 0 )
		{
			copied = len - offset;

			if( copied > bufLen - 1 )
				copied = bufLen - 1;

			SDL_memcpy( buf, text + offset, copied );
			buf[copied] = '\0';
		}

		SDL_free( text );

		return copied;
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
			{
				// window-local pixels down to the logical coordinate space
				int lx = (int)ev.motion.x / host.scale;
				int ly = (int)ev.motion.y / host.scale;

				// remember it so GetMousePos can report it in ignore-mouse mode
				// (lets scripted drags/sliders track the injected cursor)
				host.injMouseX = lx;
				host.injMouseY = ly;
				internalCursorMoved( lx, ly, surface );
				break;
			}
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
					{
						// SDL's clicks field is a monotonic counter that keeps
						// climbing (1,2,3,4...) for the whole multi-click window,
						// so testing clicks>1 would route every rapid click as a
						// double and starve single-click widgets until it resets.
						// Instead mirror Win32/original-VGUI: every press is a real
						// press, and a completed double (clicks==2) delivers an
						// extra doublePressed on top, pressed-then-double.
						internalMousePressed( mc, surface );

						if( ev.button.clicks == 2 )
							internalMouseDoublePressed( mc, surface );
					}
					else
						internalMouseReleased( mc, surface );
				}
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				internalMouseWheeled( (int)ev.wheel.y, surface );
				break;
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
			{
				// in-game this was Ctrl+Shift+Alt+B; the hotkey is app glue,
				// build mode itself ships with VGUI.

				// NOTE it is NOT App::enableBuildMode - that flag only freezes the App input dispatch
				if( ev.key.down && ev.key.scancode == SDL_SCANCODE_B && ( ev.key.mod & SDL_KMOD_CTRL ))
				{
					BuildMode_Toggle();
					break;
				}

				// Ctrl+R: outline every panel's bounds, Ctrl+E: label them with
				// their class name (only visible while bounds are on)
				if( ev.key.down && ev.key.scancode == SDL_SCANCODE_R && ( ev.key.mod & SDL_KMOD_CTRL ))
				{
					host.drawBounds = !host.drawBounds;
					break;
				}

				if( ev.key.down && ev.key.scancode == SDL_SCANCODE_E && ( ev.key.mod & SDL_KMOD_CTRL ))
				{
					host.drawLabels = !host.drawLabels;
					break;
				}

				KeyCode kc = ScancodeToKeyCode( ev.key.scancode );

				if( kc == KeyCode::KEY_LAST )
					break;

				if( ev.key.down )
				{
					// key auto-repeat arrives as extra down events, same as
					// the WM_KEYDOWN repeats vgui was written against
					internalKeyPressed( kc, surface );
					internalKeyTyped( kc, surface );
				}
				else
					internalKeyReleased( kc, surface );
				break;
			}
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

			// one completed frame (paintTraverse calls swapBuffers, which
			// presents and writes any pending screenshot). the event server
			// waits on this so HTTP replies only after the work is rendered
			host.frameCount++;
		}
	}

	virtual void main( int argc, char *argv[] )
	{

	}
private:
	Panel *rootpanel = nullptr;
	SDLSurface *surface = nullptr;
};

#ifndef _WIN32
static void ScreenshotSignalHandler( int )
{
	host.screenshotRequested = 1;
}
#endif

int main( int argc, char *argv[] )
{
	static SDLApp app;
	int width, height;

	sys_argc = argc;
	sys_argv = argv;

	if( !Sys_GetIntFromCmdLine( "-width", &width ))
		width = 640;

	if( !Sys_GetIntFromCmdLine( "-height", &height ))
		height = 480;

	int scale;

	if( !Sys_GetIntFromCmdLine( "-scale", &scale ))
		scale = 1;

	Sys_SetRenderScale( scale );

	// force the software renderer (the hint is read when the renderer is created)
	if( Sys_CheckParm( "-software" ))
		SDL_SetHint( SDL_HINT_RENDER_DRIVER, "software" );

	// create the window hidden so scripted/test runs don't flash windows or
	// steal focus (offscreen with SDL_VIDEODRIVER=offscreen for no display).
	// implies ignore-mouse, since a hidden window can't receive real input.
	// non-headless scripts toggle ignore-mouse at runtime via the event server
	if( Sys_CheckParm( "-headless" ))
		Sys_SetHeadless( true );

#ifndef _WIN32
	signal( SIGUSR1, ScreenshotSignalHandler );
#endif

	// fatal in headless: without the event server there is no way to drive or
	// screenshot the app, and a taken port would mean cross-talk with another
	// instance (e.g. a parallel test run)
	if( !EventServer_Start())
	{
		printf( "Fatal: event server could not start (port in use?)\n" );
		return EXIT_FAILURE;
	}

	if( !app.InitSDL())
		return EXIT_FAILURE;

	if( !app.CreateRootPanel( width, height ))
		return EXIT_FAILURE;

	if( !app.CreateSurface())
		return EXIT_FAILURE;

	app.start();
	app.setMinimumTickMillisInterval( 0 );

	CreateViewport( app.RootPanel());

	app.Loop();

	return EXIT_SUCCESS;
}
