MessageBot
==========

Allows to send messages to players with a Steam Bot

### How-to build on apt based systems: ###

1. `sudo apt-get install libssl-dev`
2. `wget http://curl.haxx.se/download/curl-7.38.0.tar.gz`
3. `tar -xvzf curl-7.38.0.tar.gz`
4. `cd curl-7.38.0`
5. `./configure --disable-shared --enable-static --disable-rtsp --disable-ldap --disable-ldaps --disable-sspi --disable-tls-srp --without-librtmp --without-libidn --without-libssh2 --without-nghttp2 --without-gssapi`
6. `make`
7. `sudo make install`
8. `Download Opensteamworks and place it one path before`
9. `Download Sourcemod and place it one path before`
10. Goto Makefile of MessageBot and: `make all`

Binarys can be found in the [CallAdmin Steam Module](https://forums.alliedmods.net/showthread.php?t=213670)
