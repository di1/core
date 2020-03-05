#!/bin/bash

case "$(uname -s)" in
  Darwin)
    echo 'Mac OS X'
    ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null
    brew install libwebsockets
    brew install openssl
    ;;
  *)
    git clone https://libwebsockets.org/repo/libwebsockets
    cd libwebsockets
    mkdir build
    cd build

    cmake ..
    sudo make install

    ;;
esac
