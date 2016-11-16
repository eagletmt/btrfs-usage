FROM ubuntu:16.04

RUN apt-get update && apt-get install -y debhelper dh-make devscripts

# dependencies
RUN apt-get -y install btrfs-tools cmake

RUN mkdir -p /build
