#include <stdio.h>
#include <math.h>

#define ARCSEC_IN_RADIAN 206264.806
#define ARCSEC_IN_HOUR 54000.;
#define ARCSEC_IN_DEGREE 3600.;

void pointing_model(teleha, teledec, sense) 
   double *teleha, *teledec;
   int sense; 

/* Applies corrections to an hour angle (in arcseconds) and a 
declination (in arcseconds) to model for telescope imperfections.
It is assumed that telescope-independent corrections ....
precession, nutation, aberration, and refraction ... are
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
   
   radha = *teleha / ARCSEC_IN_RADIAN;
   raddec = *teledec / ARCSEC_IN_RADIAN;
   
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
   	*teleha -= dha;
   	*teledec -= ddec;
   }
   else if(sense == -1) {  /* telescope coords -> true coords */
   	*teleha += dha;
   	*teledec += ddec;
   }
   else printf("error in point-model: illegal sense.\n");
}
   
main() {

	double ha = 0., dec = 0., harcsec, decarcsec;

	printf("Testbed ... large neg ha exits.\n");
	while(ha > -1000.) {
		printf("Give hour angle and delta:");
		scanf("%lf",&ha);
		if(ha < -1000.) break;
		scanf("%lf",&dec);
		
                printf("\n\nInput assumed true, output telescope:\n");
		harcsec = ha * ARCSEC_IN_HOUR;
		decarcsec = dec * ARCSEC_IN_DEGREE;
		printf("In arcsec: %f %f\n",harcsec,decarcsec);		
		pointing_model(&harcsec, &decarcsec, 1);
		printf("After cor: %f %f\n",harcsec,decarcsec);
	
		printf("\nInput assumed telescope, output true:\n");
		harcsec = ha * ARCSEC_IN_HOUR;
		decarcsec = dec * ARCSEC_IN_DEGREE;
		printf("In arcsec: %f %f\n",harcsec,decarcsec);		
		pointing_model(&harcsec, &decarcsec, -1);
		printf("After cor: %f %f\n",harcsec,decarcsec);
	}
}
	
		 
