/* -------------------------------------------------------------------
Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
------------------------------------------------------------------- */

#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_WIN


#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include "hiltner.h"
#include "controls.h"
#include "about.h"

/* Undocumented WinDrawBorder flag */
#define DB_RAISED       0x0400

/* Sorting thread structure         */
#define ARRAY_SIZE 4096
typedef struct
{
    HWND   hWndDlg;                    /* Dialog window handle      */
    ULONG  ulCnt;                      /* Number of entries to sort */
    ULONG  ulArray[1];                 /* Array of data to sort     */
} SORTDATA;
typedef SORTDATA *PSORTDATA;

/* Window and Dialog Functions      */
MRESULT EXPENTRY ClientWndProc     (HWND,ULONG,MPARAM,MPARAM);
MRESULT EXPENTRY TIDDlgProc        (HWND,ULONG,MPARAM,MPARAM);
MRESULT EXPENTRY SortingDlgProc    (HWND,ULONG,MPARAM,MPARAM);

/* Thread Functions                 */
//VOID ThermometerThread (HWND);
//VOID SortingThread (PSORTDATA);

/* Local Functions                  */
//VOID ArrangeThreadWindows (VOID);
//VOID CreateSortingThread (HWND);
//HWND CreateThreadWindow (HWND);
//VOID DestroyThreadWindow (HWND);
//VOID ThreadMessageLoop (HAB,HWND,HWND);
 
/* User-Defined messages            */
#define WM_USER_KILL            WM_USER+1
#define WM_USER_UPPRIORITY      WM_USER+2
#define WM_USER_DOWNPRIORITY    WM_USER+3
#define WM_USER_CRITSECTION     WM_USER+4
#define WM_USER_SEMAPHORE       WM_USER+5
#define WM_USER_PERCENTCOMPLETE WM_USER+6

/* Global Variables                 */
HAB      hab;                          /* Handle to anchor block    */
HWND     hWndFrame,                    /* Frame window handle       */
         hWndClient,                   /* Client window handle      */
         hWndMainTID,                  /* Main thread ID            */
         hWndMainThermo;               /* Main thermometer window   */
CHAR     szTitle[64];                  /* Title string              */
RECTL    RectlTID;                     /* Thread dialog size        */
HMTX     ThreadSem;                    /* Thread semaphore          */
ULONG    ulThreadCnt  = 0;             /* Number of QMSG threads    */
SHORT    sScrollPos   = 0;             /* Current scroll position   */
SHORT    sScrollRange = 0;             /* Current scroll range      */


/* ----------------- Main Application Function -------------------- */

int main()
{
    HMQ   hmq;
    ULONG flFrameFlags    = FCF_TITLEBAR | FCF_SYSMENU | FCF_ICON |
           FCF_SIZEBORDER | FCF_MINMAX   | FCF_SHELLPOSITION | 
           FCF_TASKLIST | FCF_MENU | FCF_NOBYTEALIGN | FCF_VERTSCROLL;
    CHAR  szClientClass[] = "CLIENT";
    CHAR  szFailBuff [CCHMAXPATH];
    HMODULE hDLLModule;

    hab = WinInitialize (0L);
    hmq = WinCreateMsgQueue (hab, 0L);

    /* Load the controls dynamic link library */
    if (DosLoadModule (szFailBuff, sizeof(szFailBuff), "CONTROLS", 
            &hDLLModule))
    {
        WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
            "Unable to load CONTROLS.DLL",
				"Hiltner 2.4 Meter Telescope",
            0, MB_OK | MB_CRITICAL | MB_MOVEABLE);
    }
    else
    {
        WinRegisterClass (hab, szClientClass, ClientWndProc, 
            CS_CLIPCHILDREN, 0);
        WinLoadString (hab, 0L, ID_APPNAME, sizeof(szTitle), 
            szTitle);

        /* Create the main application window */
        hWndFrame = WinCreateStdWindow (HWND_DESKTOP, 0, 
            &flFrameFlags, szClientClass, szTitle, 0, 0,
            ID_APPNAME, &hWndClient);

        WinShowWindow (hWndFrame, TRUE);

        /* Create main thread thermometer window */
        hWndMainTID    = CreateThreadWindow (hWndClient);
        hWndMainThermo = WinWindowFromID (hWndMainTID, 
                             IDC_THERMOMETER);
        WinEnableControl (hWndMainTID, IDC_KILL, FALSE);
        WinEnableControl (hWndMainTID, IDC_SEMAPHORE, FALSE);

        /* Create the thread semaphore */
        DosCreateMutexSem (NULL, &ThreadSem, 0L, FALSE);

        ThreadMessageLoop (hab, hWndMainTID, hWndMainThermo);

        /* Close the thread semaphore */
        DosCloseMutexSem (ThreadSem);

        DestroyThreadWindow (hWndMainTID);

        WinDestroyWindow (hWndFrame);

        /* Release the controls dynamic link library */
        DosFreeModule (hDLLModule);
    }

    WinDestroyMsgQueue (hmq);
    WinTerminate (hab);

    return (0);
}


