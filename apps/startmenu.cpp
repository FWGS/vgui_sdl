// SPDX-License-Identifier: LGPL-3.0-or-later

#include "apps.h"
#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_Menu.h>
#include <VGUI_MenuItem.h>
#include <VGUI_MenuSeparator.h>
#include <VGUI_MiniApp.h>

static DesktopIcon *(*const s_apps[])() =
{
	CreateSDLInfoApp,
	CreateVGUIInfoApp,
	CreateWebcamApp,
	CreateEventTesterApp,
};

#define NUM_APPS ( (int)( sizeof( s_apps ) / sizeof( s_apps[0] )))
#define TASKBAR_TALL 36 // TODO: get rid of this...
#define ITEM_TALL 26

class LaunchAppSignal : public ActionSignal
{
public:
	LaunchAppSignal( DesktopIcon *icon, Menu *menu ) : icon( icon ), menu( menu ) {}

	void actionPerformed( Panel *panel ) override
	{
		// calls the app the same way desktop does with the icon
		menu->setVisible( false );
		icon->doActivate();
	}

private:
	DesktopIcon *icon;
	Menu *menu;
};

class StartToggleSignal : public ActionSignal
{
public:
	StartToggleSignal( Menu *menu ) : menu( menu ) {}

	void actionPerformed( Panel *panel ) override
	{
		menu->setVisible( !menu->isVisible());
	}

private:
	Menu *menu;
};

class ShutdownSignal : public ActionSignal
{
public:
	void actionPerformed( Panel *panel ) override
	{
		// there is no other way to close VGUI from inside VGUI APIs
		// so just push SDL_QUIT event to the event queue
		SDL_Event ev = {};

		ev.type = SDL_EVENT_QUIT;
		SDL_PushEvent( &ev );
	}
};

void CreateStartMenu( DesktopEx *desktop )
{
	int deskWide, deskTall;

	desktop->getSize( deskWide, deskTall );

	// TODO: get rid of this too...
	int menuTall = ( NUM_APPS + 2 ) * ITEM_TALL + 8;

	Menu *menu = new Menu( 2, deskTall - TASKBAR_TALL - menuTall, 150, menuTall );
	menu->setVisible( false );

	for( int i = 0; i < NUM_APPS; i++ )
	{
		DesktopIcon *icon = s_apps[i]();
		char name[64];

		// the launcher needs a desktop for doActivate to reach iconActivated
		icon->setDesktop( desktop );
		icon->getMiniApp()->getName( name, sizeof( name ));

		MenuItem *item = new MenuItem( name );
		item->addActionSignal( new LaunchAppSignal( icon, menu ));
		menu->addMenuItem( item );
	}

	menu->addMenuItem( new MenuSeparator( "" ));

	MenuItem *shutdown = new MenuItem( "Shut Down" );
	shutdown->addActionSignal( new ShutdownSignal );
	menu->addMenuItem( shutdown );

	desktop->addChild( menu );
	desktop->getTaskBar()->getStartButton()->addActionSignal( new StartToggleSignal( menu ));
}
