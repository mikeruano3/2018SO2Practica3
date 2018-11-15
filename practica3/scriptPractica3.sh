#!/bin/bash
sudo rm -rf /montar
sudo mkdir /montar
sudo chmod +777 /montar
fusermount -u /montar
rm -rf fuse201504429Practica3
gcc -Wall `pkg-config fuse --cflags` fuse201504429Practica3.c -o fuse201504429Practica3 `pkg-config fuse --libs`

sudo mkdir /filesystem_201504429
sudo chmod +777 /filesystem_201504429
gnome-terminal --working-directory="/montar/filesystem_201504429"
./fuse201504429Practica3 -d -f -s /montar
