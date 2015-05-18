#include "dtboard.h"
#include "portbit.h"
#include <time.h>
#define	CLICKTIME	250
		unsigned short PushSwitch::read_state ()
		{
		//	char buf[256];
		//	sprintf (buf,"s:%d ns:%d nns:%d  str:%d  stp:%d c:%d",
		//				state,nextstate,nextnextstate,starttime,stoptime,clicks);
		//	debug.post (buf);
			nextnextstate = get_bit ();
			if  (nextnextstate == activestate) {
				nextstate = activestate;
				if (stoptime-starttime > CLICKTIME) clickflag = 1;
			}
			if (nextnextstate != activestate && nextstate == activestate) {
				starttime = clock ();
				nextstate = !activestate;
			}
			stoptime = clock ();
			if (stoptime-starttime > CLICKTIME && state == activestate &&
        nextstate != activestate) {
				state = !activestate; clicks = 0;
			}
			else if (stoptime-starttime <= CLICKTIME && state == activestate &&
				 nextstate == activestate && clickflag) {
				clicks++;  clickflag = 0;
			}
			else if (/*stoptime-starttime > CLICKTIME &&*/ nextstate == activestate)
				state = activestate;

/*
			if (nextstate != activestate && lastnextstate == activestate) {
				 starttime = clock (); state = activestate;
			}
			else if (nextstate != activestate && state == activestate) {
				stoptime = clock ();
				if (stoptime-starttime > CLICKTIME) { // wait..
					state = activestate==FALSE; clicks = 0;
				}
				else { state = activestate; }			}

			else if (nextstate == activestate && lastnextstate != activestate &&
								state != activestate) {
				state = activestate;
			}

			else if (nextstate == activestate && lastnextstate != activestate &&
								state == activestate) {
				clicks++; state = activestate;
			}
			else if (nextstate == 1 & lastnextstate == activestate) {
				state = activestate;
			}
			laststate = state;
			lastnextstate = nextstate;
			return (state == activestate);
}
*/
/*
			// transition from active to non active state.
			if (state!=activestate && laststate== activestate && clickflag) {
				stoptime = clock ();  // waiting...
				if (stoptime-starttime < CLICKTIME) { // wait..
					state = activestate; clickflag = 1;
				}
				else { // go non-active...
					clickflag = 0;
					starttime = clock ();
				}
			}			else if (state!=activestate && laststate== activestate) {
				 starttime = clock ();
				 // wait a little before actually changing states.
				 state = activestate; clickflag = 1;
			}
			// this state and last state not active.
			else if (state != activestate && laststate != activestate) {
				 stoptime = clock ();
				 if (stoptime-starttime > CLICKTIME) {
					 clicks = 0;
				 }
			}
			// transition from active to non active.
			else if (state != activestate && laststate == activestate) {
				 stoptime = clock ();
				 if (stoptime-starttime > CLICKTIME) {
					 clicks++; clickflag = 0;
				 }			}

*/
			laststate = state;
			return (state == activestate);
		}

/*
			if (clickflag) {  // on, check for clicks....
				lastmillasec = millasec;
				millasec = clock ();
				elapsed += (double)(millasec-lastmillasec)/(CLK_TCK)*1000.0;
				if (state == 0 ) {
					if (elapsed > CLICKTIME) { // transition to off.
						clicks = 0; clickflag = 0; state = 0;
					}
				}
				else if (state == 1) {
					if (elapsed < CLICKTIME) {  // caught a click. first time is transition to on.
						clicks += 1;
					}
				}

			}
			else if (state == 1 && !clickflag) { // tranition from off to on.
				clickflag = 1;
				clicks = 0;
			}
			else clicks = 0;  // state=0 and clickflag =0  off state

			return clicks;

		}
*/