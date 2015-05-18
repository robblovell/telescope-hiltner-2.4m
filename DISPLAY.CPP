/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include "telescpe.h"
#include "about.h"
#include "hiltner.h"
#include "display.h"
int dialogstate;
extern "C" {
	// main handler for dialogs.
	MRESULT EXPENTRY DisplayDlgProc (HWND hWnd,ULONG msg, MPARAM mp1,MPARAM mp2)
	{
		MRESULT mreturn = 0L;
		BOOL bhandled = TRUE;
		//char buf[256];
		//sprintf (buf,"dialog proc  %d",msg);
		//debug.post (buf);
		switch (msg) {
			case WM_INITDLG:
				 //WinSetFocus (HWND_DESKTOP, WinWindowFromID (hWnd, IDC_WININFO));

				 dialogstate = 1;
				break;
			case WM_COMMAND:
			case WM_CONTROL:
//sprintf (buf,"dialog proc cntrl;...............");
//debug.post (buf);
			break;
			case WM_DESTROY:
			case WM_CLOSE :
//sprintf (buf,"dialog proc end;...............");
//debug.post (buf);				dialogstate = 0;
//	 WinDestroyWindow (WinQueryWindow (hWnd,QW_PARENT));
				 break;

			default: bhandled = FALSE;
				break;
		 }
		 if (!bhandled)
			 mreturn = WinDefDlgProc (hWnd,msg,mp1,mp2);
		 return mreturn;
	}
}   // extern "C"
/**************************  Value class functions ***********************/
/**********************************  set  ********************************/
/*
		 Used to initialize the display of a variable to the screen.  Set
	 assigns how the variable is displayed by recording a format statement,
	 a type and a dialog item.  The dialog item determines where on the
	 screen the value is displayed.
 */
	void Value::set (void *val, int d,char *form,int type_) {
	 value = val; ditl = d;  strcpy (format,form); type = type_;
	 valid = 1;  lastvalid = '!';
	 if (lastvalue == NULL)
		 switch (type) {
				case VINTEGER:
						lastvalue = (int*) new (int);
					break;
				case VULONG:
						lastvalue = (unsigned long*) new (unsigned long);
					break;				case VSHORT:
						 lastvalue = (short*)new (short);
					break;
				case VDOUBLE:
						 lastvalue = (double *) new (double);
					break;
				case VCHAR:
							lastvalue = (char *) new (char);
				case VSTRING:
							lastvalue = (char **)new (char*);
						 //	*((char **)lastvalue) = (char *) new (char)[100];
					break;
				case VFLOAT:
						 lastvalue = (float *)new (float);
					break;
				default:

					break;
			}
	}
/********************************  Display  *******************************/
/*
		 Paint display is used for PM manager redraw events, and timed display
	 is used for the timer which catches changes of the variable values.
 */
/******************************  paint display  *************************/
/*
		 redraw events....
 */
void Display::paint_display (HWND hWnd)
	{
		HPS hps;

		hps = WinBeginPaint (hWnd,0,&viewport);
		draw_display (hWnd,hps,1);
		WinEndPaint (hps);
	}
/******************************  timed display  *****************************/
/*
			for value changes of variables...
 */
void Display::timed_display (HWND )
	{
		HPS hps;

		hps = WinGetPS (dwnd);
		draw_display (dwnd,hps,0);
		WinEndPaint (hps);
	}
/****************************  draw display  *****************************/
/*

			The display of status information is accomplished through a dialog
	 window.  Any variable that is displayed in this dialog must be added
	 to a list in the "Display" class.  The varaible's type, address of its
	 value and dialog item are recorded in this class in a list.  The dialog
	 item number corresponds to one of the dialog's items which contains
	 information about where to display the information.
			For this dialog window, all variable information is displayed
	 in string format.  This means that integers, doubles, structures etc
	 must be converted to a string format so that they can be displayed.
	 The "Value" class keeps track of a display variables type and a pointer
	 to its location in memory.  This information is used to convert the
	 variable into a string format.
 */
