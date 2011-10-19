#include <input/input.h>
#include <usb/usbmain.h>

#include <math.h>
#include <stdio.h>		/* For the definition of NULL */
 
#include "SDL_error.h"
#include "SDL_joystick.h"
#include "SDL_sysjoystick.h"
#include "SDL_joystick_c.h"
#include "SDL_events.h"

#define XBINPUT_DEADZONE 0.24f
#define AXIS_MIN -32768         /* minimum value for axis coordinate */
#define AXIS_MAX 32767          /* maximum value for axis coordinate */
#define MAX_AXES 4              /* each joystick can have up to 5 axes */
#define MAX_BUTTONS 12          /* and 12 buttons */
#define MAX_HATS 2

char joyName[50];

/* The private structure used to keep track of a joystick */
struct joystick_hwdata
{ 
        struct controller_data_s curpad;      

};

int SDL_SYS_JoystickInit(void)
{
	SDL_numjoysticks = 4;
	return(SDL_numjoysticks);
}

/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
	
	sprintf(joyName,"XENON Gamepad %ld",index);
	return(joyName);
}

int SDL_SYS_JoystickOpen(SDL_Joystick *joystick)
{
	
         /* allocate memory for system specific hardware data */
        joystick->hwdata = (struct joystick_hwdata *) SDL_malloc(sizeof(*joystick->hwdata));
        
        if (joystick->hwdata == NULL)
        {
                SDL_OutOfMemory();
                return(-1);
        }
        
        SDL_memset(joystick->hwdata, 0, sizeof(*joystick->hwdata));

        /* fill nbuttons, naxes, and nhats fields */
        joystick->nbuttons = MAX_BUTTONS;
        joystick->naxes = MAX_AXES;
        joystick->nhats = MAX_HATS;
        joystick->name  = "XENON SDL Gamepad";
        return(0);
}

