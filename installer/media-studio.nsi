; Media Studio Windows Installer Script
; This NSIS script creates an installer for Media Studio (OBS-based streaming application)

!include "MUI2.nsh"
!include "x64.nsh"
!include "LogicLib.nsh"

; ===== Basic Settings =====
Name "Media Studio"
OutFile "..\build_x64\rundir\RelWithDebInfo\Media-Studio-Installer.exe"
InstallDir "$PROGRAMFILES64\Media Studio"
InstallDirRegKey HKLM "Software\MediaStudio" "Install_Dir"

Var StartMenuFolder

; ===== MUI2 Settings =====
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\frontend\data\license\gplv2.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU "Application" $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "French"

; ===== Installer Configuration =====
SetCompress force
SetDatablockOptimize on
SetOverwrite on
CRCCheck on

; ===== Version Information =====
VIProductVersion "30.0.0.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "Media Studio"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Professional Streaming Application"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright @ 2026"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "30.0.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "30.0.0"

RequestExecutionLevel admin

; ===== Installer Sections =====
Section "!Media Studio Core (Required)" SecCore
    SectionIn RO

    ; Copy main application files
    SetOutPath "$INSTDIR\bin\64bit"
    File /r "..\build_x64\rundir\RelWithDebInfo\bin\64bit\*.*"
    SetOutPath "$INSTDIR\data"
    File /r "..\build_x64\rundir\RelWithDebInfo\data\*.*"
    SetOutPath "$INSTDIR\obs-plugins"
    File /r "..\build_x64\rundir\RelWithDebInfo\obs-plugins\*.*"

    ; Store installation folder
    WriteRegStr HKLM "Software\MediaStudio" "Install_Dir" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MediaStudio" "DisplayName" "Media Studio"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MediaStudio" "DisplayVersion" "30.0.0"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MediaStudio" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MediaStudio" "EstimatedSize" 500000

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
    SetOutPath "$INSTDIR\bin\64bit"
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Media Studio.lnk" "$INSTDIR\bin\64bit\obs64.exe"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Desktop Shortcut" SecDesktop
    SetOutPath "$INSTDIR\bin\64bit"
    CreateShortcut "$DESKTOP\Media Studio.lnk" "$INSTDIR\bin\64bit\obs64.exe"
SectionEnd

; ===== Section Descriptions =====
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "Core application files (required)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "Create Start Menu shortcuts"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Create Desktop shortcut"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; ===== Uninstaller Section =====
Section "Uninstall"
    ; Remove files
    RMDir /r "$INSTDIR\bin"
    RMDir /r "$INSTDIR\data"
    RMDir /r "$INSTDIR\obs-plugins"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"

    ; Remove shortcuts
    RMDir /r "$SMPROGRAMS\$StartMenuFolder"
    Delete "$DESKTOP\Media Studio.lnk"

    ; Remove registry
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MediaStudio"
    DeleteRegKey HKLM "Software\MediaStudio"
SectionEnd

; ===== Function Callbacks =====
Function .onInit
    ${If} ${RunningX64}
        ; 64-bit system
    ${Else}
        MessageBox MB_ICONSTOP|MB_TOPMOST "This application requires Windows x64!"
        Abort
    ${EndIf}
FunctionEnd

Function un.onInit
    MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to uninstall Media Studio?" /SD IDYES IDYES +2
    Abort
FunctionEnd
