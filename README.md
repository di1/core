# Riski

This project is an attempt to develop and maintain mathematical conjectures
and implementations of various technical analysis used in modeling
traded security charts (Japanese Candle Stick Charts). Riski is intended
to be an open source trading platform used for backtesting, and live trading. 

### Building

*Note Riski is highly dependent on POSIX and Windows builds are not planned
to be supported*

Building the entire Riski framework requires a few packages. Since Riski
in intended to be rolling release there is no minimum version for packages.
Instead make sure to always build with the latest dependencies.

Building the entire Riski framework is as simple as running `sudo bash build.sh`
The script will hault if a package needs to be installed and will not continue.

### Feed Support
Currently IEX tick data is supported by running

`riski -pcap_feed FILE`

And live oanda feed is supported by running. The live oanda feed currently
only works with demo accounts.

`riski -oanda API_KEY`

### Implementing Analysis and Strategies through the C API

### Acknowledgements

[![Oanda](https://avatars0.githubusercontent.com/u/658105?s=32)](https://github.com/oanda)
[![IEX Group](https://avatars3.githubusercontent.com/u/4103535?s=32)](https://github.com/iexg)
