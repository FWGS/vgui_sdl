// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "controls/edit.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "layout.h"
#include "controls/text.h"
#else
#include <VGUI_EditPanel.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_StackLayout.h>
#include <VGUI_TextEntry.h>
#include <VGUI_TextGrid.h>
#include <VGUI_TextPanel.h>
#endif

// one frame for the whole char-based text family:
// * TextEntry (single line)
// * EditPanel (multi line)
// * TextPanel (wrapping static text)
// * TextGrid (character cell grid a la text mode)
//
// label already has it's own test :)
class TextEntryTestMiniApp : public MiniApp
{
public:
	TextEntryTestMiniApp()
	{
		setName( "Text" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 260, 420 );

		frame->setTitle( "Text" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();
		client->setLayout( new StackLayout( 4, false ));

		client->addChild( new Label( "TextEntry:" ));
		client->addChild( new TextEntry( "", 0, 0, 200, 24 ));

		client->addChild( new Label( "TextEntry, hidden:" ));

		TextEntry *hidden = new TextEntry( "", 0, 0, 200, 24 );
		hidden->setTextHidden( true );
		client->addChild( hidden );

		client->addChild( new Label( "EditPanel:" ));
		client->addChild( new EditPanel( 0, 0, 220, 64 ));

		client->addChild( new Label( "TextPanel:" ));
		client->addChild( new TextPanel(
			"The quick brown fox jumps over the lazy dog while "
			"TextImage wraps this text greedily.", 0, 0, 220, 48 ));

		client->addChild( new Label( "TextGrid:" ));

		TextGrid *grid = new TextGrid( 26, 2, 0, 0, 220, 40 );
		grid->printf( "TextGrid line one" );
		grid->newLine();
		grid->printf( "and line two" );
		client->addChild( grid );

		return frame;
	}
};

DesktopIcon *CreateTextEntryTest()
{
	return new DesktopIcon( new TextEntryTestMiniApp(), LoadTGA( "icons/text_editor.tga" ));
}
