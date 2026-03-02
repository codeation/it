#!/bin/bash

cd $(dirname $0)/four
../docker/gcc-it-call.sh make
cd ..

cd $(dirname $0)/tools
../docker/gcc-it-call.sh make
cd ..