/* ------------------ Sorting Thread Function --------------------- */

/*
   The Sorting thread is responsible for sorting the array of unsigned
   long numbers.  The input to this thread is the address of a
   SORTDATA structure.  This thread is for demonstration only
   so it performs the very slow bubble sort.  A 
   WM_USER_PERCENTCOMPLETE message is posted to the dialog window as 
   it completes each percent of the sorting process.  When sorting is 
   complete the thread posts a 100% WM_USER_PERCENTCOMPLETE message to 
   the dialog and terminates itself.

   This is a Non-Message queue thread so it cannot send messages to
   a message queue thread. 
*/

#ifdef __IBMC__
#pragma linkage(SortingThread, system)
#endif

#ifdef __WATCOM__
VOID __stdcall SortingThread (PSORTDATA pData)
#else
VOID SortingThread (PSORTDATA pData)
#endif
{
    ULONG ulInx,
          ulJnx,
          ulNewPercent,
          ulPercent,
          ulTotalComparisons,
          ulNumComparisons;

    /* Total number of comparisons = ulCnt! */
    ulTotalComparisons = (pData->ulCnt * (pData->ulCnt + 1)) / 2;
    ulNumComparisons   = 0;
    ulPercent          = 0L;

    /* Perform bubble sort */
    for (ulInx = 0; ulInx < (pData->ulCnt - 1); ulInx++)
    {
        for (ulJnx = ulInx+1; ulJnx < pData->ulCnt; ulJnx++)
            if (pData->ulArray[ulInx] > pData->ulArray[ulJnx])
            {
                /* Swap values using exclusive-or operation */
                pData->ulArray[ulInx] ^= pData->ulArray[ulJnx];
                pData->ulArray[ulJnx] ^= pData->ulArray[ulInx];
                pData->ulArray[ulInx] ^= pData->ulArray[ulJnx];
            }

        /* Increment number of comparisons done so far */
        ulNumComparisons += (pData->ulCnt - ulInx - 1);

        ulNewPercent = (ulNumComparisons * 100) / ulTotalComparisons;
        if (ulNewPercent != ulPercent)
        {
            ulPercent = ulNewPercent;
            WinPostMsg (pData->hWndDlg, WM_USER_PERCENTCOMPLETE, 
                (MPARAM)ulPercent, 0L);
        }
    }

    /* Post 100% completion message */
    WinPostMsg (pData->hWndDlg, WM_USER_PERCENTCOMPLETE, 
        (MPARAM)100L, 0L);

    DosExit (EXIT_THREAD, 0L);
}


/* --------------- Thermometer Thread Function -------------------- */

/*
    The Thermometer thread is responsible for simply incrementing
    and decrementing the current thermometer value.  Most of the
    thread processing is performed in the ThreadMessageLoop function.

    Since this thread will be directly updating the dialog box control
    windows it must create a message queue.  Thus it will be a
    Message queue thread.
*/

#ifdef __IBMC__
#pragma linkage(ThermometerThread, system)
#endif

