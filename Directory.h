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
* File: Directory.h
*
* Description:
* This file holds the general structure of each directory that
* will be written into volume. It also includes the prototype to
* all of its related functions.
**************************************************************/
//#include "VolumeControlBlock.h"

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define ROOT_DIR_LOCATION 1
#define MAX_DIR_ENTRIES 10
#define MAGIC_NUM_DIR 9
#define DIR_SIZE 1

typedef struct fsDirEntry {
    int id;                     // file id 
    char filename[32];          // file name
    int entrySize;              // size of file
    int fileBlockLocation;      // file location
    char isADir;                // is file a directory?
    char dateCreated[10];       // date created 
    char author[32];            // file author
    char permissions[9];        // file permissions
} fsDirEntry;


typedef struct DirectoryInfo {
    char name[32];              // directory name 
    int parentBlockLocation;    // location for parent block
    int currentBlockLocation;   // location of self
    int magicNumber;            // file magic number 
    fsDirEntry directryEntries [MAX_DIR_ENTRIES];
} fsDir;

fsDir* initRootDir();
fsDir* findDir(const char* name);
fsDir* findDirFrom(fsDir* src, char* dirname);
fsDir* fetchRootDir();
fsDir* loadDirFromBlock(int blockLocation);



#endif //DIRECTORY_H