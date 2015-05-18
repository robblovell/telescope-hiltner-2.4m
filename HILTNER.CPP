/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSDATETIME
#include <time.h>
#include <stdio.h>
#include "hiltner.h"
#include "const.h"
#include "timeuniv.h"
#include "debug.h"
#include "hooks.h"
#include "xtcslink.h"
#include "spdstat.h"
char *Autodomeon="Auto Dome On";
char *Autodomeoff="Auto Dome Off";

/*
		The low level hardware classes are used to manipulate the Data
	Translation boards directly through memory.  Three Counter timer
	boards and one digital to analog boards are used.  Boards 1-3 are the
	counter timers and board 4 is the D/A board.
 */
DT2819 board1 (0x230,OUTPUT,INPUT); // initialize boards...
DT2819 board2 (0x250,OUTPUT,INPUT);
DT2819 board3 (0x270,INPUT,INPUT);
DT2816 board4 (0x290);
DT2819 board5 (0x210,INPUT,INPUT);

/*
		The encoders for the telescope are read through the counter timer boards.
	The DEC and RA encoders use two counters cascaded together to make a 32
	bit counter.  The other encoders only require only one 16 bit counter.
 */
Encoder dec_encoder (RES32,COUNTERRES32,&board1,&board3,4,4,
										 DECRAWTOARCSEC,0);
Encoder ra_encoder (RES32,COUNTERRES32,&board2,&board2,2,4,
										 RARAWTOARCSEC,0);
Encoder focus_encoder (RES16,COUNTERRES16,&board1,&board3,3,3,
										 FOCUSRAWTOARCSEC,1);
Continuous_Encoder dome_encoder
	 (360.0*3600.0/DOMERAWTOARCSEC,COUNTERRES16,&board1,&board1,1,2,
											DOMERAWTOARCSEC);
Encoder inst_encoder (RES16,COUNTERRES16,&board3,
										&board3,1,2,INSTRAWTOARCSEC,0);

/*
		 The "Axis" class is used as an encapsilation of the movement routines
	for the DEC and RA motor assemblies.  (RA and DEC have identical hardware
	configurations.)  The axis class consists of objects which control or
	read a dc motor, a steppermotor, the clutch, an encoder, and two limit
	switches.  One slight monkey wrench into the whole thing is that the
	RA stepper motor is controlled from a counter and the DEC stepper motor
	is controlled from a frequency out signal.
 */
Axis RA (&board1,&board4,0,1000.0,&board1,3,2,7,6,5.0,9.0,&board2,1,
					15.002,1.0,RASTEPSIZE,                        // slew speeds in volts...
				 -HARAWLIMIT*HOURSTOARCSEC,HARAWLIMIT*HOURSTOARCSEC,-1,"RA",
				 5.0,6.0,8.0,9.0,2.0,60.0);
Axis DEC (&board2,&board4,1,2000.0,&board1,0,1,4,5,2.0,8.0,&board5,1,
					0.0,1.0,DECSTEPSIZE,
					-50.0*DEGREESTOARCSEC,110.0*DEGREESTOARCSEC,1,"DEC",
					2.0,3.0,4.0,8.0,2.0,60.0);
// Dome motion routines are contained in the Dome class.
Dome dome (&board1,&board3,&board4,&dome_encoder);
// focus motion routines are in the focus class.
Focus focus(&board2,&focus_encoder);// Drive chassis on/off is used to turn the drives on and off.
Control drivechassisonoff  (&board1,0,5,ACTIVEHIGH); // port 0 bit 5 drives on off.
//
// **** Limit switches are now contained within the Axis class.
//Limits limit5 (&board1,1,0); // 5 degree limits (port 1 bits 0-3)
//Limits limit8 (&board1,1,2); // 8 degree limits (port 1 bits 4-7)
//
// Code to read the hand paddle is contained in the paddle class.
Paddle paddle (&board3);
// Front panel input and output.
FrontPanel frontpanel(&board2,&board3);
// Auto guider
AutoGuide guider(&board3);



int pause;
/*
	 The variables below are used to keep track of the OS/2 window which
	displays the status of the telescope.
 */
HAB   hab;
HWND  hWndFrame,
			hWndClient,
			hWndDisplay;
