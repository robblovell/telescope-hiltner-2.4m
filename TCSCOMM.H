#ifndef SerialIOhh
#define SerialIOhh
#define INCL_DOS
#define INCL_DOSDEVIOCTL

#include <iostream.h>
#include <stdlib.h>
#include <os2.h>
#include <string.h>
#include <stdio.h>


class SerialIO {
 public:

	HFILE comport;
	unsigned short thebaud,thedatabits,theparity,thestopbits,breaktransmitflag;
	int fd;
	char sbuf[255],rbuf[255];// send and receive buffers.
	unsigned long slen,rlen;

  SerialIO () {
		thebaud = 9600; // baud of 9600.
		thedatabits = 8;  // 8 data bits.  5,6,7 are possible.
		theparity = 0x00; // Parity none.  01=odd,02=even,03=mark,04=space.
		thestopbits = 0x00; // 1 stop bit. 0x01=1.5 sb, 0x02=2 stop bits.
		breaktransmitflag = 0x00; // no break character transmitted. 0x01 for breaks.
		rlen = 0; slen = 0;
  }
	void initialize_port (int whichport);
	~SerialIO () {
		serial_cleanup ();
  }
	void serial_cleanup ();

	int receive ();
	int receive (char *buff);
	int next ();
	int next (char **buff);
	int send ();
	int send (char *buff);
	friend ostream &operator << (ostream& os, SerialIO& p);
};
#endif
