// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "controls/button.h"
#include "controls/configwizard.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "controls/treefolder.h"
#else
#include <VGUI_ActionSignal.h>
#include <VGUI_Button.h>
#include <VGUI_ConfigWizard.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_TreeFolder.h>
#endif

// the Ok/Cancel/Apply/Help buttons are protected with no getters, expose them the usual way like in ListPanel
class ConfigWizardEx : public ConfigWizard
{
public:
	ConfigWizardEx( int x, int y, int wide, int tall ) : ConfigWizard( x, y, wide, tall )
	{
	}

	Button *getOkButton()
	{
#if USE_FREEVGUI_HEADERS
		return okButton;
#else
		return _okButton;
#endif
	}

	Button *getCancelButton()
	{
#if USE_FREEVGUI_HEADERS
		return cancelButton;
#else
		return _cancelButton;
#endif
	}

	Button *getApplyButton()
	{
#if USE_FREEVGUI_HEADERS
		return applyButton;
#else
		return _applyButton;
#endif
	}

	Button *getHelpButton()
	{
#if USE_FREEVGUI_HEADERS
		return helpButton;
#else
		return _helpButton;
#endif
	}
};

class ButtonStatusSignal : public ActionSignal
{
public:
	ButtonStatusSignal( Label *status, const char *text ) : status( status ), text( text )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		status->setText( "%s", text );
	}

private:
	Label *status;
	const char *text;
};

class ConfigWizardTestMiniApp : public MiniApp
{
public:
	ConfigWizardTestMiniApp()
	{
		setName( "ConfigWizard" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 360, 320 );

		frame->setTitle( "ConfigWizard" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		// layout happens only in setSize, the ctor leaves the children at their creation spots
		ConfigWizardEx *wizard = new ConfigWizardEx( 10, 10, 330, 250 );
		wizard->setSize( 330, 250 );

		// the stock tree folder is named "DonkeyFoo" bruh :(
		TreeFolder *folder = wizard->getFolder();

		TreeFolder *video = new TreeFolder( "Video" );
		video->addChild( new Label( "Resolution" ));
		video->addChild( new Label( "Quality" ));
		folder->addChild( video );

		TreeFolder *audio = new TreeFolder( "Audio" );
		audio->addChild( new Label( "Volume" ));
		folder->addChild( audio );

		TreeFolder *controls = new TreeFolder( "Controls" );
		controls->addChild( new Label( "Bindings" ));
		controls->addChild( new Label( "Mouse" ));
		folder->addChild( controls );

		// open the root so the categories show up as closed "+" boxes
		folder->setOpened( true );

		Label *status = new Label( "press a button", 0, 0 );
		wizard->getClient()->addChild( status );

		wizard->getOkButton()->addActionSignal( new ButtonStatusSignal( status, "Ok pressed" ));
		wizard->getCancelButton()->addActionSignal( new ButtonStatusSignal( status, "Cancel pressed" ));
		wizard->getApplyButton()->addActionSignal( new ButtonStatusSignal( status, "Apply pressed" ));
		wizard->getHelpButton()->addActionSignal( new ButtonStatusSignal( status, "Help pressed" ));

		client->addChild( wizard );

		return frame;
	}
};

DesktopIcon *CreateConfigWizardTest()
{
	return new DesktopIcon( new ConfigWizardTestMiniApp(), LoadTGA( "icons/this_computer.tga" ));
}
