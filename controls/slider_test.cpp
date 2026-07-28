// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#include <VGUI_Frame.h>
#include <VGUI_IntChangeSignal.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_Slider.h>

class IntValueLabelSignal : public IntChangeSignal
{
public:
	IntValueLabelSignal( Label *label ) : label( label )
	{
	}

	void intChanged( int value, Panel *panel ) override
	{
		label->setText( "%d", value );
	}

private:
	Label *label;
};

class SliderTestMiniApp : public MiniApp
{
public:
	SliderTestMiniApp()
	{
		setName( "Slider" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 260, 180 );

		frame->setTitle( "Slider" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		// stock Slider computes its thumb as trackLen/rangeWindow*trackLen,
		// without a (pixel-unit) range window the thumb breaks, doesn't update vertically, negative horizontally
		Slider *horizontal = new Slider( 10, 10, 180, 24, false );
		horizontal->setRange( 0, 100 );
		horizontal->setRangeWindow( 1350 ); // thumb = 180*180/1350 = 24px
		horizontal->setRangeWindowEnabled( true );
		client->addChild( horizontal );

		Label *value = new Label( "0", 200, 10, 40, 24 );
		horizontal->addIntChangeSignal( new IntValueLabelSignal( value ));
		client->addChild( value );

		client->addChild( new Label( "vertical:", 10, 44 ));

		Slider *vertical = new Slider( 10, 64, 24, 70, true );
		vertical->setRange( 0, 10 );
		vertical->setRangeWindow( 245 ); // thumb = 70*70/245 = 20px
		vertical->setRangeWindowEnabled( true );
		client->addChild( vertical );

		Label *vvalue = new Label( "0", 44, 64, 40, 24 );
		vertical->addIntChangeSignal( new IntValueLabelSignal( vvalue ));
		client->addChild( vvalue );

		return frame;
	}
};

DesktopIcon *CreateSliderTest()
{
	return new DesktopIcon( new SliderTestMiniApp(), LoadTGA( "icons/sounds.tga" ));
}
