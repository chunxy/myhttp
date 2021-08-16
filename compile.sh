#!/usr/bin/sh

gcc -g -Wall *.c http/*.c -o build/server -lpthread
