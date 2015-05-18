#include "Skycalc.h"
#include "const.h"
double bab_to_dec(struct coord bab)
	 /* converts a "babylonian" (sexigesimal) structure into
      double-precision floating point ("decimal") number. */
	 {
	 double x;
	 if (bab.sign == 0) bab.sign = 1;
   x = bab.sign * (bab.hh + bab.mm / 60. + bab.ss / 3600.);
   return(x);
	 }
double adj_time(double x)
{
	/* adjusts a time (decimal hours) to be between -12 and 12, 
	   generally used for hour angles.  */

	if(fabs(x) < 100000.) {  /* too inefficient for this! */
		while(x > 12.) {
			x = x - 24.;
		}
		while(x < -12.) {
			x = x + 24.;
		}
	}

	return(x);
}
void adj_time_ha (double &x)
{
	/* adjusts a time (decimal hours) to be between -12 and 12, 
	   generally used for hour angles.  */

	if(fabs(x) < 100000.) {  /* too inefficient for this! */
		while(x > 12.) {
			x = x - 24.;
		}
		while(x < -12.) {
			x = x + 24.;
		}
	}

}
void adj_time_24 (double &x)
{
	/* adjusts a time (decimal hours) to be between -12 and 12, 
	   generally used for hour angles.  */

	if(fabs(x) < 100000.) {  /* too inefficient for this! */
		while(x >= 24.) {
			x = x - 24.;
		}
		while(x < 0.) {
			x = x + 24.;
		}
	}

}

void dec_to_bab (double deci,struct coord *bab)
	 // function for converting decimal to babylonian hh mm ss.ss
{
   int hr_int, min_int;

   if (deci >= 0.) bab->sign = 1; 
   else {
      bab->sign = -1;
      deci = -1. * deci;
   }
	 hr_int = (int)deci;   // use conversion conventions to truncate
	 bab->hh = (double) hr_int;
	 min_int = (int) (60. * (deci - bab->hh));
	 bab->mm = (double)min_int;
	 bab->ss = 3600. * (deci - bab->hh - bab->mm / 60.);

}

#define ROUNDERRORFIX
#ifdef ROUNDERRORFIX
double round(x,places)

double x;
int places;

/* rounds argument x to places places, e.g. 2.32839,1 -> 2.3. */

{
	double tmp, base = 1.;
	int i, ip;

	for(i = 1; i <= places; i++) {  /* bet this is faster than pow ... */
		base *= 10.;
	}
	tmp = x * base;
	if(tmp >= 0.) 
        	tmp += 0.5;
				else tmp -= 0.5;
	ip = (int) tmp;
	tmp = ((double) ip) / base;
	return(tmp);
}

void round_coord(tmpcoord, outcoord, prec)

struct coord *tmpcoord, *outcoord;
int prec;

/* Rounds the seconds of a struct coord to a specified precision;
	 if they turn out to be sixty, does the carry to the other fields.

	precision 0 -- whole minutes  (seconds set to zero)
						1 -- tenths of a minute (seconds set to zero)
			2 -- whole seconds
			3 -- tenths of a second
				4 -- hundredths ...
				etc.
*/

{
	struct coord *incoord;
	incoord = tmpcoord;
	outcoord->sign = incoord->sign;

/* initialize */
	
	outcoord->ss = incoord->ss,prec;
	outcoord->mm = incoord->mm;
	outcoord->hh = incoord->hh;

	if(prec <= 1) {
		outcoord->mm = round((outcoord->mm + outcoord->ss / 60.),prec);
		outcoord->ss = 0.;
		if(outcoord->mm >= 59.99) {  /* permissible because of limit
				on prec */
			outcoord->mm -= 60.;
			outcoord->hh += 1.;
		}
	}
	else {
		outcoord->ss = round(outcoord->ss,(prec-2));
		if( outcoord->ss >= 59.999999999) {  /* as many digits as
				one would ever want ... */
			outcoord->ss -= 60.;
			outcoord->mm += 1.;
			if(outcoord->mm >= 59.999999999) {
				outcoord->mm -= 60.;
				outcoord->hh += 1.;
			}
		}
	}
}
#endif
void correct_24_time (struct date_time *tm)
{
	 if (tm->s >= 59.5) {tm->mn++; tm->s =  0.0;}
	 else if (tm->s <=59.5 && tm->s >= 59.0) tm->s = 59.0;
	 if (tm->mn >= 59.5)  {tm->h++; tm->mn = 0; }
	 else if (tm->mn <=59.5 && tm->mn >=59.0) tm->mn = 59;
	 adj_time_24 (tm->h);
}
void correct_ha_time (struct date_time *tm)
{
	 if (tm->s >= 59.1) {tm->mn++; tm->s =  0.0;}
	 else if (tm->s <=59.1 && tm->s >= 59.0) tm->s = 59.0;
	 if (tm->mn >= 59.4)  {tm->h++; tm->mn = 0.0; }
	 else if (tm->mn <=59.5 && tm->mn >=59.0) tm->mn = 59.0;
	 adj_time_ha (tm->h);
}void correct_24_hours (struct coord *bab)
{
	 if (bab->ss >= 59.999) {bab->mm++; bab->ss =  0.0;}
	 if (bab->mm >= 59.999)  {bab->hh++; bab->mm = 0.0; }
	 adj_time_24 (bab->hh);
}

void correct_24_degrees (struct coord *bab)
{
	 if (bab->ss >= 59.99999) {bab->mm++; bab->ss =  0.0;}
	 if (bab->mm >= 59.99999)  {bab->hh++; bab->mm = 0.0; }

}void correct_bab_hours (struct coord *bab)
{
	 if (bab->ss >= 59.99999) {bab->mm++; bab->ss =  0.0;}
	 if (bab->mm >= 59.99999)  {bab->hh++; bab->mm = 0.0; }
	 adj_time (bab->hh);
}

void correct_bab_degrees (struct coord *bab)
{
	 if (bab->ss >= 59.99999) {bab->mm++; bab->ss =  0.0;}
	 if (bab->mm >= 59.99999)  {bab->hh++; bab->mm = 0.0; }

}

void load_site(longit,lat,stdz,use_dst,
	zone_name,zabr,elevsea,elev,horiz,site_name)
              
	double *longit,*lat;
   	double *stdz;
	short *use_dst;
   	char *zone_name, *zabr;
	double *elevsea;
		double *elev,*horiz;
	char *site_name;
{

		strcpy(site_name,"Kitt Peak [MDM Obs.]");
		strcpy(zone_name, "Mountain");
		*zabr = 'M';
		*use_dst = 0;
		*longit = MDMHOURLONGITUDE; //7.44111; /* decimal hours */
		*lat = MDMLATITUDE; //31.9533;    /* decimal degrees */
		*stdz = 7.;
		*elevsea = MDMELEVATION; //1925.;  /* for MDM observatory, strictly */
		*elev = 700.;  /* approximate -- to match KPNO tables */

}

void min_max_alt(lat,dec,min,max)

	double lat,dec,*min,*max;

{
	/* computes minimum and maximum altitude for a given dec and
	    latitude. */

	double x;
	lat = lat / DEG_IN_RADIAN; /* pass by value! */
	dec = dec / DEG_IN_RADIAN;
	x = cos(dec)*cos(lat) + sin(dec)*sin(lat);
	if(fabs(x) <= 1.) {
		*max = asin(x) * DEG_IN_RADIAN;
	}
	//else fprintf ("error in min_max_alt -- arcsin(>1)\n");
	x = sin(dec)*sin(lat) - cos(dec)*cos(lat);
	if(fabs(x) <= 1.) {
		*min = asin(x) * DEG_IN_RADIAN;
	}
	//else printf("Error in min_max_alt -- arcsin(>1)\n");
}
#define  TWOPI             6.28318530717959
#define  PI_OVER_2         1.57079632679490  /* From Abramowitz & Stegun */   
#define OLD
#ifdef NEW
double atan_circ(x,y)
                
	double x,y;
                
