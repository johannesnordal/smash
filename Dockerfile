FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Bifrost 
ENV C_INCLUDE_PATH=$C_INCLUDE_PATH:/usr/local/include/
ENV CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:/usr/local/include/
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
ENV LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib/
ENV PATH=$PATH:/usr/local/lib/

# golang
ENV PATH=$PATH:/usr/local/go/bin

WORKDIR /root/smash
COPY . .
RUN /bin/sh setup.sh
