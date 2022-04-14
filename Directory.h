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
#define DIR_SIZE 2

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

/*
* @ owns: nothing, you will need to free the memory returned
* @ params name: name of directory
* @ params blockLocation: which block in the lba the dir will be located at
* @ params parentDirEntry: the DirectoryEntry of the parent 
*/
fsDir* makeDir(const char* name, int blockLocation, fsDirEntry parentDirEntry);
/*
* @ params targetDir: directory you want to add a Directory Entry to
* @ params sourceDir: directory you want to get your Directory Entry data from
* @ params targetIndex: index of the targetDir Directory entry that you want to write the sourceDir data to
* @ returns void
*/
void addDirEntryFromDir(fsDir* targetDir, fsDir* sourceDir, int targetIndex);
//fsDir* findDir(const char* name);

/*
* WARNING: Will return null if entry is not located in directory
* @ owns: its return value, do not free the entry returned
* @ param src: The directory this function will search in
* @ param dirname: the string name of entry to find, this is case sensitive
*/
fsDirEntry* findDirEntry(fsDir* src, char* dirname);
fsDir* fetchRootDir();

/*
* WARNING: Check that the block you reading holds a directory, this function will not check that 
* what its reading is a dir, it will assume it is.
*
* @ owns: nothing, You are incharge of deallocating the memory used by the return
*/
fsDir* loadDirFromBlock(int blockLocation);



#endif //DIRECTORY_H