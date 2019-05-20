[![Build Status](https://travis-ci.org/yksz/netlib.svg?branch=master)](https://travis-ci.org/yksz/netlib)

# What is this?
This is a cross platform network library for C++11.

This library's main features:
- TCP client/server
- UDP client/server
- Endian Conversion
- Getting a list of the system's nerwork interfaces

# Installation
```
./build.sh
cd build
make
sudo make install
```
NOTE: Run "./build.sh -DNETLIB_USE_OPENSSL=ON" if use SSL/TLS.

or

Copy files in src directory to your project.

# Platform
- Windows
- Mac OS X
- Linux

# License
The MIT license
