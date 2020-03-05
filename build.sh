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

command -v clang >/dev/null 2>&1 || {
  echo >&2 "[ERROR] clang is not installed."
  exit 1
}

command -v clang-format >/dev/null 2>&1 || {
  echo >&2 "[ERROR] clang-format is not installed."
  exit 1
}

echo "[3RDPTY] Installing libwebsockets"
mkdir -p 3rdparty/
cd 3rdparty
if [ -d "libwebsockets" ]
then
  cd libwebsockets
  git pull origin master
else
  git clone https://libwebsockets.org/repo/libwebsockets
  cd libwebsockets
fi

mkdir -p build/
cd build/
cmake ..
make

cd ../../../web/

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

for chFile in `find ./inc/ -name "*.h" -o -name "*.c"`
do
  echo "[FORMAT] ${chFile}"
  clang-format -i -style=Google ${chFile}
done

for chFile in `find ./src/ -name "*.h" -o -name "*.c"`
do
  echo "[FORMAT] ${chFile}"
  clang-format -i -style=Google ${chFile}
done


echo

rm -rf build/
mkdir -p build/
cd build/
export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
cmake ..
make

cd ..
rm -rf build
mkdir -p build/
cd build/
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
cmake ..
make
cd ..
