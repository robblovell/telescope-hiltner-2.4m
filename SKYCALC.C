

/* SKY CALCULATOR PROGRAM 

	John Thorstensen, Dartmouth College.

   This program computes many quantities frequently needed by 
   the observational astronomer.  It is written as a completely
   self-contained program in standard c, so it should be
   very transportable; the only issue I know of that really affects
   portability is the adequacy of the double-precision floating
   point accuracy on the machine.  Experience shows that c compilers
   on various systems have idiosyncracies, though, so be sure
   to check carefully.

   This is intended as an observatory utility program; I assume the
   user is familiar with astronomical coordinates and nomenclature.
   While the code should be very transportable, I also 
   assume it will be installed by a conscientious person who
   will run critical tests before it is released at a new site.
   Experience shows that some c compilers generate unforseen errors
   when the code is ported, so the output should be checked meticulously
   against data from other sites.
 
   The first part (the almanac) lists the phenomena for a single night (sunset, 
   twilight, moonrise, mooset, etc.) in civil clock time. 
   The rise-set and twilight times given are good
   to a minute or two; the moon ephemeris used for rise/set is good to 
   +- 0.3 degrees or so; it's from the Astronomical Almanac's 
   low precision formulae, (with topocentric corrections included).
   The resulting moon rise/set times are generally good to better than
   two minutes.  The moon coordinates for midnight and in the 'calculator
   mode' are from a more accurate routine and are generally better than
   1 arcmin.  The elevation of an observatory above its effective
   horizon can be specified; if it is non-zero, rise/set times are
   corrected approximately for depression of the horizon.

   After displaying the phenomena for one night, the program goes
   into a 'calculator mode', in which one can -

	- enter RA, dec, proper motion, epoch, date, time, 
	     new site parameters, etc. ...

	- compute and display circumstances of observation for the
	   current parameters, including precessed coordinates,
	   airmass, interference from moon or twilight, parallactic
	   angle, etc; the program also gives calendar date in 
	   both UT and local, Julian date, and barycentric corrections. 

	- compute and display a table of airmasses (etc) at
	   hourly intervals through the night.  This is very useful
	   at the telescope.  Also, if one has a modest number of 
	   objects, it may be convenient (using system utilities)
	   to redirect the output and print a hard copy of these 
	   tables for ready reference.

	- compute and display galactic and ecliptic coordinates.
   
	- compute and display rough (of order 0.1 degree, but often
	  much better) positions of the major planets.

	- display the almanac for the current night.

    The program is self-contained.  It was developed on a VMS system,
   but should adapt easily to any system with a c compiler.  It has
   been ported to, and tested on, several popular workstations.
   
	** BUT CAUTION ... **
   Because many of the routines take a double-precision floating point
   Julian Date as their time argument, one must be sure that the machine
   and compiler carry sufficient mantissa to reach the desired accuracy.
   On VAX/VMS, the time resolution is of order 0.01 second.  This has also
   proven true on Sun and IBM workstations.  

LEGALITIES: 

   I make no guarantee as to the accuracy, reliability, or
   appropriateness of this program, though I have found it to be 
   reasonably accurate and quite useful to the working astronomer.
   
   The program is COPYRIGHT 1993 BY JOHN THORSTENSEN.  
   Permission is hereby granted for non-profit scientific or educational use.
   For-profit use (e. g., by astrologers!) must be through negotiated
   license.  The author requests that observatories and astronomy 
   departments which install this as a utility notify the author
   by paper mail, just so I know how widely it is used.     

   Credits:  
    * The julian date and sidereal time routines were 
    originally coded in PL/I by  Steve Maker of Dartmouth College.  
    They were based on routines in the old American Ephemeris.
    * The conversion from julian date to calendar date is adapted
    from Numerical Recipes in c, by Press et al. (Cambridge University
    Press). I highly recommend this excellent, very useful book.


    APOLOGIES/DISCLAIMER:
    I am aware that the code here does not always conform to
    the best programming practices.  Not every possible error condition
    is anticipated, and no guarantee is given that this is bug-free.
    Nonetheless, most of this code has been shaken down at several
    hundred sites for several years, and I have never received any
    actual bug reports.  Many users have found this program
    to be useful.

    CHANGES SINCE THE ORIGINAL DISTRIBUTION ....

	The program as listed here is for the most part similar to that
	posted on the IRAF bulletin board in 1990.  Some changes
	include:

	01 In the original code, many functions returned structures, which
	   some c implementations do not like.  These have been eliminated.

	02 The original main() was extremely cumbersome; much of it has
	   been broken into smaller (but still large) functions.

	03 The hourly airmass includes a column for the altitude of the
	   sun, which is printed if it is greater than -18 degrees.

	04 The planets are included (see above).  As part of this, the
	   circumstances calculator issues a warning when one is within
	   three degrees of a major planet.  This warning is now also
	   included in the hourly-airmass table.
    
	05 The changeover from standard to daylight time has been rationalized.
	   Input times between 2 and 3 AM on the night when DST starts (which
	   are skipped over and  hence don't exist) are now disallowed; input 
	   times between 1 and 2 AM on the night when DST ends (which are
	   ambiguous) are interpreted as standard times.  Warnings are printed
	   in both the almanac and calculator mode when one is near to the
	   changeover.

	06 a much more accurate moon calculation has been added; it's used
	   when the moon's coordinates are given explicitly, but not for
	   the rise/set times, which iterate and for which a lower precision 
	   is adequate.

	07 It's possible now to set the observatory elevation; in a second 
	   revision there are now two separate elevation parameters specified.
	   The elevation above the horizon used only in rise/set calculations 
	   and adjusts rise/set times assuming the parameter is the elevation 
	   above flat surroundings (e. g., an ocean).  The true elevation above
	   sea level is used (together with an ellipsoidal earth figure) in
	   determining the observatory's geocentric coordinates for use in 
	   the topocentric correction of the moon's position and in the 
	   calculation of the diurnal rotation part of the barycentric velocity
	   correction.  These refinements are quite small.

	08 The moon's altitude above the horizon is now printed in the 
	   hourly airmass calculation; in the header line, its illuminated
	   fraction and angular separation from the object are included,
	   as computed for local midnight.

	09 The helio/barycentric corrections have been revised and improved.
	   The same routines used for planetary positions are used to
	   compute the offset from heliocentric to solar-system
	   barycentric positions and velocities.  The earth's position
	   (and the sun's position as well) have been improved somewhat
	   as well.

	10 The printed day and date are always based on the same truncation
	   of the julian date argument, so they should now always agree
	   arbitrarily close to midnight.

	11 A new convention has been adopted by which the default is that the
	   date specified is the evening date for the whole night.  This way,
	   calculating an almanac for the night of July 3/4 and then specifying
	   a time after midnight gives the circumstances for the *morning of
	   July 4*.  Typing 'n' toggles between this interpretation and a
	   literal interpretation of the date.

	12 The planetary proximity warning is now included in the hourly airmass
	   table.

	13 A routine has been added which calculates how far the moon is from
	   the nearest cardinal phase (to coin a phrase) and prints a 
	   description.  This information is now included in both the almanac
	   and the calculator mode.

	14 The output formats have been changed slightly; it's hoped this 
	   will enhance comprehensibility.

	15 A substantial revision affecting the user interface took place 
	   in September of 1993.  A command 'a' has been added to the 
	   'calculator' menu, which simply prints the almanac (rise, set,
	   and so on) for the current night.  I'd always found that it was
	   easy to get disoriented using the '=' command -- too much 
	   information about the moment, not enough about the time
	   context.  Making the almanac info *conveniently* available
	   in the calculator mode helps your get oriented.

	   When the 'a' almanac is printed, space is saved over the 
	   almanac printed on entry, because there does not need
	   to be a banner introducing the calculator mode.  Therefore some 
	   extra information is included with the 'a' almanac; this includes
	   the length of the night from sunset to sunrise, the number of 
	   hours the sun is below -18 degrees altitude, and the number of hours 
	   moon is down after twilight.  In addition, moonrise and moonset
	   are printed in the order in which they occur, and the occasional
	   non-convergence of the rise/set algorithms at high latitude are
	   signalled more forcefully to the user.

	16 I found this 'a' command to be convenient in practice, and never
	   liked the previous structure of having to 'quit' the calculator
	   mode to see almanac information for a different night.  D'Anne
	   Thompson of NOAO also pointed out how hokey this was, especially the
	   use of a negative date to exit. So, I simply removed the outer 
	   'almanac' loop and added a 'Q' to the main menu for 'quit'.  The
	   use of upper case -- for this one command only --  should guard
	   against accidental exit.

	17 The menu has been revised to be a little more readable.  

	18 More error checking was added in Nov. 1993, especially for numbers.  
	   If the user gives an illegal entry (such as a number which isn't
	   legal), the rest of the command line is thrown away (to avoid
	   having scanf simply chomp through it) and the user is prompted
	   as to what to do next.  This seems to have stopped all situations
	   in which execution could run away.  Also, typing repeated carriage
	   returns with nothing on the line -- which a frustrated novice
	   user may do because of the lack of any prompts -- causes a
	   little notice to be printed to draw attention to the help and menu
	   texts.
	 
	19 I found in practice that, although the input parameters and
	   conditions are deducible *in principle* from such things as the
	   'a' and '=' output, it takes too much digging to find them.  So
	   I instituted an 'l' command to 'look' at the current parameter
	   values.  To make room for this I put the 'Cautions and legalities'
	   into the 'w' (inner workings) help text.  This looks as though
	   it will be be very helpful to the user. 

	20 The more accurate moon calculation is used for moonrise and
	   moonset; the execution time penalty appears to be trivial. 
	   Low precision moon is still used for the summary moon information
	   printed along with the hourly airmass table.

	21 A calculation of the expected portion of the night-sky 
	   brightness due to moonlight has been added.  This is based on
	   Krisciunas and Schaefer's analytic fits (PASP, 1991).  Obviously,
	   it's only an estimate which will vary considerably depending on
	   atmospheric conditions.

	22 A very crude estimator of the zenith sky brightness in twilight
	   has been added.

	23 A topocentric correction has been added for the sun, in anticipation
	   of adding eclipse prediction.

	24 The code now checks for eclipses of the sun and moon, by making
	   very direct use of the predicted positions.  If an eclipse is
	   predicted, a notice is printed in print_circumstances; also, a
	   disclaimer is printed for the lunar sky brightness if a lunar
	   eclipse is predicted to be under way.

	25 In the driver of the main calculator loop, a provision has been
	   added for getting characters out of a buffer rather than reading
	   them directly off the command line.  This allows one to type any
	   valid command character (except Q for quit) directly after a number 
	   in an argument without generating a complaint from the program 
	   (see note 18).  This had been an annoying flaw.

	26 In 1993 December/1994 January, the code was transplanted
	   to a PC and compiled under Borland Turbo C++, with strict
	   ANSI rules.  The code was cut into 9 parts -- 8 subroutine
	   files, the main program, and an incude file containing
	   global variables and function prototypes.

	27 An "extra goodies" feature has been added -- at present it
	   computes geocentric times of a repeating phenomenon as
	   viewed from a site.  This can be used for relatively little-used
           commands to save space on the main menu.  

	28 The sun and moon are now included in the "major planets"
	   printout.  This allows one to find their celestial positions
	   even when they are not visible from the selected site.
	
	29 A MAJOR new feature was added in February 1994, which computes
           the observability of an object at new and full moon over a 
           range of dates.  The galactic/ecliptic coordinate converter
           was moved to the extra goodies menu to make room for this.

	30 Inclusion of a season-long timescale means that it's not
           always necessary to specify a date on entry to the program.
           Accordingly, the program immediately starts up in what used
           to be called "calculator" mode -- only the site is prompted
           for.  It is thought that the site will be relevant to nearly
           all users.

	31 Because the user is not led by the hand as much as before, the
           startup messages were completely revised to direct new users
           toward a short `guided tour' designed to show the program's 
	   command structure and capabilities very quickly.  Tests on 
	   volunteers showed that users instinctively go for anything 
	   called the `menu', despite the fact that that's a slow way to 
	   learn, so all mention of the menu option is removed from the 
	   startup sequence; they'll catch on soon enough.

	32 Code has been added to automatically set the time and
           date to the present on startup.  A menu option 'T' has been
           added to set the time and date to the present plus a settable
           offset.  This should be very useful while observing.
	
	33 Because Sun machines apparently do not understand ANSI-standard
           function declarations, the code has been revised back to K&R
           style.  It's also been put back in a monolithic block for
           simplicity in distribution.
	
	34 The startup has been simplified still further, in that the
           coordinates are set automatically to the zenith on startup.
	   An 'xZ' menu item sets to the zenith for the currently specified
           time and date (not necessarily the real time and date.)

	35 Another MAJOR new capability was added in early 1994 --
           the ability to read in a list of objects and set the current
	   coordinates to an object on the list.  The list can be sorted
           in a number of ways using information about the site, date
           and time.
	
	35 Calculator-like commands were added to the extra goodies menu
           to do precessions and to convert Julian dates to calendar
           dates.  An option to set the date and time to correspond to
           a given julian date was also added.
	
	36 Another substantial new capability was added Aug 94 -- one can
           toggle open a log file (always named "skyclg") and keep
           a record of the output.  This is done simply by replacing
           most occurrences of "printf" with "oprintf", which mimics
			  printf but writes to a log file as well if it is open.
	   This appears to slow down execution somewhat.

	37 12-degree twilight has been added to the almanac.  While the
	   awkward "goto" statements have been retained, the statement
			  labels have been revised to make them a little clearer.

	38 The precession routine was generalized to include nutation and
		aberration, and routines to calculate these effects are now
			  included.  Nearly all calls to the new routine leave the
			  aberration and nutation out, but 'xa' extra goodies gives
			  apparent place up to aberration and nutation.  Tests against the
		FK5 and "Apparent Places of Fundamental Stars" shows the
			  agreement to 0.1 arcsec or smaller.

*/
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

