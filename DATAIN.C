/* --------------------------------------------------------------------
                              File Search
                              Chapter 15

                    Real World Programming for OS/2
             Copyright (c) 1993 Blain, Delimon, and English
-------------------------------------------------------------------- */

#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_WIN
#define INCL_ERRORS

#include <os2.h>
#include <string.h>
#include "misc.h"
#include "drives.h"
#include "filesrch.h"

/* Thread Function */

#ifdef __WATCOM__
VOID __stdcall SearchThread (HWND);
#else
VOID SearchThread (HWND);
#endif

/* Local Functions    */
VOID SearchForFile (HWND);
VOID SetControlStates (HWND);
VOID StartSearch (HWND);
VOID StopSearch (VOID);

/* External variables */
extern HAB       hab;                  /* Handle to anchor block     */
extern DRIVEINFO DriveInfo[];
extern ULONG     ulNumDrives;

/* Global variables */
TID  SearchThreadID;                   /* Thread ID of Search thread */
CHAR szFileName[CCHMAXPATHCOMP];       /* Filename to search         */
CHAR szFullPath[CCHMAXPATH] = " :\\";  /* File path buffer           */
HWND hWndFileSearch         = 0;       /* File search dialog window  */
BOOL bSearching             = FALSE;   /* Search in progress flag    */

/* ------------------ Search Thread Function ----------------------- */

/*
    The Search thread is responsible for searching the selected disk
    for the specified filename.  This function will search all
    subdirectories starting with the directory in szFullPath.
    The field IDC_CURRENTDIR is updated with the name of each directory 
    that is being searched.  Each matching filename is added to
    the IDC_DIRLIST listbox.

    Since this thread will be directly updating the dialog box control
    windows it must create a message queue.  Thus it will be a
    Message queue thread.

    The thread is automatically terminated when all subdirectories have
    been search or when the bSearching flag is reset.
*/

#ifdef __IBMC__
#pragma linkage(SearchThread, system)
#endif

#ifdef __WATCOM
VOID __stdcall SearchThread (HWND hWnd)
#else
VOID SearchThread (HWND hWnd)
#endif

{
    HMQ hmqThread;
    HAB habThread;

    /* Create message queue for this thread */
    habThread = WinInitialize (0);
    hmqThread = WinCreateMsgQueue (hab, 0);

    bSearching = TRUE;

    SetControlStates (hWnd);

    /* Perform the search */
    SearchForFile (hWnd);

    bSearching = FALSE;

    SetControlStates (hWnd);

    /* Destroy the message queue for this thread */
    WinDestroyMsgQueue (hmqThread);
    WinTerminate (habThread);

    DosExit (EXIT_THREAD, 0L);
}

/* ----------------------- Local Functions ----------------------- */

VOID SearchForFile (HWND hWnd)
{ 
    HDIR         hDir        = HDIR_CREATE;
    ULONG        ulDirPos    = strlen (szFullPath);
    ULONG        ulFileCount = 1L;
    FILEFINDBUF3 FindBuf3;

    /* Check for filename length overflow */
    if ((ulDirPos + strlen(szFileName)) > CCHMAXPATH)
        return;

    WinSetDlgItemText (hWnd, IDC_CURRENTDIR, szFullPath);

    /* Search for all files in directory matching the filename */
    strcat (szFullPath, szFileName);    
    if (!DosFindFirst (szFullPath, &hDir, FILE_NORMAL, &FindBuf3,
            sizeof(FILEFINDBUF3), &ulFileCount, FIL_STANDARD))
    {
        do
        {
            szFullPath[ulDirPos] = '\0';
            strncat (szFullPath, FindBuf3.achName, CCHMAXPATH);
            WinSendDlgItemMsg (hWnd, IDC_DIRLIST, LM_INSERTITEM, (MPARAM)LIT_END,
                szFullPath);
            ulFileCount = 1L;
        }
        while (bSearching &&
               !DosFindNext (hDir, &FindBuf3, sizeof(FILEFINDBUF3), &ulFileCount));
        DosFindClose (hDir);
    }

    /* Has the user cancelled the searching? */
    if (!bSearching)
        return;

    /* For each subdirectory in this directory call SearchForFile */
    szFullPath[ulDirPos] = '\0';
    strcat (szFullPath, "*.*");
    ulFileCount = 1L;
    hDir        = HDIR_CREATE;
    if (!DosFindFirst (szFullPath, &hDir, MUST_HAVE_DIRECTORY, &FindBuf3,
            sizeof(FILEFINDBUF3), &ulFileCount, FIL_STANDARD))
    {
        do
        {
            if (!strcmp(FindBuf3.achName, ".") ||
                !strcmp(FindBuf3.achName, ".."))
                continue;

            /* Check for path name length overflow */
            if ((ulDirPos + FindBuf3.cchName + 1) >= CCHMAXPATH)
                continue;

            szFullPath[ulDirPos] = '\0';
            strcat (szFullPath, FindBuf3.achName);
            strcat (szFullPath, "\\");
            SearchForFile (hWnd);
            ulFileCount = 1L;
        }
        while (bSearching &&
               !DosFindNext (hDir, &FindBuf3, sizeof(FILEFINDBUF3), &ulFileCount));
        DosFindClose (hDir);
    }

    return;
}

