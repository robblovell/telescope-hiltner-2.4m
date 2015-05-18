/* --------------------------------------------------------------------
								  About Dialog Handler

Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
-------------------------------------------------------------------- */

#define INCL_WIN

#include <os2.h>
#include "about.h"

#define DB_RAISED       0x0400
#define DB_DEPRESSED    0x0800

VOID DisplayAbout (HWND hWnd, PSZ pszAppName)
{
    WinDlgBox (HWND_DESKTOP, hWnd, AboutDlgProc, 0L, 10000, pszAppName);
    return;
}

/* ----------------------  Dialog Function ----------------------- */
extern "C" {
MRESULT EXPENTRY AboutDlgProc (HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    BOOL    bHandled = TRUE;
    MRESULT mReturn  = 0;
    ULONG   ulScrWidth, ulScrHeight;
    RECTL   Rectl;
    SWP     Swp;
    HPS     hps;

    switch (msg)
    {           
        case WM_INITDLG:
            /* Center dialog on screen */
            ulScrWidth  = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
            ulScrHeight = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);
            WinQueryWindowRect (hWnd, &Rectl);
            WinSetWindowPos (hWnd, HWND_TOP, (ulScrWidth-Rectl.xRight)/2,
                (ulScrHeight-Rectl.yTop)/2, 0, 0, SWP_MOVE | SWP_ACTIVATE);

            /* Set application title */
            WinSetDlgItemText (hWnd, 10001, (PSZ)mp2);
            break;

        case WM_PAINT:
            hps = WinBeginPaint (hWnd,0,0);
            WinQueryWindowRect (hWnd, &Rectl);
            WinFillRect (hps, &Rectl, CLR_PALEGRAY);
            WinDrawBorder (hps, &Rectl, 
                WinQuerySysValue(HWND_DESKTOP,SV_CXDLGFRAME), 
                WinQuerySysValue(HWND_DESKTOP,SV_CYDLGFRAME), 
                CLR_DARKGRAY, CLR_WHITE, DB_RAISED);
            GpiMove (hps, (PPOINTL)&Rectl);
            Rectl.xRight--;
            Rectl.yTop--;
            GpiBox (hps, DRO_OUTLINE, (PPOINTL)&Rectl.xRight, 0L, 0L);
            WinQueryWindowPos (WinWindowFromID (hWnd, 10002), &Swp);
            Rectl.xLeft   = Swp.x-1;
            Rectl.yBottom = Swp.y-1;
            Rectl.xRight  = Swp.x + Swp.cx + 1;
            Rectl.yTop    = Swp.y + Swp.cy + 1;
            WinDrawBorder (hps, &Rectl, 1L, 1L,
                CLR_DARKGRAY, CLR_WHITE, DB_DEPRESSED);
            WinQueryWindowPos (WinWindowFromID (hWnd, 10003), &Swp);
            Rectl.xLeft   = Swp.x-1;
            Rectl.yBottom = Swp.y-1;
            Rectl.xRight  = Swp.x + Swp.cx + 1;
            Rectl.yTop    = Swp.y + Swp.cy + 1;
            WinDrawBorder (hps, &Rectl, 1L, 1L,
                CLR_DARKGRAY, CLR_WHITE, DB_DEPRESSED);
            WinEndPaint (hps);
            break;

		  case WM_COMMAND:
            WinDismissDlg (hWnd, DID_OK);
            break;

        default:
            bHandled = FALSE;
            break;
    }

    if (!bHandled)
        mReturn = WinDefDlgProc (hWnd, msg, mp1, mp2);

    return (mReturn);
}
}