{
	/* returns radian angle 0 to 2pi for coords x, y --
	   get that quadrant right !! */

	double theta;

	if((x == 0.) && (y == 0.)) return(0.);  /* guard ... */

	theta = atan2(y,x);  /* turns out there is such a thing in math.h */
	while(theta < 0.) theta += TWOPI;
	return(theta);
}

void xyz_cel(x,y,z,ra,dec) 
	double x,y,z;  /* cartesian coordinate triplet */
	double *ra, *dec;  /* corresponding right ascension and declination,
                returned in decimal hours and decimal degrees. */

/* A much cleaner rewrite of the original skycalc code for this,
   which was transcribed from a PL/I routine .... */

{

   double mod;    /* modulus */
   double xy;     /* component in xy plane */

   /* normalize explicitly and check for bad input */

   mod = sqrt(x*x + y*y + z*z);
   if(mod > 0.) {
	   x = x / mod; y = y / mod; z = z / mod;  
   }
   else {   /* this has never happened ... */
	printf("Bad data in xyz_cel .... zero modulus position vector.\n");
	*ra = 0.; *dec = 0.;
	return;
   }
   
   xy = sqrt(x*x + y*y);

   if(xy < 1.0e-11) {   /* practically on a pole -- limit is arbitrary ...  */
      *ra = 0.;   /* degenerate anyway */
      *dec = PI_OVER_2; 
      if(z < 0.) *dec *= -1.;
   }
   else { /* in a normal part of the sky ... */
      *dec = asin(z);
      *ra = atan_circ(x,y);
   }

   *ra *= HRS_IN_RADIAN;
   *dec *= DEG_IN_RADIAN;
}

#endif
#ifdef OLD

double atan_circ(x,y)
                
	double x,y;

{
	/* returns radian angle 0 to 2pi for coords x, y --
		 get that quadrant right !! */

	double theta;

	if(x == 0.) {
		if(y > 0.) theta = PI / 2.;
		else if(y < 0.) theta = 3.* PI / 2.;
		else theta = 0.;   /* x and y zero */
	}
	else theta = atan(y/x);
	if(x < 0.) theta = theta + PI;
	if(theta < 0.) theta = theta + 2.* PI;
	return(theta);
}

void xyz_cel(x, y, z, r, d)

	double x, y, z, *r, *d;

		 /* Cartesian coordinate triplet */

{
	 /* converts a coordinate triplet back to a standard ra and dec */

	 double mod;    /* modulus */
	 double xy;     /* component in xy plane */
	 short sign;    /* for determining quadrant */
	 double radian_ra, radian_dec;

   /* this taken directly from pl1 routine - no acos or asin available there,
       as it is in c. Easier just to copy, though */

   mod = sqrt(x*x + y*y + z*z);
   x = x / mod;
   y = y / mod;
	 z = z / mod;   /* normalize 'em explicitly first. */

   xy = sqrt(x*x + y*y);

	 if(xy < 1.0e-10) {
      radian_ra = 0.;  /* too close to pole */
      radian_dec = PI / 2.;
      if(z < 0.) radian_dec = radian_dec * -1.;
	 }
   else {
      if(fabs(z/xy) < 3.) radian_dec = atan(z / xy);
	 else if (z >= 0.) radian_dec = PI / 2. - atan(xy / z);
	 else radian_dec = -1. * PI / 2. - atan(xy / z);
      if(fabs(x) > 1.0e-10) {
	 if(fabs(y / x) < 3.) radian_ra = atan(y / x);
	 else if ((x * y ) >= 0.) radian_ra = PI / 2. - atan(x/y);
	 else radian_ra = -1. *  PI / 2. - atan(x / y);
      }
      else {
	 radian_ra = PI / 2.;
	 if((x * y)<= 0.) radian_ra = radian_ra * -1.;
      }
      if(x <0.) radian_ra = radian_ra + PI ;
			if(radian_ra < 0.) radian_ra = radian_ra + 2. * PI ;
   }

	 *r = radian_ra * HRS_IN_RADIAN;
   *d = radian_dec * DEG_IN_RADIAN;

}
#endif
#define OLDA
#ifdef NEWA
double altit(double dec,double ha,double lat,double *az,double *parang)
/* returns altitude(degr) for dec, ha, lat (decimal degr, hr, degr);
    also computes and returns azimuth through pointer argument,
    and as an extra added bonus returns parallactic angle (decimal degr)
		through another pointer argument.
 */
{
	double x,y,z;
	double sinp, cosp;  /* sin and cos of parallactic angle */
	double cosdec, sindec, cosha, sinha, coslat, sinlat;
			/* time-savers ... */

	dec = dec / DEG_IN_RADIAN;
	ha = ha / HRS_IN_RADIAN;
	lat = lat / DEG_IN_RADIAN;  /* thank heavens for pass-by-value */
	cosdec = cos(dec); sindec = sin(dec);
	cosha = cos(ha); sinha = sin(ha);
	coslat = cos(lat); sinlat = sin(lat);
	x = DEG_IN_RADIAN * asin(cosdec*cosha*coslat + sindec*sinlat);
	y =  sindec*coslat - cosdec*cosha*sinlat; /* due N comp. */
	z =  -1. * cosdec*sinha; /* due east comp. */

 //	*az = atan2(z,y);

	*az = atan_circ(y,z);

	/* as it turns out, having knowledge of the altitude and
					 azimuth makes the spherical trig of the parallactic angle
					 less ambiguous ... so do it here!  Method uses the
		 "astronomical triangle" connecting celestial pole, object,
					 and zenith ... now know all the other sides and angles,
					 so we can crush it ... */

	if(cosdec != 0.) { /* protect divide by zero ... */ 
		 sinp = -1. * sin(*az) * coslat / cosdec;
		/* spherical law of sines .. note cosdec = sin of codec,
			coslat = sin of colat .... */
		 cosp = -1. * cos(*az) * cosha - sin(*az) * sinha * sinlat;
		/* spherical law of cosines ... also transformed to local
                      available variables. */
		 *parang = atan2(sinp,cosp) * DEG_IN_RADIAN;
		/* let the library function find the quadrant ... */
	}
	else { /* you're on the pole */
		 if(lat >= 0.) *parang = 180.;
		 else *parang = 0.;
	}

	*az *= DEG_IN_RADIAN;  /* done with taking trig functions of it ... */
//	while(*az < 0.) *az += 360.;  /* force 0 -> 360 */
//	while(*az >= 360.) *az -= 360.;
	
	return(x);
}
#endif
#ifdef OLDA
double altit(dec,ha,lat,az,par)

	double dec,ha,lat,*az,*par;

