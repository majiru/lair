#!/bin/sh
mkdir -p lib include
cp $PLAN9/lib/*.a lib/
cp $PLAN9/include/*.h include/
cp $PLAN9/bin/9c ./
cp $PLAN9/bin/9l ./
