// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#include <VGUI_BorderPair.h>
#include <VGUI_EtchedBorder.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_LineBorder.h>
#include <VGUI_LoweredBorder.h>
#include <VGUI_MiniApp.h>
#include <VGUI_RaisedBorder.h>

class BorderTestMiniApp : public MiniApp
{
public:
	BorderTestMiniApp()
	{
		setName( "Borders" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 340, 270 );

		frame->setTitle( "Border zoo" );
		frame->addFrameSignal( new TestFrameSignal );

		AddSpecimen( frame, 0, "LineBorder", new LineBorder());
		AddSpecimen( frame, 1, "LineBorder(4)", new LineBorder( 4 ));
		AddSpecimen( frame, 2, "LineBorder(red)", new LineBorder( Color( 255, 0, 0, 0 )));
		AddSpecimen( frame, 3, "EtchedBorder", new EtchedBorder());
		AddSpecimen( frame, 4, "LoweredBorder", new LoweredBorder());
		AddSpecimen( frame, 5, "RaisedBorder", new RaisedBorder());
		// stock borderpair doesn't set inset, it's the app job to do it
		// excercise both behaviors
		AddSpecimen( frame, 6, "Pair (stock)", new BorderPair( new RaisedBorder(), new LineBorder()));

		BorderPair *pair = new BorderPair( new RaisedBorder(), new LineBorder());
		pair->setInset( 2, 2, 2, 2 );
		AddSpecimen( frame, 7, "Pair setInset(2)", pair );

		return frame;
	}

private:
	void AddSpecimen( Frame *frame, int slot, const char *name, Border *border )
	{
		Label *label = new Label( name, 10 + ( slot % 2 ) * 160, 10 + ( slot / 2 ) * 50, 150, 40 );

		label->setBorder( border );
		frame->getClient()->addChild( label );
	}
};

DesktopIcon *CreateBorderTest()
{
	return new DesktopIcon( new BorderTestMiniApp(), LoadTGA( "icons/paint.tga" ));
}
