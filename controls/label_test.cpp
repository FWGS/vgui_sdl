#include "vgui_sdl.h"
#include <VGUI_BorderLayout.h>
#include <VGUI_DesktopIcon.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>

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
		label->setLayoutInfo( layout->createLayoutInfo( BorderLayout::a_center ));

		frame->getClient()->addChild( label );

		return frame;
	}
};

DesktopIcon *CreateLabelTest()
{
	DesktopIcon *di = new DesktopIcon( new LabelTestMiniApp(), LoadTGA( "icons/notepad.tga" ));

	return di;
}

