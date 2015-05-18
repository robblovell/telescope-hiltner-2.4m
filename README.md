# telescope-hiltner-2.4m
Telescope control system for MDM Hiltner 2.4 Meter Telescope on Kitt Peak.

## Computer

This control system is built for an old and ancient museum piece of a computer from circa 1993.
The computer runs version 2.1 of IBM OS/2 and is compatible with dos computers of that era.
The computer has two floppy (gasp) drives, one 3.25 inch drive, and one 5.5 inch drive.

## Booting

It currently doesn't boot properly and a series of incantations must be performed if the computer is turned off completely.
To boot the computer from a cold start, perform the following seance:

    1. Hold the del key on power up so that it opens to the bios editing screen
    2. Modify/Set 4 bios parameters by mousing to the correct icon in each of three areas and selecting the appropriate icon.
    There are four areas with groups of icons, you will be visiting three of these areas.
        * Area 1: The right top area, select and auto detect the hard drive.  This will tell the computer where the C: hard drive is which will allow it to boot.
        * Area 2: The bottom right area, select "Optimal Settings".  If you don't do this, the computer does nothing, ignoring you in spiteful silence.
        * Area 3: The top left area, setup the 2 floppy drives.  This is optional.  Set the 5.5 drive to b: and teh 3.25 drive to a:.  Select highest density (1.44 for the 3.25 drive)
    3. If you have done this right, press <esc> and save the settings and the computer will boot into OS/2.

## Building

Building the Hiltner 2.4 meter TCS is somewhat cryptic.  The code is located on the computer in the c:/usr/mdm/hiltner directory.
A copy of Borland C++ is installed on the computer and a project file "Hiltner.prj" can be opened with it.
Once opening the project, just select build all.

## Files

* .CPP:  These consist of source code.  Only files included in the Hiltner.prj file are used.
* .H:    Header files.
* low     More source code for any code that talks to an install i/o board.
* iopl    home of iopl.asm which performs direct memory manipulation of i/o boards.  This code must access protected * memory and is the reason the Hiltner.def file contains the line:
```
SEGMENTS
        _IOSEG IOPL
```
* .def   Tells the compiler how to link the program and specifies what code access protected memory and what code is used by the display.
* .rc    Resource files for the windows display code.  This arranges the text and icons in the hiltner window when the program is running.
* .dlg   Included in the .rc files to arrange text on the screen.

## Display

The main TCS computer runs without a display window.
This is accomplished by removing the Hiltner.ico file from the directory and commenting out the "POINTER ID_APPNAME HILTNER.ICO" line in the hiltner.rc file.
Note, the hiltner.cpp file also opens a window of 0,0 size.

## Auto-boot Setup

The hiltner.exe file is "auto-booted" when the computer starts.  This is accomplished by placing "PROGRAMS" in the autostart line in the autoexec.bat file.
Once the autoexec.bat file has PROGRAMS on the autostart line, anything running when the computer is shutdown through the launch control will start on the next boot.
Note that all the placement of windows and any other program will also start, so make sure a minimum set of programs are running.
Make sure Hiltner.exe is running when you shutdown so that the next time the computer boots, it will run.

If you need to rebuild the code, you will need to remove the "PROGRAMS" line from the autoexec.bat and reboot so that you can write the hiltner.exe file.



