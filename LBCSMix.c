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

#include "LBCSMix.h"

HAB   hab;			    /* anchor block for the process */
HMQ   hmq;		    /* handle to the process' message queue */
HWND  hwndMain;			/* handle to the main client window */
HSWITCH hswitch;
HEV	hev;			//My semaphore

CHAR  szTitle[MESSAGELEN];

CHAR	szIniApp[]="LBCSMix";
CHAR	szIniKeyBase[]="BasePort";
CHAR	szIniKeyLock[]="LockRate";
CHAR	szIniKeyAUX1Name[]="AUX1Name";
CHAR	szIniKeyAUX2Name[]="AUX2Name";
CHAR	szIniKeyLineName[]="LineName";
CHAR	szIniKeyMonoName[]="MonoName";
CHAR	szIniKeyRestore[]="RestoreState";
CHAR	szIniKeyDACLock[]="DACLock";
CHAR	szIniKeyADCLock[]="ADCLock";
CHAR	szIniKeyAUX1Lock[]="AUX1Lock";
CHAR	szIniKeyAUX2Lock[]="AUX2Lock";
CHAR	szIniKeyLineLock[]="LineLock";
CHAR	szIniKeyMonoLock[]="MonoLock";
CHAR	szIniKeyLoopbackLock[]="LoopbackLock";
CHAR	szIniKeyDACBoth[]="DACBoth";
CHAR	szIniKeyADCBoth[]="ADCBoth";
CHAR	szIniKeyAUX1Both[]="AUX1Both";
CHAR	szIniKeyAUX2Both[]="AUX2Both";
CHAR	szIniKeyLineBoth[]="LineBoth";
CHAR	szIniKeyDACLeft[]="DACLeft";
CHAR	szIniKeyDACRight[]="DACRight";
CHAR	szIniKeyADCLeft[]="ADCLeft";
CHAR	szIniKeyADCRight[]="ADCRight";
CHAR	szIniKeyAUX1Left[]="AUX1Left";
CHAR	szIniKeyAUX1Right[]="AUX1Right";
CHAR	szIniKeyAUX2Left[]="AUX2Left";
CHAR	szIniKeyAUX2Right[]="AUX2Right";
CHAR	szIniKeyLineLeft[]="LineLeft";
CHAR	szIniKeyLineRight[]="LineRight";
CHAR	szIniKeyMono[]="Mono";
CHAR	szIniKeyLoopback[]="Loopback";
CHAR	szKeyWindowPos[]="WindowPos";
CHAR    szSemaphore[]="\\SEM32\\LBCSMIX";

int	BasePort=0;
int	LockRate=0;
int	AccessedPort=0;
int	Changing=FALSE;		// Set to TRUE while setting sliders

int	DACLeft,DACRight;
int	AUX1Left,AUX1Right;
int	AUX2Left,AUX2Right;
int	LineLeft,LineRight;
int	Mono;
int	Loopback;
int	ADCLeft,ADCRight;


int	DACLock=FALSE;
int	AUX1Lock=FALSE;
int	AUX2Lock=FALSE;
int	LineLock=FALSE;
int	MonoLock=FALSE;
int	LoopbackLock=FALSE;
int	ADCLock=FALSE;
int	RestoreState=FALSE;
int	DACBoth=FALSE;
int	AUX1Both=FALSE;
int	AUX2Both=FALSE;
int	LineBoth=FALSE;
int	ADCBoth=FALSE;

int	LockPriority=PRTYC_TIMECRITICAL;
TID	LockTID=0;

int AllCtrls[]={IDC_LEFTVOL,IDC_RIGHTVOL,IDC_LEFTMUTE,IDC_RIGHTMUTE,
                IDC_DACBOTH,IDC_AUX1GRP,IDC_AUX2GRP,IDC_LINEGRP,
                IDC_MONOGRP,IDC_AUX1LEFTVOL,IDC_AUX1RIGHTVOL,
                IDC_AUX1LEFTMUTE,IDC_AUX1RIGHTMUTE,IDC_AUX1BOTH,
                IDC_AUX2LEFTVOL,IDC_AUX2RIGHTVOL,IDC_AUX2LEFTMUTE,
                IDC_AUX2RIGHTMUTE,IDC_AUX2BOTH,IDC_LINELEFTVOL,
                IDC_LINERIGHTVOL,IDC_LINELEFTMUTE,IDC_LINERIGHTMUTE,
                IDC_LINEBOTH,IDC_LOOPEN,IDC_LOOPVOL,IDC_MONOVOL,
                IDC_MONOBYPASS,IDC_MONOIMUTE,IDC_MONOOMUTE,IDC_DACGRP,
	        IDC_LOOPGRP,IDC_ADCLEFTVOL,IDC_ADCRIGHTVOL,IDC_ADCLEFT20,
	        IDC_ADCRIGHT20,IDC_ADCBOTH,IDC_ADCLEFTSRC,IDC_ADCRIGHTSRC,
	        IDC_ADCGRP,IDC_ADCSOURCE, IDC_ADC20DB,
                0};

int AllLocks[]={IDC_DACLOCK,IDC_AUX1LOCK,IDC_AUX2LOCK,IDC_LINELOCK,
                IDC_MONOLOCK,IDC_LOOPLOCK,IDC_ADCLOCK,
		0};

void SetRegister(unsigned char idx,unsigned char data) {
   if (BasePort) {
      if (BasePort!=AccessedPort) {
         _portaccess(BasePort,BasePort+1);
         AccessedPort=BasePort;
         }
      _outp8(BasePort,idx);
      _outp8(BasePort+1,data);
      }
   }

