/* -------------------------------------------------------------------
Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
------------------------------------------------------------------- */
/*
															Hooks.cpp

		 This file contains procdures which perform functions that an
	astronomer might be interested in changing.
			The hooks.cpp file contains astronomer related conversions and
	calculations.  The file is named hooks because it contains places
	where changes to thes calculations can be "hooked" into the code.
*/

#define INCL_DOS
#include <os2.h>
#include "const.h"
#include "skycalc.h"
#include "hiltner.h"

// Telescope model constants.  See Tpoint software.
/*
	ih=ha index.
	id=dec index.
	pdd=
	np=
	for polar axial misallignment:
		ma=
		me=

	ch=
	pdd2=
 */
double IH,ID,PDD,NP, MA,ME, CH, PDD2;

/***************************  get ut time  ******************************/
/*
		Get universal time is a routine which gets the current time as figured
	 by the Dell computer clock.  This clock is set by this program in
	 set_ut_time and is the current universal time.
 */
void get_ut_time (struct date_time  &LocalTime)
{
	DATETIME DateTime;

	DosGetDateTime (&DateTime);
	LocalTime.y = DateTime.year;
	LocalTime.mo = DateTime.month;
	LocalTime.d = DateTime.day;
	LocalTime.h = DateTime.hours;
	LocalTime.mn = DateTime.minutes;
	LocalTime.s = (float)(double)DateTime.seconds+
								(double)DateTime.hundredths/100.0;
}
int therc;
/***************************  set ut time  ******************************/
/*
		This routine sets the dell computer clock to the current universal time.
 */
void set_local_time (struct date_time &LocalTime)
{
	DATETIME DateTime;
	DosGetDateTime (&DateTime);
 //	DateTime.timezone = 0; // MST is +7 from GMT.
	DateTime.year = LocalTime.y;
	DateTime.month = LocalTime.mo;
	DateTime.day = LocalTime.d;
	DateTime.hours = LocalTime.h;
	DateTime.minutes = LocalTime.mn;
	DateTime.seconds = LocalTime.s;
/*
	DateTime.hundredths = (double)((double)LocalTime.s -
												(double) ((int) LocalTime.s)*100.0);
*/
	therc = DosSetDateTime (&DateTime);

}
/************************  get_dates_n_times  **************************/
/*
				Get the local time, figure the julian date and the sidereal
	 time in decimal hours.  The structure date_time is defined in
	 skycalc.h
				This routine uses skycalc.c written by John Thorstensen to calculate
	 the appropriate values. */
void get_dates_n_times (struct date_time &localtime,struct date_time
												&univtime,double &juliandate,
												double &siderealhours,double &currentepoch)

{
	 short tmp,weekday;
	 struct date_time tmpdate;

	// update universal time.
	get_ut_time (univtime);
	// figure out julian date.
	juliandate = date_to_jd (univtime);
	// figure out sidreal time in decimal hours.
	siderealhours = lst (juliandate,MDMHOURLONGITUDE);
	// figure local time (used only for display).
	caldat (juliandate-(7.0/24.0),&localtime,&weekday);

	// julian epoch.
	currentepoch = (juliandate-J2000)/365.25 + 2000.0;
}
/************************  calculate_airmass  **************************/
/*
				Calculate airmass was created by Bob Barr.
 */
void calculate_airmass (double &airmass,double &targetairmass)
{
	double ha = hatele*DEGREESTORADIANS*15.0;
	double dec = dectele*DEGREESTORADIANS;

	double targha = hatarget*DEGREESTORADIANS*15.0;
	double targdec = dectarget*DEGREESTORADIANS;

	airmass = 1.0/(
					sin (dec) * MDMLATITUDESIN +
					cos (dec) * cos (ha) * MDMLATITUDECOS
								);
	targetairmass = 1.0/(
					sin (targdec) * MDMLATITUDESIN +
					cos (targdec) * cos (targha) * MDMLATITUDECOS
								);}
/*****************  mean_place_to_telescope_coordinates  *****************/
/*
				Uses skycalc's cooxform to apply presession, nutation, and
	 abberation, and refraction.
 */
