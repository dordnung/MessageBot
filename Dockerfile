FROM debian:wheezy

WORKDIR /build

RUN apt-get update && apt-get install lib32stdc++6 zip \
    unzip gcc-multilib g++-multilib git make wget -y
