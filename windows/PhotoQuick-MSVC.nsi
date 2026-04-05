; PhotoQuick NSIS Installer Script (MSVC Build)
; Optimized for Visual C++ builds with Qt5

!define PROG_NAME "PhotoQuick"
!define PROG_VERSION "4.21.0"
!define PROG_PUBLISHER "Arindamsoft"
!define PROG_ICON "photoquick.ico"
!define PROG_EXEC "photoquick.exe"
!define PROG_URL "https://github.com/ksharindam/photoquick"

!define PRODUCT_DIR_REGKEY "Software\${PROG_NAME}"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROG_NAME}"

Name "${PROG_NAME}"
OutFile "${PROG_NAME}-${PROG_VERSION}-x64.exe"
InstallDir "$PROGRAMFILES64\${PROG_NAME}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
SetCompressor /SOLID lzma

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

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

; Version information
VIProductVersion "${PROG_VERSION}.0"
VIFileVersion "${PROG_VERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "PhotoQuick"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "PhotoQuick - Image Viewer and Editor"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Arindamsoft"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${PROG_VERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${PROG_VERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Arindam Chaudhuri <ksharindam@gmail.com>"

; Skip directory page if upgrading
Function SkipDirectoryPage
  ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" ""
  ${IF} $0 != ""
    Abort
  ${EndIf}
FunctionEnd

; Main installation section
Section "Install" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite try
  
  ; Application files
  File "photoquick.exe"
  File "${PROG_ICON}"
  
  ; Qt5 Libraries
  File "Qt5Core.dll"
  File "Qt5Gui.dll"
  File "Qt5Svg.dll"
  File "Qt5Widgets.dll"
  
  ; Image format plugins
  SetOutPath "$INSTDIR\imageformats"
  File "imageformats\qjpeg.dll"
  File "imageformats\qsvg.dll"
  File "imageformats\qico.dll"
  File "imageformats\qgif.dll"
  File "imageformats\qtiff.dll"
  File "imageformats\qwebp.dll"
  
  ; Platform plugins
  SetOutPath "$INSTDIR\platforms"
  File "platforms\qwindows.dll"
  
  ; PhotoQuick plugins
  SetOutPath "$INSTDIR\plugins"
  File "plugins\invert.dll"
  File "plugins\text-tool.dll"
  File "plugins\photo-optimizer.dll"
  
  ; Create shortcuts
  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\${PROG_NAME}"
  CreateShortCut "$SMPROGRAMS\${PROG_NAME}\${PROG_NAME}.lnk" "$INSTDIR\${PROG_EXEC}" "" "$INSTDIR\${PROG_ICON}"
  CreateShortCut "$SMPROGRAMS\${PROG_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$DESKTOP\${PROG_NAME}.lnk" "$INSTDIR\${PROG_EXEC}" "" "$INSTDIR\${PROG_ICON}"
  
  ; File associations
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".jpg" "JPEG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".jpeg" "JPEG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".jfif" "JFIF Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".png" "PNG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".gif" "GIF Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".bmp" "Bitmap Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".svg" "SVG Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".ico" "Windows Icon"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".tiff" "TIFF Image"
  ${registerExtension} "$INSTDIR\${PROG_EXEC}" ".webp" "WebP Image"
SectionEnd

; Registry entries
Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\${PROG_EXEC}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "${PROG_NAME}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${PROG_ICON}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PROG_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PROG_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PROG_URL}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
SectionEnd

; Uninstall functions
Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Do you want to completely remove $(^Name)?" IDYES +2
  Abort
FunctionEnd

; Uninstall section
Section Uninstall
  ; Remove application files
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\${PROG_EXEC}"
  Delete "$INSTDIR\${PROG_ICON}"
  
  ; Remove Qt5 libraries
  Delete "$INSTDIR\Qt5*.dll"
  
  ; Remove image format plugins
  RMDir /r "$INSTDIR\imageformats"
  
  ; Remove platform plugins
  RMDir /r "$INSTDIR\platforms"
  
  ; Remove PhotoQuick plugins
  RMDir /r "$INSTDIR\plugins"
  
  ; Remove shortcuts
  Delete "$DESKTOP\${PROG_NAME}.lnk"
  RMDir /r "$SMPROGRAMS\${PROG_NAME}"
  
  ; Remove installation directory
  RMDir "$INSTDIR"
  
  ; Unregister file associations
  ${unregisterExtension} ".jpg" "JPEG Image"
  ${unregisterExtension} ".jpeg" "JPEG Image"
  ${unregisterExtension} ".jfif" "JFIF Image"
  ${unregisterExtension} ".png" "PNG Image"
  ${unregisterExtension} ".gif" "GIF Image"
  ${unregisterExtension} ".bmp" "Bitmap Image"
  ${unregisterExtension} ".svg" "SVG Image"
  ${unregisterExtension} ".ico" "Windows Icon"
  ${unregisterExtension} ".tiff" "TIFF Image"
  ${unregisterExtension} ".webp" "WebP Image"
  
  ; Remove registry entries
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "${PROG_NAME} was successfully removed."
FunctionEnd
