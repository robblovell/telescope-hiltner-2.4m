/* -------------------------------------------------------------------
								Hiltner 2.4 Meter Telescope Control System
																	by

														Robb E. Lovell

------------------------------------------------------------------- */
#ifndef hiltnerhh
#define hiltnerhh
#define INCL_DOSDATETIME
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>


#include "telescpe.h"
#include "about.h"
#include "display.h"/* Dialog and Control Identifiers     */
//#include <time.h>
#include "debug.h"
extern Debug debug;
// Screen display and OS/2 variables.
extern int debug1,debug2,debug3,debug4;
extern HAB   hab;extern HWND  hWndFrame,
			hWndClient;
extern CHAR  szTitle[64];
extern CHAR szFontFace[];
// Low level hardware..
extern DT2819 board1;
extern DT2819 board2;
extern DT2819 board3;
extern DT2816 board4;
// Movement variables.
extern Axis RA;
extern Axis DEC;
extern Dome dome;
extern Control drivechassisonoff; // port 0 bit 5 drives on off.
extern Limits limit5; // 5 degree limits (port 1 bits 0-3)
extern Limits limit8; // 8 degree limits (port 1 bits 4-7)
extern Paddle paddle;
extern FrontPanel frontpanel;
extern AutoGuide guider;
extern Focus focus;
extern Encoder dec_encoder;
extern Encoder ra_encoder;
extern Encoder focus_encoder;
extern Continuous_Encoder dome_encoder;
extern Encoder inst_encoder ;
// moveflag defines whether the program is executing a point to point move
// or waiting for a manual paddle move.
extern int moveflag,trailflag,trackingflag;
extern double HA_target,DEC_target;
extern int domeflag;
extern int stopflag;
extern int autoguideflag;
// variables used to take care of cosdec rate corrections.
extern int cosdecflag;
extern double cosdec,lastcosdec;
extern double setrate,guiderate;
// rate corrections....
extern int ratecorflag;// date and time variables.
extern struct date_time univtime,local_time,d_univtime,d_local_time;
extern struct date_time siderealtime,d_siderealtime;

extern double juliandate;
extern double siderealhours;
extern double airmass,targetairmass;
extern double targetparallactic,teleparallactic,dometargetparallactic;
extern double dometargetaltitude,dometargetazimuth,targetaltitude,targetazimuth,telealtitude,teleazimuth,domeazimuth,azimutherror;
extern double deadband,targetband;
extern int autodomeflag,domehomeflag;
extern int nomodel;
extern double trailrate,traillength,trailposangle;
extern double domeerror;// current epoch is the current time.
// standard epoch is the epoch of coordinates used by the astronomer.
extern double currentepoch,stdepoch,targetepoch;//=2000.0;// current location of the telescope.
extern double ramean,decmean,rameantarget,decmeantarget;
extern struct coord meanra,meandec,targetha,targetdec,targetramean,targetdecmean,teleha,teledec,apparentra,apparentdec;

extern double hatele,dectele,hatarget,dectarget; //
extern double haarcsec,decarcsec;  //
extern double haraw,decraw,harawtarget,decrawtarget;  //
// semaphores.
extern HMTX  setencoders_mex_sem,steppermotor_mex_sem;
extern char meanrasign,meandecsign,targetdecmeansign,targetrameansign,teledecsign,targethasign,targetdecsign,telehasign;
extern int all_run;

#endif
