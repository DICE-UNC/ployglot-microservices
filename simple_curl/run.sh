#!/bin/sh

set -e

gcc `curl-config --cflags` -c main.c
gcc -o main main.o `curl-config --libs`
./main
