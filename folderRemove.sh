echo "How many folders?"
read numClients

for ((i=0;i<$numClients;i++));
do
    echo "deleting files in $PWD/ClientFolder_$i"
    
    directory=$PWD/ClientFolder_$i
    
    cd $PWD/ClientFolder_$i
    
    if [ "$PWD" = "$directory" ]; then
        rm *
        cd ..
        rmdir $PWD/ClientFolder_$i
    fi
done