extern double star_tzero, star_terr,
	star_period, star_perr;  /* for ephemeris calculations ... global */

FILE *sclogfl = NULL;

double star_tzero, star_terr,
	star_period, star_perr;  /* for ephemeris calculations ... global */

void oprntf(char *fmt, ...)

/* This routine should look almost exactly like printf in terms of its
	arguments (format list, then a variable number of arguments
	to be formatted and printed).  It is designed to behave just
	like printf (though perhaps not all format types are supported yet)
	EXCEPT that IF the globally-defined file pointer "sclogfl" is
	defined, IT ALSO WRITES TO THAT FILE using fprintf.  The skeleton
   for this came from Kernighan and Ritchie, 2nd edition, page 156 --
	their "minprintf" example.  I modified it to include the
   entire format string (e.g., %8.2f, %7d) and to write to the 
   file as well as standard output.  */

{
	va_list ap;        /* see K&R for explanation of these macros */
	char *p, *sval;
	char outform[10];  /* an item's output format, e.g. %8.2f */
	char strout[150];
	int ival, i;
	short shval;
	char cval;
	double dval;

	va_start(ap,fmt);
	for (p = fmt; *p; p++) {
		if (*p != '%') {
			putchar(*p);
/* overkill to put in these preprocessor flags, perhaps. */
#if LOG_FILES_OK == 1
			if(sclogfl != NULL) fputc(*p,sclogfl);
#endif
			continue;
		}
		i = 0;
		outform[i] = '%';
		p++;
		while(*p != 'd' && *p != 'f' && *p != 's' && *p != 'c'
		   && *p != 'h') {
			outform[++i] = *p++;
		}				
		switch (*p) {
		case 'd':			
			ival = va_arg(ap, int);
			outform[++i] = *p;
			outform[++i] = '\0';
			printf(outform, ival);
#if LOG_FILES_OK == 1 
			if(sclogfl != NULL)
				fprintf(sclogfl,outform,ival);
#endif
			break;
		case 'h':    /* signals short argument ... */			
			shval = va_arg(ap, short);
			outform[++i] = 'd';
			outform[++i] = '\0';
			++p;  /* skip the 'd' in '%hd' */
			printf(outform, shval);
#if LOG_FILES_OK == 1 
			if(sclogfl != NULL)
				fprintf(sclogfl,outform,shval);
#endif
			break;
		case 'c':			
			/* cval = va_arg(ap, char); 
			  ... compiler problem on Sun machines */
			cval = va_arg(ap, int);
			outform[++i] = *p;
			outform[++i] = '\0';
			printf(outform, cval);
#if LOG_FILES_OK == 1 
			if(sclogfl != NULL)
				fprintf(sclogfl,outform,cval);
#endif
			break;
		case 'f':
			dval = va_arg(ap, double);
			outform[++i] = *p;
			outform[++i] = '\0';
			printf(outform, dval);
#if LOG_FILES_OK == 1 
			if(sclogfl != NULL) 
				fprintf(sclogfl,outform,dval);
#endif
			break;
			
		case 's':
			outform[++i] = *p;
			outform[++i] = '\0';
			i = 0;
			for (sval = va_arg(ap, char *); *sval; sval++) {
				strout[i++] = *sval;
			}
			strout[i] = '\0';
			printf(outform,strout);
#if LOG_FILES_OK == 1 
			if(sclogfl != NULL) fprintf(sclogfl,outform,strout);
#endif
			break;
		default:
	                ;
		}
	}
	va_end(ap);
}

/* elements of K&R hp calculator, basis of commands */

char buf[BUFSIZE];
int bufp=0;


char getch() /* get a (possibly pushed back) character */
{
	return((bufp > 0) ? buf[--bufp] : getchar());
}

void ungetch(c) /* push character back on input */
	int c;
{
	if(bufp > BUFSIZE)
		printf("Ungetch -- too many characters.\n");
	else 
		buf[bufp++] = c;
}

/* some functions for getting well-tested input. */

int legal_num_part(c)
	char c; 

{
	if((c != '.') && (c != '-') && (c < '0' || c > '9')) 
		return(-1);  /* not a legal number part */
	else return(0);
}
 
int legal_int_part(c)
	char c; 
   
{
	if((c != '-') && (c < '0' || c > '9')) 
		return(-1);  /* not a legal number part */
	else return(0);
} 

int legal_command_char(c)
	char c;
{
	/* Allows more sophisticated argument checking by seeing if
	      a character appended to an argument is actually a 
	      legal commmand. */
	switch(c) {
	  case '?': return(1);

	  case 'i': return(1);

	  case 'f': return(1);
		
	  case 'w': return(1);
		  
	  case 'r': return(1);
		
	  case 'd': return(1);
		
	  case 'y': return(1);
		
	  case 't': return(1);
		
	  case 'T': return(1);
		
	  case 'n': return(1);
		
	  case 'g': return(1);
		
	  case 'e': return(1);
		
	  case 'p': return(1);
		
	  case 's': return(1);
		
	  case 'l': return(1);
		
	  case '=': return(1);
		
	  case 'a': return(1);
		
	  case 'h': return(1);
		
	  case 'o': return(1);
		
	  case 'm': return(1);
		
	  case 'c': return(1);
		
	  case 'x': return(1);
		
	  /* let's not allow 'Q' here! */
	  default: return(0);
	}
	return (0);
}

int parsedouble(s,d)
               
	char *s;
	double *d;
                  
	/* return values 0 = ok, with number, 1 = found a valid command,
	   but no number, and -1 = an error of some sort (unexpected char)*/

{
   short i=0, legal = 0;

   while((*(s+i) != '\0') && (legal == 0)) {
	if(legal_num_part(*(s+i)) == 0) i++;
	else if(legal_command_char(*(s+i)) == 1) {
		/* to allow command to follow argument without blanks */
		ungetch(s[i]);
		*(s+i) = '\0';  /* will terminate on next pass. */
	}
	else legal = -1;
   } 
   
   if(legal == 0) {
	if(i > 0) {
		sscanf(s,"%lf",d);
		return(0);
	} 
	else if (i == 0) { /* ran into a command character -- no input */
		*d = 0.;
		return(1);  /* ok, actually */
	}
   }
   else {
	printf("%s is not a legal number!! Try again!\n",s);
	return(-1);
	}
	return (1);
}


