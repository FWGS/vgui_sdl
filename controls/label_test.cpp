#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "layout.h"
#include "controls/desktop.h"
#include "controls/frame.h"
#include "controls/label.h"
#else
#include <VGUI_BorderLayout.h>
#include <VGUI_DesktopIcon.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#endif

class LabelTestMiniApp : public MiniApp
{
public:
	LabelTestMiniApp()
	{
		setName( "LabelTest" );
	}

	// a factory: every desktop icon activation spawns a fresh frame
	Frame *createInstance()
	{
		Frame *frame = new Frame( 200, 200, 200, 100 );

		frame->setTitle( "LabelTest" );
		frame->addFrameSignal( new TestFrameSignal );

		BorderLayout *layout = new BorderLayout( 0 );
		frame->getClient()->setLayout( layout );

		Label *label = new Label( "Hello World!" );
#if USE_FREEVGUI_HEADERS
		label->setLayoutInfo( layout->createLayoutInfo( BorderLayout::CENTER ));
#else
		label->setLayoutInfo( layout->createLayoutInfo( BorderLayout::a_center ));
#endif

		frame->getClient()->addChild( label );

		return frame;
	}
};

DesktopIcon *CreateLabelTest()
{
	DesktopIcon *di = new DesktopIcon( new LabelTestMiniApp(), LoadTGA( "icons/notepad.tga" ));

	return di;
}

