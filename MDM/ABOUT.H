/* --------------------------------------------------------------------
                           About Header File

                    Real World Programming for OS/2
             Copyright (c) 1993 Blain, Delimon, and English
-------------------------------------------------------------------- */
// The about procedure is in "about.c".
VOID DisplayAbout (HWND hWnd, PSZ pszAppName);VOID DisplayAbout (HWND,PSZ);

/* Exported Function */
extern "C" {
MRESULT EXPENTRY AboutDlgProc  (HWND,ULONG,MPARAM,MPARAM);
}
