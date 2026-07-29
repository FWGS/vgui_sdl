// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#include <VGUI_Button.h>
#include <VGUI_Frame.h>

#define START_WIDE 64 // TODO: replace it with the actual size calc

TaskBarEx::TaskBarEx( int x, int y, int wide, int tall ) : TaskBar( x, y, wide, tall )
{
	start = new Button( "Start", 0, 0, START_WIDE, tall - 4 );
	start->setImage( LoadTGA( "icons/start.tga" ));
	start->setContentFitted( false ); // keep our fixed size, don't grow to the icon
	start->setContentAlignment( Label::a_center );
	start->setTextAlignment( Label::a_east );
	start->setParent( this );
}

void TaskBarEx::performLayout()
{
	int wide, tall;

	TaskBar::performLayout();

	getSize( wide, tall );
	start->setBounds( 2, 2, START_WIDE, tall - 12 );

	// stock lays the task buttons out from the left edge, nudge them
	// past the Start button
	for( int i = 0; i < _taskButtonDar.getCount(); i++ )
	{
		int bx, by;

		_taskButtonDar[i]->getPos( bx, by );
		_taskButtonDar[i]->setPos( bx + START_WIDE + 4, by );
	}
}

// add missing method to remove the frame
void TaskBarEx::removeFrame( Frame *frame )
{
	for( int i = 0; i < _frameDar.getCount(); i++ )
	{
		if( _frameDar[i] != frame )
			continue;

		removeChild( _taskButtonDar[i] );

		_frameDar.removeElementAt( i );
		_taskButtonDar.removeElementAt( i );

		invalidateLayout( false );
		repaint();
		return;
	}
}

DesktopEx::DesktopEx( int x, int y, int wide, int tall ) : Desktop( x, y, wide, tall )
{
	// hide original taskbar and instead set our instead. It leaks but who cares
	_taskBar->setVisible( false );

	taskbar = new TaskBarEx( 0, 0, wide, 36 );
	taskbar->setParent( this );
	_taskBar = taskbar;

	setSize( wide, tall ); // re-run stock layout to position the new taskbar
}

TaskBarEx *DesktopEx::getTaskBar()
{
	return taskbar;
}