void mean_place_to_telescope_coordinates (
										double mean_ra,double mean_dec,double mean_epoch,
										double siderealhours,
										double &ha,double &dec,
										int refractflag,int justprecess)
{
	double haout,raout,decout,size;
	// ra in hours dec in degrees.
	// epochs in years, 1st epoch is catalog, 3rd epoch is now.
	// two flags, last is direction
	//  2nd to last: 0- nutation and aberrations.number of corrections
	//               1- just precess.
	cooxform (mean_ra,mean_dec,mean_epoch,currentepoch,&raout,&decout,
						 justprecess,1);

	haout = siderealhours-raout; // calculate hour angle.
	// size is the amount of correction.
	// flag of 1 is the direction of refraction correction.

	if (refractflag) {
		refract_corr (&haout,&decout,&size,MDMLATITUDE,MDMELEVATION,1);
	}
	ha = haout;
	dec = decout;
}
/*****************  telescope_coordinates_to_mean_place  *****************/
/*
				Uses skycalc's cooxform to un-apply presession, nutation, and
	 abberation, and refraction.
 */

void telescope_coordinates_to_mean_place (
											double ha,double dec,
											double siderealhours,
											double epoch,double &mean_ra,double &mean_dec,
											int refractflag,int justprecess)
{
	double rin,din,size;
	if (refractflag) {
		refract_corr (&ha,&dec,&size,MDMLATITUDE,MDMELEVATION,-1);
	}
	rin = siderealhours-ha;
	din = dec;
	cooxform (rin,din,epoch,currentepoch,&mean_ra,&mean_dec,justprecess,-1);
}

/*************************  set_guide_speeds  **************************/
/*
				Set guide speeds sets the guide speed, if necessary it corrects
		the RA speed according to the cosine of the declination.  Note that a
		"fast" speed is set to 2 times the guide speed.  This "fast" speed is
		used by the autoguider.
 */

void set_guide_speeds (double speed)
{
	if (cosdecflag) {
		if (cosdec > 	cos (88*DEGREESTORADIANS))
			RA.guidespeed = speed / cosdec;
		else
			RA.guidespeed = 80.0;
	}
	else
		RA.guidespeed = speed;
	DEC.guidespeed = speed;
	if (RA.guidespeed > 80.0) RA.guidespeed = 80.0;
	if (DEC.guidespeed > 80.0) DEC.guidespeed = 80.0;
	RA.fastspeed = RA.guidespeed*2.0;
	DEC.fastspeed = DEC.guidespeed*2.0;
	if (RA.fastspeed > 80.0) RA.fastspeed = 80.0;
	if (DEC.fastspeed > 80.0) DEC.fastspeed = 80.0;

}

/*************************  set_set_speeds  **************************/
/*
				Set the set speed, if necessary correct the RA by the cosine of
	 the declination.
 */
void set_set_speeds (double speed)
{
	if (cosdecflag) {
		if (cosdec > 	cos (60*DEGREESTORADIANS))
			RA.setspeed = speed / cosdec;
		else
			RA.setspeed = 80.0;
	}	else
		RA.setspeed = speed;
	DEC.setspeed = speed;
	if (RA.setspeed > 80.0) RA.setspeed = 80.0;
	if (DEC.setspeed > 80.0) DEC.setspeed = 80.0;
}
/************************  set_encoderpositions  **************************/
/*
			Sets the ra and dec telescope positions by changing the encoders.
 */
void set_encoderpositions ()
{
	 // Critical section...

	 // needs arcseconds.
	 RAdata.encoder->set_to_arcsec_location (haraw);
	 DECdata.encoder->set_to_arcsec_location (decraw);
}


//#define ARCSEC_IN_RADIAN 206264.806
#define ARCSEC_IN_HOUR 54000.;
#define ARCSEC_IN_DEGREE 3600.;


/**********************  apply_telescope_model  ************************/

