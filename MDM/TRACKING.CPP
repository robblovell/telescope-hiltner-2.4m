/* -------------------------------------------------------------------
Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
------------------------------------------------------------------- */
/*
   Tracking.cpp

	  This file contains code for moving the telescope's RA or DEC
  axises via the tracking switch.
*/

#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>


#include "telescpe.h"

#include "hiltner.h"
#include "debug.h"

#include "hooks.h"
#include "spdstat.h"
/*

    Tracking process regulates the tracking of the telescope.  Note that
	tracking is active during paddle moves as well as point to point moves.
	This means that the speed of the stepper motor is added to the current
	speed as assigned by paddle and point to point moves.  In addition at
	any point the tracking speed may be changed or turned off due to a switch
	change or limit condition.
 */
void trackingprocess (ProcessData *data)
{
	double trackspeed=0.0;
	int tracking,turnon=0;

	while (1) {
		DosSleep (73);
		if (data->axis->limit.softwarelimit)
	    tracking = 0;
		else
	    tracking = trackingflag;
		 char buf[256];
	/*	sprintf (buf,"trackspeed: %lf",trackspeed);
	 debug.post (buf);     */
	  // change in speed with rate correction on.
		if ((trackspeed > (-(data->axis->trackspeed+data->axis->ratecorr)+0.0001)
				|| trackspeed < (-(data->axis->trackspeed+data->axis->ratecorr)-0.0001))
				&&
				tracking && ratecorflag) {

			double ratecorrit = data->axis->ratecorr;
			if  (DosRequestMutexSem (steppermotor_mex_sem,10L)!=ERROR_TIMEOUT) {
			 //	data->axis->stepmtr.stop_at_direction (trackspeed,0);
		 sprintf (buf,"%s rate corr trackspeed %lf  new %lf ",data->axis->which,
						trackspeed,
					 data->axis->trackspeed+ratecorrit);
			debug.post (buf);
				if (data->axis->stepmtr.change_speed_at_direction
				 (trackspeed,0,-( data->axis->trackspeed+ratecorrit),0) != 0.0)
					trackspeed = -(data->axis->trackspeed+ratecorrit);
				else
					trackspeed = 0.0;
				DosReleaseMutexSem (steppermotor_mex_sem);
			}
			if (trackspeed != 0.0 && turnon == 0) {
				data->status->trackon ();
				turnon = 1;
			}
			if (tracking && trackspeed == 0 && turnon == 1) {
				data->status->trackoff ();
				turnon = 0;
			}
		}

		// change of speed with rate correction off.
		if ((trackspeed != -(data->axis->trackspeed)/*+0.0001)
				|| trackspeed < -(data->axis->trackspeed-0.0001)*/)
				&& tracking && !ratecorflag) {			if (DosRequestMutexSem(steppermotor_mex_sem,10L) != ERROR_TIMEOUT) {
			 //	data->axis->stepmtr.stop_at_direction (trackspeed,0);
		 sprintf (buf,"%s normal trackspeed %lf  new %lf ",data->axis->which,trackspeed,
					 data->axis->trackspeed);
			debug.post (buf);
			if (data->axis->stepmtr.change_speed_at_direction
											(trackspeed,0,-data->axis->trackspeed,0) != 0.0)
				trackspeed = -data->axis->trackspeed;
			else
				trackspeed = 0.0;
				DosReleaseMutexSem (steppermotor_mex_sem);
			}
			if (trackspeed != 0.0 && turnon ==0) {
				data->status->trackon ();
				turnon = 1;
			}
			if (tracking && trackspeed == 0 && turnon == 1) {
				data->status->trackoff ();
				turnon = 0;
			}
	  }
	  // tracking is turned off or on.
		else if (!tracking && trackspeed != 0) {
			if (DosRequestMutexSem(steppermotor_mex_sem,1000L) != ERROR_TIMEOUT) {
		 sprintf (buf,"%s stop trackspeed %lf  new 0.0 ",data->axis->which,trackspeed);
			debug.post (buf);
				data->axis->stepmtr.stop_at_direction (trackspeed,0);
				data->axis->stepmtr.speed_at_direction (0,0);
				trackspeed = 0;
				DosReleaseMutexSem (steppermotor_mex_sem);
			}
			if (turnon == 1)  {
				data->status->trackoff ();
				turnon = 0;
			}
	  }
	}
}