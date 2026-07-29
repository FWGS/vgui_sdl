// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_Menu.h>
#include <VGUI_MenuItem.h>
#include <VGUI_MenuSeparator.h>
#include <VGUI_MiniApp.h>

// we don't implement showPopup, showcase menus as is
class MenuToggleSignal : public ActionSignal
{
public:
	MenuToggleSignal( Menu *menu ) : menu( menu )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		menu->setVisible( !menu->isVisible());
	}

private:
	Menu *menu;
};

class MenuPickSignal : public ActionSignal
{
public:
	MenuPickSignal( Label *label, Menu *menu, const char *item ) : label( label ), menu( menu ), item( item )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		label->setText( "picked: %s", item );
		menu->setVisible( false );
	}

private:
	Label *label;
	Menu *menu;
	const char *item;
};

class MenuTestMiniApp : public MiniApp
{
public:
	MenuTestMiniApp()
	{
		setName( "Menu" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 260, 220 );

		frame->setTitle( "Menu" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		Label *picked = new Label( "picked: nothing", 10, 165 );
		client->addChild( picked );

		// Menu never auto-grows: StackLayout's "fit" is width-only, the tall stays as constructed
		// Keep both behaviors
		Menu *sized = BuildMenu( picked, 10, 36, 110, 130 );
		Menu *stock = BuildMenu( picked, 130, 36, 110, 60 );

		Button *toggleSized = new Button( "Sized", 10, 10, 55, 24 );
		toggleSized->addActionSignal( new MenuToggleSignal( sized ));
		client->addChild( toggleSized );

		Button *toggleStock = new Button( "Stock", 130, 10, 55, 24 );
		toggleStock->addActionSignal( new MenuToggleSignal( stock ));
		client->addChild( toggleStock );

		// added last so the open menus paint over their surroundings
		client->addChild( sized );
		client->addChild( stock );

		return frame;
	}

private:
	Menu *BuildMenu( Label *picked, int x, int y, int wide, int tall )
	{
		Menu *menu = new Menu( x, y, wide, tall );
		menu->setVisible( false );

		for( const char *item : { "New", "Open", "Save" } )
		{
			MenuItem *mi = new MenuItem( item );
			mi->addActionSignal( new MenuPickSignal( picked, menu, item ));
			menu->addMenuItem( mi );
		}

		menu->addMenuItem( new MenuSeparator( "" ));

		MenuItem *quit = new MenuItem( "Quit" );
		quit->addActionSignal( new MenuPickSignal( picked, menu, "Quit" ));
		menu->addMenuItem( quit );

		return menu;
	}
};

DesktopIcon *CreateMenuTest()
{
	return new DesktopIcon( new MenuTestMiniApp(), LoadTGA( "icons/news.tga" ));
}
