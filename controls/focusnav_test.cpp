// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#include <VGUI_FocusChangeSignal.h>
#include <VGUI_FocusNavGroup.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_TextEntry.h>

class FieldFocusSignal : public FocusChangeSignal
{
public:
	FieldFocusSignal( Label *status, const char *name ) : status( status ), name( name )
	{
	}

	void focusChanged( bool lost, Panel *panel ) override
	{
		if( !lost )
			status->setText( "focus: %s", name );
	}

private:
	Label *status;
	const char *name;
};

class FocusNavTestMiniApp : public MiniApp
{
public:
	FocusNavTestMiniApp()
	{
		setName( "FocusNav" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 260, 270 );

		frame->setTitle( "FocusNav" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		client->addChild( new Label( "Tab / Up / Down switch fields", 10, 10 ) );

		Label *status = new Label( "focus: none", 10, 210 );
		client->addChild( status );

		// Panel::internalKeyTyped has stock auto-focus-nav (default on) that maps Tab/Shift+Tab and Down/Up onto
		// panel's FocusNavGroup requestFocusNext/Prev.
		// All we do is put the fields in one group via setFocusNavGroup (which calls FocusNavGroup::addPanel).
		FocusNavGroup *nav = new FocusNavGroup;

		const char *names[] = { "first", "second", "third", "fourth", "fifth" };
		TextEntry *first = nullptr;

		for( int i = 0; i < 5; i++ )
		{
			TextEntry *field = new TextEntry( names[i], 10, 40 + i * 30, 150, 24 );

			field->setFocusNavGroup( nav );
			field->addFocusChangeSignal( new FieldFocusSignal( status, names[i] ) );
			client->addChild( field );

			if( !first )
				first = field;
		}

		first->requestFocus();

		return frame;
	}
};

DesktopIcon *CreateFocusNavTest()
{
	return new DesktopIcon( new FocusNavTestMiniApp(), LoadTGA( "icons/workspace.tga" ) );
}
