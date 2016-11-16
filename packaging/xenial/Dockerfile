FROM ubuntu:16.04

RUN mkdir -p /build \
  && apt-get update \
  && apt-get install -y debhelper dh-make devscripts gnupg2 gpgv2 \
  && dpkg-divert --local --rename --add /usr/bin/gpg \
  && dpkg-divert --local --rename --add /usr/bin/gpgv \
  && ln -s gpg2 /usr/bin/gpg \
  && ln -s gpgv2 /usr/bin/gpgv

# dependencies
RUN apt-get -y install btrfs-tools cmake
