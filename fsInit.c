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
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
It checks if the volume control block pointer matches the signature, 
if it does it will initalizze the root directory, volume control block
and free space. 
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Directory.h"
#include "VolumeControlBlock.h"
#include "FreeSpace.h"

#include "fsLow.h"
#include "mfs.h"


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	VolumeControlBlock* vcb = malloc(BLOCK_SIZE);
	LBAread(vcb, 1, 0);

	if(vcb->magicNumber != 16) 
		{
		VolumeControlBlock* vcb = vcbInit(BLOCK_SIZE, floor(BLOCK_SIZE/VOLUME_SIZE));
		fsDir* dir = initRootDir();
		initFreeSpace();
		LBAwrite(vcb,1,VOLUME_LOCATION);
		LBAwrite(dir,DIR_SIZE, ROOT_DIR_LOCATION);
		}
	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}