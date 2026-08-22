// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "controls/button.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#else
#include <VGUI_Button.h>
#include <VGUI_CheckButton.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_RadioButton.h>
#include <VGUI_ToggleButton.h>
#endif

// A systematic visual state matrix: each widget shown enabled and disabled in
// fixed positions, so the harness can additionally drive the enabled column's
// top widget through hover / pressed / focused (see tests/cases/states-*.sh).
// The enabled Button sits at (95,30); its centre ~(125,43) is the target.
class StateMatrixMiniApp : public MiniApp
{
public:
	StateMatrixMiniApp()
	{
		setName( "States" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 300, 220 );

		frame->setTitle( "State Matrix" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		client->addChild( new Label( "enabled", 95, 8 ));
		client->addChild( new Label( "disabled", 190, 8 ));

		addRow( client, "Button", 30, new Button( "Press", 95, 30 ), new Button( "Press", 190, 30 ));
		addRow( client, "Toggle", 62, new ToggleButton( "Toggle", 95, 62 ), new ToggleButton( "Toggle", 190, 62 ));
		addRow( client, "Check", 94, new CheckButton( "Check", 95, 94 ), new CheckButton( "Check", 190, 94 ));
		addRow( client, "Radio", 126, new RadioButton( "Radio", 95, 126 ), new RadioButton( "Radio", 190, 126 ));

		return frame;
	}

private:
	// place the row label plus the enabled/disabled pair; the second is disabled
	void addRow( Panel *client, const char *name, int y, Button *enabled, Button *disabled )
	{
		client->addChild( new Label( name, 10, y + 4 ));

		client->addChild( enabled );

		disabled->setEnabled( false );
		client->addChild( disabled );
	}
};

DesktopIcon *CreateStateMatrixTest()
{
	return new DesktopIcon( new StateMatrixMiniApp(), LoadTGA( "icons/calendar.tga" ));
}
