#!/bin/sh

# install bifrost and smash deps
apt update -y
apt install -y \
    build-essential \
    cmake \
    curl \
    libhiredis-dev \
    libpq-dev \
    postgresql \
    postgresql-contrib \
    redis \
    vim \
    zlib1g-dev

# install latest golang version
curl -LO https://golang.org/dl/go1.16.6.linux-amd64.tar.gz
rm -rf /usr/local/go && tar -C /usr/local -xzf go1.16.6.linux-amd64.tar.gz

rm go1.16.6.linux-amd64.tar.gz

cd bifrost \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make \
    && make install

cd /root/smash/server
go mod init server/smash
go get -u github.com/gin-gonic/gin
go mod tidy

cd /root/smash
make

apt purge -y \
    curl \
    libhiredis-dev \
    libpq-dev \
    postgresql \
    postgresql-contrib \
    redis \
    vim \
    zlib1g-dev

service postgresql start
