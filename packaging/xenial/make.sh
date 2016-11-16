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
# TODO: GPG sign
docker run --rm \
  --volume "${here}/debian:/debian:ro" \
  --volume "${PWD}:/src:ro" \
  --volume "${here}:/output" \
  --workdir /build \
  "$IMAGE_TAG" \
  bash -c "cp -a /debian . && ln -s /src/${source_archive} / && debuild -uc -us && cp -pv /*.deb /output/"
