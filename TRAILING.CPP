
/* -------------------------------------------------------------------
Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
------------------------------------------------------------------- */
/*
	Trailing.cpp

			This file contains code for moving the telescope's RA or DEC
	axises when a trail command is issued from XTCS.
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
PSZ trailsem="trailsem";
HEV trailevent;
int trailpost = 0;
/*************************  trail  init  ***************************/
/*
		 Trail_init initializes the semaphore that the trail process uses
	 coordinate activity of the DEC and RA.
 */
void trail_init ()
{
}
/*
		 trail process moves an axis back and forth in a given timing and speed.
	 The end of each turn around, (change in direction) all the trail processes
	 must coordinate to start at the same time.
 */
int coordination,gcoordination,trailmode=0;
void trail_process ()
{
	int decdirection,radirection,flopper=0;
	double radistance,decdistance,raspeed,decspeed;
	clock_t starttime,stoptime;
	char buf[256];
	while (all_run) {
		DosSleep (17);

		if (trailflag) {
			if (RA.traillength < 0) { 				radirection = -1;  			}
			else {				radirection = 1; 			}
			if (radirection == 0) radirection = 1;
			if (DEC.traillength < 0) { 				decdirection = -1;  			}
			else {				decdirection = 1; 			}
			if (decdirection == 0) decdirection = 1;

			RAdata.status->trailoff ();
			DECdata.status->trailoff ();
			while ((trailflag || flopper) && all_run) {
				DosSleep (50);
				flopper = 0;
				if (trailmode)	flopper = flopper == 0;   // to return to origin
				if (moveflag) {   // no movement during point to point move.
					DosSleep (501);
					RAdata.status->trailoff ();
					DECdata.status->trailoff ();
					continue;
				}
				RAdata.status->trailon ();
				DECdata.status->trailon ();
				raspeed = RA.trailspeed;
				radistance = RA.traillength;
				decspeed = DEC.trailspeed;
				decdistance = DEC.traillength;

				double rafsecs = fabs (radistance/raspeed * CLK_TCK);
				double decfsecs = fabs (decdistance/decspeed * CLK_TCK);
				// the unit of time_t is seconds.
				double fsecs;
				if (decfsecs == 0) fsecs = rafsecs;
				else if (rafsecs == 0) fsecs = decfsecs;
				else fsecs = (rafsecs+decfsecs)/2.0;
				if (rafsecs == 0 && decfsecs == 0) break;
sprintf (buf,"%lf %lf RA: dst:%3.3lf spd:%3.3lf sec:%3.5lf DEC: dst:%3.3lf spd:%3.3lf sec:%3.5lf",
					CLK_TCK,fsecs,radistance,raspeed,rafsecs,decdistance,decspeed,decfsecs);
debug.post (buf);
 //		WinStopTimer (hab,hWndClient,1); //,200);
//DosEnterCritSec ();
				if (DosRequestMutexSem(steppermotor_mex_sem,10000L) != ERROR_TIMEOUT) {
					DosEnterCritSec (); DosSleep (1);
						RA.stepmtr.speed_at_direction (raspeed,radirection);
						DEC.stepmtr.speed_at_direction (decspeed,decdirection);
					DosExitCritSec ();
					DosReleaseMutexSem (steppermotor_mex_sem);
				}			 // wait for the telescope to move the prescribed amount.

				starttime=clock ();
				stoptime = clock ();

				do {
					stoptime = clock ();

					//DosSleep (3);
				}
				while ((abs (stoptime-starttime) < (clock_t)fsecs) && trailflag);// take out && trailflag for return to origin.
				if (DosRequestMutexSem(steppermotor_mex_sem,10000L) != ERROR_TIMEOUT) {
					DosEnterCritSec (); DosSleep (1);

						RA.stepmtr.stop_at_direction (raspeed,radirection);
						DEC.stepmtr.stop_at_direction (decspeed,decdirection);
					DosExitCritSec ();

					DosReleaseMutexSem (steppermotor_mex_sem);
				}
				radirection *= -1; decdirection *= -1;
	 //			DosExitCritSec ();
 //		WinStartTimer (hab,hWndClient,1,200);
			}
		}
		else { // if trailflag.
			RAdata.status->trailoff ();
			DECdata.status->trailoff ();
		}
	}

}
/************************ figure trail directions *********************/
/*
			Given the trail rate, length, and position angle calculate the
	 distance each axis must move to produce the given length and direction.
	 each axis will move at a different rate of speed so that they both
	 arrive at the end of the trail at the same time.
 */
void figure_trail_directions (double &rate,double length,double posangle,
				 double &ra,double &dec,double &rarate,double &decrate)
{
	posangle *= DEGREESTORADIANS;
	dec = length * cos (posangle);
	ra = length *sin (posangle) /cos(dectele*DEGREESTORADIANS);

	decrate = fabs (rate * cos (posangle));
	if (decrate > 80.0) {
		decrate = 80.0;
		rate = fabs (decrate/cos(posangle));
	}
	if (decrate > 0.01) {
		decrate = DEC.stepmtr.estimate_speed (decrate);
		rate = fabs (decrate/cos(posangle));
	}
	rarate = fabs (rate * sin (posangle) /cos(dectele*DEGREESTORADIANS));
	if (rarate > 80.0) {
		rarate = 80.0;
		rate = fabs ((rarate*cos(dectele*DEGREESTORADIANS))/sin(posangle));
		decrate = fabs (rate * cos (posangle));
	}

}
