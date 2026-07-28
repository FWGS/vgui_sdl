#include "vgui_sdl.h"
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

		label = new Label( "Hello World!" );

		frame->addChild( label );
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
	DesktopIcon *di = new DesktopIcon(
	    new LabelTestMiniApp(),
	    LoadTGA( "icons/notepad.tga" ));

	return di;
}

