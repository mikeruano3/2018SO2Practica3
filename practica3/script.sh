#!/bin/bash
fusermount -u /filesystem_201504429
gcc -Wall `pkg-config fuse --cflag` fuse201504429.c -o fuse201504429 `pkg-config fuse --libs`
rm -rf fuse201504429
rm -rf fuse201504429.o
sudo rmdir /filesystem_201504429
sudo mkdir /filesystem_201504429
./fuse201504429 /filesystem_201504429/ -o allow_other