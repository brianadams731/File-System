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
#define FREE_ARRAY_SIZE (VOLUME_SIZE/BLOCK_SIZE)
#endif

#ifndef FREE_ARRAY_LOCATION
#define FREE_ARRAY_LOCATION 3
#endif

#ifndef FREE_BLOCK_CONSUMPTION
#define FREE_BLOCK_CONSUMPTION (FREE_ARRAY_SIZE/BLOCK_SIZE)
#endif


typedef char FsArray[FREE_ARRAY_SIZE];

static int initializeFreeSpace = 0;

typedef struct mapLocation {
    int start;
    int end;
} FileScope;

void initFreeSpace();

FileScope findFree(int blockAmount);
void markFree(int location);


typedef struct freeData{
    int start; // FIRST BLOCK WRITTEN TO
    int end; // LAST BLOCK WRITTEN TO, this block is not free
    long freeBlockCount;
} freeData;

/*
 * This function will mark a block as used in the vcb.
 * X if a block is taken 
 */
void markUsedSpace(freeData file);
/*
 * This function will mark block by block as used in the vcb.
 * X if a block is taken 
 */
void markUsedSpaceByBlock(int start, int numberOfBlocks);
/*
 * This function will mark a location in the vcb as free
 * 0 for a free block
 */
void markFreeSpace(int location, int size);
/*
* @ params blockAmount: the amount of blocks needed by the user
* This function will check to see if there is free space in
* the vcb, will return the amount needed.
* X if a block is taken 
* O for a free block
*/
freeData getFreeSpace(int blockAmount);

/*
 * This function will check what has been written
 * to the block in the vcb.
 */
char* getDataFromBlock(char* buffer, int bufferSize);
/*
 * This function will get a key from a block to
 * keep track of keeping data together
 */
int getKeyFromBlock(char* buffer, int bufferSize);
/*
 * This function will write a key to a block that will
 * keep track of data being stored. Allowing for contiguous
 */
int writeKeyToBuffer(char* buffer, int bufferSize, int key);

#endif