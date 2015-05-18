#define INCL_DOS
#include <os2.h>
#include "const.h"
#include "skycalc.h"
#include <iostream.h>

// the target location
extern double HA_target,DEC_target;

extern int cosdecflag;
extern double cosdec,lastcosdec;
extern double setrate,guiderate;
extern int ratecorflag;

// time of day and other time formats.
// date_time structure is declared in skycalc.h
extern struct date_time univtime,local_time;
extern struct date_time siderealtime;
extern double juliandate;
extern double siderealhours;
extern double airmass;
// current epoch is the current time.
// standard epoch is the epoch of coordinates used by the astronomer.
extern double currentepoch,stdepoch;
// current location of the telescope.
extern double ramean,decmean,rameantarget,decmeantarget; // catalog locations.
extern struct coord meanra,meandec,targetha,targetdec,targetramean,targetdecmean,teleha,teledec,apparentra,apparentdec;

extern double hatele,dectele,hatarget,dectarget; // ra 0 at zenith,  dec 0 at north star.
extern double haarcsec,decarcsec;  //
extern double haraw,decraw,harawtarget,decrawtarget;  //
extern double juliandate;
extern double siderealhours;
// current epoch is the current time.
// standard epoch is the epoch of coordinates used by the astronomer.
extern double currentepoch,stdepoch;
// current location of the telescope.
extern double ramean,decmean;
// not used...extern double raapparent,decapparent; // ra 0 at zenith,dec 0 at north star.
extern double raarcsec,decarcsec;  // 0,0 at zenith
extern double raraw,decraw;  // 0,0 at zeinth.
/*
		 This file contains procdures which perform functions that an
	astronomer might be interested in changing.
*/

/*
		 routine to get the current time as figured by the computer's
	 clock.
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
/*
	 routine to set the computer's clock.
 */
void set_local_time (struct date_time &LocalTime)
{
	DATETIME DateTime;
	//DateTime.timezone = 7; // MST is +7 from GMT.
	DateTime.year = LocalTime.y;
	DateTime.month = LocalTime.mo;
	DateTime.day = LocalTime.d;
	DateTime.hours = LocalTime.h;
	DateTime.minutes = LocalTime.mn;
	DateTime.seconds = LocalTime.s;
	DateTime.hundredths = (double)((double)LocalTime.s -
												(double) ((int) LocalTime.s)*100.0);
	DosSetDateTime (&DateTime);
}
/*
	 Get the local time, and figure the julian date and the sidereal
	 time in decimal hours.  The structure date_time is defined in
	 skycalc.h
 */
//extern "C" {
//	 extern double slaGmst ( double ut1 );
//}
void get_dates_n_times (struct date_time &localtime,struct date_time
												&univtime,double &juliandate,
												double &siderealhours,double &currentepoch)

{

	 short tmp,weekday;
	 struct date_time tmpdate;
		// this method uses skycalc.c written by John Thorstensen to calculate
		// the appropriate values.
							// update universal time.

	get_ut_time (univtime);							// figure out julian date.

	juliandate = date_to_jd (univtime);
	//correct_time (&univtime);
	// figure out sidreal time in decimal hours.
	siderealhours = lst (juliandate,MDMHOURLONGITUDE);

	caldat (juliandate-(7.0/24.0),&localtime,&weekday);

	// julian epoch.
	currentepoch = (juliandate-J2000)/365.250 + 2000.0;

}
void calculate_airmass (double &airmass)
{
	double ha = hatele*DEGREESTORADIANS*15.0;
	double dec = dectele *DEGREESTORADIANS ;

	airmass = 1.0/(
					sin (dec) * MDMLATITUDESIN +
					cos (dec) * cos (ha) * MDMLATITUDECOS
								);
}
/*
	 Uses skycalc's cooxform to apply presession, nutation, and
	 abberation, uses refract_coor to correct for refraction.

 */
 //	 mean_place_to_telescope_coordinates (rameantarget,decmeantarget,stdepoch,
 //								juliandate,siderealhours,hatarget,dectarget,1,0);

void mean_place_to_telescope_coordinates (
										double mean_ra,double mean_dec,double mean_epoch,
										double jd,double siderealhours,
										double &ha,double &dec,
										int refractflag,int justprecess)
{
	double haout,raout,decout,size;
	// ra in hours
	// dec in degrees.
	// epochs in years
	// 2nd epoch 4th arg epoch of now.  years.
	// two flags, last is direction
	//  2nd to last: 0- nutation and aberrations.number of corrections
	//               1- just precess.
	//cerr <<"julian date:"<<jd<<endl;
	cooxform (mean_ra,mean_dec,mean_epoch,currentepoch,&raout,&decout,
						 justprecess,1);
	//cerr <<"after precess & abberation -->ra "<<raout<<"  dec:"<<decout<<endl;
	haout = siderealhours-raout; // calculate hour angle.
	//cerr <<"siderealhours:"<<siderealhours<<"  ha:"<<haout<<endl;
	// size is the amount of correction.
	// flag of 1 is the direction of refraction correction.
	if (refractflag) {
		refract_corr (&haout,&decout,&size,MDMLATITUDE,MDMELEVATION,1);
		//cerr <<"refraction-->ha "<<haout<<"  dec:"<<decout<<endl;

	}
	ha = haout;
	dec = decout;
}
void telescope_coordinates_to_mean_place (
											double ha,double dec,
											double jd,double siderealhours,
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

void hourangle (double ra,double sr,double &ha)
{   //sr is sidereal decimal hours.
	ha = sr-ra; // calculate hour angle.
}
void apply_telescope_model (double &teleha,double &teledec,int direction)
{
	 direction = 0;
	 teleha+=0*direction;
	 teledec+=0*direction;

}
/*
		Convert from HA and DEC in units of hours and degrees to raw telescope
	coordinates of arcseconds with 0,0 at zenith
 */

void telescope_coordinates_to_raw_coordinates (double teleha,double teledec,
																	double &harawarcsec,double &decrawarcsec)
{
	//apply_telescope_model (teleha,teledec,1);
	double currentha = harawarcsec/HOURSTOARCSEC;
	double currentdec = decrawarcsec/HOURSTOARCSEC;

	harawarcsec = teleha*HOURSTOARCSEC;
	decrawarcsec = (teledec-MDMLATITUDE)*DEGREESTOARCSEC;

}
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
	// if (ratecorflag)
	//apply_telescope_model (teleha,teledec,-1);
}

// Encoder Positions for RA and DEC ---> "3\r"

// Coordinates, set the target location. --> "6\r"
void set_coordinates ()
{
	 mean_place_to_telescope_coordinates (rameantarget,decmeantarget,stdepoch,
								juliandate,siderealhours,hatarget,dectarget,1,0);
	 telescope_coordinates_to_raw_coordinates (hatarget,dectarget,
																						 harawtarget,decrawtarget);
}