/*
			hWndDlg is the dialog which displays the status of the telescope.
	See the file "hiltner.rc" for the layout of the dialog.  The Resources
	editor can be used read this file and to modify its contents.  Modifying
	this file will change the appearance of the status window.
 */
HWND  hWndDlg;  // the main status dialog.
CHAR  szTitle[64];
CHAR szFontFace[]="8.Helv";
// The display class is used to draw the status dialog.
Display display (hWndClient,0,0,610,400,CLR_BLUE);
//  This function handles messages from the operating system for the
//	main window.
MRESULT EXPENTRY ClientWndProc (HWND,ULONG,MPARAM,MPARAM);
// debug information window.
Debug debug;
/*
		 The next series of functions represent various parallel threads that
	are executed in order to control the telescope.
 */
extern void FrontPanelProcess ();

extern void PaddleGuiderProcess ();
extern void FocusProcess ();
extern void focus_absolute_move ();
extern void DomeProcess ();
extern void MoveProcess ();
extern void trail_process (); //ProcessData *data);
extern void MoveAxisProcess (ProcessData *data);
extern void trackingprocess (ProcessData *data);
extern void GuiderAxisProcess (ProcessData *data);
extern void TimeProcess ();
extern void get_time ();
extern void xtcslink_Process ();
extern void initialize_tcs ();
extern void set_trail (int &length,char xtcsbuf[],double &ra,
							 double &dec, double &rarate,double &decrate,
							 double &rate,double &tlength,double &posangle);
SpeedStatus rastatus,decstatus;
// Two axis processes are run to control the RA and DEC.  The "AxisProcess"
// procedure takes data which defines which axis is being controlled.  That
// data is defined below.  Keep in mind that the RA and DEC are controlled
// with the same code, only the data is different (i.e. what the process is
// controlling.
extern void AxisProcess (ProcessData *data);
ProcessData RAdata (RA,paddle.W,paddle.E,guider.W,guider.E,ra_encoder,rastatus);
ProcessData DECdata (DEC,paddle.N,paddle.S,guider.N,guider.S,dec_encoder,decstatus);
// moveflag defines whether the program is executing a point to point move
// or waiting for a manual paddle move.
int moveflag = 0,trailflag = 0,trackingflag = 0;

// the target location
double HA_target,DEC_target;
// dome flag defines the direction of motion of the dome.
// used for commands from the menu... will be taken out?
int domeflag = 0;
// if stop flag becomes true, the system stops all motors.
int stopflag = 0;
// variables used to take care of cosdec rate corrections.
int cosdecflag = 0;
double cosdec,lastcosdec;
double setrate=DEFAULTSETSPEED,guiderate=DEFAULTGUIDESPEED;
int ratecorflag = 0;

// time of day and other time formats.
// date_time structure is declared in skycalc.h
struct date_time univtime,local_time,d_univtime,d_local_time;
struct date_time siderealtime,d_siderealtime;
double juliandate;
double siderealhours;
double airmass,targetairmass;
double dometargetaltitude,dometargetazimuth,targetaltitude,targetazimuth,telealtitude,teleazimuth,domeazimuth,azimutherror;
double targetparallactic,teleparallactic,dometargetparallactic;
double deadband,targetband;
double haratecorr,decratecorr;
// some flags which notify processes when things are supposed to be active.
int autodomeflag=0,domehomeflag=0;
int autoguideflag=0;
int nomodel = 0;
int paintall = 0;
double domeerror;
// current epoch is the current time.
// standard epoch is the epoch of coordinates used by the astronomer.
double currentepoch,stdepoch,targetepoch;
// current location of the telescope.
double ramean,decmean,rameantarget,decmeantarget; // catalog locations.
struct coord meanra,meandec,targetha,targetdec,targetramean,targetdecmean,teleha,teledec,apparentra,apparentdec;

double hatele,dectele,hatarget,dectarget; // ra 0 at zenith,  dec 0 at north star.
double haarcsec,decarcsec;  //
double haraw,decraw,harawtarget,decrawtarget;  // raw locations.
extern void invalidatemove ()  ;
double trailrate=20.0,traillength=120.0,trailposangle=180.0;
char meanrasign,meandecsign,targetdecmeansign,targetrameansign,teledecsign,targethasign,targetdecsign,telehasign;
extern char **raspeedstatus;
extern char **decspeedstatus;
int all_run = 1;
extern void trail_init ();
// Semaphores....
HMTX  setencoders_mex_sem,steppermotor_mex_sem;
 /*********************************  all stop  **************************/
