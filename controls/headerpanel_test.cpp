// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "controls/header.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#else
#include <VGUI_HeaderPanel.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#endif

// HeaderPanel is the draggable column-header strip a ListPanel sits under. Each
// addSectionPanel() adds a column; the dividers between them can be dragged.
class HeaderPanelTestMiniApp : public MiniApp
{
public:
	HeaderPanelTestMiniApp()
	{
		setName( "HeaderPanel" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 340, 140 );

		frame->setTitle( "HeaderPanel" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		// no layout: the header spans the top, a note sits below it
		HeaderPanel *header = new HeaderPanel( 0, 0, 330, 26 );
		header->addSectionPanel( new Label( "Name" ));
		header->addSectionPanel( new Label( "Size" ));
		header->addSectionPanel( new Label( "Type" ));
		header->addSectionPanel( new Label( "Modified" ));

		// dividers default to 0, collapsing every section but the first; place
		// them so all four are visible (this is what ListPanel does internally)
		header->setSliderPos( 0, 130 );
		header->setSliderPos( 1, 190 );
		header->setSliderPos( 2, 250 );

		client->addChild( header );

		client->addChild( new Label( "drag the dividers between sections", 10, 40 ));

		return frame;
	}
};

DesktopIcon *CreateHeaderPanelTest()
{
	return new DesktopIcon( new HeaderPanelTestMiniApp(), LoadTGA( "icons/spreadsheet_file.tga" ));
}
