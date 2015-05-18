
/*
  program used to test the motor control signals that eminate from the
	DT2819 and DT2816 boards.  Tests both Stepper motors and DC motors.

	This program uses the hand paddle to control the telescope.
  */
#include <iostream.h>
#define INCL_DOS
#include <os2.h>
#include <stdio.h>

#include "iopl.h"#include <stdlib.h>
#include <conio.h>
#include <string.h>
//extern "C" {
//#include "pacerdef.h"
//};

#include "portbit.h"
#include "motor.h"
#include "stpmtr.h"
#include "encoder.h"
#include "dcmotor.h"


#define OUTPUT	0
#define INPUT	1
#define RES32	pow(2,32)  //0-7
#define RES16	pow(2,16)
#define COUNTERRES32	32
#define COUNTERRES16	16   //0-7


  DT2819 board1 (0x230,OUTPUT,INPUT); // initialize boards...
  DT2819 board2 (0x250,OUTPUT,INPUT);
  DT2819 board3 (0x270,INPUT,INPUT);
  DT2816 board4 (0x290);

  PortBit drivechassisonoff  (&board1,0,5); // port 0 bit 5 dirves on off.
  PortBit drivesonoff (&board2,1,0); // drives on off from front panel..

  // RA directions,clutch and enables.
  PortBit rafwdrev (&board1,0,0); // port 0, bit 0.
  PortBit raclockfout (&board1,0,FOUTBIT); // fout.
  PortBit raclutch (&board1,0,2);
  PortBit raslewenable (&board1,0,3);
  PortBit radcfwdrev (&board1,0,4); // port 0, bit 4.


  // RA directions,clutch and enables.
  PortBit decfwdrev (&board2,0,0); // port 0, bit 0. stepper motor fwd/rev
  PortBit decclockfout (&board2,0,FOUTBIT); // fout for dec steppermotor
  PortBit decclutch (&board2,0,2);    // clutch
  PortBit decslewenable (&board2,0,3);  // slew enable
  PortBit decdcfwdrev (&board2,0,4); // port 0, bit 4. dec fwd/rev for slew.

  // limit switches.
  PortBit limitN5 (&board1,1,0);
  PortBit limitS5 (&board1,1,1);
  PortBit limitE5 (&board1,1,2);
  PortBit limitW5 (&board1,1,3);
  PortBit limitN8 (&board1,1,4);
  PortBit limitS8 (&board1,1,5);
  PortBit limitE8 (&board1,1,6);
  PortBit limitW8 (&board1,1,7);
  // Paddle.
  PortBit paddleN (&board3,0,0);
  PortBit paddleS (&board3,0,1);
  PortBit paddleE (&board3,0,2);
  PortBit paddleW (&board3,0,3);
  PortBit guide_set (&board3,0,4);
  PortBit slew (&board3,0,5);
  PortBit focusin (&board3,0,6);
  PortBit focusout (&board3,0,7);
  // Front Panel
  PortBit autodome (&board2,1,1);
  PortBit autoguide (&board2,1,2);
  PortBit go (&board2,1,3);  // front panel go.
  PortBit track (&board1,1,5);  // auto guide.
  PortBit executego (&board3,1,0); // not used
  PortBit stop (&board3,1,1); // not used
  PortBit AutoN (&board3,1,2);
  PortBit AutoS (&board3,1,3);
  PortBit AutoE (&board3,1,4);
  PortBit AutoW (&board3,1,5);
  PortBit Autoguide_fast (&board3,1,6);
  // dome direction
  PortBit domerev (&board1,0,6);
  PortBit domefwd (&board1,0,7);

  // focus direction.
	PortBit focusfwd (&board2,0,5);
	PortBit focusrev (&board2,0,6);
  PortBit focusfwdlimit (&board2,1,6);
  PortBit focusrevlimit (&board2,1,7);  // DC motors.
  DCMotor radc (&board4,0,radcfwdrev); // dac 0.
  DCMotor decdc (&board4,1,decdcfwdrev); //dac 1.
  DCMotor dome (&board4,2,domefwd,domerev); // dac 2.

  // Stepper motors.
  StepperMotor focus (focusfwd,focusrev,20.4);
  StepperMotor rastep (rafwdrev,raclockfout,20.4);
  StepperMotor decstep (decfwdrev,decclockfout,20.4);
