// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#include <stdlib.h>
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "controls/button.h"
#include "layout.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/menu.h"
#include "controls/messagebox.h"
#include "controls/desktop.h"
#include "controls/text.h"
#else
#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_CheckButton.h>
#include <VGUI_FlowLayout.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_Menu.h>
#include <VGUI_MenuItem.h>
#include <VGUI_MenuSeparator.h>
#include <VGUI_MessageBox.h>
#include <VGUI_MiniApp.h>
#include <VGUI_StackLayout.h>
#include <VGUI_TextEntry.h>
#endif

// The stock Frame wires only its minimize and close caption buttons (they fire
// the frame's signals). Its menu, tray and maximize buttons are decorative, so
// this test attaches handlers to them to exercise the whole window-control
// surface, and drives the frame's public property setters from checkboxes and
// text fields.

// each checkbox drives one stock Frame bool setter through a pointer-to-member
typedef void ( Frame::*FrameSetter )( bool );

class PropCheckSignal : public ActionSignal
{
public:
	PropCheckSignal( Frame *frame, Button *check, FrameSetter setter ) :
		frame( frame ), check( check ), setter( setter )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		( frame->*setter )( check->isSelected());
	}

private:
	Frame *frame;
	Button *check;
	FrameSetter setter;
};

// maximize fills the parent (the desktop foreground); a second press restores the
// saved bounds. The window-controls menu shares one instance so its Maximize and
// Restore items stay in sync with the caption button
class MaximizeSignal : public ActionSignal
{
public:
	MaximizeSignal( Frame *frame ) : frame( frame )
	{
	}

	void maximize()
	{
		Panel *parent = frame->getParent();

		if( !parent || maximized )
			return;

		frame->getBounds( restore[0], restore[1], restore[2], restore[3] );

		int wide, tall;
		parent->getSize( wide, tall );
		frame->setBounds( 0, 0, wide, tall );
		maximized = true;
	}

	void restoreBounds()
	{
		if( !maximized )
			return;

		frame->setBounds( restore[0], restore[1], restore[2], restore[3] );
		maximized = false;
	}

	void actionPerformed( Panel *panel ) override
	{
		if( maximized )
			restoreBounds();
		else maximize();
	}

private:
	Frame *frame;
	bool maximized = false;
	int restore[4] = {};
};

// menu button toggles the drop-down (we don't implement showModal popups)
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

class CloseBoxSignal : public ActionSignal
{
public:
	CloseBoxSignal( Frame *box ) : box( box )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		DestroyFrame( box );
	}

private:
	Frame *box;
};

// there is no taskbar tray yet, so just prove the tray button fires
class TrayMessageSignal : public ActionSignal
{
public:
	TrayMessageSignal( Frame *frame ) : frame( frame )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		MessageBox *box = new MessageBox( "Tray", "Tray button pressed!", 0, 0 );

		box->addFrameSignal( new TestFrameSignal );
		box->addActionSignal( new CloseBoxSignal( box ));

		Panel *parent = frame->getParent();

		if( parent )
		{
			int x, y;

			frame->getPos( x, y );
			box->setParent( parent );
			box->setPos( x + 40, y + 40 );
		}
	}

private:
	Frame *frame;
};

enum MenuAct { ACT_MINIMIZE, ACT_MAXIMIZE, ACT_RESTORE, ACT_CLOSE };

class MenuActionSignal : public ActionSignal
{
public:
	MenuActionSignal( Frame *frame, Menu *menu, MaximizeSignal *maxer, MenuAct act ) :
		frame( frame ), menu( menu ), maxer( maxer ), act( act )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		switch( act )
		{
		case ACT_MINIMIZE: frame->fireMinimizingSignal(); break;
		case ACT_MAXIMIZE: maxer->maximize(); break;
		case ACT_RESTORE:  maxer->restoreBounds(); break;
		case ACT_CLOSE:    frame->fireClosingSignal(); break;
		}

		menu->setVisible( false );
	}

private:
	Frame *frame;
	Menu *menu;
	MaximizeSignal *maxer;
	MenuAct act;
};

class SetTitleSignal : public ActionSignal
{
public:
	SetTitleSignal( Frame *frame, TextEntry *entry ) : frame( frame ), entry( entry )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		char buf[128];

		entry->getText( 0, buf, sizeof( buf ));
		frame->setTitle( buf );
	}

private:
	Frame *frame;
	TextEntry *entry;
};

class SetMinSizeSignal : public ActionSignal
{
public:
	SetMinSizeSignal( Frame *frame, TextEntry *wide, TextEntry *tall ) :
		frame( frame ), wideEntry( wide ), tallEntry( tall )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		char buf[32];

		wideEntry->getText( 0, buf, sizeof( buf ));
		int wide = atoi( buf );

		tallEntry->getText( 0, buf, sizeof( buf ));
		int tall = atoi( buf );

		frame->setMinimumSize( wide, tall );

		// nudge the frame so the new minimum is applied immediately
		int x, y, w, h;
		frame->getBounds( x, y, w, h );
		frame->setSize( w, h );
	}

private:
	Frame *frame;
	TextEntry *wideEntry;
	TextEntry *tallEntry;
};

