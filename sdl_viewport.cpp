#include "vgui_sdl.h"
#include <VGUI_Desktop.h>
#include <VGUI_DesktopIcon.h>

class Viewport
{
public:
	Viewport( Panel *rootpanel ) :
	            rootpanel( rootpanel )
	{
		int x, y, w, h;

		size_t size;
		void *file;

		// vgui can't load system fonts, so grab temporary bitmap fonts
		file = SDL_LoadFile( "fonts/primary1.tga", &size );
		rootpanel->getApp()->getScheme()->setFont( Scheme::sf_primary1,
		        new Font( "Arial", file, size, 23, 0, 0, 0, false, false, false, false ));
		SDL_free( file );

		file = SDL_LoadFile( "fonts/primary2.tga", &size );
		rootpanel->getApp()->getScheme()->setFont( Scheme::sf_primary2,
		        new Font( "Arial", file, size, 18, 0, 0, 0, false, false, false, false ));
		SDL_free( file );

		file = SDL_LoadFile( "fonts/primary3.tga", &size );
		rootpanel->getApp()->getScheme()->setFont( Scheme::sf_primary3,
		        new Font( "Arial", file, size, 21, 0, 0, 0, false, false, false, false ));
		SDL_free( file );


		rootpanel->getPos( x, y );
		rootpanel->getSize( w, h );

		desktop = new Desktop( x, y, w, h );

		rootpanel->addChild( desktop );
	}

	void AddIcon( DesktopIcon *di )
	{
		desktop->addIcon( di );
	}

	~Viewport()
	{
		rootpanel->removeAllChildren();

		delete desktop;
	}

private:
	Panel *rootpanel;
	Desktop *desktop;
};

static Viewport *viewport;

void CreateViewport( Panel *rootpanel )
{
	viewport = new Viewport( rootpanel );

	viewport->AddIcon( CreateLabelTest( ));
}

void DeleteViewport()
{
	delete viewport;
	viewport = nullptr;
}
