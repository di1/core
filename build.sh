#!/bin/bash

set -e

source scripts/log.sh

LS_LEVEL=LS_DEBUG_LEVEL
LSINFO Riski \(C\) Vittorio Papandrea


LSINFO ==== BUILDING FRONTEND ====

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

echo
echo

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
LSINFO ==== FINISHED SERVER ====
