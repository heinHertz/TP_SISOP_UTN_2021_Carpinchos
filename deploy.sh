#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD
echo -e "\n\nInstalling commons libraries...\n\n"
COMMONS="so-commons-library"
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD
echo -e "\n\nBuilding projects...\n\n"
make -C ./swamp
make -C ./memoria
make -C ./kernel
mkdir lib
rm lib/libmatelib.so
gcc -shared -o "lib/libmatelib.o" matelib/deploy/matelib.c  -lcommons
mv lib/libmatelib.o lib/libmatelib.so
sudo cp lib/libmatelib.so /usr/lib/
echo -e "\n\nDeploy done!\n\n"