#!/bin/bash

# Create a 5gb file
echo "Generate large (500 mb) file?"
echo "1: No, 2: Yes"
read needed

echo "How many clients?"
read numClients

echo "Which port?"
read port

echo "Automated? This will make the client not ask for files to download and will download all files."
echo "0: No, 1: Sequential, 2: Parallel"
read automated

if [ $needed -eq 2 ]; then
    dd if=/dev/zero of=file.out bs=1MB count=500
fi

make clean
make all

gnome-terminal --working-directory $PWD --tab -e "make run-server PORT=$port" 

for ((i=0;i<$numClients;i++));
do
    gnome-terminal --working-directory $PWD --tab -e "make run-client PORT=$port FOLDER_INDEX=$i AUTO=$automated"
done