unsigned char GetRegister(unsigned char idx) {
   if (BasePort) {
      if (BasePort!=AccessedPort) {
         _portaccess(BasePort,BasePort+1);
         AccessedPort=BasePort;
         }
      _outp8(BasePort,idx);
      return _inp8(BasePort+1);
      }
   return 0;
   }

void DoLock() {

   if (DACLock) {
      SetRegister(6,DACLeft);
      SetRegister(7,DACRight);
      }
   if (AUX1Lock) {
      SetRegister(2,AUX1Left);
      SetRegister(3,AUX1Right);
      }
   if (AUX2Lock) {
      SetRegister(4,AUX2Left);
      SetRegister(5,AUX2Right);
      }
   if (LineLock) {
      SetRegister(18,LineLeft);
      SetRegister(19,LineRight);
      }
   if (LoopbackLock) SetRegister(13,Loopback);
   if (MonoLock) SetRegister(26,Mono);
   if (ADCLock) {
      SetRegister(0,ADCLeft);
      SetRegister(1,ADCRight);
      }

   return;
   }

VOID APIENTRY LockThread(ULONG Dummy) {
   int	Pause;

   DosSetPriority(PRTYS_THREAD, LockPriority, 0, 0);

   do {
      Pause=LockRate;
      if (Pause) {
         DosSleep(Pause);
         DoLock();
         }
      } while (Pause);
   LockTID=0;
   return;
   }


INT main(int argc, char *argv[])
{
   QMSG qmsg;					      /* message structure */

   if (DosCreateEventSem(szSemaphore,&hev,DC_SEM_SHARED,FALSE)) {
      DosBeep(BEEP_WARN_FREQ, BEEP_WARN_DUR);
      return(RETURN_ERROR);
      }

   hab = WinInitialize(0);
   if(!hab)  {
       DosBeep(BEEP_WARN_FREQ, BEEP_WARN_DUR);
       return(RETURN_ERROR);
   }
   hmq = WinCreateMsgQueue(hab, 0);
   if(!hmq)  {
       DosBeep(BEEP_WARN_FREQ, BEEP_WARN_DUR);
       WinTerminate(hab);
       return(RETURN_ERROR);
   }
   if(!Init()) {
       MessageBox(HWND_DESKTOP, IDMSG_INITFAILED, "Error !",
			   MB_OK | MB_ERROR | MB_MOVEABLE, TRUE);
       DosExit(EXIT_PROCESS, RETURN_ERROR);
       }
/*   InitHelp(); */

   while(WinGetMsg(hmq, (PQMSG)&qmsg, 0L, 0L, 0L))
	   WinDispatchMsg(hmq, (PQMSG)&qmsg);
					   /* destroy the help instance */
/*   DestroyHelpInstance(); */
				    /* will normally be put in ExitProc */
//   WinStopTimer(hab,hwndMain,MY_TIMER);
   if (LockTID) DosKillThread(LockTID);
   DosExit(EXIT_PROCESS, RETURN_SUCCESS);
   return 0;
}							       /* main() */

BOOL Init(VOID)
{
    SWCNTRL SwData;
    PID pid;

    /* Add ExitProc to the exit list to handle the exit processing */
    if(DosExitList(EXLST_ADD, ExitProc)) return FALSE;

   WinLoadString(hab, 0, IDS_TITLE, MESSAGELEN, szTitle);



/* Create subwindows */
    hwndMain=WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, (PFNWP)MainDlgProc,
			(HMODULE) NULL, ID_RESOURCE, NULL);
    WinSetWindowText(hwndMain,szTitle);
    WinQueryWindowProcess(hwndMain,&pid,NULL);
    SwData.hwnd=hwndMain;
    SwData.hwndIcon=NULLHANDLE;
    SwData.hprog=NULLHANDLE;
    SwData.idProcess=pid;
    SwData.idSession=0;
    SwData.uchVisibility=SWL_VISIBLE;
    SwData.fbJump=SWL_JUMPABLE;
    WinLoadString(hab, 0, IDS_SWTITLE, MESSAGELEN, SwData.szSwtitle);
    hswitch=WinCreateSwitchEntry(hab,&SwData);
    return TRUE;
}							  /* Init() */

VOID APIENTRY ExitProc(ULONG usTermCode)
				  /* code for the reason for termination */
{
   WinDestroyWindow(hwndMain);



   WinDestroyMsgQueue(hmq);

   WinTerminate(hab);
   DosCloseEventSem(hev);
   DosExitList(EXLST_EXIT, (PFNEXITLIST)0L);	/* termination complete */

   return;
}							   /* ExitProc() */
LONG MessageBox(HWND hwndOwner, LONG IdMsg, PSZ pszMsg, LONG fsStyle,
		     BOOL bBeep)
{
    CHAR szText[MESSAGELEN];
    LONG usRet;

    if(!WinLoadMessage(hab, (HMODULE)NULL, IdMsg, MESSAGELEN, (PSZ)szText)) {
	WinAlarm(HWND_DESKTOP, WA_ERROR);
	return RETURN_ERROR;
        }
    if(bBeep) WinAlarm(HWND_DESKTOP, WA_ERROR);

    usRet = WinMessageBox(HWND_DESKTOP,
			 hwndOwner,
			 szText,
			 (PSZ)pszMsg,
			 IDM_MSGBOX,
			 fsStyle);
    return usRet;
}						    /* MessageBox() */


void EnableCtrls() {
   int i,State;

   if (BasePort) State=TRUE;
   else State=FALSE;
   for (i=0;AllCtrls[i];i++)
      WinEnableWindow(WinWindowFromID(hwndMain,AllCtrls[i]),State);

   if (LockRate) State=TRUE;
   else State=FALSE;
   for (i=0;AllLocks[i];i++)
      WinEnableWindow(WinWindowFromID(hwndMain,AllLocks[i]),State);
   }

