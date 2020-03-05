#!/bin/bash

git clone https://libwebsockets.org/repo/libwebsockets
cd libwebsockets
mkdir build
cd build
cmake ..
sudo make install
cd ..
rm -rf libwebsockets
