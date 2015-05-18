#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <iostream.h>
#include "iopl.h"
extern "C" {
#include <math.h>
#include "const.h"
#include <conio.h>#include <stdlib.h>   /* include standard library file				*/
#include <string.h>
}

#define	FORWARD_LO		0
#define	REVERSE_LO		2

#define	FORWARD_HI		1
#define	REVERSE_HI		3

#define 	LOW_RES		16
#define	HIGH_RES		32

#define RES32	4294967296  //0-7
#define RES16	65536
#define COUNTERRES32	32
#define COUNTERRES16	16   //0-7
#define	CASCADE12	12
#define	CASCADE23	23
#define	CASCADE34	34
#define	CASCADE45	45
	double min, max, range;

	 /*
			Each encoder can move in two directions, forward and
	 reverse and two or four counters to keep track of the motion.
	 When two counters are used, the resolution is 16 bits, and one
	 counter is used for forward and the other for reverse.  When
	 four counters are used, the resolution is 32 bits, and two
	 counters are cascaded for one direction, two for the other.
	*/
	unsigned long afwd,arev; // Each encoder can move in both directions.
	unsigned long fwdroll,revroll;
	double rawlocation,encoderlocation,arcseclocation,location;
	int keeppositve;
	// short fwdroll,revroll;
	int count[4];		  // up to four counters to digitize 1 encoder.
	int resolution;// counter resoution=16 for 16 bit, resolution=32 for 32 bit.
	double cntrangepow =pow(2,resolution);;    // range of the encoder device.
	double toarcsec;		// scale tor translate encoder counts to arc seconds

  /*
			An encoder can be located on any two of three boards in the system,
	  using up to four of any five counters per board.  Individual
	  directions can be on different boards.  If a encoder is 16 bit,
	  then only one counter is used for each direction.  If an encoder
	  is 32 bit then two counters are used for each direction, either
		counters 2 and 3 or counters 4 and 5. All values in the arrays
	  must be set.  Counters are labeled 1 through 5, boards 1 through 3.
	*/

		// 1-3
  int counter[2];    // 1-5

assign_fwd_n_rev (unsigned long tmp[6],unsigned long &fwd,unsigned long &rev)
{

	switch (resolution) {
	 default:
	 case 16:
		fwd = tmp[FORWARD_LO];
		rev = tmp[REVERSE_LO];
		break;
	 case 32:
		fwd = (unsigned long)tmp[FORWARD_HI];
		rev = (unsigned long)tmp[REVERSE_HI];
	 //	fwd *= 65536;
	 //	rev *= 65536;
		fwd <<= 16; // 65536;
		rev <<= 16; //5536;
		//fwd &= (unsigned long )0xffff0000;
		//rev &= (unsigned long )0xffff0000;
		fwd += (unsigned long)tmp[FORWARD_LO];
		rev += (unsigned long)tmp[REVERSE_LO];
cerr <<"fwd_n_rev: "<<fwd<<"  "<<rev<<endl;
		break;
 }
}
double calculate_location (unsigned long fwd,unsigned long rev)
{
	// test to see if counter rolled over.
	// this works for devices which can't turn in a complete circle.

	if ((double)fwd-(double)rev < -range/2.0 ||
			(double)fwd-(double)rev > range/2.0) { // rolled over.
		// fact: fwd != rev
	//	if (fwd > rev) { // rev rolled over.
			rawlocation  = (double)fwd-((double)rev+range);
	//	}
	 //	else if (rev > fwd) {
	 //		rawlocation = (double)rev-((double)fwd+range);
	 //	}
	}
	else {
		rawlocation = (double)fwd-(double)rev;
	}

 
//		rawlocation = (double)fwd-(double)rev;

	char buf[100];
	if (keeppositve) {
		if ((long)rawlocation == 0) encoderlocation = (long) rawlocation;
		else if ((long)range == 0) encoderlocation = (long) rawlocation;
		else {
			encoderlocation = (long)rawlocation % (long)range;
			while (encoderlocation < 0) encoderlocation += range;
			while (encoderlocation > range) encoderlocation -= range;
		}
		arcseclocation = encoderlocation*toarcsec;

		location = fabs (((double)arcseclocation/3600.0));
	}
	else {
		if ((long)rawlocation == 0) encoderlocation = (long) rawlocation;
		else if ((long)range == 0) encoderlocation = (long) rawlocation;
		else encoderlocation = (long)rawlocation % (long)range;
		arcseclocation = ((double)encoderlocation*toarcsec);
		location = ((double)arcseclocation/3600.0);
	}
//	 sprintf (buf,"fwd:%d  rev:%d  arcsecs:%d  location:%d  raw loc:%d  %lf",
//fwd,rev,arcseclocation,(long)((double)arcseclocation/3600.0),rawlocation,toarcsec);
//	 debug.post (buf);
//	 debug.dout (buf);
	return encoderlocation;
}
/*
	After a "read_counters", the values of the counters are in a 5 position
	array.  This procedure takes those values and assigns them to the class's
	fwd & rev variables depending on the resolution of the counter.
 */


