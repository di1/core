#!/bin/bash

usage() {
  echo "Usage: $0 debug|release|clean" 1>&2
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
  echo "release build"
  exit 0
elif [ "$1" = "clean" ]
then
  rm -rf build/
  mkdir build/
  exit 0
else
  usage
fi
