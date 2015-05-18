/* -------------------------------------------------------------------
Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
------------------------------------------------------------------- */
#include <string.h>
#include "telescpe.h"
#include "skycalc.h"
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  Limits  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*
  Limits class functions.
 */
/**************************  limits constructor  *************************/
Limits::Limits (DTBoard *board,unsigned int port,
unsigned int bit5f,unsigned int bit5r,unsigned int bit8f,
unsigned int bit8r,double fwd,double rev,double _minslew,
double _maxslew)
			: fwd5((DTIO*)board,port,bit5f,ACTIVEHIGH),
			  rev5((DTIO*)board,port,bit5r,ACTIVEHIGH),
			  fwd8((DTIO*)board,port,bit8f,ACTIVEHIGH),
			  rev8((DTIO*)board,port,bit8r,ACTIVEHIGH)
{
	// given in arcseconds.
	maxslew = _maxslew;
	minslew = _minslew;
	fwdmax = fwd;
	revmax = rev;
	limitstate = 0;
	softwarelimit = 0;
}
/*******************************  readstatus *********************************/
/*
	Readstatus reads the limit hardware switches.
 */
void Limits::readstatus ()
{
			fwd5.read_state ();
			rev5.read_state ();
			fwd8.read_state ();
			rev8.read_state ();
}
/*******************************  limits *********************************/
/*
    Returns the state of the limit switches.
 */
int Limits::limits ()
{
		 readstatus ();
		 if (rev5.on ()) return -5;
		 else if (rev8.on ()) return -8;
		 else if (fwd5.on ()) return 5;
		 else if (fwd8.on ()) return 8;
		 return 0;
}
/*
	   Returns the state of hardware and software limit switches.
    The variable softwarelimit is set externally from the limits
    class data type through the function externallimit ().
    Externallimt is called in the timeproc.cpp file evrey 2ms.
 */
int Limits::limits (int direction)
{

		readstatus ();
//char buf[256];
//sprintf (buf,"%d %d %d %d direction:%d  revmax:%lf fwdmax:%lf  ",
//fwd8.on (),rev8.on (),fwd5.on (),rev5.on (),direction,revmax,fwdmax);
//debug.post (buf);debug.dout (buf);
		switch (direction) {
			case REVERSE:
				if (rev5.on ()) return -5;
				else if (rev8.on ()) return -8;
				else if (softwarelimit)  return -1;
				else return 0;
			case FORWARD:
				if (fwd5.on ()) return 5;
				else if (fwd8.on ()) return 8;
				else if (softwarelimit)  return 1;
				else return 0;
		}
		return 0;
}
/**********************************  limits (OBSOLETE)( *****************/
/* OBSOLETE
	   Returns the state of hardware and software limit switches.
    The variable softwarelimit is set externally from the limits
    class data type through the function externallimit ().
    Externallimt is called in the timeproc.cpp file evrey 2ms.
    This function is identical to the above function because it
    the way limits are checked changed while the program was being
    written.  The position of the telescope used to be passed
    to this function so that a soft limit could be tested.  This
    functionality was moved to the timeproc.cpp file.
 */
int Limits::limits (double ,int direction)   // OBSOLETE

{
	readstatus ();
//char buf[256];
//sprintf (buf,"%d %d %d %d direction:%d,  revmax:%lf fwdmax:%lf  ",
//	fwd8.on (),rev8.on (),fwd5.on (),rev5.on (),
//   direction,revmax,fwdmax);
//debug.post (buf);debug.dout (buf);

		switch (direction) {
			case REVERSE:
				if (rev5.on ()) return -5;
				else if (rev8.on ()) return -8;
				else if (softwarelimit) return -1;
				else return 0;
			case FORWARD:
				if (fwd5.on ()) return 5;
				else if (fwd8.on ()) return 8;
				else if (softwarelimit)  return 1;
				else return 0;
		}
		return 0;
}
/*****************************  external limit ******************************/
/*
	software limits need to be checked very quickly while the telescope is
  moving.  The time process in timeproc.cpp calls this function when the
  telescope has moved outside of the soft limits.
 */
void Limits::externallimit (int reached_limit) {
	softwarelimit = reached_limit;
}
/****************************  inside raw limits *****************************/
/*
	A test based on altitude for limits.
 */
