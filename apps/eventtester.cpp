// SPDX-License-Identifier: LGPL-3.0-or-later

#include "apps.h"
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "controls/frame.h"
#include "controls/label.h"
#include "image.h"
#include "controls/desktop.h"
#include "panel.h"
#include "layout.h"
#else
#include <VGUI_FocusChangeSignal.h>
#include <VGUI_Frame.h>
#include <VGUI_InputSignal.h>
#include <VGUI_Label.h>
#include <VGUI_LineBorder.h>
#include <VGUI_MiniApp.h>
#include <VGUI_Panel.h>
#include <VGUI_StackLayout.h>
#endif
#include <stdarg.h>

#define LOG_LINES 12
#define ROW_TALL 18

static const char *MouseName( MouseCode code )
{
	switch( code )
	{
	case MOUSE_LEFT:
		return "left";
	case MOUSE_RIGHT:
		return "right";
	case MOUSE_MIDDLE:
		return "middle";
	}

	return "wtf?";
}

// shows input the way VGUI's platform-independent layer delivers it
class EventTester : public Frame, public InputSignal, public FocusChangeSignal
{
public:
	EventTester() : Frame( 0, 0, 360, 430 )
	{
		setTitle( "Event Tester" );
		addFrameSignal( new TestFrameSignal );

		Panel *client = getClient();
		client->setLayout( new StackLayout( 2, true ));

		client->addChild( Header( "Click, type, move and wheel in the box:" ));

		box = new Panel( 0, 0, 320, 44 );
		box->setBorder( new LineBorder );
		box->addInputSignal( this );
		box->addFocusChangeSignal( this );
		client->addChild( box );

		cursor = AddState( client, "cursor" );
		buttons = AddState( client, "buttons" );
		keyfocus = AddState( client, "key focus" );
		keyfocus->setText( "key focus: none" );

		client->addChild( Header( "events (VGUI InputSignal):" ));
		for( int i = 0; i < LOG_LINES; i++ )
		{
			lines[i] = NewLabel( "", 320 );
			client->addChild( lines[i] );
		}

		box->requestFocus();
		UpdateState();
	}

	// mouse
	void cursorMoved( int x, int y, Panel *panel ) override
	{
		cx = x;
		cy = y;
		UpdateState();
	}
	void cursorEntered( Panel *panel ) override
	{
		Log( "cursorEntered" );
	}
	void cursorExited( Panel *panel ) override
	{
		Log( "cursorExited" );
	}
	void mousePressed( MouseCode code, Panel *panel ) override
	{
		down[code] = true;
		panel->requestFocus(); // so keyboard events flow to the box too
		Log( "mousePressed %s", MouseName( code ));
		UpdateState();
	}
	void mouseDoublePressed( MouseCode code, Panel *panel ) override
	{
		Log( "mouseDoublePressed %s", MouseName( code ));
	}
	void mouseReleased( MouseCode code, Panel *panel ) override
	{
		down[code] = false;
		Log( "mouseReleased %s", MouseName( code ));
		UpdateState();
	}
	void mouseWheeled( int delta, Panel *panel ) override
	{
		Log( "mouseWheeled %d", delta );
	}

	// keyboard
	void keyPressed( KeyCode code, Panel *panel ) override
	{
		Log( "keyPressed %s", KeyName( code ));
	}
	void keyTyped( KeyCode code, Panel *panel ) override
	{
		Log( "keyTyped %s", KeyName( code ));
	}
	void keyReleased( KeyCode code, Panel *panel ) override
	{
		Log( "keyReleased %s", KeyName( code ));
	}
	void keyFocusTicked( Panel *panel ) override
	{
		// spammy, disabled
	}

	void focusChanged( bool lost, Panel *panel ) override
	{
		keyfocus->setText( "key focus: %s", lost ? "none" : "box" );
	}

private:
	const char *KeyName( KeyCode code )
	{
		static char buf[48];
		char text[16];

		App::getInstance()->getKeyCodeText( code, text, sizeof( text ));

		if( text[0] )
			SDL_snprintf( buf, sizeof( buf ), "code=%d '%s'", (int)code, text );
		else
			SDL_snprintf( buf, sizeof( buf ), "code=%d", (int)code );

		return buf;
	}

	void UpdateState()
	{
		cursor->setText( "cursor: %d, %d", cx, cy );
		buttons->setText( "buttons: %s%s%s",
			down[MOUSE_LEFT] ? "left " : "",
			down[MOUSE_MIDDLE] ? "middle " : "",
			down[MOUSE_RIGHT] ? "right " : "" );
	}

	void Log( const char *fmt, ... )
	{
		char line[64];
		va_list ap;

		va_start( ap, fmt );
		SDL_vsnprintf( line, sizeof( line ), fmt, ap );
		va_end( ap );

		if( count < LOG_LINES )
		{
			SDL_strlcpy( logbuf[count++], line, sizeof( logbuf[0] ));
		}
		else
		{
			for( int i = 1; i < LOG_LINES; i++ )
				SDL_strlcpy( logbuf[i - 1], logbuf[i], sizeof( logbuf[0] ));
			SDL_strlcpy( logbuf[LOG_LINES - 1], line, sizeof( logbuf[0] ));
		}

		for( int i = 0; i < LOG_LINES; i++ )
			lines[i]->setText( "%s", i < count ? logbuf[i] : "" );
	}

	Label *NewLabel( const char *text, int wide )
	{
		Label *label = new Label( text, 0, 0, wide, ROW_TALL );

#if USE_FREEVGUI_HEADERS
		label->setContentAlignment( Label::LEFT );
#else
		label->setContentAlignment( Label::a_west );
#endif
		label->setContentFitted( false );
		return label;
	}

	Label *Header( const char *text )
	{
		Label *label = NewLabel( text, 320 );

		label->setFgColor( 0, 0, 128, 0 );
		return label;
	}

	Label *AddState( Panel *client, const char *name )
	{
		Label *label = NewLabel( "", 320 );

		client->addChild( label );
		return label;
	}

	Panel *box;
	Label *cursor, *buttons, *keyfocus;
	Label *lines[LOG_LINES];
	char logbuf[LOG_LINES][64] = {};
	int count = 0;
	int cx = 0, cy = 0;
	bool down[MOUSE_LAST] = {};
};

class EventTesterMiniApp : public MiniApp
{
public:
	EventTesterMiniApp()
	{
		setName( "Event Tester" );
	}

	Frame *createInstance()
	{
		return new EventTester;
	}
};

DesktopIcon *CreateEventTesterApp()
{
	return new DesktopIcon( new EventTesterMiniApp(), LoadTGA( "icons/sticky_note.tga" ));
}