void Display::draw_display (HWND ,HPS ,int redrawall )
{
	Value *pv;
	char tmp[256];
//GpiSetBackColor (hps,CLR_RED);
	for (pv=v;pv<v+numvalues;pv++) {
		// assign the string value to the appropriate dialog item.
		if (!pv->valid) {
			if (pv->lastvalid != '*') {
				if (pv->type == VCHAR) sprintf (tmp," ");
				else sprintf (tmp," *");
				WinSetDlgItemText (ddialog,pv->ditl,tmp);
				pv->lastvalid = '*';
			}
		}
		else {
			pv->lastvalid = '!';
			switch (pv->type) {
				case VINTEGER:
						if (  (*(int*)pv->value) != (*(int*)pv->lastvalue) || redrawall)  {
							sprintf (tmp,pv->format,(*(int*)(pv->value)));
							WinSetDlgItemText (ddialog,pv->ditl,tmp);
							*(int*)pv->lastvalue = *(int*)pv->value;
						}
					break;
				case VULONG:
						if (  (*(unsigned long*)pv->value) != (*(unsigned long*)pv->lastvalue) || redrawall)  {
							sprintf (tmp,pv->format,(*(unsigned long*)(pv->value)));
							WinSetDlgItemText (ddialog,pv->ditl,tmp);
							*(unsigned long*)pv->lastvalue = *(unsigned long*)pv->value;
						}
					break;
				case VSHORT:
						if (  (*(short*)pv->value) != (*(short*)pv->lastvalue)  || redrawall)  {
							sprintf (tmp,pv->format,(*(short*)(pv->value)));
							WinSetDlgItemText (ddialog,pv->ditl,tmp);
							*(short*)pv->lastvalue = *(short*)pv->value;
						}
					break;
				case VDOUBLE:

							if (  (*(double*)pv->value) != (*(double*)pv->lastvalue) || redrawall )  {
								sprintf (tmp,pv->format,(*(double*)(pv->value)));
								WinSetDlgItemText (ddialog,pv->ditl,tmp);
								*(double*)pv->lastvalue = *(double*)pv->value;
							}


					break;
				case VCHAR:
						if (  (*(char*)pv->value) != (*(char*)pv->lastvalue) || redrawall )  {
							sprintf (tmp,pv->format,(*(char*)(pv->value)));
							WinSetDlgItemText (ddialog,pv->ditl,tmp);
							*(char*)pv->lastvalue = *(char*)pv->value;
						}
					break;
				case VSTRING:
						if (  (*(char**)pv->value) != (*(char**)pv->lastvalue) || redrawall )  {
							sprintf (tmp,pv->format,(*(char**)(pv->value)));
							WinSetDlgItemText (ddialog,pv->ditl,tmp);
							*(char**)pv->lastvalue = *(char**)pv->value;
						}
					break;
				case VFLOAT:
						if (  (*(float*)pv->value) != (*(float*)pv->lastvalue) || redrawall )  {
							sprintf (tmp,pv->format,(*(float*)(pv->value)));
							WinSetDlgItemText (ddialog,pv->ditl,tmp);
							*(float*)pv->lastvalue = *(float*)pv->value;
						}
					break;
				default:
						sprintf (tmp,"ERROR");
					break;
			}

		}
	}
}
/******************************  mark invalid  ***************************/
/*
			Sometimes a variable's value should not be displayed.  This function
		marks a particular value so that it won't be displayed.
 */
void Display::mark_invalid (int dialogitem)
{
	Value *pv;

	for (pv=v;pv<v+numvalues;pv++) {
		if (pv->ditl == dialogitem) {pv->valid = 0; break; }
	}
}
/*******************************  mark valid  *****************************/
/*
		 Undoes what mark invalid does.  Makes a variable display again.
 */
void Display::mark_valid (int dialogitem)
{
	Value *pv;

	for (pv=v;pv<v+numvalues;pv++) {
		if (pv->ditl == dialogitem) {pv->valid = 1; break; }
	}
}
/*******************************  post  *******************************/
/*
		 displays a message in the red background debug screen.  This routine
	 is flakey because it can be called from concurrent processes which
	 sometimes step on each other.  The routine puts the messages in a list
	 which is displayed later from another process.
 */
void Display::post (char *astring)
{

		++postlines;
		postlines %= 8;
		strcpy (postlist[postlines],astring);

}
void Display::post (int i,char *astring)
{
		postlines = 8;
		strcpy (postlist[i],astring);
}
/******************************  display posted  **************************/
/*
		 This displays the list of messages in the dialog items 126 through 133.
 */
void Display::displayposted ()
{

		for (int i=0;i<postlines;i++) {
				 add_value (&postlist[i],133-i,"%s",VSTRING);
		}
		postlines=0;

}