/* Applies corrections to an hour angle (in arcseconds) and a 
declination (in arcseconds) to model for telescope imperfections.
It is assumed that telescope-independent corrections ....
precession, nutation, aberration, and refraction ... are
dealt with elsewhere.  The argument "sense" gives the sense of the
correction:

if sense == 1, applies correction to a true ha and dec (as derived
    from a catalog) to give the position to which the telescope should be
    aimed;  

if sense == -1, takes the coordinates derived from the telescope 
    encoders and gives an estimate of the true ha and dec. 
	
The coefficients used are not intended to be final ... in this 
first version, they derive from a "piggyback" pointing experiment
conducted by Thorstensen during normal observations in 1995 January.
They were derived from the pointing data through use of the starlink
TPOINT software.  

J. Thorstensen & Robb Lovell 1995 July

*/
void apply_telescope_model (double *teleha,double * teledec,int sense)
{
	if (nomodel) return;
   double radha, raddec, dha = 0., ddec = 0.;

 /* TPOINT pointing correction coefficients ...
	all are in arcsec.  Notation follows TPOINT.  */
#ifdef comment
   double
	IH = -89.42,   /* index error in hour angle */
		ID = 90.68,    /* index error in dec .. interacts with poly terms */
		PDD = -231.05, /* first order poly term in dec */
		NP = -6.14,    /* non-perpendicularity of HA and dec axes */
		MA = -3.47,    /* east-west error of polar axis */
		ME = 20.56,    /* north-south error of polar axis */
		CH = 64.52,    /* non-perpendicularity of dec and optical axes */
		PDD2 = 19.94;  /* 2nd order poly term in dec */
#endif
	 radha = *teleha * HOURSTORADIANS;
	 raddec = *teledec * DEGREESTORADIANS;
   
   dha += IH;
   
   ddec += ID;
   
	 ddec += PDD * raddec;/* here's hoping they wanted argument in radians */
   
   ddec += PDD2 * raddec * raddec;
   
   if(raddec < 1.562)  /* 89.5 degrees */
      dha += NP * tan(raddec);
   else dha += NP * tan(1.562);
   
   if(raddec < 1.562) /* 89.5 degrees again */
      dha += -1. * MA * cos(radha) * tan(raddec);
   else 
      dha += -1. * MA * cos(radha) * tan(1.562);
   
   ddec += MA * sin(radha);
   
   
   if(raddec < 1.562) 
      dha += ME * sin(radha) * tan(raddec);
   else dha += ME * sin(radha) * tan(1.562);
   
   ddec += ME * cos(radha);
   
   if(raddec < 1.562) 
      dha += CH / cos(raddec);
   else
      dha += CH / cos(1.562);

   printf("dha = %f, ddec = %f\n",dha,ddec);
   
   if(sense == 1) {  /* true coords -> telescope coords */
		*teleha -= dha/HOURSTOARCSEC;
		*teledec -= ddec/DEGREESTOARCSEC;
   }
	 else if(sense == -1) {  /* telescope coords -> true coords */
		*teleha += dha/HOURSTOARCSEC;
		*teledec += ddec/DEGREESTOARCSEC;
   }
	 else printf("error in point-model: illegal sense.\n");
}
/***********************  read telescope model.  ***********************/
/*
			 Reads 8 constants from a file.  These constants represent various
	 pointing error corrections due to telescope flexure and other parameters.
	 These parameters were calculated by the TPoint software authored by
	 Pat Wallace in England and modified for our use by John Thorstensen.
 */
void read_telescope_model (double &IH,double &ID,double &PDD,double &NP,
											double &MA,double &ME, double &CH, double &PDD2)
{
// below is the original model by John T, It is a rough stab with not very
// trustworthy data.  Its here for historical reasons?

#ifdef comment
//double
	IH = -89.42,   /* index error in hour angle */
		ID = 90.68,    /* index error in dec .. interacts with poly terms */
		PDD = -231.05, /* first order poly term in dec */
		NP = -6.14,    /* non-perpendicularity of HA and dec axes */
		MA = -3.47,    /* east-west error of polar axis */
		ME = 20.56,    /* north-south error of polar axis */
		CH = 64.52,    /* non-perpendicularity of dec and optical axes */
		PDD2 = 19.94;  /* 2nd order poly term in dec */
#endif
	FILE *fp;

	if ((fp = fopen ("c:\\tmodel.tcs","r"))!= NULL) {
		fscanf (fp,"%lf %lf %lf %lf %lf %lf %lf %lf",&IH,&ID,&PDD,&NP,&MA,&ME,&CH,&PDD2);

		fclose (fp);
	//	char buf[256];
	//	sprintf (buf,"%lf %lf %lf %lf %lf %lf %lf %lf ",IH,ID,PDD,NP,MA,ME,CH,PDD2);

	//	debug.post (buf);
	}
}
/***********************  save last position  ***********************/
/*
		 Save all encoder positions.
 */