/* returns altitude(degr) for dec, ha, lat (decimal degr, hr, degr);
		also computes and returns azimuth through pointer argument. */
{
	double x,y,z;
	dec = dec / DEG_IN_RADIAN;
	ha = ha / HRS_IN_RADIAN;
	lat = lat / DEG_IN_RADIAN;  /* thank heavens for pass-by-value */
	double cosdec = cos(dec), sindec = sin(dec);
	double cosha = cos(ha), sinha = sin(ha);
	double coslat = cos(lat), sinlat = sin(lat);

	x = DEG_IN_RADIAN * asin(cos(dec)*cos(ha)*cos(lat) + sin(dec)*sin(lat));
	y =  sin(dec)*cos(lat) - cos(dec)*cos(ha)*sin(lat); /* due N comp. */
	z =  -1. * cos(dec)*sin(ha); /* due east comp. */
	double sinp,cosp;
	*az = atan_circ(y,z);
	if(cos(dec) != 0.) { /* protect divide by zero ... */
		 sinp = -1. * sin(*az) * coslat / cosdec;
		/* spherical law of sines .. note cosdec = sin of codec,
			coslat = sin of colat .... */
		 cosp = -1. * cos(*az) * cosha - sin(*az) * sinha * sinlat;
		/* spherical law of cosines ... also transformed to local
                      available variables. */
		 *par = atan2(sinp,cosp) * DEG_IN_RADIAN;
		/* let the library function find the quadrant ... */
	}
	else { /* you're on the pole */
		 if(lat >= 0.) *par = 180.;
		 else *par = 0.;
	}
	while (*par > 90.0) *par-=180.0;
	while (*par < -90.0) *par+=180.0;
	*az *= DEG_IN_RADIAN;

	return(x);
}

#endif


double secant_z(alt)   // returns the airmass, zenith angle, ie altitude.
	double alt;
{
	/* Computes the secant of z, assuming the object is not
					 too low to the horizon; returns 100. if the object is
           low but above the horizon, -100. if the object is just
           below the horizon. */

	double secz;
	if(alt != 0) secz = 1. / sin(alt / DEG_IN_RADIAN);
	else secz = 100.;
	if(secz > 100.) secz = 100.;
	if(secz < -100.) secz = -100.;
	return(secz);
} 

double ha_alt(dec,lat,alt)

	double dec,lat,alt;

{
	/* returns hour angle at which object at dec is at altitude alt.
	   If object is never at this altitude, signals with special 
	   return values 1000 (always higher) and -1000 (always lower). */
	
	double x,coalt,min,max;
	
	min_max_alt(lat,dec,&min,&max);
	if(alt < min) 
		return(1000.);  /* flag value - always higher than asked */
	if(alt > max)
		return(-1000.); /* flag for object always lower than asked */
	dec = (0.5*PI) - dec / DEG_IN_RADIAN;
	lat = (0.5*PI) - lat / DEG_IN_RADIAN;
	coalt = (0.5*PI) - alt / DEG_IN_RADIAN;
	x = (cos(coalt) - cos(dec)*cos(lat)) / (sin(dec)*sin(lat));
	if(fabs(x) <= 1.) return(acos(x) * HRS_IN_RADIAN);
	else {
		//oprntf("Error in ha_alt ... acos(>1).\n");
		return(1000.);
	}
}

double subtend(ra1,dec1,ra2,dec2)
              
	double ra1,dec1,ra2,dec2;

  /*args in dec hrs and dec degrees */

{
	/* angle subtended by two positions in the sky --
	   return value is in radians.  Hybrid algorithm works down
	   to zero separation except very near the poles. */

	double x1, y1, z1, x2, y2, z2;
	double theta;

	ra1 = ra1 / HRS_IN_RADIAN;
	dec1 = dec1 / DEG_IN_RADIAN;
	ra2 = ra2 / HRS_IN_RADIAN;
	dec2 = dec2 / DEG_IN_RADIAN;
	x1 = cos(ra1)*cos(dec1);
	y1 = sin(ra1)*cos(dec1);
	z1 = sin(dec1);
	x2 = cos(ra2)*cos(dec2);
	y2 = sin(ra2)*cos(dec2);
	z2 = sin(dec2);
	theta = acos(x1*x2+y1*y2+z1*z2);
     /* use flat Pythagorean approximation if the angle is very small
	*and* you're not close to the pole; avoids roundoff in arccos. */
	if(theta < 1.0e-5) {  /* seldom the case, so don't combine test */
		if(fabs(dec1) < (PI/2. - 0.001) && 
		    fabs(dec2) < (PI/2. - 0.001))    {
			/* recycled variables here... */
			x1 = (ra2 - ra1) * cos((dec1+dec2)/2.);
			x2 = dec2 - dec1;
			theta = sqrt(x1*x1 + x2*x2);
		}
	}
	return(theta);
}
double date_to_jd(date)

	struct date_time date;

/* Converts a date (structure) into a julian date.
   Only good for 1900 -- 2100. */

{
	short yr1=0, mo1=1;     
	long jdzpt = 1720982, jdint, inter;
	double jd,jdfrac;
	if((date.y <= 1900) | (date.y >= 2100)) {
		printf("Date out of range.  1900 - 2100 only.\n");
		return(0.);
	}
	if(date.mo <= 2) {
		yr1 = -1;
		mo1 = 13;
	}
	jdint = 365.25*(date.y+yr1);  /* truncates */
	inter = 30.6001*(date.mo+mo1);
	jdint = jdint+inter+date.d+jdzpt;
	jd = jdint;
	jdfrac=date.h/24.+date.mn/1440.+date.s/SEC_IN_DAY;
	if(jdfrac < 0.5) {
		jdint--;
		jdfrac=jdfrac+0.5;
	}
	else jdfrac=jdfrac-0.5;                 
	jd=jdint+jdfrac;
	return(jd);
}

short day_of_week(jd)
	double jd;
{ 
	/* returns day of week for a jd, 0 = Mon, 6 = Sun. */

	double x;//,y;
	long i;
	short d;
	
	jd = jd+0.5;
	i = jd; /* truncate */
	x = i/7.+0.01; 
	d = 7.*(x - (long) x);   /* truncate */
	return(d);
}


void caldat(jdin,date,dow)
           
	double jdin;
	struct date_time *date;
	short *dow;
           
#define IGREG 2299161

{ 
	/* Returns date and time for a given julian date;
	   also returns day-of-week coded 0 (Mon) through 6 (Sun).
	   Adapted from Press, Flannery, Teukolsky, & 
	   Vetterling, Numerical Recipes in C, (Cambridge
	   University Press), 1st edn, p. 12. */

	int mm, id, iyyy;  /* their notation */
	long ja, jdint, jalpha, jb, jc, jd, je;
	float jdfrac;
	double x;

	jdin = jdin + 0.5;  /* adjust for 1/2 day */
	jdint = jdin;
	x = jdint/7.+0.01; 
	*dow = 7.*(x - (long) x);   /* truncate for day of week */
	jdfrac = jdin - jdint;
	date->h = jdfrac * 24; /* truncate */
	date->mn = (jdfrac - ((float) date->h)/24.) * 1440.;
	date->s = (jdfrac - ((float) date->h)/24. - 
			((float) date->mn)/1440.) * SEC_IN_DAY;
	
	if(jdint > IGREG) {
		jalpha=((float) (jdint-1867216)-0.25)/36524.25;
		ja=jdint+1+jalpha-(long)(0.25*jalpha);
	}
	else
		ja=jdint;
	jb=ja+1524;
	jc=6680.0+((float) (jb-2439870)-122.1)/365.25;
	jd=365*jc+(0.25*jc);
	je=(jb-jd)/30.6001;
	id=jb-jd-(int) (30.6001*je);
	mm=je-1;
	if(mm > 12) mm -= 12;
	iyyy=jc-4715;
	if(mm > 2) --iyyy;
	if (iyyy <= 0) --iyyy;
	date->y = iyyy;
	date->mo = mm;
	date->d = id;
}