void LoadCombos() {
   char Buff[4][32];
   int i,SetL,SetR;

   Changing=TRUE;
   SetL=(int)WinSendDlgItemMsg(hwndMain, IDC_ADCLEFTSRC,
                  LM_QUERYSELECTION,(MPARAM)NULL,(MPARAM)NULL);
   SetR=(int)WinSendDlgItemMsg(hwndMain, IDC_ADCRIGHTSRC,
                  LM_QUERYSELECTION,(MPARAM)NULL,(MPARAM)NULL);

   WinSendMsg(WinWindowFromID(hwndMain,IDC_ADCLEFTSRC),LM_DELETEALL,
      (MPARAM)NULL,(MPARAM)NULL );
   WinSendMsg(WinWindowFromID(hwndMain,IDC_ADCRIGHTSRC),LM_DELETEALL,
      (MPARAM)NULL,(MPARAM)NULL );

   WinQueryWindowText(WinWindowFromID(hwndMain,IDC_LINEGRP),32,Buff[0]);
   WinQueryWindowText(WinWindowFromID(hwndMain,IDC_AUX1GRP),32,Buff[1]);
   WinLoadString(hab, 0, IDS_MIC,32, Buff[2]);
   WinLoadString(hab, 0, IDS_MIX,32, Buff[3]);

   for (i=0;i<4;i++) {
      WinSendMsg(WinWindowFromID(hwndMain,IDC_ADCLEFTSRC),LM_INSERTITEM,
         (MPARAM)LIT_END,MPFROMP(Buff[i]));
      WinSendMsg(WinWindowFromID(hwndMain,IDC_ADCRIGHTSRC),LM_INSERTITEM,
         (MPARAM)LIT_END,MPFROMP(Buff[i]));
      }

   if (SetL!=LIT_NONE)
      WinSendMsg(WinWindowFromID(hwndMain,IDC_ADCLEFTSRC),LM_SELECTITEM,
                          (MPARAM)(SetL),(MPARAM)TRUE );
   if (SetR!=LIT_NONE)
      WinSendMsg(WinWindowFromID(hwndMain,IDC_ADCRIGHTSRC),LM_SELECTITEM,
                          (MPARAM)(SetR),(MPARAM)TRUE );
   Changing=FALSE;

   return;
   }

void SaveState() {
   char Buff[16];

   _itoa(DACLock,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyDACLock,Buff);
   _itoa(ADCLock,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyADCLock,Buff);
   _itoa(AUX1Lock,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Lock,Buff);
   _itoa(AUX2Lock,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Lock,Buff);
   _itoa(LineLock,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineLock,Buff);
   _itoa(MonoLock,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyMonoLock,Buff);
   _itoa(LoopbackLock,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLoopbackLock,Buff);

   _itoa(DACBoth,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyDACBoth,Buff);
   _itoa(ADCBoth,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyADCBoth,Buff);
   _itoa(AUX1Both,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Both,Buff);
   _itoa(AUX2Both,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Both,Buff);
   _itoa(LineBoth,Buff,10);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineBoth,Buff);
   _itoa(MonoLock,Buff,10);

   _itoa(DACLeft,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyDACLeft,Buff);
   _itoa(DACRight,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyDACRight,Buff);
   _itoa(ADCLeft,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyADCLeft,Buff);
   _itoa(ADCRight,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyADCRight,Buff);
   _itoa(AUX1Left,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Left,Buff);
   _itoa(AUX1Right,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Right,Buff);
   _itoa(AUX2Left,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Left,Buff);
   _itoa(AUX2Right,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Right,Buff);
   _itoa(LineLeft,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineLeft,Buff);
   _itoa(LineRight,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineRight,Buff);
   _itoa(Mono,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyMono,Buff);
   _itoa(Loopback,Buff,16);
   PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLoopback,Buff);

   }

void LoadMxState() {
   DACLeft=GetRegister(6);
   DACRight=GetRegister(7);
   AUX1Left=GetRegister(2);
   AUX1Right=GetRegister(3);
   AUX2Left=GetRegister(4);
   AUX2Right=GetRegister(5);
   LineLeft=GetRegister(18);
   LineRight=GetRegister(19);
   ADCLeft=GetRegister(0);
   ADCRight=GetRegister(1);
   Loopback=GetRegister(13);
   Mono=GetRegister(26);
   return;
   }


void SetDACLeft() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_LEFTVOL,SLM_SETSLIDERINFO,
            MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            (MPARAM)((DACLeft<<1)&0x7F));
   WinSendDlgItemMsg(hwndMain, IDC_LEFTMUTE,BM_SETCHECK,
            (MPARAM)!(DACLeft>>7),0);
   SetRegister(6,DACLeft);
   Changing=FALSE;
   return;
   }

void SetDACRight() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_RIGHTVOL,SLM_SETSLIDERINFO,
                     MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                     (MPARAM)((DACRight<<1)&0x7F));
   WinSendDlgItemMsg(hwndMain, IDC_RIGHTMUTE,BM_SETCHECK,
                     (MPARAM)!(DACRight>>7),0);
   SetRegister(7,DACRight);
   Changing=FALSE;
   return;
   }

void SetAUX1Left() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_AUX1LEFTVOL,SLM_SETSLIDERINFO,
            MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            (MPARAM)(AUX1Left&0x1F));
   WinSendDlgItemMsg(hwndMain, IDC_AUX1LEFTMUTE,BM_SETCHECK,
            (MPARAM)!(AUX1Left>>7),0);
   SetRegister(2,AUX1Left);
   Changing=FALSE;
   return;
   }