#ifdef __WATCOM__
VOID __stdcall ThermometerThread (HWND hWnd)
#else
VOID ThermometerThread (HWND hWnd)
#endif
{
    HMQ   hmqThread;
    HAB   habThread;
    HWND  hWndTID,
          hWndThermo;

    /* Create message queue for this thread */
    habThread = WinInitialize (0L);
    hmqThread = WinCreateMsgQueue (hab, 0L);

    /* Create the thread window */
    hWndTID    = CreateThreadWindow (hWnd);
    hWndThermo = WinWindowFromID(hWndTID, IDC_THERMOMETER);

    ThreadMessageLoop (hab, hWndTID, hWndThermo);

    /* Destroy the thread's window */
    DestroyThreadWindow (hWndTID);

    /* Destroy the message queue for this thread */
    WinDestroyMsgQueue (hmqThread);
    WinTerminate (habThread);

    DosExit (EXIT_THREAD, 0L);
}


/* ----------------------- Local Functions ------------------------ */

VOID ArrangeThreadWindows ()
{
    HENUM hEnum;
    PSWP  pSwp;
    RECTL Rectl;
    ULONG ulNumPerRow,
          ulCnt = 0;
    HWND  hWndThread;

    /* Allocate memory to hold the SWP structures */
    if (!DosAllocMem ((PPVOID)&pSwp, sizeof(SWP)*ulThreadCnt, fALLOC))
    {
        /* Calculate number of thread windows that fit in each row */
        WinQueryWindowRect (hWndClient, &Rectl);
        ulNumPerRow = Rectl.xRight / RectlTID.xRight;
        if (ulNumPerRow == 0)
            ulNumPerRow++;

        /* Enter critical section so that the list of current windows
           remains unchanged while we enumerate */
        DosEnterCritSec ();

        /* Begin thread window enumeration */
        hEnum = WinBeginEnumWindows (hWndClient);

        /* Get handle to each thread window */
        while ((hWndThread = WinGetNextWindow (hEnum)) != 0)
        {
            pSwp[ulCnt].fl     = SWP_MOVE | SWP_NOADJUST | SWP_SHOW;
            pSwp[ulCnt].x      = (ulCnt % ulNumPerRow) *
                                            RectlTID.xRight;
            pSwp[ulCnt].y      = Rectl.yTop - 
                (((ulCnt / ulNumPerRow) + 1) * RectlTID.yTop);
            pSwp[ulCnt++].hwnd = hWndThread;
        }

        /* End Thread window enumeration */
        WinEndEnumWindows (hEnum);

        /* Exit the critical section since we have enumerated all the
           current windows */
        DosExitCritSec ();

        /* Arrange the tiled windwos */
        WinSetMultWindowPos (hab, pSwp, ulCnt);

        /* Set the scroll range and initial position */
        sScrollPos   = 0;
        sScrollRange = (SHORT)-pSwp[ulCnt-1].y;
        WinSendDlgItemMsg (hWndFrame, FID_VERTSCROLL, 
            SBM_SETSCROLLBAR, 
            MPFROMSHORT(0), MPFROM2SHORT(0,sScrollRange));

        /* Free the allocated memory    */
        DosFreeMem ((PVOID)pSwp);
    }

    return;
}

VOID CreateSortingThread (HWND hWnd)
{
    TID            ThreadID;
    ULONG          ulInx;
    PSORTDATA      pData;
    RECTL          Rectl;
    POINTL         Ptl;

    /* Allocate a block of memory to contain the sorting data plus all
       of the array entries to sort */
    if (!DosAllocMem ((PPVOID)&pData, 
            sizeof(SORTDATA) + ((ARRAY_SIZE - 1) * sizeof(ULONG)),
            fALLOC))
    { 
        /* Initialize array as an array of descending numbers to be 
           sorted in ascending order (worst case order) */
        for (ulInx = 0; ulInx < ARRAY_SIZE; ulInx++)
            pData->ulArray[ulInx] = ARRAY_SIZE - ulInx;

        pData->ulCnt = ARRAY_SIZE;

        /* Create dialog to display sorting status */
        pData->hWndDlg = WinLoadDlg (HWND_DESKTOP, hWnd, 
            SortingDlgProc, 0L, IDD_SORTINGDLG, NULL);

        /* Create the sorting thread in a suspended state */
        DosCreateThread (&ThreadID, (PFNTHREAD)SortingThread, 
            (ULONG)pData, 0x0001, 0x2000);

        WinSetWindowUShort (pData->hWndDlg, QWS_ID, 
            (USHORT)ThreadID);
        WinSetDlgItemShort (pData->hWndDlg, IDC_THREADID, 
            (USHORT)ThreadID, FALSE);
        WinSetWindowPtr (pData->hWndDlg, QWL_USER, (PVOID)pData);

        /* Position the sorting dialog */
        WinQueryWindowRect (pData->hWndDlg, &Rectl);
        Ptl.x = Ptl.y = 0L;
        WinMapWindowPoints (hWndFrame, HWND_DESKTOP, &Ptl, 1L);
        WinSetWindowPos (pData->hWndDlg, 0L,
            Ptl.x + (ThreadID & 1) * Rectl.xRight,
            ((ThreadID & 7) >> 1) * Rectl.yTop,
            0L, 0L, SWP_MOVE | SWP_SHOW);

        /* Start the thread */
        DosResumeThread (ThreadID);
    }

    return;
}

