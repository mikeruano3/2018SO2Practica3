#!/bin/bash

fusermount -u /filesystem_201503666
echo "Desmontado"
rm -f fuse201503666 #eliminar los make
rm -f fuse201503666.o
echo "Eliminar make"
sudo rm -rf /filesystem_201503666
sudo mkdir /filesystem_201503666
echo "Truncar /filesystem_201503666"
gcc -Wall `pkg-config fuse --cflags` fuse201503666.c -o fuse201503666 `pkg-config fuse --libs`
./fuse201503666 /filesystem_201503666 -o allow_other
echo "Montado!"