void SetAUX1Right() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_AUX1RIGHTVOL,SLM_SETSLIDERINFO,
            MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            (MPARAM)(AUX1Right&0x1F));
   WinSendDlgItemMsg(hwndMain, IDC_AUX1RIGHTMUTE,BM_SETCHECK,
            (MPARAM)!(AUX1Right>>7),0);
   SetRegister(3,AUX1Right);
   Changing=FALSE;
   return;
   }

void SetAUX2Left() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_AUX2LEFTVOL,SLM_SETSLIDERINFO,
            MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            (MPARAM)(AUX2Left&0x1F));
   WinSendDlgItemMsg(hwndMain, IDC_AUX2LEFTMUTE,BM_SETCHECK,
            (MPARAM)!(AUX2Left>>7),0);
   SetRegister(4,AUX2Left);
   Changing=FALSE;
   return;
   }

void SetAUX2Right() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_AUX2RIGHTVOL,SLM_SETSLIDERINFO,
            MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            (MPARAM)(AUX2Right&0x1F));
   WinSendDlgItemMsg(hwndMain, IDC_AUX2RIGHTMUTE,BM_SETCHECK,
            (MPARAM)!(AUX2Right>>7),0);
   SetRegister(5,AUX2Right);
   Changing=FALSE;
   return;
   }

void SetLineLeft() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_LINELEFTVOL,SLM_SETSLIDERINFO,
            MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            (MPARAM)(LineLeft&0x1F));
   WinSendDlgItemMsg(hwndMain, IDC_LINELEFTMUTE,BM_SETCHECK,
            (MPARAM)!(LineLeft>>7),0);
   SetRegister(18,LineLeft);
   Changing=FALSE;
   return;
   }

void SetLineRight() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_LINERIGHTVOL,SLM_SETSLIDERINFO,
            MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            (MPARAM)(LineRight&0x1F));
   WinSendDlgItemMsg(hwndMain, IDC_LINERIGHTMUTE,BM_SETCHECK,
            (MPARAM)!(LineRight>>7),0);
   SetRegister(19,LineRight);
   Changing=FALSE;
   return;
   }

void SetADCLeft() {
   Changing=TRUE;
   WinSendMsg(WinWindowFromID(hwndMain,IDC_ADCLEFTSRC),LM_SELECTITEM,
                           (MPARAM)(ADCLeft>>6),(MPARAM)TRUE );
   WinSendDlgItemMsg(hwndMain, IDC_ADCLEFTVOL,SLM_SETSLIDERINFO,
                       MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                       (MPARAM)(ADCLeft&0xF));
   WinSendDlgItemMsg(hwndMain, IDC_ADCLEFT20,BM_SETCHECK,
                       (MPARAM)((ADCLeft>>5)&1),0);
   SetRegister(0,ADCLeft);
   Changing=FALSE;
   return;
   }

void SetADCRight() {
   Changing=TRUE;
   WinSendMsg(WinWindowFromID(hwndMain,IDC_ADCRIGHTSRC),LM_SELECTITEM,
                           (MPARAM)(ADCRight>>6),(MPARAM)TRUE );
   WinSendDlgItemMsg(hwndMain, IDC_ADCRIGHTVOL,SLM_SETSLIDERINFO,
                       MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
                       (MPARAM)(ADCRight&0xF));
   WinSendDlgItemMsg(hwndMain, IDC_ADCRIGHT20,BM_SETCHECK,
                       (MPARAM)((ADCRight>>5)&1),0);
   SetRegister(1,ADCRight);
   Changing=FALSE;
   return;
   }

void SetLoopback() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_LOOPVOL,SLM_SETSLIDERINFO,
            MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            (MPARAM)(Loopback>>2));
   WinSendDlgItemMsg(hwndMain, IDC_LOOPEN,BM_SETCHECK,
            (MPARAM)(Loopback&1),0);
   SetRegister(13,Loopback);
   Changing=FALSE;
   return;
   }

void SetMono() {
   Changing=TRUE;
   WinSendDlgItemMsg(hwndMain, IDC_MONOVOL,SLM_SETSLIDERINFO,
            MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE),
            (MPARAM)(Mono&0xF));
   WinSendDlgItemMsg(hwndMain, IDC_MONOBYPASS,BM_SETCHECK,
            (MPARAM)((Mono>>5)&1),0);
   WinSendDlgItemMsg(hwndMain, IDC_MONOOMUTE,BM_SETCHECK,
            (MPARAM)((Mono>>6)&1),0);
   WinSendDlgItemMsg(hwndMain, IDC_MONOIMUTE,BM_SETCHECK,
            (MPARAM)!(Mono>>7),0);
   SetRegister(26,Mono);
   Changing=FALSE;
   return;
   }

