// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "controls/button.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "controls/text.h"
#include "controls/wizard.h"
#else
#include <VGUI_ActionSignal.h>
#include <VGUI_CheckButton.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_TextEntry.h>
#include <VGUI_WizardPanel.h>
#endif

class WizardStatusSignal : public ActionSignal
{
public:
	WizardStatusSignal( WizardPanel *wizard, Label *status, const char *event )
		: wizard( wizard ), status( status ), event( event )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		if( wizard )
		{
			char title[64];

			wizard->getCurrentWizardPageTitle( title, sizeof( title ));
			status->setText( "%s: %s", event, title );
		}
		else
		{
			status->setText( "%s", event );
		}
	}

private:
	WizardPanel *wizard;
	Label *status;
	const char *event;
};

// wizard is pretty cool and can branch
class WizardBranchSignal : public ActionSignal
{
public:
	WizardBranchSignal( CheckButton *check, WizardPanel::WizardPage *from, WizardPanel::WizardPage *taken, WizardPanel::WizardPage *skipped )
		: check( check ), from( from ), taken( taken ), skipped( skipped )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		from->setNextWizardPage( check->isSelected() ? taken : skipped );
	}

private:
	CheckButton *check;
	WizardPanel::WizardPage *from;
	WizardPanel::WizardPage *taken;
	WizardPanel::WizardPage *skipped;
};

class WizardTestMiniApp : public MiniApp
{
public:
	WizardTestMiniApp()
	{
		setName( "Wizard" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 320, 310 );

		frame->setTitle( "Wizard" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		Label *status = new Label( "page: none", 10, 220 );
		client->addChild( status );

		Label *switched = new Label( "switch: none", 10, 240 );
		client->addChild( switched );

		WizardPanel *wizard = new WizardPanel( 10, 10, 290, 190 );

		WizardPanel::WizardPage *welcome = new WizardPanel::WizardPage();
		welcome->setTitle( "Welcome" );
		welcome->addChild( new Label( "Welcome to the test wizard", 10, 10 ));
		welcome->setBackButtonVisible( false ); // no back button at first page
		welcome->addSwitchingToNextPageSignal( new WizardStatusSignal( nullptr, switched, "switch: leaving Welcome fwd" ));

		WizardPanel::WizardPage *options = new WizardPanel::WizardPage();
		options->setTitle( "Options" );
		options->addChild( new Label( "Pick your options", 10, 10 ));
		options->addSwitchingToBackPageSignal( new WizardStatusSignal( nullptr, switched, "switch: leaving Options back" ));

		// the wizard focuses this on page change (wanted focus)
		TextEntry *name = new TextEntry( "your name here", 10, 60, 150, 24 );
		options->addChild( name );
		options->setWantedFocus( name );

		WizardPanel::WizardPage *advanced = new WizardPanel::WizardPage();
		advanced->setTitle( "Advanced" );
		advanced->addChild( new Label( "Optional page, taken only when", 10, 10 ));
		advanced->addChild( new Label( "the checkbox was on", 10, 30 ));

		WizardPanel::WizardPage *finish = new WizardPanel::WizardPage();
		finish->setTitle( "Finish" );
		finish->addChild( new Label( "All done", 10, 10 ));
		// custom per-page button text, and no pointless Next
		finish->setFinishedButtonText( "Done!" );
		finish->setFinishedButtonEnabled( true );
		finish->setNextButtonVisible( false );

		CheckButton *branch = new CheckButton( "Visit the advanced page", 10, 90 );
		branch->addActionSignal( new WizardBranchSignal( branch, options, advanced, finish ));
		options->addChild( branch );

		welcome->setNextWizardPage( options );
		options->setBackWizardPage( welcome );
		options->setNextWizardPage( finish ); // checkbox off by default
		advanced->setBackWizardPage( options );
		advanced->setNextWizardPage( finish );
		finish->setBackWizardPage( options );

		wizard->addPageChangedActionSignal( new WizardStatusSignal( wizard, status, "page" ));
		// these forward onto the Finished/Cancel buttons' signal lists
		wizard->addFinishedActionSignal( new WizardStatusSignal( nullptr, status, "wizard: finished" ));
		wizard->addCancelledActionSignal( new WizardStatusSignal( nullptr, status, "wizard: cancelled" ));

		wizard->setCurrentWizardPage( welcome );

		client->addChild( wizard );

		return frame;
	}
};

DesktopIcon *CreateWizardTest()
{
	return new DesktopIcon( new WizardTestMiniApp(), LoadTGA( "icons/tools.tga" ));
}
