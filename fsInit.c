/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
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


	fsDir* dir = initRootDir();
	VolumeControlBlock* vcb = vcbInit(BLOCK_SIZE, floor(BLOCK_SIZE/VOLUME_SIZE));

	initFreeSpace();
	LBAwrite(vcb,1,0);
	LBAwrite(dir,1,1);

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}