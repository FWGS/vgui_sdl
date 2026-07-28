// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#include <VGUI_Frame.h>
#include <VGUI_ImagePanel.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_TextImage.h>

class ImagePanelTestMiniApp : public MiniApp
{
public:
	ImagePanelTestMiniApp()
	{
		setName( "ImagePanel" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 220, 140 );

		frame->setTitle( "ImagePanel" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		// a bitmap image and a text image, both hosted by ImagePanel
		ImagePanel *bitmap = new ImagePanel( LoadTGA( "icons/world.tga" ));
		bitmap->setBounds( 10, 10, 32, 32 );
		client->addChild( bitmap );

		TextImage *text = new TextImage( "TextImage in an ImagePanel" );
		text->setFont( Scheme::sf_primary2 );

		int wide, tall;
		text->getTextSize( wide, tall );
		text->setSize( wide, tall );

		ImagePanel *textpanel = new ImagePanel( text );
		textpanel->setBounds( 10, 50, wide, tall );
		client->addChild( textpanel );

		return frame;
	}
};

DesktopIcon *CreateImagePanelTest()
{
	return new DesktopIcon( new ImagePanelTestMiniApp(), LoadTGA( "icons/image_file.tga" ));
}
