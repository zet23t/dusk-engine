#/bin/sh

gcc -o progAddComponent "$(dirname "$0")"/progAddComponent.c
./progAddComponent $1 $2 $3 $4 
rm progAddComponent