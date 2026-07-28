// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#include <VGUI_Button.h>
#include <VGUI_Frame.h>

TaskBarEx::TaskBarEx( int x, int y, int wide, int tall ) : TaskBar( x, y, wide, tall )
{
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
