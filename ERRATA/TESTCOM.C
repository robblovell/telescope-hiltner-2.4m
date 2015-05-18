/* _bios_serialcom example */

#include <bios.h>
#include <conio.h>
#include <fcntl.h>
#include <stdlib.h>
#define COM1       0
#define DATA_READY 0x100
#define TRUE       1
#define FALSE      0
#define SETTINGS (_COM_9600 | _COM_CHR8 | _COM_STOP1 | _COM_NOPARITY)

int main(void)
{
  unsigned in, out, status;
  int thex,they;
  int comfp;
  char thestring[8];
  _bios_serialcom(_COM_INIT, COM1, SETTINGS);
  cprintf("... _BIOS_SERIALCOM [ESC] to exit ...\r\n");
  for (;;)
  {
	 //comfp = open ("com1:",O_RDWR);
	 status = _bios_serialcom(_COM_STATUS, COM1, 0);
	 if (status & DATA_READY)
		if ((out = _bios_serialcom(_COM_RECEIVE, COM1, 0) & 0x7F) != 0)
		  putch(out);

	 if (kbhit())
	 {
		if ((in = getch()) == '\x1B')
		  break;
		  //printf ("%c %d %x\n",in,in,in);

		thex = (int)((float)rand()/(float)RAND_MAX*640.0);
		they = (int)((float)rand()/(float)RAND_MAX*480.0);
		printf("%d,%d\n",thex,they);
		thestring[0] = 0;
		thestring[0] = thex/128;
		thestring[1] = thex%128;
		thestring[2] = they/128;
		thestring[3] = they%128;
		thestring[4] = '\0';
		printf (" %d %d %d %d \n",thestring[0],thestring[1],thestring[2],thestring[3]);
		/*
		write (comfp,thestring,6);
		*/
		_bios_serialcom(_COM_SEND, COM1, (char)thestring[0]);

		_bios_serialcom(_COM_SEND, COM1, (char)thestring[1]);
		_bios_serialcom(_COM_SEND, COM1, (char)thestring[2]);
		_bios_serialcom(_COM_SEND, COM1, (char)thestring[3]);
		_bios_serialcom(_COM_SEND, COM1, (char)thestring[4]);
	  //	_bios_serialcom(_COM_SEND, COM1, '\0');

	 }
  }
  return 0;
}

