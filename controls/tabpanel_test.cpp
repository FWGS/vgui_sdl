// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "layout.h"
#include "controls/button.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "controls/tab.h"
#else
#include <VGUI_BorderLayout.h>
#include <VGUI_Button.h>
#include <VGUI_FlowLayout.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_StackLayout.h>
#include <VGUI_TabPanel.h>
#endif

class TabPanelTestMiniApp : public MiniApp
{
public:
	TabPanelTestMiniApp()
	{
		setName( "TabPanel" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 260, 180 );

		frame->setTitle( "TabPanel" );
		frame->addFrameSignal( new TestFrameSignal );

		BorderLayout *layout = new BorderLayout( 2 );
		frame->getClient()->setLayout( layout );

		TabPanel *tabs = new TabPanel( 0, 0, 64, 64 );
#if USE_FREEVGUI_HEADERS
		tabs->setLayoutInfo( layout->createLayoutInfo( BorderLayout::CENTER ));
#else
		tabs->setLayoutInfo( layout->createLayoutInfo( BorderLayout::a_center ));
#endif
		frame->getClient()->addChild( tabs );

		// GridLayout is nonexistent, so we don't test it here

		Panel *labels = tabs->addTab( "Labels" );
		labels->setLayout( new StackLayout( 4, false ));
		labels->addChild( new Label( "stacked one" ));
		labels->addChild( new Label( "stacked two" ));
		labels->addChild( new Label( "stacked three" ));

		Panel *buttons = tabs->addTab( "Buttons" );
		buttons->setLayout( new FlowLayout( 4 ));
		buttons->addChild( new Button( "One", 0, 0 ));
		buttons->addChild( new Button( "Two", 0, 0 ));
		buttons->addChild( new Button( "Three", 0, 0 ));

		return frame;
	}
};

DesktopIcon *CreateTabPanelTest()
{
	return new DesktopIcon( new TabPanelTestMiniApp(), LoadTGA( "icons/workspace.tga" ));
}