// used when I wasn't sure the program would work....
// emergency stop..
void all_stop ()
{
	char buf[256]; sprintf (buf,"\r\nSTOP ALL MOTORS........................");
	debug.post (buf); debug.dout (buf);
 //	domeflag = 0;
	stopflag = 1;
/*
	DosSuspendThread (RAThreadID);

	DosSuspendThread (DECThreadID);
*/
/*
DosSuspendThread (FocusThreadID);
	DosSuspendThread (DomeThreadID);
 */
	 // make doubley sure.
	dome.stop ();
	RA.stop ();
	DEC.stop ();
	focus.stop ();
 //	DosSleep (500);
}

/****************************  initialize tcs  ***************************/
/*
		 The tcs is initialized with either the last position of the telescope,
	 found in the file "encoders.tcs" or to the zenith position.  A target of
	 zenith is set in case a go is given without a location.
 */
void initialize_tcs ()
{
	 double rapos,decpos,focuspos,domepos,instpos;

	 read_telescope_model (IH,ID,PDD,NP, MA,ME, CH, PDD2);
	 get_time ();
	 stdepoch = targetepoch = currentepoch;
	 if (restore_last_position (haraw,decraw,domepos,focuspos,instpos)) {
		 set_encoderpositions ();
		 dome.encoder->set_to_arcsec_location (domepos);
		 focus.encoder->set_to_raw_location (focuspos);
		 inst_encoder.set_to_arcsec_location (instpos);
	 }
	 else {
		 dectele = MDMLATITUDE; //31.95133
		 hatele = 0.0;
		 telescope_coordinates_to_raw_coordinates_set_encoders (
									 hatele,dectele,haraw,decraw);	// decmean = MDMLATITUDE;
		 set_encoderpositions ();

		 domeazimuth = 320.0;
		 dome.encoder->set_to_arcsec_location (domeazimuth*DEGREESTOARCSEC);

		 focus.encoder->set_to_raw_location (5000.0);
		 inst_encoder.set_to_arcsec_location (0.0);
	 }
	 decraw = dec_encoder.read_arcsec_location ();
	 haraw = ra_encoder.read_arcsec_location ();

	 raw_coordinates_to_telescope_coordinates (haraw,decraw,hatele,dectele);

	 telescope_coordinates_to_mean_place (hatele,dectele,
								siderealhours,stdepoch,ramean,decmean,1,0);

	 // set a target location.
	 dectarget = MDMLATITUDE;
	 hatarget = 0.0;
	 telescope_coordinates_to_mean_place (hatarget,dectarget,
								siderealhours,stdepoch,rameantarget,decmeantarget,1,0);
	 targetaltitude = altit(dectarget,hatarget,MDMLATITUDE,&targetazimuth,&targetparallactic);

	 figure_trail_directions (trailrate,traillength,trailposangle,
					RAdata.axis->traillength,DECdata.axis->traillength,
					RAdata.axis->trailspeed,DECdata.axis->trailspeed);
	 // initialize the trailing data.

	trailrate =0.0; traillength =0.0; trailposangle = 0.0;
	figure_trail_directions (trailrate,traillength,trailposangle,
					RAdata.axis->traillength,DECdata.axis->traillength,
					RAdata.axis->trailspeed,DECdata.axis->trailspeed);}


/*****************************  main   *****************************/
/*
			The main function handles the startup of all of the concurrent
	 processes and then drops into the normal PM manager handling.  Note
	 that the dialog paint messages and menu commands are handled by the
	 procedures called from the PM manager.
 */
