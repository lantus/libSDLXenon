/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifndef _SDL_config_xenon_h
#define _SDL_config_xenon_h
  
#include <stdarg.h>
#include <xetypes.h>
#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <xenos/xenos.h>
#include <usb/usbmain.h>
#include <console/console.h>
#include <xenon_smc/xenon_smc.h>
#include <xenon_soc/xenon_power.h>

typedef unsigned int size_t;

#define HAVE_MMAP 0

#define SDL_BYTEORDER	SDL_BIG_ENDIAN 
#define SDL_CDROM_DISABLED	1 
#define SDL_JOYSTICK_DISABLED	1 
#define SDL_LOADSO_DISABLED	1 
#define SDL_THREADS_XENON	1
#define SDL_AUDIO_DRIVER_XENON	1
#define SDL_VIDEO_DRIVER_XENON	1
#define DEBUG_THREADS		1
#define SDL_THREADS_DISABLED	1
#define HAVE_MALLOC             1
#define HAVE_MEMCPY             1
#define HAVE_MEMSET             1
#define HAVE_FREE               1
#define HAVE_CALLOC             1
#define HAVE_REALLOC            1
#define HAVE_STDIO_H		1

#endif /* _SDL_config_xenon_h */