double frac_part(x)

	double x;
{
	long i;
	i = x;
	x = x - i;
	return(x);
}
/*
  given a decimal hour, spit out the hour minute and second format.
*/
void decimalhour_to_hms (double dechour,struct date_time &hms) //int &hour,int &min,double &sec)
{

	hms.h = (int)dechour;
	dechour -= (double) hms.h;
	dechour = (double) (dechour*60.0);
	hms.mn = (int)dechour;

	dechour -= (double) hms.mn;

	hms.s = (int) (dechour*60.0);
	if (hms.s == 60) {
		hms.s = 0;
		hms.mn++;
	}
	if (hms.mn == 60) {
		hms.mn = 0;
		hms.h++;
	}
	if (hms.h == 24) {
		hms.h = 0;
	}
}


double lst(jd,longit)

	double jd,longit; 

{
	/* returns the local MEAN sidereal time (dec hrs) at julian date jd
		at west longitude long (decimal hours)(longit).  Follows
		definitions in 1992 Astronomical Almanac, pp. B7 and L2.
		Expression for GMST at 0h ut referenced to Aoki et al, A&A 105,
		p.359, 1982.  On workstations, accuracy (numerical only!)
		is about a millisecond in the 1990s. */

	double t, ut, jdmid, jdint, jdfrac, sid_g, sid;
	long jdin, sid_int;

	jdin = jd;         /* fossil code from earlier package which
			split jd into integer and fractional parts ... */
	jdint = jdin;
	jdfrac = jd - jdint;
	if(jdfrac < 0.5) {
		jdmid = jdint - 0.5;
		ut = jdfrac + 0.5;
	}
	else {
		jdmid = jdint + 0.5;
		ut = jdfrac - 0.5;
	}
	t = (jdmid - J2000)/36525;
	sid_g = (24110.54841+8640184.812866*t+0.093104*t*t-6.2e-6*t*t*t)/SEC_IN_DAY;
	sid_int = sid_g;
	sid_g = sid_g - (double) sid_int;
	sid_g = sid_g + 1.0027379093 * ut - longit/24.;
	sid_int = sid_g;
	sid_g = (sid_g - (double) sid_int) * 24.;
	if(sid_g < 0.) sid_g = sid_g + 24.;
	return(sid_g);
}


 void eclrot(jd, x, y, z)

	double jd, *x, *y, *z;
           
/* rotates ecliptic rectangular coords x, y, z to
   equatorial (all assumed of date.) */

{
	double incl;
	double xpr,ypr,zpr;
	double T;

	T = (jd - J2000) / 36525;  /* centuries since J2000 */
	
	incl = (23.439291 + T * (-0.0130042 - 0.00000016 * T))/DEG_IN_RADIAN; 
		/* 1992 Astron Almanac, p. B18, dropping the 
		   cubic term, which is 2 milli-arcsec! */
	ypr = cos(incl) * *y - sin(incl) * *z;
	zpr = sin(incl) * *y + cos(incl) * *z;
	*y = ypr;
	*z = zpr;
	/* x remains the same. */       
}

double circulo(x)
	double x;
{
	/* assuming x is an angle in degrees, returns 
	   modulo 360 degrees. */

	int n;

	n = (int)(x / 360.);
	return(x - 360. * n);
}       


void geocent(geolong,geolat,height,x_geo,y_geo,z_geo)
            
	double geolong, geolat, height, *x_geo, *y_geo, *z_geo;
            
/* computes the geocentric coordinates from the geodetic 
(standard map-type) longitude, latitude, and height. 
These are assumed to be in decimal hours, decimal degrees, and
meters respectively.  Notation generally follows 1992 Astr Almanac, 
p. K11 */


{
	
	double denom, C_geo, S_geo;

	geolat = geolat / DEG_IN_RADIAN;
	geolong = geolong / HRS_IN_RADIAN;      
	denom = (1. - FLATTEN) * sin(geolat);
	denom = cos(geolat) * cos(geolat) + denom*denom;
	C_geo = 1. / sqrt(denom);
	S_geo = (1. - FLATTEN) * (1. - FLATTEN) * C_geo;
	C_geo = C_geo + height / EQUAT_RAD;  /* deviation from almanac
		       notation -- include height here. */
	S_geo = S_geo + height / EQUAT_RAD;
	*x_geo = C_geo * cos(geolat) * cos(geolong);
	*y_geo = C_geo * cos(geolat) * sin(geolong);
	*z_geo = S_geo * sin(geolat);
}
 

double etcorr(jd)

double jd;

{

	/* Given a julian date in 1900-2100, returns the correction
           delta t which is:
		TDT - UT (after 1983 and before 1993)
		ET - UT (before 1983)
		an extrapolated guess  (after 1993). 

	For dates in the past (<= 1993) the value is linearly
        interpolated on 5-year intervals; for dates after the present,
        an extrapolation is used, because the true value of delta t
	cannot be predicted precisely.  Note that TDT is essentially the
	modern version of ephemeris time with a slightly cleaner 
	definition.  

	Where the algorithm shifts there is an approximately 0.1 second
        discontinuity.  Also, the 5-year linear interpolation scheme can 
        lead to errors as large as 0.5 seconds in some cases, though
 	usually rather smaller. */

	double jd1900 = 2415019.5;
	double dates[20] = {1900,1905,1910,1915,1920,1925,1930,1935,1940,1945,
		    1950,1955,1960,1965,1970,1975,1980,1985,1990,1993};
	double delts[20]={-2.72,3.86,10.46,17.20,21.16,23.62,24.02,23.93,24.33,26.77,
		  29.15,31.07,33.15,35.73,40.18,45.48,50.54,54.34,56.86,59.12};
	double year, delt;
	short i;

	year = 1900. + (jd - 2415019.5) / 365.25;

	if(year < 1993.0 && year >= 1900.) {
		i = (year - 1900) / 5;
		delt = delts[i] + 
		 ((delts[i+1] - delts[i])/(dates[i+1] - dates[i])) * (year - dates[i]);
	}

	else if (year > 1993. && year < 2100.)
		delt = 33.15 + (2.164e-3) * (jd - 2436935.4);  /* rough extrapolation */

	else if (year < 1900) {
	 //	oprntf("etcorr ... no ephemeris time data for < 1900.\n");
       		delt = 0.;
	}

	else if (year >= 2100.) {
	 //	oprntf("etcorr .. very long extrapolation in delta T - inaccurate.\n");
		delt = 180.; /* who knows? */
	} 

	return(delt);
}

double zone(use_dst,stdz,jd,jdb,jde) 
           
	short use_dst;
	double stdz,jd,jdb,jde; 
           
{
	/* Returns zone time offset when standard time zone is stdz,
	   when daylight time begins (for the year) on jdb, and ends
	   (for the year) on jde.  This is parochial to the northern
	   hemisphere.  */
	/* Extension -- specifying a negative value of use_dst reverses
	   the logic for the Southern hemisphere; then DST is assumed for
	   the Southern hemisphere summer (which is the end and beginning
	   of the year. */

	if(use_dst == 0) return(stdz);
	else if((jd > jdb) && (jd < jde) && (use_dst > 0)) return(stdz-1.);
	   /* next line .. use_dst < 0 .. for Southern Hemisphere sites. */
	else if(((jd < jdb) || (jd > jde)) && (use_dst < 0)) return(stdz-1.);
	else return(stdz);
}
void find_dst_bounds(yr,stdz,use_dst,jdb,jde) 

	short yr;
	double stdz;
	short use_dst;
  	double *jdb,*jde; 

