#include <stdio.h>
#include <math.h>

#define HOURSTORADIANS     0.261799388
#define DEGREESTORADIANS   0.0174532925
#define ARCSEC_IN_HOUR 54000.
#define ARCSEC_IN_DEGREE 3600.
#define MDMLATITUDE 31.9514
#define MDMELEVATION 1925.

void apply_telescope_model(teleha, teledec, sense)
   double *teleha, *teledec;
   int sense;

/* Applies corrections to an hour angle (in arcseconds) and a
declination (in arcseconds) to model for telescope imperfections.
It is assumed that telescope-independent corrections ....  precession, 
nutation, aberration, and refraction ... are
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

{

   double radha, raddec, dha = 0., ddec = 0.;

 /* TPOINT pointing correction coefficients ...
        all are in arcsec.  Notation follows TPOINT.  */

   double
        IH = -89.42,   /* index error in hour angle */
        ID = 90.68,    /* index error in dec .. interacts with poly terms */
        PDD = -231.05, /* first order poly term in dec */
        NP = -6.14,    /* non-perpendicularity of HA and dec axes */
        MA = -3.47,    /* east-west error of polar axis */
        ME = 20.56,    /* north-south error of polar axis */
        CH = 64.52,    /* non-perpendicularity of dec and optical axes */
        PDD2 = 19.94;  /* 2nd order poly term in dec */

   radha = *teleha * HOURSTORADIANS;
   raddec = *teledec * DEGREESTORADIANS;

   dha += IH;

   ddec += ID;

   ddec += PDD * raddec;   /* here's hoping they wanted argument in radians  */

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
        *teleha -= dha / ARCSEC_IN_HOUR;
        *teledec -= ddec / ARCSEC_IN_DEGREE;
   }
   else if(sense == -1) {  /* telescope coords -> true coords */
        *teleha += dha / ARCSEC_IN_HOUR;
        *teledec += ddec / ARCSEC_IN_DEGREE;
   }
   else printf("error in point-model: illegal sense.\n");
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


double refract_size(alt, elev)
        double alt;   /* altitude in degrees */
        double elev;  /* meters */

{
        /* Almanac for 1992, p. B 62.  Ignores variation in temperature
           and just assumes T = 20 celsius.  */

        double pressure, r, altrad, crit_alt, app_alt;
        int i;

        altrad = alt * DEGREESTORADIANS;

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

refract_corr(ha , dec, lat, elev, size, sense)
        double *ha, *dec, *size, lat, elev;
        int sense;

/* if sense == 1 , applies refraction to a true ha and dec; if
   == -1, de-corrects already refracted coordinates. Uses elevation of
   observatory above sea level to estimate a mean atmospheric pressure. */

{
        double x,y,z, xpr, ypr, localdec, localha, alt, az, norm,
                sinlat, coslat;
        localdec = *dec * DEGREESTORADIANS;
        localha = *ha * HOURSTORADIANS;
        lat = lat * DEGREESTORADIANS;  /* thank heavens for pass-by-value */
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

        *size = refract_size(asin(x)/DEGREESTORADIANS, 0.);
                /* (this gives zero for zenith) */

        if(*size > 0.) {  /* guard against singular result at zenith */
                norm = pow((y * y + z * z), 0.5);  /* in-ground component */

                x = norm * tan(atan(x/norm) + sense * *size * DEGREESTORADIANS);
                norm = pow((x*x + y*y + z*z),0.5);

                x = x / norm; y = y / norm; z = z / norm;

                xpr = x * coslat - y * sinlat;
                ypr = x * sinlat + y * coslat;

                *dec = asin(ypr) / DEGREESTORADIANS;
                *ha = -1. * atan2(z, xpr) / HOURSTORADIANS;
        }
}

void ratecorr_size(ha, dec, lat, elev, haratecorr, decratecorr) 

double ha, dec, lat, elev, *haratecorr, *decratecorr;

/* Units are decimal hours, decimal degrees x 2, meters; returns rate 
   corrections in arcsec per second in the two axes. 

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
	refract_corr(&ha1appar, &dec1appar, lat, elev,
	      &size, 1); 
	printf("ha1 %lf dec %lf  refract: %lf %lf\n",
		ha1,dec,ha1appar,dec1appar);
	apply_telescope_model(&ha1appar, &dec1appar, 1);
	printf("flexed: %lf %lf\n", ha1appar,dec1appar);

	ha2appar = ha2; dec2appar = dec;  /* copy */
	refract_corr(&ha2appar, &dec2appar, lat, elev,
	      &size, 1); 
	printf("ha2 %lf dec %lf  refract: %lf %lf\n",
		ha2,dec,ha2appar,dec2appar);
	apply_telescope_model(&ha2appar, &dec2appar, 1);
	printf("flexed: %lf %lf\n", ha2appar,dec2appar);
	
	diffha = (ha2appar - ha1appar - interval) * ARCSEC_IN_HOUR;
	
	diffdec = (dec2appar - dec1appar) * ARCSEC_IN_DEGREE;
	
	printf("Differences in ha and dec (arcsec): %lf %lf\n",
		diffha, diffdec);
	
	/* put out rate corrs in arcseconds per second. */
 
	*haratecorr = diffha / (interval * 3600 * 1.0027379093);
	*decratecorr = diffdec / (interval * 3600 *  1.0027379093); 

	/* the 1.0027379093 is ridiculous ... takes account of fact that
           computation interval is "interval" sidereal hours, not
           normal hours.  */

}

main() {

	/* testbed */

	double ha = 0., dec, haratecorr, decratecorr;
	
	while(ha > -1000.) {
		printf("Give hour angle and dec (decimal hrs and deg):");
		scanf("%lf",&ha);
		if(ha < -1000.) break;
		scanf("%lf",&dec);
		ratecorr_size(ha, dec, MDMLATITUDE,
			MDMELEVATION, &haratecorr, &decratecorr);
		printf("rate corrs: ha %lf  dec %lf\n",
			haratecorr, decratecorr);
	}
}
