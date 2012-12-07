/*  LBCSMix installation script by Lesha Bogdanow */
IniFile="USER"
ObjectName="LB Crystal Mixer"
AppName="LBCSMix"
PgmName="LBCSMix.exe"

call RxFuncAdd "SysLoadFuncs","REXXUTIL","SysLoadFuncs"
call SysLoadFuncs

PgmName=stream(PgmName,'c','query exists')
Config=insert(SysBootDrive(),"\CONFIG.SYS")
Port=''
Line=''
AUX1=''
AUX2=''
Mono=''

say
say "This script will create a desktop object and initial configuration for LBCSMix."
say
say "Please specify your sound card:"
say
say "1. - Turtle Beach TBS-2000"
say "0. - Other"
pull sel
sel=left(sel,1)
if sel="1" then do
   Line='MIDI'
   AUX1='Line In'
   AUX2='CD'
   Mono='Mono'
   end

do while chars(Config)<>0
   Next=linein(Config)
   if (translate(left(word(Next,1),7))="BASEDEV")&(pos('=',Next)<>0) then do
      Next=substr(Next,pos('=',Next)+1)
      if translate(word(Next,1))="CWCONFIG.SYS" then do
         i=1
         do while word(Next,i)<>''
            if translate(left(word(Next,i),3))='/P:' then
               Port=substr(word(Next,i),4)
            i=i+1
            end
         end
      end
   end
if Port=='' then say "Base port can not be determined."


if (Port<>'')|(Line<>'')|(AUX1<>'')|(AUX2<>'')|(Mono<>'') then do
   if (Port<>'') then SysIni(IniFile,AppName,"BasePort",insert(Port,D2C(0)))
   if (Line<>'') then SysIni(IniFile,AppName,"LineName",insert(Line,D2C(0)))
   if (AUX1<>'') then SysIni(IniFile,AppName,"AUX1Name",insert(AUX1,D2C(0)))
   if (AUX2<>'') then SysIni(IniFile,AppName,"AUX2Name",insert(AUX2,D2C(0)))
   if (Mono<>'') then SysIni(IniFile,AppName,"MonoName",insert(Mono,D2C(0)))
   end

if PgmName<>'' then 
   call SysCreateObject "WPProgram",ObjectName,"<WP_DESKTOP>",insert("EXENAME=",PgmName)