HWND CreateThreadWindow (HWND hWnd)
{
    HWND   hWndTID;

    hWndTID = WinLoadDlg (hWnd, hWnd, TIDDlgProc, 0L, IDD_TIDDLG, 
                  NULL);

    /* Increment count of message queue threads within a critical 
       section since multiple threads may be calling this function at 
       the same time */
    DosEnterCritSec();
    ulThreadCnt++;
    DosExitCritSec();

    WinSetWindowPos (hWndTID, HWND_BOTTOM, 0L, 0L, 0L, 0L,SWP_ZORDER);
    ArrangeThreadWindows ();

    return (hWndTID);
}
                                                           
VOID DestroyThreadWindow (HWND hWnd)
{
    WinDestroyWindow (hWnd);

    /* Decrement count of message queue threads within a critical 
       section since multiple threads may be calling this function at 
       the same time */
    DosEnterCritSec();
    ulThreadCnt--;
    DosExitCritSec();

    return;
}

VOID ThreadMessageLoop (HAB hab, HWND hWndMain, HWND hWndThermo)
{
    APIRET RetCode;
    QMSG   qmsg;
    PPIB   ppib;
    PTIB   ptib;
    BOOL   bContinue          = TRUE;
    ULONG  ulValue            = 0L;
    LONG   lIncDec            = 1L;
    LONG   lPriority          = 0L;
    ULONG  ulCurPriority      = 0L;
    BOOL   bDoCriticalSection = FALSE;
    BOOL   bDoSemaphore       = FALSE;
    BOOL   bHaveSemaphore     = FALSE;

    /* Message processing loop for a thread.  Peek messages from the
       message queue and dispatch them as the default.  If the message
       requires special processing then do it here.  This function
       executes on behalf of each thread that calls it so the message
       processing is per thread using each thread's stack space. */

    do
    {
        while (bContinue &&
               WinPeekMsg (hab, &qmsg, 0L, 0L, 0L, PM_REMOVE))
        {
            switch (qmsg.msg)
            {
                case WM_QUIT:
                case WM_USER_KILL:
                    bContinue = FALSE;

                    /* Force release of semaphore */
                    ulValue      = 0L;
                    bDoSemaphore = FALSE;
                    break;

                case WM_USER_UPPRIORITY:
                    /* Increase thread priority */
                    lPriority++;
                    DosSetPriority (PRTYS_THREAD, PRTYC_REGULAR, 
                        lPriority, 0L);
                    if (lPriority == PRTYD_MAXIMUM)
                        WinEnableControl (hWndMain, IDC_UPPRIORITY, 
                            FALSE);
                    WinEnableControl (hWndMain, IDC_DOWNPRIORITY, 
                        TRUE);
                    break;

                case WM_USER_DOWNPRIORITY:
                    /* Decrease thread prioity */
                    lPriority--;
                    DosSetPriority (PRTYS_THREAD, PRTYC_REGULAR, 
                        lPriority, 0L);
                    if (lPriority == 0L)
                        WinEnableControl (hWndMain, IDC_DOWNPRIORITY, 
                            FALSE);
                    WinEnableControl (hWndMain, IDC_UPPRIORITY, TRUE);
                    break;

                case WM_USER_CRITSECTION:
                    bDoCriticalSection = !bDoCriticalSection;
                    break;

                case WM_USER_SEMAPHORE:
                    bDoSemaphore = !bDoSemaphore;
                    break;

                default:
                    WinDispatchMsg (hab, &qmsg);
            }
        }

        /* No more messages in the queue */

        /* Update thread priority in the dialog if it has changed */
        DosGetInfoBlocks (&ptib, &ppib);
        if (ulCurPriority != (ptib->tib_ptib2)->tib2_ulpri)
        {
            ulCurPriority = (ptib->tib_ptib2)->tib2_ulpri;
            WinSetDlgItemShort (hWndMain, IDC_THREADPRIORITY, 
                (USHORT)ulCurPriority, FALSE);
        }

        /* If the user has indicated that this thread should enter a 
           critical section then enter a critical section and sleep 
           for 100 milliseconds */
        if (bDoCriticalSection)
        {
            WinSetDlgItemText (hWndMain, IDC_STATE, "Crit Sec");
            DosEnterCritSec ();
            DosSleep (100L);
            DosExitCritSec ();
            WinSetDlgItemText (hWndMain, IDC_STATE, 
                bHaveSemaphore ? "Semaphore" : "Running");
        }

        /* Update thread thermometer value */
        WinSendMsg (hWndThermo, THM_SETVALUE, (MPARAM)ulValue, 0L);

        /* If the thermometer value is zero then reverse direction and
           check if the semaphore should be requested or released */
        if (ulValue == 0)
        {
           lIncDec = 1L;

           /* If this thread has the semaphore then release it */
           if (bHaveSemaphore)
           {
               bHaveSemaphore = FALSE;
               WinSetDlgItemText (hWndMain, IDC_STATE, "Running");
               DosReleaseMutexSem(ThreadSem);
           }

           /* If the user has indicated this thread should request
              the semaphore each time the thermometer value reaches 
              zero then request it.  If after 2 seconds the thread 
              hasn't received the semaphore then continue on anyway.*/
           if (bDoSemaphore)
           {
               WinSetDlgItemText (hWndMain, IDC_STATE, "Req Sem");
               RetCode = DosRequestMutexSem (ThreadSem, 2000L);
               if (RetCode == 0L)
               {
                   bHaveSemaphore = TRUE;
                   WinSetDlgItemText (hWndMain, IDC_STATE, 
                       "Semaphore");
               }
               else if (RetCode == ERROR_TIMEOUT)
                   WinSetDlgItemText (hWndMain, IDC_STATE, 
                       "Sem Timeout");
               else if (RetCode == ERROR_SEM_OWNER_DIED)
                   WinSetDlgItemText (hWndMain, IDC_STATE, 
                       "Sem Owner Died");
               else
                   WinSetDlgItemText (hWndMain, IDC_STATE, 
                       "Sem Req Error");
           }

           /* Give lower priority threads a chance to do something */
           DosSleep (5L);

        }
        else if (ulValue == 100)
           lIncDec = -1L;
        ulValue += lIncDec;

    } while (bContinue);

    return;
}

