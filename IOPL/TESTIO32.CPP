/*
  testio.c - test IOPL functions 
*/

#define INCL_DOS
#include <os2.h>
#include <stdio.h>


#define CONTROL   0x234
#define DATA0   0x232
#define DATA1   0x233

extern "C" {
extern USHORT  _far16 _pascal IN_PORT (USHORT);
extern void  _far16 _pascal OUT_PORT (USHORT,USHORT);
}
void out_port (USHORT,USHORT);
int main(void)
{
	 USHORT in_stuff,in_stuff2;


	 OUT_PORT(CONTROL,0x0c);
	 OUT_PORT (DATA1,0x70);
	 OUT_PORT(DATA0,0x7f);

	 in_stuff = IN_PORT (DATA0);
	 in_stuff2 = IN_PORT (DATA1);
	 printf ("%x  %x\n",in_stuff,in_stuff2);
	 return 0;
}
