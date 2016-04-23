@echo off

set DIRNAME="eclipse-mingw"

cd /d %~dp0
if not exist %DIRNAME% (
    mkdir %DIRNAME%
)
cd %DIRNAME%

cmake -G "Eclipse CDT4 - MinGW Makefiles" ^
    -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE ^
    -DCMAKE_ECLIPSE_VERSION=4.4 ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_CXX_COMPILER_ARG1=-std=c++11 ^
    -Dbuild_tests=ON ^
    -Dbuild_examples=ON ^
    ../netlib
pause

rem On Eclipse: Import > General > Existing Projects into Workspace
