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
  1. `wget https://www.openssl.org/source/openssl-1.0.2d.tar.gz && tar -xvzf openssl-1.0.2d.tar.gz`
  2. `cd openssl-1.0.2d`
  3. `setarch i386 ./config -m32 no-shared && make`
  4. `cd $BUILD_DIR`

- **Build zlib**
  1. `wget http://zlib.net/zlib128.zip && unzip zlib128.zip`
  2. `cd zlib-1.2.8`
  3. `CFLAGS=-m32 ./configure -static && make`
  4. `cd $BUILD_DIR`

- **Build libcurl**
  1. `wget http://curl.haxx.se/download/curl-7.45.0.zip && unzip curl-7.45.0.zip`
  2. `cd curl-7.45.0`
  3. `env LIBS="-ldl" CPPFLAGS="-I$BUILD_DIR/zlib-1.2.8" LDFLAGS="-L$BUILD_DIR/openssl-1.0.2d -L$BUILD_DIR/zlib-1.2.8" ./configure --with-ssl=$BUILD_DIR/openssl-1.0.2 --with-zlib=$BUILD_DIR/zlib-1.2.8 --disable-shared --enable-static --disable-rtsp --disable-ldap --disable-ldaps --disable-sspi --disable-tls-srp --without-librtmp --without-libidn --without-libssh2 --without-nghttp2 --without-gssapi --host=i686-pc-linux-gnu CFLAGS=-m32 CC=/usr/bin/gcc && make`
  4. `cd $BUILD_DIR`

- **Get Sourcemod 1.7**
  - `wget https://github.com/alliedmodders/sourcemod/archive/1.7-dev.zip -O sourcemod.zip && unzip sourcemod.zip`

- **Build messagebot**
  1. `wget https://github.com/popoklopsi/MessageBot/archive/master.zip -O messagebot.zip && unzip messagebot.zip`
  2. `cd MessageBot-master`
  3. `make SMSDK=$BUILD_DIR/sourcemod-1.7-dev OPENSSL=$BUILD_DIR/openssl-1.0.2d ZLIB=$BUILD_DIR/zlib-1.2.8 CURL=$BUILD_DIR/curl-7.45.0`

### On Windows (Visual Studio 2013): ###
- **Build libcurl**
  1. Download curl from `http://curl.haxx.se/download/curl-7.45.0.zip` and unzip
  2. Add VS to the system PATH:
    - For example: `C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin` 
  3. Open command line at `curl-7.45.0/winbuild`
  4. Type `vcvars32.bat` and press ENTER
  5. Type `nmake /f Makefile.vc mode=static VC=12 MACHINE=x86` and press ENTER
  6. Move the `libcurl_a.lib` and the `include` folder from `curl-7.45.0/builds/libcurl-XXX` to some folder
  7. Add a new system variable named `CURL` with the path to the .lib and the include folder

- **Get Sourcemod**
  1. Download sourcemod from `https://github.com/alliedmodders/sourcemod/archive/master.zip` and unzip
  2. Add a new system variable named `SOURCEMOD` with the path to sourcemod

- **Build messagebot**
  1. Download MessageBot from `https://github.com/popoklopsi/MessageBot/archive/master.zip` and unzip
  2. Open `msvc13/messagebot.sln` 
  3. Build the project.