#!/bin/bash

export ROOTDIR="${PWD}"
if [  -f ${ROOTDIR}/../build/bin/asn1c ]; then
    echo "asn1c exist"
    exit
fi

#git clone https://github.com/vlm/asn1c.git
#git clone https://github.com/velichkov/asn1c.git
#git clone https://github.com/open5gs/asn1c.git
git clone https://github.com/mouse07410/asn1c.git

cd asn1c

export PATH=${HOME}/ac_install/bin:$PATH
export PKG_CONFIG_PATH=${ROOTDIR}/../build/lib/pkgconfig:$PKG_CONFIG_PATH

autoreconf --force --install
./configure \
    --prefix=${ROOTDIR}/../build \
    --enable-static \
    --enable-ASN_DEBUG

make -j4
make install