void SetCtrls() {
   SetDACLeft();
   SetDACRight();
   SetADCLeft();
   SetADCRight();
   SetAUX1Left();
   SetAUX1Right();
   SetAUX2Left();
   SetAUX2Right();
   SetLineLeft();
   SetLineRight();
   SetLoopback();
   SetMono();

   WinSendDlgItemMsg(hwndMain,IDC_DACLOCK,BM_SETCHECK,(MPARAM)DACLock,0);
   WinSendDlgItemMsg(hwndMain,IDC_ADCLOCK,BM_SETCHECK,(MPARAM)ADCLock,0);
   WinSendDlgItemMsg(hwndMain,IDC_AUX1LOCK,BM_SETCHECK,(MPARAM)AUX1Lock,0);
   WinSendDlgItemMsg(hwndMain,IDC_AUX2LOCK,BM_SETCHECK,(MPARAM)AUX2Lock,0);
   WinSendDlgItemMsg(hwndMain,IDC_LINELOCK,BM_SETCHECK,(MPARAM)LineLock,0);
   WinSendDlgItemMsg(hwndMain,IDC_MONOLOCK,BM_SETCHECK,(MPARAM)MonoLock,0);
   WinSendDlgItemMsg(hwndMain,IDC_LOOPLOCK,BM_SETCHECK,(MPARAM)LoopbackLock,0);
   WinSendDlgItemMsg(hwndMain,IDC_DACBOTH,BM_SETCHECK,(MPARAM)DACBoth,0);
   WinSendDlgItemMsg(hwndMain,IDC_ADCBOTH,BM_SETCHECK,(MPARAM)ADCBoth,0);
   WinSendDlgItemMsg(hwndMain,IDC_AUX1BOTH,BM_SETCHECK,(MPARAM)AUX1Both,0);
   WinSendDlgItemMsg(hwndMain,IDC_AUX2BOTH,BM_SETCHECK,(MPARAM)AUX2Both,0);
   WinSendDlgItemMsg(hwndMain,IDC_LINEBOTH,BM_SETCHECK,(MPARAM)LineBoth,0);
   
   return;
   }

