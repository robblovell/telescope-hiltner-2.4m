/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/***************************  xtcslink.cpp  ****************************/
#define INCL_DOSERRORS
#include <string.h>
#include "tcscomm.h"
#include "skycalc.h"
#include "hooks.h"
#include "telescpe.h"
#include "hiltner.h"
#include "display.h"
extern Display display;
extern int paintall;
extern char *d1status;
extern char *d2status;
extern char *d3status;
extern char *d4status;
extern void get_time ();
int cancelit = 0;
extern int trailmode;
extern char **messages[12];
extern int moveflag;
SerialIO xtcscomm;
extern int debug1;
extern char **messagestatus;
#define STRINGBUFLEN	257
char xtcsbuf[STRINGBUFLEN];
char statusbuf[STRINGBUFLEN];
extern int focusmove;
//  encoders...
extern void set_encoderpositions ();    // in hooks.cpp
// initialization ---> "\r"
extern void initialize_tcs ();      // in hiltner.cpp
/**************************  xtcs connect  ***************************/
/*
		 Initialize the serial port.
 */
void xtcs_connect ()
{
	xtcscomm.initialize_port (1);
}

/*************************  hms to arcsec  ******************************/
void hms_to_arcsec (double h,double m,double s,double &arcsec)
{
	arcsec = h+m/60.0+s/3600.0;
}
/****************************  get_coordinates  *************************/
/*
		 Used by set coordinates and set ra and dec encoders to read
	from the communications buffer the ra and dec coordinates and then
	convert mean to raw place.
 */
void get_coordinates (int &length,char xtcsbuf [],double &thera,
            double &thedec, double &theepoch)
{	// get the ra coordinates.
	cancelit = 0;
	length = xtcscomm.next (&xtcsbuf);
	xtcscomm.send ("*");
	char *next;
	struct coord ra,dec;
	ra.hh = (double)strtol (xtcsbuf,&next,10);
	ra.mm = (double)strtol (next,&next,10);
	ra.ss = strtod (next,&next);
	ra.sign = 1;

char buf[256];
	sprintf (buf,"ra hours %lf %lf %lf %s",ra.hh,ra.mm,ra.ss,xtcsbuf);
	debug.post (buf);	// get the dec coordinates.

	length = xtcscomm.next (&xtcsbuf);
	xtcscomm.send ("*");
	dec.sign = 1;
	for (int i=0;i<10 && i <strlen (xtcsbuf);i++) {
		if (xtcsbuf[i] == '-') { dec.sign = -1; break; }
	}
	dec.hh = (double)strtol (xtcsbuf,&next,10);
	dec.mm = (double)strtol (next,&next,10);
	dec.ss = strtod (next,&next);


	if (dec.hh < 0) {dec.hh*=-1; dec.sign = -1; }

	sprintf (buf,"dec %d %lf %lf %lf %s",dec.sign,dec.hh,dec.mm,dec.ss,xtcsbuf);
	debug.post (buf);

	// the epoch is last.

	length = xtcscomm.next (&xtcsbuf);

	theepoch = strtod (xtcsbuf,&next);
	if (theepoch == -1) theepoch = stdepoch;
	// semaphore not needed here since this is serial with the display
	// interupt which calculates apparent place.
	thera = (double)bab_to_dec ((struct coord)ra);
	thedec = (double)bab_to_dec ((struct coord)dec);
	//sprintf (buf,"epoch:%lf %s",theepoch,xtcsbuf);
	//debug.post (buf);
}
/**********************  get offset coordinates  **************************/
/*
			Get offset coordinates decifers the message from xtcs that contains
		the offset information.
 */
