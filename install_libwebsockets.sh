#!/bin/bash

git clone https://libwebsockets.org/repo/libwebsockets
cd libwebsockets
mkdir build
cd build

case "$(uname -s)" in

   Darwin)
     echo 'Mac OS X'
     cmake -DOPENSSL_ROOT_DIR=/usr/local/ssl -DOPENSSL_LIBRARIES=/usr/local/ssl/lib ..
     ;;

   Linux)
     echo 'Linux'
     cmake ..
     sudo make install
     ;;

   CYGWIN*|MINGW32*|MSYS*|MINGW*)
     echo 'MS Windows'
     ;;

   # Add here more strings to compare
   # See correspondence table at the bottom of this answer

   *)
     echo 'Other OS'
     cmake ..
     sudo make install
     ;;
esac

cd ..
rm -rf libwebsockets