{
	/* finds jd's at which daylight savings time begins 
	    and ends.  The parameter use_dst allows for a number
	    of conventions, namely:
		0 = don't use it at all (standard time all the time)
		1 = use USA convention (1st Sun in April to
		     last Sun in Oct after 1986; last Sun in April before)
		2 = use Spanish convention (for Canary Islands)
		-1 = use Chilean convention (CTIO).
		-2 = Australian convention (for AAT).
	    Negative numbers denote sites in the southern hemisphere,
	    where jdb and jde are beginning and end of STANDARD time for
	    the year. 
	    It's assumed that the time changes at 2AM local time; so
	    when clock is set ahead, time jumps suddenly from 2 to 3,
	    and when time is set back, the hour from 1 to 2 AM local 
	    time is repeated.  This could be changed in code if need be. */

	struct date_time trial;

	if((use_dst == 1) || (use_dst == 0)) { 
	    /* USA Convention, and including no DST to be defensive */
	    /* Note that this ignores various wrinkles such as the
		brief Nixon administration flirtation with year-round DST,
		the extended DST of WW II, and so on. */
		trial.y = yr;
		trial.mo = 4;
		if(yr >= 1986) trial.d = 1;
		else trial.d = 30; 
		trial.h = 2;
		trial.mn = 0;
		trial.s = 0;

		/* Find first Sunday in April for 1986 on ... */
		if(yr >= 1986) 
			while(day_of_week(date_to_jd(trial)) != 6) 
				trial.d++;
			
		/* Find last Sunday in April for pre-1986 .... */
		else while(day_of_week(date_to_jd(trial)) != 6) 
				trial.d--;

		*jdb = date_to_jd(trial) + stdz/24.;    

		/* Find last Sunday in October ... */
		trial.mo = 10;
		trial.d = 31;
		while(day_of_week(date_to_jd(trial)) != 6) {
			trial.d--;
		}
		*jde = date_to_jd(trial) + (stdz - 1.)/24.;             
	}
	else if (use_dst == 2) {  /* Spanish, for Canaries */
		trial.y = yr;
		trial.mo = 3;
		trial.d = 31; 
		trial.h = 2;
		trial.mn = 0;
		trial.s = 0;

		while(day_of_week(date_to_jd(trial)) != 6) {
			trial.d--;
		}
		*jdb = date_to_jd(trial) + stdz/24.;    
		trial.mo = 9;
		trial.d = 30;
		while(day_of_week(date_to_jd(trial)) != 6) {
			trial.d--;
		}
		*jde = date_to_jd(trial) + (stdz - 1.)/24.;             
	}               
	else if (use_dst == -1) {  /* Chilean, for CTIO, etc.  */
	   /* off daylight 2nd Sun in March, onto daylight 2nd Sun in October */
		trial.y = yr;
		trial.mo = 3;
		trial.d = 8;  /* earliest possible 2nd Sunday */
		trial.h = 2;
		trial.mn = 0;
		trial.s = 0;

		while(day_of_week(date_to_jd(trial)) != 6) {
			trial.d++;
		}
		*jdb = date_to_jd(trial) + (stdz - 1.)/24.;
			/* note jdb is beginning of STANDARD time in south,
				hence use stdz - 1. */  
		trial.mo = 10;
		trial.d = 8;
		while(day_of_week(date_to_jd(trial)) != 6) {
			trial.d++;
		}
		*jde = date_to_jd(trial) + stdz /24.;           
	}                       
	else if (use_dst == -2) {  /* For Anglo-Australian Telescope  */
	   /* off daylight 1st Sun in March, onto daylight last Sun in October */
		trial.y = yr;
		trial.mo = 3;
		trial.d = 1;  /* earliest possible 1st Sunday */
		trial.h = 2;
		trial.mn = 0;
		trial.s = 0;

		while(day_of_week(date_to_jd(trial)) != 6) {
			trial.d++;
		}
		*jdb = date_to_jd(trial) + (stdz - 1.)/24.;
			/* note jdb is beginning of STANDARD time in south,
				hence use stdz - 1. */  
		trial.mo = 10;
		trial.d = 31;
		while(day_of_week(date_to_jd(trial)) != 6) {
			trial.d--;
		}
		*jde = date_to_jd(trial) + stdz /24.;           
	}               
}


double true_jd(date, use_dst, enter_ut, night_date, stdz)
       
/* takes the values in the date-time structure, the standard time
   zone (in hours west), the prevailing conventions for date and
   time entry, and returns the value of the true julian date. */

	struct date_time date;
	short use_dst, enter_ut, night_date;
	double stdz;
{
	double jd, jdb, jde, test;

	if(enter_ut == 0) {
					 find_dst_bounds(date.y,stdz,use_dst,&jdb,&jde);
	   jd = date_to_jd(date);
	   if((night_date == 1)  && (date.h < 12)) jd = jd + 1.;
	   if(use_dst != 0)  {  /* check at time changes */
		test = jd + stdz/24. - jdb;
		if((test > 0.) && (test < 0.041666666))   {
			/* 0.0416 = 1 hr; nonexistent time */
		 //	oprntf("Error in true_jd -- nonexistent input time during std->dst change.\n");
			//oprntf("Specify as 1 hour later!\n");
			return(-1.); /* signal of nonexistent time */
		}
		test = jd + stdz/24. - jde;
		if((test > 0.) && (test < 0.041666666))   {
			//oprntf("WARNING ... ambiguous input time during dst->std change!\n");
		}
	   }
		 jd = jd + zone(use_dst,stdz,(jd+stdz/24.),jdb,jde)/24.;
			/* effect should be to default to standard time. */
        }
	else jd = date_to_jd(date);
	
	return(jd);
}
   


