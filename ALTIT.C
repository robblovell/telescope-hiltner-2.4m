

double altit(dec,ha,lat,az,parang)
            
	double dec,ha,lat,*az,*parang;
            
/* returns altitude(degr) for dec, ha, lat (decimal degr, hr, degr); 
    also computes and returns azimuth through pointer argument,
		and as an extra added bonus returns parallactic angle (decimal degr)
    through another pointer argument. */
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
	*az = atan2(z,y);   

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
	while(*az < 0.) *az += 360.;  /* force 0 -> 360 */
	while(*az >= 360.) *az -= 360.;
	
	return(x);
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