int getdouble(d,least,most,errprompt) 
             
	double *d,least,most;
	char *errprompt; 
             

{
    char s[30], buf[200], c;
    short success = -1, ndiscard = 0;

    scanf("%s",s);
    while(success < 0) {
	success = parsedouble(s,d);
	if((success == 0) && ((*d < least) || (*d > most))) {
	   printf("%g is out of range; allowed %g to %g -- \n",
			*d,least,most);
	   success = -1;
	} 
	if(success < 0) {
	   /* if there's error on input, clean out the rest of the line */
	   ndiscard = 0;
	   while((c = getchar()) != '\n')  {
		buf[ndiscard] = c;
		ndiscard++;
	   }
	   if(ndiscard > 0) {
		buf[ndiscard] = '\0';  /* terminate the string */         
		printf("Rest of input (%s) has been discarded.\n",buf);
	   }
	   printf("%s",errprompt);
	   printf("\nTry again:");
	   scanf("%s",s);
	}
    }
    return((int) success);
}    

int parseshort(s,d)
              
	char *s;
	short *d;
              
{
   short i=0, legal = 0;

   while((*(s+i) != '\0') && (legal == 0)) {
	if(legal_int_part(*(s+i)) == 0) i++;
	else if(legal_command_char(*(s+i)) == 1) {
		/* to allow command to follow argument without blanks */
		ungetch(s[i]);
		*(s+i) = '\0';  /* will terminate on next pass. */
	}
	else legal = -1;
   } 

   if(legal == 0) {
	if(i > 0) {
		sscanf(s,"%hd",d);
		return(0);
	} 
	else if (i == 0) { /* ran into a command character -- no input */
		*d = 0.;
		return(1);  /* didn't get a number, but something else legal */
	}
   } 
   else {
	printf("%s is not a legal integer number!! Try again!\n",s);
	return(-1);
	}
	return (1);
}

int getshort(d,least,most,errprompt) 

	short *d,least,most;
	char *errprompt;

{
    char s[30];
    short success = -1, ndiscard = 0;
    char c, buf[200];

    scanf("%s",s);
    while(success < 0) {
	success = parseshort(s,d);
	if((success == 0) && ((*d < least) || (*d > most))) {
	   printf("%d is out of range; allowed %d to %d -- try again.\n",
			*d,least,most);
	   success = -1;
	} 
	if(success < 0) {
	   /* if there's error on input, clean out the rest of the line */
	   ndiscard = 0;
	   while((c = getchar()) != '\n')  {
		buf[ndiscard] = c;
		ndiscard++;
	   }
	   if(ndiscard > 0) {
		buf[ndiscard] = '\0';  /* cap the string */       
		printf("Rest of input (%s) has been discarded.\n",buf);
	   }
	   printf("%s",errprompt);
	   printf("Try again:");
	   scanf("%s",s);
	}
    }
    return( (int) success);
}    


double bab_to_dec()
struct coord bab;
	 /* converts a "babylonian" (sexigesimal) structure into
      double-precision floating point ("decimal") number. */
	 {
   double x;
   x = bab.sign * (bab.hh + bab.mm / 60. + bab.ss / 3600.);
   return(x);
	 }

void dec_to_bab (double deci,struct coord *bab)
	 /* function for converting decimal to babylonian hh mm ss.ss */

{
   int hr_int, min_int;

   if (deci >= 0.) bab->sign = 1; 
   else {
      bab->sign = -1;
      deci = -1. * deci;
   }
   hr_int = deci;   /* use conversion conventions to truncate */
   bab->hh = hr_int;
   min_int = 60. * (deci - bab->hh);
   bab->mm = min_int;
   bab->ss = 3600. * (deci - bab->hh - bab->mm / 60.);
}

short get_line(s)
              
	char *s;   	

/* gets a line terminated by end-of-line and returns number of characters. */
{       
	char c;
	short i = 0;

	c = getchar(); /* get the first character */
	/* chew through until you hit non white space */
	while((c == '\n') || (c == ' ') || (c == '\t')) c = getchar();

	s[i]=c;
	i++;

	/* keep going til the next newline */
	while((c=getchar()) != '\n') {
		s[i]=c;
		i++;
	}
	s[i]='\0';  /* terminate with null */
	return(i);
}

double get_coord()

/* Reads a string from the terminal and converts it into
   a double-precision coordinate.  This is trickier than 
   it appeared at first, since a -00 tests as non-negative; 
   the sign has to be picked out and handled explicitly. */
/* Prompt for input in the calling routine.*/
{
   short sign;
   double hrs, mins, secs;
   char hh_string[6];  /* string with the first coord (hh) */
   char hh1[1];
	//char errprompt[80];
   short i = 0;
   int end_in = 0;

   /* read and handle the hour (or degree) part with sign */

   scanf("%s",hh_string);
   hh1[0] = hh_string[i];

   while(hh1[0] == ' ') {
       /* discard leading blanks */
       i++;
       hh1[0] = hh_string[i];
   }

   if(hh1[0] == '-') sign = -1;

     else sign = 1;

   if((end_in = parsedouble(hh_string,&hrs)) < 0) {
	printf("Didn't parse correctly -- set parameter to zero!!\n");
	return(0.);
   }

   if(sign == -1) hrs = -1. * hrs;

   /* read in the minutes and seconds normally */           
   if(end_in == 0)      
       	end_in = getdouble(&mins,0.,60.,
	  "Give minutes again, then seconds; no further prompts.\n");
   else return(sign * hrs);
   if(end_in == 0) end_in = getdouble(&secs,0.,60.,
     "Give seconds again, no further prompts.\n");
   else if(end_in == 1) secs = 0.;
   return(sign * (hrs + mins / 60. + secs / 3600.));
}

	
void put_coords(deci, precision)
               
	double deci;
	short precision;
               
/* prints out a struct coord in a nice format; precision
   is a code for how accurate you want it.  The options are:
     precision = 0;   minutes rounded to the nearest minute
     precision = 1;   minutes rounded to the nearest tenth.
     precision = 2;   seconds rounded to the nearest second
     precision = 3;   seconds given to the tenth
     precision = 4;   seconds given to the hundredth
   The program assumes that the line is ready for the coord
   to be printed and does NOT deliver a new line at the end
   of the output. */
 
