// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#include <VGUI_Frame.h>
#include <VGUI_InputSignal.h>
#include <VGUI_Label.h>
#include <VGUI_ListPanel.h>
#include <VGUI_MiniApp.h>
#include <VGUI_ScrollBar.h>

// we need to get scrollbar pointer to fix the mouse wheel (and essentially be able to test it)
class ListPanelEx : public ListPanel
{
public:
	ListPanelEx( int x, int y, int wide, int tall ) : ListPanel( x, y, wide, tall )
	{
	}

	ScrollBar *getScrollBar()
	{
		return _scroll;
	}
};

// bundled widgets ignore mouse wheel in vgui.so. Here we override input signal to catch mouse wheel events
// and drive scrollbar directly
class ListWheelSignal : public InputSignal
{
public:
	ListWheelSignal( ScrollBar *scroll ) : scroll( scroll )
	{
	}

	void mouseWheeled( int delta, Panel *panel ) override
	{
		scroll->setValue( scroll->getValue() - delta * 20 );
	}

	void cursorMoved( int x, int y, Panel *panel ) override {}
	void cursorEntered( Panel *panel ) override {}
	void cursorExited( Panel *panel ) override {}
	void mousePressed( MouseCode code, Panel *panel ) override {}
	void mouseDoublePressed( MouseCode code, Panel *panel ) override {}
	void mouseReleased( MouseCode code, Panel *panel ) override {}
	void keyPressed( KeyCode code, Panel *panel ) override {}
	void keyTyped( KeyCode code, Panel *panel ) override {}
	void keyReleased( KeyCode code, Panel *panel ) override {}
	void keyFocusTicked( Panel *panel ) override {}

private:
	ScrollBar *scroll;
};

class ListPanelTestMiniApp : public MiniApp
{
public:
	ListPanelTestMiniApp()
	{
		setName( "ListPanel" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 260, 220 );

		frame->setTitle( "ListPanel" );
		frame->addFrameSignal( new TestFrameSignal );

		ListPanelEx *list = new ListPanelEx( 10, 10, 230, 150 );
		frame->getClient()->addChild( list );

		// add our mouse wheel to the frame. here is a catch, mouse wheel is driven like a keyboard event
		frame->addInputSignal( new ListWheelSignal( list->getScrollBar()));

		for( int i = 0; i < 15; i++ )
		{
			char text[64];

			snprintf( text, sizeof( text ), "List item %d of 30 (addString)", i + 1 );
			list->addString( text );
		}

		// addString's internal labels are hardcoded to 80x20
		for( int i = 0; i < 15; i++ )
		{
			char text[64];

			snprintf( text, sizeof( text ), "List item %d of 30 (Label addItem)", i + 1 );
			list->addItem( new Label( text, 0, 0, 200, 20 ));
		}

		return frame;
	}
};

DesktopIcon *CreateListPanelTest()
{
	return new DesktopIcon( new ListPanelTestMiniApp(), LoadTGA( "icons/contact_book.tga" ));
}
