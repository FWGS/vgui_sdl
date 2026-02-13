#include "vgui_sdl.h"
#include <VGUI_DesktopIcon.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_TextImage.h>

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
	TextImage *ti = new TextImage("Label");
	ti->setFont( vgui::Scheme::sf_primary1 );
	ti->setSize( 32, 32 );

	DesktopIcon *di = new DesktopIcon(
	    new LabelTestMiniApp(),
	    ti );

	return di;
}