{
   
   double minutes;  /* for rounding off if necess. */
   struct coord out_coord, coords;
   char out_string[20];  /* for checking for nasty 60's */

   dec_to_bab(deci,&coords);  /* internally convert to coords*/

   if(precision == 0) {   /* round to nearest minute */
      minutes = coords.mm + coords.ss / 60.;
			/* check to be sure minutes aren't 60 */
      sprintf(out_string,"%.0f %02.0f",coords.hh,minutes);
      sscanf(out_string,"%lf %lf",&out_coord.hh,&out_coord.mm);
      if(fabs(out_coord.mm - 60.) < 1.0e-7) {
	 out_coord.mm = 0.;
	 out_coord.hh = out_coord.hh + 1.;
      }
      if(out_coord.hh < 100.) oprntf(" ");  /* put in leading blanks explicitly
	  for 'h' option below. */
      if(out_coord.hh < 10.) oprntf(" ");
      if(coords.sign == -1) oprntf("-");
	else oprntf(" ");   /* preserves alignment */
      oprntf("%.0f %02.0f",out_coord.hh,out_coord.mm);
   }

   else if(precision == 1) {    /* keep nearest tenth of a minute */
      minutes = coords.mm + coords.ss / 60.;
	   /* check to be sure minutes are not 60 */
      sprintf(out_string,"%.0f %04.1f",coords.hh,minutes);
      sscanf(out_string,"%lf %lf",&out_coord.hh, &out_coord.mm);
      if(fabs(out_coord.mm - 60.) < 1.0e-7) {
	 out_coord.mm = 0.;
	 out_coord.hh = out_coord.hh + 1.;
      }
      if(out_coord.hh < 10.) oprntf(" ");
      if(coords.sign == -1) oprntf("-");
	else oprntf(" ");   /* preserves alignment */
      oprntf("%.0f %04.1f", out_coord.hh, out_coord.mm);
   }
   else if(precision == 2) {
	  /* check to be sure seconds are not 60 */
      sprintf(out_string,"%.0f %02.0f %02.0f",coords.hh,coords.mm,coords.ss);
      sscanf(out_string,"%lf %lf %lf",&out_coord.hh,&out_coord.mm,
	   &out_coord.ss);
      if(fabs(out_coord.ss - 60.) < 1.0e-7) {
	  out_coord.mm = out_coord.mm + 1.;
	  out_coord.ss = 0.;
	  if(fabs(out_coord.mm - 60.) < 1.0e-7) {
	      out_coord.hh = out_coord.hh + 1.;
	      out_coord.mm = 0.;
	  }
      }
      if(out_coord.hh < 10.) oprntf(" ");
      if(coords.sign == -1) oprntf("-");
	 else oprntf(" ");   /* preserves alignment */
      oprntf("%.0f %02.0f %02.0f",out_coord.hh,out_coord.mm,out_coord.ss);
   }
   else if(precision == 3) {
	  /* the usual shuffle to check for 60's */
      sprintf(out_string,"%.0f %02.0f %04.1f",coords.hh, coords.mm, coords.ss);
      sscanf(out_string,"%lf %lf %lf",&out_coord.hh,&out_coord.mm,
	   &out_coord.ss);
      if(fabs(out_coord.ss - 60.) < 1.0e-7) {
	  out_coord.mm = out_coord.mm + 1.;
	  out_coord.ss = 0.;
	  if(fabs(out_coord.mm - 60.) < 1.0e-7) {
	     out_coord.hh = out_coord.hh + 1.;
	     out_coord.mm = 0.;
	  }
      }
      if(out_coord.hh < 10.) oprntf(" ");
      if(coords.sign == -1) oprntf("-");
	 else oprntf(" ");   /* preserves alignment */
      oprntf("%.0f %02.0f %04.1f",out_coord.hh,out_coord.mm,out_coord.ss);
   }
   else {
      sprintf(out_string,"%.0f %02.0f %05.2f",coords.hh,coords.mm,coords.ss);
      sscanf(out_string,"%lf %lf %lf",&out_coord.hh,&out_coord.mm,
	   &out_coord.ss);
      if(fabs(out_coord.ss - 60.) < 1.0e-6) {
	 out_coord.mm = out_coord.mm + 1.;
	 out_coord.ss = 0.;
	 if(fabs(out_coord.mm - 60.) < 1.0e-6) {
	    out_coord.hh = out_coord.hh + 1.;
	    out_coord.mm = 0.;
	 }
      }
      if(out_coord.hh < 10.) oprntf(" ");
      if(coords.sign == -1) oprntf("-");
	 else oprntf(" ");   /* preserves alignment */
      oprntf("%.0f %02.0f %05.2f",out_coord.hh, out_coord.mm, out_coord.ss);
   }
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
               
/* sets the site-specific quantities; these are
		longit     = W longitude in decimal hours
		lat        = N latitude in decimal degrees
		stdz       = standard time zone offset, hours
		elevsea    = elevation above sea level (for absolute location)
		elev       = observatory elevation above horizon, meters
		horiz      = (derived) added zenith distance for rise/set due
				to elevation
		use_dst    = 0 don't use it
			     1 use USA convention
			     2 use Spanish convention
			     < 0 Southern hemisphere (reserved, unimplimented)
		zone_name  = name of time zone, e. g. Eastern
		zabr       = single-character abbreviation of time zone
		site_name  = name of site.  */

{
	short nch;
	char obs_code[3];  /* need only one char, but why not? */
	char errprompt[50];

	printf("*SELECT SITE* - Enter single-character code:\n");
	printf("   n .. NEW SITE, prompts for all parameters.\n");
	printf("   x .. exit without change (current: %s)\n",site_name);
	printf("   k .. Kitt Peak [MDM Obs.]\n");
	printf("   s .. Shattuck Observatory, Dartmouth College, Hanover NH\n");
	printf("   e .. European Southern Obs, La Silla\n");
	printf("   a .. Anglo-Australian Telelescope, Siding Spring\n");
	printf("   h .. Mt. Hopkins, AZ (MMT, FLWO)\n");
	printf("   p .. Palomar Observatory\n");
	printf("   t .. Cerro Tololo \n");
	printf("   r .. Roque de los Muchachos, La Palma, Canary Is.\n");
	printf("   b .. Black Moshannon Obs., Penn State U.\n");
	printf("   d .. Dominion Astrophysical Obs., Victoria, BC\n");
	printf("   o .. McDonald Observatory, Mt. Locke, Texas\n");
	printf("   m .. Mauna Kea, Hawaii\n");
	printf("   l .. Lick Observatory\n");
	printf("Your answer --> ");
	scanf("%s",obs_code);
	if(obs_code[0] == 'x') {
		printf("No action taken.  Current site = %s.\n",site_name);
		return;
	}
	if(obs_code[0] == 'k') {
		strcpy(site_name,"Kitt Peak [MDM Obs.]");
		strcpy(zone_name, "Mountain");
		*zabr = 'M';
		*use_dst = 0;
		*longit = 7.44111; /* decimal hours */
		*lat = 31.9533;    /* decimal degrees */
		*stdz = 7.;
		*elevsea = 1925.;  /* for MDM observatory, strictly */ 
		*elev = 700.;  /* approximate -- to match KPNO tables */
	}
	else if (obs_code[0] == 's') {
		strcpy(site_name, "Shattuck Observatory");
		strcpy(zone_name,"Eastern");
		*zabr = 'E';
		*use_dst = 1;
		*longit = 4.81889;
		*lat = 43.7033;
		*stdz = 5.;
		*elevsea = 183.;
		*elev = 0.;  /* below surrouding horizon */
	}
	else if (obs_code[0] == 'e') {
		strcpy(site_name, "ESO, Cerro La Silla");
		strcpy(zone_name, "Chilean");
		*zabr = 'C';
		*use_dst = -1;
		*longit = 4.7153;
		*lat = -29.257;
		*stdz = 4.;
		*elevsea = 2347.; 
		*elev = 2347.; /* for ocean horizon, not Andes! */
		printf("\n\n** Will use daylght time, Chilean date conventions. \n\n");
	}
	else if (obs_code[0] == 'p') {
		strcpy(site_name, "Palomar Observatory");
		strcpy(zone_name, "Pacific");
		*zabr = 'P';
		*use_dst = 1;
		*longit = 7.79089;
		*lat = 33.35667;
		*elevsea = 1706.;
		*elev = 1706.;  /* not clear if it's appropriate ... */
		*stdz = 8.;
	}
	else if (obs_code[0] == 't') {
		strcpy(site_name, "Cerro Tololo");
		strcpy(zone_name, "Chilean");
		*zabr = 'C';
		*use_dst = -1;
		*longit = 4.721;
		*lat = -30.165;
		*stdz = 4.;
		*elevsea = 2215.;
		*elev = 2215.; /* for ocean horizon, not Andes! */
		printf("\n\n** Will use daylght time, Chilean date conventions. \n\n");
	}
	else if (obs_code[0] == 'h') {
		strcpy(site_name, "Mount Hopkins, Arizona");
		strcpy(zone_name, "Mountain");
		*zabr = 'M';
		*use_dst = 0;
		*longit = 7.39233;
		*lat = 31.6883;
		*elevsea = 2608.;
		*elev = 500.;  /* approximate elevation above horizon mtns */
		*stdz = 7.;
	}
/*      else if (obs_code[0] == 'c') {
		strcpy(site_name,"Harvard College Observatory");
		strcpy(zone_name,"Eastern");
		*zabr = 'E';
		*use_dst = 1;
		*longit = 4.742;
		*lat = 42.38;
		*elevsea = 0.;  /* small, anyhow *?
		*elev = 0.;
		*stdz = 5.;
	}                                      --- COMMENTED OUT */
        else if (obs_code[0] == 'o') {
		strcpy(site_name,"McDonald Observatory");
		strcpy(zone_name,"Central");
		*zabr = 'C';
		*use_dst = 1;
		*longit = 6.93478;
                *lat = 30.6717;
                *elevsea = 2075;
                *elev = 1000.;  /* who knows? */
                *stdz = 6.;
        }
	else if (obs_code[0] == 'a') {
		strcpy(site_name, "Anglo-Australian Tel., Siding Spring");
		strcpy(zone_name, "Australian");
		*zabr = 'A';
		*use_dst = -2;
		*longit = -9.937739;
		*lat = -31.277039;
		*elevsea = 1149.;
		*elev = 670.;
		*stdz = -10.;
	}
	else if (obs_code[0] == 'b') {
		strcpy(site_name, "Black Moshannon Observatory");
		strcpy(zone_name, "Eastern");
		*zabr = 'E';
		*use_dst = 1;
		*longit = 5.20033;
		*lat = 40.92167;
		*elevsea = 738.;
		*elev = 0.;  /* not set */
		*stdz = 5.;
	}
	else if (obs_code[0] == 'd') {
		strcpy(site_name, "DAO, Victoria, BC");
		strcpy(zone_name, "Pacific");
		*zabr = 'P';
		*use_dst = 1;
		printf("\n\nWARNING: United States conventions for DST assumed.\n\n");
		*longit = 8.22778;
		*lat = 48.52;
		*elevsea = 74.;
		*elev = 74.;  /* not that it makes much difference */
		*stdz = 8.;             
	}
	else if (obs_code[0] == 'm') {
		strcpy(site_name, "Mauna Kea, Hawaii");
		strcpy(zone_name, "Hawaiian");
		*zabr = 'H';
		*use_dst = 0;
		*longit = 10.36478;
		*lat = 19.8267;
		*elevsea = 4215.;
		*elev = 4215.;  /* yow! */
		*stdz = 10.;
	}
	else if (obs_code[0] == 'l') {
		strcpy(site_name, "Lick Observatory");
		strcpy(zone_name, "Pacific");
		*zabr = 'P';
		*use_dst = 1;
		*longit = 8.10911;
		*lat = 37.3433;
		*elevsea = 1290.;
		*elev = 1290.; /* for those nice Pacific sunsets */
		*stdz = 8.;
	}
	else if (obs_code[0] == 'r') {
		strcpy(site_name, "Roque de los Muchachos");
		strcpy(zone_name, "pseudo-Greenwich");
		*zabr = 'G';
		*use_dst = 2;
		*longit = 1.192;
		*lat = 28.75833;
		*elevsea = 2326.;
		*elev = 2326.;
		*stdz = 0.;
	}
	else if (obs_code[0] == 'n') {
		printf("Enter new site parameters; the prompts give current values.\n");
		printf("(Even if current value is correct you must re-enter explicitly.)\n");
		printf("WEST longitude, (HOURS min sec); current value ");
		put_coords(*longit,3);
		printf(": ");
		*longit = get_coord();
		printf("Latitude, (d m s); current value ");
		put_coords(*lat,2);
		printf(": ");
		*lat = get_coord();
		printf("Actual elevation (meters) above sea level,");
		printf(" currently %5.0f:",*elevsea);
		strcpy(errprompt,"Because of error,");
		    /* situation is uncomplicated, so simple errprompt */
		getdouble(elevsea,-1000.,100000.,errprompt);
		printf("Effective elevation, meters (for rise/set),");
		printf(" currently %5.0f:",*elev); 
		getdouble(elev,-1000.,20000.,errprompt); /* limits of approx. ... */
		printf("Site name (< 30 char): ");
		nch=get_line(site_name);
		printf("Std time zone, hours W; currently %3.0f :",*stdz);
		getdouble(stdz,-13.,13.,errprompt);
		printf("Time zone name, e. g., Central: ");
		nch = get_line(zone_name);
		printf("Single-character zone abbreviation, currently %c : ",*zabr);
		scanf("%c",zabr);
		printf("Type daylight savings time option --- \n");
		printf("   0  ... don't use it, \n");
		printf("   1  ... use United States convention for clock change.\n");
		printf("   2  ... use Spanish (Continental?) convention.\n");
		printf("  -1  ... use Chilean convention.\n");
		printf("  -2  ... use Australian convention.\n");
		printf("(Other options would require new code). Answer: --> ");
		getshort(use_dst,-100,100,errprompt);
	}
		else {
		printf("UNKNOWN SITE '%c' -- left as %s. Note input is case-sensitive.\n",
			      obs_code[0],site_name);
	} 
	/* now compute derived quantity "horiz" = depression of horizon.*/
	*horiz = sqrt(2. * *elev / 6378140.) * DEG_IN_RADIAN;   
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

int get_pm(dec, mura, mudec) 

double dec, *mura, *mudec;

{
	/* This gets the proper motions.  New routine
	(2/94) assumes that primary PM convention will
	be rate of change of right ascension in seconds
	of time per year.  Either one can be entered here,
	but the value passed out is seconds of time per year at the
        equator (i.e., rate of change of RA itself). */

	char pmtype[3];
        int status;

	printf("Note -- two different conventions for RA proper motion.\n");
	printf("Enter RA p.m., either as delta RA(sec) or arcsec per yr.:");
	scanf("%lf",mura);
        if(*mura != 0.) {
	  printf("Type s if this is RA change in time sec per yr,\n");
	  printf("or a if this is motion in arcsec per yr:");
	  scanf("%s",pmtype);
        }
	else pmtype[0] = 's';  /* if pm is zero, it doesn't matter. */

	if(pmtype[0] == 's') status = 0;
	else if (pmtype[0] == 'a') {
		*mura = *mura /(15. * cos(dec / DEG_IN_RADIAN));
		printf("Equivalent to %8.5f sec of time per yr. at current dec.\n",
		     *mura);
                               printf("(Will only be correct at this dec.)\n");
                status = 0;
	}
	else {
		printf("UNRECOGNIZED PM TYPE ... defaults to sec/yr\n");
		status = -1;
        }
	printf("Give declination PM in arcsec/yr:");
	scanf("%lf",mudec);
	printf("(Note: Proper motion correction only ");
	printf("by mu * delta t; inaccurate near pole.)\n");
	printf("Don't forget to reset for new object.\n");

        return(status);  /* 0 = success */
}


int get_date(date) 

	struct date_time *date;
{	
	
	int valid_date = 0;

	while(valid_date == 0) {        
	 	getshort(&(date->y),-10,2100,      
           	   "Give year again, then month and day.\n");
		if(date->y <= 0) return(-1);

	/* scan for mo and day here, *then* error check. */
		getshort(&(date->mo),1,12,
		   "Give month again (as number 1-12), then day.\n");
		getshort(&(date->d),0,32,"Give day again.\n");
		/* a lot of this error checking is redundant with the
		    checks in the new getshort routine.... */
		if(date->y < 100)  {
			date->y = date->y + 1900;
			printf("(Your answer assumed to mean %d)\n",date->y);
		}
		else if((date->y <= 1900 ) | (date->y >= 2100)){
			printf("Date out of range: only 1901 -> 2099 allowed.\n");
			printf("Try it again!\n");
		}
		/* might be nice to allow weird input dates, but calendrical
		   routines will not necessarily handle these right ... */
		else if((date->d < 0) || (date->d > 32)) 
			printf("day-of-month %d not allowed -- try again!\n",
				date->d);
		else if((date->mo < 1) || (date->mo > 12)) 
			printf("month %d doesn't exist -- try again!\n",
				date->mo);
		else {
			valid_date = 1;
			return(0);  /* success */
		}
	}
}

int get_time(date) 

	struct date_time *date;

{
	
	struct coord ttime;
	
	dec_to_bab(get_coord(),&ttime);
	date->h = (short) ttime.hh;
		/* awkward, because h and m of date are short. */
	date->mn = (short) ttime.mm;
	date->s = ttime.ss;
	return(0);
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


void print_day(d)
	short d;

{
	/* prints day of week given number 0=Mon,6=Sun */
	char *days = "MonTueWedThuFriSatSun";
	char day_out[4];

	day_out[0] = *(days+3*d);
	day_out[1] = *(days+3*d+1);
	day_out[2] = *(days+3*d+2);
	day_out[3] = '\0';  /* terminate with null char */
	
	oprntf("%s",day_out);
}


void print_all(jdin)

	double jdin;
{
	/* given a julian date,
	prints a year, month, day, hour, minute, second */

	struct date_time date;
        int ytemp, dtemp; /* compiler bug workaround ... SUN
         and silicon graphics */
	char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
	char mo_out[4];
	double out_time;
	short dow;

	caldat(jdin,&date,&dow);

	print_day(dow);
	oprntf(", ");

	mo_out[0] = *(months + 3*(date.mo - 1));
	mo_out[1] = *(months + 3*(date.mo - 1) + 1);
	mo_out[2] = *(months + 3*(date.mo - 1) + 2);
	mo_out[3] = '\0';

	/* going through the rigamarole to avoid 60's */

	out_time = date.h + date.mn / 60. + date.s / 3600.;

	ytemp = (int) date.y;
	dtemp = (int) date.d;
	oprntf("%d %s %2d, time ",
		ytemp,mo_out,dtemp);
	put_coords(out_time,3);
}

void print_current(date,night_date,enter_ut) 
        struct date_time date;
	short night_date, enter_ut;
{
	/* just prints out the date & time and a little statement
           of whether time is "local" or "ut".  Functionalized to 
           compactify some later code. */
	
	double jd;

	jd = date_to_jd(date);
        if((night_date == 1) && (date.h < 12)) jd = jd + 1.0;
	print_all(jd);
	if(enter_ut == 0) oprntf(" local time.");
	else oprntf(" Universal time.");
}

void print_calendar(jdin,dow)
                   
	double jdin;
	short *dow;
                   
{
	/* given a julian date prints a year, month, day.
	   Returns day of week (0 - 6) for optional printing ---
	   idea is to do one roundoff only to eliminate possibility
	   of day/date disagreement. */
	
	struct date_time date;
	char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
	char mo_out[4]; 
	int ytemp, dtemp;  /* compiler bug workaround -- SUN and
           Silicon Graphics machines */

	caldat(jdin,&date,dow); 
	mo_out[0] = *(months + 3*(date.mo - 1));
	mo_out[1] = *(months + 3*(date.mo - 1) + 1);
	mo_out[2] = *(months + 3*(date.mo - 1) + 2);
	mo_out[3] = '\0';
        ytemp = (int) date.y;
        dtemp = (int) date.d;
	oprntf("%d %s %d",ytemp,mo_out,dtemp);
}

void print_time(jdin,prec)

	double jdin;
	short prec;

{
	/* given a julian day, prints time only; 
	   special precision of "-1" prints only hours!  */
	struct date_time date;
	double temptime;
	short dow;
	int temp; /* to get around a Sun compiler bug ... */
	
	caldat(jdin,&date,&dow);
	temp = date.h;  /* cast from short to int */

	temptime = date.h + date.mn/60. + date.s/3600.;

	if(prec >= 0) put_coords(temptime,prec);
	else if(date.mn < 30) oprntf("%2.0d hr",temp);
	else oprntf("%2.0d hr",(temp+1)); /* round it up */
}       

double frac_part(x) 

	double x;
{
	long i;
	i = x;
	x = x - i;
	return(x);
}


double lst(jd,longit)

	double jd,longit; 

{
	/* returns the local MEAN sidereal time (dec hrs) at julian date jd
		at west longitude long (decimal hours).  Follows
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

double adj_time(x)
	double x;

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
	else oprntf("Out of bounds in adj_time!\n");
	return(x);
}

void lpmoon(jd,lat,sid,ra,dec,dist)  

	double jd,lat,sid,*ra,*dec,*dist;  

/* implements "low precision" moon algorithms from
   Astronomical Almanac (p. D46 in 1992 version).  Does
   apply the topocentric correction. 
Units are as follows
jd,lat, sid;   decimal hours 
*ra, *dec,   decimal hours, degrees 
	*dist;      earth radii */
{

	double T, lambda, beta, pie, l, m, n, x, y, z, alpha, delta,
		rad_lat, rad_lst, distance, topo_dist;
	char dummy[40];  /* to fix compiler bug on IBM system */

	T = (jd - J2000) / 36525.;  /* jul cent. since J2000.0 */

	lambda = 218.32 + 481267.883 * T 
	   + 6.29 * sin((134.9 + 477198.85 * T) / DEG_IN_RADIAN)
	   - 1.27 * sin((259.2 - 413335.38 * T) / DEG_IN_RADIAN)
	   + 0.66 * sin((235.7 + 890534.23 * T) / DEG_IN_RADIAN)
	   + 0.21 * sin((269.9 + 954397.70 * T) / DEG_IN_RADIAN)
	   - 0.19 * sin((357.5 + 35999.05 * T) / DEG_IN_RADIAN)
	   - 0.11 * sin((186.6 + 966404.05 * T) / DEG_IN_RADIAN);
	lambda = lambda / DEG_IN_RADIAN;
	beta = 5.13 * sin((93.3 + 483202.03 * T) / DEG_IN_RADIAN)
	   + 0.28 * sin((228.2 + 960400.87 * T) / DEG_IN_RADIAN)
	   - 0.28 * sin((318.3 + 6003.18 * T) / DEG_IN_RADIAN)
	   - 0.17 * sin((217.6 - 407332.20 * T) / DEG_IN_RADIAN);
	beta = beta / DEG_IN_RADIAN;
	pie = 0.9508 
	   + 0.0518 * cos((134.9 + 477198.85 * T) / DEG_IN_RADIAN)
	   + 0.0095 * cos((259.2 - 413335.38 * T) / DEG_IN_RADIAN)
	   + 0.0078 * cos((235.7 + 890534.23 * T) / DEG_IN_RADIAN)
	   + 0.0028 * cos((269.9 + 954397.70 * T) / DEG_IN_RADIAN);
	pie = pie / DEG_IN_RADIAN;
	distance = 1 / sin(pie);

	l = cos(beta) * cos(lambda);
	m = 0.9175 * cos(beta) * sin(lambda) - 0.3978 * sin(beta);
	n = 0.3978 * cos(beta) * sin(lambda) + 0.9175 * sin(beta);

	x = l * distance; 
	y = m * distance; 
	z = n * distance;  /* for topocentric correction */
	/* lat isn't passed right on some IBM systems unless you do this
	   or something like it! */
	sprintf(dummy,"%f",lat);
	rad_lat = lat / DEG_IN_RADIAN;
	rad_lst = sid / HRS_IN_RADIAN;
	x = x - cos(rad_lat) * cos(rad_lst);
	y = y - cos(rad_lat) * sin(rad_lst);
	z = z - sin(rad_lat);


	topo_dist = sqrt(x * x + y * y + z * z);

	l = x / topo_dist; 
	m = y / topo_dist; 
	n = z / topo_dist;

	alpha = atan_circ(l,m);
	delta = asin(n);
	*ra = alpha * HRS_IN_RADIAN;
	*dec = delta * DEG_IN_RADIAN;
	*dist = topo_dist;
}


void lpsun(jd,ra,dec)

	double jd, *ra, *dec;

/* Low precision formulae for the sun, from Almanac p. C24 (1990) */
/* ra and dec are returned as decimal hours and decimal degrees. */

{
	double n, L, g, lambda,epsilon,alpha,delta,x,y,z;

	n = jd - J2000;
	L = 280.460 + 0.9856474 * n;
	g = (357.528 + 0.9856003 * n)/DEG_IN_RADIAN;
	lambda = (L + 1.915 * sin(g) + 0.020 * sin(2. * g))/DEG_IN_RADIAN;
	epsilon = (23.439 - 0.0000004 * n)/DEG_IN_RADIAN;

	x = cos(lambda); 
	y = cos(epsilon) * sin(lambda); 
	z = sin(epsilon)*sin(lambda);

	*ra = (atan_circ(x,y))*HRS_IN_RADIAN;
	*dec = (asin(z))*DEG_IN_RADIAN;
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
		oprntf("etcorr ... no ephemeris time data for < 1900.\n");
       		delt = 0.;
	}

	else if (year >= 2100.) {
		oprntf("etcorr .. very long extrapolation in delta T - inaccurate.\n");
		delt = 180.; /* who knows? */
	} 

	return(delt);
}


void accumoon(jd,geolat,lst,elevsea,geora,geodec,geodist,
     topora,topodec,topodist)
             
	double jd,geolat,lst,elevsea;
     	double *geora,*geodec,*geodist,*topora,*topodec,*topodist;
             
  /* jd, dec. degr., dec. hrs., meters */
/* More accurate (but more elaborate and slower) lunar 
   ephemeris, from Jean Meeus' *Astronomical Formulae For Calculators*,
   pub. Willman-Bell.  Includes all the terms given there. */

{       
/*      double *eclatit,*eclongit, *pie,*ra,*dec,*dist; geocent quantities,
		formerly handed out but not in this version */
	double pie, dist;  /* horiz parallax */
	double Lpr,M,Mpr,D,F,Om,T,Tsq,Tcb;
	double e,lambda,B,beta,om1,om2;
	double sinx, x, y, z, l, m, n;
	double x_geo, y_geo, z_geo;  /* geocentric position of *observer* */    

	jd = jd + etcorr(jd)/SEC_IN_DAY;   /* approximate correction to ephemeris time */
	T = (jd - 2415020.) / 36525.;   /* this based around 1900 ... */
	Tsq = T * T;
	Tcb = Tsq * T;

	Lpr = 270.434164 + 481267.8831 * T - 0.001133 * Tsq 
			+ 0.0000019 * Tcb;
	M = 358.475833 + 35999.0498*T - 0.000150*Tsq
			- 0.0000033*Tcb;
	Mpr = 296.104608 + 477198.8491*T + 0.009192*Tsq 
			+ 0.0000144*Tcb;
	D = 350.737486 + 445267.1142*T - 0.001436 * Tsq
			+ 0.0000019*Tcb;
	F = 11.250889 + 483202.0251*T -0.003211 * Tsq 
			- 0.0000003*Tcb;
	Om = 259.183275 - 1934.1420*T + 0.002078*Tsq 
			+ 0.0000022*Tcb;

	Lpr = circulo(Lpr);
	Mpr = circulo(Mpr);     
	M = circulo(M);
	D = circulo(D);
	F = circulo(F);
	Om = circulo(Om);

	
	sinx =  sin((51.2 + 20.2 * T)/DEG_IN_RADIAN);
	Lpr = Lpr + 0.000233 * sinx;
	M = M - 0.001778 * sinx;
	Mpr = Mpr + 0.000817 * sinx;
	D = D + 0.002011 * sinx;
	
	sinx = 0.003964 * sin((346.560+132.870*T -0.0091731*Tsq)/DEG_IN_RADIAN);

	Lpr = Lpr + sinx;
	Mpr = Mpr + sinx;
	D = D + sinx;
	F = F + sinx;

	sinx = sin(Om/DEG_IN_RADIAN);
	Lpr = Lpr + 0.001964 * sinx;
	Mpr = Mpr + 0.002541 * sinx;
	D = D + 0.001964 * sinx;
	F = F - 0.024691 * sinx;
	F = F - 0.004328 * sin((Om + 275.05 -2.30*T)/DEG_IN_RADIAN);

	e = 1 - 0.002495 * T - 0.00000752 * Tsq;

	M = M / DEG_IN_RADIAN;   /* these will all be arguments ... */
	Mpr = Mpr / DEG_IN_RADIAN;
	D = D / DEG_IN_RADIAN;
	F = F / DEG_IN_RADIAN;

	lambda = Lpr + 6.288750 * sin(Mpr)
		+ 1.274018 * sin(2*D - Mpr)
		+ 0.658309 * sin(2*D)
		+ 0.213616 * sin(2*Mpr)
		- e * 0.185596 * sin(M) 
		- 0.114336 * sin(2*F)
		+ 0.058793 * sin(2*D - 2*Mpr)
		+ e * 0.057212 * sin(2*D - M - Mpr)
		+ 0.053320 * sin(2*D + Mpr)
		+ e * 0.045874 * sin(2*D - M)
		+ e * 0.041024 * sin(Mpr - M)
		- 0.034718 * sin(D)
		- e * 0.030465 * sin(M+Mpr)
		+ 0.015326 * sin(2*D - 2*F)
		- 0.012528 * sin(2*F + Mpr)
		- 0.010980 * sin(2*F - Mpr)
		+ 0.010674 * sin(4*D - Mpr)
		+ 0.010034 * sin(3*Mpr)
		+ 0.008548 * sin(4*D - 2*Mpr)
		- e * 0.007910 * sin(M - Mpr + 2*D)
		- e * 0.006783 * sin(2*D + M)
		+ 0.005162 * sin(Mpr - D);

		/* And furthermore.....*/

	lambda = lambda + e * 0.005000 * sin(M + D)
		+ e * 0.004049 * sin(Mpr - M + 2*D)
		+ 0.003996 * sin(2*Mpr + 2*D)
		+ 0.003862 * sin(4*D)
		+ 0.003665 * sin(2*D - 3*Mpr)
		+ e * 0.002695 * sin(2*Mpr - M)
		+ 0.002602 * sin(Mpr - 2*F - 2*D)
		+ e * 0.002396 * sin(2*D - M - 2*Mpr)
		- 0.002349 * sin(Mpr + D)
		+ e * e * 0.002249 * sin(2*D - 2*M)
		- e * 0.002125 * sin(2*Mpr + M)
		- e * e * 0.002079 * sin(2*M)
		+ e * e * 0.002059 * sin(2*D - Mpr - 2*M)
		- 0.001773 * sin(Mpr + 2*D - 2*F)
		- 0.001595 * sin(2*F + 2*D)
		+ e * 0.001220 * sin(4*D - M - Mpr)
		- 0.001110 * sin(2*Mpr + 2*F)
		+ 0.000892 * sin(Mpr - 3*D)
		- e * 0.000811 * sin(M + Mpr + 2*D)
		+ e * 0.000761 * sin(4*D - M - 2*Mpr)
		+ e * e * 0.000717 * sin(Mpr - 2*M)
		+ e * e * 0.000704 * sin(Mpr - 2 * M - 2*D)
		+ e * 0.000693 * sin(M - 2*Mpr + 2*D)
		+ e * 0.000598 * sin(2*D - M - 2*F)
		+ 0.000550 * sin(Mpr + 4*D)
		+ 0.000538 * sin(4*Mpr)
		+ e * 0.000521 * sin(4*D - M)
		+ 0.000486 * sin(2*Mpr - D);
	
/*              *eclongit = lambda;  */

	B = 5.128189 * sin(F)
		+ 0.280606 * sin(Mpr + F)
		+ 0.277693 * sin(Mpr - F)
		+ 0.173238 * sin(2*D - F)
		+ 0.055413 * sin(2*D + F - Mpr)
		+ 0.046272 * sin(2*D - F - Mpr)
		+ 0.032573 * sin(2*D + F)
		+ 0.017198 * sin(2*Mpr + F)
		+ 0.009267 * sin(2*D + Mpr - F)
		+ 0.008823 * sin(2*Mpr - F)
		+ e * 0.008247 * sin(2*D - M - F) 
		+ 0.004323 * sin(2*D - F - 2*Mpr)
		+ 0.004200 * sin(2*D + F + Mpr)
		+ e * 0.003372 * sin(F - M - 2*D)
		+ 0.002472 * sin(2*D + F - M - Mpr)
		+ e * 0.002222 * sin(2*D + F - M)
		+ e * 0.002072 * sin(2*D - F - M - Mpr)
		+ e * 0.001877 * sin(F - M + Mpr)
		+ 0.001828 * sin(4*D - F - Mpr)
		- e * 0.001803 * sin(F + M)
		- 0.001750 * sin(3*F)
		+ e * 0.001570 * sin(Mpr - M - F)
		- 0.001487 * sin(F + D)
		- e * 0.001481 * sin(F + M + Mpr)
		+ e * 0.001417 * sin(F - M - Mpr)
		+ e * 0.001350 * sin(F - M)
		+ 0.001330 * sin(F - D)
		+ 0.001106 * sin(F + 3*Mpr)
		+ 0.001020 * sin(4*D - F)
		+ 0.000833 * sin(F + 4*D - Mpr);
     /* not only that, but */
	B = B + 0.000781 * sin(Mpr - 3*F)
		+ 0.000670 * sin(F + 4*D - 2*Mpr)
		+ 0.000606 * sin(2*D - 3*F)
		+ 0.000597 * sin(2*D + 2*Mpr - F)
		+ e * 0.000492 * sin(2*D + Mpr - M - F)
		+ 0.000450 * sin(2*Mpr - F - 2*D)
		+ 0.000439 * sin(3*Mpr - F)
		+ 0.000423 * sin(F + 2*D + 2*Mpr)
		+ 0.000422 * sin(2*D - F - 3*Mpr)
		- e * 0.000367 * sin(M + F + 2*D - Mpr)
		- e * 0.000353 * sin(M + F + 2*D)
		+ 0.000331 * sin(F + 4*D)
		+ e * 0.000317 * sin(2*D + F - M + Mpr)
		+ e * e * 0.000306 * sin(2*D - 2*M - F)
		- 0.000283 * sin(Mpr + 3*F);
	
	om1 = 0.0004664 * cos(Om/DEG_IN_RADIAN);        
	om2 = 0.0000754 * cos((Om + 275.05 - 2.30*T)/DEG_IN_RADIAN);
	
	beta = B * (1. - om1 - om2);
/*      *eclatit = beta; */
	
	pie = 0.950724 
		+ 0.051818 * cos(Mpr)
		+ 0.009531 * cos(2*D - Mpr)
		+ 0.007843 * cos(2*D)
		+ 0.002824 * cos(2*Mpr)
		+ 0.000857 * cos(2*D + Mpr)
		+ e * 0.000533 * cos(2*D - M)
		+ e * 0.000401 * cos(2*D - M - Mpr)
		+ e * 0.000320 * cos(Mpr - M)
		- 0.000271 * cos(D)
		- e * 0.000264 * cos(M + Mpr)
		- 0.000198 * cos(2*F - Mpr)
		+ 0.000173 * cos(3*Mpr)
		+ 0.000167 * cos(4*D - Mpr)
		- e * 0.000111 * cos(M)
		+ 0.000103 * cos(4*D - 2*Mpr)
		- 0.000084 * cos(2*Mpr - 2*D)
		- e * 0.000083 * cos(2*D + M)
		+ 0.000079 * cos(2*D + 2*Mpr)
		+ 0.000072 * cos(4*D)
		+ e * 0.000064 * cos(2*D - M + Mpr)
		- e * 0.000063 * cos(2*D + M - Mpr)
		+ e * 0.000041 * cos(M + D)
		+ e * 0.000035 * cos(2*Mpr - M)
		- 0.000033 * cos(3*Mpr - 2*D)
		- 0.000030 * cos(Mpr + D)
		- 0.000029 * cos(2*F - 2*D)
		- e * 0.000029 * cos(2*Mpr + M)
		+ e * e * 0.000026 * cos(2*D - 2*M)
		- 0.000023 * cos(2*F - 2*D + Mpr)
		+ e * 0.000019 * cos(4*D - M - Mpr);

	beta = beta/DEG_IN_RADIAN;
	lambda = lambda/DEG_IN_RADIAN;
	l = cos(lambda) * cos(beta);    
	m = sin(lambda) * cos(beta);
	n = sin(beta);
	eclrot(jd,&l,&m,&n);
	
	dist = 1/sin((pie)/DEG_IN_RADIAN);
	x = l * dist;
	y = m * dist;
	z = n * dist;

	*geora = atan_circ(l,m) * HRS_IN_RADIAN;
	*geodec = asin(n) * DEG_IN_RADIAN;        
	*geodist = dist;

	geocent(lst,geolat,elevsea,&x_geo,&y_geo,&z_geo);
	
	x = x - x_geo;  /* topocentric correction using elliptical earth fig. */
	y = y - y_geo;
	z = z - z_geo;

	*topodist = sqrt(x*x + y*y + z*z);
	
	l = x / (*topodist);
	m = y / (*topodist);
	n = z / (*topodist);

	*topora = atan_circ(l,m) * HRS_IN_RADIAN;
	*topodec = asin(n) * DEG_IN_RADIAN; 

}

void flmoon(n,nph,jdout) 
           
	int n,nph;
	double *jdout;
           
/* Gives jd (+- 2 min) of phase nph on lunation n; replaces
less accurate Numerical Recipes routine.  This routine 
implements formulae found in Jean Meeus' *Astronomical Formulae
for Calculators*, 2nd edition, Willman-Bell.  A very useful
book!! */

/* n, nph lunation and phase; nph = 0 new, 1 1st, 2 full, 3 last 
 *jdout   jd of requested phase */

{
	double jd, cor;
	double M, Mpr, F;
	double T;
	double lun;

	lun = (double) n + (double) nph / 4.;
	T = lun / 1236.85;
	jd = 2415020.75933 + 29.53058868 * lun  
		+ 0.0001178 * T * T 
		- 0.000000155 * T * T * T
		+ 0.00033 * sin((166.56 + 132.87 * T - 0.009173 * T * T)/DEG_IN_RADIAN);
	M = 359.2242 + 29.10535608 * lun - 0.0000333 * T * T - 0.00000347 * T * T * T;
	M = M / DEG_IN_RADIAN;
	Mpr = 306.0253 + 385.81691806 * lun + 0.0107306 * T * T + 0.00001236 * T * T * T;
	Mpr = Mpr / DEG_IN_RADIAN;
	F = 21.2964 + 390.67050646 * lun - 0.0016528 * T * T - 0.00000239 * T * T * T;
	F = F / DEG_IN_RADIAN;
	if((nph == 0) || (nph == 2)) {/* new or full */
		cor =   (0.1734 - 0.000393*T) * sin(M)
			+ 0.0021 * sin(2*M)
			- 0.4068 * sin(Mpr)
			+ 0.0161 * sin(2*Mpr)
			- 0.0004 * sin(3*Mpr)
			+ 0.0104 * sin(2*F)
			- 0.0051 * sin(M + Mpr)
			- 0.0074 * sin(M - Mpr)
			+ 0.0004 * sin(2*F+M)
			- 0.0004 * sin(2*F-M)
			- 0.0006 * sin(2*F+Mpr)
			+ 0.0010 * sin(2*F-Mpr)
			+ 0.0005 * sin(M+2*Mpr);
		jd = jd + cor;
	}
	else {
		cor = (0.1721 - 0.0004*T) * sin(M)
			+ 0.0021 * sin(2 * M)
			- 0.6280 * sin(Mpr)
			+ 0.0089 * sin(2 * Mpr)
			- 0.0004 * sin(3 * Mpr)
			+ 0.0079 * sin(2*F)
			- 0.0119 * sin(M + Mpr)
			- 0.0047 * sin(M - Mpr)
			+ 0.0003 * sin(2 * F + M)
			- 0.0004 * sin(2 * F - M)
			- 0.0006 * sin(2 * F + Mpr)
			+ 0.0021 * sin(2 * F - Mpr)
			+ 0.0003 * sin(M + 2 * Mpr)
			+ 0.0004 * sin(M - 2 * Mpr)
			- 0.0003 * sin(2*M + Mpr);
		if(nph == 1) cor = cor + 0.0028 - 
				0.0004 * cos(M) + 0.0003 * cos(Mpr);
		if(nph == 3) cor = cor - 0.0028 +
				0.0004 * cos(M) - 0.0003 * cos(Mpr);
		jd = jd + cor;

	}
	*jdout = jd;
}

float lun_age(jd, nlun) 
             
	double jd; 
	int *nlun; 
             
{
	/* compute age in days of moon since last new,
	   and lunation of last new moon. */

	int n; /* appropriate lunation */
	int nlast;
	double newjd, lastnewjd;
	short kount=0;
	float x;

	nlast = (jd - 2415020.5) / 29.5307 - 1;
	 
	flmoon(nlast,0,&lastnewjd);
	nlast++;
	flmoon(nlast,0,&newjd);
	while((newjd < jd) && (kount < 40)) {
		lastnewjd = newjd;
		nlast++;
		flmoon(nlast,0,&newjd);
	}
 	if(kount > 35) {
		oprntf("Didn't find phase in lun_age!\n");
		x = -10.;
                *nlun = 0;
	}
	else {
	  x = jd - lastnewjd;
	  *nlun = nlast - 1;
        }

	return(x);
}       

void print_phase(jd)
	double jd; 

{
	/* prints a verbal description of moon phase, given the
	   julian date.  */

	int n; /* appropriate lunation */
	int nlast, noctiles;
	double newjd, lastnewjd;
	double fqjd, fljd, lqjd;  /* jds of first, full, and last in this lun.*/
	short kount=0;
	float x;

	nlast = (jd - 2415020.5) / 29.5307 - 1;  /* find current lunation */
	 
	flmoon(nlast,0,&lastnewjd);
	nlast++;
	flmoon(nlast,0,&newjd);
	while((newjd < jd) && (kount < 40)) {
		lastnewjd = newjd;
		nlast++;
		flmoon(nlast,0,&newjd);
	}
	if(kount > 35) {  /* oops ... didn't find it ... */
		oprntf("Didn't find phase in print_phase!\n");
		x = -10.;
	}
	else {     /* found lunation ok */
		x = jd - lastnewjd;
		nlast--;
		noctiles = x / 3.69134;  /* 3.69134 = 1/8 month; truncate. */
		if(noctiles == 0) oprntf("%3.1f days since new moon",x);
		else if (noctiles <= 2) {  /* nearest first quarter */
			flmoon(nlast,1,&fqjd);
			x = jd - fqjd;
			if(x < 0.)
			  oprntf("%3.1f days before first quarter",(-1.*x));
			else
			  oprntf("%3.1f days since first quarter",x);
		}
		else if (noctiles <= 4) {  /* nearest full */
			flmoon(nlast,2,&fljd);
			x = jd - fljd;
			if(x < 0.) 
			  oprntf("%3.1f days until full moon",(-1.*x));
			else
			  oprntf("%3.1f days after full moon",x);
		}
		else if (noctiles <= 6) {  /* nearest last quarter */
			flmoon(nlast,3,&lqjd);
			x = jd - lqjd;
			if(x < 0.)
			  oprntf("%3.1f days before last quarter",(-1.*x));
			else
			  oprntf("%3.1f days after last quarter",x);
		}
		else oprntf("%3.1f days before new moon",(newjd - jd));
	}
}       

double lunskybright(alpha,rho,kzen,altmoon,alt, moondist) 

	double alpha,rho,kzen,altmoon,alt,moondist; 
		   
/* Evaluates predicted LUNAR part of sky brightness, in 
   V magnitudes per square arcsecond, following K. Krisciunas
   and B. E. Schaeffer (1991) PASP 103, 1033.

   alpha = separation of sun and moon as seen from earth,
   converted internally to its supplement,
   rho = separation of moon and object,
   kzen = zenith extinction coefficient, 
   altmoon = altitude of moon above horizon,
   alt = altitude of object above horizon 
   moondist = distance to moon, in earth radii

   all are in decimal degrees. */

{

    double istar,Xzm,Xo,Z,Zmoon,Bmoon,fofrho,rho_rad,test;

    rho_rad = rho/DEG_IN_RADIAN;
    alpha = (180. - alpha); 
    Zmoon = (90. - altmoon)/DEG_IN_RADIAN;
    Z = (90. - alt)/DEG_IN_RADIAN;
    moondist = moondist/(60.27);  /* divide by mean distance */

    istar = -0.4*(3.84 + 0.026*fabs(alpha) + 4.0e-9*pow(alpha,4.)); /*eqn 20*/
    istar =  pow(10.,istar)/(moondist * moondist);
    if(fabs(alpha) < 7.)   /* crude accounting for opposition effect */
	istar = istar * (1.35 - 0.05 * fabs(istar));
	/* 35 per cent brighter at full, effect tapering linearly to 
	   zero at 7 degrees away from full. mentioned peripherally in 
	   Krisciunas and Scheafer, p. 1035. */
    fofrho = 229087. * (1.06 + cos(rho_rad)*cos(rho_rad));
    if(fabs(rho) > 10.)
       fofrho=fofrho+pow(10.,(6.15 - rho/40.));            /* eqn 21 */
    else if (fabs(rho) > 0.25)
       fofrho= fofrho+ 6.2e7 / (rho*rho);   /* eqn 19 */
    else fofrho = fofrho+9.9e8;  /*for 1/4 degree -- radius of moon! */
    Xzm = sqrt(1.0 - 0.96*sin(Zmoon)*sin(Zmoon));
    if(Xzm != 0.) Xzm = 1./Xzm;  
	  else Xzm = 10000.;     
    Xo = sqrt(1.0 - 0.96*sin(Z)*sin(Z));
    if(Xo != 0.) Xo = 1./Xo;
	  else Xo = 10000.; 
    Bmoon = fofrho * istar * pow(10.,(-0.4*kzen*Xzm)) 
	  * (1. - pow(10.,(-0.4*kzen*Xo)));   /* nanoLamberts */
    if(Bmoon > 0.001) 
      return(22.50 - 1.08574 * log(Bmoon/34.08)); /* V mag per sq arcs-eqn 1 */
    else return(99.);                                     
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

}

double jd_moon_alt(alt,jdguess,lat,longit,elevsea)
                  
	double alt,jdguess,lat,longit,elevsea;
                  
{
	/* returns jd at which moon is at a given 
	altitude, given jdguess as a starting point. In current version
	uses high-precision moon -- execution time does not seem to be
	excessive on modern hardware.  If it's a problem on your machine,
	you can replace calls to 'accumoon' with 'lpmoon' and remove
	the 'elevsea' argument. */

	double jdout;
	double deriv, err, del = 0.002;
	double ra,dec,dist,geora,geodec,geodist,sid,ha,alt2,alt3,az;
	short i = 0;

	/* first guess */
	
	sid=lst(jdguess,longit);
	accumoon(jdguess,lat,sid,elevsea,&geora,&geodec,&geodist,
				&ra,&dec,&dist);
	ha = lst(jdguess,longit) - ra;
	alt2 = altit(dec,ha,lat,&az);
	jdguess = jdguess + del;
	sid = lst(jdguess,longit);
	accumoon(jdguess,lat,sid,elevsea,&geora,&geodec,&geodist,
				&ra,&dec,&dist);
	alt3 = altit(dec,(sid - ra),lat,&az);
	err = alt3 - alt;
	deriv = (alt3 - alt2) / del;
	while((fabs(err) > 0.1) && (i < 10)) {
		jdguess = jdguess - err/deriv;
		sid=lst(jdguess,longit);
		accumoon(jdguess,lat,sid,elevsea,&geora,&geodec,&geodist,
				&ra,&dec,&dist);
		alt3 = altit(dec,(sid - ra),lat,&az);
		err = alt3 - alt;
		i++;
		if(i == 9) oprntf("Moonrise or -set calculation not converging!!...\n");
	}       
	if(i >= 9) jdguess = -1000.;
	jdout = jdguess;
	return(jdout);
}

double jd_sun_alt(alt,jdguess,lat,longit)
                 
	double alt,jdguess,lat,longit; 

{
	/* returns jd at which sun is at a given 
	altitude, given jdguess as a starting point. Uses
	low-precision sun, which is plenty good enough. */

	double jdout;
	double deriv, err, del = 0.002;
	double ra,dec,ha,alt2,alt3,az;
	short i = 0;

	/* first guess */
	
	lpsun(jdguess,&ra,&dec);
	ha = lst(jdguess,longit) - ra;
	alt2 = altit(dec,ha,lat,&az);
	jdguess = jdguess + del;
	lpsun(jdguess,&ra,&dec);
	alt3 = altit(dec,(lst(jdguess,longit) - ra),lat,&az);
	err = alt3 - alt;
	deriv = (alt3 - alt2) / del;
	while((fabs(err) > 0.1) && (i < 10)) {
		jdguess = jdguess - err/deriv;
		lpsun(jdguess,&ra,&dec);
		alt3 = altit(dec,(lst(jdguess,longit) - ra),lat,&az);
		err = alt3 - alt;
		i++;
		if(i == 9) oprntf("Sunrise, set, or twilight calculation not converging!\n");
	}       
	if(i >= 9) jdguess = -1000.;
	jdout = jdguess;
	return(jdout);
}

float ztwilight(alt) 
	double alt;
{

/* evaluates a polynomial expansion for the approximate brightening
   in magnitudes of the zenith in twilight compared to its 
   value at full night, as function of altitude of the sun (in degrees).
   To get this expression I looked in Meinel, A.,
   & Meinel, M., "Sunsets, Twilight, & Evening Skies", Cambridge U.
   Press, 1983; there's a graph on p. 38 showing the decline of 
   zenith twilight.  I read points off this graph and fit them with a
   polynomial; I don't even know what band there data are for! */
/* Comparison with Ashburn, E. V. 1952, JGR, v.57, p.85 shows that this
   is a good fit to his B-band measurements.  */

	float y, val;
	
	y = (-1.* alt - 9.0) / 9.0;  /* my polynomial's argument...*/
	val = ((2.0635175 * y + 1.246602) * y - 9.4084495)*y + 6.132725;
	return(val);
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
			oprntf("Error in true_jd -- nonexistent input time during std->dst change.\n");
			oprntf("Specify as 1 hour later!\n");
			return(-1.); /* signal of nonexistent time */
		}
		test = jd + stdz/24. - jde;
		if((test > 0.) && (test < 0.041666666))   {
			oprntf("WARNING ... ambiguous input time during dst->std change!\n");
		}
	   }
	   jd = jd + zone(use_dst,stdz,(jd+stdz/24.),jdb,jde)/24.;
			/* effect should be to default to standard time. */
        }
	else jd = date_to_jd(date);
	
	return(jd);
}


void print_tz(jd,use,jdb,jde,zabr) 
             
	double jd;
	short use;
	double jdb,jde;
	char zabr; 
             
{
	/* prints correct time abbreviation, given zabr as the
	   single character abbreviation for the time zone,
	   "D" or "S" depending on daylight or standard (dst 
	    begins at jdb, ends at jde) and current jd. */
    
	oprntf(" %c",zabr);
	if((jd > jdb) && (jd < jde) && (use > 0)) oprntf("D");
	  else if(((jd < jdb) || (jd > jde)) && (use < 0)) oprntf("D");  
	  else oprntf("S");
	oprntf("T");
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

aberrate(epoch, vec, from_std) 
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
			else oprntf("Error in parang..asin(>1)\n");
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
			else oprntf("Error in parang..asin(>1)\n");
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

double refract_size(alt, elev) 
	double alt;   /* altitude in degrees */
	double elev;  /* meters */

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


void refract_corr(ha , dec, lat, elev, size, sense)
	double *ha, *dec, *size, lat, elev;  // meters for elevation  size is amount of correction.
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
