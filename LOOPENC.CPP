#include <stdio.h>
#include <iostream.h>
#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <iostream.h>
#include "iopl.h"
extern "C" {
#include <math.h>

#include <conio.h>
#include <stdlib.h>   /* include standard library file				*/
#include <string.h>
}
#define	 AMCONTROL	(address+1)
#define  AMDATA		(address+0)
#define  MASTERMODE	(address+0)
#define  DT19CONTROL	(address+4)
#define  DT19STATUS	(address+4)
#define  DT19PORT0	(address+2)
#define  DT19PORT1	(address+3)
	 unsigned long address = 0x230;
void status_wait ()
{
	unsigned int status;
		while (((status = IN_PORT (DT19STATUS))>>7) == 1) ;
}
main () {

	 unsigned short mastermode;
	 unsigned short low,high,all,value,counter=1,pause;

			fprintf (stderr,"Board: %x\n",address);
		// Reset the Dt2819 to known state...
		OUT_PORT (DT19CONTROL,0x00);
		OUT_PORT (AMCONTROL,0xFF);
		status_wait ();

		// Reset counters to 0. value comes from load registers.
		OUT_PORT (AMCONTROL,0x5F);
		status_wait ();
		// binary division,no auto inc,data bus 8 bit,fout disabled,divide by 1,
		// source F5,compare disable...


		cerr <<"init board done:"<<address<<endl;
		/*
			 WRITE MASTER MODE
		 */
		mastermode = 0x42B0; //0101 0001 1111 0000  set base freq = 1Mhz and
		//write_master_mode (mastermode);
		OUT_PORT (AMCONTROL,0x17);//write to the master mode on next data write.
		status_wait ();
		fprintf (stderr,"write mastermode: %x\n",mastermode);
		value = mastermode;
		//write_amdata (value);
		OUT_PORT (AMDATA,((value) & 0x00FF));
		status_wait ();
		// write high byte.
		OUT_PORT (AMDATA,((value >>8) & 0x00FF));
		status_wait ();
 /*
			 SET UP COUNTER 1...
	*/
 //		write_mode (1,0x0B29);

	 counter = 1;
	 value = 0x0b29;   //  count repeat on source F1.
	// select mode....
 //	 for (counter =1;counter <6;counter++) {
	 OUT_PORT (AMCONTROL,0x00+counter); // 000 00 + 001-101
	 status_wait ();
	// write amdata. (mode)
	 OUT_PORT (AMDATA,((value) & 0x00FF));
	 status_wait ();
	 // write high byte.
	 OUT_PORT (AMDATA,((value >>8) & 0x00FF));
	 status_wait ();
	 fprintf (stderr,"COUNTER MODE:%x /n",value);
 /*
			 SET UP COUNTER 2...
	*/
 //		write_mode (1,0x0B29);

	 counter = 2;
	 value = 0x0028;   //  count repeat on source F1.
	// select mode....
 //	 for (counter =1;counter <6;counter++) {
	 OUT_PORT (AMCONTROL,0x00+counter); // 000 00 + 001-101
	 status_wait ();
	// write amdata. (mode)
	 OUT_PORT (AMDATA,((value) & 0x00FF));
	 status_wait ();
	 // write high byte.
	 OUT_PORT (AMDATA,((value >>8) & 0x00FF));
	 status_wait ();
	 fprintf (stderr,"COUNTER MODE:%x /n",value);		//}
		 //	load_and_arm_counters (1,2);
	 OUT_PORT (AMCONTROL,0x60+pow(2,1-1)+pow(2,2-1));
	 //load and arm all counters.
	 //OUT_PORT (AMCONTROL,0x63);
	 status_wait ();




 while (1) {
		if ((value = kbhit ())!= 0) {
			value = getch ();
			//cerr <<value<<endl;
			if (value == 13) pause = 0;
			else if (value == 32) pause = 1;
			else break;
		}
	 // save count to hold
	 counter = 1;
	 OUT_PORT (AMCONTROL,0xBF); // 101 xxxxx one hold register.
	 status_wait ();
	 // select hold
	 OUT_PORT (AMCONTROL,0x18+counter); // 000 11 + 001-101
	 status_wait ();
		// read amdata...
		 low = (unsigned char) (IN_PORT (AMDATA) & 0x00FF);
		status_wait ();
		 //fprintf (stderr,"amdata lo:%x ",low);
		 high = (unsigned char) (IN_PORT (AMDATA) & 0x00FF);
		status_wait ();
		 //fprintf (stderr,"amdata hi:%x ",high);
		 high = high << 8;
		 high &= 0xFF00;
		 //fprintf (stderr,"amdata:%x ",high);
		 status_wait ();
		 all = high | low;
		// fprintf (stderr,"amdata v:%x ",all);
		value = all;
		fprintf (stderr,"COUNTER 1:%x ",value);
	 // save count to hold
	 counter = 2;
	 OUT_PORT (AMCONTROL,(0xA0+pow(2,counter))); // 101 xxxxx one hold register.
	 status_wait ();
	 // select hold
	 OUT_PORT (AMCONTROL,0x18+counter); // 000 11 + 001-101
	 status_wait ();
		// read amdata...
		 low = (unsigned char) (IN_PORT (AMDATA) & 0x00FF);
		status_wait ();
		 //fprintf (stderr,"amdata lo:%x ",low);
		 high = (unsigned char) (IN_PORT (AMDATA) & 0x00FF);
		status_wait ();
		 //fprintf (stderr,"amdata hi:%x ",high);
		 high = high << 8;
		 high &= 0xFF00;
		 //fprintf (stderr,"amdata:%x ",high);
		 status_wait ();
		 all = high | low;
		 //fprintf (stderr,"amdata v:%x ",all);
		value = all;
		fprintf (stderr,"COUNTER 2:%x \n",value);	}}