int inside_rawlimits (double rawha,double rawdec)
{
	 /*
				 raw ha and dec are in the coordinate system with
			 zenith at 0,0.  They are in arcseconds.
		*/
	 double ateleparallactic,ateleazimuth,atelealtitude = altit (
					 rawdec/DEGREESTOARCSEC+MDMLATITUDE,
					 rawha/HOURSTOARCSEC,
					 MDMLATITUDE,&ateleazimuth,&ateleparallactic);
//char buf[256];
//sprintf (buf,"ha:%lf   dec:%lf   %lf %lf  \n", rawha/HOURSTOARCSEC,
//	rawdec/DEGREESTOARCSEC+MDMLATITUDE,telealtitude,teleazimuth);
//debug.post (buf);

	 return (atelealtitude > 8.0 && fabs (rawha/HOURSTOARCSEC) < HARAWLIMIT);

}
/*************************  inside ambiguous zone  *************************/
/*
	If the softlimits overlap at the pole, then this function is used
   to see if the telescope is in the overlapped region.  (overlapp occurs
   when the telescope can move to +-7 hrs from zenith and +-7 hrs from under
   the pole.)  Currently there is no overlap region used and the maximum
   deflection is +-6 hours from zenith.
 */
int inside_ra_ambiguous_zone (double targetha)
{
	 return ((targetha > HARAWLIMITREFLECT6 && targetha < HARAWLIMIT) ||
		    (targetha < -HARAWLIMITREFLECT6 && targetha > -HARAWLIMIT));
}
/***************************  inside soft limits ****************************/
// location given in arcseconds...
int Limits::inside_softlimits (double location)
{
	if (revmax < 0)
	 return location > revmax && location < fwdmax;
	else
	 return location > fwdmax && location < revmax;
}
/***************************  outside soft limits ****************************/
int Limits::outside_softlimits (double location)
{
	if (revmax < 0)
	 return location <= revmax && location >= fwdmax;
	else
	 return location <= fwdmax && location >= revmax;
}
/******************************  maxspeed  ***********************************/
/*
	The maxspeed is used to determine the maximum slew speed allowed given a
  direction of travel and the state of the limit switches and software limits.
  Used to quickly determine a maximum slew speed.
 */
double Limits::maxspeed (int direction)
{

		limitstate = limits (direction);
		if (limitstate == 1 || limitstate == -1) {
		 return 0.0;
		}
		else if (limitstate == 5 || limitstate == -5) {
		 return 0.0;
		}
		else if (limitstate == 8 || limitstate == -8) {
		 return minslew;
		}
		else
		 return maxslew;
}
/**************************** maxspeed (OBSOLETE) ****************************/
/* OBSOLETE: This function is not used...
	The maxspeed is used to determine the maximum slew speed allowed given a
  direction of travel and the state of the limit switches and software limits.
 */