/* ----------------- Window and Dialog Functions ------------------ */

MRESULT EXPENTRY ClientWndProc (HWND hWnd, ULONG msg, MPARAM mp1, 
                     MPARAM mp2)
{
    BOOL    bHandled = TRUE;
    MRESULT mReturn  = 0;
    RECTL   Rectl;
    HPS     hps;
    HENUM   hEnum;
    TID     ThreadID;
    HWND    hWndThread;
    SHORT   sNewScrollPos;

    switch (msg)
    {
        case WM_PAINT:
            hps = WinBeginPaint (hWnd,0L,NULL);
            WinQueryWindowRect (hWnd, &Rectl);
            WinFillRect (hps, &Rectl, CLR_PALEGRAY);
            WinEndPaint (hps);
            break;

        case WM_SIZE:
            ArrangeThreadWindows ();
            break;

        case WM_INITMENU:
		/*      if (SHORT1FROMMP(mp1) == IDM_THREADS)
                WinEnableMenuItem ((HWND)mp2, IDM_KILLALLTHREADS, 
						  (ulThreadCnt > 1L));
            break;
		  */
        case WM_VSCROLL:
            sNewScrollPos = sScrollPos;    
            switch (SHORT2FROMMP(mp2))
            {
                case SB_LINEUP:
                    sNewScrollPos--;
                    break;

                case SB_LINEDOWN:
                    sNewScrollPos++;
                    break;

                case SB_PAGEUP:
                    sNewScrollPos -= (SHORT)RectlTID.yTop;
                    break;

                case SB_PAGEDOWN:
                    sNewScrollPos += (SHORT)RectlTID.yTop;
                    break;

                case SB_SLIDERPOSITION:
                    sNewScrollPos = SHORT1FROMMP(mp2);
                    break;
            }

            sNewScrollPos = 
                (SHORT)max (0, min (sNewScrollPos, sScrollRange));
            if (sNewScrollPos != sScrollPos)
            {
                WinScrollWindow (hWndClient, 0L, 
                   sNewScrollPos - sScrollPos, NULL, NULL, 0, NULL,
                   SW_SCROLLCHILDREN | SW_INVALIDATERGN);
                sScrollPos = sNewScrollPos;
                WinSendDlgItemMsg (hWndFrame, FID_VERTSCROLL,
                    SBM_SETPOS, MPFROMSHORT(sScrollPos), 0L);
                WinUpdateWindow (hWndClient);
            }
            break;

		  case WM_COMMAND:
		      switch (LOUSHORT(mp1))
            {
				  /*  case IDM_NEWQMSGTHREAD:
                    DosCreateThread (&ThreadID, 
                        (PFNTHREAD)ThermometerThread, 
                        (ULONG)hWndClient, 0, 0x5000);
                    break;

                case IDM_NEWSORTINGTHREAD:
                    CreateSortingThread (hWnd);
                    break;

                case IDM_KILLALLTHREADS:
                    hEnum = WinBeginEnumWindows (hWnd);
                    while (hWndThread = WinGetNextWindow (hEnum))
                    {

                        if (WinQueryWindowUShort 
                                (hWndThread, QWS_ID) > 1L)
                            WinPostMsg (hWndThread, WM_USER_KILL, 
                                0L, 0L);
                    }
                    WinEndEnumWindows (hEnum);
						  break;
						  */
					/*
					 case IDM_ARRANGE:
						  ArrangeThreadWindows ();
                    break;
					 */
                case IDM_ABOUT:
                    DisplayAbout (hWnd, szTitle);
                    break;
            }

        default:
            bHandled = FALSE;
            break;
    }

    if (!bHandled)
        mReturn = WinDefWindowProc (hWnd,msg,mp1,mp2);

    return (mReturn);
}

