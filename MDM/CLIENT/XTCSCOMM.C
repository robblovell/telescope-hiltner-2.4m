/* TCS communication via serial link 08/31/92 MRM, JT */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/termio.h>
#include <sys/termios.h>
#include <sys/ttold.h>
#include <sys/file.h>
#include <sys/time.h>
#include <time.h>
#include "tcscomm.h"

static int message();
static int cosdec=0, ratecor=0;

int
tcs_open(port)
char *port;
{
	int	fd,i;
	struct termios tio;

	if ((fd = open(port, O_RDWR|O_NDELAY|O_NOCTTY)) == -1) {
		perror("open");
		fprintf(stderr,"Can't open TCS port %s\n", port);
		return -1;
	}

	i = 1;
	ioctl(fd, TIOCGSOFTCAR, &i);
	if (ioctl(fd, TIOCEXCL, &i) == -1) {
		perror("ioctl");
		fprintf(stderr, "Can't set exclusive mode\n");
		close(fd);
		return -1;
	}

	if (ioctl(fd, TCGETS, &tio) == -1) {
		perror("ioctl");
		fprintf(stderr, "Can't get tty info\n");
		close(fd);
		return -1;
	}

	if (fcntl(fd, F_SETFL, FNDELAY) == -1) {
		perror("fcntl");
		fprintf(stderr, "Can't set non-blocking I/O\n");
		close(fd);
		return -1;
	}

	tio.c_iflag &= ~(IMAXBEL|IXON|IXOFF|BRKINT);
	tio.c_iflag |= IGNBRK;
	tio.c_iflag |= IGNCR;
	tio.c_oflag &= ~(ONLCR|CRDLY);
	tio.c_oflag |= CR3;
	tio.c_cflag = (tio.c_cflag & ~(CRTSCTS|CBAUD|CIBAUD|CSIZE|PARENB)) | B9600 | CS8;
	tio.c_lflag &= ~(ECHO|ECHOCTL|ICANON|ISIG);
	tio.c_lflag |= NOFLSH;

	tio.c_cc[VMIN] = 0; tio.c_cc[VTIME] = 12;	/* Set 1 second read timeout */

#ifdef TCSDEBUG
	printf("Flags: i=%011o o=%011o c=%011o l=%011o\n",tio.c_iflag, tio.c_oflag, tio.c_cflag, tio.c_lflag);
#endif

	if (ioctl(fd, TCSETS, &tio) == -1) {
		perror("ioctl");
		fprintf(stderr, "Can't set tty info\n");
		close(fd);
		return -1;
	}
		
	return fd;
}

int
sendtotcs(fd_tcs, cmdto)
	int	fd_tcs;
	char	*cmdto;
{
	int	i,n,zer=0;

/*
	if (ioctl(fd_tcs, TCFLSH, TCOFLUSH) == -1) {
		perror("ioctl");
		message("Comm error: can't flush");
		return -1;
	}
*/

	n = strlen(cmdto);
	for (i=0; i<n; i++) {
		if (write(fd_tcs, &cmdto[i], 1) == -1) {
			message("Comm error: Serial link write failed");
			return(-1);
		}
		usleep(1000);
	}

#ifdef TCSDEBUG
	printf("Sent: <%s>\n (%1d)\n",cmdto, strlen(cmdto));
#endif

	return 0;
}

int
rcvfromtcs(fd_tcs, buf, max)
	int	fd_tcs, max;
	char	*buf;
{
	int	i,j;

	i = 0;
	while ((j = read(fd_tcs, buf+i, max-i)) > 0) {
		i += j;
		usleep(150000);
	}
		
	if (j == -1 && errno != EWOULDBLOCK) {
		message("Serial link read failed");
		return(-1);
	}

	buf[i] = '\0';
#ifdef TCSDEBUG
	printf("Got: <%s> (%1d)\n",buf,i);
#endif
	return i;
}

int
tcs_init(fd_tcs)
	int	fd_tcs;
{
	int	i;

	message("Clearing TCS link...");

/* 	ioctl(fd_tcs, TCSBRK, 0); */

	i = tcs_sync(fd_tcs, 3);
	if (i<0) {
		message("Trying again...");
		i = tcs_sync(fd_tcs, 3);
		if (i<0) message("TCS link clear failed");
	} else {
		message("TCS link OK");
	}
	return i;
}


