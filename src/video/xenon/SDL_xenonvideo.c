/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_xboxvideo.c,v 1.1 2003/07/18 15:19:33 lantus Exp $";
#endif

/* XBOX SDL video driver implementation; this is just enough to make an
 *  SDL-based application THINK it's got a working video driver, for
 *  applications that call SDL_Init(SDL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting SDL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that SDL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 *
 * Initial work by Ryan C. Gordon (icculus@linuxgames.com). A good portion
 *  of this was cut-and-pasted from Stephane Peter's work in the AAlib
 *  SDL video driver.  Renamed to "XBOX" by Sam Lantinga.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "SDL.h"
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_mouse.h"

#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_xenonvideo.h" 
#include "../SDL_yuvfuncs.h"

#include "prim_textured_pixel_shader.h"
#include "prim_textured_vertex_shader.h"

#define XBOXVID_DRIVER_NAME "XENON"

/* Initialization/Query functions */
static int XENON_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **XENON_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *XENON_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int XENON_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void XENON_VideoQuit(_THIS);

/* Hardware surface functions */
static int XENON_AllocHWSurface(_THIS, SDL_Surface *surface);
static int XENON_LockHWSurface(_THIS, SDL_Surface *surface);
static void XENON_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void XENON_FreeHWSurface(_THIS, SDL_Surface *surface);
static int XENON_RenderSurface(_THIS, SDL_Surface *surface);
static int XENON_FillHWRect(_THIS, SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);
static int XENON_CheckHWBlit(_THIS, SDL_Surface *src, SDL_Surface *dst);
static int XENON_HWAccelBlit(SDL_Surface *src, SDL_Rect *srcrect,SDL_Surface *dst, SDL_Rect *dstrect);
static int XENON_SetHWAlpha(_THIS, SDL_Surface *surface, Uint8 alpha);
static int XENON_SetHWColorKey(_THIS, SDL_Surface *surface, Uint32 key);
static int XENON_SetFlickerFilter(_THIS, SDL_Surface *surface, int filter);
static int XENON_SetSoftDisplayFilter(_THIS, SDL_Surface *surface, int enabled);

static void XENON_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

/* XBOX driver bootstrap functions */

static int XENON_Available(void)
{
	return(1);
}

static void XENON_DeleteDevice(SDL_VideoDevice *device)
{
	free(device->hidden);
	free(device);
}

static SDL_VideoDevice *XENON_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)malloc(sizeof(SDL_VideoDevice));
	if ( device ) {
		memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *)
				malloc((sizeof *device->hidden));
	}
	if ( (device == NULL) || (device->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( device ) {
			free(device);
		}
		return(0);
	}
	memset(device->hidden, 0, (sizeof *device->hidden));

	/* Set the function pointers */
	device->VideoInit = XENON_VideoInit;
	device->ListModes = XENON_ListModes;
	device->SetVideoMode = XENON_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = XENON_SetColors;
	device->UpdateRects = XENON_UpdateRects;
	device->VideoQuit = XENON_VideoQuit;
	device->AllocHWSurface = XENON_AllocHWSurface;
	device->CheckHWBlit = NULL;
	device->FillHWRect = NULL;
	device->SetHWColorKey = XENON_SetHWColorKey;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = XENON_LockHWSurface;
	device->UnlockHWSurface = XENON_UnlockHWSurface;
	device->FlipHWSurface = XENON_RenderSurface;
	device->FreeHWSurface = XENON_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->InitOSKeymap = XENON_InitOSKeymap;
	device->PumpEvents = XENON_PumpEvents;
	device->free = XENON_DeleteDevice;
	device->SetGammaRamp = XENON_SetGammaRamp;

	return device;
}

VideoBootStrap XBOX_bootstrap = {
	XBOXVID_DRIVER_NAME, "Xenon SDL video driver V0.01",
	XENON_Available, XENON_CreateDevice
};

int XENON_VideoInit(_THIS, SDL_PixelFormat *vformat)
{	 
	 
}

const static SDL_Rect
	RECT_1280x720 = {0,0,1280,720},
	RECT_800x600 = {0,0,800,600},
	RECT_640x480 = {0,0,640,480},
	RECT_512x384 = {0,0,512,384},
	RECT_400x300 = {0,0,400,300},
	RECT_320x240 = {0,0,320,240},
	RECT_320x200 = {0,0,320,200};
const static SDL_Rect *vid_modes[] = {
	&RECT_1280x720,
	&RECT_800x600,
	&RECT_640x480,
	&RECT_512x384,
	&RECT_400x300,
	&RECT_320x240,
	&RECT_320x200,
	NULL
};

SDL_Rect **XENON_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
   	 return &vid_modes;
}