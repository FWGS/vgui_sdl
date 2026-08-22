// SPDX-License-Identifier: LGPL-3.0-or-later
// tiny localhost HTTP server that injects POSTed JSON events into the
// SDL event queue, so external tools can drive any SDL app, e.g.:
//   curl -d '{"type":"click","x":31,"y":26,"clicks":2}' http://127.0.0.1:4938/

#include "vgui_sdl.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int socket_t;
#define INVALID_SOCKET (-1)
#define closesocket close
#endif

// minimal flat-JSON value extraction, good enough for test input

static const char *json_find_value( const char *json, const char *key )
{
	char pattern[64];
	snprintf( pattern, sizeof( pattern ), "\"%s\"", key );

	const char *p = strstr( json, pattern );
	if( !p )
		return nullptr;

	p = strchr( p + strlen( pattern ), ':' );
	if( !p )
		return nullptr;

	for( p++; *p == ' ' || *p == '\t'; p++ );
	return p;
}

static bool json_get_string( const char *json, const char *key, char *out, size_t size )
{
	const char *p = json_find_value( json, key );

	if( !p || *p != '"' )
		return false;

	const char *end = strchr( ++p, '"' );
	if( !end || (size_t)( end - p ) >= size )
		return false;

	memcpy( out, p, end - p );
	out[end - p] = '\0';
	return true;
}

static int json_get_int( const char *json, const char *key, int fallback )
{
	const char *p = json_find_value( json, key );
	return p ? atoi( p ) : fallback;
}

static bool json_get_bool( const char *json, const char *key, bool fallback )
{
	const char *p = json_find_value( json, key );

	if( !p )
		return fallback;
	return !strncmp( p, "true", 4 );
}

// JSON -> SDL_Event
static Uint8 parse_button( const char *json )
{
	char name[16];

	if( json_get_string( json, "button", name, sizeof( name )))
	{
		if( !strcmp( name, "right" ))
			return SDL_BUTTON_RIGHT;
		if( !strcmp( name, "middle" ))
			return SDL_BUTTON_MIDDLE;
	}

	return SDL_BUTTON_LEFT;
}

static SDL_Scancode parse_key( const char *json )
{
	char name[32];

	if( !json_get_string( json, "key", name, sizeof( name )))
		return SDL_SCANCODE_UNKNOWN;

	if( !name[1] && name[0] >= 'a' && name[0] <= 'z' )
		name[0] -= 'a' - 'A';

	// fallback to SDL scancode names
	return SDL_GetScancodeFromName( name );
}

static void push_motion( int x, int y )
{
	SDL_Event ev = {};

	// clients post logical coords; scale up to window pixels so platTick's
	// divide-by-scale lands back on the same logical point
	int scale = host.scale;

	ev.type = SDL_EVENT_MOUSE_MOTION;
	ev.motion.x = (float)( x * scale );
	ev.motion.y = (float)( y * scale );
	SDL_PushEvent( &ev );
}

static void push_button( Uint8 button, bool down, int clicks )
{
	SDL_Event ev = {};

	ev.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
	ev.button.button = button;
	ev.button.down = down;
	ev.button.clicks = clicks;
	SDL_PushEvent( &ev );
}

static bool inject_event( const char *json )
{
	char type[32];

	if( !json_get_string( json, "type", type, sizeof( type )))
		return false;

	if( !strcmp( type, "motion" ))
	{
		push_motion( json_get_int( json, "x", 0 ), json_get_int( json, "y", 0 ));
	}
	else if( !strcmp( type, "button" ))
	{
		push_button( parse_button( json ), json_get_bool( json, "down", true ), json_get_int( json, "clicks", 1 ));
	}
	else if( !strcmp( type, "click" ))
	{
		// full press/release sequence at a position, clicks=2 double-clicks
		Uint8 button = parse_button( json );
		int clicks = json_get_int( json, "clicks", 1 );

		push_motion( json_get_int( json, "x", 0 ), json_get_int( json, "y", 0 ));

		for( int i = 1; i <= clicks; i++ )
		{
			push_button( button, true, i );
			push_button( button, false, i );
		}
	}
	else if( !strcmp( type, "wheel" ))
	{
		SDL_Event ev = {};

		ev.type = SDL_EVENT_MOUSE_WHEEL;
		ev.wheel.x = (float)json_get_int( json, "x", 0 );
		ev.wheel.y = (float)json_get_int( json, "y", 0 );
		SDL_PushEvent( &ev );
	}
	else if( !strcmp( type, "key" ))
	{
		SDL_Event ev = {};

		ev.type = json_get_bool( json, "down", true ) ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
		ev.key.scancode = parse_key( json );
		ev.key.down = json_get_bool( json, "down", true );

		if( ev.key.scancode == SDL_SCANCODE_UNKNOWN )
			return false;
		SDL_PushEvent( &ev );
	}
	else if( !strcmp( type, "ignoremouse" ))
	{
		// make GetMousePos report the injected cursor instead of the real
		// pointer, so a non-headless script can drag frames/sliders:
		// {ignoremouse on} -> drag events -> {ignoremouse off}. headless mode
		// turns this on by itself. takes effect immediately, nothing to wait for.
		host.ignoreMouse = json_get_bool( json, "enabled", true );
	}
	else if( !strcmp( type, "screenshot" ))
	{
		host.screenshotRequested = 1;
	}
	else if( !strcmp( type, "quit" ))
	{
		SDL_Event ev = {};

		ev.type = SDL_EVENT_QUIT;
		SDL_PushEvent( &ev );
	}
	else
	{
		return false;
	}

	return true;
}

