// decimal hours.
#define MDMHOURLONGITUDE					7.441106667
//#define MDMLONGITUDE					-111.61660
// decimal degrees.
#define MDMLATITUDE					31.95133
#define MDMLATITUDERAD			0.557632696
#define MDMLATITUDECOS			0.848497931
#define MDMLATITUDESIN			0.529198697
#define MDMLATITUDEARCSEC		(MDMLATITUDE*3600.0)
// in meters.
#define MDMELEVATION					1925.0
// acceleration in volts per millisecond.
//#define ACCELTIME		(8.0/10.0*1000.0)
/*
	 Speed and frequency calculation constants.
 */
#define RASTEPSIZE		0.051427
#define DECSTEPSIZE		0.051427
#define DEFAULTSETSPEED (60.0)
#define DEFAULTGUIDESPEED (2.0)
#define DEFAULTFASTSPEED (4.0)
#define DEFAULTOFFSETSPEED (30.0)
/*
	Encoder Translations to and from arcseconds.
 */// conversion of raw encoder counts to arcseconds.
#define DECRAWTOARCSEC			0.499474
#define RARAWTOARCSEC				0.749019

//#define DECRAWTOARCSEC			1.06299
//#define RARAWTOARCSEC				0.999836

// (360.0*3600.0/2322.0)
#define	DOMERAWTOARCSEC			558.1395348
#define INSTRAWTOARCSEC			(0.01*3600.0)
#define FOCUSRAWTOARCSEC		3600.0 // focus is an arbitrary value not an angle.
/*
		 Degrees and hours to arcseconds.
 */
#define DEGREESTOARCSEC			3600.0
#define HOURSTOARCSEC	    	54000.0

#define DEGREESTORADIANS		0.0174532925
#define HOURSTORADIANS		0.261799388

#define SIDEREALRATE		1.002737909
#define ADJUSTEDSIDEREALRATE	1.002866285
