#!/bin/sh

VERSION='1.7.0'
INSTALL_DIR='/usr/local'
DOWNLOAD_DIR='/tmp'

# Download
cd $DOWNLOAD_DIR
wget https://github.com/google/googletest/archive/release-${VERSION}.tar.gz
tar xzvf release-${VERSION}.tar.gz

# Build
cd googletest-release-${VERSION}
cmake .
make

# Install
sudo mkdir -p ${INSTALL_DIR}/include
sudo cp -r include/gtest ${INSTALL_DIR}/include
sudo mkdir -p ${INSTALL_DIR}/lib
sudo cp *.a ${INSTALL_DIR}/lib

# Cleanup
cd $DOWNLOAD_DIR
rm release-${VERSION}.tar.gz
rm -r googletest-release-${VERSION}
