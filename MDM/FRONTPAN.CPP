#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "xtcslink.h"

#include "telescpe.h"
#include "about.h"
#include "display.h"
#include "hiltner.h"
#include "skycalc.h"
#include "timeuniv.h"
#include "values.h"
#include "hooks.h"
#include "spdstat.h"
#define STATUSX	200
#define STATUSY 150
#define PADDLEX 30
#define PADDLEY	150
#define ENCODERX	10
#define ENCODERY 300
#define SEP	14
extern Display display;
extern int axismoveflag,focusmoveflag;
char **drivestatus = (char **)new (char *);
char **trackingstatus = (char **)new (char *);
char **modelstatus = (char **)new (char *);
char **initstatus = (char **)new (char *);
char **guidestatus = (char **)new (char *);
char **domestatus = (char **)new (char *);
char **domehomestatus = (char **)new (char *);
char **cosdecstatus = (char **)new (char *);
char **ratecorstatus = (char **)new (char *);
char **trailstatus = (char **)new (char *);
char **move1status = (char **)new (char *);
char **messagestatus = (char **)new (char *);
char **zenithstatus = (char **)new (char *);
char **raspeedstatus = (char **)new (char *);
char **decspeedstatus = (char **)new (char *);
char **messages[12];
extern int therc;
/************************* Paddle/Guider Process *************************/
/*
			A dedicate loop that reads the paddle and guider switches.  This was
		originally part of the front panel process, but the front panel process
		became to slow.  this provides a faster resonse time to paddle and
		guider button pushes.
 */
void PaddleGuiderProcess ()
{
	DosSetPriority (PRTYS_PROCESSTREE,PRTYC_TIMECRITICAL,16,0);
	int sleeptime = 100;
	while (all_run) {
		 DosSleep (sleeptime);
		 if (moveflag) {
			 sleeptime = 1000;
			 continue;
		 }
		 else sleeptime = 100;
		 paddle.readstatus ();
		 guider.readstatus ();
	}
}

/****************************  display diagnostics  **********************//*
			Displays the encoder positions in the last 5 display fields of the
	 diagnostics part of the screen.
 */
void display_diagnostics ()
{
	char temp[40];
	static int flop;

	static char rastr1[40],rastr2[40];
	static char decstr1[40],decstr2[40];
	static char domestr1[40],domestr2[40];
	static char focusstr1[40],focusstr2[40];
	static char inststr1[40],inststr2[40];

	sprintf (temp,"RA :%-9ld %-9ld %-5.0lf %-4.0lf",
								 ra_encoder.fwd,ra_encoder.rev,
								 ra_encoder.rawlocation,ra_encoder.rawoffset);
	if (flop) {			 strcpy (rastr1,temp); *messages[6] = rastr1;		 }
	else {			 strcpy (rastr2,temp); *messages[6] = rastr2;		 }
	sprintf (temp,"DEC:%-9ld %-9ld %-5.0lf %-4.0lf",
								 dec_encoder.fwd,dec_encoder.rev,
								 dec_encoder.rawlocation,dec_encoder.rawoffset);
	if (flop) {			 strcpy (decstr1,temp); *messages[7] = decstr1;		 }
	else { strcpy (decstr2,temp); *messages[7] = decstr2;		 }
	sprintf (temp,"DOME :%-6ld %-6ld %-5.0lf %-4.0lf",
								 dome_encoder.fwd,dome_encoder.rev,
								 dome_encoder.rawlocation,dome_encoder.rawoffset);
	if (flop) {			 strcpy (domestr1,temp); *messages[8] = domestr1;		 }
	else {			 strcpy (domestr2,temp); *messages[8] = domestr2;		 }
	sprintf (temp,"FOCUS:%-6ld %-6ld %-5.0lf %-4.0lf",
					 focus_encoder.fwd,focus_encoder.rev,
					 focus_encoder.rawlocation,focus_encoder.rawoffset);
	if (flop) {			 strcpy (focusstr1,temp); *messages[9] = focusstr1;		 }
	else {			 strcpy (focusstr2,temp); *messages[9] = focusstr2;		 }
	sprintf (temp,"INST  :%-6ld %-6ld %-5.0lf %-4.0lf",
								 inst_encoder.fwd,inst_encoder.rev,
								 inst_encoder.rawlocation,inst_encoder.rawoffset);
	if (flop) {			 strcpy (inststr1,temp); *messages[10] = inststr1;		 }
	else {			 strcpy (inststr2,temp); *messages[10] = inststr2;		 }

	flop = flop == 0;
}
/************************  Front Panel Process  ************************/
/*
		 This process sets up the screen display and also reads the front
		panel switches.  Some error messages are displayed in this routine as
		well.
 */
