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
* File: fsShell.c
*
* Description: The FreeSpace file holds the structure of the
* free space map that writes into the volume. Keep track of
* the available space in the sample volume.
**************************************************************/

#include <stdlib.h>

#include "fsLow.h"
#include "FreeSpace.h"

#include <stdlib.h>
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

    // 0 vcb, 5 fsm (verify #of blocks), 6 root [xxxxxx, ooooooooooooooooo....]
    // Marks off the blocks for free space
    for (int i = 0; i < 3; i++) 
    {
        freeArray[i] = 'X';
    }

    /*
    // Marks the location off of free space
    for (int i = FS_LOCATION - 1; i < VOLUME_SIZE / BLOCK_SIZE; i++) 
    {
        freeArray[i] = 'X';
    }
    */
    //freeArrayptr = &freeArray;
    int retValue = LBAwrite( &freeArray,FREE_ARRAY_SIZE, FREE_ARRAY_LOCATION);
    
}