// a body may be one object or an array of objects
static bool inject_events( char *body )
{
	bool ok = false;

	for( char *obj = strchr( body, '{' ); obj; )
	{
		char *end = strchr( obj, '}' );

		if( !end )
			break;

		*end = '\0';
		ok = inject_event( obj );
		*end = '}';

		if( !ok )
			break;
		obj = strchr( end, '{' );
	}

	return ok;
}

// barebones HTTP server
static void handle_client( socket_t client )
{
	char buf[8192];
	size_t got = 0;

	while( got < sizeof( buf ) - 1 )
	{
		int n = recv( client, buf + got, sizeof( buf ) - 1 - got, 0 );

		if( n <= 0 )
			break;

		got += n;
		buf[got] = '\0';

		char *body = strstr( buf, "\r\n\r\n" );
		if( !body )
			continue;
		body += 4;

		const char *cl = SDL_strcasestr( buf, "Content-Length:" );
		if( cl && got - ( body - buf ) < (size_t)atoi( cl + 15 ))
			continue; // body not complete yet

		const char *response = inject_events( body ) ?
			"HTTP/1.1 200 OK\r\nContent-Length: 3\r\nConnection: close\r\n\r\nok\n" :
			"HTTP/1.1 400 Bad Request\r\nContent-Length: 4\r\nConnection: close\r\n\r\nbad\n";

		send( client, response, strlen( response ), 0 );
		break;
	}

	closesocket( client );
}

// accept loop; the listening socket is already bound by EventServer_Start
static int EventServerThread( void *data )
{
	socket_t server = (socket_t)(intptr_t)data;

	for( ;; )
	{
		socket_t client = accept( server, nullptr, nullptr );

		if( client != INVALID_SOCKET )
			handle_client( client );
	}

	return 0;
}

// returns false only when the server was required but couldn't start; the caller
// treats that as fatal. bind happens synchronously here (not in the thread) so a
// taken port is detected before the app runs
bool EventServer_Start( void )
{
	const char *env = SDL_getenv( "VGUI_EVENT_PORT" );
	int port = env ? atoi( env ) : 4938;

	if( port <= 0 )
		return true; // VGUI_EVENT_PORT=0 disables the server

	// in headless there is no other way to drive or screenshot the app, and a
	// port collision would otherwise leave us talking to a *different* instance
	// (e.g. a parallel run on the same port), so a failure here must be fatal
	bool fatal = host.headless;

#ifdef _WIN32
	WSADATA wsa;

	if( WSAStartup( MAKEWORD( 2, 2 ), &wsa ))
		return !fatal;
#endif

	socket_t server = socket( AF_INET, SOCK_STREAM, 0 );

	if( server == INVALID_SOCKET )
		return !fatal;

	int on = 1;
	setsockopt( server, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof( on ));

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK ); // localhost only
	addr.sin_port = htons( port );

	if( bind( server, (sockaddr *)&addr, sizeof( addr )) < 0 || listen( server, 4 ) < 0 )
	{
		printf( "Event server: can't listen on port %d\n", port );
		closesocket( server );
		return !fatal;
	}

	printf( "Event server listening on http://127.0.0.1:%d\n", port );

	SDL_Thread *thread = SDL_CreateThread( EventServerThread, "eventserver", (void *)(intptr_t)server );

	SDL_DetachThread( thread );
	return true;
}
