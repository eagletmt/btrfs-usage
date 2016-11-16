FROM ubuntu:16.04

RUN mkdir -p /build \
  && apt-get update \
  && apt-get install -y debhelper dh-make devscripts gnupg2

# dependencies
RUN apt-get -y install btrfs-tools cmake
