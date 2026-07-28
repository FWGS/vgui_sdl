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

		frame = new Frame( 200, 200, 200, 100 );
		frame->setTitle( "LabelTest" );

		BorderLayout *layout = new BorderLayout( 0 );
		frame->getClient()->setLayout( layout );

		label = new Label( "Hello World!" );
		label->setLayoutInfo( layout->createLayoutInfo( BorderLayout::a_center ));

		frame->getClient()->addChild( label );
	}

	Frame *createInstance()
	{
		return frame;
	}

private:

	Frame *frame;
	Label *label;
};

DesktopIcon *CreateLabelTest()
{
	DesktopIcon *di = new DesktopIcon( new LabelTestMiniApp(), LoadTGA( "icons/notepad.tga" ));

	return di;
}

