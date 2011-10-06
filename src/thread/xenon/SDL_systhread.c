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
#include "SDL_config.h"

/* Thread management routines for SDL */

#include <xenon_soc/xenon_power.h>
#include <ppc/atomic.h>
#include <xetypes.h>
#include <ppc/register.h>
#include <ppc/xenonsprs.h>

#include "SDL_thread.h"
#include "SDL_systhread_c.h"
#include "../SDL_thread_c.h"
#include "../SDL_systhread.h"

typedef struct thread_s{
    unsigned int lock  __attribute__ ((aligned (128)));
    unsigned int states;
    int (*func)(void);
}thread_t;

static thread_t thread_states[5];
static unsigned char stack[5 * 0x1000];

static volatile int hardwareThread = 1;

void RunThread(void *data)
{	
	SDL_RunThread(data);
	return;
}


int SDL_SYS_CreateThread(SDL_Thread *thread, void *args)
{		
	printf("Creating Thread\n");
	thread->handle = hardwareThread;
	thread->threadid = hardwareThread;

	xenon_run_thread_task(hardwareThread,stack + (hardwareThread * 0x1000) - 0x100, RunThread);

	printf("Created Thread\n");

	hardwareThread++;

	if (hardwareThread >=6)
		hardwareThread = 1;

	return 0;
}

void SDL_SYS_SetupThread(void)
{
	return;
}

Uint32 SDL_ThreadID(void)
{
	return mfspr(pir);
}

void SDL_SYS_WaitThread(SDL_Thread *thread)
{

	int wait = 1;
    	
	while(wait){
		lock(&thread_states[thread->threadid].lock);
		if(thread_states[thread->threadid].states==0){
			wait = 0;
		}
	        unlock(&thread_states[thread->threadid].lock);
	}
   
	return;
}

void SDL_SYS_KillThread(SDL_Thread *thread)
{

	// put thread to sleep
	xenon_sleep_thread(thread->threadid);

	// release states on thread
	lock(&thread_states[thread->threadid].lock);
	thread_states[thread->threadid].states = 0;
	thread_states[thread->threadid].func = NULL;
	unlock(&thread_states[thread->threadid].lock);

 
}


