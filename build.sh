#!/bin/bash

set -e

source scripts/log.sh

LS_LEVEL=LS_DEBUG_LEVEL
LSINFO Riski \(C\) Vittorio Papandrea


LSINFO ==== FINDING REQUIREMENTS ====

NPMLOCATION=$(which npm 2> /dev/null)

if [[ $? == 1 ]]; then
  LSERROR npm is not installed
  exit 1
else
  LSDEBUG npm location $NPMLOCATION
fi

CLANGLOCATION=$(which clang 2>/dev/null)

if [[ $? == 1 ]]; then
  LSERROR clang is not installed
  exit 1
else
  LSDEBUG clang location $CLANGLOCATION
fi


CLANGPPLOCATION=$(which clang++ 2>/dev/null)

if [[ $? == 1 ]]; then
  LSERROR clang++ is not installed
  exit 1
else
  LSDEBUG clang++ location $CLANGLOCATION
fi

GITLOCATION=$(which git 2>/dev/null)

if [[ $? == 1 ]]; then
  LSERROR git is not installed
  exit 1
else
  LSDEBUG git location $GITLOCATION
fi

CMAKELOCATION=$(which cmake 2>/dev/null)

if [[ $? == 1 ]]; then
  LSERROR cmake is not installed
  exit 1
else
  LSDEBUG cmake location $CMAKELOCATION
fi

MAKELOCATION=$(which make 2>/dev/null)

if [[ $? == 1 ]]; then
  LSERROR make is not installed
  exit 1
else
  LSDEBUG make location $MAKELOCATION
fi

LSINFO ==== FOUND REQUIREMENTS ====

LSINFO ==== BUILDING LIBWEBSOCKETS ====

if [[ -d "./libwebsockets/" ]]; then
  cd libwebsockets
  LSDEBUG in $(pwd)
else
  git clone https://libwebsockets.org/repo/libwebsockets
  cd libwebsockets
  LSDEBUG in $(pwd)
fi

git checkout v4.0-stable
mkdir -p build/
cd build/

LSDEBUG in $(pwd)

cmake ..
make
sudo make install

cd ../../
LSDEBUG in $(pwd)
mv libwebsockets/cmake/FindLibWebSockets.cmake ./cmake/

LSINFO ==== FINISHED LIBWEBSOCKETS ====

LSINFO ==== BUILDING FRONTEND ====
LSDEBUG in $(pwd)
cd web/
LSDEBUG in $(pwd)

LSINFO npm update
npm update

LSINFO npm install
npm install > /dev/null

for file in $(find ./ts -name *.ts); do
  LSDEBUG linting $file
  ./node_modules/.bin/eslint $file
done

LSINFO compiling typescript
./node_modules/.bin/tsc

LSINFO minimizing javascript for faster performance
./node_modules/.bin/uglifyjs main.js -o main.min.js 

LSINFO ==== FINISHED FRONTEND ====
LSINFO ==== BUILDING SERVER ====

cd ../
LSDEBUG in $(pwd)
mkdir -p build/
cd build
LSDEBUG in $(pwd)
LSINFO running cmake

CC=$CLANGLOCATION
CXX=$CLANGPPLOCATION

cmake ..
LSINFO running make
make

LSINFO organizing build

cd ..
LSDEBUG in $(pwd)
rm -rf out/
mkdir out/

LSINFO copying executable
cp build/src/riski out/
LSINFO copying iex symbol list

# TODO find an alternative to the symbols table
cp symbols.csv out/


LSINFO building minimal website dependencies
mkdir out/web
cp web/index.html out/web/
cp web/main.min.js out/web
cp -R web/img out/web/

tree out/web

mkdir out/analysis
LSINFO copying dynamic analysis libraries
for file in $(find ./build/libs/ -name "*.so"); do
  LSINFO found library $file
  cp $file out/analysis/
done

LSINFO creating symbolic link to compile_commands for developers that use
LSINFO vim + ycm

ln -s build/compile_commands.json ./ 2> /dev/null

LSINFO ==== FINISHED SERVER ====
