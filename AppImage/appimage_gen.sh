#!/bin/bash

check_dep()
{
  DEP=$1
  if [ -z $(which $DEP) ] ; then
    echo "Error : $DEP command not found"
    exit 0
  fi
}

check_dep appimagetool
check_dep linuxdeploy
check_dep gcc

#ARCH=`dpkg --print-architecture`
MULTIARCH=`gcc -dumpmachine`
LIBDIR=lib/${MULTIARCH}

mkdir -p AppDir/usr/bin/plugins
mkdir -p AppDir/usr/share/applications
mkdir -p AppDir/usr/share/metainfo

cd AppDir
APPDIR=`pwd`

# copy executable, plugins and desktop file
cp ../../src/photoquick usr/bin
cp ../../plugins/*.so usr/bin/plugins
cp ../../data/photoquick.desktop usr/share/applications/io.github.ksharindam.photoquick.desktop
cp ../../data/io.github.ksharindam.photoquick.metainfo.xml usr/share/metainfo
cp ../AppRun .


# copy qt5 plugins. Qt5 searches plugins in the same directory as the program binary.
QT_PLUGIN_PATH=${APPDIR}/usr/bin

mkdir -p ${QT_PLUGIN_PATH}/imageformats
cd /usr/${LIBDIR}/qt5/plugins/imageformats
cp libqjpeg.so libqgif.so libqwebp.so libqsvg.so libqtiff.so \
   libqico.so libqwbmp.so kimg_avif.so kimg_psd.so ${QT_PLUGIN_PATH}/imageformats
cd ${APPDIR}

mkdir -p ${QT_PLUGIN_PATH}/printsupport
cp /usr/${LIBDIR}/qt5/plugins/printsupport/libcupsprintersupport.so ${QT_PLUGIN_PATH}/printsupport

# this is most necessary plugin for x11 support. without it application won't launch
mkdir -p ${QT_PLUGIN_PATH}/platforms
cp /usr/${LIBDIR}/qt5/plugins/platforms/libqxcb.so ${QT_PLUGIN_PATH}/platforms
# Wayland support (requires qtwayland5 package)
cp /usr/${LIBDIR}/qt5/plugins/platforms/libqwayland-generic.so ${QT_PLUGIN_PATH}/platforms
cp -r /usr/${LIBDIR}/qt5/plugins/wayland-shell-integration ${QT_PLUGIN_PATH}
cp -r /usr/${LIBDIR}/qt5/plugins/wayland-graphics-integration-client ${QT_PLUGIN_PATH}

# bundling libqgtk2style.so can create lot of problems, so not bundling.
mkdir -p ${QT_PLUGIN_PATH}/styles
cp /usr/${LIBDIR}/qt5/plugins/styles/libqt5ct-style.so ${QT_PLUGIN_PATH}/styles

linuxdeploy --appimage-extract-and-run --appdir . \
  --icon-file=../../data/photoquick.png \
  --deploy-deps-only=${QT_PLUGIN_PATH}/imageformats \
  --deploy-deps-only=${QT_PLUGIN_PATH}/printsupport \
  --deploy-deps-only=${QT_PLUGIN_PATH}/styles \
  --deploy-deps-only=${QT_PLUGIN_PATH}/platforms \
  --deploy-deps-only=${QT_PLUGIN_PATH}/wayland-shell-integration \
  --deploy-deps-only=${QT_PLUGIN_PATH}/wayland-graphics-integration-client




cd ..

#if [ "$MULTIARCH" = "x86_64-linux-gnu" ]; then
#    appimagetool --appimage-extract-and-run -u "zsync|https://github.com/ksharindam/photoquick/releases/latest/download/PhotoQuick-x86_64.AppImage.zsync" AppDir
#else
appimagetool --appimage-extract-and-run AppDir
#fi
