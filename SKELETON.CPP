/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include "hiltner.h"
#include "about.h"
#include "telescpe.h"
#include "display.h"

DT2819 board1 (0x230,OUTPUT,INPUT); // initialize boards...
DT2819 board2 (0x250,OUTPUT,INPUT);
DT2819 board3 (0x270,INPUT,INPUT);
DT2816 board4 (0x290);
Axis RA (&board1,&board4,0);
Axis DEC (&board2,&board4,1);
Dome dome (&board1,&board4);
PortBit drivechassisonoff  (&board1,0,5); // port 0 bit 5 drives on off.


Limits limit5 (&board1,1,0,5); // 5 degree limits (port 1 bits 0-3)
Limits limit8 (&board1,1,4,8); // 8 degree limits (port 1 bits 4-7)
Paddle paddle (&board3);
FrontPanel frontpanel(&board2,&board3);
AutoGuide autoguider(&board3);
Focus focus(&board2);
	Encoder dec_encoder (RES32,COUNTERRES32,&board2,&board2,1,3);
	Encoder ra_encoder (RES32,COUNTERRES32,&board1,&board1,1,3);
	Encoder focus_encoder (RES16,COUNTERRES16,&board2,&board3,5,4);
	Encoder dome_encoder (RES16,COUNTERRES16,&board1,&board3,5,3);
	Encoder inst_encoder (RES16,COUNTERRES16,&board3,&board3,1,2);

HAB   hab;
HWND  hWndFrame, 
      hWndClient;
CHAR  szTitle[64];
CHAR szFontFace[]="8.Helv";Display display (hWndClient,0,0,600,390,CLR_BLUE);

MRESULT EXPENTRY ClientWndProc (HWND,ULONG,MPARAM,MPARAM);


typedef struct blackboard {
	int value1,value2,value3;
} blackboard;
blackboard bb;

void TheThread ()
{
	while (1) {
		DosSleep (1000);
		bb.value1++;
	}
}
int main()
{
		TID ThreadID;
		HMQ   hmq;
		QMSG  qmsg;
		ULONG flFrameFlags    = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
														FCF_MINMAX   | FCF_SHELLPOSITION | FCF_TASKLIST |
														FCF_ICON | FCF_MENU;
		CHAR  szClientClass[] = "CLIENT";

		hab = WinInitialize (0);
		hmq = WinCreateMsgQueue (hab, 0);

		WinRegisterClass (hab, szClientClass, (PFNWP)ClientWndProc, CS_SIZEREDRAW, 0);
		WinLoadString (hab, 0L, ID_APPNAME, sizeof(szTitle), szTitle);

		hWndFrame = WinCreateStdWindow (HWND_DESKTOP, WS_VISIBLE,
				&flFrameFlags, szClientClass, szTitle, 0, 0, ID_APPNAME, &hWndClient);

		// start thread which updates the blackboard.
DosCreateThread (&ThreadID,(PFNTHREAD)TheThread,0,0x0001,0x2000);
DosResumeThread (ThreadID);
		// start timer...
		WinStartTimer (hab,hWndClient,1,100);
		//
		while (WinGetMsg (hab, &qmsg, 0, 0, 0))
				WinDispatchMsg (hab, &qmsg);

		WinDestroyWindow (hWndFrame);
		WinDestroyMsgQueue (hmq);
		WinTerminate (hab);
		return (0);
}
void APIENTRY process_menu (HWND hWnd, MPARAM mp1,MPARAM mp2) {
	switch (SHORT1FROMMP (mp1))
	{
								case IDM_ABOUT:
										DisplayAbout (hWnd, szTitle);
										break;


	}
}MRESULT EXPENTRY ClientWndProc (HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
		HPS     hps;
		BOOL    bHandled = TRUE;
		MRESULT mReturn  = 0;
		switch (msg)
		{
				case WM_PAINT:
							display.draw_display ();
						break;

				case WM_ERASEBACKGROUND:
							mReturn = MRFROMLONG(1L);
						break;
				case  WM_SIZE:

						break;
				case WM_TIMER:  // update the displayed values of the blackboard
							display.draw_display ();
						break;

				case WM_COMMAND:
							 process_menu (hWnd,mp1,mp2);
						break;
				default:
						bHandled = FALSE;
						break;
		}

		if (!bHandled)
				mReturn = WinDefWindowProc (hWnd,msg,mp1,mp2);

		return (mReturn);
}

