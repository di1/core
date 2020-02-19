#!/bin/bash

#Exit on first error
set -e

command -v npm >/dev/null 2>&1 || { echo >&2 "I require npm but it's not installed.  Aborting."; exit 1; }

cd web
npm update
npm install

# Run ES lint on all typescript files
for tsFile in `find ts -name "*.ts"`;
do
  echo "[LINT] ${tsFile}"
  ./node_modules/.bin/eslint ${tsFile}
done

./node_modules/.bin/tsc
./node_modules/.bin/uglifyjs main.js -o main.min.js
cd ..

mkdir -p build/
cd build/
cmake ..
make
cd ..
