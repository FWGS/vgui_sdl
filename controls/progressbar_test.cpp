// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "controls/button.h"
#include "controls/frame.h"
#include "controls/desktop.h"
#include "controls/progressbar.h"
#else
#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_Frame.h>
#include <VGUI_MiniApp.h>
#include <VGUI_ProgressBar.h>
#endif

// progress is not a 0..1 fraction, it's segments >_<
class ProgressStepSignal : public ActionSignal
{
public:
	ProgressStepSignal( ProgressBar *bar, float *progress, float step ) : bar( bar ), progress( progress ), step( step )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		*progress += step;

		if( *progress < 0.0f )
			*progress = 0.0f;
		if( *progress > (float)bar->getSegmentCount())
			*progress = (float)bar->getSegmentCount();

		bar->setProgress( *progress );
	}

private:
	ProgressBar *bar;
	float *progress;
	float step;
};

class ProgressBarTestMiniApp : public MiniApp
{
public:
	ProgressBarTestMiniApp()
	{
		setName( "ProgressBar" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 260, 130 );

		frame->setTitle( "ProgressBar" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		ProgressBar *bar = new ProgressBar( 10 );
		bar->setBounds( 10, 10, 200, 24 );

		float *progress = new float( 3.0f ); // segments, not a fraction
		bar->setProgress( *progress );
		client->addChild( bar );

		Button *minus = new Button( "-", 10, 44, 40, 24 );
		minus->addActionSignal( new ProgressStepSignal( bar, progress, -1.0f ));
		client->addChild( minus );

		Button *plus = new Button( "+", 60, 44, 40, 24 );
		plus->addActionSignal( new ProgressStepSignal( bar, progress, 1.0f ));
		client->addChild( plus );

		return frame;
	}
};

DesktopIcon *CreateProgressBarTest()
{
	return new DesktopIcon( new ProgressBarTestMiniApp(), LoadTGA( "icons/cd_drive.tga" ));
}
