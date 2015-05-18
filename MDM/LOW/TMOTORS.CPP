/*
  program used to test the motor control signals that eminate from the
  DT2819 and DT2816 boards.  Tests both Stepper motors and DC motors.
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
//#include "stpmotor.h"
#include "dcmotor.h"


#define OUTPUT	0
#define INPUT	1


  DT2819 board1 (0x230,OUTPUT,INPUT); // initialize boards...
  DT2819 board2 (0x250,OUTPUT,INPUT);
  DT2819 board3 (0x270,INPUT,INPUT);
  DT2816 board4 (0x290);
  PortBit rafwdrev (&board1,0,0); // port 0, bit 0.
  //PortBit raclock (&board1,0,1); // port 0, bit 1.
  PortBit raclockfout (&board1,0,FOUTBIT); // fout.
  PortBit raclutch (&board1,0,2);
  PortBit raslewenable (&board1,0,3);
  PortBit radcfwdrev (&board1,0,4); // port 0, bit 4.
  PortBit drivesonoff  (&board1,0,5); // port 0 bit 5 dirves on off.

  PortBit decfwdrev (&board2,0,0); // port 0, bit 0.
  //PortBit decclock (&board2,0,1); // port 0, bit 1.
  PortBit decclockfout (&board2,0,FOUTBIT); // fout
  PortBit decclutch (&board2,0,2);
  PortBit decslewenable (&board2,0,3);
  PortBit decdcfwdrev (&board2,0,4); // port 0, bit 4.


  PortBit domerev (&board1,0,6);
  PortBit domefwd (&board1,0,7);

  PortBit focusfwd (&board2,0,5);
  PortBit focusrev (&board2,0,6);

  DCMotor radc (&board4,0,radcfwdrev); // dac 0.
  DCMotor decdc (&board4,1,decdcfwdrev); //dac 1.
  DCMotor dome (&board4,2,domefwd,domerev); // dac 2.

  //StepperMotor rastep (rafwdrev,raclock,20.4);
  //StepperMotor decstep (decfwdrev,decclock,20.4);
  StepperMotor focus (focusfwd,focusrev,20.4);
	// for fout as clock.
  StepperMotor rastep (rafwdrev,raclockfout,20.4);
  StepperMotor decstep (decfwdrev,decclockfout,20.4);

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
void one_ports ()
{
  board1.output_digital_value (0,0xff);
  board2.output_digital_value (0,0xff);
  board3.output_digital_value (0,0xff);
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
void stop_motor ()
{
	int choice;
	char buf[128];

	cerr <<"\r\nSTOP MOTOR..............................";
	strcpy(buf,"\r\n 0=ra,1=dec,2=dome,3=ra step,4=dec step,5=focus, other=none-> ");
	inputanint (choice,buf);  // exit out of program.
	switch (choice) {
	  case 0: radc.stop ();
		 break;
	  case 1: decdc.stop ();
		 break;
	  case 2: dome.stop ();
		 break;
	  case 3: rastep.stop ();
		 break;
	  case 4: decstep.stop ();
		 break;
	  case 5: focus.stop ();
		 break;

	}
}
void motor_direction ()
{
	int choice,direction;
	char buf[128];

	cerr <<"\r\n MOTOR DIRECTION..............................";
	strcpy(buf,"\r\n 0=ra,1=dec,2=dome,3=ra step,4=dec step,5=focus, other=none-> ");
	inputanint (choice,buf);
	strcpy(buf,"\r\n direction:0 or 1-> ");
	inputanint (direction,buf);
	if (!direction) direction = -1;
	switch (choice) {
	  case 0: radc.set_direction (direction); radc.direction (direction);
		 break;
	  case 1: decdc.set_direction (direction); decdc.direction (direction);
		 break;
	  case 2: dome.set_direction (direction);  dome.direction (direction);
		 break;
	  case 3: if (direction == 0) direction = -1;
		  rastep.direction (direction);
		  rastep.set_direction (direction);
		 break;
	  case 4: if (direction == 0) direction = -1;
		  decstep.set_direction (direction);
		  decstep.direction (direction);
		 break;
	  case 5: focus.set_direction (direction);
		 break;
	}
}
void motor_speed ()
{
	int choice,speed;//,distance,direction;
	char buf[128];

	cerr <<"\r\nSPEED (MOVE MOTOR)..............................";
	strcpy(buf,"\r\n 0=ra,1=dec,2=dome,3=ra step,4=dec step,5=focus, other=none-> ");
	inputanint (choice,buf);
	if (choice < 3)
		 strcpy(buf,"\r\n speed:0 to 8-> ");
	else
	  strcpy(buf,"\r\n speed:0 to 1000000-> ");
	inputanint (speed,buf);
	if ((speed <= 0 || speed >= 8) && choice < 3) speed = 0;
	switch (choice) {
	  case 0:raspeedflag = speed;
				radc.speed ((double) speed);
		 break;
	  case 1: decspeedflag = speed;
				 decdc.speed ((double) speed);
		 break;
	  case 2: dome.speed ((double) speed);
		 break;
	  case 3:
	  /*
	strcpy(buf,"\r\n distance:-> ");
	inputanint (distance,buf);
	strcpy(buf,"\r\n direction:-> ");
	inputanint (direction,buf);
			rastep.move ((double )distance,(double) speed, direction);
		*/
		cerr <<"ra speed...\n";
			rastep.speed ((double) speed);
		 break;
	  case 4:
	  		cerr <<"dec speed...\n";
			decstep.speed ((double) speed);
		 break;
	  case 5: focus.speed ((double) speed);
		 break;
	}
}
void clutch ()
{
	int choice,engage;
	char buf[128];

	cerr <<"\r\nCLUTCH..............................";
	strcpy(buf,"\r\n 0=ra,1=dec.-> ");
	inputanint (choice,buf);
	strcpy(buf,"\r\n 1=engage,0=disengage-> ");
	inputanint (engage,buf);

	switch (choice) {
	  case 0:
			if (engage == 1) {
			  raclutch.set_bit ();
			}
			else { //if (raclutchflag) {
			  raclutch.clear_bit ();
			}
			raclutchflag = engage;
						cerr <<"ra clutch....:"<<raclutchflag<<"\n";
		 break;
	  case 1:

			if (engage == 1) { // && !decclutchflag && decspeedflag == 0.0) {
			  decclutch.set_bit ();
			  cerr <<"set bit...\n";
			}
			else { //if (decclutchflag) {
			  decclutch.clear_bit ();
			  cerr <<"clear bit...\n";
			}
			decclutchflag = engage;
			cerr <<"dec clutch....:"<<decclutchflag<<"\n" ;
		 break;
	}
}
void main (int ,char *[])
{
			  int radec;
  int choice,quit;
  char buf[255];
  raspeedflag = 0.0;
  decspeedflag = 0.0;
  raclutchflag = 0;
  decclutchflag = 0;
  quit = 0;
  do
  {

	  cerr <<"\r\n";
	  cerr <<"0->stop all motors\r\n";
	  cerr <<"1->motor\r\n";
	  cerr <<"2->motor direction\r\n";
	  cerr <<"3->motor speed\r\n";
	  cerr <<"4->clutch\r\n";
	  cerr <<"5->zero all ports\r\n";
	  cerr <<"6->drives on/off\r\n";
	  cerr <<"7->slew fwd/rev\r\n";
	  cerr <<"8->slew enable\r\n";
	  cerr <<"9->quit program\r\n";
	  strcpy(buf,"\r\n choice-> ");
	  if (inputanint (choice,buf)) break;  // exit out of program.

	  switch (choice) {
		 case 0: all_stop ();
		         one_ports ();
			break;
		 case 1: stop_motor ();
			break;
		 case 2: motor_direction ();
			break;
		 case 3: motor_speed ();
			break;
		 case 4: clutch ();
			break;
		 case 5: zero_ports ();
			break;
		 case 6:
			  strcpy(buf,"\r\ndrives on/off 1=on, 0=off-> ");
			  if (inputanint (choice,buf)) break;  // exit out of program.
				  cerr <<choice <<endl;
			  if (choice == 1) drivesonoff.set_bit ();
			  else drivesonoff.clear_bit ();
			break;
		 case 7:
			  strcpy(buf,"\r\nra=0 dec=1-> ");

			  if (inputanint (radec,buf)) break;  // exit out of program.

			  strcpy(buf,"\r\nfwd/rev 1=on, 0=off-> ");
			  if (inputanint (choice,buf)) break;  // exit out of program.
								cerr <<choice<<endl;
			  switch (radec) {
				  case 0:
						if (choice == 1) radcfwdrev.set_bit ();
						else radcfwdrev.clear_bit ();
					break;
				  case 1:
						if (choice == 1) decdcfwdrev.set_bit ();
						else decdcfwdrev.clear_bit ();
					 break;
			  }
			 break;
		 case 8:

						  strcpy(buf,"\r\nra=0 dec=1-> ");

			  if (inputanint (radec,buf)) break;  // exit out of program.

			  strcpy(buf,"\r\nslew enable 1=on, 0=off-> ");
			  if (inputanint (choice,buf)) break;  // exit out of program.
								cerr <<choice<<endl;
			  switch (radec) {
				  case 0:
						if (choice == 1) raslewenable.set_bit ();
						else raslewenable.clear_bit ();
					break;
				  case 1:
						if (choice == 1) decslewenable.set_bit ();
						else decslewenable.clear_bit ();
					 break;
			  }
			 break;

		 case 9: quit = 1;
	  }
	}  while (!quit);
	//all_stop ();
	//exit (0);

}
/*
  dome.speed(0.0);
  domerev.clear_bit ();
  domefwd.clear_bit ();
  while (! kbhit ()) cerr <<"waiting  speed 0...\n";
  dome.direction (-1);
  while (! kbhit ()) cerr <<"waiting.direct -1..\n";
  dome.speed (8.5);
  while (! kbhit ()) cerr <<"waiting.speed 8.5..\n";
  getch ();
  dome.speed (0);
  while (! kbhit ()) cerr <<"waiting..speed 0.\n";
  getch ();
  dome.direction (1);
  dome.speed (8.5);
  while (! kbhit ()); cerr <<"waiting.speed 8.5 direction 1..\n";
  getch ();
  dome.speed (0);
  domerev.clear_bit ();
  domefwd.clear_bit ();

  //radc.speed (9.9);  radc.speed (0);  radc.speed (9.9);  radc.speed (0);
  //radc.speed (9.9);  radc.speed (0);
  //rastep.speed (10000);

  //rastep.dout ();
  //rastep.move (1000,10,1);
  //for (;;) {
  //}
  */