static int
tcsnresp(fd)
	int	fd;
{
	message("TCS timed out, wait...");
	if (tcs_sync(fd, 3) != 0) {
		message("TCS timed out, try clear link");
	} else {
		message("TCS timed out, retry last command");
	}
	return TCS_ERR_NORESP;
}

/* What the TCS sends us if it's in main menu mode */
static char mmenu[9] = { 0x29, 0x3a, 0x1b, 0x54, 0x1b, 0x3d, 0x32, 0x4f, 0x00 };

/* What the TCS sends us when it's happy in terse mode */
static char tcsok[] = "TCSREADY";
/* What the TCS sends us when it's unhappy in terse mode */
static char tcserr[] = "TCSERROR";

int
tcs_sync(fd, ns)
	int	fd;
	int	ns;
{
	int	i,j,k,n;
	char	buf[512];

	i = 0; n = 0;
	for (k=0; k<3*ns; k++) {
	        if (ioctl(fd, TCFLSH, TCIOFLUSH) != 0) {
	                message("Comm error: can't flush x");
	                return -1;
	        }
		sendtotcs(fd, "\r");
		sleep(1);
		for (j=0; j<2; j++) {
			if ((i = rcvfromtcs(fd, &buf[n], 512-n)) > 0) {
				n += i;
				if (n>=8) break;
			}
			if (i<0) {
				message("TCS I/O Error (sync)");
				return i;
			}
			sleep(1);
		}
		if (n<8) continue;		/* Didn't get enough to work with */
		if (strncmp(mmenu, &buf[i-8], 8) == 0) {
			/* Ouch, we're at the verbose menu; switch to terse */
			sendtotcs(fd, "20\r");
			message("TCS in wrong state, reinitializing...");
			usleep(100000);
			k = 0;
			continue;
		}
		if (strncmp(tcsok,  &buf[n-8], 8) == 0 ||
		    strncmp(tcserr, &buf[n-8], 8) == 0) {
		    	/* Got it! */
			message("");
			return 0;
		}
	}

	message("TCS timed out: resync failed");
	return TCS_ERR_NORESP;
}

/* Wait for terse prompt back from TCS.  Returns negative on failure
 * (including timeout), 0 on success prompt (TCSREADY), 1 on bad
 * prompt (TCSERROR).  Timeout is currently 10 seconds.
 */
int
tcs_pwait(fd, nsec)
	int	fd, nsec;
{
	int	i,j,n;
	char	buf[512];

	n = 0;
	for (j=0; j<nsec && n < 512; j++) {
		if ((i = rcvfromtcs(fd, &buf[n], 512-n)) > 0) {
			n += i;
			if (n>=512) break;
			if (n<8) { sleep(1); continue; }
			if (strncmp(tcsok,  &buf[n-8], 8) == 0 ||
				strncmp(tcserr, &buf[n-8], 8) == 0 ||
				( n > 8 && (strncmp(tcsok, &buf[n-9], 8) == 0 ||
				strncmp(tcserr, &buf[n-9], 8) == 0))) break;
		}
		if (i<0) {
			message("TCS I/O Error (pwait)");
			return i;
		}
		sleep(1);
	}
	if (n<8) {
		message("TCS timed out");
		return TCS_ERR_NORESP;
	}
	usleep(TCS_WAITAFTER);			/* TCS too slow? */
	if (strncmp(tcsok,  &buf[n-8], 8) == 0 ||
		(n > 8 && strncmp(tcsok, &buf[n-9], 8) == 0))
			{ message(""); return 0;}
	if (strncmp(tcserr, &buf[n-8], 8) == 0 ||
		(n > 8 && strncmp(tcserr, &buf[n-9], 8) == 0))
			 { message("TCS Error on last command"); return 1;}
	message("TCS timed out");
	return TCS_ERR_NORESP;
}

static int	nbuf;
static char	expectbuf[512];

