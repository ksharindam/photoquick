; PhotoQuick Installer for Qt5 + MinGW64
!define PROG_NAME "PhotoQuick"
!define PROG_VERSION "4.21.0"
!define PROG_PUBLISHER "Arindamsoft"
!define PROG_ICON "photoquick.ico"
!define PROG_EXEC "photoquick.exe"

!define PRODUCT_DIR_REGKEY "Software\${PROG_NAME}"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}"


Name "${PROG_NAME}"
OutFile "${PROG_NAME}-${PROG_VERSION}.exe"
InstallDir "$PROGRAMFILES64\${PROG_NAME}"
; Get previous install directory if already installed
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
SetCompressor lzma

; Required Plugins
!include "FileAssociation.nsh"
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\data\${PROG_ICON}"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; MUI Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
!define MUI_PAGE_CUSTOMFUNCTION_PRE SkipDirectoryPage
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PROG_EXEC}"
!insertmacro MUI_PAGE_FINISH

; Skip choosing directory when updating older version
Function SkipDirectoryPage
  ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" ""
  ${IF} $0 != ""
    Abort
  ${EndIf}
FunctionEnd

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

; This shows version info in installer, VIFileVersion and VIProductVersion must be in x.x.x.x format
VIProductVersion "${PROG_VERSION}.0"
VIFileVersion "${PROG_VERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "PhotoQuick"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "PhotoQuick Image Viewer and Editor"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Arindamsoft"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${PROG_VERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Arindam Chaudhuri <ksharindam@gmail.com>"


; Default installation directories
!ifndef QTBINDIR
  !define QTBINDIR "C:\Qt\5.15.2\bin"
!endif
!define MINGWBINDIR "C:\mingw64\bin"
!define BUILDDIR "..\src"
!define PLUGINS_DIR "..\plugins"

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite try

  ; Install MinGW64 runtime libraries
  File "${MINGWBINDIR}\libgcc_s_seh-1.dll"
  File "${MINGWBINDIR}\libstdc++-6.dll"
  File "${MINGWBINDIR}\libwinpthread-1.dll"
  File "${MINGWBINDIR}\libgomp-1.dll"

  ; Install Qt5 Core Libraries
  File "${QTBINDIR}\Qt5Core.dll"
  File "${QTBINDIR}\Qt5Gui.dll"
  File "${QTBINDIR}\Qt5Widgets.dll"
  File "${QTBINDIR}\Qt5PrintSupport.dll"
  File "${QTBINDIR}\Qt5Svg.dll"

  ; Install Qt5 platform and style plugins
  SetOutPath "$INSTDIR\platforms"
  File "${QTBINDIR}\..\plugins\platforms\qwindows.dll"
  SetOutPath "$INSTDIR\styles"
  File "${QTBINDIR}\..\plugins\styles\qwindowsvistastyle.dll"
  ; Image format plugins
  SetOutPath "$INSTDIR\imageformats"
  File "${QTBINDIR}\..\plugins\imageformats\*.dll"
  ; Print Support
  SetOutPath "$INSTDIR\printsupport"
  File "${QTBINDIR}\..\plugins\printsupport\windowsprintersupport.dll"

  ; Install PhotoQuick plugins
  SetOutPath "$INSTDIR\plugins"
  File "${PLUGINS_DIR}\invert.dll"
  File "${PLUGINS_DIR}\text-tool.dll"
  File "${PLUGINS_DIR}\photo-optimizer.dll"

  ; Install main executable and icon
  SetOutPath "$INSTDIR"
  File "${BUILDDIR}\photoquick.exe"
  File "..\data\${PROG_ICON}"

  ; Create shortcuts
  CreateShortCut "$SMPROGRAMS\${PROG_NAME}.lnk" "$INSTDIR\${PROG_EXEC}" "" "$INSTDIR\${PROG_ICON}"
  CreateShortCut "$DESKTOP\${PROG_NAME}.lnk" "$INSTDIR\${PROG_EXEC}" "" "$INSTDIR\${PROG_ICON}"

  ; Associate file types
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".jpg" "JPEG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".jpeg" "JPEG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".jfif" "JFIF Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".png" "PNG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".gif" "GIF Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".svg" "SVG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".ico" "Windows Icon"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".tiff" "TIFF Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".tif" "TIFF Image"
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
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
SectionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Do you really want to completely remove $(^Name)?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  ; Remove executable and main files
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\photoquick.exe"
  Delete "$INSTDIR\${PROG_ICON}"

  ; Remove PhotoQuick plugins
  Delete "$INSTDIR\plugins\invert.dll"
  Delete "$INSTDIR\plugins\text-tool.dll"
  Delete "$INSTDIR\plugins\photo-optimizer.dll"

  ; Remove Qt5 plugins
  RMDir /r "$INSTDIR\platforms"
  RMDir /r "$INSTDIR\styles"
  RMDir /r "$INSTDIR\imageformats"
  RMDir /r "$INSTDIR\printsupport"

  ; Remove Qt5 libraries
  Delete "$INSTDIR\Qt5Core.dll"
  Delete "$INSTDIR\Qt5Gui.dll"
  Delete "$INSTDIR\Qt5Widgets.dll"
  Delete "$INSTDIR\Qt5PrintSupport.dll"
  Delete "$INSTDIR\Qt5Svg.dll"

  ; Remove MinGW64 runtime libraries
  Delete "$INSTDIR\libgcc_s_seh-1.dll"
  Delete "$INSTDIR\libstdc++-6.dll"
  Delete "$INSTDIR\libwinpthread-1.dll"
  Delete "$INSTDIR\libgomp-1.dll"

  ; Remove shortcuts
  Delete "$DESKTOP\${PROG_NAME}.lnk"
  Delete "$SMPROGRAMS\${PROG_NAME}.lnk"

  ; Remove directories
  RMDir "$INSTDIR\plugins"
  RMDir "$INSTDIR"

  ; Unregister file associations
  ${unregisterExtension} ".jpg" "JPEG Image"
  ${unregisterExtension} ".jpeg" "JPEG Image"
  ${unregisterExtension} ".jfif" "JFIF Image"
  ${unregisterExtension} ".png" "PNG Image"
  ${unregisterExtension} ".gif" "GIF Image"
  ${unregisterExtension} ".svg" "SVG Image"
  ${unregisterExtension} ".ico" "Windows Icon"
  ${unregisterExtension} ".tiff" "TIFF Image"
  ${unregisterExtension} ".tif" "TIFF Image"
  ${unregisterExtension} ".webp" "WebP Image"

  ; Remove registry entries
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd
