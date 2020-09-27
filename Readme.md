### CS-550 Programming Assignment 1

#### Ankur Saikia (A20445640)
========================================================================================================================================
----------------------------------------------------------------------------------------------------------------------------------------
### Dependencies
* gcc 9+
* gnome-terminal (default for ubuntu)
----------------------------------------------------------------------------------------------------------------------------------------
### How to run?
* Provide execute permission to the test.sh script by running the below command
```bash
    chmod +x test.sh
```
* Run the script using the below command
```bash
    ./test.sh
```
* The script would ask for inputs and would download the files to ClientFolder_{__clientNumber__} folder
----------------------------------------------------------------------------------------------------------------------------------------
### For more customization
* For better granularity and options to download files in parallel, we can manually start and run the server and individual clients
* Run the below command to start the server using own port number
```bash
    make run-server PORT={ portNumber }
```
* In another terminal start a client using the below command
```bash
    make run-client PORT={ portNumber } FOLDER_INDEX={ folderIndex } AUTO={ automated }
```
  * The  *PORT*  has to be same as that to the server
  * The  *FOLDER_INDEX*  value would set the directory to save the new files. e.g. **FOLDER_INDEX=1** would save the files in  */ClientFolder_1/*  folder
  * The  *AUTO*  tag is for automated downloads of all files. Seting this to **0** would make the program ask for which files to download and whether to download them parallely or serially. 
  * *AUTO* tag has two options **[0: No, 1: Yes]**