# Riski

This project is an attempt to develop and maintain mathematical conjectures
and implementations of various technical analysis used in modeling
traded security charts (Japanese Candle Stick Charts).

### Building
Building the entire Riski framework requires a few packages. Since Riski
in intended to be rolling release there is no minimum version for packages.
Instead make sure to always build with the latest dependencies.

Before continuing make sure `npm`, `nodejs` and `clang` are installed.

To build the entire framework first run the `scripts/install_libwebsockets.sh`
then the `build.sh` file. Note that `build.sh` requires clang and will not work
with gcc although manually building the server code with gcc is possible. All
commits to this repository should be compiled with clang and not gcc.