MRESULT EXPENTRY TIDDlgProc (HWND hWnd, ULONG msg, MPARAM mp1, 
                     MPARAM mp2)
{
    PPIB    ppib;
    PTIB    ptib;
    MRESULT mReturn  = 0L;
    BOOL    bHandled = TRUE;

    switch (msg)
     {
        case WM_INITDLG:
            /* Initialize thread dialog values */
            DosGetInfoBlocks (&ptib, &ppib);
            WinSetWindowUShort (hWnd, QWS_ID, 
                (USHORT)((ptib->tib_ptib2)->tib2_ultid));
            WinSetDlgItemShort (hWnd, IDC_THREADID, 
                (USHORT)((ptib->tib_ptib2)->tib2_ultid), FALSE);
            WinSetDlgItemShort (hWnd, IDC_THREADPRIORITY, 
                (USHORT)((ptib->tib_ptib2)->tib2_ulpri), FALSE);
            WinEnableControl (hWnd, IDC_DOWNPRIORITY, FALSE);

            /* Set thermometer attributes */
            WinSendDlgItemMsg (hWnd, IDC_THERMOMETER, THM_SETRANGE, 
                0L, (MPARAM)100L);
            WinSendDlgItemMsg (hWnd, IDC_THERMOMETER, THM_SETVALUE, 
                0L, 0L);
            WinSendDlgItemMsg (hWnd, IDC_THERMOMETER, THM_SETCOLOR,
                MPFROM2SHORT((ptib->tib_ptib2)->tib2_ultid % 15, 
                CLR_PALEGRAY), 0L);

            /* Store the size of the thread dialog window */
            WinQueryWindowRect (hWnd, &RectlTID);
            break;

		  case WM_COMMAND:
		      switch (LOUSHORT(mp1))
            {
                case IDC_KILL:
                    WinPostMsg (hWnd, WM_USER_KILL, 0L, 0L);
                    break;

                case IDC_UPPRIORITY:
                    WinPostMsg (hWnd, WM_USER_UPPRIORITY, 0L, 0L);
                    break;

                case IDC_DOWNPRIORITY:
                    WinPostMsg (hWnd, WM_USER_DOWNPRIORITY, 0L, 0L);
                    break;

                case IDC_CRITSECTION:
                    WinPostMsg (hWnd, WM_USER_CRITSECTION, 0L, 0L);
                    break;

                case IDC_SEMAPHORE:
                    WinPostMsg (hWnd, WM_USER_SEMAPHORE, 0L, 0L);
                    break;
            }
            break;

        default:
           bHandled = FALSE;
           break;
    }

    if (!bHandled)
        mReturn = WinDefDlgProc (hWnd, msg, mp1, mp2);

    return (mReturn);
}

