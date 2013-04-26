;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "conscriptor"
  OutFile "Setup.exe"

  ;Default installation folder
  InstallDir "C:\Program Files\Conscriptor"
  
  ;Get installation folder from registry if available
  ;InstallDirRegKey HKCU "Software\Modern UI Test" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

  Icon ..\bob.ico
  UninstallIcon ..\bob.ico
;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "conscriptor" conscriptor

  SetOutPath "$INSTDIR"
  
  ;ADD YOUR OWN FILES HERE...
 
;  File ..\conscriptor\debug\conscriptor.exe
  File ..\conscriptor\release\conscriptor.exe
;  File ..\serial\mingw\qextserialport.dll
  File ..\bob.ico
  File mingwm10.dll
  File libgcc_s_dw2-1.dll
  File QtCored4.dll
  File QtCore4.dll
  File QtGuid4.dll
  File QtGui4.dll
  File libstdc++-6.dll

  File /r imageformats
  File /r ..\*.bdf
  File /r ..\*.b4f
  File /r ..\doc

;create desktop shortcut
  CreateShortCut "$DESKTOP\conscriptor.lnk" "$INSTDIR\conscriptor.exe" "" "$INSTDIR\bob.ico"
  ;Store installation folder
  ;WriteRegStr HKCU "Software\Modern UI Test" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  createShortCut "$SMPROGRAMS\conscriptor.lnk" "$INSTDIR\conscriptor.exe"

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_conscriptor ${LANG_ENGLISH} "conscriptor"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${conscriptor} $(DESC_conscriptor)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

;  ;ADD YOUR OWN FILES HERE...

  RMDir /r "$INSTDIR"

  delete "$SMPROGRAMS\conscriptor.lnk"
  delete "$DESKTOP\conscriptor.lnk"

;  DeleteRegKey /ifempty HKCU "Software\Modern UI Test"

SectionEnd
