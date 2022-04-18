/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"

#include "parsePath.h"
#include "Directory.h"
#include "FreeSpace.h"
#include "VolumeControlBlock.h"
#include "fsLow.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

#define BufferWithKeyOffset (BLOCK_SIZE - sizeof(int))


typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer

	int blockOfDirFileIsIn;
	int isStart;
	int isWrite;
	char name[32];
	int initialKey;
	int blockCount;
	int fileSize;

	int prevKey;
	fsDirEntry entry;
	char accessMode[3];  // -- or rwd

	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

char writeBlock[BLOCK_SIZE];
int bytesInWriteBlock;

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
		
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	if(returnFd != -1){
		// set up fcb on write
		
    	if(filename[0] != '/'){
			char* currentPath = malloc(300);
			fs_getcwd(currentPath, 300);
        	fs_Path* parsedPath = parsePath(currentPath);
			if(!parsedPath){
				printf("Error: Path not valid");
				return -1;
			}
			fcbArray[returnFd].blockOfDirFileIsIn = parsedPath->entry->fileBlockLocation;
		}else{
			char usePath[300];
        	// handel absolute path;
    	}
		

		fcbArray[returnFd].isStart = 1;
		fcbArray[returnFd].index = 0;
		fcbArray[returnFd].prevKey = -1;
		fcbArray[returnFd].buflen = -1;
		fcbArray[returnFd].isWrite = 0;
		strcpy(fcbArray[returnFd].name, filename);
	}
	
	return (returnFd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
		
	if (startup == 0) b_init();  //Initialize our system
	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)){
		return (-1); 					//invalid file descriptor
	}

	if(fcbArray[fd].isStart){
		fcbArray[fd].isStart = 0;
		fcbArray[fd].isWrite = 1;
		fcbArray[fd].buflen = 200;
		freeData freeSpace = getFreeSpace(1);

		fcbArray[fd].initialKey = freeSpace.start;
		fcbArray[fd].prevKey = freeSpace.start;
		//printf("INIT KEY: %d\n",freeSpace.start);
		if(freeSpace.end == 0){
			printf("NO FREE SPACE\n");
			return -1;
		}
	}

	int bytesLeftToFillBuffer = BufferWithKeyOffset - bytesInWriteBlock - 1;

	if(count < bytesLeftToFillBuffer){
		//printf("FILLING BUFFER\n");
		memcpy(&writeBlock[bytesInWriteBlock], buffer, count);
		bytesInWriteBlock = bytesInWriteBlock + count;
		if(count < fcbArray[fd].buflen){ // last write
			//printf("\n\n\nLAST BUFFER\n\n\n");
			//printf("\nLast Count In Block: %d\n", bytesInWriteBlock);
			//printf("\n%s\n",writeBlock);
			int i = count;
			for(i; i<BufferWithKeyOffset; i++){
				writeBlock[i] = '0';
			}

			LBAwrite(writeBlock,1, fcbArray[fd].prevKey);
			fcbArray[fd].blockCount++;
			fcbArray[fd].fileSize = fcbArray[fd].fileSize + count;
			markUsedSpaceByBlock(fcbArray[fd].prevKey, 1);
		}
	}else{
		memcpy(&writeBlock[bytesInWriteBlock], buffer, bytesLeftToFillBuffer);
		markUsedSpaceByBlock(fcbArray[fd].prevKey, 1);
		freeData freeSpace = getFreeSpace(1);
		int key = freeSpace.start;
		//printf("Bytes in Buff: %d, Bytes Left To Fill Buff: %d\n", bytesInWriteBlock, bytesLeftToFillBuffer);
		//printf("Count In Block: %d", bytesInWriteBlock + bytesLeftToFillBuffer);
		//printf("\n%s\n",writeBlock);

		writeKeyToBuffer(writeBlock, BLOCK_SIZE, key);

		LBAwrite(writeBlock,1, fcbArray[fd].prevKey);
		fcbArray[fd].blockCount++;
		fcbArray[fd].fileSize = fcbArray[fd].fileSize + bytesLeftToFillBuffer + sizeof(int); // TODO: Consider if file size should include key?
		
		fcbArray[fd].prevKey = key; // need to write after LBAwrite

		int startOfNextBlock = count - bytesLeftToFillBuffer;
		memcpy(&writeBlock[0], &buffer[bytesLeftToFillBuffer], startOfNextBlock);
		bytesInWriteBlock = startOfNextBlock;
		
		//printf("START OF NEXT BLOCK: %d\n", startOfNextBlock);
		if(freeSpace.end == 0){
			printf("ERROR: Insufficient freespace\n");
			return -1;
		}

		if(count < fcbArray[fd].buflen){ // last write
			//printf("\n\nLAST BUFFER\n\n\n");
			//printf("LEFT: %d", count - startOfNextBlock);
			//printf("START OF NEXT BLOCK: %d", startOfNextBlock);

			int i = startOfNextBlock;
			for(i; i<BufferWithKeyOffset; i++){
				writeBlock[i] = '0';
			}

			//printf("\n%s\n",writeBlock);
			LBAwrite(writeBlock, 1, fcbArray[fd].prevKey);
			markUsedSpaceByBlock(fcbArray[fd].prevKey, 1);
			fcbArray[fd].fileSize = fcbArray[fd].fileSize + startOfNextBlock;
		}
	}

	//TODO: Figure out ret val
	return (0); //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	return (0);	//Change this
	}
	
// Interface to Close the file	
void b_close (b_io_fd fd)
	{
		if(fcbArray[fd].isWrite){
			fsDir* parentDir = loadDirFromBlock(fcbArray[fd].blockOfDirFileIsIn);
			fsDirEntry* entryToAdd = malloc(sizeof(fsDirEntry));

			strcpy(entryToAdd->author,"user");
			strcpy(entryToAdd->filename, fcbArray[fd].name);

			entryToAdd->dateCreated = getCurrentDateTime();
			entryToAdd->lastModified = getCurrentDateTime();
			entryToAdd->lastAccess = getCurrentDateTime();
			entryToAdd->fileBlockLocation = fcbArray[fd].initialKey;
			strcpy(entryToAdd->permissions,"rwd");
			entryToAdd->fileSizeBytes = fcbArray[fd].fileSize;
			entryToAdd->entrySize = fcbArray[fd].blockCount;
			entryToAdd->isADir = 'F';

			addExistingDirEntry(parentDir, entryToAdd);
			LBAwrite(parentDir, DIR_SIZE, parentDir->currentBlockLocation);
			free(parentDir);
			free(entryToAdd);
		}


		//TEST CODE
		/*
		fsDir* rootDir = fetchRootDir();
		fsDirEntry* dirEntry = findDirEntry(rootDir, "test");
		char* buff = malloc(BLOCK_SIZE);
		int nextKey = dirEntry->fileBlockLocation;

		int i = 0;
		for(i; i <= dirEntry->entrySize; i++){
			LBAread(buff,1, nextKey);
			nextKey = getKeyFromBlock(buff, BLOCK_SIZE);
			char* data = getDataFromBlock(buff, BLOCK_SIZE);
			printf("%s\n",data);
			if(i != dirEntry->entrySize){
				printf("KEY: %d", nextKey);
			}
			free(data);
		}

		free(buff);
		free(rootDir);
		*/
		//END TEST
	}
