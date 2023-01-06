#!/bin/bash

DEV_GCC_VERSION=12.2.0

docker build \
    --build-arg dev_gcc_version=$DEV_GCC_VERSION \
    -t it-build/gcc:$DEV_GCC_VERSION \
    ./docker/gcc-it-build

docker run -it --rm --name gcc-it-build \
    -u $(id -u ${USER}):$(id -g ${USER}) \
    -v $PWD:$PWD \
    -w "$PWD" \
    it-build/gcc:$DEV_GCC_VERSION "$@"