int
tcs_expect(fd, expstr, numsec)
	int fd,numsec;
	char *expstr;
{
	int	i,j,nexp;
	
	if (expstr == NULL || expstr[0]=='\0') return 0;
	nexp = strlen(expstr);
#ifdef TCSDEBUG
	printf("Expect: <%s> (%1d)\n", expstr, nexp);
#endif
	nbuf = 0;
	for (j=0; j<numsec; j++) {
		if ((i = rcvfromtcs(fd, expectbuf+nbuf, sizeof(expectbuf)-nbuf)) > 0) {
			nbuf += i;
			if (nbuf>=nexp && strncmp(&expectbuf[nbuf-nexp], expstr, nexp) == 0) 
				break;
		}
		if (i<0) {
			message("TCS I/O Error (expect)");
			return i;
		}
		usleep(500000);
	}
#ifdef TCSDEBUG
	printf("Found%c: <%s> (%1d)\n", j<numsec?'!':' ',expectbuf, nbuf);
#endif
	if (nbuf<nexp) return tcsnresp(fd);
	if (strncmp(&expectbuf[nbuf-nexp], expstr, nexp) != 0) {
		if (nbuf>=8 && strncmp(&expectbuf[nbuf-8], tcserr, 8) == 0) {
			message("TCS Error");
			usleep(TCS_WAITAFTER);			/* TCS too slow? */
			return TCS_ERR_TCSERR;
		}
		i = tcsnresp();
		
	} else {
		i = 0;
	}
	
	usleep(TCS_WAITAFTER);			/* TCS too slow? */
	return i;
}

/* Set dome position encoders */
int
tcs_setdome(fd, domepos)
	int fd;
	double domepos;
{
	int	i;
	char	str[256];
	
	if (domepos < 0.0 || domepos > 360.0) {
		message("Dome position out of range");
		return TCS_ERR_BADVAL;
	}
		
	message("");
	sendtotcs(fd, "2\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 8)) {
		if (i<0) return i;
		/* XXX */ message("No prompt");
	}
	sprintf(str, "%-1.1f\r", domepos);
	sendtotcs(fd, str);
	if (!(i = tcs_pwait(fd, 10))) {
		sprintf(str, "Dome azimuth set to %-5.1f", domepos);
		message(str);
		return 0;
	}
	return i;
}

/* Set instrument rotator encoders */
int
tcs_setrot(fd, rotpos)
	int fd;
	double rotpos;
{
	int	i;
	char	str[256];
	
	if (rotpos < 0.0 || rotpos > 360.0) {
		message("Rotator angle out of range");
		return TCS_ERR_BADVAL;
	}
		
	message("");
	sendtotcs(fd, "4\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 8)) {
		if (i<0) return i;
		/* XXX */ message("No prompt");
	}
	sprintf(str, "%-1.1f\r", rotpos);
	sendtotcs(fd, str);
	if (!(i = tcs_pwait(fd, 10))) {
		sprintf(str, "Rotator angle set to %-5.1f", rotpos);
		message(str);
		return 0;
	}
	return i;
}

/* Set guide/set/offset rate */
int
tcs_setrate(fd, which, rate)
	int fd, which;
	double rate;
{
	int	i;
	char	str[256];
	
	if (rate < 0 || rate >= 100 || (rate > 10 && which == TCS_GSO_GUIDE)) {
		message("Rate out of range");
		return TCS_ERR_BADVAL;
	}
		
	message("");
	switch (which) {
		case TCS_GSO_GUIDE:
			sendtotcs(fd, "12\r");
			break;
		case TCS_GSO_SET:
			sendtotcs(fd, "13\r");
			break;
		case TCS_GSO_OFFSET:
			sendtotcs(fd, "14\r");
			break;
		default:
			return -1;
	}
		
	if (i = tcs_expect(fd, TCS_PROMPT, 8)) {
		if (i<0) return i;
		/* XXX */ message("No prompt");
	}
	sprintf(str, "%-4.4f\r", rate);
	sendtotcs(fd, str);
	if (!(i = tcs_pwait(fd, 10))) {
		sprintf(str, "%s rate set to %-7.3f", (which==TCS_GSO_GUIDE?"Guide":
			(which==TCS_GSO_SET?"Set":"Offset")), rate);
		message(str);
		return 0;
	}
	return i;
}