#define COASTDISTANCE	4.5
extern double speeds[300],seconds[300];
extern double domecoastdistance;
void save_last_position ()
{

	FILE *fp;

	if ((fp = fopen ("c:\\encoders.tcs","w+"))!= NULL) {
		fprintf (fp,"%lf %lf %lf %lf %lf\n",
							ra_encoder.arcseclocation,dec_encoder.arcseclocation,
							dome_encoder.arcseclocation,
							focus_encoder.rawlocation,
							inst_encoder.arcseclocation
							 );
		fclose (fp);

	}
	if ((fp = fopen ("c:\\domemem.tcs","w+"))!= NULL) {

		for (int i=0;i<300;i++) {
			fprintf (fp,"%lf %lf ",speeds[i],seconds[i]);
		}
		fprintf (fp,"%lf",domecoastdistance);
		fclose (fp);

	}

}

/***********************  restore last position  ***********************/
/*
		 Restore all encoder positions.
 */
int restore_last_position (double &rapos,double &decpos,
													 double &domepos,double &focuspos,double &instpos)
{

	FILE *fp;
	int thereturn = 0;
	if ((fp = fopen ("c:\\encoders.tcs","r"))!= NULL) {
		fscanf (fp,"%lf %lf %lf %lf %lf",
							&rapos,&decpos,&domepos,&focuspos,&instpos
							 );

		fclose (fp);
		thereturn = 1;
	}
	if ((fp = fopen ("c:\\domemem.tcs","r"))!= NULL) {

		for (int i=0;i<300;i++) {
			fscanf (fp,"%lf %lf ",&speeds[i],&seconds[i]);
		}
		fscanf (fp,"%lf",&domecoastdistance);
		fclose (fp);

	}
	else {
		for (int i=0;i<300;i++) {
			speeds[i]=seconds[i]=0.0;
		}

		domecoastdistance = COASTDISTANCE;	}	return thereturn;
}

/********  set_encoders_telescope_coordinates_to_raw_coordinates  ********/
/*
		Convert from HA and DEC in units of hours and degrees to raw telescope
	coordinates of arcseconds with 0,0 at zenith.

	This routine is identical to telescope_coordinates_to_raw_coordinates
	except for cases around the pole.  In principle the encoders should never
	be set while observing around the pole, but just in case they are, this
	routine handles this case by assuming that the current location is
	close to where the actual position is located.  That is if the
	current location is above the pole, the new encoder location is above
	the pole.
 */
int set_encoders_telescope_coordinates_to_raw_coordinates
																 (double teleha,double teledec,
																	double &harawarcsec,double &decrawarcsec)
{
	apply_telescope_model (&teleha,&teledec,1);
	if (teledec > 60.0) {
		if (decrawarcsec > 90.0*DEGREESTOARCSEC) {
			// assume the dec is being set to a value under the pole.
			 teledec = 90.0 + (90.0-teledec);
			// ha > west or ha < east then under the pole.
			if (teleha*HOURSTOARCSEC > RA.limit.revmax ||
					teleha*HOURSTOARCSEC < RA.limit.fwdmax) {
				// not passed by reference...
				teleha -= 12;
				adj_time_ha (teleha);
			}
		}
		// ha > west or ha < east then under the pole.
		if (teleha*HOURSTOARCSEC > RA.limit.revmax ||
				teleha*HOURSTOARCSEC < RA.limit.fwdmax) {
			//problem with coordinates don't do anything...
			return 0;
		}
	}
	harawarcsec = teleha*HOURSTOARCSEC;
	decrawarcsec = (teledec-MDMLATITUDE)*DEGREESTOARCSEC;
	return 1;
}
/**************  telescope_coordinates_to_raw_coordinates  ***************/
/*
 */
void telescope_coordinates_to_raw_coordinates (double teleha,double teledec,
																	double &harawarcsec,double &decrawarcsec)
{
	apply_telescope_model (&teleha,&teledec,1);

 //	double currentha = harawarcsec/HOURSTOARCSEC;

	double currentdec = decrawarcsec/HOURSTOARCSEC;
	if (RA.limit.inside_softlimits (teleha*HOURSTOARCSEC)) {
		//current dec > 90.0 then assume that
		if (inside_ra_ambiguous_zone (teleha) && currentdec > 90.0) {
			teledec = 90.0 + (90.0-teledec);
			teleha -= 12;
			adj_time_ha (teleha);
		}

	}
	else {
		teledec = 90.0 + (90.0-teledec);
		teleha -= 12;
		adj_time_ha (teleha);
	}
	harawarcsec = teleha*HOURSTOARCSEC;
	decrawarcsec = (teledec-MDMLATITUDE)*DEGREESTOARCSEC;

}
/**************  raw_coordinates_to_telescope_coordinates  ***************/
/*
 */

