
/*
  This program is used from dos to read and write the ttl inputs of the
  DT2819 board.
*/
#define INCL_DOS
#include <math.h>     /* include math function definitions         */
#include <stdio.h>    /* include standard I/O file                 */
#include <stdlib.h>   /* include standard library file				*/
#include <string.h>
#include <iostream.h>

#include <os2.h>
//#include "lpclerrs.h" /* include LPCLAB error codes                *///#include "lpcldefs.h" /* include LPCLAB function declarations      */
//#include "pacerdef.h"
#include "dtboard.h"
#include "iopl.h"
#include <conio.h>

#include "portbit.h"

#define OUTPUT	0
#define INPUT	1
DT2819 board1 (0x230,OUTPUT,INPUT); // initialize boards...
DT2819 board2 (0x250,OUTPUT,INPUT);
DT2819 board3 (0x270,INPUT,INPUT);
unsigned short whichbit [8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };

int inputanint (int &port,char *entrystring)
{
	  for (port = -1;port == -1;) {
		 printf ("%s",entrystring);
		 scanf("%d", &port);
	  }
	  if (port  == 999 || (char)port == 'q') return 1;
	  return 0;
}

void output_value (int board,unsigned short port,unsigned short bit,unsigned short &value) {
  int result,error,temp;
  char buf[255];
  DT2819 *theboard;
  strcpy (buf, "\r\n value (0 or 1):  ");
  inputanint (temp,buf); // can't exit here...
  value = (USHORT) temp;
  switch (board) {

	 case 0:
		theboard = &board1;
		break;
	 case 1:
		theboard = &board2;
		break;
	 case 2:
		theboard = & board3;
		break;
  }
  /* Set up Port */
  theboard->output_digital_value (port,bit,value);
  printf ("output:%x\n",value>>bit);
}

void input_value (int board,int port,int bit,unsigned short &value) {
  int result,error;
  DT2819 *theboard;
  if (board > 2) return;

  switch (board) {
	 case 0:
		theboard = &board1;

	 case 1:
		theboard = &board2;

	 case 2:
		theboard = &board3;  }
  value = 0;

  theboard->input_digital_value (port,value);
  if(error)
  {
	 printf("\r\n\n PA_INPUT_DIGITAL_VALUE error code returned is %d.\r\n", error);
  }
  else {
	 printf ("input:%x\n",value>>bit);
  }
}

void main ()
{
	int error, board, port, bit, iotype;
	unsigned short value;

	char buf[256];

	printf("\r\n\n\n Switch -- Input or Output a value to an TTL port on");
	printf ("\r\n                  the I/O board.");

	do
	{
	  strcpy(buf,"\r\n Board (0-2):  ");
	  if (inputanint (board,buf)) break;  // exit out of program.

	  strcpy(buf,"\r\n Port for I/O (0-1):  ");
	  if (inputanint (port,buf)) break;  // exit out of program.
	  strcpy (buf,"\r\n Bit (0-7):  ");
	  if (inputanint (bit,buf)) break;  // exit out of program.
	  switch (board) {
		 case 0:case1:cerr <<"board 1 & 2 have output on port 0 and input on port 1\n";
			  if (port == 0) iotype = 0;
			  else if (port == 1) iotype = 1;
			break;
		 case 2: cerr <<"board 3 is for input only.\n";
			  iotype = 1;
			break;
	  }

	  if (iotype == 'I' || iotype == 'i' || iotype == 1) {
		 input_value (board,port,bit,value);
	  }
	  else {
		 output_value (board,port,bit,value);

	  }


	}  while (1);

	printf("\r\n\n End of Program.  Bye!");
}