/* Set focus encoder */
int
tcs_setfocus(fd, pos)
	int fd;
	double pos;
{
	int	i;
	char	str[256];
	
	if (pos < 0 || pos >= 10000.0) {
		message("Focus position out of range");
		return TCS_ERR_BADVAL;
	}
		
	message("");
	sendtotcs(fd, "5\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 8)) {
		if (i<0) return i;
	}
	sprintf(str, "%-1.1f\r", pos);
	sendtotcs(fd, str);
	if (!(i = tcs_pwait(fd, 10))) {
		sprintf(str, "Focus encoder set to %-6.1f", pos);
		message(str);
		return 0;
	}
	return i;
}

/* Set TCS display epoch */
int
tcs_setepoch(fd, epoch)
	int fd;
	double epoch;
{
	int	i;
	char	str[256];
	
	if ((epoch < 1700.0 || epoch > 2200.0) && (epoch != -1.0)) {
		message("Epoch out of range");
		return TCS_ERR_BADVAL;
	}
		
	message("");
	sendtotcs(fd, "17\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 8)) {
		if (i<0) return i;
	}
	sprintf(str, "%-3.3f\r", epoch);
	sendtotcs(fd, str);
	if (!(i = tcs_pwait(fd, 10))) {
		sprintf(str, "Display epoch set to %-8.3f", epoch);
		message(str);
		return 0;
	}
	return i;
}

/* Toggle/set/clear COSDEC */
int
tcs_cosdec(fd, what)
	int fd,what;
{
	int	i,val;
	char	str[256];
	
	if (what < -1 || what > 1) {
		return TCS_ERR_BADVAL;
	}
		
	message("");
	sendtotcs(fd, "15\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 8)) {
		if (i<0) return i;
	}
	val = 0;
	switch (what) {
		case -1: val = !cosdec;
			break;
		case 0:	val = 0;
			break;
		case 1:	val = 1;
			break;
	}
	sendtotcs(fd, val ? "1\r" : "0\r");
	if (!(i = tcs_pwait(fd, 10))) {
		sprintf(str, "COS DEC turned %s",val?"on":"off");
		message(str);
		cosdec = val;
		return 0;
	}
	return i;
}

/* Toggle/set/clear RATECOR */
int
tcs_ratecor(fd, what)
	int fd,what;
{
	int	i,val;
	char	str[256];
	
	if (what < -1 || what > 1) {
		return TCS_ERR_BADVAL;
	}
		
	message("");
	sendtotcs(fd, "16\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 8)) {
		if (i<0) return i;
	}
	val = 0;
	switch (what) {
		case -1: val = !ratecor;
			break;
		case 0:	val = 0;
			break;
		case 1:	val = 1;
			break;
	}
	sendtotcs(fd, val ? "1\r" : "0\r");
	if (!(i = tcs_pwait(fd, 10))) {
		sprintf(str, "RATE COR turned %s",val?"on":"off");
		message(str);
		ratecor = val;
		return 0;
	}
	return i;
}

/* Send next coords */
int
tcs_sendcoord(fd, rah, ram, ras, decsi, dech, decm, decs, epoch)
	int	fd, rah, ram, decsi, dech, decm;
	double	ras, decs, epoch;
{
	int	i,val,iras,irarem,idecs,idecrem;
	char	str[256];
	
	if ((epoch < 1700.0 || epoch > 2200.0) && (epoch != -1.0)) {
		message("Epoch out of range");
		return TCS_ERR_BADVAL;
	}
	ras += 0.05; decs += 0.05;
	iras = ras; irarem = (ras - iras)*10.0;
	idecs = decs; idecrem = (decs - idecs)*10.0;
/*	tcs_cancel(fd); */
	sprintf(str, "Sending   %02d %02d %4.1f   %c%02d %02d %4.1f   %7.2f",
		rah, ram, ras, decsi==1?'+':'-', dech, decm, decs, epoch);
	message(str);

	sendtotcs(fd, "6\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i<0) return i;
	}
	sprintf(str, "%02d %02d %02d.%1d\r", rah, ram, iras, irarem);
	sendtotcs(fd, str);
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i==TCS_ERR_TCSERR) message("TCS Error (Coordinate out of range)");
		if (i<0) return i;
	}
	sprintf(str, "%c%02d %02d %02d.%1d\r", decsi==1?'+':'-', dech, decm, idecs, idecrem);
	sendtotcs(fd, str);
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i==TCS_ERR_TCSERR) message("TCS Error (Coordinate out of range)");
		if (i<0) return i;
	}
	sprintf(str, "%-3.3f\r", epoch);
	sendtotcs(fd, str);
	if (!(i = tcs_pwait(fd, 10))) {
		message("Coordinates sent to TCS");
		return 0;
	}
	return i;
}