void get_offset_coordinates (int &length,char xtcsbuf [],double &thera,
														 double &thedec,double &theepoch)
{	// get the ra coordinates.
				cancelit = 0;

	char *next;
	struct coord ra,dec;
	length = xtcscomm.next (&xtcsbuf);
	thera  =  (double) strtod (xtcsbuf,&next);
	xtcscomm.send ("*");
	length = xtcscomm.next (&xtcsbuf);
	thedec = (double) strtod (xtcsbuf, &next);

	ra.hh = 0; // (double)strtol (xtcsbuf,&next,10);
	ra.mm = 0;//(double)strtol (next,&next,10);
	ra.ss = thera; //rel (double) strtod (xtcsbuf,&next);       //  in arcseconds.
	ra.ss /= 15.0;  // convert to seconds from arcseconds...
	ra.sign = 1;
	//rel xtcscomm.send ("*");
	// get the dec coordinates.
//DosSleep (50);
	//rel length = xtcscomm.next (&xtcsbuf);

	dec.hh = 0; //(double)strtol (xtcsbuf,&next,10);
	dec.mm = 0;//(double)strtol (next,&next,10);
	dec.ss = thedec; //rel (double) strtod (xtcsbuf,&next);

//	sprintf (buf,"dec hours %lf %lf %lf",dec.hh,dec.mm,dec.ss);
//	debug.post (buf);

	dec.sign = 1;
	if (dec.hh < 0) {dec.hh*=-1; dec.sign = -1; }

	// the epoch is last.
//DosSleep (50);  */
	xtcscomm.send ("*");
	length = xtcscomm.next (&xtcsbuf);
	theepoch = strtod (xtcsbuf,&next);
	if (theepoch == -1) theepoch = stdepoch;
	//char buf[256];
	//sprintf (buf,"epoch %lf ra: %lf %lf %lf dec hours %lf %lf %lf",
	//			theepoch,ra.hh,ra.mm,ra.ss,dec.hh,dec.mm,dec.ss);
	//debug.post (buf);	//???? check this... semaphore not needed here since this is serial with the display interupt
	// which calculates apparent place.
	//rel thera = (double)bab_to_dec ((struct coord)ra);
	//rel thedec = (double)bab_to_dec ((struct coord)dec);
//	sprintf (buf,"declination hours %lf ",thedec);
 //	debug.post (buf);
 //	DosSleep (10);
}

// Universal time ---> "1\r"
void set_universaltime (int &length,char xtcsbuf[])
{
	char *next;
	date_time utime;

	length = xtcscomm.next (&xtcsbuf); // year
	utime.y = atoi (xtcsbuf);
	xtcscomm.send ("*");
	length = xtcscomm.next (&xtcsbuf); // month
	utime.mo = atoi (xtcsbuf);
	xtcscomm.send ("*");
	length = xtcscomm.next (&xtcsbuf); // day
	utime.d = atoi (xtcsbuf);
	xtcscomm.send ("*");
	length = xtcscomm.next (&xtcsbuf); //  hour minute second.
	xtcscomm.send ("*");
	utime.h = strtol (xtcsbuf,&next,10);
	utime.mn = strtol (next,&next,10);
	utime.s = strtol (next,&next,10);
	utime.s -= 2.;
	if (utime.s < 0) {

		utime.s += 60.; utime.mn-=1;
		if (utime.mn < 0) {
			utime.mn += 60; utime.h-=1;
			if (utime.h < 0) {
				utime.h += 24; utime.d-=1;
				if (utime.d < 0) {
					// wait two seconds and then set the time...
					DosSleep (1999.0);
					utime.s += 2.; utime.mn -= 60; utime.h -= 24;
					utime.d++;
	} } } }


	set_local_time (utime);
	length = 0;
}
// Dome Position ---> "2\r"
// values given in degrees.
void set_domeposition (int &length,char xtcsbuf[])
{
	length = xtcscomm.next (&xtcsbuf); // 0-360 dome position.
	double pos = (double)atoi (xtcsbuf);
 //	xtcscomm.send ("*");
	dome.encoder->set_to_arcsec_location (pos*DEGREESTOARCSEC);

	length = 0;
}
// Encoder Positions for RA and DEC ---> "3\r"
extern void set_encoderpositions ();

