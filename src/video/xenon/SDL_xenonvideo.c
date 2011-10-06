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

// XENON video driver 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "SDL.h"
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_sysvideo.h"
#include "SDL_pixels_c.h"

#include "SDL_xenonvideo.h" 
#include "SDL_xenonevents.h"
#include "../SDL_yuvfuncs.h"

#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <xenos/xenos.h>
#include <usb/usbmain.h>
#include <console/console.h>
#include <xenon_smc/xenon_smc.h>
#include <xenon_soc/xenon_power.h>

#include "prim_textured_pixel_shader.h"
#include "prim_textured_vertex_shader.h"

#define XENONVID_DRIVER_NAME "XENON"

static struct XenosDevice 	_xe;
int have_texture = 0;

enum {
    UvBottom = 0,
    UvTop,
    UvLeft,
    UvRight
};
float ScreenUv[4] = {0.f, 1.0f, 1.0f, 0.f};

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
static int XENON_FlipHWSurface(_THIS, SDL_Surface *surface);
static int XENON_FillHWRect(_THIS, SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);
static int XENON_CheckHWBlit(_THIS, SDL_Surface *src, SDL_Surface *dst);
static int XENON_HWAccelBlit(SDL_Surface *src, SDL_Rect *srcrect,SDL_Surface *dst, SDL_Rect *dstrect);
static int XENON_SetHWAlpha(_THIS, SDL_Surface *surface, Uint8 alpha);
static int XENON_SetHWColorKey(_THIS, SDL_Surface *surface, Uint32 key);
static int XENON_SetFlickerFilter(_THIS, SDL_Surface *surface, int filter);
static int XENON_SetSoftDisplayFilter(_THIS, SDL_Surface *surface, int enabled);
static int XENON_SetGammaRamp(_THIS, Uint16 *ramp);
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
	device->FlipHWSurface = XENON_FlipHWSurface;
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

	printf("XENON SDL Video: Created Device\n");

	return device;
}

VideoBootStrap XENON_bootstrap = {
	XENONVID_DRIVER_NAME, "Xenon SDL video driver V0.01",
	XENON_Available, XENON_CreateDevice
};

