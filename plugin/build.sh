#!/usr/bin/env bash

set -ex

#git submodule update --init --recursive

protobuf_prefix="$PWD/protobuf-install"

if [ ! -x "$protobuf_prefix/bin/protoc" ]; then
    pushd protobuf
    git submodule update --init --recursive
    ./autogen.sh
    #./configure --disable-shared --prefix="$protobuf_prefix"
    ./configure --prefix="$protobuf_prefix"
    make -j
    #make check
    make install
    popd
fi

mkdir -p build
pushd build
cmake ..
make -j
if [ "$(uname)" = "Darwin" ]; then
    if [ ! -z "$CODESIGN_ID" ]; then
        for p in *.xpl ; do
          codesign -v "$p" || codesign -s "$CODESIGN_ID" -v "$p"
        done
    else
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        echo "WARNING: CODESIGN_ID env var not set, not signing!"
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    fi
fi
for p in *.xpl ; do
    otool -L "$p"
done
popd