// Instrument Rotator Encoder ---> "4\r"
void set_instposition (int &length,char xtcsbuf[])
{
	char *next;
	length = xtcscomm.next (&xtcsbuf); // 0-360 inst rotator position.
	double pos = (double) strtod (xtcsbuf,&next);
	inst_encoder.set_to_arcsec_location (pos*DEGREESTOARCSEC);
	length = 0;
}
// Focus Encoder  ---> "5\r"
void set_focusposition (int &length,char xtcsbuf[])
{
	length = xtcscomm.next (&xtcsbuf); // 0-10000 focus position.
	double pos = (double) atoi (xtcsbuf);

	focus.encoder->set_to_raw_location (pos);
	length = 0;
}
// Coordinates, same as set encoders. --> "6\r"
extern void set_coordinates ();


// Offsets  ---> "7\r"
// uses set_coordinates....


// Trail  ---> "8\r"
void set_trail (int &length,char xtcsbuf[],double &ra,
							 double &dec, double &rarate,double &decrate,
							 double &rate,double &tlength,double &posangle)
{
	char *next;
	//double rate, tlength, posangle;

	length = xtcscomm.next (&xtcsbuf); // .
	xtcscomm.send ("*");
 	rate = (double)strtol (xtcsbuf,&next,10);
	
	length = xtcscomm.next (&xtcsbuf);
	xtcscomm.send ("*");
	tlength = (double)strtol (xtcsbuf,&next,10);

	length = xtcscomm.next (&xtcsbuf);
	posangle = strtod (xtcsbuf,&next);//*DEGREESTORADIANS;
	figure_trail_directions (rate,tlength,posangle,ra,dec,rarate,decrate);

	char buf[256];
	sprintf (buf,"dec:%lf rt:%lf l:%lf p:%lf dlen:%lf rlen:%lf drt:%lf rart:%lf",
				 dectele,rate,tlength,posangle,dec,ra,decrate,rarate);
	debug.post (buf);
}
// Track Rates ---> "11\r"
int set_trackrates (int &length,char xtcsbuf[])
{
	length = xtcscomm.next (&xtcsbuf); // .
	xtcscomm.send ("*");

	double d = atof (xtcsbuf);
	if (d < -80 || d > 80) return 0;
	RA.trackspeed = d;

	length = xtcscomm.next (&xtcsbuf);
	d = atof (xtcsbuf);
	if (d < -80 || d > 80) return 0;
	DEC.trackspeed = d;
	//xtcscomm.send ("*");
	length = 0;
	return 1;
}
// Guide Rate  ---> "12\r"
int set_guiderate (int &length,char xtcsbuf[])
{
 //	xtcscomm.send ("*");

	char buf[256];
	length = xtcscomm.next (&xtcsbuf); // .
	double d = atof (xtcsbuf);
//	if (d < -85 || d > 85) return 0;
	guiderate = d;
	set_guide_speeds (guiderate);
	sprintf (buf,"set guide rate:%lf  %d %s",d,length,xtcsbuf); debug.post (buf);
	length = 0;
	return 1;
}

// Set Rate  ---> "13\r"
int set_setrate (int &length,char xtcsbuf[])
{
	length = xtcscomm.next (&xtcsbuf); // .
	double d = atof (xtcsbuf);
	if (d < -80 || d > 80) return 0;
	setrate = d;
	set_set_speeds (setrate);
	char buf[256];
	sprintf (buf,"set rate:%lf ",setrate);
	debug.post (buf);
	length = 0;	return 1;
}
// Offset Rate  ---> "14\r"
int set_offsetrate (int &length,char xtcsbuf[])
{
	length = xtcscomm.next (&xtcsbuf);
	double d = atof (xtcsbuf);
	if (d < -80 || d > 80) return 0;
	RA.offsetspeed = d;
	DEC.offsetspeed = d;	length = 0;
	return 1;}