void MainControl(HWND hwnd,MPARAM mp1) {
   int Setting;

   if (!Changing) switch(LOUSHORT(mp1)) {
      case IDC_DACLOCK:
         DACLock=(int)WinSendDlgItemMsg(hwnd, IDC_DACLOCK,BM_QUERYCHECK,0,0);
         break;
      case IDC_AUX1LOCK:
         AUX1Lock=(int)WinSendDlgItemMsg(hwnd, IDC_AUX1LOCK,BM_QUERYCHECK,0,0);
         break;
      case IDC_AUX2LOCK:
         AUX2Lock=(int)WinSendDlgItemMsg(hwnd, IDC_AUX2LOCK,BM_QUERYCHECK,0,0);
         break;
      case IDC_LINELOCK:
         LineLock=(int)WinSendDlgItemMsg(hwnd, IDC_LINELOCK,BM_QUERYCHECK,0,0);
         break;
      case IDC_LOOPLOCK:
         LoopbackLock=(int)WinSendDlgItemMsg(hwnd, IDC_LOOPLOCK,BM_QUERYCHECK,0,0);
         break;
      case IDC_MONOLOCK:
         MonoLock=(int)WinSendDlgItemMsg(hwnd, IDC_MONOLOCK,BM_QUERYCHECK,0,0);
         break;
      case IDC_ADCLOCK:
         ADCLock=(int)WinSendDlgItemMsg(hwnd, IDC_ADCLOCK,BM_QUERYCHECK,0,0);
         break;
      case IDC_DACBOTH:
         DACBoth=(int)WinSendDlgItemMsg(hwnd, IDC_DACBOTH,BM_QUERYCHECK,0,0);
         break;
      case IDC_AUX1BOTH:
         AUX1Both=(int)WinSendDlgItemMsg(hwnd, IDC_AUX1BOTH,BM_QUERYCHECK,0,0);
         break;
      case IDC_AUX2BOTH:
         AUX2Both=(int)WinSendDlgItemMsg(hwnd, IDC_AUX2BOTH,BM_QUERYCHECK,0,0);
         break;
      case IDC_LINEBOTH:
         LineBoth=(int)WinSendDlgItemMsg(hwnd, IDC_LINEBOTH,BM_QUERYCHECK,0,0);
         break;
      case IDC_ADCBOTH:
         ADCBoth=(int)WinSendDlgItemMsg(hwnd, IDC_ADCBOTH,BM_QUERYCHECK,0,0);
         break;
      case IDC_LEFTVOL:
      case IDC_LEFTMUTE:
         Setting=0x7F&(int)WinSendDlgItemMsg(hwnd, IDC_LEFTVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
         Setting>>=1;
         if (!WinSendDlgItemMsg(hwnd, IDC_LEFTMUTE,BM_QUERYCHECK,0,0)) Setting|=0x80;
         DACLeft=Setting;
         SetRegister(6,Setting);
         if (DACBoth) {
            DACRight=Setting;
            SetDACRight();
            }
         break;
      case IDC_RIGHTVOL:
      case IDC_RIGHTMUTE:
         Setting=0x7F&(int)WinSendDlgItemMsg(hwnd, IDC_RIGHTVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
         Setting>>=1;
         if (!WinSendDlgItemMsg(hwnd, IDC_RIGHTMUTE,BM_QUERYCHECK,0,0)) Setting|=0x80;
         DACRight=Setting;
         SetRegister(7,Setting);
         if (DACBoth) {
            DACLeft=Setting;
            SetDACLeft();
            }
         break;
      case IDC_AUX1LEFTVOL:
      case IDC_AUX1LEFTMUTE:
         Setting=0x1F&(int)WinSendDlgItemMsg(hwnd, IDC_AUX1LEFTVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
         if (!WinSendDlgItemMsg(hwnd, IDC_AUX1LEFTMUTE,BM_QUERYCHECK,0,0)) Setting|=0x80;
         AUX1Left=Setting;
         SetRegister(2,Setting);
         if (AUX1Both) {
            AUX1Right=Setting;
            SetAUX1Right();
            }
         break;
      case IDC_AUX1RIGHTVOL:
      case IDC_AUX1RIGHTMUTE:
         Setting=0x1F&(int)WinSendDlgItemMsg(hwnd, IDC_AUX1RIGHTVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
         if (!WinSendDlgItemMsg(hwnd, IDC_AUX1RIGHTMUTE,BM_QUERYCHECK,0,0)) Setting|=0x80;
         AUX1Right=Setting;
         SetRegister(3,Setting);
         if (AUX1Both) {
            AUX1Left=Setting;
            SetAUX1Left();
            }
         break;
      case IDC_AUX2LEFTVOL:
      case IDC_AUX2LEFTMUTE:
         Setting=0x1F&(int)WinSendDlgItemMsg(hwnd, IDC_AUX2LEFTVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
         if (!WinSendDlgItemMsg(hwnd, IDC_AUX2LEFTMUTE,BM_QUERYCHECK,0,0)) Setting|=0x80;
         AUX2Left=Setting;
         SetRegister(4,Setting);
         if (AUX2Both) {
            AUX2Right=Setting;
            SetAUX2Right();
            }
         break;
      case IDC_AUX2RIGHTVOL:
      case IDC_AUX2RIGHTMUTE:
         Setting=0x1F&(int)WinSendDlgItemMsg(hwnd, IDC_AUX2RIGHTVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
         if (!WinSendDlgItemMsg(hwnd, IDC_AUX2RIGHTMUTE,BM_QUERYCHECK,0,0)) Setting|=0x80;
         AUX2Right=Setting;
         SetRegister(5,Setting);
         if (AUX2Both) {
            AUX2Left=Setting;
            SetAUX2Left();
            }
         break;
      case IDC_LINELEFTVOL:
      case IDC_LINELEFTMUTE:
         Setting=0x1F&(int)WinSendDlgItemMsg(hwnd, IDC_LINELEFTVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
         if (!WinSendDlgItemMsg(hwnd, IDC_LINELEFTMUTE,BM_QUERYCHECK,0,0)) Setting|=0x80;
         LineLeft=Setting;
         SetRegister(18,Setting);
         if (LineBoth) {
            LineRight=Setting;
            SetLineRight();
            }
         break;
      case IDC_LINERIGHTVOL:
      case IDC_LINERIGHTMUTE:
         Setting=0x1F&(int)WinSendDlgItemMsg(hwnd, IDC_LINERIGHTVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
         if (!WinSendDlgItemMsg(hwnd, IDC_LINERIGHTMUTE,BM_QUERYCHECK,0,0)) Setting|=0x80;
         LineRight=Setting;
         SetRegister(19,Setting);
         if (LineBoth) {
            LineLeft=Setting;
            SetLineLeft();
            }
         break;
      case IDC_LOOPVOL:
      case IDC_LOOPEN:
         Setting=(0x3F&(int)WinSendDlgItemMsg(hwnd, IDC_LOOPVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL))<<2;
         if (WinSendDlgItemMsg(hwnd, IDC_LOOPEN,BM_QUERYCHECK,0,0)) Setting|=1;
         Loopback=Setting;
         SetRegister(13,Setting);
         break;
      case IDC_MONOVOL:
      case IDC_MONOBYPASS:
      case IDC_MONOIMUTE:
      case IDC_MONOOMUTE:
         Setting=0xF&(int)WinSendDlgItemMsg(hwnd, IDC_MONOVOL,SLM_QUERYSLIDERINFO,
 	         MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
         if (WinSendDlgItemMsg(hwnd, IDC_MONOBYPASS,BM_QUERYCHECK,0,0)) Setting|=0x20;
         if (WinSendDlgItemMsg(hwnd, IDC_MONOOMUTE,BM_QUERYCHECK,0,0)) Setting|=0x40;
         if (!WinSendDlgItemMsg(hwnd, IDC_MONOIMUTE,BM_QUERYCHECK,0,0)) Setting|=0x80;
         Mono=Setting;
         SetRegister(26,Setting);
         break;
      case IDC_ADCLEFT20:
      case IDC_ADCLEFTVOL:
      case IDC_ADCLEFTSRC:
          Setting=(int)WinSendDlgItemMsg(hwnd, IDC_ADCLEFTSRC,
                  LM_QUERYSELECTION,(MPARAM)NULL,(MPARAM)NULL);
          if (Setting!=LIT_NONE) {
             Setting<<=6;
             Setting|=0xF&(int)WinSendDlgItemMsg(hwnd, IDC_ADCLEFTVOL,SLM_QUERYSLIDERINFO,
     	                   MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
             if (WinSendDlgItemMsg(hwnd,IDC_ADCLEFT20,BM_QUERYCHECK,0,0)) Setting|=0x20;
             ADCLeft=Setting;
             SetRegister(0,Setting);
             if (ADCBoth) {
                ADCRight=Setting;
                SetADCRight();
                }
             }
         break;
      case IDC_ADCRIGHT20:
      case IDC_ADCRIGHTVOL:
      case IDC_ADCRIGHTSRC:
          Setting=(int)WinSendDlgItemMsg(hwnd, IDC_ADCRIGHTSRC,
                  LM_QUERYSELECTION,(MPARAM)NULL,(MPARAM)NULL);
          if (Setting!=LIT_NONE) {
             Setting<<=6;
             Setting|=0xF&(int)WinSendDlgItemMsg(hwnd, IDC_ADCRIGHTVOL,SLM_QUERYSLIDERINFO,
     	                   MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE),(MPARAM)NULL);
             if (WinSendDlgItemMsg(hwnd,IDC_ADCRIGHT20,BM_QUERYCHECK,0,0)) Setting|=0x20;
             ADCRight=Setting;
             SetRegister(1,Setting);
             if (ADCBoth) {
                ADCLeft=Setting;
                SetADCLeft();
                }
             }
         break;
      }
   return;
   }

void MainCommand(HWND hwnd,MPARAM mp1) {

   switch(LOUSHORT(mp1)) {
      case IDC_SETTINGS:
         WinDlgBox(hwndMain, hwndMain, (PFNWP)SettingsDlgProc,
			(HMODULE) NULL, IDD_SETTINGS, NULL);
         LoadCombos();
         EnableCtrls();
         break;
      case IDC_ABOUT:
         WinDlgBox(hwndMain, hwndMain, (PFNWP)AboutDlgProc,
			(HMODULE) NULL, IDD_ABOUT, NULL);
         break;
      case IDC_EXIT:
         WinPostMsg( hwndMain, WM_CLOSE, (MPARAM)0,(MPARAM)0 );
         break;
      }
   return;
   }

void LoadProfile() {
   char Buff[32];
   unsigned int temp;

   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyBase,NULL,Buff,32)) {
      temp=strtoul(Buff,NULL,16)&0xFFFF;
      if (temp) BasePort=temp;
      }
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLock,NULL,Buff,32)) {
      temp=atoi(Buff);
      if ((temp>=MINLOCKRATE)&&(temp<=MAXLOCKRATE)) LockRate=temp;
      }
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineName,NULL,Buff,32))
      WinSetWindowText(WinWindowFromID(hwndMain,IDC_LINEGRP),Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Name,NULL,Buff,32))
      WinSetWindowText(WinWindowFromID(hwndMain,IDC_AUX1GRP),Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Name,NULL,Buff,32))
      WinSetWindowText(WinWindowFromID(hwndMain,IDC_AUX2GRP),Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyMonoName,NULL,Buff,32))
      WinSetWindowText(WinWindowFromID(hwndMain,IDC_MONOGRP),Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyRestore,NULL,Buff,32))
      RestoreState=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyDACLock,NULL,Buff,32))
      DACLock=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyADCLock,NULL,Buff,32))
      ADCLock=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Lock,NULL,Buff,32))
      AUX1Lock=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Lock,NULL,Buff,32))
      AUX2Lock=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineLock,NULL,Buff,32))
      LineLock=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyMonoLock,NULL,Buff,32))
      MonoLock=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLoopbackLock,NULL,Buff,32))
      LoopbackLock=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyDACBoth,NULL,Buff,32))
      DACBoth=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyADCBoth,NULL,Buff,32))
      ADCBoth=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Both,NULL,Buff,32))
      AUX1Both=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Both,NULL,Buff,32))
      AUX2Both=atoi(Buff);
   if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineBoth,NULL,Buff,32))
      LineBoth=atoi(Buff);

   if (RestoreState) {
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyDACLeft,NULL,Buff,32))
         DACLeft=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyDACRight,NULL,Buff,32))
         DACRight=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyADCLeft,NULL,Buff,32))
         ADCLeft=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyADCRight,NULL,Buff,32))
         ADCRight=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Left,NULL,Buff,32))
         AUX1Left=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Right,NULL,Buff,32))
         AUX1Right=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Left,NULL,Buff,32))
         AUX2Left=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Right,NULL,Buff,32))
         AUX2Right=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineLeft,NULL,Buff,32))
         LineLeft=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineRight,NULL,Buff,32))
         LineRight=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyMono,NULL,Buff,32))
         Mono=strtoul(Buff,NULL,16)&0xFF;
      if (PrfQueryProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLoopback,NULL,Buff,32))
         Loopback=strtoul(Buff,NULL,16)&0xFF;
      }