/*
	To set the counters the values of forward and reverse have to be
	placed in a five position array.  This is the opposite of assign_fwd_n_rev.

 */
void fwd_n_rev_to_bytes (unsigned long tmp[6],unsigned long fwd,unsigned long rev)
{
	switch (resolution) {
	 default:
	 case 16:
			tmp[FORWARD_LO] = fwd;
			tmp[REVERSE_LO] = rev;
		break;
	 case 32:
			tmp[FORWARD_HI] = (unsigned short) (fwd>>16 & (unsigned long)0xffff);
			tmp[REVERSE_HI] = (unsigned short) (rev>>16 & (unsigned long)0xffff);
			tmp[FORWARD_LO] = (unsigned short) (fwd & (unsigned long)0xffff);
			tmp[REVERSE_LO] = (unsigned short) (rev & (unsigned long)0xffff);
		break;

 }
/*	char buf[256];
	sprintf (buf,"BYTES OF FWD N REV::fwd:%d rev%d fwd lo:%ud  fwd hi:%ud  rev lo:%ud  rev hi:%ud ",
				fwd,rev,tmp[FORWARD_LO],tmp[FORWARD_HI],
				tmp[REVERSE_LO],tmp[REVERSE_HI]);
	debug.post (buf);	debug.dout (buf);
*/
}

main ()
{
	 int i;
	 unsigned long tmp[6];
	 unsigned long up,down;
	 double thefwd=0,therev=0;
	 double loc,loc2;
	 unsigned long cntrange;
	 resolution = 32;
	 cntrange = pow (2,resolution);
	 keeppositve = 0;
	 range = pow(2,resolution); min = 0; max = range-1;
	 toarcsec = .499;
	 afwd = 0; arev = 0;
	 for (i=0;i<=10000;i+= 1) {
			if (abs (thefwd-therev) > cntrange/4.0)
				thefwd+= 1000000000.0;
			else therev+=1000000000.0;
			afwd = (unsigned long)thefwd;//%cntrange;
			arev = (unsigned long)therev;//%cntrange;
			fwd_n_rev_to_bytes (tmp,afwd,arev);
			assign_fwd_n_rev (tmp,up,down);
			loc = calculate_location (afwd,arev);
			loc2 = calculate_location (up, down);
			cerr <<"i:"<<i<<"f:"<<afwd<<" r:"<<arev<<" l:"<<loc<<"  ";
			cerr <<"u:"<<up<<" d:"<<down<<" l:"<<loc2<<endl;
			cerr <<tmp[0]<<"  "<<tmp[1]<<"  "<<tmp[2]<<"  "<<tmp[3]<<endl;
      DosSleep (500);
	 }

}

