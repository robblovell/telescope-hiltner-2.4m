/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSDATETIME
#include <time.h>
#include "hiltner.h"
#include "const.h"
#include "timeuniv.h"
#include "debug.h"
#include "hooks.h"
#include "xtcslink.h"
#include "hiltner.h"
#include "display.h"
extern int cancelit;
extern Display display;
extern char **raspeedstatus,**decspeedstatus;
extern double domestopflag;
extern void invalidatemove ();
extern int domemoveflag,focusmoveflag,moveflag;
extern char **messages[12];/*********************************  get time  *****************************/
/*
		 Gets the time of day from the PC system clock.
 */
void get_time ()
{
		get_dates_n_times (local_time,univtime,juliandate,siderealhours,currentepoch);

		// figure out sidreal hours minutes and seconds.

		decimalhour_to_hms (siderealhours,siderealtime);
		// following is not used....
		siderealtime.y = univtime.y;
		siderealtime.mo = univtime.mo;
		siderealtime.d = univtime.d;
}
/****************************  Time Process Loop  **************************/
/*
			This process is a very fast loop which reads the encoders, calculates
		the mean location of the telescope, calculates the altitude and azimuth
		and other statistics, figures out the dome error and deadband, figures
		the software limits, figures out if the encoders are responding and
		moniters when the telescope is performing a point to point move.
 */

