#!/bin/bash

[ ! -f main.c ] && echo call ./build.sh in impress terminal source directory && exit

./docker/gcc-it-call.sh make

cd tools
../docker/gcc-it-call.sh make
cd ..
