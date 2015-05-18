/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/***************************  Dome.cpp  ******************************/
/*
			This file handles the intelligence of moving the dome in response
	 to telescope motions.
 */

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "telescpe.h"
#include "hiltner.h"

#define SLOW	1000
#define SMALLDISTANCE	10.0
#define LONGMOVE		7

double speeds[300],seconds[300];
double domecoastdistance;
int domemoveflag=0;

double domestopflag = 1;
/*************************  Dome Process  *************************/
/*
		 Messages are passed from the time process in timeproc.cpp which
	 tell this process when to follow the motion of the telescope and
	 when to stay idle.
			The dome process is an adaptive feedback algorithm which learns
	 the characteristics of the dome as it moves.  Initially the routine
	 uses experimentally found values to move the dome small distances.
	 Then as the dome makes errors it corrects these experimentally found
	 values and records in two arrays: speeds and seconds.  The dome
	 process also learns what the coasting distance of the dome is after
	 long motions to particular locations.
 */
void DomeProcess ()
{
	int domestate=0,lastdomestate = 1,domedirection = OFF,
					lastdomedirection = OFF;

	char buf[256],buf1[256];
	double distance,errordist,speed,volts,second,startlocation,stoplocation;
	double startdomeerror;
	int distix;
	domemoveflag = 0;
	while (all_run) {
		DosSleep (200);
		if (stopflag || domestopflag) {
			dome.stop (); domestopflag = 0;
			lastdomestate = domestate = 0;
			domemoveflag = 0;
			continue;
		}
			/*
			sprintf (buf,"dm:%d %d %d %d %lf bnd:%lf daz:%lf taz:%lf err:%lf dr:%d alt: %lf",
			domestate,lastdomestate,domehomeflag,autodomeflag,
			dome_encoder.arcseclocation,deadband,domeazimuth,dometargetazimuth,
			domeerror,domedirection,dometargetaltitude);
			debug.post (buf);
			*/
		// if moving or the dead band is wrong then move the dome.
		if (fabs(domeerror) > deadband || moveflag) {
			// while there is an error do...
			while (fabs (domeerror) > targetband) {
				if (fabs (domeerror) < domecoastdistance) {
					if (domestate == LONGMOVE)  { // just finished a long move...
						dome.stop ();
						DosSleep (1200); // wait for dome to settle.
						domemoveflag = 0;
						if (fabs(domeerror)>targetband/4.0){ // if the coast distance is wrong...
							// slowly adapt to the a new coasting distance.
							if (startdomeerror != 0.0) {
								if (domeerror/startdomeerror < 0)
									domecoastdistance += 0.005;
								else
									domecoastdistance -= 0.005;
								if (domecoastdistance > SMALLDISTANCE)
									domecoastdistance = SMALLDISTANCE;
								else if (domecoastdistance < 2.0)
									domecoastdistance = 2.0;
							}
						}
						domestate = SLOW;
					}
					else {
						domestate = OFF;
					}
				}
				// if still have a small distance to move...
				else if (fabs (domeerror) < SMALLDISTANCE && domestate != LONGMOVE) {
					domestate = SLOW;
				}
				// have more than the coast distance to move...
				else {
					domestate = LONGMOVE;
				}
/*
			sprintf (buf,"DM:%d %d %d %d %lf bnd:%lf daz:%lf taz:%lf err:%lf dr:%d alt: %lf",
			domestate,lastdomestate,domehomeflag,autodomeflag,
			dome_encoder.arcseclocation,deadband,domeazimuth,targetazimuth,
			domeerror,domedirection,targetaltitude);		debug.post (buf);
 */
				// which way to move...
				if (domeerror > 0) {
					domedirection = CW;
				}
				else {
					domedirection = CCW;
				}
				// calculate error distance. used as an index into speeds and seconds.
				errordist = fabs (domeerror);distix = ((int)(errordist*10.0));
				// make sure not out of bounds of the arrays.  should never happen.
				if (distix < -299) distix = -299;
				else if (distix > 299) distix = 299;
				if (distix < 0) distix = 299-distix;
				startlocation = domeazimuth;
				if (errordist < SMALLDISTANCE && errordist > 0.5 &&
						domestate != LONGMOVE) {
					if (speeds[distix] <= 0.1 && seconds[distix] <= 0.1)// ||
					//	seconds[distix] > 30.0)
					{
						// empirical data used to move.  first guess....
						domemoveflag = 1;
						dome.move_distance (errordist,domedirection,speed,volts,second);
						speeds[distix] = volts;
						seconds[distix] = second;
					}
					else {
						// learned data used to move....
						domemoveflag = 1;
						dome.move_distance (speeds[distix],seconds[distix],domedirection);

					}
					domestate = SLOW; lastdomestate = SLOW;
					DosSleep (1200);  // wait for dome to settle.
					domemoveflag = 0;
					stoplocation = domeazimuth;
					distance = fabs (stoplocation-startlocation);
					if (distance < -180) distance += 360.0;
					else if (distance > 180) distance -= 360.0;

sprintf (buf1,"DM%d d%2.3lf,er%2.3lf /%2.3lf st%3.2lf st%3.2lf sc%2.3lf sd%2.3lf",
			 distix,distance,errordist,(double)(fabs (errordist)/fabs (distance)),
			 startlocation,stoplocation,seconds[distix],speeds[distix]);
					// adapt learned data to results of the move...
					if (distance != 0.0) {
			 //	if (fabs(domeerror) >= targetband && distance != 0.0) { // the error factor...
					//if ((fabs (errordist)/fabs (distance)) * seconds[distix] < 30.0) {
						seconds[distix] = (fabs(errordist)/fabs (distance)) * seconds[distix];
						 sprintf (buf,"! %2.3lf %s",seconds[distix],buf1);
						if (seconds[distix] < 0.3) {
							seconds[distix] = 0.3;
							speeds[distix] -= 0.1;
							if (speeds[distix] < 1.0) speeds[distix] = 1.0;
						}
					//}
					}
debug.post (buf);
    
				}
				else if (fabs(domeerror) >= domecoastdistance) { // long move..
					if (lastdomestate != domestate || domedirection != lastdomedirection)  {
						domestate = LONGMOVE;
						dome.move (7.0,domedirection );
						startdomeerror = domeerror;
						domemoveflag = 1;
					}
				}
				lastdomedirection = domedirection;
				lastdomestate = domestate;
				domestopflag = 1;

				DosSleep (100);
			}  //		while (fabs (domeerror) > targetband)


		} //if (fabs(domeerror) > deadband || moveflag)

	} //while (all_run)
}