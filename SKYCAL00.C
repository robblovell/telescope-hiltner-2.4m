
#include "skycalc.h"
void refract_corr(ha , dec, lat, elev, size, sense)
	double *ha, *dec, *size, lat, elev;
	int sense;
	
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

void mass_precess() {
  
    double rorig = 1., dorig, orig_epoch, final_epoch, rf, df, 
            mura = 0., mudec = 0., dt;
    short do_proper = 0;

    printf("Mass precession.  The '=' command does precessions in a pinch, but\n");
    printf("the present command is convenient for doing several (all with same\n");
    printf("starting and ending epochs).  This routine does not affect parameters\n");
    printf("in the rest of the program.\n\n");
    printf("Type 1 if you need proper motions (They're a pain!), or 0:");
    scanf("%hd",&do_proper);
    if(do_proper == 1) {
	printf("\nA proper motion correction will be included -- it's a simple\n");
	printf("linear correction (adds mu * dt to coordinate).  Proper motion\n");
	printf("itself is NOT rigorously transformed (as in B1950->J2000, which\n");
	printf("involves a small change of inertial reference frame!)\n\n");
    }
    printf("Give epoch to precess from:");
    scanf("%lf",&orig_epoch);
    printf("Give epoch to precess to:");
    scanf("%lf",&final_epoch);
    dt = final_epoch - orig_epoch;
	 while(rorig >= 0.) {
	printf("\nGive RA for %7.2f (h m s, -1 0 0 to exit):",orig_epoch);
		  rorig = get_coord();
        if(rorig < 0.) {
		printf("Exiting precession routine.  Type '?' if you want a menu.\n");
		return;
        }
        printf("Give dec (d m s):");
        dorig = get_coord();
	if(do_proper == 1) get_pm(dorig,&mura,&mudec);
		  cooxform(rorig, dorig, orig_epoch, final_epoch, &rf, &df,
	     XFORM_JUSTPRE, XFORM_FROMSTD);	
	rf = rf + mura * dt / 3600.;
        df = df + mudec * dt / 3600.;
        oprntf("\n\n %7.2f : RA = ",orig_epoch);
        put_coords(rorig,4);
        oprntf(", dec = ");
        put_coords(dorig,3);
        oprntf("\n %7.2f : RA = ",final_epoch);
        put_coords(rf,4);
        oprntf(", dec = ");
        put_coords(df,3);
        if(do_proper == 1) oprntf("\n RA p.m. = %8.4f sec/yr, dec = %8.3f arcsec/yr",mura,mudec);
	oprntf("\n");
    }
}


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
        else oprntf("Error in min_max_alt -- arcsin(>1)\n");
        x = sin(dec)*sin(lat) - cos(dec)*cos(lat);
        if(fabs(x) <= 1.) {
                *min = asin(x) * DEG_IN_RADIAN;
        }
        else oprntf("Error in min_max_alt -- arcsin(>1)\n");
}

double altit(dec,ha,lat,az)
            
        double dec,ha,lat,*az;
            
/* returns altitude(degr) for dec, ha, lat (decimal degr, hr, degr); 
    also computes and returns azimuth through pointer argument. */
{
        double x,y,z;
        dec = dec / DEG_IN_RADIAN;
        ha = ha / HRS_IN_RADIAN;
        lat = lat / DEG_IN_RADIAN;  /* thank heavens for pass-by-value */
        x = DEG_IN_RADIAN * asin(cos(dec)*cos(ha)*cos(lat) + sin(dec)*sin(lat));
        y =  sin(dec)*cos(lat) - cos(dec)*cos(ha)*sin(lat); /* due N comp. */
        z =  -1. * cos(dec)*sin(ha); /* due east comp. */
        *az = atan_circ(y,z) * DEG_IN_RADIAN;   
        return(x);
}

double secant_z(alt)
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
                oprntf("Error in ha_alt ... acos(>1).\n");
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