int
tcs_cancel(fd)
	int	fd;
{
	int	i;

	i = sendtotcs(fd, "\023");
	tcs_sync(fd, 1);
	if (i==0) message("Cancelled");
	return i;
}

int
tcs_stop(fd)
	int	fd;
{
	int	i;

	i = sendtotcs(fd, "\023");
	tcs_sync(fd, 1);
	message("Whoa! \007");
	if (i!=0) message("WARNING: Couldn't send stop!  Help!");
	return i;
}

int
tcs_go(fd)
	int	fd;
{
	int	i;

	i = sendtotcs(fd, "\007");
/*	if (i==0) message("Yo!"); */
	if (i==0) message("Telescope motion initiated.");
	return i;
}

int
tcs_offset(fd, raoff, decoff, epoch)
	int	fd;
	double	raoff, decoff, epoch;
{
	int	i;
	char	str[256];

	if ((epoch < 1700.0 || epoch > 2200.0) && (epoch != -1.0)) {
		message("Epoch out of range");
		return TCS_ERR_BADVAL;
	}

	if (raoff < -90*3600 || raoff > 90*3600) {
		message("RA offset out of range");
		return TCS_ERR_BADVAL;
	}

	if (decoff < -90*3600 || decoff > 90*3600) {
		message("Dec offset out of range");
		return TCS_ERR_BADVAL;
	}

	sprintf(str, "Sending offset %.2f %.2f arcsec,  %7.2f",
		raoff, decoff, epoch);
	message(str);
	sendtotcs(fd, "7\r");
	if ((i = tcs_expect(fd, TCS_PROMPT, 15))<0) return i;
	sprintf(str, "%-2.2f\r", raoff);
	sendtotcs(fd, str);
	if ((i = tcs_expect(fd, TCS_PROMPT, 15))<0) return i;
	sprintf(str, "%-2.2f\r", decoff);
	sendtotcs(fd, str);
	if ((i = tcs_expect(fd, TCS_PROMPT, 15))<0) return i;
	sprintf(str, "%-4.4f\r", epoch);
	sendtotcs(fd, str);

	if (!(i = tcs_pwait(fd, 10))) {
		message("Offset sent to TCS");
		return 0;
	}
	return i;
	
}


int
tcs_trail(fd, rate, length, pa)
	int	fd;
	double	rate, length, pa;
{
	int	i;
	char	str[256];

	if (rate < 0 || rate > 100) {
		message("Trail rate out of range");
		return TCS_ERR_BADVAL;
	}

	if (length < 0 || length > 90*3600) {
		message("Trail length out of range");
		return TCS_ERR_BADVAL;
	}

	if (pa < -360 || pa > 360) {
		message("Position angle out of range");
		return TCS_ERR_BADVAL;
	}

	sprintf(str,"Sending trail %.1f  %.1f  %.1f", rate, length, pa);
	message(str);
	sendtotcs(fd, "8\r");
	if ((i = tcs_expect(fd, TCS_PROMPT, 15))<0) return i;
	sprintf(str, "%-2.2f\r", rate);
	sendtotcs(fd, str);
	if ((i = tcs_expect(fd, TCS_PROMPT, 15))<0) return i;
	sprintf(str, "%-2.2f\r", length);
	sendtotcs(fd, str);
	if ((i = tcs_expect(fd, TCS_PROMPT, 15))<0) return i;
	sprintf(str, "%-1.1f\r", pa);
	sendtotcs(fd, str);

	if (!(i = tcs_pwait(fd, 10))) {
		message("TCS Trailing");
		return 0;
	}
	return i;
	
}