/***************************  Time Process  ****************************/
void TimeProcess () {
	double lastracount,lastdeccount,lastdomecount,lastfocuscount;
	int movewait=0,focusmovewait=0,domemovewait=0,completor = 0;
//clock_t tmp,starttime,stoptime;
//char buf[100];
	//clock_t start,stop,thetime;
 //	char buf[512];
	moveflag = 1;
	while (1) {

		DosSleep (5);

		//start = clock ();

		// semaphore with set encoders....
		if (DosRequestMutexSem (setencoders_mex_sem,10000L) != ERROR_TIMEOUT) {
			// read encoders
			focus_encoder.read_arcsec_location ();
			dome_encoder.read_arcsec_location ();
			domeazimuth = dome_encoder.location;
			inst_encoder.read_arcsec_location ();
			decraw = dec_encoder.read_arcsec_location ();
			haraw = ra_encoder.read_arcsec_location ();
			// software limits.
			if (inside_rawlimits (haraw,decraw)) {
				RA.limit.externallimit (0);
				DEC.limit.externallimit (0);
			}
			else {
				RA.limit.externallimit (1);
				DEC.limit.externallimit (1);
			}
			/*
					 Current location of the telescope...
			 */			raw_coordinates_to_telescope_coordinates (haraw,decraw,hatele,dectele);

			get_time ();

			telescope_coordinates_to_mean_place (hatele,dectele,
								siderealhours,stdepoch,ramean,decmean,1,0);
			set_coordinates ();
			// calculate airmass...
			calculate_airmass (airmass,targetairmass);

//sprintf (buf,"asloc:%lf loc:%lf raw:%lf fwd:%lu rev:%lu",
//dec_encoder.arcseclocation,dec_encoder.location,dec_encoder.rawlocation,dec_encoder.fwd,dec_encoder.rev);
//debug.post (buf);

#define GAP	8.0   // DFM used 10.0.
#define DOMERADIUS	216
#define DOMEHOME	320.0
			/* dome calculations... azimuth is in decimal degrees*/
			targetaltitude = altit(dectarget,hatarget,MDMLATITUDE,
														 &targetazimuth,&targetparallactic);

			if (domehomeflag && autodomeflag) { // home the dome...
				// find the contact plate...
				deadband = 0.4;
				targetband = 0.4;
				dometargetazimuth = DOMEHOME;
			}
			else {  // not homing, so see if the program must follow the telescope.
				if (!moveflag ) {
					// moving using the paddle then follow the telescope.
					// OR moving using tracking.
					dometargetaltitude = altit(dectele,hatele,MDMLATITUDE,
																 &dometargetazimuth,&dometargetparallactic);
					if (autodomeflag) {
						if (dometargetaltitude > 89.7) {
							deadband = 100000.0;
							targetband = 100000.0;
						}
						else { // dead band goes from 5.3 @ 89.6 to 0.0371 @ 5 degrees.
							deadband = ((GAP/
									 (DOMERADIUS * cos (dometargetaltitude*DEGREESTORADIANS))
												 )/
										DEGREESTORADIANS);
							// make sure deadband doesn't fall below the resolution of the
							// dome encoder.
							if (deadband < 0.2)
								deadband += 0.2;
							targetband = 0.5;
						}
					}
					else { // don't move the dome is not active.
						deadband = 100000.0; // avalue that can't be reached...
						targetband = 100000.0;
						domestopflag = 1;
					}
				}
				else { // point to point move is active.
					if (autodomeflag) {
						if (dometargetaltitude > 89.7) {
							dometargetaltitude = targetaltitude;
							dometargetazimuth = targetazimuth;							deadband = 100000.0;
							targetband = 100000.0;
						}
						else {							dometargetaltitude = targetaltitude;
							dometargetazimuth = targetazimuth;
							deadband = 0.0;
							targetband = 1.0;
						}
					}
				}
			}
			domeerror = domeazimuth-dometargetazimuth;
			while (domeerror < -180.) domeerror+= 360.;
			while (domeerror > 180.) domeerror -= 360.;
			telealtitude = altit(dectele,hatele,MDMLATITUDE,&teleazimuth,
						 &teleparallactic);

			DosReleaseMutexSem (setencoders_mex_sem);
			rastatus.update_message (&raspeedstatus);
			decstatus.update_message (&decspeedstatus);

// for detection of encoder response.
#define MOVETIMEOUT		2500     // five seconds....
#define FOCUSMOVETIMEOUT		1000     // two seconds....
#define DOMEMOVETIMEOUT		1000     // two seconds....

		 if (moveflag || RAdata.axismoveflag || DECdata.axismoveflag) {
			 movewait++;
			 if (movewait > MOVETIMEOUT) {
				 if (lastdeccount == dec_encoder.rawlocation &&
						 lastracount == ra_encoder.rawlocation)
					 *messages[5] = "RA or DEC Encoders Not Responding";

				 else if (lastracount == ra_encoder.rawlocation)
					 *messages[5] = "RA Encoder Not Responding";
				 else if (lastdeccount == dec_encoder.rawlocation)
					 *messages[5] = "DEC Encoder Not Responding";


				 movewait = 0;
			 }
			 lastracount = ra_encoder.rawlocation;
			 lastdeccount = dec_encoder.rawlocation;		 }
		 else {
			 movewait = 0;
			 lastracount = ra_encoder.rawlocation;
			 lastdeccount = dec_encoder.rawlocation;
		 }
		 if (focusmoveflag) {
			 focusmovewait++;
			 if (focusmovewait > FOCUSMOVETIMEOUT) {
				 if (lastfocuscount == focus_encoder.rawlocation)
					 *messages[5] = "FOCUS Encoder Not Responding";


				 focusmovewait = 0;
			 }
			 lastfocuscount = focus_encoder.rawlocation;
		 }
		 else {
			 focusmovewait = 0;
			 lastfocuscount = focus_encoder.rawlocation;
		 }
		 if (domemoveflag) {
			 domemovewait++;
			 if (domemovewait > DOMEMOVETIMEOUT) {
				 if (lastdomecount == dome_encoder.rawlocation)
					 *messages[5] = "DOME Encoder Not Responding";
				 domemovewait = 0;
			 }
			 lastdomecount = dome_encoder.rawlocation;
		 }
		 else {
			 domemovewait = 0;

			 lastdomecount = dome_encoder.rawlocation;
		 }		//stop = clock ();
		}
		if (!domemoveflag && !focusmoveflag && !(moveflag || RAdata.axismoveflag ||DECdata.axismoveflag))
			*messages[5] = " ";
//thetime   = start-stop;
//sprintf (buf,"%20f %f %f",(float)thetime,(float)CLK_TCK,(float)thetime/CLK_TCK);
/*
sprintf (buf,"%f meanra:%lf  meandec:%lf epoch %lf ha:%lf dec:%lf  srh:%lf %lf %lf fwd:%lu rev:%lu fwd:%lu rev:%lu",
thetime,ramean,decmean,currentepoch,hatele,dectele,siderealhours,
haraw,decraw,ra_encoder.fwd,ra_encoder.rev,
dec_encoder.fwd,dec_encoder.rev);
*/
//debug.post (buf);
		// detects end of a point to point move.
		if (RA.moveflag == 0 && DEC.moveflag == 0  &&
				RA.offsetmoveflag == 0 && DEC.offsetmoveflag == 0 && moveflag != 0)
		{
			moveflag = 0;
			if (!cancelit)
			invalidatemove ();
			*messages[1] = "Move Complete";
			ratecorr_size(hatele,dectele,MDMLATITUDE, MDMELEVATION,
					&RA.ratecorr, &DEC.ratecorr);			cancelit = 0;
			figure_trail_directions (trailrate,traillength,trailposangle,
					RAdata.axis->traillength,DECdata.axis->traillength,
					RAdata.axis->trailspeed,DECdata.axis->trailspeed);
			completor=0;

	 //	thetime = stop-start;
		}

		if (completor++ == 200) {
			*messages[1] = " ";

		}
	}
}
