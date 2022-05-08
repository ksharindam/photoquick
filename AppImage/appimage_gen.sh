#!/bin/bash

# When deploying a qt4 based program, two major problems occur
# First is qt plugins are not loaded. because Qt searches the absolute path
# /usr/lib/x86_64-linux-gnu/qt4/plugins . So it can not see the bundled plugins

# Second is, theme is dirty in some systems. Because Trolltech.conf contains
# theme settings and Qt searches Trolltech.conf in /etc/xdg and ~/.config

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
mkdir -p AppDir/usr/share/icons/hicolor/scalable/apps
mkdir -p AppDir/usr/share/metainfo

cd AppDir

# copy executable, icon and desktop file
cp ../../src/photoquick usr/bin
cp ../../plugins/*.so usr/bin/plugins
cp ../../data/photoquick.png usr/share/icons/hicolor/scalable/apps
cp ../../data/photoquick.desktop usr/share/applications/com.ksharindam.photoquick.desktop
cp ../com.ksharindam.photoquick.appdata.xml usr/share/metainfo


# copy plugins
mkdir -p usr/${LIBDIR}/qt4/plugins/imageformats
cp /usr/${LIBDIR}/qt4/plugins/imageformats/*.so usr/${LIBDIR}/qt4/plugins/imageformats


linuxdeploy --appdir . --deploy-deps-only=usr/${LIBDIR}/qt4/plugins/imageformats

# Replacing absolute path with relative path, so that Qt can find plugins and settings

# Tried setting qt_prfxpath ../. and ./.. but they dont work
sed -i -e 's#prfxpath=/usr#prfxpath=././#g' usr/lib/libQtCore.so.4
sed -i -e "s#/usr/${LIBDIR}/qt4/plugins#./../${LIBDIR}/qt4/plugins#g" usr/lib/libQtCore.so.4
# force Qt to search Trolltech.conf in usr/etc directory
sed -i -e 's#/etc/xdg#./../etc#g' usr/lib/libQtCore.so.4

# write Trolltech.conf for theme style setting
mkdir -p usr/etc

cat > usr/etc/Trolltech.conf << EOF
[Qt]
style=Cleanlooks
EOF

cd ..
appimagetool AppDir
#appimagetool -u "zsync|https://github.com/ksharindam/photoquick/releases/latest/download/PhotoQuick-${ARCH}.AppImage.zsync" AppDir