// Cos Dec on/off  ---> "15\r"
void set_cosdec (int &length,char xtcsbuf[])
{
	length = xtcscomm.next (&xtcsbuf);
	cosdecflag = atoi (xtcsbuf); //	cosdecflag = cosdecflag == 0;
	if (cosdecflag ) *cosdecstatus="Cos Dec On";
	else *cosdecstatus="Cos Dec Off";
	length = 0;
}
// Rate cor  ---> "16\r"
void set_rate_cor (int &length,char xtcsbuf[])
{
	ratecorr_size(hatele,dectele,MDMLATITUDE, MDMELEVATION,
					&RA.ratecorr, &DEC.ratecorr);
	length = xtcscomm.next (&xtcsbuf);
	ratecorflag = atoi (xtcsbuf);	//ratecorflag = ratecorflag == 0;
	if (ratecorflag ) *ratecorstatus="Rate Corr On";
	else *ratecorstatus="Rate Corr Off";
	length = 0;
}
// Epoch  ---> "17\r"
void set_epoch (int &length,char xtcsbuf[])
{
	 char *next;
	length = xtcscomm.next (&xtcsbuf); // Epoch.

	stdepoch = strtod (xtcsbuf,&next);
	if (stdepoch == -1) stdepoch = currentepoch;
	length = 0;
}
// Get status  ---> "19\r"
int get_status (int &length,char *xtcsbuf,char *statusbuf)
{
	length = xtcscomm.next (&xtcsbuf);
	int i = atoi (xtcsbuf);

DosSleep (500);
	if (i < 0 || i > 80) return 0;
	switch (i) {
		case 1:
				sprintf (statusbuf,
"*RA: %9.6lf   DEC: %9.6lf   EP: %8.3lf   FOC: %6.1lf   ROT: %6.2lf ",
				ramean,decmean,stdepoch,focus_encoder.location,inst_encoder.location);
			break;
		case 3:
				sprintf (statusbuf,
"*HA: %9.6lf   DEN: %9.5lf   RAP: %9.6lf   DAP: %9.5lf",
				hatele,dectele,ramean,decmean);
			break;
		default:
			 strcpy (statusbuf,"TCSERROR");
	}

	return 1;}

// Cancel & Stop ---> "\023"
void cancel_stop ()
{
	cancelit = 1;
	RA.moveflag = 0;
	DEC.moveflag = 0;
	DosSleep (100);
	moveflag = 0;
}
// go  ---> "\007"
void go ()
{
	// check for valid coordinates before moving...

		if (inside_rawlimits (RAdata.axis->target,DECdata.axis->target) ) {
			RA.moveflag = 1;
			DEC.moveflag = 1;
			moveflag = 1;
		}
		cancelit = 0;
}
/************************* Auto Focus Routines  ***********************/
		/* Added 6/3/96 for auto focus. */
extern int autofocusmoveflag;
extern double focustarget;
int move_focus_relative  (int &length,char xtcsbuf[])
{
	 char *next,buf[256];
	 length = xtcscomm.next (&xtcsbuf); // 0-360 inst rotator position.
	 double pos = (double) strtod (xtcsbuf,&next);
	 if (fabs (pos) > 1000) {
		 return 0;
	 }	 focustarget = focus_encoder.location + pos;
	 autofocusmoveflag = 1;

//	 sprintf (buf,"move focus by: %lf to:%lf",pos,focustarget);
//	 debug.post (buf);
	 return 1;
}
int move_focus_absolute  (int &length,char xtcsbuf[])
{
	 char *next,buf[256];
	 length = xtcscomm.next (&xtcsbuf); // 0-360 inst rotator position.
	 double pos = (double) strtod (xtcsbuf,&next);
	 if ( fabs (focus_encoder.location-pos) > 1000) {
		 return 0;
	 }
	 focustarget = pos;
	 autofocusmoveflag = 1;

	// sprintf (buf,"move focus to: %lf",pos);
	// debug.post (buf);
	 return 1;
}
int read_focus_position (int &,char *,char *statusbuf)
{
	sprintf (statusbuf,"FOC: %6.1l",focus_encoder.location);
	return 1;
}

