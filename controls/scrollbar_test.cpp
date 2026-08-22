// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "controls/frame.h"
#include "signals.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "controls/scroll.h"
#else
#include <VGUI_Frame.h>
#include <VGUI_IntChangeSignal.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_ScrollBar.h>
#endif

// report a standalone ScrollBar's value into a Label as it changes
class ScrollReport : public IntChangeSignal
{
public:
	ScrollReport( Label *label, const char *name ) : label( label ), name( name )
	{
	}

	void intChanged( int value, Panel *panel ) override
	{
		label->setText( "%s: %d", name, value );
	}

private:
	Label *label;
	const char *name;
};

// ScrollBar is normally embedded in ScrollPanel; here it stands alone. It owns
// two arrow Buttons and a Slider. setRangeWindow makes the thumb proportional.
class ScrollBarTestMiniApp : public MiniApp
{
public:
	ScrollBarTestMiniApp()
	{
		setName( "ScrollBar" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 280, 220 );

		frame->setTitle( "ScrollBar" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		Label *hval = new Label( "horizontal: 0", 10, 10 );
		client->addChild( hval );

		// STOCK BUGS on show here (see docs/spec/Slider.md, binary-verified):
		//  - the horizontal Slider paintBackground is Valve's "doesn't work" path,
		//    so the horizontal bar renders wrong regardless of setup;
		//  - the nob size is trackLen/rangeWindow*trackLen, so a small rangeWindow
		//    oversizes the thumb until it clamps to the whole track (looks like a
		//    fill that grows with value) -- a proper knob needs a huge rangeWindow,
		//    which is only what ScrollPanel supplies (see scrollpanel_test).
		// Value tracking still works: the arrow buttons move it 0..100. FreeVGUI
		// reproduces the same behaviour, which is the point -- document the bugs.
		ScrollBar *h = new ScrollBar( 10, 34, 220, 18, false );
		h->setRange( 0, 100 );
		h->addIntChangeSignal( new ScrollReport( hval, "horizontal" ));
		client->addChild( h );

		Label *vval = new Label( "vertical: 0", 10, 66 );
		client->addChild( vval );

		ScrollBar *v = new ScrollBar( 10, 90, 18, 100, true );
		v->setRange( 0, 100 );
		v->addIntChangeSignal( new ScrollReport( vval, "vertical" ));
		client->addChild( v );

		return frame;
	}
};

DesktopIcon *CreateScrollBarTest()
{
	return new DesktopIcon( new ScrollBarTestMiniApp(), LoadTGA( "icons/search.tga" ));
}
