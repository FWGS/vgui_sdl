// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "controls/button.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "layout.h"
#else
#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_ButtonGroup.h>
#include <VGUI_CheckButton.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
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

class ButtonTestMiniApp : public MiniApp
{
public:
	ButtonTestMiniApp()
	{
		setName( "Buttons" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 240, 400 );

		frame->setTitle( "Buttons" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();
		client->setLayout( new StackLayout( 4, false ));

		Label *counter = new Label( "Clicked 0 times" );
		client->addChild( counter );

		Button *button = new Button( "Click me", 0, 0 );
		button->addActionSignal( new ClickCounterSignal( counter ));
		client->addChild( button );

		client->addChild( new ToggleButton( "ToggleButton", 0, 0 ));

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
