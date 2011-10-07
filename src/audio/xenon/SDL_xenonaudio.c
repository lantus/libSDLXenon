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

#include <stdio.h>

#include "SDL_types.h"
#include "SDL_error.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "SDL_audio_c.h"
#include "SDL_xenonaudio.h"
 
/* Audio driver functions */
static int XENON_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void XENON_WaitAudio_BusyWait(_THIS);
static void XENON_PlayAudio(_THIS);
static Uint8 *XENON_GetAudioBuf(_THIS);
static void XENON_WaitDone(_THIS);
static void XENON_CloseAudio(_THIS);

/* Audio driver bootstrap functions */

static int Audio_Available(void)
{
	// Audio is always available on Xenon
	return(1);
}


static void XENON_Unload(void)
{
	// do nothing
}
static int XENON_Load(void)
{	
	return 1;
}

static void Audio_DeleteDevice(SDL_AudioDevice *device)
{
	 if (device->hidden)
	 {
		 free(device->hidden);
		 device->hidden = NULL;
	 }
}

static SDL_AudioDevice *Audio_CreateDevice(int devindex)
{ 
	SDL_AudioDevice *this;

	if ( XENON_Load() < 0 ) {
		return(NULL);
	}

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		memset(this, 0, (sizeof *this));
		this->hidden = (struct SDL_PrivateAudioData *)
				malloc((sizeof *this->hidden));
	}
	if ( (this == NULL) || (this->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( this ) {
			free(this);
		}
		return(0);
	}
	memset(this->hidden, 0, (sizeof *this->hidden));

	/* Set the function pointers */

	this->OpenAudio = XENON_OpenAudio;
    	this->WaitAudio = XENON_WaitAudio_BusyWait;
    	this->PlayAudio = XENON_PlayAudio;
    	this->GetAudioBuf = XENON_GetAudioBuf;
    	this->CloseAudio = XENON_CloseAudio;

	this->free = Audio_DeleteDevice;                
                
	return this;
}

AudioBootStrap XENONAudio_bootstrap = {
	"XENON Audio", "XENON Audio SDL Driver",
	Audio_Available, Audio_CreateDevice
};

static void XENON_WaitAudio_BusyWait(_THIS)
{
	 
}

static void XENON_PlayAudio(_THIS)
{       
	memcpy(&pAudioBuffers[currentBuffer * mixlen], locked_buf, mixlen);
	
	while(xenon_sound_get_unplayed()>(4*mixlen)) udelay(50);
	
	xenon_sound_submit(&pAudioBuffers[currentBuffer * mixlen], mixlen);

	currentBuffer++;
	currentBuffer %= (NUM_BUFFERS);	       
}

static Uint8 *XENON_GetAudioBuf(_THIS)
{       
	return(locked_buf);
}

static void XENON_WaitDone(_THIS)
{
	Uint8 *stream;
        
	/* Wait for the playing chunk to finish */
	stream = this->GetAudioBuf(this);
	if ( stream != NULL ) {
	memset(stream, silence, mixlen);
	this->PlayAudio(this);
	}
	this->WaitAudio(this);

 
 
}

static void XENON_CloseAudio(_THIS)
{

	if (locked_buf)
	{
		free(locked_buf);
		locked_buf = NULL;
	}

	if (pAudioBuffers)
	{
		free(pAudioBuffers);
		pAudioBuffers = NULL;
	}
}

static int XENON_OpenAudio(_THIS, SDL_AudioSpec *spec)
{

	/* Determine the audio parameters from the AudioSpec */
	switch ( spec->format & 0xFF ) {
		case 8:
			/* Unsigned 8 bit audio data */
			spec->format = AUDIO_U8;
			silence = 0x80;			
			break;
		case 16:
			/* Signed 16 bit audio data */
			spec->format = AUDIO_S16;
			silence = 0x00;
			break;
		default:
			SDL_SetError("Unsupported audio format");
			return(-1);
	}

	/* Update the fragment size as size in bytes */
	SDL_CalculateAudioSpec(spec);
 
	locked_buf = (unsigned char *)malloc(spec->size);
	 
	/* Create the audio buffer to which we write */
	NUM_BUFFERS = -1;

	if ( NUM_BUFFERS < 0 )
		NUM_BUFFERS = 4;

	pAudioBuffers = (unsigned char *)malloc(spec->size*NUM_BUFFERS);
 	playing = 0;
	mixlen = spec->size;
        
	return(0);
}
