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

#define BufferWithKeyOffset (BLOCK_SIZE - sizeof(int) - 1)
#define EXTERNALBUFFERSIZE 200


typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer

	int blockNumberOfParentDir;
	char name[32];
	int initialKey;
	int blockCount;
	int fileSize;
	char type;

	int totalBytesRead;
	int bytesReadInBlock;
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

		// checks for valid parent path, and returns adds parent path, a file name 		
    	if(filename[0] != '/'){
			// Handle for Relative Path
			char* currentPath = malloc(300);
			fs_getcwd(currentPath, 300);
        	fs_Path* parsedPath = parsePath(currentPath);
			if(!parsedPath){
				printf("Error: Path not valid (b_io)\n");
				return -1;
			}
			// TODO: Handle file names that traverse relative 
			fcbArray[returnFd].blockNumberOfParentDir = parsedPath->entry->fileBlockLocation;
			strcpy(fcbArray[returnFd].name, filename);
			
			free(currentPath);
			freePath(parsedPath);
		}else{
			// Handle for Absolute Path
			parentPath* parent = getParentPath(filename);
			fs_Path* parsedPath = parsePath(parent->path);
			if(!parsedPath){
				printf("Error: Path not valid (b_io)\n");
				return -1;
			}
			fcbArray[returnFd].blockNumberOfParentDir = parsedPath->entry->fileBlockLocation;
			strcpy(fcbArray[returnFd].name, parent->name);

			free(parent);
			freePath(parsedPath);
    	}
		if(flags == (O_WRONLY | O_CREAT | O_TRUNC)){
			printf("WRITE\n");
			// write
			// Check if file exists in directory already
			fsDir* dirToIO = loadDirFromBlock(fcbArray[returnFd].blockNumberOfParentDir);
			if(fileNameExistsInDirEntry(dirToIO, fcbArray[returnFd].name) == 1){
				printf("Error: Filename already exists (b_io)\n");
				return -1;
			}
			free(dirToIO);

			// write set fcb setup
			fcbArray[returnFd].type = 'w';
			fcbArray[returnFd].index = 0;
			fcbArray[returnFd].buflen = 200;
			freeData freeSpace = getFreeSpace(1);
			fcbArray[returnFd].initialKey = freeSpace.start;
			fcbArray[returnFd].prevKey = freeSpace.start;
			if(freeSpace.end == 0){
				printf("NO FREE SPACE\n");
				return -1;
			}
		}
		if(flags == (O_RDONLY)){
			printf("READ\n");
			// read
			// Adding dir entry of file to read;
			fsDir* dir = loadDirFromBlock(fcbArray[returnFd].blockNumberOfParentDir);
			fsDirEntry* dirEntry = findDirEntry(dir, fcbArray[returnFd].name);
			if(!dirEntry){
				printf("Error: File Does not Exist");
				return -1;
			}
			memcpy(&fcbArray[returnFd].entry, dirEntry, sizeof(fsDirEntry));
			free(dir);

			fcbArray[returnFd].type = 'r';
			fcbArray[returnFd].index = 0;
			fcbArray[returnFd].prevKey = fcbArray[returnFd].entry.fileBlockLocation;
			fcbArray[returnFd].buflen = 200;
			fcbArray[returnFd].totalBytesRead = 0;
			fcbArray[returnFd].bytesReadInBlock = 0;
		}
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


	int bytesLeftToFillBuffer = BufferWithKeyOffset - bytesInWriteBlock;

	if(count < bytesLeftToFillBuffer){
		//printf("FILLING BUFFER\n");
		memcpy(&writeBlock[bytesInWriteBlock], buffer, count);
		fcbArray[fd].fileSize = fcbArray[fd].fileSize + count;
		bytesInWriteBlock = bytesInWriteBlock + count;
		if(count < fcbArray[fd].buflen){ // last write
			//printf("\n\n\nLAST BUFFER\n\n\n");
			//printf("\nLast Count In Block: %d\n", bytesInWriteBlock);
			//printf("\n%s\n",writeBlock);
			int i = count;
			for(i; i<=BufferWithKeyOffset; i++){
				writeBlock[i] = '0';
			}

			LBAwrite(writeBlock,1, fcbArray[fd].prevKey);
			fcbArray[fd].blockCount++;
			fcbArray[fd].fileSize = fcbArray[fd].fileSize + count;
			markUsedSpaceByBlock(fcbArray[fd].prevKey, 1);
		}
	}else{
		memcpy(&writeBlock[bytesInWriteBlock], buffer, bytesLeftToFillBuffer);
		fcbArray[fd].fileSize = fcbArray[fd].fileSize + bytesLeftToFillBuffer;
		markUsedSpaceByBlock(fcbArray[fd].prevKey, 1);
		freeData freeSpace = getFreeSpace(1);
		int key = freeSpace.start;
		//printf("Bytes in Buff: %d, Bytes Left To Fill Buff: %d\n", bytesInWriteBlock, bytesLeftToFillBuffer);
		//printf("Count In Block: %d", bytesInWriteBlock + bytesLeftToFillBuffer);
		//printf("\n%s\n",writeBlock);

		writeKeyToBuffer(writeBlock, BLOCK_SIZE, key);

		LBAwrite(writeBlock,1, fcbArray[fd].prevKey);
		fcbArray[fd].blockCount++;
		
		fcbArray[fd].prevKey = key; // need to write after LBAwrite

		int startOfNextBlock = count - bytesLeftToFillBuffer;
		memcpy(&writeBlock[0], &buffer[bytesLeftToFillBuffer], startOfNextBlock);
		fcbArray[fd].fileSize = fcbArray[fd].fileSize + startOfNextBlock;
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
			for(i; i<=BufferWithKeyOffset; i++){
				writeBlock[i] = '0';
			}

			//printf("\n%s\n",writeBlock);
			LBAwrite(writeBlock, 1, fcbArray[fd].prevKey);
			markUsedSpaceByBlock(fcbArray[fd].prevKey, 1);
			fcbArray[fd].fileSize = fcbArray[fd].fileSize + startOfNextBlock;
			fcbArray[fd].blockCount++;
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
	// -------------------------------------------------------------


	/* READ STATES
	* 0 - buffer can be filled by current block
	* 1 - buffer cannot be filled by current block
	* 2 - last block to be written
	*/
	int readState;
	
		// Blocks read         >=    total entry blocks     **Last Block
	if(fcbArray[fd].blockCount >= fcbArray[fd].entry.entrySize - 1){
		readState = 2;
	}
	// (total space in block -  bytes read from blockBuffer) >=  Size of out buffer  ** Can fill out buf with current block
	else if(BufferWithKeyOffset - fcbArray[fd].bytesReadInBlock >= fcbArray[fd].buflen){
		readState = 0;
	}
	// **the block spans two logical blocks
	else{
		readState = 1;
	}
	// -------------------------------------------------------------------
	char* blockBuffer = malloc(BLOCK_SIZE);
	LBAread(blockBuffer, 1, fcbArray[fd].prevKey);

	int countToCopy;
	if(readState == 0){
		// Copy part of blockBuffer into full outbuffer
		countToCopy = fcbArray[fd].buflen;
		memcpy(buffer, &blockBuffer[fcbArray[fd].bytesReadInBlock], countToCopy);
		fcbArray[fd].bytesReadInBlock += countToCopy;
		// increment total bytes read
		fcbArray[fd].totalBytesRead += countToCopy;

		// free and exit
		free(blockBuffer);
		return countToCopy;


	}else if(readState == 1){
		// Copy the remaining logical block, get block key, increment total bytes read with remaining block
		countToCopy = BufferWithKeyOffset - fcbArray[fd].bytesReadInBlock;
		fcbArray[fd].prevKey = getKeyFromBlock(blockBuffer, BLOCK_SIZE);
		memcpy(buffer, &blockBuffer[fcbArray[fd].bytesReadInBlock], countToCopy);
		// increment total bytes read
		fcbArray[fd].totalBytesRead += countToCopy;

		// Increment the block count, and reset bytes read in block count
		fcbArray[fd].blockCount++;
		fcbArray[fd].bytesReadInBlock = 0;

		// Read in the next block, set count to copy based off remaining space in outbuf - count already copied
		// write this to out buffer
		LBAread(blockBuffer, 1, fcbArray[fd].prevKey);
		int oldCountToCopy = countToCopy;
		countToCopy = fcbArray[fd].buflen - countToCopy;
		memcpy(&buffer[oldCountToCopy], &blockBuffer[fcbArray[fd].bytesReadInBlock], countToCopy);
		// increment total bytes read;
		fcbArray[fd].totalBytesRead += countToCopy;

		// increment bytes read in writeBlock by bytes copied to out buffer 
		fcbArray[fd].bytesReadInBlock += countToCopy;

		// free and exit
		free(blockBuffer);
		return oldCountToCopy + countToCopy;


	}else if(readState == 2){
		printf("LAST BLOCK\n");
		printf("ICI\n");
		printf("TOTAL FILE SIZE: %d, TOTAL BYTES READ: %d\n",fcbArray[fd].entry.fileSizeBytes, fcbArray[fd].totalBytesRead);
		printf("COUNT TO CPY: %d, COUNT OF BYTES IN BUFFER %d\n", countToCopy, fcbArray[fd].bytesReadInBlock);
		// count to copy = total filesize - bytes already read
		countToCopy = fcbArray[fd].entry.fileSizeBytes - fcbArray[fd].totalBytesRead;
		memcpy(buffer, &blockBuffer[fcbArray[fd].bytesReadInBlock], countToCopy);
		fcbArray[fd].totalBytesRead += countToCopy;

		printf("TOTAL BYTES WRITTEN %d\n",fcbArray[fd].totalBytesRead);
		printf("TOTAL SIZE OF FILE %d\n",fcbArray[fd].entry.fileSizeBytes);

		// free and exit
		free(blockBuffer);
		return countToCopy;
	}

	// total size on fs 4212
	// over run on read 78
	// linux size 4134
	// bug on write size!!!
	/* // Prefactor
	free(blockBuffer);
	return countToCpy;

	int countToCpy = BufferWithKeyOffset - fcbArray[fd].bytesReadInBlock >= fcbArray[fd].buflen ? fcbArray[fd].buflen : BufferWithKeyOffset - fcbArray[fd].bytesReadInBlock;
	
	if(fcbArray[fd].blockCount >= fcbArray[fd].entry.entrySize - 1){
			// end of block read!
			printf("LAST BLOCK\n");
			printf("ICI\n");
			printf("TOTAL FILE SIZE: %d, TOTAL BYTES READ: %d\n",fcbArray[fd].entry.fileSizeBytes, fcbArray[fd].totalBytesRead);
			printf("COUNT TO CPY: %d, COUNT OF BYTES IN BUFFER %d", countToCpy, fcbArray[fd].bytesReadInBlock);
			memcpy(buffer, &blockBuffer[fcbArray[fd].bytesReadInBlock], fcbArray[fd].entry.fileSizeBytes - fcbArray[fd].totalBytesRead);
			fcbArray[fd].totalBytesRead += countToCpy;
			printf("TOTAL BYTES WRITTEN %d\n",fcbArray[fd].totalBytesRead);
			printf("TOTAL SIZE OF FILE %d\n",fcbArray[fd].entry.fileSizeBytes);
			free(blockBuffer);
			return fcbArray[fd].entry.fileSizeBytes, fcbArray[fd].totalBytesRead;
	}

	if(countToCpy < fcbArray[fd].buflen){ // reached end of block;
		fcbArray[fd].blockCount++;
		fcbArray[fd].prevKey = getKeyFromBlock(blockBuffer, BLOCK_SIZE);
		memcpy(buffer, &blockBuffer[fcbArray[fd].bytesReadInBlock], countToCpy);
		printf("PRE COUNT CPY: %d, PRE BYTES IN BUFF: %d\n", countToCpy, fcbArray[fd].bytesReadInBlock);

		fcbArray[fd].bytesReadInBlock = 0;
		fcbArray[fd].totalBytesRead += countToCpy;

		LBAread(blockBuffer, 1, fcbArray[fd].prevKey);
		int oldCountToCpy = countToCpy;
		countToCpy = fcbArray[fd].buflen - countToCpy;
		memcpy(&buffer[oldCountToCpy], &blockBuffer[fcbArray[fd].bytesReadInBlock], countToCpy);
		fcbArray[fd].bytesReadInBlock += countToCpy;
		fcbArray[fd].totalBytesRead += countToCpy;

		printf("POST COUNT CPY: %d, POST BYTES IN BUFF: %d\n\n", countToCpy, fcbArray[fd].bytesReadInBlock);
		return oldCountToCpy + countToCpy;
	}

	memcpy(buffer, &blockBuffer[fcbArray[fd].bytesReadInBlock], countToCpy);
	fcbArray[fd].bytesReadInBlock += countToCpy;
	fcbArray[fd].totalBytesRead += countToCpy;

	free(blockBuffer);
	return countToCpy;
	*/
	}
	
// Interface to Close the file	
void b_close (b_io_fd fd)
	{
		if(fcbArray[fd].type == 'w'){
			fsDir* parentDir = loadDirFromBlock(fcbArray[fd].blockNumberOfParentDir);
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

			printf("FILE SIZE: %d\n",entryToAdd->fileSizeBytes);
			printf("FILE BLOCKS: %d\n", entryToAdd->entrySize);
			// should be 4134
		}else if(fcbArray[fd].type == 'r'){

		}
	}
