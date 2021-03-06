#!/bin/bash

NAME=btrfs-usage
IMAGE_TAG=ubuntu-xenial-packaging

version="$1"
if [ x"$version" = x"" ]; then
  echo "Usage: $0 <version>"
  exit 1
fi

set -ex
here="$PWD/$(dirname $0)"
docker build -t "$IMAGE_TAG" "$here"
source_archive="${NAME}_${version}.orig.tar.gz"
git archive --format=tar --prefix "${NAME}-${version}/" "v${version}" | gzip > "$source_archive"
docker run --rm \
  --volume "${here}/debian:/debian:ro" \
  --volume "${PWD}:/src:ro" \
  --volume "${here}:/output" \
  --volume "/run/user/${UID}/gnupg/S.gpg-agent:/root/.gnupg/S.gpg-agent" \
  --workdir /build \
  "$IMAGE_TAG" \
  bash -c "tar xf /src/${source_archive} && ln -s /src/${source_archive} /build/ && cd "${NAME}-${version}" && cp -a /debian . && gpg2 --import < /src/pub.asc && debuild -pgpg2 && cp -pv /build/*.{deb,debian.tar.xz,dsc,changes} /output/"
