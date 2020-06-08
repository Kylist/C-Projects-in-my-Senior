#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "utils.h"

int main()
{
    int fd = open("drive", O_RDWR | O_CREAT, 0755);
    ftruncate(fd, sizeof(fileDrive)+sizeof(fs_get_filesize));
    fileDrive *D = mmap(0, sizeof(fileDrive), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    initializeDrive(D);

    printf("Testing directory creation\n");
    fs_mkdir("/testingDir", D);

    printf("\nTesting subdirectory creation\n");
    fs_mkdir("/testingDir/testingSubDir", D);

    printf("\nTesting file creation\n");
    fs_create("/testingDir/testingSubDir/myFile.txt", 0, D);

    printf("\nTesting file opening\n");
    fP * ptr = fs_open("/testingDir/testingSubDir/myFile.txt", D);
    printf("\nIf writeFile (below) is successful, then open file also succeeded\n");

    printf("\nSleeping for 3 seconds before writing to file to change modified time\n");
    sleep(3);

    printf("\nTesting file write\n");
    for(int i = 0; i<5; i++){
        fs_write(ptr, "I am testing writeFile 5 times", 31, D);
    }
    printf("\nRewinding pointer to reuse for readFile\n");
    rewindF(ptr);
    printf("\nTesting readFile 5 times\n");
    char buff[31];
    int i = 0;
    int k;
    while((k = fs_read(buff, ptr, 31, D)) != 0&&i<5){
    	printf("Just read \"%s\" from file %s\n", buff, ptr->meta->name);
	printf("Bytes read: %d, i: %d\n",k,i);
	i++;
    }

    printf("\nTesting print metadata function for file. Because I sleep for 3 seconds before writing, modified time should be 3 seconds later than creation time\n\n");
    printPointerInfo(ptr);

    printf("\nTesting file deletion before closing file pointer. Function should fail. Below is the message from deleteFile:\n");
    fs_delete("/testingDir/testingSubDir/myFile.txt", D);

    printf("\nTesing closeFile and retry file deletion. If file deletion is successful, then closeFile is working properly. Below is the message from deleteFile:\n");
    fs_close(ptr);
    fs_delete("/testingDir/testingSubDir/myFile.txt", D);

    printf("\nRetrying openFile. If deleteFile works properly, openFile will fail. Below is the message from openFile:\n");
    ptr = fs_open("/testingDir/testingSubDir/myFile.txt",D);

    munmap(D, sizeof(fileDrive));
    return 0;
}

// Initialize drive
void initializeDrive(fileDrive *D) {
    // Set all bytes to 0
    memset(D, 0, sizeof(fileDrive));
    // Mark the start of the root directory
    D->FAT[0] = -1;
}


// Allocate a new block
int allocateBlock(fileDrive *D) {
    // Find the first empty block
    for (int i = 1; i < NUMB; i++) {
        if (D->FAT[i] == 0) {
            D->FAT[i] = -1;
            // Set block to occupied and return the block number
            return i;
        }
    }
    //If everything is occupied, return -1
    return -1;
}

fs_get_filesize * get_filesize(char *path, fileDrive *D){
    if (strcmp(path, "") == 0) {
        return &ROOT;
    }
    char cpyPath[strlen(path)];
    strcpy(cpyPath, path);

    //create a pointer to  null 
    fs_get_filesize *meta = &NULLFILE;
    // Initialize loop variables
    int found = 0; //1 if found the File with the path
    int last = 1; //type of the last file read
    // Split one token at a time
    char * token;
    for (token = strtok(cpyPath, "/"); token != NULL; token = strtok(NULL, "/")) {
        // Can't enter a file, return NULL
        if (last == 0) {
            printf("Cannot access the path %s\n", path);
            return NULL;
        }
        // Traverse directory
        for (int b = meta->startBlock; b != -1 && !found; b = D->FAT[b]) {
            for (int i = 0; i < BSIZE / sizeof(fs_get_filesize) && !found; i++) {
                if (strcmp(D->data[b].dir[i].name, token) == 0) {
                    // Name matches, continue search
                    last = D->data[b].dir[i].isDir;
                    meta = &(D->data[b].dir[i]);
                    found = 1;
                }
            }
        }
        // If found = 0 after the loop, file doesn't exist
        if (!found) {
            return NULL;
        }
        found = 0;
    }
    // File was never found, return NULL
    if (meta == &NULLFILE) {
	printf("File (Directory) not found\n");
        return NULL;
    }
    // Return the last pointer found
    return meta;
}

// Create a file at the given path
int fs_create(char *path, int type, fileDrive *D) {
    // Copy the path into another string
    char cpyPath[strlen(path)];
    strcpy(cpyPath, path);
    // Check if file already exists
    if (get_filesize(cpyPath, D) != NULL) {
        printf("File path %s already exist!\n", path);
        return -1;//Failed
    }
    // Split name of new file into new variable
    char *name = strrchr(cpyPath, '/');
    if (name == NULL) { //Invalid name
        return -1;
    } else {
	//remove the '/' character
        *name = '\0';
        name = name + 1;
    }
    // Make sure name of directory/file is less than 20 characters
    if (strlen(name) > MAXLEN) {
        return -1;
    }
    // Find start of directory
    int dir;
    fs_get_filesize *dirMeta = get_filesize(cpyPath, D);
    // Check if directory exists
    if (dirMeta == NULL) {
	printf("Directory path does not exist\n");
        return -1;
    } else {
        // Set the start block
        dir = dirMeta->startBlock;
    }
    // Allocate block
    int fileStart = allocateBlock(D);
    if (fileStart == -1) {
        printf("Failed to allocate block\n");
        // if allocation failed, return -1
        return -1;
    }

    // Find first empty slot in directory
    for (; dir != -1; dir = D->FAT[dir]) {
        for (int i = 0; i < BSIZE / sizeof(fs_get_filesize); i++) {
            if (D->data[dir].dir[i].name[0] == '\0') {
                D->data[dir].dir[i].startBlock = fileStart;
                D->data[dir].dir[i].fileSize = 0;
                D->data[dir].dir[i].isDir = type;
                strcpy(D->data[dir].dir[i].name, name);
		if(type==0){
		printf("Successfully create file at path: %s\n", path);
		} else {
		printf("Successfully create directory at path %s\n", path);
		}
                return 0; //Success
            }
        }
    }
    // If reaches this part of the code, previous block is full => allocate new block for directory
    int newBlock = allocateBlock(D);
    if (newBlock == -1) {
        printf("Failed to allocate block\n");
        return -1;
    }
    // Create the file entry in the new block
    D->FAT[dir] = newBlock;
    D->data[newBlock].dir[0].startBlock = fileStart;
    D->data[newBlock].dir[0].fileSize = 0;
    D->data[newBlock].dir[0].isDir = type;
    strcpy(D->data[newBlock].dir[0].name, name);
    if(type==0){
	printf("Successfully create file at path: %s\n", path);
    } else {
	printf("Successfully create directory at path %s\n", path);
    }
    return 0;
}

int fs_mkdir(char * path, fileDrive * D){
	return fs_create(path, 1, D);
}

// Write to file pointer
int fs_write(fP *file, char *data, int len, fileDrive *D) {
    
    // Keep track of what has been written
    int offset = 0;
    // To prevent block overflow
    int amount = file->ptr + len > BSIZE ? BSIZE - file->ptr : len;
    // Loop while there is data left to write
    while (len > 0) {
        if( memcpy(&(D->data[file->currentBlock].fileContent[file->ptr]), data + offset, amount) == NULL){
		printf("Write failed\n");
		return -1;
	}
        // Advance loop
        len -= amount;
        file->ptr += amount;
        offset += amount;
        file->meta->fileSize += amount;
        if (file->ptr == BSIZE&&len>0) {
            // Add a new block and set it to the current block
	    int newB = allocateBlock(D);
	    if(newB == -1){
 		printf("fs_write stopped because drive is full\n");
		return offset;
	    }
            D->FAT[file->currentBlock] = newB;
            file->currentBlock = D->FAT[file->currentBlock];
            // Reset pointer
            file->ptr = 0;
        }
        amount = len > BSIZE ? BSIZE : len;
    }
    printf("Just write \"%s\" to file %s\n", data, file->meta->name);
    return offset; //return number of bytes written
}

// Open file
fP *fs_open(char *path, fileDrive *D) {
    char cpyPath[strlen(path)];
    strcpy(cpyPath, path);
    // Create pointer for file entry
    fs_get_filesize *meta = get_filesize(cpyPath, D);
    // If file doesn't exist or if file is already open, it can't be opened
    if (meta == NULL || meta->isOpen) {
	printf("File %s not found or is being opened in another pointer\n",path);
        return NULL;
    }
    // Create file pointer
    fP *file = (fP*) malloc(sizeof(fP));
    file->meta = meta;
    file->ptr = 0;
    file->currentBlock = file->meta->startBlock;
    file->meta->isOpen = 1;

    return file;
}

// Read from file pointer
int fs_read(char *buff, fP *file, int len, fileDrive *D) {
    // To advance pointer
    int offset = 0;
    // clear the buffer string
    memset(buff, 0, len);
    // Calculate amount of data to be read
    int amount = file->ptr + len> BSIZE ? BSIZE - (file->ptr) : len;
    if(file->currentBlock==-1){
	printf("Reading stopped because end of file has been reached\n");
	return 0; //Read nothing because pointer reaches the end of file
    }
    while (len > 0 && file->currentBlock != -1) {

        // Copy data into the destination array
        memcpy(buff + offset, &(D->data[file->currentBlock].fileContent[file->ptr]), amount);
        // Advance loop
        offset += amount;
        len -= amount;
        file->ptr += amount;
        if (file->ptr == BSIZE) {
            // If reaches end of block, reset file pointer and move to next block
            file->ptr = 0;
            file->currentBlock = D->FAT[file->currentBlock];
        }
        amount = len > BSIZE ? BSIZE : len;
    }
    return offset; //Return number of bytes read
}

// Close file
void fs_close(fP *file) {
    file->meta->isOpen = 0;
    free(file);
}

// Delete file
int fs_delete(char *path, fileDrive *D) {
    char cpyPath[strlen(path)];
    strcpy(cpyPath, path);
    // Get the file's metadata
    fs_get_filesize *file = get_filesize(cpyPath, D);
    if (file == NULL || file->isOpen) {
	printf("Cannot delete %s: file does not exist or is being opened\n", path);
        return -1;
    }
    // Check if the file is a directory
    if (file->isDir == 1) {
        // Check if directory is empty
        for (int block = file->startBlock; block != -1; block = D->FAT[block]){
            for (int i = 0; i < BSIZE / sizeof(fs_get_filesize); i++) {
                if (D->data[block].dir[i].name != '\0') {
	            printf("Cannot delete %s: directory is not empty\n", path);
                    return -1;
                }
            }
        }
    }

    // Set all blocks to free
    for (int block = file->startBlock, nextBlock = -1; block != -1; block = nextBlock) {
        nextBlock = D->FAT[block];
        D->FAT[block] = 0;
    }
    // Delete the file's entry
    memset(file, 0, sizeof(fs_get_filesize));
    printf("Successfully delete %s\n", path);
    return 0;
}

// Rewind pointer to starting position
void rewindF(fP *file) {
    file->currentBlock = file->meta->startBlock;
    file->ptr = 0;
}



//print entry using filesize struct
void printInfo(fs_get_filesize * e){
    printf("Infomation about the file:\n");
    printf("Name: %s\n", e->name);
    printf("File size: %d bytes\n", e->fileSize);
    char * type = e->isDir == 0 ? "file" : "directory";
    
}

//print entry using file pointer
void printPointerInfo(fP * f){
    printf("Infomation about the file:\n");
    printf("Name: %s\n", f->meta->name);
    printf("Start block: %d\n", f->meta->startBlock);
    printf("File size: %d bytes\n", f->meta->fileSize);
    char * type = f->meta->isDir == 0 ? "file" : "directory";
    
    
}
