// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "layout.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "controls/scroll.h"
#else
#include <VGUI_BorderLayout.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_ScrollPanel.h>
#endif

class ScrollPanelTestMiniApp : public MiniApp
{
public:
	ScrollPanelTestMiniApp()
	{
		setName( "ScrollPanel" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 220, 160 );

		frame->setTitle( "ScrollPanel" );
		frame->addFrameSignal( new TestFrameSignal );

		BorderLayout *layout = new BorderLayout( 2 );
		frame->getClient()->setLayout( layout );

		ScrollPanel *scroll = new ScrollPanel( 0, 0, 64, 64 );
#if USE_FREEVGUI_HEADERS
		scroll->setLayoutInfo( layout->createLayoutInfo( BorderLayout::CENTER ));
#else
		scroll->setLayoutInfo( layout->createLayoutInfo( BorderLayout::a_center ));
#endif
		frame->getClient()->addChild( scroll );

		// more content than the clip area can show, so the vertical
		// scrollbar has something to do
		for( int i = 0; i < 20; i++ )
		{
			char text[256];

			snprintf( text, sizeof( text ), "Item %d of 20", i + 1 );
			scroll->getClient()->addChild( new Label( text, 0, i * 20 ));
		}

		scroll->setScrollBarAutoVisible( true, true );
		scroll->validate();

		return frame;
	}
};

DesktopIcon *CreateScrollPanelTest()
{
	return new DesktopIcon( new ScrollPanelTestMiniApp(), LoadTGA( "icons/folder_open.tga" ));
}
