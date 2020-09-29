## CS-550 Programming Assignment 1

#### Ankur Saikia (A20445640)
----------------------------------------------------------------------------------------------------------------------------------------

### Dependencies
* gcc 9+
* gnome-terminal (default for ubuntu)
----------------------------------------------------------------------------------------------------------------------------------------
### How to run?
* ```VARIABLES.h``` contains **MAX_CLIENTS** which is set to 1024 by default but can be set to limited amounts like 64 if we want limited clients
* In the ```/Code``` folder run the below comand to clean up existing compiled files
```bash
    make clean
```
* Now to compile the code run the below commmand
```bash
    make all
```
* Provide execute permission to the ```test.sh``` script by running the below command
```bash
    chmod +x test.sh
```
* Run the script using the below command
```bash
    ./test.sh
```
* The script would ask for user inputs to the below questions
    * Generate large (500 mb) file?
    * How many clients?
    * Which port?
    * Automated? This will make the client not ask for files to download and will download all files.
* The files would be downloaded to folder with name like ClientFolder_{__clientNumber__}
* To delete the folders and their respective files after testing run the ```folderRemove.sh``` script with the number of folders as input.
```bash
    chmod +x folderDelete.sh
    ./folderDelete.sh 8     // Here 8 is the number of client folders to delete
```
----------------------------------------------------------------------------------------------------------------------------------------
### For more customization
* For better granularity, we can manually start and run the server and individual clients using ```MAKE```
* Run the below command to start the server using own port number
```bash
    make run-server PORT={ portNumber }
```
* In another terminal start a client using the below command
```bash
    make run-client PORT={ portNumber } FOLDER_INDEX={ folderIndex } AUTO={ automated }
```
* The options for ```run-client``` are as under
    * The  *PORT*  has to be same as that to the server
    * The  *FOLDER_INDEX*  value would set the directory to save the new files. e.g. **FOLDER_INDEX=1** would save the files in  */ClientFolder_1/*  folder
    * The  *AUTO*  tag is for automated downloads of all files. Seting this to **0** would make the program ask for which files to download and whether to download them parallely or serially. 
    * *AUTO* tag has three options **[0: No, 1: Sequential, 2: Parallel]**
----------------------------------------------------------------------------------------------------------------------------------------
### Scaling from 8 clients to 64 clients
* To run the scaling script we need to give execute peermission to two bash scripts
```bash
    chmod +x folderRemove.sh
    chmod +x stepTest.sh
```
* After execute permissions we need to run the script
```bash
    ./stepTest.sh
```
* This script asks for a single user input which is whether to download files in sequence or in parallel
* Once this is done, it would automatically spin up the server and clients in bursts
* The output times get stored inside ```/Times``` and the number of files with Md5 errors get stored inside ```/Errors```
* Additionally the server output gets stored in ```serverOutput.txt```