//   if (LockRate) WinStartTimer(hab,hwndMain,MY_TIMER,LockRate);
     if ((LockRate)&&(!LockTID)) DosCreateThread(&LockTID,LockThread,0,0,8192);
   return;
   }


MRESULT EXPENTRY MainDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT sRC;

     switch (msg) {
       case WM_INITDLG:
          WinRestoreWindowPos(szIniApp,szKeyWindowPos,hwnd);
          WinSendMsg(hwnd,WM_SETICON,
            (MPARAM) WinLoadPointer(HWND_DESKTOP,(HMODULE) NULL,ID_RESOURCE ),
            (MPARAM) 0 );
          WinPostMsg( hwnd, WM_USER, (MPARAM)0,(MPARAM)0 );
 	  break;
       case WM_USER:
          LoadProfile();
          if (!RestoreState) LoadMxState();
          LoadCombos();
          SetCtrls();
          EnableCtrls();
          break;
/*       case WM_TIMER:
          DoLock();
	  break; */
       case WM_CLOSE:
          SaveState();
          WinStoreWindowPos(szIniApp,szKeyWindowPos,hwnd);
          WinPostMsg( hwndMain, WM_QUIT, (MPARAM)0,(MPARAM)0 );
          break;
       case WM_COMMAND:
          MainCommand(hwnd,mp1);
          break;
       case WM_CONTROL:
          MainControl(hwnd,mp1);
          break;
       default:	
	  sRC = WinDefDlgProc(hwnd, msg, mp1, mp2);
	  return sRC;
       }
   return (MRESULT)0L;
}


void SettingsControl(HWND hwnd,MPARAM mp1) {
   char Buff[32];
   int p;

   switch(LOUSHORT(mp1)) {
      case IDC_BASEPORT:
         p=0;
         if (WinQueryWindowText(WinWindowFromID(hwnd,IDC_BASEPORT),32,Buff))
            p=strtoul(Buff,NULL,16)&0xFFFF;
         if (p) WinEnableWindow(WinWindowFromID(hwnd,IDC_OK),TRUE);
         else WinEnableWindow(WinWindowFromID(hwnd,IDC_OK),FALSE);
         break;
      }
   return;
   }
