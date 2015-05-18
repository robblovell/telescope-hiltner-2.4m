
/*
	program to test for switch input from the DT2819.
*/
extern "C" {
#include <math.h>     /* include math function definitions         */
#include <stdio.h>    /* include standard I/O file                 */
#include <stdlib.h>   /* include standard library file				*/
#include <string.h>
#include "lpclerrs.h" /* include LPCLAB error codes                */
#include "lpcldefs.h" /* include LPCLAB function declarations      */
#include "pacerdef.h"

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
void init_counters () {

  int result;
		/* Reset the board to the power up state */

		result = PA_INITIALIZE(0x230);
		if(result) printf("Error on PA_INITIALIZE: %d\n",result);
		result = PA_RESET();
		if(result) printf("Error on PA_reset: %d\n",result);

		result = PA_ENABLE_FOR_OUTPUT(0);
		if(result) printf("Error on PA_ENABLE_FOR_OUTPUT: %d\n",result);
		result = PA_ENABLE_FOR_INPUT(1,0);  // parameters are port and latch enable.
		if(result) printf("Error on PA_ENABLE_FOR_INPUT: %d\n",result);

		result = PA_INITIALIZE(0x250);
		if(result) printf("Error on PA_INITIALIZE: %d\n",result);
		result = PA_RESET();
		if(result) printf("Error on PA_reset: %d\n",result);

		result = PA_ENABLE_FOR_OUTPUT(0);
		if(result) printf("Error on PA_ENABLE_FOR_OUTPUT: %d\n",result);
		result = PA_ENABLE_FOR_INPUT(1,0);
		if(result) printf("Error on PA_ENABLE_FOR_INPUT: %d\n",result);

		result = PA_INITIALIZE(0x270);
		if(result) printf("Error on PA_INITIALIZE: %d\n",result);

		result = PA_RESET();
		if(result) printf("Error on PA_reset: %d\n",result);

		result = PA_ENABLE_FOR_INPUT(0,0);
		if(result) printf("Error on PA_ENABLE_FOR_INPUT: %d\n",result);
		result = PA_ENABLE_FOR_INPUT(1,0);
		if(result) printf("Error on PA_ENABLE_FOR_INPUT: %d\n",result);
}
void output_value (int board,int port,int bit,unsigned short &value) {
  int result,error;
  char buf[255];

  strcpy (buf, "\r\n value (0 or 1):  ");
  inputanint ((int)value,buf); // can't exit here...

  switch (board) {
	 case 3:


		 error = lp_enable_for_output (port);

		 if (error != E_NORMAL)
		 {
			printf("\r\n\n LP_ENABLE_FOR_OUTPUT error code returned is %d.\r\n", error);
		 }
		 printf ("output:%x\n",value);
		 error = lp_output_digital_value (port,whichbit[bit],value);
		 if (error != E_NORMAL)
		 {
			printf("\r\n\n LP_OUTPUT_DIGITAL_VALUE error code returned is %d.\r\n", error);
		 }
		 	 break;
  case 0:
		/* Inform Pacer that the base address is set to 230 hex */

		result = PA_INITIALIZE(0x230);
		if(result) printf("Error on PA_INITIALIZE: %d\n",result);
		else printf ("Pacer Library for board 0x230\n");
		goto remainder;
  case 1:
		/* Inform Pacer that the base address is set to 230 hex */

		result = PA_INITIALIZE(0x250);
		if(result) printf("Error on PA_INITIALIZE: %d\n",result);
				else printf ("Pacer Library for board 0x250\n");
		goto remainder;
  case 2:
		/* Inform Pacer that the base address is set to 230 hex */

		result = PA_INITIALIZE(0x270);
		if(result) printf("Error on PA_INITIALIZE: %d\n",result);
				else printf ("Pacer Library for board 0x270\n");
remainder:
		/* Set up Port */

		// already setup for input by the init_counters routine.
		// This is here because it was in the program this one was derived from.
		//result = PA_ENABLE_FOR_OUTPUT(port);
		//if(result) printf("Error on PA_ENABLE_FOR_OUTPUT: %d\n",result);

		error = PA_OUTPUT_DIGITAL_VALUE (port,value);
		 if (error)
		 {
			printf("\r\n\n PA_OUTPUT_DIGITAL_VALUE error code returned is %d.\r\n", error);
		 }
		 else {
			printf ("output:%x\n",value>>bit);
		 }
	 break;
  }
}

void input_value (int board,int port,int bit,unsigned short &value) {
  int result,error;

  switch (board) {
	 case 3:
		 error = lp_enable_for_input (port);
		 bit = whichbit[bit];
		 if (error != E_NORMAL)
		 {
			printf("\r\n\n LP_ENABLE_FOR_INPUT error code returned is %d.\r\n", error);
		 }
		 value = 0;
		 error = lp_input_digital_value (port,whichbit[bit],&value);
		 if (error != E_NORMAL)
		 {
			printf("\r\n\n LP_INPUT_DIGITAL_VALUE error code returned is %d.\r\n", error);
		 }
		 else {
			printf ("input:%x\n",value>>bit);
		 }
	 break;
  case 0:
		/* Inform Pacer that the base address is set to 230 hex */

		result = PA_INITIALIZE(0x230);
		if(result) printf("Error on PA_INITIALIZE: %d\n",result);
		else printf ("Pacer Library for board 0x230\n");
		goto remainder;
  case 1:
		/* Inform Pacer that the base address is set to 230 hex */

		result = PA_INITIALIZE(0x250);
		if(result) printf("Error on PA_INITIALIZE: %d\n",result);
				else printf ("Pacer Library for board 0x250\n");
		goto remainder;
  case 2:
		/* Inform Pacer that the base address is set to 230 hex */

		result = PA_INITIALIZE(0x270);
		if(result) printf("Error on PA_INITIALIZE: %d\n",result);
				else printf ("Pacer Library for board 0x270\n");
remainder:
		/* Set up Port */
		// already setup for input by the init_counters routine.
		// This is here because it was in the program this one was derived from.
		result = PA_ENABLE_FOR_INPUT(port,0);
		if(result) printf("Error on PA_ENABLE_FOR_INPUT: %d\n",result);
      printf ("port:%d  bit:%d*  iotype:input \n",port,bit);
		value = 0;
		error = PA_INPUT_DIGITAL_VALUE (port,&value);
		 if (error)
		 {
			printf("\r\n\n PA_INPUT_DIGITAL_VALUE error code returned is %d.\r\n", error);
		 }
		 else {
			printf ("the input:%x  %d\n",value,value);
		 }
	 break;
  }
}
void main ()
{
	int  board, port, bit, iotype;
	unsigned short value;

	char buf[256];

	printf ("configuring counter timer Digital I/O ports.\n");
	printf ("Board 0 at 0x230:  Port 0->out  Port 1-> in\n");
	printf ("Board 1 at 0x250:  Port 0->out  Port 1-> in\n");
	printf ("Board 2 at 0x270:  Port 0->in  Port 1-> in\n");

	init_counters ();
	lp_initialize();                 /* initialize the LPCLAB subroutines */
	lp_select_board( 2 );            /* address board 1, the first unit */

	do
	{
	  strcpy(buf,"\r\n Board (0-3):  ");
	  if (inputanint (board,buf)) break;  // exit out of program.

	  strcpy(buf,"\r\n Port for I/O (0-3):  ");
	  if (inputanint (port,buf)) break;  // exit out of program.
	  bit = 0;
	  //strcpy (buf,"\r\n Bit (0-7):  ");
	  //if (inputanint (bit,buf)) break;  // exit out of program.
	  if (board == 0 || board == 1) {
		 if (port == 0) iotype = 0; // output.
		 else iotype = 1; // input
	  }
	  else if (board == 2) {
		 iotype = 1;
	  }
	  else { // dedicated io board. dt 2819.
		 strcpy (buf, "\r\n Input or Output (1 for input or 0 for output):  ");
		 if (inputanint (iotype,buf)) break; // exit...
	  }

	  if (iotype == 'I' || iotype == 'i' || iotype == 1) {
		 input_value (board,port,bit,value);
	  }
	  else {
		 output_value (board,port,bit,value);
	  }


	}  while (1);

	lp_terminate();  /* terminate operations */
	printf("\r\n\n End of Program.  Bye!");

	exit( 0 );

} /* end of switch */
}