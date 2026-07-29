// SPDX-License-Identifier: LGPL-3.0-or-later

#include "apps.h"
#include <VGUI_ActionSignal.h>
#include <VGUI_BorderLayout.h>
#include <VGUI_Button.h>
#include <VGUI_FlowLayout.h>
#include <VGUI_Font.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_Scheme.h>
#include <VGUI_StackLayout.h>
#include <VGUI_TabPanel.h>

#define ROW_TALL 18

static const struct FontSlot
{
	Scheme::SchemeFont sf;
	const char *name;
} s_fonts[] =
{
	{ Scheme::sf_primary1, "primary1" },
	{ Scheme::sf_primary2, "primary2" },
	{ Scheme::sf_primary3, "primary3" },
	{ Scheme::sf_secondary1, "secondary1" },
};

struct ColorSlot
{
	Scheme::SchemeColor sc;
	const char *name;
} s_colors[] =
{
	{ Scheme::sc_black, "black" },
	{ Scheme::sc_white, "white" },
	{ Scheme::sc_primary1, "primary1" },
	{ Scheme::sc_primary2, "primary2" },
	{ Scheme::sc_primary3, "primary3" },
	{ Scheme::sc_secondary1, "secondary1" },
	{ Scheme::sc_secondary2, "secondary2" },
	{ Scheme::sc_secondary3, "secondary3" },
};

#define NUM_FONTS ( (int)( sizeof( s_fonts ) / sizeof( s_fonts[0] )))
#define NUM_COLORS ( (int)( sizeof( s_colors ) / sizeof( s_colors[0] )))

class VGUIInfoFrame;

class VGUIRefreshSignal : public ActionSignal
{
public:
	VGUIRefreshSignal( VGUIInfoFrame *frame ) : frame( frame ) {}

	void actionPerformed( Panel *panel ) override;

private:
	VGUIInfoFrame *frame;
};

// the idea of this app is to report what VGUI sees as platform-indepdendent layer
// there isn't a lot, vgui::App/vgui::Surface provide only so much and we implement even less
class VGUIInfoFrame : public Frame
{
public:
	VGUIInfoFrame() : Frame( 0, 0, 340, 300 )
	{
		setTitle( "VGUI Info" );
		addFrameSignal( new TestFrameSignal );

		Scheme *scheme = App::getInstance()->getScheme();

		// fill the client with the tab panel
		Panel *client = getClient();
		BorderLayout *border = new BorderLayout( 0 );
		client->setLayout( border );

		TabPanel *tabs = new TabPanel( 0, 0, 10, 10 );
		tabs->setLayoutInfo( border->createLayoutInfo( BorderLayout::a_center ));
		client->addChild( tabs );

		Panel *schemeTab = tabs->addTab( "Scheme" );
		schemeTab->setLayout( new StackLayout( 3, true ));
		for( int i = 0; i < NUM_COLORS; i++ )
			AddColorRow( schemeTab, scheme, s_colors[i] );

		Panel *fontsTab = tabs->addTab( "Fonts" );
		fontsTab->setLayout( new StackLayout( 3, true ));
		fontsTab->addChild( Header( "name: tall/wide/id + sample" ));
		for( int i = 0; i < NUM_FONTS; i++ )
			AddFontRow( fontsTab, scheme, s_fonts[i] );

		Panel *sysTab = tabs->addTab( "System" );
		sysTab->setLayout( new StackLayout( 3, true ));
		backend = AddInfoRow( sysTab, "Backend" );
		uptime = AddInfoRow( sysTab, "App uptime" );
		cursor = AddInfoRow( sysTab, "Cursor pos" );
		focus = AddInfoRow( sysTab, "Key focus" );

		Panel *buttons = new Panel( 0, 0, 300, 28 );
		buttons->setLayout( new FlowLayout( 4 ));
		Button *refresh = new Button( "Refresh", 0, 0, 70, 24 );
		refresh->addActionSignal( new VGUIRefreshSignal( this ));
		buttons->addChild( refresh );
		sysTab->addChild( buttons );

		Refresh();
	}

	void Refresh()
	{
		App *app = App::getInstance();
		int cx = 0, cy = 0;

		app->getCursorPos( cx, cy );

		const char *fv = Sys_FreeVGUIVersion();
		backend->setText( "%s", fv ? fv : "proprietary vgui" );
		uptime->setText( "%ld ms", app->getTimeMillis());
		cursor->setText( "%d, %d", cx, cy );
		focus->setText( "%s", app->getFocus() ? "a panel has focus" : "none" );
	}

private:
	Label *NewLabel( const char *text, int wide )
	{
		Label *label = new Label( text, 0, 0, wide, ROW_TALL );

		label->setContentAlignment( Label::a_west );
		label->setContentFitted( false );
		return label;
	}

	Label *Header( const char *text )
	{
		Label *label = NewLabel( text, 300 );

		label->setFgColor( 0, 0, 128, 0 );
		return label;
	}

	Label *AddInfoRow( Panel *tab, const char *name )
	{
		char text[64];

		SDL_snprintf( text, sizeof( text ), "%s:", name );

		Panel *row = new Panel( 0, 0, 300, ROW_TALL );
		row->setLayout( new FlowLayout( 4 ));
		row->addChild( NewLabel( text, 90 ));

		Label *value = NewLabel( "", 200 );
		row->addChild( value );
		tab->addChild( row );

		return value;
	}

	void AddFontRow( Panel *tab, Scheme *scheme, const FontSlot &slot )
	{
		Font *font = scheme->getFont( slot.sf );

		Panel *row = new Panel( 0, 0, 300, ROW_TALL );
		row->setLayout( new FlowLayout( 6 ));
		row->addChild( NewLabel( slot.name, 88 ));

		char metrics[48];
		if( font )
			SDL_snprintf( metrics, sizeof( metrics ), "%d/%d/%d", font->getTall(), font->getWide(), font->getId());
		else
			SDL_strlcpy( metrics, "unset", sizeof( metrics ));
		row->addChild( NewLabel( metrics, 74 ));

		// render the same sample in the actual scheme font
		Label *sample = NewLabel( "AaBbCc 0123", 140 );
		if( font )
			sample->setFont( slot.sf );
		row->addChild( sample );

		tab->addChild( row );
	}

	void AddColorRow( Panel *tab, Scheme *scheme, const ColorSlot &slot )
	{
		int r = 0, g = 0, b = 0, a = 0;

		scheme->getColor( slot.sc, r, g, b, a );

		Panel *row = new Panel( 0, 0, 300, ROW_TALL );
		row->setLayout( new FlowLayout( 6 ));

		Panel *swatch = new Panel( 0, 0, 28, ROW_TALL - 2 );
		swatch->setBgColor( r, g, b, a );
		row->addChild( swatch );

		char text[64];
		SDL_snprintf( text, sizeof( text ), "%s: %d,%d,%d,%d", slot.name, r, g, b, a );
		row->addChild( NewLabel( text, 240 ));

		tab->addChild( row );
	}

	Label *backend, *uptime, *cursor, *focus;
};

void VGUIRefreshSignal::actionPerformed( Panel *panel )
{
	frame->Refresh();
}

class VGUIInfoMiniApp : public MiniApp
{
public:
	VGUIInfoMiniApp()
	{
		setName( "VGUI Info" );
	}

	Frame *createInstance()
	{
		return new VGUIInfoFrame;
	}
};

DesktopIcon *CreateVGUIInfoApp()
{
	return new DesktopIcon( new VGUIInfoMiniApp(), LoadTGA( "icons/program.tga" ));
}