int
tcs_track(fd, rarate, decrate)
	int	fd;
	double	rarate, decrate;
{
	int	i;
	char	str[256];

	if (rarate < -25.0 || rarate > 25.0) {
		message("RA track rate out of range");
		return TCS_ERR_BADVAL;
	}

	if (decrate < -25.0 || decrate > 25.0) {
		message("Dec track rate out of range");
		return TCS_ERR_BADVAL;
	}

	sprintf(str,"Sending track rates %.4f %.4f",rarate, decrate);
	message(str);
	sendtotcs(fd, "11\r");
	if ((i = tcs_expect(fd, TCS_PROMPT, 15))<0) return i;
	sprintf(str, "%-4.4f\r", rarate);
	sendtotcs(fd, str);
	if ((i = tcs_expect(fd, TCS_PROMPT, 15))<0) return i;
	sprintf(str, "%-4.4f\r", decrate);
	sendtotcs(fd, str);

	if (!(i = tcs_pwait(fd, 10))) {
		message("Track rates set");
		return 0;
	}
	return i;
}

tcs_toggletrail(fd)
	int	fd;
{
	sendtotcs(fd, "\024");
	message("Trailing toggled");
	return 0;
}

/* Set RA/Dec encoders to position */
int
tcs_encode(fd, rah, ram, ras, decsi, dech, decm, decs, epoch)
	int	fd, rah, ram, decsi, dech, decm;
	double	ras, decs, epoch;
{
	int	i,val,iras,idecs,irarem,idecrem;
	char	str[256];
	
	if ((epoch < 1700.0 || epoch > 2200.0) && (epoch != -1.0)) {
		message("Epoch out of range");
		return TCS_ERR_BADVAL;
	}
	
	ras += 0.05; decs += 0.05;
	iras = ras; irarem = (ras - iras)*10.0;
	idecs = decs; idecrem = (decs - idecs)*10.0;
	message("Setting encoders...");
	sendtotcs(fd, "3\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i<0) return i;
	}
	sprintf(str, "%02d %02d %02d.%1d\r", rah, ram, iras, irarem);
	sendtotcs(fd, str);
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i==TCS_ERR_TCSERR) message("TCS Error (Coordinate out of range)");
		if (i<0) return i;
	}
	sprintf(str, "%c%02d %02d %02d.%1d\r", decsi==1?'+':'-', dech, decm, idecs, idecrem);
	sendtotcs(fd, str);
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i==TCS_ERR_TCSERR) message("TCS Error (Coordinate out of range)");
		if (i<0) return i;
	}
	sprintf(str, "%-3.3f\r", epoch);
	sendtotcs(fd, str);
	if (!(i = tcs_pwait(fd, 10))) {
		message("Telescope encoders set");
		return 0;
	}
	return i;
}

tcs_stow(fd)
	int	fd;
{
	int	i;

	message("Stowing telescope...");
	sendtotcs(fd, "10\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i<0) return i;
	}
	sendtotcs(fd, "1\r");
	if (!(i = tcs_pwait(fd, 300))) {
		message("Telescope stowed");
		return 0;
	}
}

int
tcs_setut(fd)
	int	fd;
{
	int	i;
	char	str[256];
	struct timeval tvnow;
	struct tm *tmnow;
	time_t	tmlater;

	message("Setting UT...");
	sendtotcs(fd, "1\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i<0) return i;
	}
	gettimeofday(&tvnow, NULL);
	tmlater = tvnow.tv_sec + 4;
	tmnow = gmtime(&tmlater);

	sprintf(str, "%1d\r", 1900+tmnow->tm_year);
	sendtotcs(fd, str);
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i<0) return i;
	}
	sprintf(str, "%1d\r", tmnow->tm_mon+1);
	sendtotcs(fd, str);
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i<0) return i;
	}
	sprintf(str, "%1d\r", tmnow->tm_mday);
	sendtotcs(fd, str);
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i<0) return i;
	}
	sprintf(str, "%02d %02d %02d\r", tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec);
	sendtotcs(fd, str);
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i<0) return i;
	}

