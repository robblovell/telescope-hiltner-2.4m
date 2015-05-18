/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
#ifndef displayhh
#define displayhh
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include "string.h"
#include "values.h"
//  space for status messages displayed on the screen.
extern char **drivestatus;
extern char **trackingstatus;
extern char **initstatus;
extern char **guidestatus;
extern char **domestatus;
extern char **cosdecstatus;
extern char **ratecorstatus;
extern char **trailstatus;
extern char **move2status;

extern Encoder dec_encoder;
extern Encoder ra_encoder;
/*********************************  Title Class ****************************/
/*
		 Not used anymore...
 */
class Title {
	public:
	POINTL pt;
	char name[100];

	Title (int x,int y, char *n)
	{
		 pt.x = x; pt.y = y; strcpy (name,n);
	}

	Title () {}
	~Title () {}
	void set (int x,int y, char *n) {
		 pt.x = x; pt.y = y; strcpy (name,n);
	}
	Title & operator = (Title &v) {
		pt.x = v.pt.x;
		pt.y = v.pt.y;
		strcpy (name,v.name);
		return *this;
	}
};
#define VSTRING	0
#define VINTEGER	1
#define VDOUBLE	2
#define	VCHAR		3
#define VFLOAT	4
#define VSHORT	5
#define VULONG	6
/************************ Value and Display Classes ***********************/
/* classes: Value & Display.

			The display of status information is accomplished through a dialog
	 window.  Any variable that is displayed in this dialog must be added
	 to a list in the "Display" class.  The varaible's type, address of its
	 value and dialog item are recorded in this class in a list.  The dialog
	 item number corresponds to one of the dialog's items which contains
	 information about where to display the information.
*/
#define	ERRORX	10
#define ERRORY  10
extern "C" {
extern	MRESULT EXPENTRY DisplayDlgProc (HWND hWnd,ULONG msg, MPARAM mp1,MPARAM mp2) ;
}
#define MAXVALUES	200

/******************************  Value Class  ******************************/
class Value {
	public:

	void *value;
	int ditl;
	char format[30];
	void *lastvalue;
	int type;
	int valid;
	char lastvalid;
	Value (void *val,int d,char *form,int type_)
	{
		 value = val; ditl = d;  strcpy (format,form); type = type_;
		 lastvalue = NULL;valid = 1;
	}

	Value () {}
	~Value () {}
	void set (void *val, int d,char *form,int type_);

	Value & operator = (Value &v) {
		value = v.value;
		ditl = v.ditl;
		strcpy (format,v.format); type = v.type;
		valid = v.valid;
		return *this;
	}
};

/****************************  Class:  Display  *************************/
class Display {
	public:
	Value v[MAXVALUES];

	HWND &dwnd;
	int numvalues;
	RECTL viewport;
	int background;
	HWND ddialog;
		int postlines;
		char postlist[8][30];  // 17 to 28 characters can be displayed...
	Display (HWND &hwnd,int x,int y,int xsize,int ysize,int bg)
		: dwnd(hwnd)
	{
		viewport.xLeft = x;
		viewport.xRight = x+xsize;
		viewport.yBottom = y;
		viewport.yTop = y+ysize;
		background = bg;
		numvalues = 0;
		postlines = 0;
	}
	// procedure which creates the display dialog.
	void create_dialog (HWND &hwnd)
	{
		ddialog = WinLoadDlg (HWND_DESKTOP,hwnd,DisplayDlgProc,
			 0L,IDD_DISPLAY,NULL);


	 /*	WinQueryWindowRect (ddialog,&viewport);  */

	}
	~Display ()
	{
	}
	void add_value (void *val,int ditl,char *form,int typ) {
		if (numvalues < MAXVALUES) {
			v[numvalues++].set (val,ditl,form,typ);
		}
	}
	void add_value (int *val,int ditl,char *form,int typ) {
		if (numvalues < MAXVALUES) {
			v[numvalues++].set (val,ditl,form,typ);
		}
	}
	void add_value (Value &newv) {
		if (numvalues < MAXVALUES)  {
			v[numvalues++] = newv;
		}
	}
	void paint_display (HWND hWnd);
	void timed_display (HWND hWnd);
	void draw_display (HWND hwnd,HPS hps,int redrawall);
	void mark_valid (int dialogitem);

	void mark_invalid (int dialogitem);
	void post (char *astring);
	void post (int i,char *astring);
	void displayposted ();};
#endif