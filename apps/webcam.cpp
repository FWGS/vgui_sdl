// SPDX-License-Identifier: LGPL-3.0-or-later

#include "apps.h"
#include <VGUI_Frame.h>
#include <VGUI_Label.h>
#include <VGUI_MiniApp.h>
#include <VGUI_Panel.h>
#include <VGUI_StackLayout.h>

// this purely works because VGUI is redrawn every frame by our Surface.
// I think the intention was to let VGUI only update when necessary
// but for game engine integration purposes I don't give a damn fuck about that.
class WebcamPanel : public Panel
{
public:
	WebcamPanel( int x, int y, int wide, int tall ) : Panel( x, y, wide, tall )
	{
	}

	void setCamera( SDL_Camera *camera )
	{
		cam = camera;
	}

protected:
	void paintBackground() override
	{
		int wide, tall;

		getSize( wide, tall );

		if( cam )
		{
			SDL_Surface *frame = SDL_AcquireCameraFrame( cam, nullptr );

			if( frame )
			{
				SDL_Surface *rgba = SDL_ConvertSurface( frame, SDL_PIXELFORMAT_RGBA32 );

				SDL_ReleaseCameraFrame( cam, frame );

				if( rgba )
				{
					if( !id )
						id = getSurfaceBase()->createNewTextureID();

					drawSetTextureRGBA( id, (const char *)rgba->pixels, rgba->w, rgba->h );
					got_frame = true;
					SDL_DestroySurface( rgba );
				}
			}
		}

		if( got_frame )
		{
			drawSetTexture( id );
			drawTexturedRect( 0, 0, wide, tall );
		}
		else
		{
			// no frame yet (no camera, or permission still pending)
			drawSetColor( 0, 0, 0, 0 );
			drawFilledRect( 0, 0, wide, tall );
		}
	}

private:
	SDL_Camera *cam = nullptr;
	int id = 0;
	bool got_frame = false;
};

class WebcamMiniApp : public MiniApp
{
public:
	WebcamMiniApp()
	{
		setName( "Webcam" );
	}

	Frame *createInstance()
	{
		Frame *frame = new Frame( 0, 0, 340, 300 );

		frame->setTitle( "Webcam" );
		frame->addFrameSignal( new TestFrameSignal );

		Panel *client = frame->getClient();
		client->setLayout( new StackLayout( 4, true ));

		WebcamPanel *view = new WebcamPanel( 0, 0, 320, 240 );
		client->addChild( view );

		Label *status = new Label( "", 0, 0, 320, 20 );
		status->setContentAlignment( Label::a_west );
		status->setContentFitted( false );
		client->addChild( status );

		SDL_InitSubSystem( SDL_INIT_CAMERA );

		int count = 0;
		SDL_CameraID *cameras = SDL_GetCameras( &count );

		if( count > 0 )
		{
			SDL_Camera *cam = SDL_OpenCamera( cameras[0], nullptr );

			view->setCamera( cam );
			status->setText( "%s", SDL_GetCameraName( cameras[0] ));
		}
		else
		{
			status->setText( "no camera found" );
		}

		SDL_free( cameras );

		return frame;
	}
};

DesktopIcon *CreateWebcamApp()
{
	return new DesktopIcon( new WebcamMiniApp(), LoadTGA( "icons/camera.tga" ));
}
