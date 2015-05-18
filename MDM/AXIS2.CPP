#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>


#include "telescpe.h"
#include "about.h"
#include "display.h"
#include "hiltner.h"



// the word "axis" stands for one of the two axis of motion, RA or DEC.
void AxisProcess (ProcessData *data) //Axis &axis,Switch &left,Switch &right)
{
	short moving;
	static unsigned short stepspeed;
	static unsigned short slewstate,clutchstate,stepstate,laststepstate;
	while (1) {
		// if using paddle.....
		if (paddle.slew.on () && (data->left.on() || data->right.on())) {//(paddle.E.on() || paddle.W.on())) {
			// slew is on inside this if...
			// either East or West is pushed.
			if (slewstate == OFF) { // transition from not slewing.
				slewstate = ON;
			}
			if (clutchstate == OFF) {
				data->axis.dcmtr.speed (0.0);
				data->axis.clutch.turn_on ();
				clutchstate = ON;
			}

			else { // wait a cycle before actually moving.
				if (data->left.on ()) {
					if (moving == RIGHT) data->axis.dcmtr.speed (0.0);
					moving = LEFT;
					data->axis.dcmtr.set_direction (LEFT);
					data->axis.slewenable.on ();
					data->axis.dcmtr.speed ((double) data->axis.slewspeed);
				}
				else if (data->right.on ()) {
					if (moving == LEFT) data->axis.dcmtr.speed (0.0);
					moving = RIGHT;
					data->axis.dcmtr.set_direction (RIGHT);
					data->axis.slewenable.on ();
					data->axis.dcmtr.speed ((double) data->axis.slewspeed);
				}
			} // clutch test.
		} //slew test.
		else { // both east and west not pushed with slew.
			// stop slewing if in slew state.
			if (slewstate == ON) {
				slewstate = OFF;
				moving = 0;
				data->axis.dcmtr.speed ((double) 0.0);
				data->axis.slewenable.off ();
				DosSleep (500);
				if (clutchstate==ON) {
					data->axis.dcmtr.speed ((double) 0.0);
					data->axis.slewenable.off ();
					data->axis.clutch.off ();
					clutchstate = OFF;
				}
			}
		}
		// stepper motors remain active throughout slew if tracking.
		// if not tracking, steppers go to zero.
		// guide and set speeds override tracking.
		// stepper motor states are: OFF,TRACK,GUIDE,SET.
		if (slewstate == OFF) {
			laststepstate = stepstate;
			if (data->left.on () || data->right.on ()) { //paddle.E.on () || paddle.W.on () ){
				if (paddle.guide_set.on ()) {//Set speed.
					stepspeed = data->axis.setspeed;
					stepstate = SET;
				}
				else { // Guide speed.
					stepspeed = data->axis.guidespeed;
					stepstate = GUIDE;
				}
			}
			else if (frontpanel.track_.on ()) {// no paddle activity. possible slew.
				stepspeed = data->axis.trackspeed;
				stepstate = TRACK;
			}
			else {
				stepspeed = 0;
				stepstate = OFF;
			}
			if (laststepstate != stepstate)
				data->axis.stepmtr.speed ((double) stepspeed);
			if (data->left.on () ) {
				data->axis.stepmtr.direction (LEFT);
			}
			else if (data->right.on () ) {
				data->axis.stepmtr.direction (RIGHT);
			}
			else {
				// ? RA.step.direction (RIGHT); // track is always west?
			}		} // slew off.

		DosSleep (50);
	}
}