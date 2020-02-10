#!/bin/bash

#Exit on first error 
set -e

command -v tsc >/dev/null 2>&1 || { echo >&2 "I require npm but it's not installed.  Aborting."; exit 1; }

cd web
npm update
tsc
uglifyjs main.js > main.min.js

cd ..

ROOT=`pwd`

export RUN_TESTS=0

usage() {
  echo "Usage: $0 debug|release|clean|test" 1>&2
  exit 1
}

mkdir -p build > /dev/null

# Make sure there is 1 argument
if (( $# != 1 ))
then
  usage
fi

if [ "$1" = "debug" ]
then
  cd build
  cmake ../
  make -s
  exit 0
elif [ "$1" = "release" ]
then
  cd build
  export RUN_TESTS=1
  cmake ../
  make
  ./riski
  rm -rf *  
  export RUN_TESTS=0
  cmake ../
  make -s
  exit 0
elif [ "$1" = "clean" ]
then
  rm -rf build/
  mkdir build/
  exit 0
elif [ "$1" = "test" ]
then
  cd build
  export RUN_TESTS=true
  cmake ../
  make -s
  ./riski
  exit 0
else
  usage
fi
