// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "signals.h"
#include "controls/button.h"
#include "controls/frame.h"
#include "controls/header.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "controls/table.h"
#include "controls/text.h"
#else
#include <VGUI_ActionSignal.h>
#include <VGUI_CheckButton.h>
#include <VGUI_Frame.h>
#include <VGUI_HeaderPanel.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_TablePanel.h>
#include <VGUI_TextEntry.h>
#endif

static char s_cells[][3][16] =
{
	{ "readme.txt",   "1 KB",   "Text"    },
	{ "hl.exe",       "716 KB", "Program" },
	{ "vgui.so",      "1.3 MB", "Library" },
	{ "valve.rc",     "2 KB",   "Config"  },
	{ "config.cfg",   "4 KB",   "Config"  },
	{ "halflife.wad", "12 MB",  "WAD"     },
	{ "liblist.gam",  "1 KB",   "Text"    },
	{ "crossfire.bsp", "1 MB",  "Map"     },
};

constexpr int NUM_ROWS = (int)( sizeof( s_cells ) / sizeof( s_cells[0] ));

// TablePanel is abstract: the app is the data model, it only asks us for
// the row count and a renderer panel per cell while painting
class DemoTable : public TablePanel
{
public:
	DemoTable( int x, int y, int wide, int tall ) : TablePanel( x, y, wide, tall, 3 )
	{
		renderer = new Label( "" );
	}

	int getRowCount() override
	{
		return NUM_ROWS;
	}

	int getCellTall( int row ) override
	{
		return 18;
	}

	// one shared Label, reparented into each cell by TablePanel::paint
	Panel *getCellRenderer( int column, int row, bool columnSelected, bool rowSelected, bool cellSelected ) override
	{
		renderer->setText( "%s", s_cells[row][column] );

		if( cellSelected )
		{
			renderer->setBgColor( 0, 0, 128, 0 );
			renderer->setFgColor( 255, 255, 255, 0 );
		}
		else if( rowSelected )
		{
			renderer->setBgColor( 168, 168, 168, 0 );
			renderer->setFgColor( 0, 0, 0, 0 );
		}
		else if( columnSelected )
		{
			renderer->setBgColor( 176, 180, 220, 0 );
			renderer->setFgColor( 0, 0, 0, 0 );
		}
		else
		{
			renderer->setBgColor( 192, 192, 192, 0 );
			renderer->setFgColor( 0, 0, 0, 0 );
		}

		return renderer;
	}

	// double click a cell to edit.
	Panel *startCellEditing( int column, int row ) override
	{
		entry = new TextEntry( s_cells[row][column], 0, 0, 80, 18 );
		edit_column = column;
		edit_row = row;

		entry->requestFocus();

		return entry;
	}

	// stock doesn't save edits, we do this here, enabled by a checkbx
	void stopCellEditing() override
	{
		if( save_edits && entry )
			entry->getText( 0, s_cells[edit_row][edit_column], sizeof( s_cells[0][0] ));

		entry = nullptr;

		TablePanel::stopCellEditing();
	}

	void setSaveEdits( bool state )
	{
		save_edits = state;
	}

private:
	Label *renderer;
	TextEntry *entry = nullptr;
	int edit_column = -1;
	int edit_row = -1;
	bool save_edits = true;
};

// drives one of the table's bool setters from a CheckButton
class TableToggleSignal : public ActionSignal
{
public:
	typedef void (DemoTable::*Setter)( bool );

	TableToggleSignal( CheckButton *check, DemoTable *table, Setter setter ) : check( check ), table( table ), setter( setter )
	{
	}

	void actionPerformed( Panel *panel ) override
	{
		( table->*setter )( check->isSelected());
	}

private:
	CheckButton *check;
	DemoTable *table;
	Setter setter;
};

class TablePanelTestMiniApp : public MiniApp
{
public:
	TablePanelTestMiniApp()
	{
		setName( "TablePanel" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 300, 290 );

		frame->setTitle( "TablePanel" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();

		HeaderPanel *header = new HeaderPanel( 10, 10, 260, 20 );

		for( const char *title : { "Name", "Size", "Type" } )
			header->addSectionPanel( new Label( title ));

		// set sizes explicitly, to avoid uninitialzied values
		header->setSliderPos( 0, 100 );
		header->setSliderPos( 1, 180 );
		header->setSliderPos( 2, 260 );

		// header, as you can see, is technically a separate panel, so putting focus on it
		// doesn't translate to focus on table itself
		client->addChild( header );

		DemoTable *table = new DemoTable( 10, 30, 260, 150 );

		// column widths follow the header (drag the dividers)
		table->setHeaderPanel( header );

		// behavior quirk:
		// asking for horizontal lines only: stock stores the first arg
		// into both visibility flags, so vertical lines show up too
		table->setGridVisible( true, false );
		table->setGridSize( 1, 1 );

		// off by default, enable it for us
		table->setColumnSelectionEnabled( true );

		client->addChild( table );

		// cell editing is misleading: it disables selection completely
		AddToggle( client, "Cell editing", 10, 185, table, &TablePanel::setCellEditingEnabled );
		AddToggle( client, "Column select", 150, 185, table, &TablePanel::setColumnSelectionEnabled );
		AddToggle( client, "Row select", 10, 205, table, &TablePanel::setRowSelectionEnabled );

		// "section" is a typo in the stock API
		AddToggle( client, "Cell select", 150, 205, table, &TablePanel::setCellSectionEnabled );
		AddToggle( client, "Save edits", 10, 225, table, &DemoTable::setSaveEdits );

		return frame;
	}

private:
	void AddToggle( Panel *client, const char *text, int x, int y, DemoTable *table, TableToggleSignal::Setter setter )
	{
		CheckButton *check = new CheckButton( text, x, y );

		check->setSelected( true ); // all four start enabled above
		check->addActionSignal( new TableToggleSignal( check, table, setter ));
		client->addChild( check );
	}
};

DesktopIcon *CreateTablePanelTest()
{
	return new DesktopIcon( new TablePanelTestMiniApp(), LoadTGA( "icons/spreadsheet_program.tga" ));
}
