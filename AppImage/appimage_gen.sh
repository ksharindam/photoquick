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
cp ../../data/photoquick.desktop usr/share/applications/com.ksharindam.photoquick.desktop
cp ../com.ksharindam.photoquick.appdata.xml usr/share/metainfo


# copy qt5 plugins. Qt5 searches plugins in the same directory as the program binary.
QT_PLUGIN_PATH=${APPDIR}/usr/bin

mkdir -p ${QT_PLUGIN_PATH}/imageformats
cd /usr/${LIBDIR}/qt5/plugins/imageformats
cp libqjpeg.so libqgif.so libqwebp.so libqsvg.so libqtiff.so libqico.so ${QT_PLUGIN_PATH}/imageformats
cd ${APPDIR}

mkdir -p ${QT_PLUGIN_PATH}/printsupport
cp /usr/${LIBDIR}/qt5/plugins/printsupport/libcupsprintersupport.so ${QT_PLUGIN_PATH}/printsupport

# this is most necessary plugin for x11 support. without it application won't launch
mkdir -p ${QT_PLUGIN_PATH}/platforms
cp /usr/${LIBDIR}/qt5/plugins/platforms/libqxcb.so ${QT_PLUGIN_PATH}/platforms

# bundling libqgtk2style.so can create lot of problems, so bundling only cleanlooks style.
mkdir -p ${QT_PLUGIN_PATH}/styles
cp /usr/${LIBDIR}/qt5/plugins/styles/libqcleanlooksstyle.so ${QT_PLUGIN_PATH}/styles


linuxdeploy --appdir . \
  --icon-file=../../data/photoquick.png \
  --deploy-deps-only=${QT_PLUGIN_PATH}/imageformats \
  --deploy-deps-only=${QT_PLUGIN_PATH}/printsupport \
  --deploy-deps-only=${QT_PLUGIN_PATH}/platforms \
  --deploy-deps-only=${QT_PLUGIN_PATH}/styles




cd ..
appimagetool AppDir
#appimagetool -u "zsync|https://github.com/ksharindam/photoquick/releases/latest/download/PhotoQuick-${ARCH}.AppImage.zsync" AppDir
