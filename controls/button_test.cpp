// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "controls/button.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "input.h"
#include "layout.h"
#else
#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_ButtonGroup.h>
#include <VGUI_CheckButton.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_MouseCode.h>
#include <VGUI_RadioButton.h>
#include <VGUI_StackLayout.h>
#include <VGUI_ToggleButton.h>
#endif

class ClickCounterSignal : public ActionSignal
{
public:
	ClickCounterSignal( Label *label ) : label( label )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		label->setText( "Clicked %d times", ++count );
	}

private:
	Label *label;
	int count = 0;
};

// drive a target button's enabled state from a checkbox
class EnableSignal : public ActionSignal
{
public:
	EnableSignal( Button *target, Button *check ) : target( target ), check( check )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		target->setEnabled( check->isSelected());
	}

private:
	Button *target;
	Button *check;
};

// drive whether the target button reacts to mouse clicks at all
class MouseClickSignal : public ActionSignal
{
public:
	MouseClickSignal( Button *target, Button *check ) : target( target ), check( check )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		bool on = check->isSelected();

		for( int mc = MOUSE_LEFT; mc < MOUSE_LAST; mc++ )
			target->setMouseClickEnabled( (MouseCode)mc, on );
	}

private:
	Button *target;
	Button *check;
};

class ButtonTestMiniApp : public MiniApp
{
public:
	ButtonTestMiniApp()
	{
		setName( "Buttons" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 240, 460 );

		frame->setTitle( "Buttons" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();
		client->setLayout( new StackLayout( 4, false ));

		Label *counter = new Label( "Clicked 0 times" );
		client->addChild( counter );

		Button *button = new Button( "Click me", 0, 0 );
		button->addActionSignal( new ClickCounterSignal( counter ));
		client->addChild( button );

		ToggleButton *toggle = new ToggleButton( "ToggleButton", 0, 0 );
		client->addChild( toggle );

		// checkboxes that drive the toggle button's own state; both start
		// selected to match the button's default (enabled, clickable)
		CheckButton *enabled = new CheckButton( "setEnabled", 0, 0 );
		enabled->setSelected( true );
		enabled->addActionSignal( new EnableSignal( toggle, enabled ));
		client->addChild( enabled );

		CheckButton *clickable = new CheckButton( "setMouseClickEnabled", 0, 0 );
		clickable->setSelected( true );
		clickable->addActionSignal( new MouseClickSignal( toggle, clickable ));
		client->addChild( clickable );

		client->addChild( new CheckButton( "CheckButton A", 0, 0 ));
		client->addChild( new CheckButton( "CheckButton B", 0, 0 ));

		// radio buttons are mutually exclusive through their ButtonGroup.
		// setButtonGroup registers the button in the group by itself
		ButtonGroup *group = new ButtonGroup;

		for( int i = 0; i < 3; i++ )
		{
			char text[32];

			snprintf( text, sizeof( text ), "RadioButton %d (group 1)", i + 1 );

			RadioButton *radio = new RadioButton( text, 0, 0 );
			radio->setButtonGroup( group );
			client->addChild( radio );
		}

		group = new ButtonGroup;

		for( int i = 0; i < 3; i++ )
		{
			char text[32];

			snprintf( text, sizeof( text ), "RadioButton %d (group 2)", i + 1 );

			RadioButton *radio = new RadioButton( text, 0, 0 );
			radio->setButtonGroup( group );
			client->addChild( radio );
		}

		return frame;
	}
};

DesktopIcon *CreateButtonTest()
{
	return new DesktopIcon( new ButtonTestMiniApp(), LoadTGA( "icons/program.tga" ));
}
