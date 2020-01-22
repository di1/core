#!/bin/bash

export RUN_TESTS=0

usage() {
  echo "Usage: $0 debug|release|clean|test" 1>&2
  exit 1
}

mkdir -p build > /dev/null

# Exit on first error
set -e

# Make sure there is 1 argument
if (( $# != 1 ))
then
  usage
fi

if [ "$1" = "debug" ]
then
  cd build
  cmake ../
  make
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
  make
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
  make
  ./riski
  exit 0
else
  usage
fi