#ifdef TCSDEBUG
	printf("UT is %04d/%02d/%02d %02d:%02d:%02d\n",1900+tmnow->tm_year,tmnow->tm_mon+1,
		tmnow->tm_mday,tmnow->tm_hour,tmnow->tm_min,tmnow->tm_sec);
#endif

	if (!(i = tcs_pwait(fd, 100))) {
		message("UT set on TCS");
		return 0;
	}
}

int
tcs_getstatus(fd, which, str)
	int	fd, which;
	char	*str;
{
	int	i,slen;
	char	snum[8];
	char	*p1,*p2;
	
	if (str==NULL) return -1;
	str[0]='\0';
	if (which < 1 || which > 7) return TCS_ERR_BADVAL;

	sendtotcs(fd, "19\r");
	if (i = tcs_expect(fd, TCS_PROMPT, 15)) {
		if (i<0) return i;
	}
	sprintf(snum, "%1d\r", which);
	sendtotcs(fd, snum);

	i = tcs_expect(fd, tcsok, 20);
	if (i != 0) return i;

	if ((p1 = strchr(expectbuf, '*')) == NULL) return -1;
	if ((p2 = strstr(expectbuf, tcsok))==NULL) return -1;
	p1++; if (p1>p2) return -1;
	slen = p2-p1;
	strncpy(str, p1, slen);
	str[slen] = '\0';

#ifdef TCSDEBUG
	printf("Status: <%s>\n", str);
#endif
	
	return slen;
}

tcs_toggledome(fd)
	int	fd;
{
	sendtotcs(fd, "\004");
	return 0;
}

int
tcs_kickdome(fd)
	int	fd;
{
	int	i,slen,stat;
	char	buf[512];
	double	domeaz,ex1,ex2,ex3,dstat;
	
	if ((slen = tcs_getstatus(fd, TCS_STAT_DOME, buf)) < 0) return slen;
	sscanf(buf, TCS_STF_DOME, &ex1, &domeaz, &ex2);
#ifdef TCSDEBUG
	printf("Dome at: %-5.1f\n", domeaz);
#endif

	if ((slen = tcs_getstatus(fd, TCS_STAT_STAT, buf)) < 0) return slen;
	sscanf(buf, TCS_STF_STAT, &dstat);
	stat = dstat+0.001;
#ifdef TCSDEBUG
	printf("Status: %#05x\n", stat);
	printf("Dome is currently %s\n",(stat&TCS_ST_SDOME)?"on":"off");
#endif

	tcs_setdome(fd, domeaz);
	if (!(stat&TCS_ST_SDOME)) {
		message("Kicking harder...");
		tcs_toggledome(fd);
		if ((slen = tcs_getstatus(fd, TCS_STAT_STAT, buf)) < 0) return slen;
		sscanf(buf, TCS_STF_STAT, &dstat);
		stat = dstat+0.001;
		if (!(stat&TCS_ST_SDOME)) {
			message("Flip the dome switch & kick again");
			return 1;
		}
	} else message("Ouch!");
	return 0;		
}

int
tcs_gettelpos(fd, ra, dec, epoch, rot, foc)
	int	fd;
	double	*ra, *dec, *epoch, *rot, *foc;
{
	int	i,slen,stat;
	char	buf[512];
	
	if ((slen = tcs_getstatus(fd, TCS_STAT_MPOS, buf)) < 0) return slen;
	if (sscanf(buf, TCS_STF_MPOS, ra, dec, epoch, foc, rot) != 5) {
		message("Comm error: can't get coordinates");
		return -1;
	}
	return 0;
}

int
tcs_gettelenc(fd, ha, den, rap, dep)
	int	fd;
	double	*ha, *den, *rap, *dep;
{
	int	i,slen,stat;
	char	buf[512];
	
	if ((slen = tcs_getstatus(fd, TCS_STAT_ENCOD, buf)) < 0) return slen;
/*	message(buf); */
	if (sscanf(buf, TCS_STF_ENCOD, ha, den, rap, dep) != 4) {
		message("Comm error: can't get encoders");
		return -1;
	}
	return 0;
}

