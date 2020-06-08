
To create, open, delete file, include the path and file name in the functions' parameters. Name must have a '/' at the beginning (e.g. "/testingDir/myFile.txt").

In my main function, I have included testings for some functions. Only function fs_lseek that I haven't created and functionfs_truncate that I use from the library.


List of functions in the final version:
 *fs_get_filesize; //Get metadata of file int, 
fs_create //Create file ,
int fs_write //Write to file, 
fs_read //Read from file,
 fP * fs_open  //Open file 
void fs_close //Close file pointer, 
int fs_delete//Delete file, 
void rewindF (fP *) //Rewind file pointer, 
int allocateBlock(fileDrive *); //Allocate empty block for files (or directories), 
void initializeDrive(fileDrive *)//Format the drive, 
int fs_mkdir(char *, fileDrive *)//Create directory (a wrapper function of createF) ,
void printFileInfo(entry *)//Print metadata of file void, 
printPointerInfo(fP *)//Same as printEntryInfo but used for file pointers

List of library used: 
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "utils.h"

I. Directory Structure 

a. File Allocation:
I will implement my File Allocation Structure using the FAT model. My drive will consist of 2 arrays, one for the block numbers and another for the memory blocks themselves. Each block will contain the content of a file .The maximum size of each block is 512 bytes. The size of the entry will be the size of a predefined C struct. An example of the implemented meta data entry would be: struct entry{ char name[20]; //Name of file int startBlock; //Mark the starting block of the file int fileSize; //size of file int type; //0 for file, 1 for directory int isOpen; //0 if file is not being opened. 1 if it is being opened. }

b. Physical Allocation:
My file system will contain about 20000 blocks of memory. Along with the table of the block numbers, the drive size is about 10 MB. The two table of blocks and blocks number are the main structure of my drive.
The storage units are the blocks, which will each has a maximum size of 512 bytes and contain either the content of a file or a list of entries to other file.. 
The storage units on the disk will be addressed based on the FAT model. In the block number table, an element at index i can either be 0 (free), or pointing to another block number (occupied). This implementation allows for storing non-contiguous files and protects integrity of the blocks. If a block is being used by one file, it should not be touched by other files, and the block numbers of other files cannot reference to the used block's number.

II. Implementation Here are the list of functions that I plan to implement for my file system. Before executing these functions, I will use mmap() to map the whole drive into memory. After finish executing the functions, I will use munmap() to unmap the drive. 

1. Open file: The open file function will return a pointer to a file descriptor that represents the file or directory specified by path, or will return NULL if the file or directory does not exist. I will implement my own file pointer. 

2. Close file: 
This will free up the pointer and change the isOpen in the file's meta data back to 0

3. Create file: 
My create file function will creates a new file with the specified directory path and returns a pointer to a handle to that file, or return NULL if the file already exists. The file will
be written to one or more blocks if needed.

4. Create Directory:
To create directory, I will use the create file function and change the type to directory type. This is because in my file system, a directory is treated mostly the same as a file.
This is only my proposed solution. If needed, I will make a separate function for creating directory, or even make a new struct to define directories.

5. Delete file: 
To delete file, I will make use of the memset() function. Before deleting a file, I will check if it exists and is not currently being opened. If one of the two conditions is not correct,
the function will do nothing. To delete directories, I will check if it is containing other files. A directory can only be deleted if it is empty.

6. Get size:
I plan to use this function to help with opening, reading, and writing files, by accessing the entries of the files themselves. This functions will be a helper function for the above functions.

7. Read & Write file:
These functions will probably take in parameters such as the file pointer, the drive pointer, and integer for the length of byte read/write, and the writing content (if writing), or the reading destination (if reading).
Before reading & writing, I will have to check if the file exists, is not a directory, and is not currently being opened in another pointer.

III. Testing
I add a testing program to the main function so people can run and try all the function that I have. The testing that I added has no problem (with the current function) and can be improved more in the future with the completation of the program.

