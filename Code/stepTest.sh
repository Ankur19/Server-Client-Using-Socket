
echo "step wise testing for server & client starting with 8 client and increasing upto 64 clients. (step of 4)"

echo "server outputs to be stored in serverOutput.txt"

echo "Which one?"
echo "1: Sequential, 2: Parallel"
read automated

lineInTime="wc -l < $PWD/time.txt"
lineInError="wc -l < $PWD/error.txt"

gnome-terminal --working-directory $PWD --tab -e 'bash -c "{ make run-server PORT=50017; exec bash; } &>> serverOutput.txt"'

for ((i=8;i<=64;i+=4));
do
    touch $PWD/time.txt
    touch $PWD/error.txt

    for ((j=0;j<$i;j++));
    do
        gnome-terminal --working-directory $PWD --tab -e "make run-client PORT=50017 FOLDER_INDEX=$j AUTO=$automated"
    done

    while :;
    do
        lineCounts=$(eval "$lineInTime")

        if [ $lineCounts -eq $i ]
        then
            break
        fi
        sleep 1;
    done

    while :;
    do
        lineCounts=$(eval "$lineInError")

        if [ $lineCounts -eq $i ]
        then
            break
        fi
        sleep 1;
    done

    mv $PWD/time.txt $PWD/Times/time_$i.txt
    mv $PWD/error.txt $PWD/Errors/error_$i.txt

    $PWD/folderRemove.sh $i

    sleep 10
done