VOID SetControlStates (HWND hWnd)
{
    /* Show the searching fields */
    WinShowWindow (WinWindowFromID (hWnd, IDC_SEARCHING), bSearching);
    WinShowWindow (WinWindowFromID (hWnd, IDC_CURRENTDIR), bSearching);

    /* Disable filename and drivelist input controls */
    WinEnableControl (hWnd, IDC_FILENAME, !bSearching);
    WinEnableControl (hWnd, IDC_DRIVELIST, !bSearching);

    /* Change button text */
    WinSetDlgItemText (hWnd, IDC_SEARCH, bSearching ? "Stop" : "Search");

    return;
}

VOID StartSearch (HWND hWnd)
{
    ULONG ulInx;

    /* Get file spec to search */
    WinQueryDlgItemText (hWnd, IDC_FILENAME, sizeof(szFileName), szFileName);
    if (!szFileName[0])
    {
        DosBeep (750, 200);
        return;
    }

    /* Get drive selection and start at the root directory */
    ulInx = (ULONG)WinSendDlgItemMsg (hWnd, IDC_DRIVELIST, 
                       LM_QUERYSELECTION, (MPARAM)LIT_FIRST, 0L);
    szFullPath[0] = DriveInfo[ulInx].szDrive[0];
    szFullPath[3] = '\0';

    /* Clear the directory listbox */
    WinSendDlgItemMsg (hWnd, IDC_DIRLIST, LM_DELETEALL, 0L, 0L);

    /* Start the search thread */
    DosCreateThread (&SearchThreadID, (PFNTHREAD)SearchThread, (ULONG)hWnd, 0, 0x10000);

    return;
}

VOID StopSearch ()
{
    /* Signal search thread to stop */
    bSearching = FALSE;

    /* Don't attempt to call DosWaitThread here.  Doing so will hang
       the process since when the search thread returns from the initial
       call to SearchForFile it will call SetControlStates.  The call
       to WinShowWindow, for example, will attempt to send a message to 
       the control window's owner and since the main thread would be
       blocked waiting for the search thread the message could not be
       sent and the search thread would block. 

       The thread will terminate as soon as it sees the bSearching
       flag has been set to FALSE. */

    return;
}

/* ----------------------  Dialog Function ----------------------- */

VOID DoFileSearch (HWND hWnd)
{
    if (!hWndFileSearch)
        hWndFileSearch = WinLoadDlg (HWND_DESKTOP, hWnd, FileSearchDlgProc, 
                             0L, IDD_FILESEARCH, NULL);
    else
        WinSetActiveWindow (HWND_DESKTOP, hWndFileSearch);
        
    return;
}

MRESULT EXPENTRY FileSearchDlgProc (HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    BOOL    bHandled = TRUE;
    MRESULT mReturn  = 0;

    switch (msg)
    {           
        case WM_INITDLG:
            QueryDrives (hWnd);
            WinShowWindow (WinWindowFromID (hWnd, IDC_SEARCHING), FALSE);
            WinShowWindow (WinWindowFromID (hWnd, IDC_CURRENTDIR), FALSE);
            WinSendDlgItemMsg (hWnd, IDC_FILENAME, EM_SETTEXTLIMIT, 
                MPFROMSHORT(CCHMAXPATHCOMP), 0L);
            break;

        case WM_COMMAND:
        case WM_SYSCOMMAND:
		      switch (SHORT1FROMMP(mp1))
            {
                case IDC_SEARCH:
                    if (bSearching)
                        StopSearch ();
                    else
                        StartSearch (hWnd);
                    break;

				    case SC_CLOSE:
                    if (bSearching)
                        StopSearch ();
                    WinDestroyWindow (hWnd);
                    bHandled = TRUE;
                    break;

                default:
                    bHandled = (msg == WM_COMMAND);
            }
            break;

        case WM_DESTROY:
            hWndFileSearch = 0;
            break;

        default:
            bHandled = FALSE;
            break;
    }

    if (!bHandled)
        mReturn = WinDefDlgProc (hWnd, msg, mp1, mp2);

    return (mReturn);
}

