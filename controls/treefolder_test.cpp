// SPDX-License-Identifier: LGPL-3.0-or-later

#include "vgui_sdl.h"
#if USE_FREEVGUI_HEADERS
#include "controls/frame.h"
#include "controls/label.h"
#include "controls/desktop.h"
#include "controls/treefolder.h"
#else
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_TreeFolder.h>
#endif

// TreeFolder is a collapsible tree node: addChild() with Labels for leaves or
// other TreeFolders for sub-branches; setOpened() expands it. Here it stands
// alone (the same widget ConfigWizard uses for its settings tree).
class TreeFolderTestMiniApp : public MiniApp
{
public:
	TreeFolderTestMiniApp()
	{
		setName( "TreeFolder" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 240, 260 );

		frame->setTitle( "TreeFolder" );
		frame->addFrameSignal( new TestFrameSignal );

		TreeFolder *root = new TreeFolder( "Options", 10, 10 );

		TreeFolder *video = new TreeFolder( "Video" );
		video->addChild( new Label( "Resolution" ));
		video->addChild( new Label( "Quality" ));
		video->setOpened( true );
		root->addChild( video );

		TreeFolder *audio = new TreeFolder( "Audio" );
		audio->addChild( new Label( "Volume" ));
		audio->addChild( new Label( "Devices" ));
		root->addChild( audio );

		root->setOpened( true );

		frame->getClient()->addChild( root );

		return frame;
	}
};

DesktopIcon *CreateTreeFolderTest()
{
	return new DesktopIcon( new TreeFolderTestMiniApp(), LoadTGA( "icons/folder_closed.tga" ));
}
