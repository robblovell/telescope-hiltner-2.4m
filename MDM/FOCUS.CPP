/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/******************************  Focus.cpp  ***************************/
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>


#include "telescpe.h"


#include "hiltner.h"
#include "debug.h"
int focusmoveflag=0;
int autofocusmoveflag=0;
double focustarget;/***************************  Focus Process  ******************************/
/*
			The focus process contains routines which move the focus given the
	 state of the hand paddle push buttons.
 */
void FocusProcess () {
	int focusstate=0,laststepstate=0,stepstate=0,fstopflag=1;

	focus.stop ();
	fstopflag = 0;
	while (all_run) {
		DosSleep (111);

		if (fstopflag) {
			fstopflag = 0;
			if (!autofocusmoveflag) focus.stop ();
			continue;
		}
		if (autofocusmoveflag) {
			if ( paddle.focusin.on () || paddle.focusout.on ()) {
				autofocusmoveflag = 0;
			}
			continue;
		}
 /*
	 char buf[256];
	 sprintf (buf,"focus: state:%d  speed:%d  %d  %d %d",focusstate,stepstate,fstopflag,paddle.guide_set.on (),paddle.guide_set.off ());
	 debug.post (buf);
*/

	 // if focus button is pushed....
	 if ( paddle.focusin.on () || paddle.focusout.on ()) {
				focusmoveflag = 1;
				if (paddle.guide_set.on () && stepstate != SET) {//Set speed.
					focus.stepspeed = focus.fastspeed;//focus.fastspeed;
#define SETPREP	128
					if (stepstate == SETPREP) {
						stepstate = SET;
						focus.set_fast ();
					}
					else {
						stepstate = SETPREP;
						focus.set_guide ();
					}

				}
				else if (paddle.guide_set.off () && stepstate != GUIDE) { // Guide speed.
					focus.stepspeed = focus.guidespeed;//cus.guidespeed;
					stepstate = GUIDE;
					focus.set_guide ();
				}

				fstopflag = 0;
		}
		else {
				focus.stepspeed = 0;
				focusstate = OFF;
				stepstate = 0;
				focus.set_guide ();
				fstopflag = 1;
				focusmoveflag = 0;
				continue;
		}

		if (paddle.focusin.off () && paddle.focusout.off() && focusstate != OFF) {
				 focusstate = OFF; fstopflag = 1;
		}
		else if (((paddle.focusout.on () && focusstate != FOCUSOUT )
					 || (stepstate != laststepstate && paddle.focusout.on ()))
					&& (stepstate == GUIDE || stepstate == SET || stepstate == SETPREP))  {
				focus.focusout ();
				focusstate = FOCUSOUT;
		}
		else if (((paddle.focusin.on () && focusstate != FOCUSIN )
							 || (stepstate != laststepstate && paddle.focusin.on ()))
					&& (stepstate == GUIDE || stepstate == SET || stepstate == SETPREP)) {
				focus.focusin ();
				focusstate = FOCUSIN;
		}

		laststepstate = stepstate;

	}
}
/**********************  Auto Focus Move Routine.  *******************/
void focus_absolute_move ()
{
	char buf[256];
	int autofocusmoving=0,focusstate = STOP,stepstate=STOP,
			lastfocusstate = STOP,laststepstate=STOP;
	int sleeptime=113;
	double focusdirection,focusdistance;
	while (all_run) {
		DosSleep (sleeptime);

		if (!autofocusmoveflag) {
			if (autofocusmoving) {
				autofocusmoving = 0;
				laststepstate=stepstate = STOP;
				lastfocusstate=focusstate = STOP;
				focus.stop ();
			}
			sleeptime = 113;			continue;
		}
		if ( frontpanel.drivesonoff_.off ()) {
			autofocusmoveflag = 0;
			continue;
		}
		sleeptime = 50;
//autofocusmoveflag = 0;
//continue;
		// take care of paddle movements by disabling if we haven't started moving.
		if (focusmoveflag && !autofocusmoving) {
			focusmoveflag = 0;
			focus.stop ();
			DosSleep (100);
		}
		if (focustarget != focus_encoder.location) {
			autofocusmoving = 1;
			focusdirection = focustarget-focus_encoder.location;
			focusdistance = fabs (focusdirection);
	 sprintf (buf,"move focus by: %lf to:%lf",focusdirection,focustarget);
	 debug.post (buf);
#define FOCUSSETDISTANCE	30

			if (focusdistance > FOCUSSETDISTANCE) {
				stepstate = SET;
				if (laststepstate != SET)
					focus.set_fast ();			}
			else {
				stepstate = GUIDE;
				if (laststepstate != GUIDE)
					focus.set_guide ();
			}
			if (laststepstate != stepstate) {
				if (focusdirection < 0) {
					focusstate = FOCUSOUT;
					if (focusstate != lastfocusstate) focus.focusout ();

				}
				else {
					focusstate = FOCUSIN;
					if (focusstate != lastfocusstate) focus.focusin ();				}
			}

			laststepstate = stepstate;
			lastfocusstate = focusstate;
		}
		else {
			autofocusmoveflag = 0;
			autofocusmoving = 0;
			laststepstate=stepstate = STOP;
			lastfocusstate=focusstate = STOP;
			focus.stop ();
		}
	}
}