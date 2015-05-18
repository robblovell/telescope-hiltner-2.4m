/* -------------------------------------------------------------------
Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
------------------------------------------------------------------- */
/*Axis.cpp

			This file contains code for moving the telescope's RA or DEC
	axises from the paddle.
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


/***************************  Axis Process  ******************************/
/*
	The word "axis" stands for one of the two axis of motion, RA or DEC.
  This procedure allows the astronomer to move the telescope via the paddle.
  Two of these processes run during execution of the program, one for the
  RA axis and the other for the DEC axis.  Modification of this routine
  effects the behavior of both axis.  Differences between the two axis are
  reflected in the values passed in the ProcessData structure parameter "data".
  This routine is an infinite loop that is only terminated upon termination of
  the program.

	The move routine is a very complicated routine and should be modified only
  with great care!!!!
*/
void AxisProcess (ProcessData *data)
{
	short moving,clicks;
	double stepspeed=0.0,lastspeed=0.0;
	int slewstate=OFF,stepstate=OFF,laststepstate=OFF;
	//int tracking = (frontpanel.track_.on () == 0);
	int slewwait = 0,sleeptime = 17;

	int step_togo_direction,lastdirection,flagdirection;
	while (all_run) {
		DosSleep (sleeptime);
//debug.post ("Axis Process...");
		if (stopflag) {
			data->axis->slew_stop ();
			continue;
		}
		else if (data->axis->moveflag) {
		   // don't allow paddle moves during point to point motions.
		   sleeptime = 1000;
		   continue;
		}
		else sleeptime = 17;
//	 char buf[256];
///	 sprintf (buf,"axis:slew:%d  step:%d  trac:%d  stepspeed:%5.5lf ",
 //				 slewstate,stepstate,tracking, stepspeed);
 //	 debug.post (buf);		// if using paddle.....
		//int astate;
		//paddle.slew.read_clicks (astate);
//	 char buf[256];
//	 sprintf (buf,"%s clicks: %d %d",
//				 data->axis->which,clicks,paddle.slew.clicks);
//	 debug.post (buf);
		if ((paddle.slew.clicks ||  paddle.slew.on ()) &&
				(data->left->on() || data->right->on() )
				&& !data->axis->limit.softwarelimit/* && !trailflag*/) {
			// slew is on inside this if...
			// either East or West is pushed.
			data->axismoveflag = 1;
			// slew turned on and off here:
			if (slewwait) {
			  if (slewstate == OFF ) {
			    data->status->slewon ();
			  }
			  slewstate = ON;
			  if (data->left->on ()) {
			    data->axis->slew (FORWARD);
			  }
			  else if (data->right->on ()) {
			    data->axis->slew (REVERSE);
			  }
			}
			if (paddle.slew.on ()) slewwait=1;

      // check for clicks of slew button.


			if (clicks != paddle.slew.clicks) {
			  data->axis->slew_slowup (paddle.slew.clicks);
			  clicks = paddle.slew.clicks;
			}

		} //slew test.
		else { // both east and west not pushed with slew.
			// stop slewing if in slew state.
			clicks = 0;
			if (slewstate == ON) {
				data->axismoveflag = 0;
				slewwait = 0;
				data->status->slewoff ();
				data->axis->slew_stop ();
				slewstate = OFF;

			}
		}

		// stepper motors remain active throughout slew if tracking.
		// if not tracking, steppers go to zero.
		// guide and set speeds override tracking.
		// stepper motor states are: OFF,TRACK,GUIDE,SET.

		if (slewstate == OFF ) {
			moving = 0;
			if (data->left->on ()) {
				step_togo_direction = LEFT;	moving = 1;
			}
			else if (data->right->on ()) {
				step_togo_direction = RIGHT; moving = 1;
			}
			if (step_togo_direction != flagdirection) {
				moving = 0;
			}
			data->axismoveflag = moving;
			if (moving && !slewwait) {
				if (paddle.guide_set.on ()) {//Set speed.
					stepspeed = data->axis->setspeed; stepstate = SET;
				}
				else { // Guide speed.
					stepspeed = data->axis->guidespeed; stepstate = GUIDE;
				}
			}
			else {

				stepstate = OFF;	stepspeed = 0.0;
			}
			// stepper motor turned on and off here:
			if  (DosRequestMutexSem (steppermotor_mex_sem,10000L)
				 != ERROR_TIMEOUT) {
			  if (laststepstate != stepstate) {
					data->axis->stepmtr.stop_at_direction (
							(double) lastspeed,lastdirection);
					data->axis->stepmtr.speed_at_direction (
							(double) stepspeed,step_togo_direction);
					if (stepstate == GUIDE) data->status->guideon ();
					else if (laststepstate == GUIDE) data->status->guideoff ();

					if (stepstate == SET) data->status->seton ();
					else if (laststepstate == SET) data->status->setoff ();

					laststepstate = stepstate;
					lastspeed = stepspeed;
					lastdirection = step_togo_direction;
			  }
			  flagdirection = step_togo_direction;
			  DosReleaseMutexSem (steppermotor_mex_sem);
			}
		} // slew off.
		data->axis->check_limits ((double)(data->encoder->arcseclocation));

	}
}