int main()
{

		TID FPThreadID,PGThreadID;
		TID MoveRAThreadID,MoveDECThreadID,RAThreadID,DECThreadID;
		TID GuideRAThreadID,GuideDECThreadID;
		TID DomeThreadID,FocusThreadID,AutoFocusThreadID,LinkThreadID,TimeThreadID;
		TID TrailThreadID;//,TrailDECThreadID;
		TID TrackingRAThreadID,TrackingDECThreadID;
		HMQ   hmq;
		QMSG  qmsg;

		ULONG flFrameFlags    = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
														FCF_MINMAX   | FCF_SHELLPOSITION | FCF_TASKLIST |
														FCF_ICON | FCF_MENU | FCF_NOBYTEALIGN;
		CHAR  szClientClass[] = "CLIENT";
		DosSetPriority (PRTYS_PROCESSTREE,PRTYC_TIMECRITICAL,15,0);
		all_run = 1;  // variable used to terminate processes at quit time.
		// standard OS/2 initializations.
		hab = WinInitialize (0);
		hmq = WinCreateMsgQueue (hab, 0);

		WinRegisterClass (hab,szClientClass,(PFNWP)ClientWndProc,CS_SIZEREDRAW, 0);
		WinLoadString (hab, 0L, ID_APPNAME, sizeof(szTitle), szTitle);

		hWndFrame = WinCreateStdWindow (HWND_DESKTOP, 0,
				&flFrameFlags,szClientClass,szTitle,0,0,ID_APPNAME,&hWndClient);
			// maximize the display window.
		WinSetWindowPos (hWndFrame,0,0L,0L,0L,0L,SWP_MAXIMIZE | SWP_SHOW);
		debug.initialize (hWndClient,10L,10L,619L,400L);
		// Create Semaphores for protection of access to shared resources.
		// mutual exclusiont semaphores...
		//sprintf (sem,"%dstepexclude",semix++);
		/*
				 Semaphores for coordination of resources and processes.
		 */
		DosCreateMutexSem ("stepper exclude",&steppermotor_mex_sem,
												DC_SEM_SHARED,FALSE);
		DosCreateMutexSem ("setencoders exclude",&setencoders_mex_sem,
												DC_SEM_SHARED,FALSE);
		trail_init (); // trailing process semaphore...
		/*
				 Start the 17 concurrent processes which control the telescope.
		*/
		DosCreateThread (&FPThreadID,(PFNTHREAD)FrontPanelProcess,0,0x0001,0x2000);
		DosResumeThread (FPThreadID);
		DosCreateThread (&PGThreadID,(PFNTHREAD)PaddleGuiderProcess,0,0x0001,0x2000);
		DosResumeThread (PGThreadID);		DosSleep (100);
		/*
				 First call to get time and then initialize the telescope.
		 */
		get_time ();
		stdepoch = currentepoch;
		initialize_tcs ();

		// The time process is the fastest loop occuring every 2 ms.  This
		// process calculates the time, telescope position statistics, and
		// dome statistics. Encoders are read in this process loop.
		DosCreateThread (&TimeThreadID,(PFNTHREAD)TimeProcess,0,0x0001,0x2000);
		DosResumeThread (TimeThreadID);

		// connect to xtcs workstation.
		xtcs_connect ();
		DosCreateThread (&LinkThreadID,(PFNTHREAD)xtcslink_Process,0,0x0001,0x2000);
		DosResumeThread (LinkThreadID);

		// start movement routines.     paddle first...
		DosCreateThread (&RAThreadID,(PFNTHREAD)AxisProcess,(ULONG)&RAdata,0x0001,0x2000);
		DosResumeThread (RAThreadID);

		DosCreateThread (&DECThreadID,(PFNTHREAD)AxisProcess,(ULONG)&DECdata,0x0001,0x2000);
		DosResumeThread (DECThreadID);

		// point to point moves.
		DosCreateThread (&MoveRAThreadID,(PFNTHREAD)MoveAxisProcess,(ULONG)&RAdata,0x0001,0x2000);
		DosResumeThread (MoveRAThreadID);

		DosCreateThread (&MoveDECThreadID,(PFNTHREAD)MoveAxisProcess,(ULONG)&DECdata,0x0001,0x2000);
		DosResumeThread (MoveDECThreadID);

		// the auto guider...
		DosCreateThread (&GuideRAThreadID,(PFNTHREAD)GuiderAxisProcess,(ULONG)&RAdata,0x0001,0x2000);
		DosResumeThread (GuideRAThreadID);

		DosCreateThread (&GuideDECThreadID,(PFNTHREAD)GuiderAxisProcess,(ULONG)&DECdata,0x0001,0x2000);
		DosResumeThread (GuideDECThreadID);
		// trailing...
		DosCreateThread (&TrailThreadID,(PFNTHREAD)trail_process,0,0x0001,0x2000);
		DosResumeThread (TrailThreadID);

 //		DosCreateThread (&TrailDECThreadID,(PFNTHREAD)trail_process,(ULONG)&DECdata,0x0001,0x2000);
 //		DosResumeThread (TrailDECThreadID);
		// tracking...
		DosCreateThread (&TrackingRAThreadID,(PFNTHREAD)trackingprocess,(ULONG)&RAdata,0x0001,0x2000);
		DosResumeThread (TrackingRAThreadID);

		DosCreateThread (&TrackingDECThreadID,(PFNTHREAD)trackingprocess,(ULONG)&DECdata,0x0001,0x2000);
	  DosResumeThread (TrackingDECThreadID);

		// focus...
		DosCreateThread (&FocusThreadID,(PFNTHREAD)FocusProcess,0,0x0001,0x2000);
		DosResumeThread (FocusThreadID);
		// focus...
		DosCreateThread (&AutoFocusThreadID,(PFNTHREAD)focus_absolute_move,0,0x0001,0x2000);
		DosResumeThread (AutoFocusThreadID);

		// dome...
		DosCreateThread (&DomeThreadID,(PFNTHREAD)DomeProcess,0,0x0001,0x2000);
		DosResumeThread (DomeThreadID);


		// Display dialog...
		/*
				A timer is used which allows the main thread to  update the display.
		 */
		// start timer...
		WinStartTimer (hab,hWndClient,1,200);// display loop delay is 200 ms

		// create main status dialog.
		display.create_dialog (hWndFrame);
		// make the dialog visible and the top process.
		WinSetWindowPos (display.ddialog,0,2L,2L,0L,0L,/*SWP_MAXIMIZE |*/ SWP_MOVE);
		WinShowWindow (display.ddialog, TRUE);
		invalidatemove (); // no point to point moves specified yet...
//}
//void mainloop ()
//{
//		HMQ   hmq;
//		QMSG  qmsg;		// Wait for message from the operating system and from the timer.
		// when a message is received by this program, the procedure below
		// called "ClientWndProc" is called.
		while (WinGetMsg (hab, &qmsg, 0, 0, 0))
				WinDispatchMsg (hab, &qmsg);

		WinDestroyWindow (hWndFrame);
		WinDestroyMsgQueue (hmq);
		WinTerminate (hab);
		return (0);
}
/****************************  process menu  ****************************/
/*
		This function handles menu selections
 */
