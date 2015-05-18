
#include <stdio.h>
#include <math.h>

/* Formats a struct coord for printing ... another attempt to deal
   with the rounding problem for 60s in the last place. */

struct coord
   {
     short sign;  /* carry sign explicitly since -0 not neg. */
     double hh;
     double mm;
     double ss;
   };

void dec_to_bab (deci,bab)
                
	double deci;
	struct coord *bab;
                
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

round_coord(incoord, outcoord, prec)

struct coord *incoord, *outcoord;
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

void put_hrs(hrs,sign,width,showpos,alignsign)

double hrs;
short sign;
int width, showpos, alignsign;

/* Puts out the hours (or decimal degrees) with the 
	 following format information:

	-- Allows "width" digits of space for hours;
		e.g. -20 would be width 2.
	-- if showpos == 1, prints a + sign if result is
								positive.
        -- if alignsign == 1, prints sign before the field;
		otherwise places sign flush with digit.
*/

{
	int i, digitsout, leadblanks;
	char outform[20];
	double tmp;

	if(alignsign == 1) {
		if(sign < 0) printf("-");
		else if(showpos == 1) printf("+");
		sprintf(outform,"%%%d.0f",width);
		printf(outform,hrs);
	}
	else {
		tmp = fabs(hrs);
		digitsout = 1;
		while(tmp >= 10.) {
			digitsout++;
			tmp /= 10.;
		}
		if(digitsout >= width) {
			if(sign < 0) printf("-");
			else if (showpos == 1) printf("+");
			printf("%.0f",hrs);
		}
		else {
			for(i = 0; i < width - digitsout; i++) 
				printf(" ");
			if(sign < 0) printf("-");	
			else if (showpos == 1) printf("+");
			else printf(" ");
			sprintf(outform,"%%%d.0f",digitsout);
			printf(outform,hrs);
		}
	}
}	

/*
void put_coords(deci, prec)

double deci;  // decimal version
int prec;   

{
	struct coord bab, babout;
	char formstr[20];
	int outstringlen;

	dec_to_bab(deci,&bab);

	round_coord(&bab,&babout,prec);

	if(prec == 0) {
		put_hrs(babout.hh, babout.sign, 2, 1, 0);
		printf(" %02.0f",babout.mm);
	}
	else if (prec == 1) {
		put_hrs(babout.hh, babout.sign, 2, 1, 0);
		printf(" %04.1f",babout.mm);
	}	
	else {
		if(prec == 2) {
			 put_hrs(babout.hh, babout.sign, 2, 1, 0);
			 printf(" %02.0f %02.0f",babout.mm,babout.ss);
		}
		else {
			put_hrs(babout.hh, babout.sign, 2, 1, 0);
			sprintf(formstr," %%02.0f %%0%d.%df",
			prec+1,prec-2);
			printf(formstr,babout.mm,babout.ss);
		}
	}
}
*/
/*
main() {

	struct coord in, out;
	int prec;
	double deci = 0.;
	
	while(deci > -10000.) {
		printf("Give decimal hours:");
		scanf("%lf",&deci);
		printf("Give precision:");
		scanf("%d",&prec);
		put_coords(deci,prec);
		printf("\n");
	}	
}
*/

