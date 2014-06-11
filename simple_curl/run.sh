#!/bin/sh

set -e

gcc `curl-config --cflags` -c postit2.c
gcc -o postit2 postit2.o `curl-config --libs`
./postit2 > libcurl.png
