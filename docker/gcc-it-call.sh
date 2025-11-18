#!/bin/bash

DEV_GCC_VERSION=15.2.0

docker build \
    --build-arg dev_gcc_version=$DEV_GCC_VERSION \
    -t it-build/gcc:$DEV_GCC_VERSION \
    "$(dirname "$0")/gcc-it-build"

docker run -t --rm --name gcc-it-build \
    -u $(id -u ${USER}):$(id -g ${USER}) \
    -v $PWD:$PWD \
    -w "$PWD" \
    it-build/gcc:$DEV_GCC_VERSION "$@"