// subclass Frame only to reach the protected caption buttons; every behavior
// lives in the signals above and drives the stock public Frame API
class ControlsFrame : public Frame
{
public:
	ControlsFrame() : Frame( 0, 0, 300, 470 )
	{
		setTitle( "Window Controls" );
		addFrameSignal( new TestFrameSignal );

		MaximizeSignal *maxer = new MaximizeSignal( this );
		Menu *menu = BuildControlsMenu( maxer );

		// wire the normally-decorative caption buttons
		// (the minimize and close buttons already fire the frame's signals)
#if USE_FREEVGUI_HEADERS
		menuButton->addActionSignal( new MenuToggleSignal( menu ));
		trayButton->addActionSignal( new TrayMessageSignal( this ));
		maximizeButton->addActionSignal( maxer );
#else
		_menuButton->addActionSignal( new MenuToggleSignal( menu ));
		_trayButton->addActionSignal( new TrayMessageSignal( this ));
		_maximizeButton->addActionSignal( maxer );
#endif

		BuildClient();

		// added last so it paints over the client; hidden until the menu opens it
		addChild( menu );
	}

private:
	Menu *BuildControlsMenu( MaximizeSignal *maxer )
	{
		struct { const char *text; MenuAct act; } items[] =
		{
			{ "Minimize", ACT_MINIMIZE },
			{ "Maximize", ACT_MAXIMIZE },
			{ "Restore",  ACT_RESTORE },
		};

		// the stock Menu never grows to fit its items, so size it up front:
		// the three items above plus a separator and the Close item
		const int itemTall = 26;
		int rows = ( sizeof( items ) / sizeof( items[0] )) + 2;

		Menu *menu = new Menu( 7, 28, 120, rows * itemTall + 8 );
		menu->setVisible( false );

		for( auto &it : items )
		{
			MenuItem *mi = new MenuItem( it.text );
			mi->addActionSignal( new MenuActionSignal( this, menu, maxer, it.act ));
			menu->addMenuItem( mi );
		}

		menu->addMenuItem( new MenuSeparator( "" ));

		MenuItem *close = new MenuItem( "Close" );
		close->addActionSignal( new MenuActionSignal( this, menu, maxer, ACT_CLOSE ));
		menu->addMenuItem( close );

		return menu;
	}

	void BuildClient()
	{
		Panel *client = getClient();
		client->setLayout( new StackLayout( 4, false ));

		// title editor
		client->addChild( new Label( "Title:" ));

		Panel *titleRow = new Panel( 0, 0, 280, 28 );
		titleRow->setLayout( new FlowLayout( 4 ));

		TextEntry *title = new TextEntry( "Window Controls", 0, 0, 190, 24 );
		titleRow->addChild( title );

		Button *setTitle = new Button( "Set", 0, 0 );
		setTitle->addActionSignal( new SetTitleSignal( this, title ));
		titleRow->addChild( setTitle );

		client->addChild( titleRow );

		// minimum size editor (stock default is 64x33)
		client->addChild( new Label( "Minimum size (w, h):" ));

		Panel *minRow = new Panel( 0, 0, 280, 28 );
		minRow->setLayout( new FlowLayout( 4 ));

		TextEntry *minW = new TextEntry( "64", 0, 0, 60, 24 );
		TextEntry *minH = new TextEntry( "33", 0, 0, 60, 24 );
		minRow->addChild( minW );
		minRow->addChild( minH );

		Button *setMin = new Button( "Set", 0, 0 );
		setMin->addActionSignal( new SetMinSizeSignal( this, minW, minH ));
		minRow->addChild( setMin );

		client->addChild( minRow );

		// properties. note the stock quirk documented in Frame.md: the caption
		// move grip also gates on isSizeable(), and isMoveable() is never
		// consulted by any drag handler, so the Moveable toggle has no effect
		client->addChild( new Label( "Properties:" ));
		AddCheck( client, "Sizeable", &Frame::setSizeable );
		AddCheck( client, "Moveable", &Frame::setMoveable );
		// internal (default) = grips resize this frame; off = grips resize the
		// frame's parent (the desktop) instead
		AddCheck( client, "Internal (off: drag resizes parent)", &Frame::setInternal );

		client->addChild( new Label( "Caption buttons visible:" ));
		AddCheck( client, "Menu", &Frame::setMenuButtonVisible );
		AddCheck( client, "Tray", &Frame::setTrayButtonVisible );
		AddCheck( client, "Minimize", &Frame::setMinimizeButtonVisible );
		AddCheck( client, "Maximize", &Frame::setMaximizeButtonVisible );
		AddCheck( client, "Close", &Frame::setCloseButtonVisible );
	}

	void AddCheck( Panel *client, const char *text, FrameSetter setter )
	{
		CheckButton *cb = new CheckButton( text, 0, 0 );

		// reflect the frame's real starting state (all true), set before the
		// signal is attached so it does not fire during setup
		cb->setSelected( true );
		cb->addActionSignal( new PropCheckSignal( this, cb, setter ));
		client->addChild( cb );
	}
};

class FrameTestMiniApp : public MiniApp
{
public:
	FrameTestMiniApp()
	{
		setName( "Frame" );
	}

	Frame *createInstance()
	{
		return new ControlsFrame;
	}
};

DesktopIcon *CreateFrameTest()
{
	return new DesktopIcon( new FrameTestMiniApp(), LoadTGA( "icons/calculator.tga" ));
}
