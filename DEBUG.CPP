#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include "debug.h"
void Debug::initialize (HWND hWnd,long x,long y,long xsize,long ysize){

	 hWndLocalList   = WinCreateWindow (hWnd, WC_LISTBOX, "",
			 WS_VISIBLE | LS_NOADJUSTPOS | LS_HORZSCROLL | WS_TABSTOP,
			 x, y, xsize, ysize, hWnd, HWND_BOTTOM, 3, NULL, NULL);


	 long lColor = CLR_DARKRED;
	 WinSetPresParam (hWndLocalList, PP_BACKGROUNDCOLORINDEX, sizeof(lColor), &lColor);
}
void Debug::post (char *astring)
{
	if (!pause) {
		++postlines;
		postlines %= 100;
		strcpy (postlist[postlines],astring);
	}
}
void Debug::displayposted ()
{
	if (!pause) {
		for (int i=0;i<postlines;i++) {
			dout (postlist[postlines]);
		}
		postlines=0;
	}
}
void Debug::dout (char *astring)
{
	if (!pause) {
		if (numlines > 100) {
			WinSendMsg (hWndLocalList,LM_DELETEITEM,(MPARAM)99L,0L);
			numlines--;
		}
		WinSendMsg (hWndLocalList,LM_INSERTITEM,0L,(MPARAM)astring);
		numlines++;
	}
}
void Debug::dout (char *astring,int anint)
{
	if (!pause) {
		char buf[100];
		sprintf (buf,"%s%d",astring,anint);
		if (numlines > 100) {
			WinSendMsg (hWndLocalList,LM_DELETEITEM,(MPARAM)99L,0L);
			numlines--;
		}
		WinSendMsg (hWndLocalList,LM_INSERTITEM,0L,(MPARAM)buf);
		numlines++;
	}
}
void Debug::dout (char *astring,double adouble)
{
	if (!pause) {
		char buf[100];
		sprintf (buf,"%s%4.2lf",astring,adouble);
		if (numlines > 100) {
			WinSendMsg (hWndLocalList,LM_DELETEITEM,(MPARAM)99L,0L);
			numlines--;
		}
		WinSendMsg (hWndLocalList,LM_INSERTITEM,0L,(MPARAM)buf);
		numlines++;
	}
}

