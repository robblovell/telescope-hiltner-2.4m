#include <stdio.h>
#include "skycalc.h"
#include "hooks.h"
// time of day and other time formats.
// date_time structure is declared in skycalc.h
struct date_time univtime,local_time;
struct date_time siderealtime;
double juliandate;
double siderealhours;
double airmass;
// current epoch is the current time.
// standard epoch is the epoch of coordinates used by the astronomer.
double currentepoch,stdepoch=2000.0;
// current location of the telescope.
double ramean,decmean,rameantarget,decmeantarget; // catalog locations.
struct coord meanra,meandec,targetha,targetdec,targetramean,targetdecmean,teleha,teledec,apparentra,apparentdec;

double hatele,dectele,hatarget,dectarget; // ra 0 at zenith,  dec 0 at north star.
double haarcsec,decarcsec;  //
double haraw,decraw,harawtarget,decrawtarget;/*
		 This file contains procdures which perform functions that an
	astronomer might be interested in changing.
*///double dectracrate=0.0;ratrackrate=15.0,guiderate=2.0,setrate=60.0,offsetrate=100.0; //arcsec/sec
void get_time ()
{
		get_dates_n_times (local_time,univtime,juliandate,siderealhours,currentepoch);

		// figure out sidreal hours minutes and seconds.

		decimalhour_to_hms (siderealhours,siderealtime);
		// following is not used....
		siderealtime.y = univtime.y;
		siderealtime.mo = univtime.mo;
		siderealtime.d = univtime.d;
}

main ()
{
	int rah,ram,dech,decm;
	double ras,decs;
	char ch = ' ';
	while (ch != 'q') {
		scanf ("%c",&ch);
		 printf ("\nra? ");
		scanf ("%d %d %lf",&rah,&ram,&ras);
		rameantarget = (double)rah+(double)ram/60.0 + ras/3600.0;
		printf ("\ndec? ");
		scanf ("%d %d %lf,",&dech,&decm,&decs);
		decmeantarget = (double)dech+(double)decm/60.0 + decs/3600.0;
		get_time ();
		printf ("epoch? ");
		scanf ("%lf",&stdepoch);
		printf ("%lf %lf %lf \n",rameantarget,decmeantarget,stdepoch);

		set_coordinates ();		printf ("ha & dec at end.... %lf %lf current epoch:%lf \n",
							hatarget,dectarget,currentepoch);
	}
 }
