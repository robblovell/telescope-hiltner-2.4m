/* -------------------------------------------------------------------
Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
------------------------------------------------------------------- */
#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "telescpe.h"
#include "hiltner.h"
#define	MAXSPEED			11

/**************************** Move Axis Process *****************************/
/*
	  This process is responsible for point to point and offset moves of the
   telescope.  Two of these processes run during execution of the program, one
   for the RA axis and the other for the DEC axis.  Modification of this routine
   effects the behavior of both axis.  Differences between the two axis are
   reflected in the values passed in the ProcessData structure parameter "data".
   This routine is an infinite loop that is only terminated upon termination of
   the program.

	The move routine is a very complicated routine and should be modified only
   with great care!!!!

   General algorithm:

	 while (1) {
	   sleep for 1/2 second.
	   if command given to move {
		while not reached target do {
		  sleep for 9 millaseconds.
		  use adaptive feed back loop to move to the target.
		}
	   }
	 }
 */
//extern int tracking;
void MoveAxisProcess (ProcessData *data)
{
	char buf[256];
	double stopresolution;
	double direction = data->encoder->arcseclocation -
									 data->axis->target;
	double distance = fabs (direction);
	double halfway = distance/2.0;
	double stepspeed = 0.0,slewspeed = 0.0,lastspeed=0.0;
	int laststepstate=OFF,stepstate=OFF,movestate=0,lastmovestate=0;
	int step_togo_direction,lastdirection,flagdirection;
	if (data->axis->which[0] == 'R') stopresolution = 0.011;
	else stopresolution = 0.23;
	while (all_run) {
		DosSleep (500);
		/*
		    Preparation in case a move is issued is to calculate the direction
		   and distance of the move.
		 */
		direction = data->encoder->arcseclocation - data->axis->target;
		if (direction < 0) {
			distance = -direction;
		}
		else {
			distance = direction;
		}
		halfway = distance/2.0;

		//sprintf (buf,"dir:%d targ:%lf  to go:%d loc:%lf  state:%d %d  %d",
		//direction,data->axis->target,distance,data->encoder->arcseclocation,
		//stepstate,movestate,data->axis->moveflag);
	 //	debug.post (buf);   //debug.dout (buf);
		/*********  small offset moves of less than 1 arc seconds. **********/
	// implemented small offset moves of less than 1 arcseconds,
//  NOT TESTED....


		/***********************  Point to Point Move  ********************/
		if (data->axis->moveflag)  { // move initiated....

			data->axis->startslew ();

			direction = data->encoder->arcseclocation - data->axis->target;
			distance = fabs (direction);


#define	SETDISTANCE	 1000
#define	SLEWSTOP		1000
#define	SLOWSLEW		7500
#define	SLOWSTOPDISTANCE	4000
#define	STOPDISTANCE	10000
#define GUIDEDISTANCE	5

			lastmovestate = movestate = OFF;
			laststepstate = stepstate = OFF;
			flagdirection = lastdirection = step_togo_direction = 1;
			slewspeed = 0.0; lastspeed = stepspeed = 0.0;
			// distance is given in arcseconds.
			while (distance > stopresolution && data->axis->moveflag) {
				direction = data->encoder->arcseclocation - data->axis->target;
				if (direction < 0) {
					distance = -direction;
				}
				else {
					distance = direction;
				}
				DosSleep (9);
//sprintf (buf,"%s. spd:%lf %lf dir:%d targ:%lf  to go:%d loc:%lf state:%d %d",
//data->axis->which,stepspeed,slewspeed, direction, data->axis->target,distance,
//data->encoder->arcseclocation, stepstate, movestate);
//debug.post (buf);  debug.dout (buf);
				/*
				    Only start slewing after the move state has changed to
				    slew.
				 */
				if (direction < 0) {
					if (movestate == SLEW)
						data->axis->slewdirection (FORWARD);
				}
				else if (direction > 0) {
					if (movestate == SLEW)
						data->axis->slewdirection (REVERSE);
				}

				/*
					 If within a small distance of the target then use the
				   stepper motor to move.
				 */
				if (distance < SETDISTANCE) {
					// steppermotor moves...
					if (direction < 0) {
						step_togo_direction = FORWARD;
					}
					else if (direction > 0) {
						step_togo_direction = REVERSE;
					}
					if (step_togo_direction != flagdirection) {
						stepstate = OFF; stepspeed = 0.0;
					}
					else if (distance < GUIDEDISTANCE) {// use set speed to glide into target.
						if (movestate == SLEW) {
							if (slewspeed != 1.0) data->axis->slew_stop ();
							slewspeed = 0.0;
						}
						stepspeed = data->axis->guidespeed;
						if (!trackingflag) stepspeed += 15.0;
						movestate = GUIDE; stepstate = GUIDE;
					}
					else  {//if (distance < SETDISTANCE) {// use slew speed to glide into target.
						if (movestate == SLEW) {
							if (slewspeed != 1.0) data->axis->slew_stop ();
							slewspeed = 0.0;
						}
						stepspeed = data->axis->setspeed;
						movestate = SET; stepstate = SET;
					}
				}
				/*
				    less than halfway, but not close yet.
				 */
				else if (distance < halfway || distance < STOPDISTANCE) { // deccelarate when within reach of destination.
				  movestate = SLEW;  stepstate = OFF; stepspeed = 0.0;
				  if (distance < SLEWSTOP) {
				    data->axis->slew_stop  (); slewspeed =  1.0; movestate = OFF;
				  }
				  else if (distance < SLOWSTOPDISTANCE) {
				    data->axis->changeslew (data->axis->moveminslewspeed);
				    slewspeed = data->axis->moveminslewspeed;
				  }
				  else if (distance < SLOWSLEW) {
				    data->axis->changeslew (data->axis->moveslowslewspeed);
				    slewspeed = data->axis->moveslowslewspeed;
				  }
				  else if (distance < STOPDISTANCE) {
				    data->axis->changeslew (data->axis->movehalfslewspeed);
				    slewspeed = data->axis->movehalfslewspeed;
				  }
				  else {
				    data->axis->changeslew (data->axis->movemaxslewspeed);
				    slewspeed = data->axis->movemaxslewspeed;
				  }
				}
				/*  greator than halfway, but not close to the target then
				   full speed ahead....
				 */
				else if (distance >= halfway) {// accelerate toward goal.
					movestate = SLEW; stepstate = OFF; stepspeed = 0.0;
					data->axis->changeslew (data->axis->movemaxslewspeed);
					slewspeed = data->axis->movemaxslewspeed;
				}

				/*  handles changes in the move state. */
				if (movestate != lastmovestate) {
					if (movestate == SLEW) data->status->slewon ();
					else if (lastmovestate == SLEW) data->status->slewoff ();
					lastmovestate = movestate;
				}
                    // stepper motor motion changes....
				if  (DosRequestMutexSem (steppermotor_mex_sem,1000L) != ERROR_TIMEOUT) {
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
				// check limits...

				data->axis->check_limits ((double)(data->encoder->location)*DEGREESTOARCSEC);
			} // while ..
			if (movestate == SLEW  && slewspeed > 0.0) {
				movestate = OFF;

				data->status->slewoff ();
				lastmovestate = movestate;
				data->axis->slew_stop ();
				slewspeed = 0.0;
			}
			data->axis->slew_stop ();
			stepstate = OFF; stepspeed = 0.0;
			if  (DosRequestMutexSem (steppermotor_mex_sem,1000L) != ERROR_TIMEOUT) {
				stepstate = OFF;
				stepspeed = 0.0;
				if (laststepstate != stepstate) {
			//if (stepstate == GUIDE || stepstate == SET) {

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
				DosReleaseMutexSem (steppermotor_mex_sem);
			}
		}

		// end of move, check for any extra movement needed....
		if (data->axis->offsetmoveflag) {
		 /*	if (distance <= stopresolution && distance > 0) {
				// always goes here when doing an offset move with a total
				// less than 1.0.

				while (!data->axis->stepmtr.move (distance,
							 data->axis->guidespeed,direction))
					;
*/
				data->axis->offsetmoveflag = 0;
			//}

		}
		data->axis->moveflag = 0;
	}


}
// NOT USED................
// This routine moves the RA and DEC axis in a point to point move.
/*
	This process waits for a signal on the bullitn board to tell it to take
	control of the telescope.  Once control is obtained, it reads a target
	location from the blackboard.  This target location is defined in units of
	encoder counts.  The process then accelertates the telescope toward the
	target location and once arriving glides the telescope into the new location.

	While the telescope is in motion, the encoders and move processes have the
	highest priority, followed by limits and serial.  All others are either
	suspended or given lower priorities.
 */
//  NOT USED.......
void MoveProcess () //Axis &axis,Switch &left,Switch &right)
{

	while (1) {
		DosSleep (500);
		if (moveflag) {   // use a semafore for this later....
		 //MovePriorities ();
		 // set speeds...?????
		 //MoveAxisProcess (&RAdata,HA_target);
		 //MoveAxisProcess (&DECdata,DEC_target);
		 //ManualPriorities ();
		}
	}
}