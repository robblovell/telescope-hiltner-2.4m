#define INCL_DOS
#include <os2.h>
#include <stdio.h>
extern "C" {extern USHORT  _far16 _pascal IN_PORT (USHORT);
extern void  _far16 _pascal OUT_PORT (USHORT,USHORT);
}