void FrontPanelProcess ()
{
	int changeit = 1;
	int wait = 0;
	 for (int i=0;i<11;i++) {
		 messages[i] = (char **)new (char *);
		 *(messages[i]) = " ";
		 display.add_value (messages[i],2000+i,"%s",VSTRING);
	 }
	 display.add_value (raspeedstatus,145,"%s",VSTRING);
	 display.add_value (decspeedstatus,180,"%s",VSTRING);
	 display.add_value (initstatus,463,"%s",VSTRING);
	 display.add_value (guidestatus,464,"%s",VSTRING);
	 display.add_value (trackingstatus,465,"%s",VSTRING);
	 display.add_value (zenithstatus,152,"%s",VSTRING);
	 display.add_value (modelstatus,130,"%s",VSTRING);
	 display.add_value (drivestatus,466,"%s",VSTRING);

	 display.add_value (domestatus,467,"%s",VSTRING);
	 display.add_value (domehomestatus,116,"%s",VSTRING);
	 display.add_value (cosdecstatus,468,"%s",VSTRING);
	 display.add_value (ratecorstatus,107,"%s",VSTRING);
	 display.add_value (trailstatus,115,"%s",VSTRING);
 /*
	 display.add_value (&messagestatus,144,"%s",VSTRING);
/*
	 display.add_value (&move2status,116,"%s",VSTRING);

	 display.add_value ((char **)&d1status,128,"%s",VSTRING);
	 display.add_value ((char **)&d2status,129,"%s",VSTRING);
	 display.add_value (&d3status,130,"%s",VSTRING);
	 display.add_value (&d4status,131,"%s",VSTRING);
*/
	 display.add_value ((int*)&paddle.N.state,167,"%d",VINTEGER);

	 display.add_value ((int*)&paddle.S.state,168,"%d",VINTEGER);
	 display.add_value ((int*)&paddle.E.state,169,"%d",VINTEGER);
	 display.add_value ((int*)&paddle.W.state,170,"%d",VINTEGER);
	 display.add_value ((int*)&paddle.slew.state,171,"%d",VINTEGER);
	 display.add_value ((int*)&paddle.guide_set.state,172,"%d",VINTEGER);
	 display.add_value ((int*)&paddle.focusin.state,173,"%d",VINTEGER);
	 display.add_value ((int*)&paddle.focusout.state,174,"%d",VINTEGER);

	 display.add_value ((int*)&guider.N.state,175,"%d",VINTEGER);

	 display.add_value ((int*)&guider.S.state,176,"%d",VINTEGER);
	 display.add_value ((int*)&guider.E.state,177,"%d",VINTEGER);
	 display.add_value ((int*)&guider.W.state,178,"%d",VINTEGER);
	 display.add_value ((int*)&guider.guide_fast.state,179,"%d",VINTEGER);


/*
	 display.add_value ((int *)&frontpanel.autoguide_.state,IDD_GUIDER,"%d",VINTEGER);
	 display.add_value ((int *)&frontpanel.track_.state,IDD_TRACKING,"%d",VINTEGER);
	 display.add_value ((int*)&frontpanel.drivesonoff_.state,IDD_DRIVES,"%d",VINTEGER);
	 display.add_value ((int *)&frontpanel.autodome_.state,IDD_AUTODOME,"%d",VINTEGER);
*/
/*
	 display.add_value ((double *)&ra_encoder.location,411,"%5.3lf",VDOUBLE);
	 display.add_value ((double *)&dec_encoder.location,409,"%5.3lf",VDOUBLE);
	 display.add_value ((unsigned long *)&ra_encoder.fwd,141,"%8lu",VULONG);
	 display.add_value ((unsigned long *)&dec_encoder.fwd,145,"%8lu",VULONG);

	 display.add_value ((unsigned long *)&ra_encoder.rev,139,"%8lu",VULONG);
	 display.add_value ((unsigned long *)&dec_encoder.rev,143,"%8lu",VULONG);
*/
	 display.add_value ((double *)&focus_encoder.location,IDD_FOCUS,"%5.1lf",VDOUBLE);
	 display.add_value ((double *)&dome_encoder.location,IDD_DOME,"%5.1lf",VDOUBLE);
	 display.add_value ((double *)&domeerror,137,"%5.1lf",VDOUBLE);

	// display.add_value ((double *)&inst_encoder.location,IDD_ROTATOR,"%5.2lf",VDOUBLE);
	 display.add_value ((double *)&inst_encoder.location,IDD_ROTATOR,"%3.1lf",VDOUBLE);

	 //display.add_value ((int *)&debug1,IDD_RA,"%d",VINTEGER);
	 //display.add_value ((int *)&debug2,IDD_DEC,"%d",VINTEGER);
	 //display.add_value ((char *)xtcsbuf,IDD_FOCUS,"%s",VSTRING);
	 //display.add_value ((int *)&debug5,IDD_DOME,"%lf",VDOUBLE);
	 //display.add_value ((int *)&debug3,IDD_FOCUS,"%d",VINTEGER);
	 //display.add_value ((int *)&debug4,IDD_DOME,"%d",VINTEGER);
	 display.add_value ((short *)&d_siderealtime.h,IDD_SIDE_HR,"%2d",VSHORT);
	 display.add_value ((short *)&d_siderealtime.mn,IDD_SIDE_MIN,"%02d",VSHORT);
	 display.add_value ((float *)&d_siderealtime.s,IDD_SIDE_SEC,"%02.0f",VFLOAT);
	 display.add_value ((short *)&d_univtime.h,IDD_UNIV_HR,"%2d",VSHORT);
	 display.add_value ((short *)&d_univtime.mn,IDD_UNIV_MIN,"%02d",VSHORT);
	 display.add_value ((float *)&d_univtime.s,IDD_UNIV_SEC,"%02.0f",VFLOAT);

	 display.add_value ((short *)&d_local_time.h,105,"%2d",VSHORT);
	 display.add_value ((short *)&d_local_time.mn,106,"%02d",VSHORT);
	 display.add_value ((float *)&d_local_time.s,112,"%02.0f",VFLOAT);

	 display.add_value ((short *)&local_time.mo,IDD_LOCAL_HR,"%2d",VSHORT);
	 display.add_value ((short *)&local_time.d,IDD_LOCAL_MIN,"%02d",VSHORT);
	 //display.add_value ((short *)&local_time.y,IDD_LOCAL_SEC,"%d",VSHORT);	 // note if the drives are on or off.
	 // mean ra.
	 display.add_value ((char*)&meanrasign,148,"%c",VCHAR);
	 display.add_value ((double *)&meanra.hh,703,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&meanra.mm,740,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&meanra.ss,741,"%4.1lf",VDOUBLE);
	 // mean dec.
	 display.add_value ((char*)&meandecsign,150,"%c",VCHAR);
	 display.add_value ((double *)&meandec.hh,120,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&meandec.mm,121,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&meandec.ss,122,"%2.0lf",VDOUBLE);	 // telescope ha.
	 // telescope ha
	 display.add_value ((char*)&telehasign,146,"%c",VCHAR);
	 display.add_value ((double *)&teleha.hh,903,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&teleha.mm,940,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&teleha.ss,941,"%4.1lf",VDOUBLE);
	 // telescope dec.
	 display.add_value ((char*)&teledecsign,185,"%c",VCHAR);
	 display.add_value ((double *)&teledec.hh,143,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&teledec.mm,144,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&teledec.ss,181,"%2.0lf",VDOUBLE);
	 // telescope target mean ra
	 display.add_value ((char *)&targetrameansign,149,"%c",VCHAR);
	 display.add_value ((double *)&targetramean.hh,1043,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&targetramean.mm,1049,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&targetramean.ss,1050,"%4.1lf",VDOUBLE);
	 // telescope target mean dec.
	 display.add_value ((char*)&targetdecmeansign,151,"%c",VCHAR);
	 display.add_value ((double *)&targetdecmean.hh,123,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&targetdecmean.mm,124,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&targetdecmean.ss,125,"%2.0lf",VDOUBLE);
	 // telescope target ha
	 display.add_value ((char*)&targethasign,147,"%c",VCHAR);
	 display.add_value ((double *)&targetha.hh,109,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&targetha.mm,110,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&targetha.ss,111,"%4.1lf",VDOUBLE);
	 // telescope target dec.
	 display.add_value ((char*)&targetdecsign,186,"%c",VCHAR);
	 display.add_value ((double *)&targetdec.hh,182,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&targetdec.mm,183,"%2.0lf",VDOUBLE);
	 display.add_value ((double *)&targetdec.ss,184,"%2.0lf",VDOUBLE);
	 // std epoch.
	 display.add_value ((double *)&stdepoch,448,"%7.2lf",VDOUBLE);
	 display.add_value ((double *)&targetepoch,163,"%7.2lf",VDOUBLE);
	 display.add_value ((double *)&currentepoch,134,"%-7.1lf",VDOUBLE);
	 display.add_value ((double *)&trailrate,453,"%5.2lf",VDOUBLE);
	 display.add_value ((double *)&traillength,454,"%5.1lf",VDOUBLE);
	 display.add_value ((double *)&trailposangle,457,"%5.1lf",VDOUBLE);	 // airmass.
	 display.add_value ((double *)&telealtitude,161,"%4.1lf",VDOUBLE);
	 display.add_value ((double *)&teleazimuth,162,"%5.1lf",VDOUBLE);
	 display.add_value ((double *)&targetaltitude,164,"%4.1lf",VDOUBLE);
	 display.add_value ((double *)&targetazimuth,165,"%5.1lf",VDOUBLE);	 // altitude n elevation.
	 display.add_value ((double *)&teleparallactic,131,"%3.1lf",VDOUBLE);
	 display.add_value ((double *)&targetparallactic,133,"%3.1lf",VDOUBLE);
	 display.add_value ((double *)&RA.ratecorr,460,"%-7.4lf",VDOUBLE);
	 display.add_value ((double *)&DEC.ratecorr,461,"%-6.3lf",VDOUBLE);	 display.add_value ((double *)&juliandate,188,"%9.2lf",VDOUBLE);
	 display.add_value ((double *)&airmass,119,"%5.3lf",VDOUBLE);
	 display.add_value ((double *)&targetairmass,138,"%5.3lf",VDOUBLE);
	 // track rates
	 display.add_value ((double *)&RA.trackspeed,441,"%-7.4lf",VDOUBLE);
	 display.add_value ((double *)&DEC.trackspeed,445,"%-6.3lf",VDOUBLE);
	 // Guide rates
	 display.add_value ((double *)&RA.guidespeed,449,"%-3.2lf",VDOUBLE);
	 display.add_value ((double *)&DEC.guidespeed,190,"%-3.2lf",VDOUBLE);

	 // Set rates
	 display.add_value ((double *)&RA.setspeed,452,"%-3.2lf",VDOUBLE);
	 display.add_value ((double *)&DEC.setspeed,191,"%-3.2lf",VDOUBLE);
	 // offset rates

	 *initstatus = "Initialized";
	 *cosdecstatus = "Cos Dec Off";
	 *ratecorstatus = "Rate Corr Off";
	 *trailstatus = "Trailing Off";
	 rastatus.update_message (&raspeedstatus);
	 decstatus.update_message (&decspeedstatus);
	 while (all_run ) { // ininite loop that updates input from the user
		 frontpanel.readstatus ();
		 if (telealtitude > 89.9) {
			 *zenithstatus = "At Zenith";
		 }
		 else {
			 *zenithstatus = " ";
		 }
		 if (telealtitude > 89.6) {
			 *messages[2] = "Dome Occulting Telescope";
		 }
		 else {
			 *messages[2] = " ";
		 }
		 if (nomodel) {
			 *modelstatus = "Model Off";
		 }
		 else {
			 *modelstatus = "Model On";
		 }
		 if (frontpanel.drivesonoff_.on ()) {
			 drivechassisonoff.turn_on (); // turn on drives.
			 *drivestatus="Drives On";
			 *messages[3]=" ";
/*
			 if ((paddle.slew.clicks ||  paddle.slew.on ()) && trailflag) {
				 *messages[3] ="Can't Slew With Trailing On!";
			 }
*/
		 }
		 else {
			 drivechassisonoff.turn_off (); // turn off drives.
			 *drivestatus="Drives Off";
			 if (moveflag || RAdata.axismoveflag || DECdata.axismoveflag)
				 *messages[3] = "Attempting to move with drives off";
			 else if (focusmoveflag)
				 *messages[3] = "Moving focus with drives off";
			 else *messages[3] = " ";
		 }

		 if (frontpanel.autodome_.on ()) {
			 autodomeflag=1;
			 *domestatus = "Auto Dome On";
		 }
		 else if (frontpanel.autodome_.off ()) {
			 autodomeflag=0; 
			 *domestatus = "Auto Dome Off";
			 if (moveflag)
				 *messages[4] = "Moving With AutoDome Off!";
			 else *messages[4] = " ";

		 }

		 if (frontpanel.domehome_.on ()) {
			 domehomeflag=1;
			 if (dome_encoder.location < 321.0 && dome_encoder.location > 319.0) {
				 *domehomestatus = "Dome at Home";
			 } else {
				 *domehomestatus = "Homing Dome";
			 }
		 }
		 else if (frontpanel.domehome_.off ()) {
			 domehomeflag=0;
			 *domehomestatus = "Dome Free";
		 }

		 if (frontpanel.autoguide_.on ()) {
			 autoguideflag=1;
			 *guidestatus = "Auto Guider On";
		 }
		 else if (frontpanel.autoguide_.off ()) {
			 autoguideflag=0;
			 *guidestatus = "Auto Guider Off";
		 }
		 if (frontpanel.track_.on ()/* && !RA.limit.softlimit && !DEC.limit.softlimit*/) {
			 *trackingstatus="Tracking On";
			 trackingflag = 1;
		 }
		 else {
			 *trackingstatus="Tracking Off";
			 trackingflag = 0;
		 }
		 // displays the encoder raw counts and location.
		 //display_diagnostics ();

		 if (wait++ > 1000 ) {
			 wait = 0;
			 ratecorr_size(hatele,dectele,MDMLATITUDE, MDMELEVATION,
					&RA.ratecorr, &DEC.ratecorr);
		 char buf[256];
		 sprintf (buf,"ratecorr: %lf %lf    %lf \n",RA.ratecorr, DEC.ratecorr,teleparallactic);
		 debug.post (buf);		 }
		 cosdec = cos (dectele*DEGREESTORADIANS);
		 if ((fabs (cosdec-lastcosdec) > 0.01) && cosdecflag) {
				set_guide_speeds (guiderate);
				set_set_speeds (setrate);
				lastcosdec = cosdec;
		 }

		 DosSleep (52); // sleep for 100 milliseconds.
		 if (!inside_rawlimits (haraw,decraw) ){
			 if (RA.limit.fwd5.on ())
				 *messages[7] = "West Limit Reached";
			 else if (RA.limit.rev5.on ())
				 *messages[7] = "Eest Limit Reached";		 else if (RA.limit.fwd8.on ())
				 *messages[7] = "Approaching West Limit";
			 else if (RA.limit.rev8.on ())
				 *messages[7] = "Approaching Eest Limit";
			 else
				 *messages[7] = "Outside Soft Limits";		 if (DEC.limit.fwd5.on ())
				 *messages[8] = "North Limit Reached";
			 else if (DEC.limit.rev5.on ())
				 *messages[8] = "South Limit Reached";		 else if (DEC.limit.fwd8.on ())
				 *messages[8] = "Approaching North Limit";
			 else if (DEC.limit.rev8.on ())
				 *messages[8] = "Approaching South Limit";
			 else
				 *messages[8] = "Outside Soft Limits";
			 changeit = 1;
		 }
		 else if (changeit) {
			 changeit = 0;
			 *messages[7] = " ";
			 *messages[8] = " ";
		 }


		 DosSleep (49);

	 }
}
/*%%%%%%%%%%%%%%%%%%%%%%%%%  Speed Status Class  %%%%%%%%%%%%%%%%%%%%%%%%%*/
/*
			 This class is used to keep track of the status of an axis for display
		on the screen.
 */
