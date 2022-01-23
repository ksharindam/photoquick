; HM NIS Edit Wizard helper defines
!define PROG_NAME "PhotoQuick"
!define PROG_VERSION "4.9.2"
!define PROG_PUBLISHER "Arindamsoft"
!define PROG_ICON "photoquick.ico"
!define PROG_EXEC "photoquick.exe"

!define PRODUCT_DIR_REGKEY "Software\${PROG_NAME}"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}"


Name "${PROG_NAME}"
OutFile "${PROG_NAME}-${PROG_VERSION}.exe"
InstallDir "$PROGRAMFILES\${PROG_NAME}"
; Get previous install directory if already installed
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
SetCompressor lzma

; Required Plugins
;!include "EnvVarUpdate.nsh"
!include "FileAssociation.nsh"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\data\${PROG_ICON}"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; MUI pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PROG_EXEC}"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES
; Language files
!insertmacro MUI_LANGUAGE "English"
; MUI end ------

; This shows version info in installer, VIFileVersion and VIProductVersion must be in x.x.x.x format
VIProductVersion "${PROG_VERSION}.0"
VIFileVersion "${PROG_VERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "PhotoQuick"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "PhotoQuick Image Viewer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Arindamsoft Co."
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${PROG_VERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Arindam Chaudhuri <ksharindam@gmail.com>"


!define BUILDDIR      "..\src"
!define PLUGINS_DIR   "..\plugins"
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
  File "${QTPLUGINS_DIR}\imageformats\qwebp.dll"
  ; Install plugins
  SetOutPath "$INSTDIR\plugins"
  File "${PLUGINS_DIR}\invert.dll"
  File "${PLUGINS_DIR}\text-tool.dll"
  File "${PLUGINS_DIR}\photo-optimizer.dll"
  ; Install program and icon
  SetOutPath "$INSTDIR"
  File "${BUILDDIR}\photoquick.exe"
  File "${MUI_ICON}"
  ; Install shortcuts
  CreateShortCut "$SMPROGRAMS\${PROG_NAME}.lnk" "$INSTDIR\${PROG_EXEC}" "" "$INSTDIR\${PROG_ICON}"
  CreateShortCut "$DESKTOP\${PROG_NAME}.lnk" "$INSTDIR\${PROG_EXEC}" "" "$INSTDIR\${PROG_ICON}"
  ; Update environment variable
  ;${EnvVarUpdate} $0 "QT_PLUGIN_PATH" "A" "HKLM" "$INSTDIR"
  ; Associate File Types
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".jpg" "JPEG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".jpeg" "JPEG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".png" "PNG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".gif" "GIF Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".svg" "SVG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".ico" "Windows Icon"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".tiff" "TIFF Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".webp" "WebP Image"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\${PROG_EXEC}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${PROG_ICON}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PROG_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PROG_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
SectionEnd


Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Do you really want to completely remove $(^Name)?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  ; Must remove uninstaller first
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\photoquick.exe"
  Delete "$INSTDIR\plugins\invert.dll"
  Delete "$INSTDIR\plugins\text-tool.dll"
  Delete "$INSTDIR\plugins\photo-optimizer.dll"
  Delete "$INSTDIR\imageformats\qjpeg4.dll"
  Delete "$INSTDIR\imageformats\qsvg4.dll"
  Delete "$INSTDIR\imageformats\qico4.dll"
  Delete "$INSTDIR\imageformats\qgif4.dll"
  Delete "$INSTDIR\imageformats\qtiff4.dll"
  Delete "$INSTDIR\imageformats\qwebp.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\libwinpthread-1.dll"
  Delete "$INSTDIR\libstdc++-6.dll"
  Delete "$INSTDIR\libgomp-1.dll"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"

  Delete "$DESKTOP\${PROG_NAME}.lnk"
  Delete "$SMPROGRAMS\${PROG_NAME}.lnk"
  Delete "$INSTDIR\${PROG_ICON}"

  RMDir "$INSTDIR\imageformats"
  RMDir "$INSTDIR\plugins"
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
  ${unregisterExtension} ".webp" "WebP Image"

  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd
