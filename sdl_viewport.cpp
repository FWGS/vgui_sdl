#include "vgui_sdl.h"
#include "apps/apps.h"
#if USE_FREEVGUI_HEADERS
#include "controls/desktop.h"
#include "image.h"
#include "inputstream.h"
#include "controls/frame.h"
#include "controls/label.h"
#else
#include <VGUI_Desktop.h>
#include <VGUI_DesktopIcon.h>
#include <VGUI_BitmapTGA.h>
#include <VGUI_FileInputStream.h>
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#endif

Image *LoadTGA( const char *path )
{
	// invertAlpha=true keeps the regular 255 = opaque convention our
	// SDL surface expects, as loadTGA inverts the alpha otherwise
	FileInputStream fis( path, false );
	BitmapTGA *bitmap = new BitmapTGA( &fis, true );

	fis.close();

	return bitmap;
}

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
#if USE_FREEVGUI_HEADERS
		rootpanel->getApp()->getScheme()->setFont( Scheme::SF_PRIMARY1,
#else
		rootpanel->getApp()->getScheme()->setFont( Scheme::sf_primary1,
#endif
		        new Font( "Arial", file, size, 23, 0, 0, 0, false, false, false, false ));
		SDL_free( file );

		file = SDL_LoadFile( "fonts/primary2.tga", &size );
#if USE_FREEVGUI_HEADERS
		rootpanel->getApp()->getScheme()->setFont( Scheme::SF_PRIMARY2,
#else
		rootpanel->getApp()->getScheme()->setFont( Scheme::sf_primary2,
#endif
		        new Font( "Arial", file, size, 18, 0, 0, 0, false, false, false, false ));
		SDL_free( file );

		// vgui::Frame wants marlett font at sf_secondary1
		file = SDL_LoadFile( "fonts/marlett.tga", &size );
#if USE_FREEVGUI_HEADERS
		rootpanel->getApp()->getScheme()->setFont( Scheme::SF_SECONDARY,
#else
		rootpanel->getApp()->getScheme()->setFont( Scheme::sf_secondary1,
#endif
		        new Font( "Marlett", file, size, 12, 0, 0, 0, false, false, false, false ));
		SDL_free( file );

		file = SDL_LoadFile( "fonts/primary3.tga", &size );
#if USE_FREEVGUI_HEADERS
		rootpanel->getApp()->getScheme()->setFont( Scheme::SF_PRIMARY3,
#else
		rootpanel->getApp()->getScheme()->setFont( Scheme::sf_primary3,
#endif
		        new Font( "Arial", file, size, 21, 0, 0, 0, false, false, false, false ));
		SDL_free( file );


		rootpanel->getPos( x, y );
		rootpanel->getSize( w, h );

		desktop = new DesktopEx( x, y, w, h );

		rootpanel->addChild( desktop );

		AddNotice( "VGUI SDL3 testbed", 3 );
		char s[128];
		snprintf( s, sizeof( s ), "Compiled with SDL %d.%d.%d",
			SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION );
		AddNotice( s, 2 );

		int v = SDL_GetVersion();
		snprintf( s, sizeof( s ), "SDL runtime is %d.%d.%d",
			SDL_VERSIONNUM_MAJOR( v ), SDL_VERSIONNUM_MINOR( v ), SDL_VERSIONNUM_MICRO( v ));
		AddNotice( s, 1 );

	}

	void AddNotice( const char *text, int pos )
	{
		int w, h;
		rootpanel->getSize( w, h );

		int lw, lt;
		Label *notice = new Label( text, 0, 0 );

		notice->setBgColor( 0, 0, 0, 255 ); // transparent
		notice->setFgColor( 255, 255, 255, 0 );
		notice->getSize( lw, lt );
		notice->setPos( w - lw - 8, h - 36 - lt * pos - 4 );

		desktop->getBackground()->addChild( notice );
	}

	void AddIcon( DesktopIcon *di )
	{
		// fix for gray background on icons (inverted alpha is stupid)
		di->setBgColor( 0, 0, 0, 255 );

		desktop->addIcon( di );
	}

	DesktopEx *GetDesktop()
	{
		return desktop;
	}

	~Viewport()
	{
		rootpanel->removeAllChildren();

		delete desktop;
	}

private:
	Panel *rootpanel;
	DesktopEx *desktop;
};

static Viewport *viewport;

// it leaks, but I don't care, VGUI is leak-driven software anyway 
void DestroyFrame( Frame *frame )
{
	frame->setVisible( false );
	frame->getApp()->requestFocus( nullptr );

	viewport->GetDesktop()->getTaskBar()->removeFrame( frame );

	if( frame->getParent())
		frame->getParent()->removeChild( frame );
}

void CreateViewport( Panel *rootpanel )
{
	viewport = new Viewport( rootpanel );

	viewport->AddIcon( CreateLabelTest( ));
	viewport->AddIcon( CreateScrollPanelTest( ));
	viewport->AddIcon( CreateTabPanelTest( ));
	viewport->AddIcon( CreateButtonTest( ));
	viewport->AddIcon( CreateTextEntryTest( ));
	viewport->AddIcon( CreateSliderTest( ));
	viewport->AddIcon( CreateProgressBarTest( ));
	viewport->AddIcon( CreateImagePanelTest( ));
	viewport->AddIcon( CreateMessageBoxTest( ));
	viewport->AddIcon( CreateMenuTest( ));
	viewport->AddIcon( CreateListPanelTest( ));
	viewport->AddIcon( CreateTablePanelTest( ));
	viewport->AddIcon( CreateWizardTest( ));
	viewport->AddIcon( CreateConfigWizardTest( ));
	viewport->AddIcon( CreateBorderTest( ));
	viewport->AddIcon( CreateBuildGroupTest( ));
	viewport->AddIcon( CreateFocusNavTest( ));
	viewport->AddIcon( CreateFrameTest( ));
	viewport->AddIcon( CreateHeaderPanelTest( ));
	viewport->AddIcon( CreateScrollBarTest( ));
	viewport->AddIcon( CreateTreeFolderTest( ));
	viewport->AddIcon( CreateStateMatrixTest( ));
	viewport->AddIcon( CreatePropertyPanelTest( ));

	viewport->GetDesktop()->arrangeIcons();

	CreateStartMenu( viewport->GetDesktop());
}

void DeleteViewport()
{
	delete viewport;
	viewport = nullptr;
}