void accusun(jd,lst,geolat,ra,dec,dist,topora,topodec,x,y,z)

	double jd,lst,geolat,*ra,*dec,*dist,*topora,*topodec;
	double *x, *y, *z;
{
			/*  implemenataion of Jean Meeus' more accurate solar
		ephemeris.  For ultimate use in helio correction! From
		Astronomical Formulae for Calculators, pp. 79 ff.  This
		gives sun's position wrt *mean* equinox of date, not
		*apparent*.  Accuracy is << 1 arcmin.  Positions given are
		geocentric ... parallax due to observer's position on earth is
		ignored. This is up to 8 arcsec; routine is usually a little
		better than that.
					// -- topocentric correction *is* included now. -- //
		Light travel time is apparently taken into
	  account for the ra and dec, but I don't know if aberration is
		and I don't know if distance is simlarly antedated.

	  x, y, and z are heliocentric equatorial coordinates of the
	  EARTH, referred to mean equator and equinox of date. */

	double L, T, Tsq, Tcb;
	double M, e, Cent, nu, sunlong;
	double Lrad, Mrad, nurad, R;
	double A, B, C, D, E, H;
	double xtop, ytop, ztop, topodist, l, m, n, xgeo, ygeo, zgeo;

	jd = jd + etcorr(jd)/SEC_IN_DAY;  /* might as well do it right .... */
	T = (jd - 2415020.) / 36525.;  /* 1900 --- this is an oldish theory*/
	Tsq = T*T;
	Tcb = T*Tsq;
	L = 279.69668 + 36000.76892*T + 0.0003025*Tsq;
	M = 358.47583 + 35999.04975*T - 0.000150*Tsq - 0.0000033*Tcb;
	e = 0.01675104 - 0.0000418*T - 0.000000126*Tsq;

	L = circulo(L);
	M = circulo(M);
/*      printf("raw L, M: %15.8f, %15.8f\n",L,M); */

	A = 153.23 + 22518.7541 * T;  /* A, B due to Venus */
	B = 216.57 + 45037.5082 * T;
	C = 312.69 + 32964.3577 * T;  /* C due to Jupiter */
		/* D -- rough correction from earth-moon 
			barycenter to center of earth. */
	D = 350.74 + 445267.1142*T - 0.00144*Tsq;  
	E = 231.19 + 20.20*T;    /* "inequality of long period .. */
	H = 353.40 + 65928.7155*T;  /* Jupiter. */
	
	A = circulo(A) / DEG_IN_RADIAN;
	B = circulo(B) / DEG_IN_RADIAN;
	C = circulo(C) / DEG_IN_RADIAN;
	D = circulo(D) / DEG_IN_RADIAN;
	E = circulo(E) / DEG_IN_RADIAN;
	H = circulo(H) / DEG_IN_RADIAN;

	L = L + 0.00134 * cos(A) 
	      + 0.00154 * cos(B)
	      + 0.00200 * cos(C)
	      + 0.00179 * sin(D)
	      + 0.00178 * sin(E);

	Lrad = L/DEG_IN_RADIAN;
	Mrad = M/DEG_IN_RADIAN;
	
	Cent = (1.919460 - 0.004789*T -0.000014*Tsq)*sin(Mrad)
	     + (0.020094 - 0.000100*T) * sin(2.0*Mrad)
	     + 0.000293 * sin(3.0*Mrad);
	sunlong = L + Cent;


	nu = M + Cent;
	nurad = nu / DEG_IN_RADIAN;
	
	R = (1.0000002 * (1 - e*e)) / (1. + e * cos(nurad));
	R = R + 0.00000543 * sin(A)
	      + 0.00001575 * sin(B)
	      + 0.00001627 * sin(C)
	      + 0.00003076 * cos(D)
	      + 0.00000927 * sin(H);
/*      printf("solar longitude: %10.5f  Radius vector %10.7f\n",sunlong,R);
	printf("eccentricity %10.7f  eqn of center %10.5f\n",e,Cent);   */
	
	sunlong = sunlong/DEG_IN_RADIAN;

	*dist = R;
	*x = cos(sunlong);  /* geocentric */
	*y = sin(sunlong);
	*z = 0.;
	eclrot(jd, x, y, z);
	
/*      --- code to include topocentric correction for sun .... */

	geocent(lst,geolat,0.,&xgeo,&ygeo,&zgeo);
	
	xtop = *x - xgeo*EQUAT_RAD/ASTRO_UNIT;
	ytop = *y - ygeo*EQUAT_RAD/ASTRO_UNIT;
	ztop = *z - zgeo*EQUAT_RAD/ASTRO_UNIT;

	topodist = sqrt(xtop*xtop + ytop*ytop + ztop*ztop);
	
	l = xtop / (topodist);
	m = ytop / (topodist);
	n = ztop / (topodist);

	*topora = atan_circ(l,m) * HRS_IN_RADIAN;
	*topodec = asin(n) * DEG_IN_RADIAN; 

	*ra = atan_circ(*x,*y) * HRS_IN_RADIAN;
	*dec = asin(*z) * DEG_IN_RADIAN; 
	
	*x = *x * R * -1;  /* heliocentric */
	*y = *y * R * -1;
	*z = *z * R * -1;

}aberrate(epoch, vec, from_std) 
	double epoch,  /* decimal year ...  */
	vec[];  /* celestial unit vector ...  */
        int from_std;  /* 1 = apply aberration, -1 = take aberration out. */

/* corrects celestial unit vector for aberration due to earth's motion. 
   Uses accurate sun position ... replace with crude one for more speed if
   needed. */

{
	double jd, jd1, jd2, g, lambda, L, Xdot, Ydot, Zdot;   /* page C24 */
	double ras, decs, dists, topora, topodec; /* throwaways */
	double x, y, z, x1, y1, z1, x2, y2, z2;
	double norm;
	double rarad, decrad;
	
	/* find heliocentric velocity of earth as a fraction of the speed of light ... */

	jd = J2000 + (epoch - 2000.) * 365.25;  
        jd1 = jd - EARTH_DIFF;
	jd2 = jd + EARTH_DIFF;
	
	accusun(jd1,0.,0.,&ras,&decs,&dists,&topora,&topodec,&x1,&y1,&z1);
	accusun(jd2,0.,0.,&ras,&decs,&dists,&topora,&topodec,&x2,&y2,&z2);
	accusun(jd,0.,0.,&ras,&decs,&dists,&topora,&topodec,&x,&y,&z);

	Xdot = KMS_AUDAY*(x2 - x1)/(2.*EARTH_DIFF * SPEED_OF_LIGHT);  /* numerical differentiation */
	Ydot = KMS_AUDAY*(y2 - y1)/(2.*EARTH_DIFF * SPEED_OF_LIGHT);  /* crude but accurate */
	Zdot = KMS_AUDAY*(z2 - z1)/(2.*EARTH_DIFF * SPEED_OF_LIGHT);

	/* approximate correction ... non-relativistic but very close.  */

	vec[1] += from_std * Xdot;
        vec[2] += from_std * Ydot;
	vec[3] += from_std * Zdot;

	norm = pow((vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]), 0.5);

	vec[1] = vec[1] / norm; vec[2] = vec[2] / norm; vec[3] = vec[3] / norm;

}

void nutation_params(date_epoch, del_psi, del_ep) 
	double date_epoch, *del_psi, *del_ep;

/* computes the nutation parameters delta psi and
   delta epsilon at julian epoch (in years) using approximate
   formulae given by Jean Meeus, Astronomical Formulae for
   Calculators, Willman-Bell, 1985, pp. 69-70. Accuracy
   appears to be a few hundredths of an arcsec or better
   and numerics have been checked against his example. 
   Nutation parameters are returned in radians. */

{

	double T, jd, L, Lprime, M, Mprime, Omega;

	jd = (date_epoch - 2000.) * 365.25 + J2000;
	T = (jd - 2415020.0) / 36525.;
	
	L = 279.6967 + (36000.7689  + 0.000303 * T) * T;
	Lprime = 270.4342 + (481267.8831 - 0.001133 * T ) * T;
	M = 358.4758 + (35999.0498 - 0.000150 * T) * T;
	Mprime = 296.1046 + (477198.8491 + 0.009192 * T ) * T;
	Omega = 259.1833 - (1934.1420 - 0.002078 * T) * T;

	L = L / DEG_IN_RADIAN;
	Lprime = Lprime / DEG_IN_RADIAN;
	M = M / DEG_IN_RADIAN;
	Mprime = Mprime / DEG_IN_RADIAN;
	Omega = Omega / DEG_IN_RADIAN;
        	
	
	*del_psi = -1. * (17.2327 + 0.01737 * T) * sin(Omega) 
	   - (1.2729 + 0.00013 * T) * sin(2. * L)
	   + 0.2088 * sin(2 * Omega)
	   - 0.2037 * sin(2 * Lprime)
	   + (0.1261 - 0.00031 * T) * sin(M)
	   + 0.0675 * sin(Mprime)
	   - (0.0497 - 0.00012 * T) * sin(2 * L + M)
	   - 0.0342 * sin(2 * Lprime - Omega)
	   - 0.0261 * sin(2 * Lprime + Mprime)
	   + 0.0214 * sin(2 * L - M)
	   - 0.0149 * sin(2 * L - 2 * Lprime + Mprime)
	   + 0.0124 * sin(2 * L - Omega) 
	   + 0.0114 * sin(2 * Lprime - Mprime);

	*del_ep = (9.2100 + 0.00091 * T) * cos(Omega)
	   + (0.5522 - 0.00029 * T) * cos(2 * L)
	   - 0.0904 * cos(2 * Omega)
	   + 0.0884 * cos(2. * Lprime)
	   + 0.0216 * cos(2 * L + M)
	   + 0.0183 * cos(2 * Lprime - Omega)
	   + 0.0113 * cos(2 * Lprime + Mprime)
	   - 0.0093 * cos(2 * L - M)
	   - 0.0066 * cos(2 * L - Omega);
	
/*	printf("del_psi = %f, del_ep = %f\n",*del_psi,*del_ep);
*/
	*del_psi = *del_psi / ARCSEC_IN_RADIAN;
	*del_ep  = *del_ep  / ARCSEC_IN_RADIAN;

/*	printf("del_psi = %f, del_ep = %f\n",*del_psi,*del_ep);
*/
}


