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
* File: FreeSpace.c
*
* Description: The FreeSpace file holds the structure of the
* free space map that writes into the volume. Keep track of
* the available space in the sample volume.
**************************************************************/

#include <stdlib.h>

#include "fsLow.h"
#include "FreeSpace.h"

#include <stdio.h>
#include <string.h>

/*******************************************************
* void initFreeSpace()
* - This function will initialize the free space,
* mark the spaces off the blocks when the file system
* is started. Which would include spaces for,
* FreeSpace map, and Volume control block.
* - Marked with 0 if a block is free
* - Marked with X if a block is being used
*******************************************************/
void initFreeSpace() 
{
    FsArray freeArray;
    FsArray *freeArrayptr;

    // Marks off the space that is free
    for (int i = 0; i < (VOLUME_SIZE / BLOCK_SIZE); i++) 
    {
        freeArray[i] = 'O';
    }

    // 0 vcb, 1 root (verify #of blocks), 2 free space [xxx000000....]
    // Marks off the blocks for free space
    for (int i = 0; i < 3; i++) 
    {
        freeArray[i] = 'X';
    }


    int retValue = LBAwrite( &freeArray,FREE_ARRAY_SIZE, FREE_ARRAY_LOCATION);
    
}

/*******************************************************
* FileScope findFree(int blockAmount)
* - This function will allocate and find a free
* set of blocks. It will check for the amount of blocks
* needed by a user and search the free space array
* to write to the file.
*******************************************************/
FileScope findFree(int blockAmount) 
{

    int counter = 0;

    //Read the free space map into buffer
    char* buffer = malloc(BLOCK_SIZE*FREE_ARRAY_SIZE);
    LBAread(buffer, 39, 1);
    FileScope bitMap;

    //search for the set of available blocks.
    //Checks for if the user needs a block or more than one block
    for(int i = 0; i < VOLUME_SIZE/BLOCK_SIZE; i++) 
    {
        //single block
        if(buffer[i] == 'O' && blockAmount == 1) 
        {

            bitMap.start = i;
            bitMap.end = i;
            buffer[i] = 'X';
            break;
        }
        ///multiple blocks
        if(buffer[i] == 'O' && blockAmount > 1) 
        {
            counter++;
            if(counter == blockAmount) 
            {

                bitMap.start = i - blockAmount;
                bitMap.end = i;
                for(int j = bitMap.start; j < bitMap.end; j++) 
                {
                    buffer[j] = 'X';

                }
                break;

            }

        }

    }

    LBAwrite(buffer,39, 1); //write to our free space
    free(buffer); //free memory allocated

    return bitMap;
}

void markFree(int location){

    char* freeUp = malloc(FREE_ARRAY_SIZE*BLOCK_SIZE);

    LBAread(freeUp,FREE_ARRAY_SIZE,FREE_ARRAY_LOCATION);
    freeUp[location] = 'O';

    LBAwrite(freeUp,FREE_ARRAY_SIZE,FREE_ARRAY_LOCATION);
    free(freeUp);

}

/*
freeData getFreeSpace(int blockAmount){
    freeData file;
    file.start = 0;
    file.end = 0;
    file.freeBlockCount = 0;

    char* freeSpaceArray = malloc(FREE_ARRAY_SIZE);
    LBAread(freeSpaceArray, sizeof(freeSpaceArray), FREE_ARRAY_LOCATION);
    int i = 0;
    for(i; i<FREE_ARRAY_SIZE; i++){
        if(freeSpaceArray[i]=='X'){
            file.start = i+1;
        }
        if(i - file.start > blockAmount){
            file.end = i;
            break;
        }
    }
    for(i; i<FREE_ARRAY_SIZE; i++){
        if(freeSpaceArray[i]=='O'){
            file.freeBlockCount++;
        }
    }
    free(freeSpaceArray);
    printf("BLOCKS TAKEN: %d\nFile End: %d\nFile Start: %d\n", file.end -file.start, file.end, file.start);

    return file;
}
void markUsedSpace(freeData file){
    char* freeSpaceArray = malloc(FREE_ARRAY_SIZE);
    LBAread(freeSpaceArray, sizeof(freeSpaceArray), FREE_ARRAY_LOCATION);
    if(file.end != 0){
        int i = file.start;
        for(i; i<= file.end; i++){
            freeSpaceArray[i] = 'X';
        }
    }
    LBAwrite(freeSpaceArray, FREE_ARRAY_SIZE, FREE_ARRAY_LOCATION);
}
*/