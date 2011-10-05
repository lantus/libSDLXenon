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

#ifndef _SDL_xenonvideo_h
#define _SDL_xenonvideo_h

#include "SDL_mouse.h"
#include "SDL_sysvideo.h"
#include "SDL_mutex.h"

/* Hidden "this" pointer for the video functions */
#define _THIS	SDL_VideoDevice *this

/* Private display data */

typedef struct _VERTEX {
		float x, y, z;
		float rhw;
		float u, v;		 
} VERTEX; //our custom vertex with a constuctor for easier assignment

struct SDL_PrivateVideoData {
    int w, h; 
    struct XenosSurface *SDL_Primary;
};

struct XenosDevice  		*xe;
struct XenosSurface 		*fb;
struct XenosShader  		*sdl_ps;
struct XenosShader  		*sdl_vs;
struct XenosVertexBuffer 	*vb;

unsigned char* screen;

SDL_Overlay *XBOX_CreateYUVOverlay(_THIS, int width, int height, Uint32 format, SDL_Surface *display);
int XBOX_DisplayYUVOverlay(_THIS, SDL_Overlay *overlay, SDL_Rect *src, SDL_Rect *dst);
int XBOX_LockYUVOverlay(_THIS, SDL_Overlay *overlay);
void XBOX_UnlockYUVOverlay(_THIS, SDL_Overlay *overlay);
void XBOX_FreeYUVOverlay(_THIS, SDL_Overlay *overlay);
int XBOX_SetGammaRamp(_THIS, Uint16 *ramp);

#endif /* _SDL_xenonvideo_h */
