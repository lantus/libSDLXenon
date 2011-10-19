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

#define MAX_UNPLAYED 32768
#define BUFFER_SIZE 65536
 
static Uint32 dma_buffer[2048];
 
static char buffer[BUFFER_SIZE]; 
static unsigned int real_freq;
static double freq_ratio;
 
int buffer_size = 1024;

static unsigned int thread_lock __attribute__ ((aligned (128))) =0;

static unsigned char thread_stack[0x10000];

static volatile void * thread_buffer=NULL;
static volatile int thread_bufsize=0;
static int thread_bufmaxsize=0;
static volatile int thread_terminate=0;
 
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

static void inline play_buffer(void)
{
        int i;

        while(xenon_sound_get_unplayed()>MAX_UNPLAYED);
     
        for(i=0;i<buffer_size/4;++i) ((int*)buffer)[i]=bswap_32(((int*)buffer)[i]);
          
        xenon_sound_submit(buffer,buffer_size);
        
         
}

static s16 prevLastSample[2]={0,0};

void ResampleLinear(s16* __restrict pStereoSamples, s32 oldsamples, s16* __restrict pNewSamples, s32 newsamples)
{
        s32 newsampL, newsampR;
        s32 i;
         
        for (i = 0; i < newsamples; ++i)
        {
                s32 io = i * oldsamples;
                s32 old = io / newsamples;
                s32 rem = io - old * newsamples;

                old *= 2;
 
                if (old==0){
                        newsampL = prevLastSample[0] * (newsamples - rem) + pStereoSamples[0] * rem;
                        newsampR = prevLastSample[1] * (newsamples - rem) + pStereoSamples[1] * rem;
                }else{
                        newsampL = pStereoSamples[old-2] * (newsamples - rem) + pStereoSamples[old] * rem;
                        newsampR = pStereoSamples[old-1] * (newsamples - rem) + pStereoSamples[old+1] * rem;
                }
                
                pNewSamples[2 * i] = newsampL / newsamples;
                pNewSamples[2 * i + 1] = newsampR / newsamples;
        }

        prevLastSample[0]=pStereoSamples[oldsamples*2-2];
        prevLastSample[1]=pStereoSamples[oldsamples*2-1];
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
              
}
 
static void XENON_WaitDone(_THIS)
{
	

 
 
}

static void XENON_CloseAudio(_THIS)
{
        thread_terminate=1;
        while(xenon_is_thread_task_running(2));
	
}

static void thread_enqueue(void * buffer,int size)
{
        while(thread_bufsize);
      
        lock(&thread_lock);
        
        if(thread_bufmaxsize<size){
               thread_bufmaxsize=size;
               thread_buffer=realloc((void*)thread_buffer,thread_bufmaxsize);
        }

        thread_bufsize=size;
        memcpy((void*)thread_buffer,buffer,thread_bufsize);   
        
        unlock(&thread_lock);
       
}

static void XENON_ConvertU8(int16_t* __restrict aDest, uint8_t* __restrict aSource, uint32_t aCount)
{
        int i = 0;
        for(i = 0; i != aCount; i ++)
        {
                int32_t tack = aSource[i];
                aDest[i] = (tack * 256) - 32768;
        }
}


static void inline add_to_buffer(void* stream, unsigned int length){
        unsigned int lengthLeft = length >> 2;
        unsigned int rlengthLeft = ceil(lengthLeft / freq_ratio);
        ResampleLinear((s16 *)stream,lengthLeft,(s16 *)buffer,rlengthLeft);
        buffer_size=rlengthLeft<<2;
        play_buffer();
}

static void thread_loop()
{
        static char * local_buffer[0x10000];
        int local_bufsize=0;
        int k;

        while(!thread_terminate){
            
                short *stream = (short *)dma_buffer;
                                     
                // grab the audio
                current_audio->spec.callback(
                current_audio->spec.userdata,
                (Uint8 *)dma_buffer,
                2048);         
  
                // queue it up
                thread_enqueue((short *)stream, 2048);            
                                                                         
                lock(&thread_lock);
                              
                if (thread_bufsize){                                                   
                        local_bufsize=thread_bufsize;
                        if (local_bufsize>sizeof(local_buffer)) local_bufsize=sizeof(local_buffer);
                        memcpy(local_buffer,(void*)thread_buffer,local_bufsize);
                        thread_bufsize-=local_bufsize;
                }
                
                unlock(&thread_lock);

                if (local_bufsize){         
                        add_to_buffer(local_buffer,local_bufsize);
                        local_bufsize=0;
                }

                for(k=0;k<100;++k) asm volatile("nop");               
        }
}

static Uint8 *XENON_GetAudioBuf(_THIS)
{       
 
	return NULL;
}

static int XENON_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
        
        spec->freq      = 32000;
        spec->format    = AUDIO_S16MSB;
        spec->channels  = 2;      
        
        freq_ratio = (double)spec->freq / 48000;        
      
	/* Update the fragment size as size in bytes */
	SDL_CalculateAudioSpec(spec);
        
      
 	playing = 0;
	mixlen = spec->size;
        
        thread_lock=0;
        thread_buffer=NULL;
        thread_bufsize=0;
        thread_bufmaxsize=0;
        thread_terminate=0;

        // Semaphores/Mutexes dont exist yet in libXenon so we dont want libSDL to handle
        // the threading. Instead create our own thread here to handle the audio.
 
        xenon_run_thread_task(2,&thread_stack[sizeof(thread_stack)-0x100],thread_loop); 
                
        // 1 means that libSDL wont call SDL_CreateThread();
        
	return(1);              
}
