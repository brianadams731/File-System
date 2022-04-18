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
* File: VolumeControlBlock.h
*
* Description: This header file includes the structure of the volume control block. 
* All interfaces in this header are implemented in VolumeControlBlock.c, and are consumed by
* fsInit. The interface for the VolumeControlBlock struct is defined here,
* as well as the interface for vcbInit.
**************************************************************/

#define VOLUME_SIZE 10000000
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
    char name[10];
 
} VolumeControlBlock;

VolumeControlBlock* vcbInit(int blockSize,int totalBlocks);