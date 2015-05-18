
/* Hi Robb .... here are rewrites of two short skycalc routines which
   are used in the TCS code.  The rewrites should be mathematically
   equivalent but they are much clearer in case anyone else ever
   looks at it ..... so perhaps you can include them. 
          regards, 
		John
 */
   
#define  TWOPI             6.28318530717959
#define  PI_OVER_2         1.57079632679490  /* From Abramowitz & Stegun */   

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
//old...
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