void raw_coordinates_to_telescope_coordinates (double harawarcsec,
		 double decrawarcsec,double &teleha,double &teledec)
{
	if (decrawarcsec/DEGREESTOARCSEC+MDMLATITUDE > 90.0) {
		teleha = harawarcsec/HOURSTOARCSEC + 12.0;
		teledec = 90.0- ((decrawarcsec/DEGREESTOARCSEC+MDMLATITUDE)-90.0);	}
	else {
		teleha = harawarcsec/HOURSTOARCSEC;
		teledec = decrawarcsec/DEGREESTOARCSEC+MDMLATITUDE;
	}
/* the above essentially does the transformation and then asks the
	 following...
	if (teledec > 90.0) {  // is it greator than 90 degrees?
		teleha += 12; // we are above the pole so change ha and dec accordingly.
		teledec = 90.0-(teledec-90.0);
	}
*/
	apply_telescope_model (&teleha,&teledec,-1);
}
/***************************  set_coordinates *****************************/
/*
		 Set coordinates, set the target location.
*/
void set_coordinates ()
{
	 mean_place_to_telescope_coordinates (rameantarget,decmeantarget,targetepoch,
								siderealhours,hatarget,dectarget,1,0);
	 telescope_coordinates_to_raw_coordinates (hatarget,dectarget,
																						 harawtarget,decrawtarget);
			RAdata.axis->target = HA_target = harawtarget;
			DECdata.axis->target = DEC_target = decrawtarget;

}
/***************************  ratecorr_size  ****************************/
/*

	 Units are decimal hours, decimal degrees x 2, meters; returns rate   corrections in arcsec per second in the two axes. 

	 Given an hour angle and a dec -- which need not be precise --
	 returns the correction to the drive rates in hour angle and
	 declination.  Works by numerical differentiation.

	-- sets up "true" HA and dec at 1/2 of interval before 
		and after the input values;
	-- applies flexure and refraction to these two pairs;
	-- compares differences in HA and dec to those of the true
           coordinate pairs;
	-- derives rate at which the star appears to drift.

	 Routine ignores precession, nutation, and aberration, which
	 change very slowly compared to the refraction and flexure.

*/
void ratecorr_size(double ha,double dec,double  lat,double  elev,
									 double * haratecorr,double * decratecorr)
{
	double interval = 0.01 ; /* of a siderial hour ... 
			for differentiation */
	double ha1, ha2;  /* "true" ha before and after ... true decs
              do not change. */ 
	double ha1appar, dec1appar;  /* input ha and dec after
              refraction and flexure correction */ 
	double ha2appar, dec2appar;
	double size;  /* of refraction corrn */
	double diffha, diffdec;

	ha1 = ha - 0.5 * interval;
	ha2 = ha + 0.5 * interval;
	
	ha1appar = ha1; dec1appar = dec;  /* copy */
	refract_corr(&ha1appar, &dec1appar, &size, lat, elev, 1);
 //	printf("ha1 %lf dec %lf  refract: %lf %lf\n",
 //		ha1,dec,ha1appar,dec1appar);
	apply_telescope_model(&ha1appar, &dec1appar, 1);
 //	printf("flexed: %lf %lf\n", ha1appar,dec1appar);

	ha2appar = ha2; dec2appar = dec;  /* copy */
	refract_corr(&ha2appar, &dec2appar, &size , lat, elev, 1);
//	printf("ha2 %lf dec %lf  refract: %lf %lf\n",
//		ha2,dec,ha2appar,dec2appar);
	apply_telescope_model(&ha2appar, &dec2appar, 1);
 //	printf("flexed: %lf %lf\n", ha2appar,dec2appar);
	
	diffha = (ha2appar - ha1appar - interval) * ARCSEC_IN_HOUR;

	diffdec = (dec2appar - dec1appar) * ARCSEC_IN_DEGREE;
	
	//printf("Differences in ha and dec (arcsec): %lf %lf\n",
	//	diffha, diffdec);
	
	/* put out rate corrs in arcseconds per second. */
 
	*haratecorr = (diffha / (interval * 3600));// * ADJUSTEDSIDEREALRATE);
	double thedecratecorr = diffdec / (interval * 3600);// *  ADJUSTEDSIDEREALRATE);
	if (thedecratecorr > 0.01)
		*decratecorr = thedecratecorr;
	else *decratecorr = 0.0;
	/* the 1.0027379093 is ridiculous ... takes account of fact that
					 computation interval is "interval" sidereal hours, not
           normal hours.  */

}




