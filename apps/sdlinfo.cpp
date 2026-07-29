// SPDX-License-Identifier: LGPL-3.0-or-later

#include "apps.h"
#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_FlowLayout.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_StackLayout.h>

// too lazy to make it proper into some grid like layout
// (gridlayout is dead API btw	)
#define NAME_WIDE 100
#define VALUE_WIDE 240
#define ROW_TALL 18

class SDLInfoFrame;

class RefreshSignal : public ActionSignal
{
public:
	RefreshSignal( SDLInfoFrame *frame ) : frame( frame ) {}

	void actionPerformed( Panel *panel ) override;

private:
	SDLInfoFrame *frame;
};

// small system-info panel driven entirely by SDL3 queries
class SDLInfoFrame : public Frame
{
public:
	SDLInfoFrame() : Frame( 0, 0, 360, 360 )
	{
		setTitle( "SDL3 Info" );
		addFrameSignal( new TestFrameSignal );

		Panel *client = getClient();
		client->setLayout( new StackLayout( 4, true ));

		platform = AddRow( client, "Platform" );
		video = AddRow( client, "Video driver" );
		renderer = AddRow( client, "Renderer" );
		window = AddRow( client, "Window" );
		display = AddRow( client, "Display" );
		cpu = AddRow( client, "CPU cores" );
		cacheline = AddRow( client, "Cache line" );
		pagesize = AddRow( client, "Page size" );
		simdalign = AddRow( client, "SIMD align" );
		ram = AddRow( client, "System RAM" );

		// "I hate x86" -- @dd86k
		client->addChild( NewLabel( "SIMD:", NAME_WIDE ));
		simd = NewLabel( "", NAME_WIDE + VALUE_WIDE );
		client->addChild( simd );

		// keep the button its own size inside a full-width flow row
		Panel *buttons = new Panel( 0, 0, 300, 28 );
		buttons->setLayout( new FlowLayout( 4 ));
		Button *refresh = new Button( "Refresh", 0, 0, 70, 24 );
		refresh->addActionSignal( new RefreshSignal( this ));
		buttons->addChild( refresh );
		client->addChild( buttons );

		Refresh();
	}

	void Refresh()
	{
		SDL_Window *win = Sys_GetWindow();
		SDL_Renderer *ren = Sys_GetRenderer();

		platform->setText( "%s", SDL_GetPlatform());

		const char *drv = SDL_GetCurrentVideoDriver();
		video->setText( "%s", drv ? drv : "?" );

		const char *name = ren ? SDL_GetRendererName( ren ) : nullptr;
		renderer->setText( "%s", name ? name : "?" );

		int wx = 0, wy = 0, ww = 0, wh = 0;
		SDL_GetWindowPosition( win, &wx, &wy );
		SDL_GetWindowSize( win, &ww, &wh );
		window->setText( "%dx%d at (%d, %d)", ww, wh, wx, wy );

		SDL_DisplayID did = SDL_GetDisplayForWindow( win );
		const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode( did );
		if( mode )
			display->setText( "%s: %dx%d @ %.0f Hz", SDL_GetDisplayName( did ), mode->w, mode->h, mode->refresh_rate );
		else
			display->setText( "?" );

		cpu->setText( "%d logical", SDL_GetNumLogicalCPUCores());
		cacheline->setText( "%d bytes", SDL_GetCPUCacheLineSize());
		pagesize->setText( "%d bytes", (int)SDL_GetSystemPageSize());
		simdalign->setText( "%d bytes", (int)SDL_GetSIMDAlignment());
		ram->setText( "%d MB", SDL_GetSystemRAM());

		// SDL3 does have a lot of nice checks huh
		simd->setText( "%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
			SDL_HasMMX() ? "MMX " : "",
			SDL_HasSSE() ? "SSE " : "",
			SDL_HasSSE2() ? "SSE2 " : "",
			SDL_HasSSE3() ? "SSE3 " : "",
			SDL_HasSSE41() ? "SSE4.1 " : "",
			SDL_HasSSE42() ? "SSE4.2 " : "",
			SDL_HasAVX() ? "AVX " : "",
			SDL_HasAVX2() ? "AVX2 " : "",
			SDL_HasAVX512F() ? "AVX512F " : "",
			SDL_HasNEON() ? "NEON " : "",
			SDL_HasARMSIMD() ? "ARMSIMD " : "",
			SDL_HasAltiVec() ? "AltiVec " : "",
			SDL_HasLSX() ? "LSX " : "",
			SDL_HasLASX() ? "LASX " : "" );
	}

private:
	Label *NewLabel( const char *text, int wide )
	{
		Label *label = new Label( text, 0, 0, wide, ROW_TALL );

		label->setContentAlignment( Label::a_west );
		label->setContentFitted( false ); // keep our width so the flow columns line up
		return label;
	}

	Label *AddRow( Panel *client, const char *name )
	{
		char text[64];

		SDL_snprintf( text, sizeof( text ), "%s:", name );

		Panel *row = new Panel( 0, 0, 300, ROW_TALL );
		row->setLayout( new FlowLayout( 4 ));
		row->addChild( NewLabel( text, NAME_WIDE ));

		Label *value = NewLabel( "", VALUE_WIDE );
		row->addChild( value );
		client->addChild( row );

		return value;
	}

	Label *platform, *video, *renderer, *window, *display;

	Label *cpu, *cacheline, *pagesize, *simdalign, *ram, *simd;
};

void RefreshSignal::actionPerformed( Panel *panel )
{
	frame->Refresh();
}

class SDLInfoMiniApp : public MiniApp
{
public:
	SDLInfoMiniApp()
	{
		setName( "SDL3 Info" );
	}

	Frame *createInstance()
	{
		return new SDLInfoFrame;
	}
};

DesktopIcon *CreateSDLInfoApp()
{
	return new DesktopIcon( new SDLInfoMiniApp(), LoadTGA( "icons/this_computer.tga" ));
}
