#!/bin/bash

export ROOTDIR="${PWD}"
if [  -f ${ROOTDIR}/../build/lib/libxml2.a ]; then
    echo "lib exist"
    exit
fi

VERSION="v2.9.11"
if [ ! -f libxml2-${VERSION}.tar.gz ]; then
    wget https://gitlab.gnome.org/GNOME/libxml2/-/archive/${VERSION}/libxml2-${VERSION}.tar.gz --no-check-certificate
fi

rm -rf libxml2-${VERSION}
tar xvf libxml2-${VERSION}.tar.gz

cd libxml2-${VERSION}

export PATH=${HOME}/ac_install/bin:$PATH
export PKG_CONFIG_PATH=${ROOTDIR}/../build/lib/pkgconfig:$PKG_CONFIG_PATH

./autogen.sh
./configure \
    --prefix=${ROOTDIR}/../build \
    --enable-static

make
make install
