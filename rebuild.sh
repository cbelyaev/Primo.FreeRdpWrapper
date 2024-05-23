#!/bin/bash

if [ -d "./vcpkg/" ]; then rm -Rf "./vcpkg/"; fi
if [ -d "./x64/" ]; then rm -Rf "./x64/"; fi

git clone  --depth 1 --branch 2024.04.26 https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh -disableMetrics
cp x64-linux-static-release.cmake ./vcpkg/triplets/community
./vcpkg/vcpkg install freerdp:x64-linux-static-release

mkdir -p ./x64/Debug ./x64/Release

gcc \
    -DDEBUG \
    -shared \
    -o ./x64/Debug/Primo.FreeRdpWrapper.so \
    -fPIC \
    -I./vcpkg/installed/x64-linux-static-release/include \
    -Wall \
    Logging.c \
    FreeRdpWrapper.c \
    ./vcpkg/installed/x64-linux-static-release/lib/libfreerdp3.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libfreerdp-client3.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libwinpr3.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libssl.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libcrypto.a

gcc \
    -shared \
    -o ./x64/Release/Primo.FreeRdpWrapper.so \
    -fPIC \
    -I./vcpkg/installed/x64-linux-static-release/include \
    -Wall \
    Logging.c \
    FreeRdpWrapper.c \
    ./vcpkg/installed/x64-linux-static-release/lib/libfreerdp3.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libfreerdp-client3.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libwinpr3.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libssl.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libcrypto.a

strip ./x64/Release/Primo.FreeRdpWrapper.so

