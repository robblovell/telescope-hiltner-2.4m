/* --------------------------------------------------------------------
                         Controls Header File
                              Chapter 10

                    Real World Programming for OS/2
             Copyright (c) 1993 Blain, Delimon, and English
-------------------------------------------------------------------- */

/*************************************************
               GENERAL USE FUNCTIONS
 ************************************************/

VOID EXPENTRY CenterWindow (HWND);


/*************************************************
                 BEVEL CONTROL
 ************************************************/

#define BEVELCLASS       "BEVEL"

/* Use low word of style for private style flags */
#define BVS_BEVELIN     0x00000000
#define BVS_BEVELOUT    0x00000001
#define BVS_RECTANGLE   0x00000000
#define BVS_LINE        0x00000002
#define BVS_FILL        0x00000004
#define BVS_PREVID      0x00000008
#define BVS_PREVWINDOW  0x00000010

#define BVS_IDMASK      0x00008000

/* Bevel control messages */
#define BVM_SETCOLOR     WM_USER+1
#define BVM_SETTHICKNESS WM_USER+2
#define BVM_RESIZE       WM_USER+3


/*************************************************
              THERMOMETER CONTROL
 ************************************************/

#define THERMOMETERCLASS "THERMOMETER"

/* Use low word of style for private style flags */
#define THS_NOPERCENT    0x00000001

/* Thermometer control messages */
#define THM_SETRANGE     WM_USER+1
#define THM_SETVALUE     WM_USER+2
#define THM_SETCOLOR     WM_USER+3