/*
  Encoder dec_encoder (RES32,COUNTERRES32,&board2,&board2,1,3);
  Encoder ra_encoder (RES32,COUNTERRES32,&board1,&board1,1,3);
  Encoder focus_encoder (RES16,COUNTERRES16,&board2,&board3,5,4);
  Encoder dome_encoder (RES16,COUNTERRES16,&board1,&board3,5,3);
  Encoder inst_encoder (RES16,COUNTERRES16,&board3,&board3,1,2);
*/
int raspeedflag,raclutchflag,decspeedflag,decclutchflag;


int inputanint (int &port,char *entrystring)
{
	  for (port = -1;port == -1;) {
		 printf ("%s",entrystring);
		 scanf("%d", &port);
	  }
	  if (port  == 999 || (char)port == 'q') return 1;
	  return 0;
}
void zero_ports ()
{
  board1.output_digital_value (0,0);
  board2.output_digital_value (0,0);
  board3.output_digital_value (0,0);
}
void all_stop ()
{
  cerr <<"\r\nSTOP ALL MOTORS........................\n";
  dome.stop ();
  radc.stop ();
  decdc.stop ();
  rastep.stop ();
  decstep.stop ();
  focus.stop ();
}
void main (int , char **) {

  cerr <<"entering program"<<endl;

  int loc,which;
	int driveson = 0;
  char buf[255];

  do {
	 zero_ports ();
	 all_stop ();
	 cerr <<"all stop"<<endl;
	 strcpy(buf,"any # to start.(999 for quit):  ");
	 if (inputanint (which,buf)) break;  // exit out of program.


	 int value,pause=0;
	 unsigned short donoff,pn,ps,pe,pw,pset_g,pslew,pfi,pfo;
double setspeed=0.0,slewspeed=7.0;
	 int raclutchstate=0,decclutchstate=0;
	  int movingns = 0, movingew =0;
	  donoff = drivesonoff.get_bit ();
	  driveson = donoff;
			 if (donoff) {
				driveson = 1;
				drivechassisonoff.clear_bit ();
			 }
			 else if (!donoff) {
				driveson = 0;
				drivechassisonoff.set_bit ();
			 }

	 do
	 {
		if ((value = kbhit ())!= 0) {
		  value = getch ();
		  //cerr <<value<<endl;
		  if (value == 13) pause = 0;
		  else if (value == 32) pause = 1;
		  else break;

		}
		for (double gg=0;gg<1000000;gg+=1) ;
 		if (!pause) {
		  cerr <<"------------------------------------------------"<<endl;
			 donoff = drivesonoff.get_bit ();
			 pn = paddleN.get_bit ();			 ps = paddleS.get_bit ();
			 pe = paddleE.get_bit ();
			 pw = paddleW.get_bit ();
			 pset_g = guide_set.get_bit ();
			 pslew = slew.get_bit ();
			 pfi = focusin.get_bit ();
			 pfo = focusout.get_bit ();
			 if (donoff && !driveson) {
				driveson = 1;
				drivechassisonoff.clear_bit ();
			 }
			 else if (driveson && !donoff) {
				driveson = 0;
				drivechassisonoff.set_bit ();
			 }

			 if (!pset_g) {
				setspeed = 20833.0;
			 }
			 else {
				setspeed = 781.0;
			 }
			 if (!pslew) {
			  if (!pslew && (!pn || !ps)) {
				if (decclutchstate) {
cerr <<"SET DEC CLUTCH BIT\n";
				  decdc.speed (0.0);
				  decclutch.set_bit ();
				  decclutchstate = 0;
				}
				if (!pn) {
				  cerr <<"SLEWING NORTH\n";
				  if (movingns == -1) decdc.speed (0.0);
				  decdc.set_direction (1);
				  decslewenable.set_bit ();
				  decdc.speed ((double) slewspeed);
				  movingns= 1;
				}
				else if (!ps) {
				  cerr <<"SLEWING SOUTH\n";
				  if (movingns == 1) decdc.speed (0.0);
				  decdc.set_direction (-1);
				  decslewenable.set_bit ();
				  decdc.speed ((double) slewspeed);
				  movingns = -1;
				}
				else {
				  movingns = 0;
				  decdc.speed ((double) 0.0);
				  decslewenable.clear_bit ();
				}
			  }			  if (!pslew && (!pe || !pw)) { // slew overrides set & guide.
				// engage clutch.
				if (raclutchstate) {
cerr <<"SET RA CLUTCH BIT\n";
					radc.speed (0.0);
				  raclutch.set_bit ();
				  raclutchstate = 0;
				}

				if (!pe) {
				  cerr <<"SLEWING EAST\n";
				  if (movingew == 1) radc.speed (0.0);
				  radc.set_direction (1);
				  raslewenable.set_bit ();
				  radc.speed ((double) slewspeed);
				  movingew = -1;
				}
				else if (!pw) {
				  cerr <<"SLEWING WEST\n";
				  if (movingew == -1) radc.speed (0.0);
				  radc.set_direction (-1);
				  raslewenable.set_bit ();
				  radc.speed ((double) slewspeed);
				  movingew = 1;
				}
				else {
				  movingew = 0;
				  radc.speed ((double) 0.0);
				  raslewenable.clear_bit ();
				}

			  }

			  if (!decclutchstate && (pn && ps)) {
				cerr <<"SLEW to zero..DEC!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
				  decdc.speed ((double) 0.0);
				  decslewenable.clear_bit ();
				  decclutch.clear_bit ();
				  decclutchstate = 1;
				}
			  if (!raclutchstate && (pe && pw)) {
				cerr <<"SLEW to zero..RA!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
				  radc.speed ((double) 0.0);
				  raslewenable.clear_bit ();
				  raclutch.clear_bit ();
				  raclutchstate = 1;
				}			 }
			 else {

				if (!raclutchstate || !decclutchstate) {

				  radc.speed ((double) 0.0);
				  decdc.speed ((double) 0.0);
				  raslewenable.clear_bit ();
				  decslewenable.clear_bit ();
cerr <<"clear clutch bit\n";
				  raclutch.clear_bit ();
				  decclutch.clear_bit ();
				  raclutchstate = 1;
				  decclutchstate = 1;
				}

				if (!pn) {
				  decstep.direction (1);
				  decstep.speed ((double) setspeed);
				}
				else if (!ps) {
				  decstep.direction (-1);

				  decstep.speed ((double) setspeed);
				}
				else {

				  decstep.speed ((double) 0.0);
				}
				if (!pe) {
				  rastep.direction (1);

				  rastep.speed ((double) setspeed);
				}
				else if (!pw) {
				  rastep.direction (-1);
				  rastep.speed ((double) setspeed);
				}
				else {
				  rastep.speed ((double) 0.0);
				}

		  }
		  cerr <<"drives onoff:"<<donoff<<"north:"<<pn<<" south:"<<ps<<" east:"<<pe
				 <<" west:"<<pw<<" set:"<<pset_g<<" slew:"<<pslew
				 <<" fin:"<<pfi<<" fout:"<<pfo<<endl;



/*
		  cerr <<"RA:"<<ra_encoder.read_location ()
				 <<"  DEC:"<<dec_encoder.read_location ()
				 <<"  Focus:"<<focus_encoder.read_location ()
				 <<"  Dome:"<<dome_encoder.read_location ()<<endl;
*/
	  }


	 }  while (1);
  }
  while (1);
}