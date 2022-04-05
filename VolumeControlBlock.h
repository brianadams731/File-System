/**************************************************************
* Class:  CSC-415-0# - Fall 2021
* Names: Salaar Karimzadeh,
         Brandon Cruz-Youll,
         Brian Adams,
         Jasmine Thind
* Student IDs:920765141, 902899041, 921039987, 920767229
* GitHub Name: BeeSeeWhy
* Group Name: Zombies
* Project: Basic File System
*
* File: VolumeControlBlock.c
*
* Description: 
**************************************************************/

#define VOLUME_SIZE 100000
#define BLOCK_SIZE 512
#define VOLUME_LOCATION 0 


typedef struct VolumeControlBlock{
    int totalBlocks;  //Overall size of the blocks in the volume
    int blockSize;   // The size of our volume block
    //int totalByteSize;
    int magicNumber; 
    int rootDirectoryBlock;
    //int initialized;
    int freeBlockCount;
 
} VolumeControlBlock;

VolumeControlBlock* vcbInit(int blockSize,int totalBlocks);