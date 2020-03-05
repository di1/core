#!/bin/bash

#Exit on first error
set -e

command -v git >/dev/null 2>&1 || {
  echo >&2 "[ERROR] git is not installed."
}

command -v npm >/dev/null 2>&1 || {
  echo >&2 "[ERROR] npm is not installed."
  exit 1
}

cd web/

echo "[WEB   ] Updating NPM"
npm update > /dev/null

echo "[WEB   ] Installing NPM packages"
npm install > /dev/null

# Run ES lint on all typescript files
for tsFile in `find ts -name "*.ts"`;
do
  echo "[LINT  ] ${tsFile}"
  ./node_modules/.bin/eslint ${tsFile} --fix
done

echo "[WEB   ] Compiling typescript"
./node_modules/.bin/tsc

echo "[WEB   ] Minimizing javascript"
./node_modules/.bin/uglifyjs main.js -o main.min.js
cd ..

echo $(pwd)

mkdir -p build/
cd build/
cmake ..
make
cd ../
