#!/bin/bash
sudo rm -rf /montar
sudo mkdir /montar
sudo chmod +777 /montar
fusermount -u /montar
rm -rf fuse201504429Proyecto
gcc -Wall `pkg-config fuse --cflags` fuse201504429Proyecto.c -o fuse201504429Proyecto `pkg-config fuse --libs`

sudo mkdir /filesystem_201504429
sudo chmod +777 /filesystem_201504429
gnome-terminal -e 'sh -c "echo INICIANDO SISTEMA;
						  echo .; sleep 1;
						  echo ..; sleep 1;
						  echo ...; sleep 1; clear;
						  cd /montar/filesystem_201504429; 
						  exec bash"'
./fuse201504429Proyecto -d -f -s /montar