void SettingsCommand(HWND hwnd,MPARAM mp1) {
   char Buff[32];
   unsigned int OldBase;

   switch(LOUSHORT(mp1)) {
      case IDC_CANCEL:
         WinDismissDlg(hwnd, FALSE);
         break;
      case IDC_OK:
         OldBase=BasePort;
         if (WinQueryWindowText(WinWindowFromID(hwnd,IDC_BASEPORT),32,Buff))
            BasePort=strtoul(Buff,NULL,16)&0xFFFF;
         if (WinQueryWindowText(WinWindowFromID(hwnd,IDC_LINENAME),32,Buff))
            WinSetWindowText(WinWindowFromID(hwndMain,IDC_LINEGRP),Buff);
         if (WinQueryWindowText(WinWindowFromID(hwnd,IDC_AUX1NAME),32,Buff))
            WinSetWindowText(WinWindowFromID(hwndMain,IDC_AUX1GRP),Buff);
         if (WinQueryWindowText(WinWindowFromID(hwnd,IDC_AUX2NAME),32,Buff))
            WinSetWindowText(WinWindowFromID(hwndMain,IDC_AUX2GRP),Buff);
         if (WinQueryWindowText(WinWindowFromID(hwnd,IDC_MONONAME),32,Buff))
            WinSetWindowText(WinWindowFromID(hwndMain,IDC_MONOGRP),Buff);
         if (WinQueryWindowText(WinWindowFromID(hwnd,IDC_LOCKRATE),32,Buff)) {
            LockRate=atoi(Buff);
            if ((LockRate<MINLOCKRATE)||(LockRate>MAXLOCKRATE)) LockRate=0;
            }
         else LockRate=0;
         RestoreState=(int)WinSendDlgItemMsg(hwnd,IDC_RESTORE,BM_QUERYCHECK,0,0);
//         if (!LockRate)WinStopTimer(hab,hwndMain,MY_TIMER);
//         if (LockRate) WinStartTimer(hab,hwndMain,MY_TIMER,LockRate);
         if ((!LockRate)&&(LockTID)) {
            DosKillThread(LockTID);
            LockTID=0;
            }
         if ((LockRate)&&(!LockTID)) DosCreateThread(&LockTID,LockThread,0,0,8192);
         _itoa(BasePort,Buff,16);
         PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyBase,Buff);
         _itoa(LockRate,Buff,10);
         PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLock,Buff);
         if (WinQueryWindowText(WinWindowFromID(hwndMain,IDC_LINEGRP),32,Buff))
            PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyLineName,Buff);
         if (WinQueryWindowText(WinWindowFromID(hwndMain,IDC_AUX1GRP),32,Buff))
            PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX1Name,Buff);
         if (WinQueryWindowText(WinWindowFromID(hwndMain,IDC_AUX2GRP),32,Buff))
            PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyAUX2Name,Buff);
         if (WinQueryWindowText(WinWindowFromID(hwndMain,IDC_MONOGRP),32,Buff))
            PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyMonoName,Buff);
         _itoa(RestoreState,Buff,10);
         PrfWriteProfileString(HINI_USERPROFILE,szIniApp,szIniKeyRestore,Buff);
         if (OldBase!=BasePort) {
            LoadMxState();
            SetCtrls();
            }

         WinDismissDlg(hwnd, FALSE);
         break;
      }
   return;
   }
void SettingsInit(HWND hwnd) {
   char Buff[32];

   if (BasePort) {
      _itoa(BasePort,Buff,16);
      WinSetWindowText(WinWindowFromID(hwnd,IDC_BASEPORT),Buff);
      }
   WinQueryWindowText(WinWindowFromID(hwndMain,IDC_LINEGRP),32,Buff);
   WinSetWindowText(WinWindowFromID(hwnd,IDC_LINENAME),Buff);
   WinQueryWindowText(WinWindowFromID(hwndMain,IDC_AUX1GRP),32,Buff);
   WinSetWindowText(WinWindowFromID(hwnd,IDC_AUX1NAME),Buff);
   WinQueryWindowText(WinWindowFromID(hwndMain,IDC_AUX2GRP),32,Buff);
   WinSetWindowText(WinWindowFromID(hwnd,IDC_AUX2NAME),Buff);
   WinQueryWindowText(WinWindowFromID(hwndMain,IDC_MONOGRP),32,Buff);
   WinSetWindowText(WinWindowFromID(hwnd,IDC_MONONAME),Buff);
   if (LockRate) {
      _itoa(LockRate,Buff,10);
      WinSetWindowText(WinWindowFromID(hwnd,IDC_LOCKRATE),Buff);
      }
   WinSendDlgItemMsg(hwnd, IDC_RESTORE,BM_SETCHECK,(MPARAM)RestoreState,0);
   return;
   }


MRESULT EXPENTRY SettingsDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT sRC;

     switch (msg) {
       case WM_INITDLG:
          SettingsInit(hwnd);
 	  break;
       case WM_CLOSE:
          WinDismissDlg(hwnd, FALSE);
          break;
       case WM_COMMAND:
          SettingsCommand(hwnd,mp1);
          break;
       case WM_CONTROL:
          SettingsControl(hwnd,mp1);
          break;
       default:	
	  sRC = WinDefDlgProc(hwnd, msg, mp1, mp2);
	  return sRC;
       }
   return (MRESULT)0L;
}

MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT sRC;

     switch (msg) {
       case WM_INITDLG:
          WinSetWindowText(WinWindowFromID(hwnd,IDC_ABOUTAPP),szTitle);
 	  break;
       case WM_COMMAND:
          if (LOUSHORT(mp1)==IDC_OK) WinDismissDlg(hwnd, TRUE);
          break;
       default:	
	  sRC = WinDefDlgProc(hwnd, msg, mp1, mp2);
	  return sRC;
       }
   return (MRESULT)0L;
}
