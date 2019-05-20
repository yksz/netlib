#!/bin/sh

BUILD_DIR_POSTFIX="_eclipse"

script_dir=`dirname "${0}"`
script_path=$(cd ${script_dir} && pwd -P)
script_basename=`basename "${script_path}"`
build_dir=${script_basename}${BUILD_DIR_POSTFIX}

cd ${script_path}/../
if [ ! -e ${build_dir} ] ; then
    mkdir ${build_dir}
fi
cd ${build_dir}

generator="Unix"
if [ $1 ] && [ $1 = "mingw" ] ; then
    generator="MinGW"
fi

cmake -G "Eclipse CDT4 - ${generator} Makefiles" \
    -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE \
    -DCMAKE_ECLIPSE_VERSION=4.4 \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER_ARG1=-std=c++11 \
    -Dbuild_tests=ON \
    -Dbuild_examples=ON \
    $@ \
    ../${script_basename}

# On Eclipse: Import > General > Existing Projects into Workspace
