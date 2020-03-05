#!/bin/bash

command -v clang-format >/dev/null 2>&1 || {
  echo >&2 "[ERROR] clang-format is not installed."
  exit 1
}

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

