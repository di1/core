#!/bin/bash

# Exit on first error
set -e

ROOT=`pwd`

# grab 3rd party
mkdir -p 3rdparty
cd 3rdparty

if [ -d libwebsockets ];
then
  cd libwebsockets
  git pull
  cd build
  cmake .. 2>/dev/null 1>/dev/null
  make 2>/dev/null 1>/dev/null
  sudo make install 2>/dev/null 1>/dev/null
else
  git clone https://libwebsockets.org/repo/libwebsockets
  cd libwebsockets
  mkdir build/
  cd build/
  cmake ..
  make
  sudo make install
fi

cd ..
cp cmake/FindLibWebSockets.cmake $ROOT/cmake/

cd $ROOT

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
