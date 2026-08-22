// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "controls/messagebox.h"
#include "controls/desktop.h"
#else
#include <VGUI_ActionSignal.h>
#include <VGUI_MessageBox.h>
#include <VGUI_MiniApp.h>
#endif

// the stock Ok button only fires the box's ActionSignal subscribers, dismissing the dialog is the app's job
class MessageBoxOkSignal : public ActionSignal
{
public:
	MessageBoxOkSignal( Frame *box ) : box( box )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		DestroyFrame( box );
	}

private:
	Frame *box;
};

class MessageBoxTestMiniApp : public MiniApp
{
public:
	MessageBoxTestMiniApp()
	{
		setName( "MessageBox" );
	}

	// MessageBox is a Frame, so the desktop can host it like any miniapp
	// window. its own Ok button handling is part of what's under test
	Frame *createInstance()
	{
		MessageBox *box = new MessageBox( "MessageBox", "This is a stock vgui::MessageBox.", 0, 0 );

		box->addFrameSignal( new TestFrameSignal );
		box->addActionSignal( new MessageBoxOkSignal( box ));

		return box;
	}
};

DesktopIcon *CreateMessageBoxTest()
{
	return new DesktopIcon( new MessageBoxTestMiniApp(), LoadTGA( "icons/mail.tga" ));
}
