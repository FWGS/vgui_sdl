// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "controls/button.h"
#include "controls/frame.h"
#else
#include <VGUI_Button.h>
#include <VGUI_Frame.h>
#endif

#define START_WIDE 64 // TODO: replace it with the actual size calc

TaskBarEx::TaskBarEx( int x, int y, int wide, int tall ) : TaskBar( x, y, wide, tall )
{
	start = new Button( "Start", 0, 0, START_WIDE, tall - 4 );
	start->setImage( LoadTGA( "icons/start.tga" ));
	start->setContentFitted( false ); // keep our fixed size, don't grow to the icon
#if USE_FREEVGUI_HEADERS
	start->setContentAlignment( Label::CENTER );
#else
	start->setContentAlignment( Label::a_center );
#endif
#if USE_FREEVGUI_HEADERS
	start->setTextAlignment( Label::RIGHT );
#else
	start->setTextAlignment( Label::a_east );
#endif
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
#if USE_FREEVGUI_HEADERS
	for( int i = 0; i < taskButtons.getCount(); i++ )
	{
		int bx, by;

		taskButtons[i]->getPos( bx, by );
		taskButtons[i]->setPos( bx + START_WIDE + 4, by );
	}
#else
	for( int i = 0; i < _taskButtonDar.getCount(); i++ )
	{
		int bx, by;

		_taskButtonDar[i]->getPos( bx, by );
		_taskButtonDar[i]->setPos( bx + START_WIDE + 4, by );
	}
#endif
}

// add missing method to remove the frame
void TaskBarEx::removeFrame( Frame *frame )
{
#if USE_FREEVGUI_HEADERS
	for( int i = 0; i < frames.getCount(); i++ )
	{
		if( frames[i] != frame )
			continue;

		removeChild( taskButtons[i] );

		frames.removeElementAt( i );
		taskButtons.removeElementAt( i );

		invalidateLayout( false );
		repaint();
		return;
	}
#else
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
#endif
}

DesktopEx::DesktopEx( int x, int y, int wide, int tall ) : Desktop( x, y, wide, tall )
{
	// hide original taskbar and instead set our instead. It leaks but who cares
#if USE_FREEVGUI_HEADERS
	taskBar->setVisible( false );
#else
	_taskBar->setVisible( false );
#endif

	taskbar = new TaskBarEx( 0, 0, wide, 36 );
	taskbar->setParent( this );
#if USE_FREEVGUI_HEADERS
	taskBar = taskbar;
#else
	_taskBar = taskbar;
#endif

	setSize( wide, tall ); // re-run stock layout to position the new taskbar
}

TaskBarEx *DesktopEx::getTaskBar()
{
	return taskbar;
}