void focus_cancel_stop ()
{
	autofocusmoveflag = 0;
	DosSleep (100);
}

/***************************  invalidate move  **************************/
/*
			After a point to point move has complete, certain values should not
	 be displayed anymore.
 */
void  invalidatemove ()
{
/*
			display.mark_invalid (1043);
			display.mark_invalid (1049);
			display.mark_invalid (1050);
*/
			display.mark_invalid (110);
			display.mark_invalid (111);
/*
			display.mark_invalid (123);
			display.mark_invalid (124);
			display.mark_invalid (125);
*/
		//	display.mark_invalid (151);
		//	display.mark_invalid (149);
			display.mark_invalid (164);
			display.mark_invalid (165);
			display.mark_invalid (182);
			display.mark_invalid (183);
			display.mark_invalid (184);
			display.mark_invalid (186);
			display.mark_invalid (133);
			display.mark_invalid (147);
			display.mark_invalid (109);

			//display.mark_invalid (163);
			display.mark_invalid (138);
}
/*******************************  validate move  **************************/
/*
		 After a offset or new cordinate position is given, display the new
	 coordinates.
 */
void validatemove ()
{

/*
		display.mark_valid (1043);
		display.mark_valid (1049);
		display.mark_valid (1050);
*/
		display.mark_valid (110);
		display.mark_valid (111);
/*
		display.mark_valid (123);
		display.mark_valid (124);
		display.mark_valid (125);
*/
	 //	display.mark_valid (151);
	 //	display.mark_valid (149);
		display.mark_valid (164);
		display.mark_valid (165);
		display.mark_valid (182);
		display.mark_valid (183);
		display.mark_valid (184);
		display.mark_valid (186);
		display.mark_valid (133);
		display.mark_valid (147);
		display.mark_valid (109);

	 //	display.mark_valid (163);
		display.mark_valid (138);
}

// toggle trail  ---> "\024"
/*
void toggle_trail (int &length,char xtcsbuf[])
{
	length = xtcscomm.next (&xtcsbuf); // .
	int i = atoi (xtcsbuf);
	length = 0;
}
*/
// init telescope to zentih  ---> "\004"
void init_to_zenith (int &length,char [])
{
 //	initialize_tcs ();
	length = 0;
}
/******************************  XTCS Link  ****************************/
/*
				Check the serial port for a message and if a message exists
		interpret that message.
 */
