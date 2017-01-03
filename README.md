MessageBot
==========

Allows to send messages to players with a Steam Bot

Check `messagebot.inc` for available functions.
Binarys can be found in the [CallAdmin Steam Module](https://forums.alliedmods.net/showthread.php?t=213670)

## How-to build: ##

### On Linux: ###
- **Set path to build**
  1. `export BUILD_DIR=$HOME`
  2. `cd $BUILD_DIR`

- **Build openssl (The available version may change!)**
  1. `wget https://www.openssl.org/source/openssl-1.0.2j.tar.gz && tar -xvzf openssl-1.0.2j.tar.gz`
  2. `cd openssl-1.0.2j`
  3. `setarch i386 ./config -m32 no-shared && make`
  4. `cd $BUILD_DIR`

- **Build zlib**
  1. `wget http://zlib.net/zlib1210.zip && unzip zlib1210.zip`
  2. `cd zlib-1.2.10`
  3. `CFLAGS=-m32 ./configure -static && make`
  4. `cd $BUILD_DIR`

- **Build libcurl**
  1. `wget http://curl.haxx.se/download/curl-7.52.1.zip && unzip curl-7.52.1.zip`
  2. `cd curl-7.52.1`
  3. `env LIBS="-ldl" CPPFLAGS="-I$BUILD_DIR/zlib-1.2.10" LDFLAGS="-L$BUILD_DIR/openssl-1.0.2j -L$BUILD_DIR/zlib-1.2.10" ./configure --with-ssl=$BUILD_DIR/openssl-1.0.2j --with-zlib=$BUILD_DIR/zlib-1.2.10 --disable-shared --enable-static --disable-rtsp --disable-ldap --disable-ldaps --disable-sspi --disable-tls-srp --without-librtmp --without-libidn --without-libssh2 --without-nghttp2 --without-gssapi --host=i686-pc-linux-gnu CFLAGS=-m32 CC=/usr/bin/gcc && make`
  4. `cd $BUILD_DIR`

- **Get Sourcemod 1.8**
  - `wget https://github.com/alliedmodders/sourcemod/archive/1.8-dev.zip -O sourcemod.zip && unzip sourcemod.zip`

- **Build messagebot**
  1. `wget https://github.com/popoklopsi/MessageBot/archive/master.zip -O messagebot.zip && unzip messagebot.zip`
  2. `cd MessageBot-master`
  3. `make SMSDK=$BUILD_DIR/sourcemod-1.8-dev OPENSSL=$BUILD_DIR/openssl-1.0.2j ZLIB=$BUILD_DIR/zlib-1.2.10 CURL=$BUILD_DIR/curl-7.52.1`

### On Windows (Visual Studio 2015): ###
- **Build libcurl**
  1. Download curl from `http://curl.haxx.se/download/curl-7.52.1.zip` and unzip
  2. Add VS to the system PATH:
    - For example: `C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin` 
  3. Open command line at `curl-7.52.1/winbuild`
  4. Type `vcvars32.bat` and press ENTER
  5. Type `nmake /f Makefile.vc mode=static VC=14 MACHINE=x86` and press ENTER
  6. Add a new system variable named `CURL` with the path to the curl-7.52.1 folder

- **Get Sourcemod**
  1. Download sourcemod from `https://github.com/alliedmodders/sourcemod/archive/master.zip` and unzip
  2. Add a new system variable named `SOURCEMOD` with the path to sourcemod

- **Build messagebot**
  1. Download MessageBot from `https://github.com/popoklopsi/MessageBot/archive/master.zip` and unzip
  2. Open `msvc15/messagebot.sln` 
  3. Build the project.