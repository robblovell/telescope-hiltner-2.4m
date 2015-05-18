#ifndef skycalchh
#define skycalchh
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

/* a couple of the system-dependent magic numbers are defined here */

#define SYS_CLOCK_OK 1    /* 1 means ANSI-standard time libraries do work,
	2 means they don't.  This is used by compiler switches in file 5 and
	the main program.  */

#define LOG_FILES_OK 1  /* 1 means that log files are enabled.
			Any other value means they're not.  */

#define MAX_OBJECTS 500
#define MINSHORT -32767   /* min, max short integers and double precision */
#define MAXSHORT 32767
#define MAXDOUBLE 1.0e38
#define MINDOUBLE -1.0e38
#define BUFSIZE 150 

#define XFORM_FROMSTD  1  /* defined quantities for apparent place transforms .. */
#define XFORM_TOSTDEP  -1
#define XFORM_JUSTPRE  1
#define XFORM_DOAPPAR  0

/* some (not all) physical, mathematical, and astronomical constants
   used are defined here. */

#define  PI                3.14159265358979
#define  ARCSEC_IN_RADIAN  206264.8062471
#define  DEG_IN_RADIAN     57.2957795130823
#define  HRS_IN_RADIAN     3.819718634205
#define  KMS_AUDAY         1731.45683633   /* km per sec in 1 AU/day */
#define  SPEED_OF_LIGHT    299792.458      /* in km per sec ... exact. */
#define  SS_MASS           1.00134198      /* solar system mass in solar units */
#define  J2000             2451545.        /* Julian date at standard epoch */
#define  SEC_IN_DAY        86400.
#define  FLATTEN           0.003352813   /* flattening of earth, 1/298.257 */
#define  EQUAT_RAD         6378137.    /* equatorial radius of earth, meters */
#define  ASTRO_UNIT        1.4959787066e11 /* 1 AU in meters */
#define  RSUN              6.96000e8  /* IAU 1976 recom. solar radius, meters */
#define  RMOON             1.738e6    /* IAU 1976 recom. lunar radius, meters */
#define  PLANET_TOL        3.          /* flag if nearer than 3 degrees 
						to a major planet ... */
#define  KZEN              0.172       /* zenith extinction, mag, for use
					  in lunar sky brightness calculations. */
#define FIRSTJD            2415387.  /* 1901 Jan 1 -- calendrical limit */
#define LASTJD             2488070.  /* 2099 Dec 31 */

/* MAGIC NUMBERS which might depend on how accurately double-
	precision floating point is handled on your machine ... */

#define  EARTH_DIFF        0.05            /* used in numerical
   differentiation to find earth velocity -- this value gives
   about 8 digits of numerical accuracy on the VAX, but is 
	about 3 orders of magnitude larger than the value where roundoff
   errors become apparent. */

#define  MIDN_TOL          0.00001         /* this is no longer
	used -- it was formerly
   how close (in days) a julian date has to be to midnight
   before a warning flag is printed for the reader.  VAX
   double precision renders a Julian date considerably
	more accurately than this.  The day and date are now based
   on the same rounding of the julian date, so they should
   always agree. */

/*  FUNCTION PROTOTYPES and type definitions ....
    These are used in breaking the code into function libraries.
    They work properly on a strictly ANSI compiler, so they
    apparently comply with the ANSI standard format.  */

struct coord
	{
	  short sign;  /* carry sign explicitly since -0 not neg. */
	  double hh;
	  double mm;
	  double ss;
	};

struct date_time
	{
	short y;
	short mo;
	short d;
	short h;
	short mn;
	float s;
	};

extern FILE *sclogfl;

extern double bab_to_dec(struct coord bab);
extern void round_coord(struct coord *tmpcoord, struct coord *outcoord,int prec);
extern void adj_time_ha (double &x);
extern void adj_time_24 (double &x);
//extern void correct_time (struct date_time *tm);
extern void correct_24_time (struct date_time *tm);

extern void correct_ha_time (struct date_time *tm);extern void correct_bab_hours (struct coord *bab);
extern void correct_bab_degrees (struct coord *bab);
extern void correct_24_hours (struct coord *bab);
extern void correct_24_degrees (struct coord *bab);

extern void dec_to_bab (double deci,struct coord *bab);extern double star_tzero, star_terr,
	star_period, star_perr;  /* for ephemeris calculations ... global */
extern void decimalhour_to_hms (double dechour,struct date_time &hms);
extern double date_to_jd(struct date_time date);
extern double lst(double jd,double longit);
extern double altit(double dec,double ha,double lat,double *az,double *parang);
extern void caldat(double jdin,
										struct date_time *date,
										short *dow);
extern void refract_corr(	double *ha,double *dec,double *size,
													double lat,double elev,
													int sense);
extern void cooxform(
	double rin, double din,  /* input ra and dec decimal hours, degrees */
	double std_epoch,   /* julian day -> - constant / 365 */
	double date_epoch,
	double *rout,double *dout,  /* output */
	int just_precess,  /* flag ... 1 does just precession, 0
			includes aberration and nutation. */
	int from_std);    /* flag ... 1 --> from std to date,
				    -1 --> from date to std. */ 


#endif
