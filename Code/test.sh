#!/bin/bash

# Create a 5gb file
echo "Generate large (500 mb) file?"
echo "1: No, 2: Yes"
read needed

echo "How many clients?"
read numClients

echo "Automated? This will make the client not ask for files to download and will download all files."
echo "0: no, 1: yes"
read automated

if [ $needed -eq 2 ]; then
    dd if=/dev/zero of=file.out bs=1MB count=500
fi

gnome-terminal --working-directory $PWD --tab -e "make run-server PORT=50017" 

for ((i=0;i<$numClients;i++));
do
    gnome-terminal --working-directory $PWD --tab -e "make run-client PORT=50017 FOLDER_INDEX=$i AUTO=$automated"
done