int XENON_VideoInit(_THIS, SDL_PixelFormat *vformat)
{	  
	xe = &_xe;	
	Xe_Init(xe);
 
	fb = Xe_GetFramebufferSurface(xe);
    	Xe_SetRenderTarget(xe, fb);
	
	vformat->BitsPerPixel = 32;
	vformat->BytesPerPixel = 4;

	vformat->Amask = 0xFF000000;
	vformat->Rmask = 0x00FF0000;
	vformat->Gmask = 0x0000FF00;
	vformat->Bmask = 0x000000FF;
		 
	this->hidden->SDL_Primary = NULL;
	screen = NULL;
 
	if (fb)
	{ 
		return 1;	
	}
	else
        {
		return 0;
	}
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

SDL_Surface *XENON_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{

	int pixel_mode,pitch;
	Uint32 Rmask, Gmask, Bmask;

	if (bpp==8)
		bpp=16;
 
	switch(bpp)
	{
		case 15:
			pitch = width*2;
			Rmask = 0x00007c00;
			Gmask = 0x000003e0;
			Bmask = 0x0000001f;
			pixel_mode = XE_FMT_5551 | XE_FMT_ARGB;
			break;
		case 8:
		case 16:
			pitch = width*2;
			Rmask = 0x0000f800;
			Gmask = 0x000007e0;
			Bmask = 0x0000001f;
			pixel_mode = XE_FMT_565 | XE_FMT_ARGB;
			break;
		case 24:
		case 32:
			pitch = width*4;
			pixel_mode = XE_FMT_8888 | XE_FMT_ARGB;
			Rmask = 0x00FF0000;
			Gmask = 0x0000FF00;
			Bmask = 0x000000FF;
			break;
		default:
			SDL_SetError("Couldn't find requested mode in list");
			return(NULL);
	}

	/* Allocate the new pixel format for the screen */
	if ( ! SDL_ReallocFormat(current, bpp, Rmask, Gmask, Bmask, 0) ) {
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return(NULL);
	}

 
	if (!have_texture)
	{
		this->hidden->SDL_Primary = Xe_CreateTexture(xe, width, height, 1, pixel_mode, 0);
	}

	if (!this->hidden->SDL_Primary)
	{
		have_texture=1;
		SDL_SetError("Couldn't create Xenon Texture!");
		return(NULL);
	}

	// set up the shaders
	
	static const struct XenosVBFFormat vbf = {
        2,
        	{
            		{XE_USAGE_POSITION, 0, XE_TYPE_FLOAT4},
            		{XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
        	}
    	};	 
	
    	sdl_ps = Xe_LoadShaderFromMemory(xe, (void*) prim_textured_pixel_shader);
    	Xe_InstantiateShader(xe, sdl_ps, 0);
    	sdl_vs = Xe_LoadShaderFromMemory(xe, (void*) prim_textured_vertex_shader);
    	Xe_InstantiateShader(xe, sdl_vs, 0);	 
    	Xe_ShaderApplyVFetchPatches(xe, sdl_vs, 0, &vbf);
	
	//Must be called before the first rendered frame
    	
	edram_init(xe);

	/* Set up the new mode framebuffer */
	current->flags = (SDL_FULLSCREEN|SDL_HWSURFACE);

	if (flags & SDL_DOUBLEBUF)
		current->flags |= SDL_DOUBLEBUF;

	this->hidden->w = current->w = width;
	this->hidden->h = current->h = height;

	current->pitch = current->w * (bpp / 8);
	current->pixels = NULL;

     
    	float x = -1.0f;
    	float y = 1.0f;
    	float w = 4.0f;
    	float h = 4.0f;
 

    	vb = Xe_CreateVertexBuffer(xe, 4 * sizeof(VERTEX));
    	VERTEX *Rect = Xe_VB_Lock(xe, vb, 0, 4 * sizeof (VERTEX), XE_LOCK_WRITE);
 
        ScreenUv[UvTop] = ScreenUv[UvTop]*2;
        ScreenUv[UvLeft] = ScreenUv[UvLeft]*2;

        // top left
        Rect[0].x = x;
        Rect[0].y = y;
        Rect[0].u = ScreenUv[UvBottom];
        Rect[0].v = ScreenUv[UvRight]; 

        // bottom left
        Rect[1].x = x;
        Rect[1].y = y - h;
        Rect[1].u = ScreenUv[UvBottom];
        Rect[1].v = ScreenUv[UvLeft]; 

        // top right
        Rect[2].x = x + w;
        Rect[2].y = y;
        Rect[2].u = ScreenUv[UvTop];
        Rect[2].v = ScreenUv[UvRight]; 

        // top right
        Rect[3].x = x + w;
        Rect[3].y = y;
        Rect[3].u = ScreenUv[UvTop];
        Rect[3].v = ScreenUv[UvRight]; 

        int i = 0;
        for (i = 0; i < 3; i++) {
            Rect[i].z = 0.0;
            Rect[i].rhw = 1.0;
        }
    
    	Xe_VB_Unlock(xe, vb);
    	Xe_SetClearColor(xe, 0);
 
	// Reset states
	Xe_InvalidateState(xe);
	Xe_SetClearColor(xe, 0);

	// Select stream and shaders
	Xe_SetTexture(xe, 0, this->hidden->SDL_Primary);
	Xe_SetCullMode(xe, XE_CULL_NONE);
	Xe_SetStreamSource(xe, 0, vb, 0, sizeof (VERTEX));
	Xe_SetShader(xe, SHADER_TYPE_PIXEL, sdl_ps, 0);
	Xe_SetShader(xe, SHADER_TYPE_VERTEX, sdl_vs, 0);

	// Draw
	Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 2);

	// Resolve
	Xe_Resolve(xe); 
	Xe_Sync(xe);
    
	/* We're done */
	return(current);
}

/* We don't actually allow hardware surfaces other than the main one */

static int XENON_AllocHWSurface(_THIS, SDL_Surface *surface)
{

	return(-1);
}
static void XENON_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

static int XENON_FlipHWSurface(_THIS, SDL_Surface *surface)
{
	if(this->hidden->SDL_Primary == NULL)
		return -1;

	// Reset states
	Xe_InvalidateState(xe);
	Xe_SetClearColor(xe, 0);

	// Select stream and shaders
	Xe_SetTexture(xe, 0, this->hidden->SDL_Primary);
	Xe_SetCullMode(xe, XE_CULL_NONE);
	Xe_SetStreamSource(xe, 0, vb, 0, sizeof (VERTEX));
	Xe_SetShader(xe, SHADER_TYPE_PIXEL, sdl_ps, 0);
	Xe_SetShader(xe, SHADER_TYPE_VERTEX, sdl_vs, 0);

	// Draw
	Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 1);
 
	Xe_Resolve(xe); 
	Xe_Sync(xe);
		 	
	return (1);
}
 