void xtcs_link ()
{
	static int firsttime = 1;
	double temptargetepoch,temprameantarget,tempdecmeantarget;

	if (firsttime) { firsttime = 0;			xtcscomm.send ("TCSREADY"); }
	int length=0;
	char *abuf = xtcsbuf;
	length = xtcscomm.next (&abuf);
//debug1 = length;
 if (length > 0) debug.post (xtcsbuf);
	while (length > 0) {
		// respond to are you awake?
		if (!strncmp (xtcsbuf,"\r",1)) {   // clear link...
			length--;
			*initstatus = "Initialized";
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"1\r",2)) { // set universal time.
			length-=2;
			xtcscomm.send ("*");
			set_universaltime (length,xtcsbuf);
			//length = xtcscomm.next (&xtcsbuf);
			*messages[1] = "Universal Time Set";
			DosSleep (200);
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"2\r",2)) { // Dome position.
			length-=2;
			xtcscomm.send ("*");
			set_domeposition (length,xtcsbuf);
			*messages[1] = "Dome Position Set";
			//length = xtcscomm.next (&xtcsbuf);
			DosSleep (200);
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"3\r",2)) { // Encoders for RA and DEC.
			length-=2;
			xtcscomm.send ("*");
			if (DosRequestMutexSem (setencoders_mex_sem,10000L) != ERROR_TIMEOUT) {

				get_coordinates (length,xtcsbuf,ramean,decmean,targetepoch);
				// needs to transform ramean and decmean to haraw and decraw.
				// should block the time process.
				get_time ();
				mean_place_to_telescope_coordinates (ramean,decmean,targetepoch,
								siderealhours,hatele,dectele,1,0);
 /*
	 dectele = MDMLATITUDE+40; //31.95133 encoders have resolution to 4.87 and 4.37
	 hatele = 0.0;
	 set_encoders_telescope_coordinates_to_raw_coordinates (hatele,dectele,haraw,decraw);	// decmean = MDMLATITUDE;
	 set_encoderpositions ();
 */				/*if (set_encoders_*/
				telescope_coordinates_to_raw_coordinates (hatele,dectele,
																									haraw,decraw);
//) {
					if (!moveflag)
						set_encoderpositions ();

					*messages[1] = "Encoder Position's Set";
				//}

				DosReleaseMutexSem (setencoders_mex_sem);
			}
			length = 0;
			xtcscomm.send ("TCSREADY");
		}
		else if (!strncmp (xtcsbuf,"4\r",2)) { // inst rot
			length-=2;
			xtcscomm.send ("*");
			set_instposition (length,xtcsbuf);
			*messages[1] = "Instrument Rotator Set";
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		// Set focus.
		else if (!strncmp (xtcsbuf,"5\r",2)) { // focus
			length-=2;
			xtcscomm.send ("*");
			set_focusposition (length,xtcsbuf);
			*messages[1] = "Focus Position Set";
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"6\r",2)) { // set coordinates
			length-=2;
			xtcscomm.send ("*");

			get_coordinates (length,xtcsbuf,rameantarget,decmeantarget,targetepoch);


			set_coordinates ();
			RAdata.axis->offsetmoveflag = 0;
			DECdata.axis->offsetmoveflag = 0;
			validatemove ();
			paintall = 1;
			*messages[1] = "Coordinates Received";
			length = 0;
			DosSleep (200);
			xtcscomm.send ("TCSREADY");
		}
		else if (!strncmp (xtcsbuf,"7\r",2)) { // offset coordinates
			length-=2;
			xtcscomm.send ("*");
			double raarcsecoffset,decarcsecoffset;
			// returns ra and dec offset in arcseconds.
			get_offset_coordinates (length,xtcsbuf,raarcsecoffset,decarcsecoffset,
												targetepoch);

// REL
// don't need to get the actual telescope position
// to calculate the
// ideal location where the astronomer want's the telescope to be located.


	//temprameantarget = (double)bab_to_dec ((struct coord)meanra);
	//tempdecmeantarget = (double)bab_to_dec ((struct coord)meandec);
//REL 7/25/99 added   cosdec flag test and modification.
// rameantarget in hours, decmean target in degrees.
				if (cosdecflag) {
					decmeantarget = decmeantarget + (decarcsecoffset/DEGREESTOARCSEC);
					rameantarget = rameantarget +
						((raarcsecoffset/cos(decmeantarget/DEG_IN_RADIAN)
						)/HOURSTOARCSEC);
								}
				else {
					decmeantarget = decmeantarget + (decarcsecoffset/DEGREESTOARCSEC);
					rameantarget = rameantarget + (raarcsecoffset/HOURSTOARCSEC);

				}
//REL end changes 7-25-99
					set_coordinates ();
 // REL end change 4-16-98

				RAdata.axis->offsetmoveflag = 1;
				DECdata.axis->offsetmoveflag = 1;
				validatemove ();
				paintall = 1;
				length = 0;
				*messages[1] = "Offsets Received";

			xtcscomm.send ("TCSREADY");
		}
		else if (!strncmp (xtcsbuf,"8\r",2)) {  // trailing
			length   -= 2;
			xtcscomm.send ("*");
			set_trail (length,xtcsbuf,
					RAdata.axis->traillength,DECdata.axis->traillength,
					RAdata.axis->trailspeed,DECdata.axis->trailspeed,
					trailrate,traillength,trailposangle);
     
			//if ((int)RAdata.axis->traillength != 0 &&
			//		(int)RAdata.axis->trailspeed != 0)
				RAdata.axis->trailflag = 1;
			//if ((int)DECdata.axis->traillength != 0 &&
			 //		(int)DECdata.axis->trailspeed != 0)
				DECdata.axis->trailflag = 1;
	//		DosSleep (500);
			xtcscomm.send ("TCSREADY");
			trailflag = 1;
			*trailstatus = "Trailing On";
		 //	DosSleep (2500);		 //	xtcscomm.send ("TCSREADY");
//char buf [256];
//sprintf (buf,"ra:%lf dec:%lf ha:%lf den:%lf     %lf ",
//		 rameantarget,decmeantarget,hatarget,dectarget,siderealhours);
//sprintf (buf,"%d %d ral:%lf decl:%lf rar:%lf decr:%lf     ",
//RAdata.axis->trailflag,DECdata.axis->trailflag,
//RAdata.axis->traillength,DECdata.axis->traillength,
//				RAdata.axis->trailspeed,DECdata.axis->trailspeed);debug.post (buf);			length = 0;
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"\024",1)) { // trailing
			length= 0;
		 //	xtcscomm.send ("*");
			trailflag = trailflag==0;
			if (trailmode) {
				if (trailflag)
					*trailstatus = "Trailing On.";
				else
					*trailstatus = "Trailing Off.";
			}
			else {
				if (trailflag)
					*trailstatus = "Trailing On!";
				else
					*trailstatus = "Trailing Off!";			}
		}
		else if (!strncmp (xtcsbuf,"\024",1)) { // trailing
			length= 0;
		 //	xtcscomm.send ("*");
			trailmode = trailmode==0;
			if (trailmode) {
				if (trailflag)
					*trailstatus = "Trailing On.";
				else
					*trailstatus = "Trailing Off.";
			}
			else {
				if (trailflag)
					*trailstatus = "Trailing On!";
				else
					*trailstatus = "Trailing Off!";			}
		}		else if (!strncmp (xtcsbuf,"9\r",2)) {  // nothing
			length=0;
			xtcscomm.send ("*");
		}
		else if (!strncmp (xtcsbuf,"10",2)) { // nothing
			length-=3;
			xtcscomm.send ("*");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"11",2)) {  // track rate
			length-=3;
			xtcscomm.send ("*");
			if (set_trackrates (length,xtcsbuf)) {
				xtcscomm.send ("TCSREADY");
			*messages[1] = "Track Rate Received";
			}
			else {
				xtcscomm.send ("TCSERROR");
			}
			length = 0;

		}
		else if (!strncmp (xtcsbuf,"12",2)) {  // guide rate
			xtcscomm.send ("*");
			if (set_guiderate (length,xtcsbuf)) {
				*messages[1] = "Guide Rate Received";
				xtcscomm.send ("TCSREADY"); }
			else  {
				xtcscomm.send ("TCSERROR"); }
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"13",2)) { //set rate
			length-=3;
			xtcscomm.send ("*");
			if (set_setrate (length,xtcsbuf))    {
			*messages[1] = "Set Rate Received";
				xtcscomm.send ("TCSREADY"); }
			else {
				xtcscomm.send ("TCSERROR"); }
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"14",2)) { // offset rate. (not used?)
			xtcscomm.send ("*");
			set_offsetrate (length,xtcsbuf);
			*messages[1] = "Rate Received";
			xtcscomm.send ("TCSREADY");
		}
		else if (!strncmp (xtcsbuf,"15",2)) {  // cos dec
			xtcscomm.send ("*");
			set_cosdec (length,xtcsbuf);

			xtcscomm.send ("TCSREADY");
			length  = 0;
		}
		else if (!strncmp (xtcsbuf,"16",2)) { // rate corr
			xtcscomm.send ("*");
			set_rate_cor (length,xtcsbuf);
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"17",2)) { // set epoch
			xtcscomm.send ("*");
			set_epoch (length,xtcsbuf);
			*messages[1] = "Epoch Set";
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"18",2)) { // nothing
			xtcscomm.send ("*");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"19",2)) { // status
			xtcscomm.send ("*");
			length = 0;
			*messages[1] = "Getting Status";
			get_status (length,xtcsbuf,statusbuf);


			xtcscomm.send (statusbuf);
			xtcscomm.send ("  TCSREADY");
		 //	DosSleep (300);
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"20",2)) { // terse mode.
			length-=3;
			xtcscomm.send ("*");
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"\023",1)) {  // cancel stop
			//xtcscomm.send ("*");
			cancel_stop ();
			*messages[1] = "Stopping!";
		 //	xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"\007",1)) { // go...
			//xtcscomm.send ("*");
			go ();

			*messages[1] = "Initiating Move";
			//xtcscomm.send ("TCSREADY");
			length = 0;
		}


		else if (!strncmp (xtcsbuf,"\004",1)) {
/*
			xtcscomm.send ("*");
			init_to_zenith (length,xtcsbuf);
			message[1] = "Moving to Zenith";
			xtcscomm.send ("TCSREADY");
			length = 0;
*/
		}
		/*********************  Auto Focus Command Parse  *********************/
		/* Added 6/3/96 for auto focus.  Communication numbers 30, 31, 32,
			 and 33.
				30 #  Relative move of focus by given amount.
				31 #  Absolute move of focus to given location.
				32    Read focus encoder.
				33    Stop focus move.
			 Remember that 5 is the set focus command (see above).
		 */
		else if (!strncmp (xtcsbuf,"30\r",3)) { // focus relative move
			length-=3;
			xtcscomm.send ("*");
			if (move_focus_relative (length,xtcsbuf))
				*messages[1] = "Focus: Relative Move";
			else {
				*messages[1] = "Focus: Cannot move > 1000";
			}
			DosSleep (100);
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"31\r",3)) { // focus absolute move
			length-=3;
			xtcscomm.send ("*");

			if (move_focus_absolute (length,xtcsbuf))
				*messages[1] = "Focus: Absolute Move";
			else				*messages[1] = "Focus: Move out of range. +-1000";
			DosSleep (100);
			xtcscomm.send ("TCSREADY");
			length = 0;
		}
		else if (!strncmp (xtcsbuf,"32\r",3)) { // focus position read
			length-=3;
			xtcscomm.send ("*");
			read_focus_position (length,xtcsbuf,statusbuf);
			xtcscomm.send (statusbuf);
			xtcscomm.send ("   TCSREADY");
		 //	DosSleep (300);
			length = 0;
			*messages[1] = "Reading Focus Position";		}
		else if (!strncmp (xtcsbuf,"33\r",3)) {  // stop focus move
			//xtcscomm.send ("*");
			focus_cancel_stop ();
			*messages[1] = "Stopping Focus!";
		 //	xtcscomm.send ("TCSREADY");
			length = 0;
		}		else {
			length=0;
	//		xtcscomm.send ("TCSREADY");
		}

	}

}
void xtcs_close ()
{
	xtcscomm.send ("bye.");
}
/****************************  xtcslink process  ****************************/
/*
		process which checks for messages from the xtcs.
 */
void xtcslink_Process () {
//	clock_t starttime,stoptime;
//	char buf[256];
	while (all_run) {
		DosSleep (103);
//		starttime = clock ();
		xtcs_link ();
/*
		stoptime = clock ();
		if (stoptime == -1 || starttime  == -1) {
			debug.post ("error in timing");
		}
		else {
			sprintf (buf,"XTSLINK time:%2.2lf start %lf stop %lf",(double)(stoptime-starttime)/CLK_TCK,(double) starttime,(double)stoptime);
			debug.post (buf); ///"time:",(double)(stoptime-starttime)/CLK_TCK);
		}
*/


	}
}