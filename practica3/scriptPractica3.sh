#!/bin/bash
sudo rm -rf /montar
sudo mkdir /montar
sudo chmod +777 /montar
fusermount -u /montar
rm -rf fuse201503666Practica3
gcc -Wall `pkg-config fuse --cflags` fuse201503666Practica3.c -o fuse201503666Practica3 `pkg-config fuse --libs`

sudo mkdir /filesystem_201503666
sudo chmod +777 /filesystem_201503666
gnome-terminal --working-directory="/montar/filesystem_201503666"
./fuse201503666Practica3 -d -f -s /montar