void cooxform(rin, din, std_epoch, 
  date_epoch, rout, dout, just_precess, from_std)
	double rin, din;  /* input ra and dec decimal hours, degrees */
	double std_epoch;   /* julian day -> - constant / 365 */
	double date_epoch;        
	double *rout, *dout;  /* output */
	int just_precess;  /* flag ... 1 does just precession, 0 
			includes aberration and nutation. */
	int from_std;    /* flag ... 1 --> from std to date,
				    -1 --> from date to std. */ 

   /* General routine for precession and apparent place. Either
      transforms from current epoch (given by jd) to a standard
      epoch or back again, depending on value of the switch 
      "from_std"; 1 transforms from standard to current, -1 goes
      the other way.  Optionally does apparent place including
      nutation and annual aberration
      (but neglecting diurnal aberration, parallax, proper motion,
      and GR deflection of light); switch for this is "just_precess",
      1 does only precession, 0 includes other aberration & nutation. */

   /* Precession uses a matrix procedures
      as outlined in Taff's Computational Spherical Astronomy book.
      This is the so-called 'rigorous' method which should give very
      accurate answers all over the sky over an interval of several
      centuries.  Naked eye accuracy holds to ancient times, too. 
      Precession constants used are the new IAU1976 -- the 'J2000'
      system. 
 
      Nutation is incorporated into matrix formalism by constructing an 
      approximate nutation matrix and taking a matrix product with 
      precession matrix.  

      Aberration is done by adding the vector velocity of the earth to 
      the velocity of the light ray .... not kosher relativistically,
      but empirically correct to a high order for the angle.  */

{

   /* all the 3-d stuff is declared as [4] 'cause I'm not using the
	 zeroth element. */
	
   double ti, tf, zeta, z, theta;  /* all as per  Taff */
   double cosz, coszeta, costheta, sinz, sinzeta, sintheta;  /* ftns */
   double p[4][4];
      /* elements of the rotation matrix */
   double n[4][4]; 
      /* elements of the nutation matrix */
   double r[4][4];
      /* their product */
   double t[4][4];  /* temporary matrix for inversion .... */
   double radian_ra, radian_dec;
   double del_psi, del_eps, eps;  /* nutation angles in radians */
   double orig[4];   /* original unit vector */
   double fin[4];   /* final unit vector */
   int i, j, k;
   double out;    /* for diagnostics */
 
 
   ti = (std_epoch - 2000.) / 100.;
   tf = (date_epoch  - 2000. - 100. * ti) / 100.;

   zeta = (2306.2181 + 1.39656 * ti + 0.000139 * ti * ti) * tf +
    (0.30188 - 0.000344 * ti) * tf * tf + 0.017998 * tf * tf * tf;
   z = zeta + (0.79280 + 0.000410 * ti) * tf * tf + 0.000205 * tf * tf * tf;
   theta = (2004.3109 - 0.8533 * ti - 0.000217 * ti * ti) * tf
     - (0.42665 + 0.000217 * ti) * tf * tf - 0.041833 * tf * tf * tf;
 
   /* convert to radians */

   zeta = zeta / ARCSEC_IN_RADIAN;
   z = z / ARCSEC_IN_RADIAN;
   theta = theta / ARCSEC_IN_RADIAN;
  
   /* compute the necessary trig functions for speed and simplicity */
 
   cosz = cos(z);
   coszeta = cos(zeta);
   costheta = cos(theta);
   sinz = sin(z);
   sinzeta = sin(zeta);
   sintheta = sin(theta);

   /* compute the elements of the precession matrix -- set up
      here as *from* standard epoch *to* input jd. */

   p[1][1] = coszeta * cosz * costheta - sinzeta * sinz;
   p[1][2] = -1. * sinzeta * cosz * costheta - coszeta * sinz;
   p[1][3] = -1. * cosz * sintheta;

   p[2][1] = coszeta * sinz * costheta + sinzeta * cosz;
   p[2][2] = -1. * sinzeta * sinz * costheta + coszeta * cosz;
   p[2][3] = -1. * sinz * sintheta;
 
   p[3][1] = coszeta * sintheta;
   p[3][2] = -1. * sinzeta * sintheta;
   p[3][3] = costheta;

   if(just_precess == XFORM_DOAPPAR) {  /* if apparent place called for */
	
   	/* do the same for the nutation matrix. */

   	nutation_params(date_epoch, &del_psi, &del_eps); 
   	eps = 0.409105;  /* rough obliquity of ecliptic in radians */	

   	n[1][1] = 1.; n[2][2] = 1.; n[3][3] = 1.;
  	n[1][2] = -1. * del_psi * cos(eps);
   	n[1][3] = -1. * del_psi * sin(eps);
   	n[2][1] = -1. * n[1][2];
   	n[2][3] = -1. * del_eps;
   	n[3][1] = -1. * n[1][3];
   	n[3][2] = -1. * n[2][3];
 
	/* form product of precession and nutation matrices ... */
   	for(i = 1; i <= 3; i++) {
		for(j = 1; j <= 3; j++) {
			r[i][j] = 0.;
			for(k = 1; k <= 3; k++) 
				r[i][j] += p[i][k] * n[k][j];
        	}
   	}
   }
   else {  /* if you're just precessing .... */
	for(i = 1; i <= 3; i++) {
		for(j = 1; j <=3; j++) 
			r[i][j] = p[i][j];  /* simply copy precession matrix */
	}
   }

   /*   Commented out code ...
	for checking against Almanac tables ... turns out we're
   	ok to a few parts in 10^7. 

   for(i = 1; i <= 3; i++) {
	for(j = 1; j<= 3; j++) {
		printf("%d %d ... ",i,j);
		if(i == j) out = (r[i][j] - 1.) * 1.0e8;
		else out = r[i][j] * 1.0e8;
		printf("%7.0f\n",out);
	}
   }
    ........ end of commented out code. */

   /* The inverse of a rotation matrix is its transpose ... */

   if(from_std == XFORM_TOSTDEP) {    /* if you're transforming back to std
					epoch, rather than forward from std */
	for(i = 1; i <= 3; i++) {
		for(j = 1; j <= 3; j++)
			t[i][j] = r[j][i];  /* store transpose ... */		                               
	}
	for(i = 1; i <= 3; i++) {
		for(j = 1; j <= 3; j++)
			r[i][j] = t[i][j];  /* replace original w/ transpose.*/
	}
   }

   /* finally, transform original coordinates */
 
   radian_ra = rin / HRS_IN_RADIAN;
   radian_dec = din / DEG_IN_RADIAN;

   orig[1] = cos(radian_dec) * cos(radian_ra);
   orig[2] = cos(radian_dec) * sin(radian_ra);
   orig[3] = sin(radian_dec);


   if(from_std == XFORM_TOSTDEP && just_precess == XFORM_DOAPPAR) 
     /* if you're transforming from jd to std epoch, and doing apparent place,
	first step is to de-aberrate while still in epoch of date ... */	
  	 aberrate(date_epoch, orig, from_std); 
 
	
   for(i = 1; i<=3; i++) {
		fin[i] = 0.;
		for(j = 1; j<=3; j++) {
		  fin[i] += r[i][j] * orig[j];
	  }
   }

   if(from_std == XFORM_FROMSTD && just_precess == XFORM_DOAPPAR) 
	/* if you're transforming from std epoch to jd,
         last step is to apply aberration correction once you're in 
         equinox of that jd. */
  	 aberrate(date_epoch, fin, from_std); 

   /* convert back to spherical polar coords */

   xyz_cel(fin[1], fin[2], fin[3], rout, dout);

   return;	
}

