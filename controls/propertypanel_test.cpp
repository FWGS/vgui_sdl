// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "controls/button.h"
#include "controls/frame.h"
#include "controls/desktop.h"
#else
#include <VGUI_Button.h>
#include <VGUI_Frame.h>
#include <VGUI_MiniApp.h>
#endif

// Panel::createPropertyPanel() is VGUI's build-mode "property inspector" factory:
// it returns a TreeFolder("Properties") holding one sub-folder per class in the
// widget's hierarchy, each listing that class's editable properties. Label and
// Button override it to append their own sub-folder, chaining up to Panel -- so a
// Button yields Properties > { Panel, Label, Button }. Build mode pops this tree
// when a widget is selected; here we call it directly on a Button and drop the
// returned tree into a frame so the A/B harness can compare FreeVGUI's
// reimplementation against the proprietary output (folder names, per-property
// leaves, and the setText row's inline TextEntry editor).
//
// Proprietary layout (verified against vgui.so, see docs/spec/PropertyPanel.md):
//   Properties
//     Panel   { setPos, setSize, setBorder, setLayout }
//     Label   { setText (Label + TextEntry editor row), setContentAlignment }
//     Button  { setSelected, setArmed }
class PropertyPanelTestMiniApp : public MiniApp
{
public:
	PropertyPanelTestMiniApp()
	{
		setName( "Property Panel" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 280, 380 );

		frame->setTitle( "Property Panel" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		// the widget under inspection
		Button *button = new Button( "Sample", 10, 10, 90, 24 );
		client->addChild( button );

		// its build-mode property tree; returned as a Panel* (a TreeFolder),
		// starts collapsed -- click the "Properties" label to expand it
		Panel *props = button->createPropertyPanel();
		props->setPos( 10, 44 );
		client->addChild( props );

		return frame;
	}
};

DesktopIcon *CreatePropertyPanelTest()
{
	return new DesktopIcon( new PropertyPanelTestMiniApp(), LoadTGA( "icons/briefcase.tga" ));
}