/************************  update_message  *******************************/
/*
		 update_message runs ascyncronously with the movement processes and
	 tries to figure out what they are doing.  The movement processes call
	 functions which increment variables like _slew and _guide so that this
	 functions know what has been activated.  This function then tries to
	 combine messages such as trail and set or track and guide together into
	 one message.  note that some things don't go together like slew and guide.

 */
		void SpeedStatus::update_message (char ***status) {
			char tmp[100],*compare;
			if (flopper) message = exchange;
			else message = hold;
			flopper = flopper == 0;
			if (_slew) (_slew<=1)?
					sprintf (message, "Slew"):
					sprintf (message,"Slew%d",_slew);
			else {
				if (_set) (_set<=1)?
					sprintf (tmp,"Set"):
					sprintf (tmp,"Set%d",_set);
				else if (_guide) (_guide<=1)?
					sprintf (tmp,"Guide"):
					sprintf (tmp,"Guide%d",_guide);
				else if (_guide) (_fast<=1)?  // the guider uses this speed.
					sprintf (tmp,"Fast"):
					sprintf (tmp,"Fast%d",_fast);
				else if (!_track && !_trail) strcpy (tmp,"Idle");
				else strcpy (tmp," ");
				if (!strcmp (tmp," ")) { // for when tracking and trailing are alone.
					if (_track && _trail) sprintf (message,"%s","Track+Trail");
					else if (_track) sprintf (message,"%s","Track");
					else if (_trail) sprintf (message,"%s","Trail");
					else  sprintf (message,"%s",tmp);
				}
				else { // for when tracking and trailing are done with a movement.
					if (_track && _trail) sprintf (message,"%s+%s",tmp,"Track+Trail");
					else if (_track) sprintf (message,"%s+%s",tmp,"Track");
					else if (_trail) sprintf (message,"%s+%s",tmp,"Trail");
					else  sprintf (message,"%s",tmp);
				}

			}
			//if (flopper) compare = exchange;
			//else compare = hold;
			//if (strcmp (message,compare)) // if they are not the same...
				**status = message;
		}