extern int dialogstate;

void APIENTRY process_menu (HWND hWnd, MPARAM mp1,MPARAM ) {
	switch (SHORT1FROMMP (mp1))
	{
		case 1004: nomodel = 0; break;
		case 1005: nomodel = 1; break;
		case IDM_ABOUT:
				DisplayAbout (hWnd, (PSZ)szTitle);
			break;
		case IDM_PAUSE:
				debug.pause = debug.pause == 0;
			break;
		case 1000:
				dialogstate = 0;
				WinShowWindow (display.ddialog, TRUE);
			break;
		case 1001:
				dialogstate = 1;
				WinShowWindow (display.ddialog, FALSE);
			break;		case 1002:
				dialogstate = dialogstate==0;
				if (dialogstate)
					WinShowWindow (display.ddialog, FALSE);
				else
					WinShowWindow (display.ddialog, TRUE);

			break;
			}
}
/***************************  char sign  ****************************/
/*
		 returns the sign as a character given a -1, or +1 value.
 */
char signaschar[3]={'-',' ',' '};
char charsign(short sign) {
	return signaschar[abs(sign+1)%3];

}

/*************************  Client Wnd Proc  *************************/
/*
		This function handles messages from the operating system for the
	main window.
 */
MRESULT EXPENTRY ClientWndProc (HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
		HPS     hps;
		BOOL    bHandled = TRUE;
		MRESULT mReturn  = 0;
		//static int firsttime = 0;
		unsigned int value;
		switch (msg)
		{
				case WM_PAINT:
							display.paint_display (hWnd);
						break;
				case WM_ERASEBACKGROUND:
							mReturn = MRFROMLONG(1L);
						break;
				case  WM_SIZE:
						break;
				case WM_TIMER: // handles display functions.
							//clock_t starttime,stoptime;
							char buf[256];
	 //starttime = clock ();
// need semafore here....
							// tidy up display variables, correct for 24 hour and 12 hour
							// clocks, get rid of 60.0 as a second or minute etc...
							dec_to_bab (ramean,&meanra);
							correct_24_hours (&meanra);
							round_coord(&meanra,&meanra,3);

							dec_to_bab (decmean,&meandec);
							correct_24_degrees (&meandec);
							round_coord(&meandec,&meandec,2);

							dec_to_bab (rameantarget,&targetramean);
							correct_bab_hours (&targetramean);
							round_coord(&targetramean,&targetramean,3);

							dec_to_bab (decmeantarget,&targetdecmean);
							correct_bab_degrees (&targetdecmean);
							round_coord(&targetdecmean,&targetdecmean,2);

							dec_to_bab (hatarget,&targetha);
							correct_bab_hours (&targetha);
							round_coord(&targetha,&targetha,3);

							dec_to_bab (dectarget,&targetdec);
							correct_bab_degrees (&targetdec);
							round_coord(&targetdec,&targetdec,2);

							dec_to_bab (hatele,&teleha);
							correct_24_hours (&teleha);
							round_coord(&teleha,&teleha,3);

							dec_to_bab (dectele,&teledec);
							correct_bab_degrees (&teledec);
							round_coord(&teledec,&teledec,2);

							dec_to_bab (haraw,&apparentra);
							correct_bab_hours (&apparentra);
							round_coord(&apparentra,&apparentra,3);

							dec_to_bab (decraw,&apparentdec);
							correct_bab_degrees (&apparentdec);
							round_coord(&apparentdec,&apparentdec,2);

							d_siderealtime.h = siderealtime.h;
							d_siderealtime.mn = siderealtime.mn;
							d_siderealtime.s = siderealtime.s;
							correct_24_time (&d_siderealtime);
							d_univtime.h = univtime.h;
							d_univtime.mn = univtime.mn;
							d_univtime.s = univtime.s;
							correct_24_time (&d_univtime);
							d_local_time.h = local_time.h;
							d_local_time.mn = local_time.mn;
							d_local_time.s = local_time.s;



							correct_ha_time (&d_local_time);


							meanrasign=charsign (meanra.sign);
							meandecsign=charsign (meandec.sign);

							targetrameansign=charsign (targetramean.sign);
							targetdecmeansign=charsign (targetdecmean.sign);
							telehasign=charsign (teleha.sign);
							teledecsign=charsign (teledec.sign);
							targethasign=charsign (targetha.sign);
							targetdecsign=charsign (targetdec.sign);

							debug.displayposted ();

							// update the displayed values of the blackboard
							if (paintall) {
								display.paint_display (hWnd);
								paintall = 0;
							}
							else
								display.timed_display (hWnd);
//sprintf (buf,"Epoch:%4.2lf RA mean:%4.2lf,app:%4.2lf DEC mean:%4.2lf,app:%4.2lf",
//						 currentepoch,ramean,hatele,decmean,dectele);
//debug.dout (buf);
		//stoptime = clock ();
		//sprintf (buf,"display time:%2.2lf start %lf stop %lf",(double)(stoptime-starttime)/CLK_TCK,(double) starttime,(double)stoptime);
		//debug.dout (buf); ///"time:",(double)(stoptime-starttime)/CLK_TCK);



						break;

				case WM_COMMAND: // The menu.
							 process_menu (hWnd,mp1,mp2);
						break;
				case WM_CLOSE:  // program quit.
							 all_stop ();
							 //drivechassisonoff.turn_off (); // turn off drives.
							 save_last_position ();
							 all_run = 0;
							 DosSleep (1000);
							 drivechassisonoff.turn_off ();
				default:
						bHandled = FALSE;
						break;
		}

		if (!bHandled)
				mReturn = WinDefWindowProc (hWnd,msg,mp1,mp2);

		return (mReturn);
}


