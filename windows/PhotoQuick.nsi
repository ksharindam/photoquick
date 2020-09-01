; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "PhotoQuick"
!define PRODUCT_VERSION "4.3.5"
!define PRODUCT_PUBLISHER "Arindamsoft Co."
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\photoquick.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_NAME} ${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show
SetCompressor lzma

; Required Plugins
;!include "EnvVarUpdate.nsh"
!include "FileAssociation.nsh"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\src\images\photoquick.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; MUI pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\photoquick.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES
; Language files
!insertmacro MUI_LANGUAGE "English"
; MUI end ------



!define BUILDDIR      "..\src"
!define QTLIB_DIR     "C:\Qt\4.8.7\bin"
!define QTPLUGINS_DIR "C:\Qt\4.8.7\plugins"
!define DEPS_DIR      "C:\mingw32\bin"

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite try
  File "${DEPS_DIR}\libgcc_s_dw2-1.dll"
  File "${DEPS_DIR}\libgomp-1.dll"
  File "${DEPS_DIR}\libstdc++-6.dll"
  File "${DEPS_DIR}\libwinpthread-1.dll"
  File "${QTLIB_DIR}\QtCore4.dll"
  File "${QTLIB_DIR}\QtGui4.dll"
  SetOutPath "$INSTDIR\imageformats"
  File "${QTPLUGINS_DIR}\imageformats\qjpeg4.dll"
  File "${QTPLUGINS_DIR}\imageformats\qsvg4.dll"
  File "${QTPLUGINS_DIR}\imageformats\qico4.dll"
  File "${QTPLUGINS_DIR}\imageformats\qgif4.dll"
  File "${QTPLUGINS_DIR}\imageformats\qtiff4.dll"
  SetOutPath "$INSTDIR"
  File "${BUILDDIR}\photoquick.exe"
  ; Install icon and shortcut
  File "..\src\images\photoquick.ico"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}.lnk" "$INSTDIR\photoquick.exe" "" "$INSTDIR\photoquick.ico"
  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\photoquick.exe" "" "$INSTDIR\photoquick.ico"
  ; Update environment variable
  ;${EnvVarUpdate} $0 "QT_PLUGIN_PATH" "A" "HKLM" "$INSTDIR"
  ; Associate File Types
  ${registerExtension} "$INSTDIR\photoquick.exe" ".jpg" "JPEG Image"
  ${registerExtension} "$INSTDIR\photoquick.exe" ".jpeg" "JPEG Image"
  ${registerExtension} "$INSTDIR\photoquick.exe" ".png" "PNG Image"
  ${registerExtension} "$INSTDIR\photoquick.exe" ".gif" "GIF Image"
  ${registerExtension} "$INSTDIR\photoquick.exe" ".svg" "SVG Image"
  ${registerExtension} "$INSTDIR\photoquick.exe" ".ico" "Windows Icon"
  ${registerExtension} "$INSTDIR\photoquick.exe" ".tiff" "TIFF Image"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\photoquick.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\photoquick.ico"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name)?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\photoquick.exe"
  Delete "$INSTDIR\imageformats\qjpeg4.dll"
  Delete "$INSTDIR\imageformats\qsvg4.dll"
  Delete "$INSTDIR\imageformats\qico4.dll"
  Delete "$INSTDIR\imageformats\qgif4.dll"
  Delete "$INSTDIR\imageformats\qtiff4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\libwinpthread-1.dll"
  Delete "$INSTDIR\libstdc++-6.dll"
  Delete "$INSTDIR\libgomp-1.dll"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"

  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}.lnk"
  Delete "$INSTDIR\photoquick.ico"

  RMDir "$INSTDIR\imageformats"
  RMDir "$INSTDIR"
  ; Remove environment variable
  ;${un.EnvVarUpdate} $0 "QT_PLUGIN_PATH" "R" "HKLM" "$INSTDIR"
  ${unregisterExtension} ".jpg" "JPEG Image"
  ${unregisterExtension} ".jpeg" "JPEG Image"
  ${unregisterExtension} ".png" "PNG Image"
  ${unregisterExtension} ".gif" "GIF Image"
  ${unregisterExtension} ".svg" "SVG Image"
  ${unregisterExtension} ".ico" "Windows Icon"
  ${unregisterExtension} ".tiff" "TIFF Image"
  
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd