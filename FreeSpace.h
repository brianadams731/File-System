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
* File: FreeSpace.h
*
* Description: The FreeSpace file holds the strucutre of the
* free space map that writes into the volume. Keep track of
* the available space in the sample volume.
**************************************************************/
#ifndef FREESPACE_H
#define FREESPACE_H

#ifndef VOLUME_SIZE
#define VOLUME_SIZE 10000000
#endif
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 512
#endif

#ifndef FS_LOCATION
#define FS_LOCATION 19459
#endif

#ifndef FREE_ARRAY_SIZE
#define FREE_ARRAY_SIZE 39
#endif

#ifndef FREE_ARRAY_LOCATION
#define FREE_ARRAY_LOCATION 2
#endif


typedef char FsArray[(VOLUME_SIZE/BLOCK_SIZE)];

static int initializeFreeSpace = 0;

typedef struct LBARange {
    int start;
    int end;
} FileScope;

void initFreeSpace();

FileScope findFreeSpace(int blockSize);



#endif