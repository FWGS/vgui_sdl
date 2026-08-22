// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "buildgroup.h"
#include "controls/button.h"
#include "controls/edit.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "controls/text.h"
#else
#include <VGUI_ActionSignal.h>
#include <VGUI_BuildGroup.h>
#include <VGUI_Button.h>
#include <VGUI_CheckButton.h>
#include <VGUI_EditPanel.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_TextPanel.h>
#endif

class PasteSignal : public ActionSignal
{
public:
	PasteSignal( EditPanel *edit ) : edit( edit )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		App *app = edit->getApp();
		int count = app->getClipboardTextCount();

		if( count <= 0 )
			return;

		char *buf = new char[count + 1];
		int len = app->getClipboardText( 0, buf, count + 1 );

		for( int i = 0; i < len; i++ )
		{
			if( buf[i] == '\n' )
				edit->doCursorNewLine();
			else if( buf[i] != '\r' )
				edit->doCursorInsertChar( buf[i] );
		}

		delete[] buf;
	}

private:
	EditPanel *edit;
};

class BuildGroupTestMiniApp : public MiniApp
{
public:
	BuildGroupTestMiniApp()
	{
		setName( "BuildGroup" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 320, 390 );

		frame->setTitle( "BuildGroup" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		TextPanel *help = new TextPanel(
			"Ctrl+B toggles build mode: drag widgets, arrows nudge,"
			"Shift+arrows resize, right-drag creates Labels, Ctrl+C generates code", 10, 10, 290, 65 );

		help->setFgColor( 0, 0, 0, 0 );
		client->addChild( help );

		Button *button = new Button( "Drag me", 20, 85, 80, 24 );
		Label *label = new Label( "Me too", 150, 85 );
		CheckButton *check = new CheckButton( "And me", 20, 125 );

		client->addChild( button );
		client->addChild( label );
		client->addChild( check );

		// panels have no build group by default. register the editable
		// widgets into one and hand it to the Ctrl+B toggle
		BuildGroup *bg = new BuildGroup;

		button->setBuildGroup( bg, "button" );
		label->setBuildGroup( bg, "label" );
		check->setBuildGroup( bg, "check" );

		// the group only engages for registered panels, so register the
		// frame client too - right-drag then creates Labels anywhere on
		// the frame (note left-dragging empty space now drags the client
		// panel itself, stock semantics)
		client->setBuildGroup( bg, "client" );

		BuildMode_Register( bg );

		EditPanel *edit = new EditPanel( 10, 195, 290, 120 );
		client->addChild( edit );

		Button *paste = new Button( "Paste", 10, 325, 60, 24 );
		paste->addActionSignal( new PasteSignal( edit ));
		client->addChild( paste );

		return frame;
	}
};

DesktopIcon *CreateBuildGroupTest()
{
	return new DesktopIcon( new BuildGroupTestMiniApp(), LoadTGA( "icons/image_editor.tga" ));
}
