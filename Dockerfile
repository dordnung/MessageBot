FROM debian:wheezy

WORKDIR /build

ENV BUILD_DIR /build/build
ENV SMBRANCH 1.9-dev
ENV MESSAGEBOT_DIR /build

RUN apt-get update && apt-get install lib32stdc++6 zip \
    unzip gcc-multilib g++-multilib git make wget -y