static int XBOX_FillHWRect(_THIS, SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
	Xe_SetClearColor(xe, color);
}


static int XENON_HWAccelBlit(SDL_Surface *src, SDL_Rect *srcrect,
					SDL_Surface *dst, SDL_Rect *dstrect)
{
	return(1);
}

static int XENON_CheckHWBlit(_THIS, SDL_Surface *src, SDL_Surface *dst)
{
	return(0);
}

/* We need to wait for vertical retrace on page flipped displays */
static int XENON_LockHWSurface(_THIS, SDL_Surface *surface)
{
	if (!this->hidden->SDL_Primary)
		return (-1);

	Xe_SetClearColor(xe, 0x00000000);

	screen = (unsigned char *)Xe_Surface_LockRect(xe, this->hidden->SDL_Primary, 0, 0, 0, 0, XE_LOCK_WRITE);

	surface->pitch = this->hidden->SDL_Primary->wpitch;
	surface->pixels = screen;

	if (surface->pixels)
		return(0);
	else
		return(-1);

}

static void XENON_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
	Xe_Surface_Unlock(xe, this->hidden->SDL_Primary);
		 
	return;
}

static void XENON_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
 
	if(this->hidden->SDL_Primary == NULL)
		return;

	// Reset states
	Xe_InvalidateState(xe);
	Xe_SetClearColor(xe, 0);

	// Select stream and shaders
	Xe_SetTexture(xe, 0, this->hidden->SDL_Primary);
	Xe_SetCullMode(xe, XE_CULL_NONE);
	Xe_SetStreamSource(xe, 0, vb, 0, sizeof (VERTEX));
	Xe_SetShader(xe, SHADER_TYPE_PIXEL, sdl_ps, 0);
	Xe_SetShader(xe, SHADER_TYPE_VERTEX, sdl_vs, 0);

	// Draw
	Xe_DrawPrimitive(xe, XE_PRIMTYPE_TRIANGLELIST, 0, 2);
 
	Xe_Resolve(xe); 
	Xe_Sync(xe);

}

int XENON_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	return(1);
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
void XENON_VideoQuit(_THIS)
{
	  
 	this->screen->pixels = NULL;
	 
}

static int XENON_SetHWAlpha(_THIS, SDL_Surface *surface, Uint8 alpha)
{
	return(1);
}

static int XENON_SetHWColorKey(_THIS, SDL_Surface *surface, Uint32 key)
{
	
	return(0);
}

static int XENON_SetGammaRamp(_THIS, Uint16 *ramp)
{
	// unsure what to do here 
	return 0;

}
 
 