double parang(ha,dec,lat) 
            
	double ha,dec,lat; 
             
 /* decimal hours, degrees, and degrees. */

{
	/* finds the parallactic angle.  This is a little 
	   complicated (see Filippenko PASP 94, 715 (1982) */

	double colat,codec,hacrit,sineta,denom;
	
	ha = ha / HRS_IN_RADIAN;
	dec = dec / DEG_IN_RADIAN;
	lat = lat / DEG_IN_RADIAN;

	/* Filippenko eqn 10 follows -- guarded against division by zero
             at the exact zenith .... */        
	denom = 
	   sqrt(1.-pow((sin(lat)*sin(dec)+cos(lat)*cos(dec)*cos(ha)),2.));
	if(denom != 0.)
	    sineta = sin(ha)*cos(lat)/denom;
	else sineta = 0.;
     
	if(lat >= 0.) {
		/* northern hemisphere case */

		/* If you're south of zenith, no problem. */

		if(dec<lat) return (asin(sineta)*DEG_IN_RADIAN);

		else {
			/* find critical hour angle -- where parallactic
				angle becomes 90 deg.  After that,
				take another root of expression. */
			colat = PI /2. - lat;
			codec = PI /2. - dec;
			hacrit = 1.-pow(cos(colat),2.)/pow(cos(codec),2.);
			hacrit = sqrt(hacrit)/sin(colat);       
			if(fabs(hacrit) <= 1.00) hacrit = asin(hacrit);
			//else oprntf("Error in parang..asin(>1)\n");
			if(fabs(ha) > fabs(hacrit))
				return(asin(sineta)*DEG_IN_RADIAN);
				/* comes out ok at large hour angle */
			else if (ha > 0)
				return((PI - asin(sineta))*DEG_IN_RADIAN);
			else return((-1.* PI - asin(sineta))*DEG_IN_RADIAN);
		}
	}
	else {  /* Southern hemisphere case follows */
		/* If you're north of zenith, no problem. */
		if(dec>lat) {
			if(ha >= 0) 
				return ((PI - asin(sineta))*DEG_IN_RADIAN);
			else return(-1*(PI + asin(sineta)) * DEG_IN_RADIAN);
		}
		else {
			/* find critical hour angle -- where parallactic
				angle becomes 90 deg.  After that,
				take another root of expression. */
			colat = -1*PI/2. - lat;
			codec = PI/2. - dec;
			hacrit = 1.-pow(cos(colat),2.)/pow(cos(codec),2.);
			hacrit = sqrt(hacrit)/sin(colat);       
			if(fabs(hacrit) <= 1.00) hacrit = asin(hacrit);
			//else oprntf("Error in parang..asin(>1)\n");
			if(fabs(ha) > fabs(hacrit)) {
				if(ha >= 0) 
				    return((PI - asin(sineta))*DEG_IN_RADIAN);
				else return(-1. * (PI + asin(sineta))*DEG_IN_RADIAN);
			}
			else return(asin(sineta)*DEG_IN_RADIAN);
		}
	}
}

double near_hor_refr(app_alt, pressure) 
	double app_alt, pressure;
{

	/* Almanac 1992, p. B62 -- ignores temperature variation */
	/* formula for near horizon, function-ized for iteration ... */
	
	double r;
        r = pressure * 
		(0.1594 + 0.0196 * app_alt + 0.00002 * app_alt * app_alt) /
		(293. * (1. + 0.505 * app_alt + 0.0845 * app_alt * app_alt));
	return(r);
}

double refract_size(double alt,double  elev)
//	double alt;   /* altitude in degrees */
//	double elev;  /* meters */

{
 	/* Almanac for 1992, p. B 62.  Ignores variation in temperature
           and just assumes T = 20 celsius.  */
	
	double pressure, r, altrad, crit_alt, app_alt;
	int i;

	altrad = alt / DEG_IN_RADIAN;

	pressure = 1013. * exp(-1. * elev/8620.);
		/* exponential atmosphere
		at T = 20 C, g = 980 cm/s^2,  and mean molecular wgt 28.8 */
	
	if(alt > 89.9) return(0.); /* avoid blowing up */
	else if (alt >= 15.0) {
		r = 0.00452 * pressure / (293. * tan(altrad));
		return(r);
	}
	else {  /* here have to start worrying about distinction between
                apparent and true altitude ... a pity as true altitude is
		what is handed in ... */
		crit_alt = -1. * pressure * 0.1594 / 293.;
		/* that's the *true* altitude corresponding to an
			*apparent* altitude of zero ... forget it if
			    it's below this. */
		if (alt > crit_alt) { /* go ahead and get it ... */ 
			app_alt = alt;  /* initial ... */
			for(i = 1; i <= 3; i++) {
			/* tests show 3 iterations is good to < 0.5 arcmin
			   for objects below geom horizon just barely rising ..
			   further accuracy is spurious. */
		           r = near_hor_refr(app_alt, pressure); 
			   app_alt = alt + r;
			}
			r = near_hor_refr(app_alt, pressure);
			return(r);
		}
	
		else {
			return(-1.);  /* below horizon. */
		}
	}
}

void refract_corr(	double *ha,double *dec,double *size,
													double lat,double ,
													int sense)
//void refract_corr(ha , dec, lat, elev, size, sense)
//	double *ha, *dec, *size, lat, elev;
// meters for elevation  size is amount of correction.
 //	int sense;
//
/* if sense == 1 , applies refraction to a true ha and dec; if
   == -1, de-corrects already refracted coordinates. Uses elevation of
   observatory above sea level to estimate a mean atmospheric pressure. */

{
	double x,y,z, xpr, ypr, localdec, localha, alt, az, norm,
		sinlat, coslat;
	localdec = *dec / DEG_IN_RADIAN;
	localha = *ha / HRS_IN_RADIAN;
	lat = lat / DEG_IN_RADIAN;  /* thank heavens for pass-by-value */
	sinlat = sin(lat);  coslat = cos(lat);

	/* The calculation is done by computing xyz coordinates in the
           horizon system, adding to the vertical component, renormalizing
           back to a unit vector, rotating to the polar system, and
           transforming back to ha and dec .... a long way around the
           barn, but completely general. */

	x =  cos(localdec)*cos(localha)*coslat + sin(localdec)*sinlat;
		/* vertical component */
	y =  sin(localdec)*coslat - cos(localdec)*cos(localha)*sinlat; 
		/* due N comp. */
	z =  -1. * cos(localdec)*sin(localha); 
		/* due east comp. */

	*size = refract_size(DEG_IN_RADIAN * asin(x), 0.);
		/* (this gives zero for zenith) */
		
	if(*size > 0.) {  /* guard against singular result at zenith */
     		norm = pow((y * y + z * z), 0.5);  /* in-ground component */
	 
		x = norm * tan(atan(x/norm) + sense * *size / DEG_IN_RADIAN);
		norm = pow((x*x + y*y + z*z),0.5);
		
		x = x / norm; y = y / norm; z = z / norm;		
	
		xpr = x * coslat - y * sinlat;
		ypr = x * sinlat + y * coslat;
	
		*dec = asin(ypr) * DEG_IN_RADIAN;
		*ha = -1. * atan2(z, xpr) * HRS_IN_RADIAN;
	}
}



