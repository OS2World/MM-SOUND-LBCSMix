/*  LBCSMix update script by Lesha Bogdanow */
IniFile="USER"
AppName="LBCSMix"
KeyName="WindowPos"

call RxFuncAdd "SysLoadFuncs","REXXUTIL","SysLoadFuncs"
call SysLoadFuncs

SysIni(IniFile,AppName,KeyName,'DELETE:')
