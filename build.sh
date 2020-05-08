#!/bin/bash

#Exit on first error
set -e

command -v npm >/dev/null 2>&1 || {
  echo >&2 "[ERROR] npm is not installed."
  exit 1
}

command -v pdflatex >/dev/null 2>&1 || {
  exit >&2 "[ERROR] pdflatex is not installed."
}

echo "[DOC   ] Compiling latex documentation"
cd docs
pdflatex riski.latex
cp riski.pdf ../

cd ..
cd web/

if [[ $1 != "-fast" ]]
then
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
fi

echo "[WEB   ] Compiling typescript"
./node_modules/.bin/tsc

echo "[WEB   ] Minimizing javascript"
./node_modules/.bin/uglifyjs main.js -o main.min.js
cd ..

export CC=/bin/clang
export CXX=/bin/clang++

mkdir -p build/
cd build/
cmake ..
make
cd ../
