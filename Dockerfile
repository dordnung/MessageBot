FROM debian:wheezy

WORKDIR /build

RUN apt-get update && apt-get install -y --no-install-recommends \
	g++-multilib \
	gcc-multilib \
	git \
	make \
	wget \
	zip \
    lib32stdc++6 \
    unzip \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
