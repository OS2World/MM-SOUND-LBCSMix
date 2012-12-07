/* 

LBCSMix - A mixer for mode 2 Crystal Semiconductor codecs by Lesha Bogdanow
Copyright (C) 1999  Lesha Bogdanow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#define USE_OS2_TOOLKIT_HEADERS
#define INCL_WIN
#define INCL_HELP
#define INCL_WINHEAP
#define INCL_WINMENU		   /* For menu control function */
#define INCL_WINDIALOGS
#define INCL_WINMESSAGEMGR
#define INCL_DOSPROCESS			   /* For init function */
#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#define INCL_BASE
#define INCL_ERRORS
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSFILEMGR
#define INCL_DOSQUEUES
#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSDATETIME


#include <stddef.h>
#include <os2.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


#define RETURN_SUCCESS	      0		 /* successful return in DosExit    */
#define RETURN_ERROR	      1		 /* error return in DosExit	    */
#define BEEP_WARN_FREQ	     60		 /* frequency of warning beep	    */
#define BEEP_WARN_DUR	    100		 /* duration of warning beep	    */
#define MESSAGELEN	     80		 /* maximum length for messages	    */

INT	main(int argc, char *argv[]);
BOOL	Init(VOID);
LONG	MessageBox(HWND, LONG, PSZ, LONG, BOOL);
VOID APIENTRY ExitProc(ULONG);
MRESULT EXPENTRY MainDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY SettingsDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

#define MINLOCKRATE	1
#define MAXLOCKRATE	10000

#include "LBCSMixRes.h"