double Limits::maxspeed (double location,int direction)
{
		// location not used in limits function.
		limitstate = limits (location,direction);
		if (limitstate == 1 || limitstate == -1) {
		 return 0.0;
		}
		else if (limitstate == 5 || limitstate == -5) {
		 return 0.0;
		}
		else if (limitstate == 8 || limitstate == -8) {
		 return minslew; //LIMITSSLEWSPEED;
		}
		else
			return maxslew; //MAXSLEWSPEED;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  Axis Class  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/***************************** Axis Constructor *****************************/
/*
 */
Axis::Axis  (DT2819 *ctboard,DT2816 *adboard,unsigned int dac,int cd,
				 DT2819 *limitboard,
				 int limitbit5f,int limitbit5r,int limitbit8f,int limitbit8r,
				 double minslew,double maxslew,
				 DT2819 *clockboard,
				 int clocksource,
				 double _trackspeed,double speedfactor,double stepsize,
				 double fwdmax,double revmax,int polarity,char *_which,
				 double s1,double s2,double s3,double s4,double s5,double s6)
		// both RA and DEC use port 0 and bits 0,2,3, & 4.
	 :fwdrev (ctboard,0,0,ACTIVEHIGH),
		clockfout (clockboard,clocksource,0),
		clutch (ctboard,0,2,ACTIVEHIGH),
		slewenable (ctboard,0,3,ACTIVEHIGH),
		//stepenable (ctboard,0,1,ACTIVELOW),
		dcfwdrev (ctboard,0,4,ACTIVEHIGH), // port 0, bit 4.
		dcmtr (adboard,dac,dcfwdrev), // dac 0.
		stepmtr (fwdrev,clockfout,stepsize,polarity),
		limit (limitboard,1,limitbit5f,limitbit5r,limitbit8f,limitbit8r,
					 fwdmax,revmax,minslew,maxslew)
{

		strcpy (which,_which);
		clutchdelay = cd;
		if (clutchdelay < 1000.0) clutchdelay = 1000.0;
		maxslewspeed = 0; //limit.maxspeed (); // in volts.
		slewspeed = 0;
		stepperdirection = 1;
		moving = 0;
		clutchstate = OFF;
		slewstate = OFF;
		slewenable.turn_off ();
		//stepenable.turn_off ();
		disengage_clutch ();
		stepmtr.speed_at_direction (0.0,1);

		// speed is in arcseconds/second.
		trackspeed = _trackspeed;
		setspeed = DEFAULTSETSPEED;
		guidespeed = DEFAULTGUIDESPEED;
		fastspeed = DEFAULTFASTSPEED;
		offsetspeed = DEFAULTOFFSETSPEED;

	moveminslewspeed=s1;moveslowslewspeed=s2;
	movehalfslewspeed=s3;movemaxslewspeed=s4;
	movesetspeed=s5*speedfactor;moveguidespeed=s6*speedfactor;

}

/******************************  engage clutch  *****************************/
void Axis::engage_clutch () {
		dcmtr.speed (0.0);
		clutch.turn_on ();
}
/****************************  disengage clutch  *****************************/
void Axis::disengage_clutch () {
		dcmtr.speed (0.0);
		DosSleep (clutchdelay);
		clutch.turn_off ();
}
/********************************** slew  ********************************/
/*
	 Slew moves either the RA or DEC in the direction given.  If the
  axis is currently moving, care is taken to stop before reversing directions.
  Also, the cluthc needs to be engaged before the slew takes place.  Note that
  the engage and disengage functions stop the motor before acting on the clutch.
 */
void Axis::slew (int direction) { // direction is 1 or -1.

		if (slewstate == OFF || direction != slewstate) {
			if (clutchstate  == OFF) {
				engage_clutch  ();
				clutchstate = ON;
			}
			if (slewstate != direction) dcmtr.speed (0.0);
			slewstate = direction;
			dcmtr.set_direction (direction);
			slewenable.turn_on ();
			// check limits for valid speed.
			maxslewspeed = limit.maxspeed (direction);
			slewspeed = maxslewspeed;
			dcmtr.speed ((double) slewspeed);

		}
}
/*******************************  slew slowup  ********************************/
/*
	Slow the slew speed by a certain amount.
 */
void Axis::slew_slowup (int byspeed) {
		if (slewstate != OFF) {
			slewspeed -= byspeed;
			if (slewspeed < 1) slewspeed = 1;
			else if (slewspeed > maxslewspeed) slewspeed = maxslewspeed;
			dcmtr.speed ((double) slewspeed );
		}
		//debug2 = clutchstate;
}
/****************************  Slew Stop  *********************************/
/*
    stop slewing and disengage the clutch.
 */
void Axis::slew_stop () {
		if (slewstate != OFF) {
			dcmtr.speed (0.0);
			slewenable.turn_off ();
			if (clutchstate) {
				disengage_clutch ();
				clutchstate = OFF;
			}
			slewstate = OFF;
			slewspeed = 0;
		}
}
/*****************************  Stop  ***************************************/
/*
  Stop all motors.
 */
void Axis::stop () {
		slew_stop ();
		stepmtr.stop (0.0);
		slewspeed = 0;
}
/******************************** check limits *******************************/
/*
	Determines the maximum slew speed allowed given the state of the limits.
 */
void Axis::check_limits (double ) {

		maxslewspeed = limit.maxspeed(slewstate);

		// chech the extreme case...
		if (slewspeed > maxslewspeed) {
		 changeslew (maxslewspeed);
		 slewspeed = maxslewspeed;
		}
//char buf[256];
//sprintf (buf,"8:%d %d 5:%d %d state:%d  spd:%lf < max:%lf",
//	 limit.fwd8.on (),limit.rev8.on (),limit.fwd5.on (),limit.rev5.on (),
//	 slewstate,slewspeed,maxslewspeed);
//debug.post (buf);
}
/*****************************  slew direction ******************************/
/*
	Used in conjunction with startslew (), start slew prepares for slewing
  and this function sets the slew in motion.
 */
void Axis::slewdirection (int direction) { // direction is 1 or -1.
		if (slewstate == OFF || direction != slewstate) {
			if (clutchstate  == OFF) {
				engage_clutch  ();
				clutchstate = ON;
			}
			if (slewstate != direction) dcmtr.speed (0.0);
			dcmtr.set_direction (direction);
			slewenable.turn_on ();
			slewstate = direction;
		}
		maxslewspeed = limit.maxspeed(direction);
}
/******************************  step direction ******************************/
// not used....
void Axis::stepdirection (int direction) { // direction is 1 or -1.
	//stepmtr.direction (direction);
	stepperdirection = direction;
 //	trackspeed = fabs(trackspeed) * direction;
}
/******************************  change slew  ********************************/
void Axis::changeslew (double _speed) {
		maxslewspeed = limit.maxspeed(slewstate);
		if (_speed > maxslewspeed) _speed = maxslewspeed;
		dcmtr.speed ((double) _speed );
		slewspeed = _speed;
}
/******************************  start slew  ********************************/
/*
	Prepare for slewing in a particular direction.
 */
void Axis::startslew () {
		dcmtr.start_speed_change ();
}
/*@@*/
/*----------------------------  DOME CLASS  --------------------------------*/

/**/
/*************************  dome Constructor  *******************************/
Dome::Dome (DTBoard *board1,DTBoard *board3,DT2816 *adboard,Encoder *enc)
	 : cw ((DTIO*) board1,0,6,ACTIVELOW),ccw((DTIO*) board1,0,7,ACTIVELOW),
		 enable ((DTIO*) board3,1,7,ACTIVEHIGH),mtr (adboard,2,cw,ccw)

{
		encoder = enc;
		maxspeed = 8.0;
		stop ();
}
/******************************  speed check  ****************************/
void Dome::speedcheck (double &speed)
{
		if (speed > maxspeed) speed = maxspeed;
		else if (speed < 0) speed = 0;
}
/*****************************  move ccw  *******************************/
void Dome::move_ccw (double speed)
{
		if (domestate != CCW) {
			stop ();
			DosSleep (2000);
		}
		DosSleep (100);
		mtr.direction (-1);
		enable.turn_on ();
		DosSleep (30);
		speedcheck (speed);
		mtr.speed (speed);
		domestate = CCW;
}
/********************************  move cw  *******************************/
void Dome::move_cw (double speed)
{
		if (domestate != CW) {
			stop ();
			DosSleep (2000);
		}
		DosSleep (100);
		mtr.direction (1);
		enable.turn_on ();
		DosSleep (30);
		speedcheck (speed);
		mtr.speed (speed);
		domestate = CW;
}
/************************  calculate speed n seconds  ************************/
/*
	A linear function for calcuating what speed and for how long the dome is
   activated to move a particular distance.  The function was obtained through
   empirical tests.  (volts in this case represents speed).  The timing is
   calculated to within a tolerance value of +-1 second down to 0.2 seconds if
   necessary.  The routine searches for a good voltage and timing starting with
   a low voltage and a longer time interval and moves to higher voltages and
   shorter time intervals.
 */
double Dome::calculate_speed_n_seconds
					    (double distance,double &volts,double &seconds)
{
	double tolerance,speed;

		for (tolerance = 0.2; tolerance < 1.0; tolerance += 0.1) {
			for (volts = 2.0; volts < 6.0; volts += 1.0) {
				speed = 1.0 / (volts * 0.60) + volts*0.009;
				seconds = distance*speed;
				if (seconds >= 0.25 &&
				    fabs(seconds-(double)(int)(seconds + 0.5)) > tolerance )
				  return speed;
			}
		}
		// couldn't find a speed and time interval, assume a small move.
		volts = 3;
		speed = 1.0 / (volts * 0.65) + volts*0.007;
		seconds = 0.2;
		return speed;

}

/*****************************  move distance  *******************************/
/*
	Move a given distance and direction.  This routine uses "calculate_speed_
  n_seconds" to calculate a speed and time interval and then moves the dome
  based on this calculation.  The dome is set into motion by setting the
  voltage on the motor and then dropping into a timing loop for the given
  amount of time.
 */
void Dome::move_distance (double distance,int domedirection,
					 double &speed,double &volts,double &seconds)
{
#define FULLSPEEDIX 0
	double lastspeed,lasterror,lastdistance;
	//	double speeds [100];
	clock_t start_time,stop_time;

	speed = calculate_speed_n_seconds (distance,volts,seconds);
					// char buf[256];
					// sprintf (buf,"volts:%lf, sec:%lf dist:%lf dir:%d",
					// volts,seconds,distance,domedirection);
					// debug.post (buf);
	move (volts,domedirection);
	// start timing...
	stop_time = clock ();
	start_time = clock ();
	while (stop_time-start_time < seconds*CLK_TCK) {
			//				sprintf (buf,"volts:%lf, sec:%lf dist:%lf dir:%d",
			//				volts,seconds,distance,domedirection);
			 //				debug.post (buf);		DosSleep (200);
		stop_time = clock ();
	}
	stop ();
}
/*
	Move a given distance and direction.  This routine is given a speed and
  time interval (rather than calculating these values) and then moves the dome
  based on this calculation.  The dome is set into motion by setting the
  voltage on the motor and then dropping into a timing loop for the given
  amount of time.
 */
void Dome::move_distance (double volts,double seconds,int domedirection)
{
#define FULLSPEEDIX 0
	double lastspeed,lasterror,lastdistance;
	    //	double speeds [100];
	clock_t start_time,stop_time;


				    //	 char buf[256];
					//sprintf (buf,"volts:%lf, sec:%lf dir:%d",
					 //volts,seconds,domedirection);
					// debug.post (buf);
	move (volts,domedirection);
	// start timing...
	stop_time = clock ();
	start_time = clock ();
	while (stop_time-start_time < seconds*CLK_TCK) {
						//	sprintf (buf,"volts:%lf, sec:%lf  dir:%d",
						//	volts,seconds,domedirection);
						 //	debug.post (buf);		DosSleep (200);
		stop_time = clock ();
	}
	stop ();
}
/*********************************  move  **********************************/
/*
	   Move starts the telescope moving in a given direction at the given
   speed.  The dome continues to move after the routine has returned.
 */
void Dome::move (double speed,int direction)
{
	 if (direction == 0) {
			stop (); return;
	 }

		if (domestate != direction) {
			stop ();
			DosSleep (2000);
		}
		DosSleep (100);
		mtr.direction (direction);
		enable.turn_on ();
		DosSleep (30);
		speedcheck (speed);
		mtr.speed (speed);
		domestate = direction;
}
/********************************  stop  ************************************/
/*
	  Stop stops the dome by writing a 0 voltage to the motor.
 */
void Dome::stop ()
{
		mtr.speed (0.0);
		enable.turn_off ();
		domestate = 0;
}


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  Front Panel Class  %%%%%%%%%%%%%%%%%%%%%%%%%*/
FrontPanel::FrontPanel (DTBoard *board2,DTBoard *board3)
		 :	drivesonoff_ ((DTIO*)board2,1,4,ACTIVELOW),
				autodome_ ((DTIO*)board2,1,1,ACTIVELOW),
				autoguide_ ((DTIO*)board2,1,0,ACTIVELOW),
				domehome_ ((DTIO*)board2,1,2,ACTIVELOW),  // front panel go.
				track_ ((DTIO*)board2,1,5,ACTIVELOW)  // auto guide.
				//executego_ ((DTIO*)board3,1,0,ACTIVELOW), // not used
				//stop_ ((DTIO*)board3,1,1,ACTIVELOW) // not used
{}
/***************************  readstatus  *******************************/
/*
	 Reads the status of the frontpanel switches.
 */
void FrontPanel::readstatus ()
{
			drivesonoff_.read_state ();
			autodome_.read_state ();
			autoguide_.read_state ();
			domehome_.read_state ();
			track_.read_state ();
			//executego_.read_state ();
			//stop_.read_state ();
}
/*******************************  notused  ***********************************/
int FrontPanel::drives () { return drivesonoff_.thestate (); }
int FrontPanel::autodome () { return autodome_.thestate (); }
int FrontPanel::autoguide () { return autoguide_.thestate (); }
int FrontPanel::track () { return track_.thestate (); }

/*------------------------------  FOCUS CLASS  ------------------------------*/
Focus::Focus (DTBoard *board2, Encoder *enc)
		: fwdrev ((DTIO*)board2,0,7,ACTIVELOW),
			enable ((DTIO*)board2,0,1,ACTIVELOW),
			fast_guide ((DTIO*)board2,0,6,ACTIVEHIGH),
			fwdlimit ((DTIO*)board2,1,6,ACTIVELOW),
			revlimit ((DTIO*)board2,1,7,ACTIVELOW)
{
		encoder = enc;
		stepspeed = 1;
		guidespeed = 2;
		fastspeed = 3;
		stop ();
		fast_guide.turn_off ();
}
/*********************************  focusin  *******************************/
void Focus::focusin () {
		fwdrev.turn_off ();
		enable.turn_off ();
}
/*********************************  focusout  ******************************/
void Focus::focusout () {
		fwdrev.turn_on ();
		enable.turn_off ();
}
/**********************************  stop  *********************************/
void Focus::stop () {
		enable.turn_on ();
}
/**********************************  speeds  *******************************/
void Focus::set_guide () {
		stepspeed = guidespeed;
		fast_guide.turn_on ();
}
void Focus::set_fast () {
		stepspeed = fastspeed;
		fast_guide.turn_off ();
}