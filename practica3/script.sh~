#!/bin/bash
sudo mkdir /montar
sudo chmod +777 /montar
fusermount -u /montar
rm -rf fuse201504429
gcc -Wall `pkg-config fuse --cflags` fuse201504429.c -o fuse201504429 `pkg-config fuse --libs`

sudo mkdir /filesystem_201504429
sudo chmod +777 /filesystem_201504429
./fuse201504429 -d /montar
