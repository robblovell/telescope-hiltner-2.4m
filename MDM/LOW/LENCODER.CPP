



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
#include "dtboard.h"
#include "encoder.h"
#include "portbit.h"
#define RES	8  //0-7
#define OUTPUT	0
#define INPUT	1
#define COUNTERRES	3

#define RES32	pow(2,32)  //0-7
#define RES16	pow(2,16)
#define COUNTERRES32	32
#define COUNTERRES16	16   //0-7
#include "debug.h"
Debug debug;
  DT2819 board1 (0x230,OUTPUT,INPUT); // initialize boards...
  DT2819 board2 (0x250,OUTPUT,INPUT);
  DT2819 board3 (0x270,INPUT,INPUT);
  DT2816 board4 (0x290);

Encoder dec_encoder (RES32,COUNTERRES32,&board1,&board3,4,4,
										 DECRAWTOARCSEC,0);
Encoder ra_encoder (RES32,COUNTERRES32,&board2,&board2,2,4,
										 RARAWTOARCSEC,0);
Encoder focus_encoder (RES16,COUNTERRES16,&board1,&board3,3,3,
										 FOCUSRAWTOARCSEC,1);
Continuous_Encoder dome_encoder
	 (360.0*3600.0/DOMERAWTOARCSEC,COUNTERRES16,&board1,&board1,1,2,
											DOMERAWTOARCSEC);
Encoder inst_encoder (360.0*3600.0/INSTRAWTOARCSEC,COUNTERRES16,&board3,
										&board3,1,2,INSTRAWTOARCSEC,1);


	Encoder *enc;
	PortBit raclockfout ((DTIO *)&board1,(unsigned int)0,(unsigned int) FOUTBIT);
	PortBit decclockfout ((DTIO *)&board2,(unsigned  int)0,(unsigned int)FOUTBIT);
int inputanint (int &port,char *entrystring)
{
	  for (port = -1;port == -1;) {
		 printf ("%s",entrystring);
		 scanf("%d", &port);
	  }
	  if (port  == 999 || (char)port == 'q') return 1;
	  return 0;
}
void main (int , char **) {

  cerr <<"entering program"<<endl;

  int loc,which;

  char buf[255];

  do {
	 strcpy(buf,"\r\nEncoder? 0=RA,1=DEC,2=DOME,3=Focus,4=Instrument,(999 for quit):  ");
	 if (inputanint (which,buf)) break;  // exit out of program.
	 switch (which) {
		// DEC
		case 1: enc = & dec_encoder;
				  break;
		// RA
		case 0: enc = &ra_encoder;
				  break;
		// Focus
		case 3: enc = &focus_encoder;
				  break;
		// Dome
		case 2: enc = &dome_encoder;
					break;
		// Instrument
		case 4: enc = & inst_encoder;
					break;

	 }
	 int value,pause=0;
	 //raclockfout.set_frequency (20000.0);
	 //decclockfout.set_frequency (20000.0);
	 do
	 {
		if (!pause) {
			loc = enc->read_arcsec_location ();
		  enc->dout ();
			cout <<"location:"<<loc<<endl;
		}
		for (double i=0;i<1000000.0;i+=1) ;
		if ((value = kbhit ())!= 0) {
			value = getch ();
			//cerr <<value<<endl;
			if (value == 13) pause = 0;
			else if (value == 32) pause = 1;
			else break;
		}
	 }  while (1);
	}
	while (1);
}