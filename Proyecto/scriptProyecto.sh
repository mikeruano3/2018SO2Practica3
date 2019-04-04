#!/bin/bash
sudo rm -rf /montar
sudo mkdir /montar
sudo chmod +777 /montar
fusermount -u /montar
rm -rf fuse201503666Proyecto
gcc -Wall `pkg-config fuse --cflags` fuse201503666Proyecto.c -o fuse201503666Proyecto `pkg-config fuse --libs`

sudo rm -rf /filesystem_201503666
sudo mkdir /filesystem_201503666
sudo chmod +777 /filesystem_201503666
gnome-terminal -e 'sh -c "echo INICIANDO SISTEMA;
						  echo .; sleep 1;
						  echo ..; sleep 1;
						  echo ...; sleep 1; clear;
						  cd /montar/filesystem_201503666; 
						  exec bash"'
./fuse201503666Proyecto -d -f -s /montar
