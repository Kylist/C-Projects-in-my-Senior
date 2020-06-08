#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>

#ifndef UTILS_UTILS_H
#define UTILS_UTILS_H

#define BSIZE 512
#define NUMB 256
#define MAXLEN 20

typedef struct {
    char name[MAXLEN];
    int startBlock;
    int fileSize;
    int isDir;
    int isOpen;
} fs_get_filesize; //File (directory) metadata 

fs_get_filesize NULLFILE = {0}; //Use for error checking
fs_get_filesize ROOT = {
	.isDir = 1,
        .startBlock = 0,
}; //Root fs_get_filesize

typedef union {
    char fileContent[BSIZE]; //content of file
    fs_get_filesize dir[BSIZE / sizeof(fs_get_filesize)];
} block;

typedef struct {
    int FAT[NUMB];
    block data[NUMB];
} fileDrive; //The drive consists of 256 blocks and an array to keep track of them

typedef struct {
    fs_get_filesize *meta; //Meta data of file
    int ptr; //Current position within the current block
    int currentBlock; //Current block of file
} fP; //File Pointer

fs_get_filesize *get_filesize(char *, fileDrive *);
int fs_create (char *, int, fileDrive *);
int fs_write (fP*, char*, int, fileDrive *);
int fs_read (char *, fP *, int , fileDrive *);
fP * fs_open(char *, fileDrive *);
void fs_close (fP *);
int fs_delete (char *, fileDrive *);
void rewindF (fP *);
int allocateBlock(fileDrive *);
void initializeDrive(fileDrive *);
int fs_mkdir(char *, fileDrive *);
void printFileSizeInfo(fs_get_filesize *);
void printPointerInfo(fP *);

#endif
