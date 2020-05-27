# Riski

This project is an attempt to develop and maintain mathematical conjectures
and implementations of various technical analysis used in modeling
traded security charts (Japanese Candle Stick Charts).

### Building

*Note Windows Builds are Not Supported nor is there plans for a Windows build*


Building the entire Riski framework including documentation
requires a few packages. Since Riski
in intended to be rolling release there is no minimum version for packages.
Instead make sure to always build with the latest dependencies.

Before continuing make sure `npm`, `nodejs`, `clang`, `pdflatex` are installed.

To build the entire framework first run the `scripts/install_libwebsockets.sh`
then the `build.sh` file. Note that `build.sh` requires clang and will not work
with gcc although manually building the server code with gcc is possible. All
commits to this repository should be compiled with clang and not gcc.

### Acknowledgements

[![Oanda](https://avatars0.githubusercontent.com/u/658105?s=32)](https://github.com/oanda)
[![IEX Group](https://avatars3.githubusercontent.com/u/4103535?s=32)](https://github.com/iexg)