/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/******************************  tcscomm.cpp  ***************************/

/*
		handles all the serial port i/o for the pc.
 */
#include <string.h>

#include "tcscomm.h"
#include "skycalc.h"
#include "hooks.h"
#include "telescpe.h"
#include <time.h>
#include "tcscomm.h"
#include "hiltner.h"
/*****************************  SerialIO Class  **************************/
/*
				Setup the serial ports.
 */
void SerialIO::initialize_port (int whichport)
{
	char whichcom[10];
	sprintf (whichcom,"COM%0d",whichport); // = "COM1";
	unsigned long anaction; // dummy variable.
	// open the comm port for read/write capabilities.
	//whichcom[3] = (char) ('1'+whichport);
	DosOpen (whichcom,&comport,&anaction,0L,FILE_NORMAL,FILE_OPEN,
						OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,0L);

	// set the baud rate to the current baud, 9600 is default in
	// class constructor.
	DosDevIOCtl (comport,IOCTL_ASYNC,ASYNC_SETBAUDRATE,
							(PVOID)&thebaud,sizeof (USHORT),NULL,NULL,0L,NULL);
	// set the data bits to 8.
	LINECONTROL lc;

	lc.bDataBits = thedatabits;
	lc.bParity = theparity;
	lc.bStopBits = thestopbits;
	lc.fTransBreak = breaktransmitflag;
	DosDevIOCtl (comport,IOCTL_ASYNC,ASYNC_SETLINECTRL,&lc,
							sizeof (LINECONTROL),NULL,NULL,0L,NULL);
	// set the flow of control to none. allow for a timeout on reads.

	DCBINFO dcbinfo;
	dcbinfo.usWriteTimeout = 25;
	dcbinfo.usReadTimeout = 25;

	dcbinfo.fbCtlHndShake = 0;
	dcbinfo.fbFlowReplace = 0;
	dcbinfo.fbTimeout = MODE_READ_TIMEOUT;
	dcbinfo.bErrorReplacementChar = '\0';
	dcbinfo.bBreakReplacementChar = '\0';
	//dcbinfo.bXONChar = 0x11; // note: xon/xoff protocol is not used.
	//dcbinfo.bXOFFChar = 0x13;
	DosDevIOCtl (comport,IOCTL_ASYNC,ASYNC_SETDCBINFO,&dcbinfo,
							sizeof (DCBINFO),0L,NULL,0L,NULL);

	// Allow tranmitting of data.
 	DosDevIOCtl (comport,IOCTL_ASYNC,ASYNC_STARTTRANSMIT,NULL,0L,NULL,NULL,0L,NULL);

}
/*************************  serial cleanup  ****************************/
/*
		 cleanup when done with the serial port.
 */
void SerialIO::serial_cleanup ()
{
	BYTE cmd;
	// flush input and output buffers.
	DosDevIOCtl (comport,IOCTL_GENERAL,DEV_FLUSHOUTPUT,(PVOID)&cmd,
			1L,NULL,NULL,0L,NULL);
	DosDevIOCtl (comport,IOCTL_GENERAL,DEV_FLUSHINPUT,(PVOID)&cmd,
			1L,NULL,NULL,0L,NULL);
	DosClose (comport);
}
/******************************  next  ***********************************/
/*
		Get a message from the serial port.
 */
int SerialIO::next ()
{
	clock_t tmp,starttime=time (&tmp),stoptime;
	rlen = 0;
	while (rlen == 0) {
		stoptime = time (&tmp);
		if (stoptime-starttime >= 3000) {
			strcpy (rbuf,"");
			return 0;
		}
		rlen = receive (rbuf);
	}
 //	if (rlen > 0) debug.post (rbuf);

	return rlen;
}
/******************************  next  ***********************************/
/*
		Get a message from the serial port.
 */int SerialIO::next (char **buff)
{
	clock_t tmp,starttime=time (&tmp),stoptime=time (&tmp);
	char data[256],thebuf[256];
	int thelen = 0;
	strcpy (*buff,"");
	strcpy (data,"");
	strcpy (thebuf,"");
int i;
	while (thelen == 0) {
//		if (thelen > 0) break;
	 /*
		if (thebuf[thelen-1] == '\r') break; //rlen == 0) {
		for (i=0;i<strlen(thebuf);i++) {
			if (thebuf[i] == '\r') { break; }
		}
 */

		stoptime = time (&tmp);
		if (stoptime-starttime >= 1000) {

			break;
		}

		thelen = receive (thebuf);
	//strcat (thebuf,data);
	}
	if (thelen > 0) debug.post (thebuf);

	for ( i=0;i<strlen(thebuf);i++) {
		if (thebuf[i] == '\r') {i++; break;}
	}

	//i = strlen(thebuf);
	strncpy (*buff,thebuf,i);
	(*buff)[i] = '\0';
	return i;
}
/*
int SerialIO::next (char **buff)
{
	clock_t tmp,starttime=time (&tmp),stoptime=time (&tmp);
	char data[256],thebuf[256];
	int thelen = 0;
	strcpy (*buff,"");
  strcpy (data,"");
	strcpy (thebuf,"");
int i;
	while (thelen != 0) {
		stoptime = time (&tmp);
		if (stoptime-starttime >= 3000) {
			break;
		}
		thelen = receive (thebuf);
	}
 //	if (thelen > 0) debug.post (thebuf);
	for ( i=0;i<strlen(thebuf);i++) {
		if (thebuf[i] == '\r') {i++; break;}
	}
	strncpy (*buff,thebuf,strlen(thebuf));
	(*buff)[strlen(thebuf)] = '\0';
	return i;
}
*/
/****************************  receive  ******************************/
/*
		 read the serial port...
 */
int SerialIO::receive ()
{
		strcpy (rbuf,"");
		DosRead (comport, rbuf, (ULONG)sizeof (rbuf), &rlen);
		if (rlen) {
			return 1;
		}
		else {
			strcpy (rbuf,"");
			rlen = 0;
			return 0;
		}
}
/****************************  receive  ******************************/
/*
		 read the serial port...
 */int SerialIO::receive (char *buff) {
		strcpy (rbuf,"");
		strcpy (buff,"");
		DosRead (comport, rbuf, (ULONG)sizeof (rbuf), &rlen);

		if (rlen) {
			strcpy (buff,rbuf);
			return rlen;
    }
		else {
			strcpy (rbuf,"");
			strcpy (buff,rbuf);
			rlen = 0;
			return rlen;
		}
	}

/********************************  send  *********************************/
/*
		send a message out the serial port.
 */
int SerialIO::send () {
	unsigned long len;
	slen = strlen (sbuf);
	DosWrite (comport,sbuf,slen,&len);
	return (int)len;
}
/********************************  send  *********************************/
/*
		send a message out the serial port.
 */
int SerialIO::send (char *buff) {
	unsigned long len;
	slen = strlen (buff);
	DosWrite (comport,buff,slen,&len);
	return (int) len;
}
// standard cerr routine...
ostream &operator << (ostream& os, SerialIO& p) {
		os <<"Serial IO::";
		for (int i=0;i<p.rlen;i++) {
			os <<(int)p.rbuf[i]<<" ";
		}
		return os;
	}