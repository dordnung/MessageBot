#!/bin/bash

cd $BUILD_DIR

# OpenSSL
wget https://www.openssl.org/source/openssl-1.1.1.tar.gz && tar -xvzf openssl-1.1.1.tar.gz
cd openssl-1.1.1
setarch i386 ./config -m32 no-shared && make
mkdir lib && cp *.a lib/
cd $BUILD_DIR

# Zlib
wget http://zlib.net/zlib1211.zip && unzip zlib1211.zip
cd zlib-1.2.11
CFLAGS=-m32 ./configure -static && make
mkdir include && mkdir lib && cp *.h include/ && cp libz.a lib
cd $BUILD_DIR

# Libidn
wget https://ftp.gnu.org/gnu/libidn/libidn2-2.0.5.tar.gz && tar -xvzf libidn2-2.0.5.tar.gz
cd libidn2-2.0.5
CFLAGS=-m32 ./configure --disable-shared --enable-static --disable-doc && make
mkdir include && cp lib/*.h include/ && cp lib/.libs/libidn2.a lib
cd $BUILD_DIR

# LibCurl
wget https://curl.haxx.se/download/curl-7.61.1.zip && unzip curl-7.61.1.zip
cd curl-7.61.1
./configure --with-ssl=$BUILD_DIR/openssl-1.1.1 --with-zlib=$BUILD_DIR/zlib-1.2.11 \
 --with-libidn2=$BUILD_DIR/libidn2-2.0.5 --disable-shared --enable-static --disable-rtsp \
 --disable-ldap --disable-ldaps --disable-manual --disable-libcurl-option --without-librtmp \
 --without-libssh2 --without-nghttp2 --without-gssapi --host=i386-pc-linux-gnu CFLAGS=-m32 && make
cd $BUILD_DIR

# SourceMod
git clone https://github.com/alliedmodders/sourcemod --recursive --branch ${SMBRANCH} --single-branch ${SMBRANCH}


cd $MESSAGEBOT_DIR
make SMSDK=$BUILD_DIR/${SMBRANCH} OPENSSL=$BUILD_DIR/openssl-1.1.1 ZLIB=$BUILD_DIR/zlib-1.2.11 IDN=$BUILD_DIR/libidn2-2.0.5 CURL=$BUILD_DIR/curl-7.61.1