void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{        
        static int prev_buttons[4] = {0};
        static Sint16 nX = 0, nY = 0;
        static Sint16 nXR = 0, nYR = 0;
        unsigned long b=0; 
        int hat=0, changed=0;

        /* Theres a bug with the current libXenon controller implementation
           that sometimes causes analog values to 'stick' and retain the same x/y values
           after release back to origin.
           */
        
        usb_do_poll();
        
        get_controller_data(&joystick->hwdata->curpad, joystick->index);
    
        if (joystick->hwdata->curpad.a)
        {
                if (!joystick->buttons[0])
                        SDL_PrivateJoystickButton(joystick, (Uint8)0, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[0])
                        SDL_PrivateJoystickButton(joystick, (Uint8)0, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.b)
        {
                if (!joystick->buttons[1])
                        SDL_PrivateJoystickButton(joystick, (Uint8)1, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[1])
                        SDL_PrivateJoystickButton(joystick, (Uint8)1, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.x)
        {
                if (!joystick->buttons[2])
                        SDL_PrivateJoystickButton(joystick, (Uint8)2, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[2])
                        SDL_PrivateJoystickButton(joystick, (Uint8)2, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.y)
        {
                if (!joystick->buttons[3])
                        SDL_PrivateJoystickButton(joystick, (Uint8)3, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[3])
                        SDL_PrivateJoystickButton(joystick, (Uint8)3, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.lb)
        {
                if (!joystick->buttons[4])
                        SDL_PrivateJoystickButton(joystick, (Uint8)4, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[4])
                        SDL_PrivateJoystickButton(joystick, (Uint8)4, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.rb)
        {
                if (!joystick->buttons[5])
                        SDL_PrivateJoystickButton(joystick, (Uint8)5, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[5])
                        SDL_PrivateJoystickButton(joystick, (Uint8)5, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.s1_z)
        {
                if (!joystick->buttons[6])
                        SDL_PrivateJoystickButton(joystick, (Uint8)6, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[6])
                        SDL_PrivateJoystickButton(joystick, (Uint8)6, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.s2_z)
        {
                if (!joystick->buttons[7])
                        SDL_PrivateJoystickButton(joystick, (Uint8)7, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[7])
                        SDL_PrivateJoystickButton(joystick, (Uint8)7, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.start)
        {
                if (!joystick->buttons[8])
                        SDL_PrivateJoystickButton(joystick, (Uint8)8, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[8])
                        SDL_PrivateJoystickButton(joystick, (Uint8)8, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.select)
        {
                if (!joystick->buttons[9])
                        SDL_PrivateJoystickButton(joystick, (Uint8)9, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[9])
                        SDL_PrivateJoystickButton(joystick, (Uint8)9, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.lt > 200)
        {
                if (!joystick->buttons[10])
                        SDL_PrivateJoystickButton(joystick, (Uint8)10, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[10])
                        SDL_PrivateJoystickButton(joystick, (Uint8)10, SDL_RELEASED);
        }

        if (joystick->hwdata->curpad.rt > 200)
        {
                if (!joystick->buttons[11])
                        SDL_PrivateJoystickButton(joystick, (Uint8)11, SDL_PRESSED);
        }
        else
        {
                if (joystick->buttons[11])
                        SDL_PrivateJoystickButton(joystick, (Uint8)11, SDL_RELEASED);
        }


        // do the HATS baby

        hat = SDL_HAT_CENTERED;
        
        if (joystick->hwdata->curpad.down)
                hat|=SDL_HAT_DOWN;
        if (joystick->hwdata->curpad.up)
                hat|=SDL_HAT_UP;
        if (joystick->hwdata->curpad.left)
                hat|=SDL_HAT_LEFT;
        if (joystick->hwdata->curpad.right)
                hat|=SDL_HAT_RIGHT;


        changed = hat^prev_buttons[joystick->index];

        if ( changed ) {
                SDL_PrivateJoystickHat(joystick, 0, hat);
        }

        prev_buttons[joystick->index] = hat;

        // Axis - LStick

        if ((joystick->hwdata->curpad.s1_x <= -14000) ||
                (joystick->hwdata->curpad.s1_x >= 14000))
        {
                if (joystick->hwdata->curpad.s1_x < 0)
                        joystick->hwdata->curpad.s1_x++;
                nX = ((Sint16)joystick->hwdata->curpad.s1_x);
        }
        else
                nX = 0;

        if ( nX != joystick->axes[0] )
                SDL_PrivateJoystickAxis(joystick, (Uint8)0, (Sint16)nX);


        if ((joystick->hwdata->curpad.s1_y <= -14000) ||
                (joystick->hwdata->curpad.s1_y >= 14000))
        {
                if (joystick->hwdata->curpad.s1_y < 0)
                        joystick->hwdata->curpad.s1_y++;
                nY = -((Sint16)(joystick->hwdata->curpad.s1_y));
        }
        else
                nY = 0;

        if ( nY != joystick->axes[1] )
                SDL_PrivateJoystickAxis(joystick, (Uint8)1, (Sint16)nY);


        // Axis - RStick

        if ((joystick->hwdata->curpad.s2_x <= -14000) ||
                (joystick->hwdata->curpad.s2_x >= 14000))
        {
                if (joystick->hwdata->curpad.s2_x < 0)
                        joystick->hwdata->curpad.s2_x++;
                nXR = ((Sint16)joystick->hwdata->curpad.s2_x);
        }
        else
                nXR = 0;

        if ( nXR != joystick->axes[2] )
                SDL_PrivateJoystickAxis(joystick, (Uint8)2, (Sint16)nXR);


        if ((joystick->hwdata->curpad.s2_y <= -14000) ||
                (joystick->hwdata->curpad.s2_y >= 14000))
        {
                if (joystick->hwdata->curpad.s2_y < 0)
                        joystick->hwdata->curpad.s2_y++;
                nYR = -((Sint16)joystick->hwdata->curpad.s2_y);
        }
        else
                nYR = 0;

        if ( nYR != joystick->axes[3] )
                SDL_PrivateJoystickAxis(joystick, (Uint8)3, (Sint16)nYR);    
        
}

void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
	 if (joystick->hwdata != NULL) {
                /* free system specific hardware data */
                SDL_free(joystick->hwdata);
                joystick->hwdata = NULL;
         }
         
         return;
}

void SDL_SYS_JoystickQuit(void)
{

    
}
