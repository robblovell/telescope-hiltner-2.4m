/*
  testio.c - test IOPL functions 
*/

#define INCL_DOS
#include <os2.h>



#define CONTROL   0x234
#define DATA0   0x232
#define DATA1   0x233
extern USHORT _Far16 _Pascal IN_PORT(USHORT);
extern void _Far16 _Pascal OUT_PORT(USHORT,USHORT);

int main(void)
{
	 USHORT in_stuff;


	 OUT_PORT (CONTROL,0x0c);
	 OUT_PORT (DATA0,0x00);
	 OUT_PORT (DATA1,0x00);
	 in_stuff = IN_PORT (DATA0);
	 printf ("%x\n",in_stuff);}