MRESULT EXPENTRY SortingDlgProc (HWND hWnd, ULONG msg, MPARAM mp1,   
                     MPARAM mp2)
{
    TID     ThreadID;
    HPS     hps;
    HWND    hWndCtl;
    RECTL   Rectl;
    MRESULT mReturn  = 0L;
    BOOL    bHandled = TRUE;

    switch (msg)
     {
        case WM_USER_PERCENTCOMPLETE:
            /* Update the percent complete bar in the dialog */
            WinSetDlgItemShort (hWnd, IDC_PERCENT, 
                (USHORT)mp1, FALSE);
            hWndCtl = WinWindowFromID (hWnd, IDC_PERCENTBAR);
            WinQueryWindowRect (hWndCtl, &Rectl);
            hps = WinGetPS (hWndCtl);
            WinDrawBorder (hps, &Rectl, 1L, 1L, SYSCLR_BUTTONDARK,
                SYSCLR_BUTTONMIDDLE, DB_RAISED);
            WinInflateRect (hab, &Rectl, -1L, -1L);
            Rectl.xRight = (Rectl.xRight * (ULONG)mp1) / 100;
            WinFillRect (hps, &Rectl, SYSCLR_ACTIVETITLE);
            WinReleasePS (hps);

            /* If sorting is 100% complete then beep and destroy the
               status window */
            if ((ULONG)mp1 == 100L)
            {
                DosBeep (1000L, 75L);
                DosBeep (800L, 75L);
                /* A process would now use the sorted array. Since 
                   this is a demonstration just free the memory */
                DosFreeMem (WinQueryWindowPtr (hWnd, QWL_USER));
                WinDestroyWindow (hWnd);
            }
            break;

		  case WM_COMMAND:
            ThreadID = (TID)WinQueryWindowUShort (hWnd, QWS_ID);
		      switch (LOUSHORT(mp1))
            {
                case IDC_KILL:
                    DosKillThread (ThreadID);
                    DosFreeMem (WinQueryWindowPtr (hWnd, QWL_USER));
                    WinDestroyWindow (hWnd);
                    break;

                case IDC_SUSPEND:
                    WinSetDlgItemText (hWnd, IDC_SUSPEND, "Resume");
                    WinSetWindowUShort (
                        WinWindowFromID (hWnd, IDC_SUSPEND),
                        QWS_ID, IDC_RESUME);
                    DosSuspendThread (ThreadID);
                    break;

                case IDC_RESUME:
                    WinSetDlgItemText (hWnd, IDC_RESUME, "Suspend");
                    WinSetWindowUShort (
                        WinWindowFromID (hWnd, IDC_RESUME),
                        QWS_ID, IDC_SUSPEND);
                    DosResumeThread (ThreadID);
                    break;

                case IDC_UPPRIORITY:
                    /* Increase the priority */
                    DosSetPriority (PRTYS_THREAD, PRTYC_NOCHANGE, 1L, 
                        ThreadID);
                    break;

                case IDC_DOWNPRIORITY:
                    /* Decrease the priority */
                    DosSetPriority (PRTYS_THREAD, PRTYC_NOCHANGE, -1L, 
                        ThreadID);
                    break;
            }
            break;

        default:
           bHandled = FALSE;
           break;
    }

    if (!bHandled)
        mReturn = WinDefDlgProc (hWnd, msg, mp1, mp2);

    return (mReturn);
}






 






