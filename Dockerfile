FROM debian:jessie

WORKDIR /build

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    g++-multilib \
    gcc-multilib \
    git \
    lib32stdc++6 \
    make \
    unzip \
    wget \
    zip \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
