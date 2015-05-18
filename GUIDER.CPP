/* -------------------------------------------------------------------
Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
------------------------------------------------------------------- */
/*
															Guider.cpp

			This file contains code for moving the telescope's RA or DEC
	axises by way of the auto guider.
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
/**************************  Guider Axis Process  **********************/
/*
				The guider process controls one axis, either RA or DEC in response
	 to guider button pushes by the guider computer.  Guider button pushes are
	 read by the paddleguider process in frontpanel.cpp.  The code here is
	 identical to the paddle process code only with a different fast speed
	 value.
 */
void GuiderAxisProcess (ProcessData *data)
{
	short moving;
	double stepspeed=0.0,lastspeed=0.0;
	int stepstate=OFF,laststepstate=OFF;

	int sleeptime = 17;

	int step_togo_direction,lastdirection,flagdirection;
	while (1) {
		DosSleep (sleeptime);
		if (stopflag) {
			sleeptime = 1000;
			continue;
		}
		else if (moveflag) {
			 sleeptime = 1000;
			 continue;
		}
		else sleeptime = 50;

		if (autoguideflag && !(data->guideleft->state==1 && data->guideright->state ==1)) {
			sleeptime = 9;
			moving = 0;
			if (data->guideleft->off ()) {
				step_togo_direction = LEFT;	moving = 1;
			}
			else if (data->guideright->off ()) {
				step_togo_direction = RIGHT; moving = 1;
			}
			if (step_togo_direction != flagdirection) {
				moving = 0;
			}
			if (moving) {
				if (guider.guide_fast.off ()) {//fast speed.
					stepspeed = data->axis->fastspeed; stepstate = FAST;
				}
				else { // Guide speed.
					stepspeed = data->axis->guidespeed; stepstate = GUIDE;
				}
			}
			else {
				stepstate = OFF;	stepspeed = 0.0;
			}
			if (laststepstate != stepstate) {
				data->axis->stepmtr.stop_at_direction (
							(double) lastspeed,lastdirection);
				data->axis->stepmtr.speed_at_direction (
							(double) stepspeed,step_togo_direction);
					if (laststepstate == GUIDE) data->status->guideoff ();
					else if (laststepstate == FAST) data->status->fastoff ();
					if (stepstate == GUIDE) data->status->guideon ();
					else if (stepstate == FAST) data->status->faston ();				laststepstate = stepstate;
				lastspeed = stepspeed;
				lastdirection = step_togo_direction;
			}
			flagdirection = step_togo_direction;
		}
	}
}