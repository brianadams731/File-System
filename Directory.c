/**************************************************************
* Class:  CSC-415-02 - Spring 2022
* Names: Salaar Karimzadeh,
         Brandon Cruz-Youll,
         Brian Adams,
         Jasmine Thind
* Student IDs:920765141, 902899041, 921039987, 920767229
* GitHub Name: BeeSeeWhy
* Group Name: Zombies
* Project: Basic File System
*
* File: Directory.c
*
* Description: This file provides the implementation for the initialization of the 
* root directory. The root directory contains it's block location, and its parent block location 
* (which is it self). The root directory also contains a magic number for identification, as well as 
* an array of directory entries. These directory entries contain metadata about the files that the root
* directory tracks. These directory entries contain the file id's, block location, size, author, date created etc.
* The first two indexes represent the parent and self. Index 0 would represent .. (the parent), and index 1 would
* represent . (self). Because this is the root index the parent and self both point the root (self). 
* 
**************************************************************/
#include "Directory.h"
#include <stdlib.h>
#include "fsLow.h"
#include <stdio.h>
#include <string.h>

// TODO: Add correct root location
#define rootLocation 1;

fsDir* initRootDir(){
    fsDir* root = malloc(sizeof(fsDir));
    strcpy(root->name, "root");
    root->currentBlockLocation = rootLocation;
    root->parentBlockLocation =  rootLocation;

    int i = 0;
    for(i; i<sizeof(root->directryEntries)/sizeof(fsDirEntry);i++){
        if(i<2){
        strcpy(root->directryEntries[i].filename, i==0?".":"..");
        root->directryEntries[i].entrySize = 1;
        root->directryEntries[i].isADir = 'T';
        root->directryEntries[i].fileBlockLocation = root->currentBlockLocation;
        strcpy(root->directryEntries[i].author, "OS");
        // TODO: Permissions if in project scope
        // TODO: Date created
        }else{
            strcpy(root->directryEntries[i].filename, "");
            root->directryEntries[i].id = -1;
        }
    }

    return root;
}

fsDir* makeDir(const char* name, int blockLocation, fsDirEntry parentDirEntry ){
    fsDir* dir = malloc(sizeof(fsDir));
    strcpy(dir->name, name);
    dir->currentBlockLocation = blockLocation;
    dir->parentBlockLocation =  parentDirEntry.fileBlockLocation;

    int i = 0;
    for(i; i<sizeof(dir->directryEntries)/sizeof(fsDirEntry);i++){
        if(i==0){
            strcpy(dir->directryEntries[i].filename,".");
            dir->directryEntries[i].entrySize = 1;
            dir->directryEntries[i].isADir = 'T';
            dir->directryEntries[i].fileBlockLocation = dir->currentBlockLocation;
            strcpy(dir->directryEntries[i].author, "USER");
        }else if(i==1){
            strcpy(dir->directryEntries[i].filename, "..");
            dir->directryEntries[i].entrySize = 1;
            dir->directryEntries[i].isADir = 'T';
            dir->directryEntries[i].fileBlockLocation = parentDirEntry.fileBlockLocation;
            strcpy(dir->directryEntries[i].author, "USER");
        }else{
            strcpy(dir->directryEntries[i].filename, "");
            dir->directryEntries[i].id = -1;
        }
    }
    return dir;
}

void addDirEntryFromDir(fsDir* targetDir, fsDir* sourceDir, int targetIndex){
    strcpy(targetDir->directryEntries[targetIndex].filename, sourceDir->name);
    targetDir->directryEntries[targetIndex].entrySize = sourceDir->directryEntries[0].entrySize;
    targetDir->directryEntries[targetIndex].isADir = sourceDir->directryEntries[0].isADir;
    targetDir->directryEntries[targetIndex].fileBlockLocation = sourceDir->currentBlockLocation;
    strcpy(targetDir->directryEntries[targetIndex].author, sourceDir->directryEntries[0].author);
}

/*fsDir* findDir(const char* name){

}*/

fsDir* fetchRootDir(){
    return loadDirFromBlock(ROOT_DIR_LOCATION);
}

fsDir* loadDirFromBlock(int blockLocation){
    fsDir* dir = malloc(MINBLOCKSIZE * DIR_SIZE);
    LBAread(dir, DIR_SIZE, blockLocation);
    return dir;
}

fsDirEntry* findDirEntry(fsDir* src, char* dirEntryName){
    int i = 0;
    fsDirEntry* dirEntry = NULL;
    for(i;i< MAX_DIR_ENTRIES;i++){
        if(strcmp(src->directryEntries[i].filename, dirEntryName) == 0){
            return &src->directryEntries[i];
        }
    }
    return dirEntry;
}

// For debug only, used to make sure dir is being initalized correctly
/*
int main(){
    fsDir* test = initRootDir();
    printf("Name: %s\n", test->name);
    printf("Location: %d\n", test->currentBlockLocation);
    printf("Parent: %d\n", test->parentBlockLocation);
    printf("DirEntry: 0 %s\n",test->directryEntries[0].author);
    printf("DirEntry: 1 %s\n",test->directryEntries[0].author);
    return 0;
}
*/