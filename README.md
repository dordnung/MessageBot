# MessageBot

[![Build Status](https://api.travis-ci.com/dordnung/MessageBot.svg)](https://travis-ci.com/dordnung/MessageBot)

Allows to send messages to players with a Steam Bot

Check `messagebot.inc` for available functions.
Binaries can be found in the [CallAdmin Steam Module](https://forums.alliedmods.net/showthread.php?t=213670) or on [the releases page](https://github.com/dordnung/MessageBot/releases).

## How-to build

### On Linux
- **Set build path**
  1. `export BUILD_DIR=$HOME`
  2. `cd $BUILD_DIR`

- **Build openssl**
  1. `wget https://www.openssl.org/source/openssl-1.1.1a.tar.gz && tar -xvzf openssl-1.1.1a.tar.gz`
  2. `cd openssl-1.1.1a`
  3. `setarch i386 ./config -m32 no-shared && make`
  4. `mkdir lib && cp *.a lib/`
  5. `cd $BUILD_DIR`

- **Build zlib**
  1. `wget http://zlib.net/zlib1211.zip && unzip zlib1211.zip`
  2. `cd zlib-1.2.11`
  3. `CFLAGS=-m32 ./configure -static && make`
  4. `mkdir include && mkdir lib && cp *.h include/ && cp libz.a lib`
  5. `cd $BUILD_DIR`

- **Build libidn**
  1. `wget https://ftp.gnu.org/gnu/libidn/libidn2-2.0.5.tar.gz && tar -xvzf libidn2-2.0.5.tar.gz`
  2. `cd libidn2-2.0.5`
  3. `CFLAGS=-m32 ./configure --disable-shared --enable-static --disable-doc && make`
  4. `mkdir include && cp lib/*.h include/ && cp lib/.libs/libidn2.a lib`
  5. `cd $BUILD_DIR`

- **Build libcurl**
  1. `wget https://curl.haxx.se/download/curl-7.62.0.zip && unzip curl-7.62.0.zip`
  2. `cd curl-7.62.0`
  3. `./configure --with-ssl=$BUILD_DIR/openssl-1.1.1a --with-zlib=$BUILD_DIR/zlib-1.2.11 --with-libidn2=$BUILD_DIR/libidn2-2.0.5 --disable-shared --enable-static --disable-rtsp --disable-ldap --disable-ldaps --disable-manual --disable-libcurl-option --without-librtmp --without-libssh2 --without-nghttp2 --without-gssapi --host=i386-pc-linux-gnu CFLAGS=-m32 && make all ca-bundle`
  4. **DO NOT INSTALL IT!**
  5. `cd $BUILD_DIR`

- **Get Sourcemod 1.9**
  1. `git clone https://github.com/alliedmodders/sourcemod --recursive --branch 1.9-dev --single-branch sourcemod-1.9`

- **Build MessageBot**
  1. `git clone https://github.com/dordnung/MessageBot`
  2. `cd MessageBot`
  3. `make SMSDK=$BUILD_DIR/sourcemod-1.9 OPENSSL=$BUILD_DIR/openssl-1.1.1a ZLIB=$BUILD_DIR/zlib-1.2.11 IDN=$BUILD_DIR/libidn2-2.0.5 CURL=$BUILD_DIR/curl-7.62.0`

### On Windows (Visual Studio 2015/2017)
- **Build zlib**
  1. Download zlib from `https://zlib.net/zlib1211.zip` and unzip to some folder
  2. Open the `Developer Command Prompt for VS 2017` or `Developer Command Prompt for VS 2015` at the `zlib-1.2.11` folder
  3. Type `vcvarsall.bat x86 8.1` and press ENTER
  4. Type `nmake /f win32/Makefile.msc LOC=-MT` and press ENTER
  5. Type `md lib include` and press ENTER
  6. Type `copy /Y zlib.lib lib` and press ENTER
  7. Type `copy /Y *h include` and press ENTER
  8. Add a new system variable named `ZLIB` pointing to the `zlib-1.2.11` folder

- **Build libcurl**
  1. Download curl from `https://curl.haxx.se/download/curl-7.62.0.zip` and unzip to some folder
  2. Reopen the `Developer Command Prompt for VS 2017` or `Developer Command Prompt for VS 2015` at the `curl-7.62.0` folder
  3. Type `vcvarsall.bat x86 8.1` and press ENTER
  4. Type `cd winbuild` and press ENTER
  5. Type `nmake /f Makefile.vc mode=static WITH_ZLIB=static ZLIB_PATH=%ZLIB% RTLIBCFG=static VC=15 MACHINE=x86` and press ENTER
  6. Add a new system variable named `CURL` pointing to the `curl-7.62.0/builds/libcurl-vc15-x86-release-static-zlib-static-ipv6-sspi-winssl` folder

- **Get Sourcemod 1.9**
  1. Retrieve Sourcemod 1.9 with: `git clone https://github.com/alliedmodders/sourcemod --recursive --branch 1.9-dev --single-branch sourcemod-1.9`
  2. Add a new system variable named `SOURCEMOD19` with the path to the sourcemod-1.9 folder

- **Build MessageBot**
  1. Retrieve MessageBot with: `git clone https://github.com/dordnung/MessageBot`
  2. Reopen the `Developer Command Prompt for VS 2017` or `Developer Command Prompt for VS 2015` at the `MessageBot` folder
  3. Type `vcvarsall.bat x86 8.1` and press ENTER
  4. Type `msbuild msvc17/messagebot.sln /p:Platform="win32"` and press ENTER