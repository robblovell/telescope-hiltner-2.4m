/*
	This program is used from dos to read the DT2819 counters.
	This program can be compiled in OS/2 becasue it uses the encoder.h
	object.
 */
#define INCL_DOS
#include <os2.h>
#include <stdio.h>

#include "iopl.h"

#include <stdlib.h>   /* include standard library file				*/
#include <string.h>
#include "encoder.h"
#define RES	8  //0-7
#define OUTPUT	0
#define INPUT	1
#define COUNTERRES	3   //0-7
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


  DT2819 board1 (0x230,OUTPUT,INPUT); // initialize boards...
  DT2819 board2 (0x250,OUTPUT,INPUT);
  DT2819 board3 (0x270,INPUT,INPUT);

  DT2819 *board[2];

  int counter[2]={1,2};


  board[0] = &board1;
  board[1] = &board1;
  cerr <<"entering program"<<endl;;
  Encoder dec (RES,COUNTERRES,board,counter);
  cerr <<"HELLO ROBB!!!"<<dec.min<<"  "<<dec.max<<"  "<<dec.resolution<<"  "<<dec.range<<endl;
  cerr <<endl;
  int value,updwn;
  int i,decval = 0;
  char buf[255];
	do
	{
	  dec.dout ();
	  strcpy(buf,"\r\n up or down (0,1)(999 for quit):  ");
	  if (inputanint (updwn,buf)) break;  // exit out of program.

	  strcpy(buf,"\r\n amount:  ");
	  if (inputanint (value,buf)) break;  // exit out of program.
	  switch (updwn) {
		 case 0: for (i=0;i<value;i++)
					  if (++decval > RES-1) decval = 0;
					dec.test_count_up (value); break;
		 case 1: for (i=0;i<value;i++)
					  if (--decval < 0) decval = RES-1;
					dec.test_count_dwn (value);
					break;
	  }
	  printf ("dec.location:%d   acutal location:%d \n",
				  dec.test_read_location (),decval);
	}  while (1);

	exit (0);
}