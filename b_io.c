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
#include <math.h>

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

	char* readBlock;
	int bytesReadInBlock;
	int prevKey;
	fsDirEntry entry;
	char accessMode[3];  // -- or rwd

	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//char writeBlock[BLOCK_SIZE];
//int bytesInWriteBlock;

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
	
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
/*
 * b_io_fd b_open (char * filename, int flags)
 * - Interface to open a buffered file
 * - Our open will check for a valid parent path,
 * it will handle for a relative or absolute path
 * and parse through the file. Check if the file
 * exists in the directory if not we setup
 * the fcb.
 */
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

		// Base reset
		fcbArray[returnFd].buf = malloc(5);
		strcpy(fcbArray[returnFd].buf, "open");

		fcbArray[returnFd].index = 0;
		fcbArray[returnFd].blockCount = 0;
		fcbArray[returnFd].buflen = 200;
		fcbArray[returnFd].bytesReadInBlock = 0;
		fcbArray[returnFd].totalBytesRead = 0;
		fcbArray[returnFd].initialKey = 0;
		fcbArray[returnFd].blockCount = 0;
		fcbArray[returnFd].fileSize = 0;

		
		if(flags == (O_WRONLY | O_CREAT | O_TRUNC)){
			//printf("WRITE\n");
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
			freeData freeSpace = getFreeSpace(1);
			fcbArray[returnFd].initialKey = freeSpace.start;
			fcbArray[returnFd].prevKey = freeSpace.start;
			fcbArray[returnFd].readBlock = malloc(BLOCK_SIZE);
			if(freeSpace.end == 0){
				printf("NO FREE SPACE\n");
				return -1;
			}
		}
		if(flags == (O_RDONLY)){
			//printf("READ\n");
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
			fcbArray[returnFd].initialKey = fcbArray[returnFd].entry.fileBlockLocation; // used for debug
			fcbArray[returnFd].prevKey = fcbArray[returnFd].entry.fileBlockLocation;
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



/*
 * int b_write (b_io_fd fd, char * buffer, int count)
 * - First state (0)  - will be able to fit
 * the entire buffer into the read block.
 * - Second state (1) - will  fill the read buffer
 *  copy the write location to a key, allowing 
 * us to keep track of the filled blocks. Then reset
 * the buffer and refill the remaining overflow.
 * - Third state (2)  - will check if it can or
 * can't fill the entire buffer then we repeat the
 * steps of the second state
 */
int b_write (b_io_fd fd, char * buffer, int count)
	{
		
	if (startup == 0) b_init();  //Initialize our system
	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)){
		return (-1); 					//invalid file descriptor
	}
	if(count < 0){
		printf("ERROR: Invalid linux file\n");
		return -1;
	}
	/* Write States
	* 0 - readBuffer fit inBuffer
	* 1 - readBuffer needs to wite block, then continue to be filled by inBuffer
	* 2 - final inBufferFill
	*/
	int state;
	// can fit entire inbuffer in readbuffer
	if(count < fcbArray[fd].buflen){
		state = 2;
	}else if(BufferWithKeyOffset - fcbArray[fd].bytesReadInBlock >= count){
		state = 0; 
	}else{
		state = 1;
	}

	//printf("BYTES IN READ BLOCK %d\n",  fcbArray[fd].bytesReadInBlock);
	
	if(state == 0){
		//printf("FILL\n");
		memcpy(&fcbArray[fd].readBlock[fcbArray[fd].bytesReadInBlock], buffer, count);
		fcbArray[fd].fileSize += count;
		fcbArray[fd].bytesReadInBlock += count;
	}
	// cannot fit entire buffer into read block
	else if(state == 1){
		//printf("REFILL\n");
		// Fill read buffer
		int amountRequiredToFillReadBuffer = BufferWithKeyOffset - fcbArray[fd].bytesReadInBlock;
		memcpy(&fcbArray[fd].readBlock[fcbArray[fd].bytesReadInBlock], buffer, amountRequiredToFillReadBuffer);
		fcbArray[fd].fileSize += amountRequiredToFillReadBuffer;
		
		// Copy write location for filled block
		int writeLoc = fcbArray[fd].prevKey;

		// Mark block with next block key
		markUsedSpaceByBlock(writeLoc, 1);
		freeData freeSpace = getFreeSpace(1);
		fcbArray[fd].prevKey = freeSpace.start;
		writeKeyToBuffer(fcbArray[fd].readBlock, BLOCK_SIZE, fcbArray[fd].prevKey);

		// write logical block
		LBAwrite(fcbArray[fd].readBlock, 1, writeLoc);
		fcbArray[fd].blockCount++;

		// reset bytes read in block
		fcbArray[fd].bytesReadInBlock = 0;

		// Begin refilling readBuffer
		int amountLeftInInputBuffer = count - amountRequiredToFillReadBuffer;
		memcpy(fcbArray[fd].readBlock, &buffer[amountRequiredToFillReadBuffer], amountLeftInInputBuffer);
		fcbArray[fd].fileSize += amountLeftInInputBuffer;
		fcbArray[fd].bytesReadInBlock += amountLeftInInputBuffer;

	}
	// last write
	else{
		//printf("END\n");
		int amountRequiredToFillReadBuffer = BufferWithKeyOffset - fcbArray[fd].bytesReadInBlock;
		// can fit remaining inBuffer in readBuffer
		if(amountRequiredToFillReadBuffer >= count){
			memcpy(&fcbArray[fd].readBlock[fcbArray[fd].bytesReadInBlock], buffer, count);
			fcbArray[fd].fileSize += count;
			fcbArray[fd].bytesReadInBlock += count;

			LBAwrite(fcbArray[fd].readBlock, 1, fcbArray[fd].prevKey);
			fcbArray[fd].blockCount++;
			markUsedSpaceByBlock(fcbArray[fd].prevKey, 1);
		// cannot fit remaining inBuffer in readBuffer
		}else{
			memcpy(&fcbArray[fd].readBlock[fcbArray[fd].bytesReadInBlock], buffer, amountRequiredToFillReadBuffer);
			fcbArray[fd].fileSize += amountRequiredToFillReadBuffer;
		
			// Copy write location for filled block
			int writeLoc = fcbArray[fd].prevKey;

			// Mark block with next block key
			markUsedSpaceByBlock(writeLoc, 1);
			freeData freeSpace = getFreeSpace(1);
			fcbArray[fd].prevKey = freeSpace.start;
			writeKeyToBuffer(fcbArray[fd].readBlock, BLOCK_SIZE, fcbArray[fd].prevKey);
			
			// write logical block
			LBAwrite(fcbArray[fd].readBlock, 1, writeLoc);
			fcbArray[fd].blockCount++;

			// reset bytes read in block
			fcbArray[fd].bytesReadInBlock = 0;

			//write in final portion of buffer into readBlock
			int remainingLeftFromInBuffer = count - amountRequiredToFillReadBuffer;
			memcpy(fcbArray[fd].readBlock, &buffer[amountRequiredToFillReadBuffer], remainingLeftFromInBuffer);
			fcbArray[fd].fileSize += remainingLeftFromInBuffer;
			fcbArray[fd].bytesReadInBlock += remainingLeftFromInBuffer;

			// Write logical block
			LBAwrite(fcbArray[fd].readBlock, 1, fcbArray[fd].prevKey);
			fcbArray[fd].blockCount++;
			markUsedSpaceByBlock(fcbArray[fd].prevKey, 1);
		}
	}

	return count;
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
int b_read (b_io_fd fd, char * buffer, int count){

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
	if((fcbArray[fd].entry.fileSizeBytes - fcbArray[fd].totalBytesRead) <= fcbArray[fd].buflen){
		//printf("TOTAL FILE SIZE: %d\n",fcbArray[fd].entry.fileSizeBytes);
		//printf("TOTAL BYTES READ: %d\n",fcbArray[fd].totalBytesRead);
		readState = 2;
	}
	// (total space in block -  bytes read from blockBuffer) >=  Size of out buffer  ** Can fill out buf with current block
	else if(BufferWithKeyOffset - fcbArray[fd].bytesReadInBlock >= fcbArray[fd].buflen){
		//printf("FULL\n");
		readState = 0;
	}
	// **the block spans two logical blocks
	else{
		//printf("SPLIT\n");
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
		//printf("PREV KEY: %d\n",fcbArray[fd].prevKey);
		fcbArray[fd].prevKey = getKeyFromBlock(blockBuffer, BLOCK_SIZE);
		//printf("THIS KEY: %d\n",fcbArray[fd].prevKey);
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


	}else{
		int bytesLeftInCurrentOpenBlock = BufferWithKeyOffset - fcbArray[fd].bytesReadInBlock;
		int bytesLeftInFileToRead = fcbArray[fd].entry.fileSizeBytes - fcbArray[fd].totalBytesRead;
		
		// Last Block Rolls over into another block
		if(bytesLeftInCurrentOpenBlock < bytesLeftInFileToRead){
			countToCopy = bytesLeftInCurrentOpenBlock;
			memcpy(buffer, &blockBuffer[fcbArray[fd].bytesReadInBlock], countToCopy);
			fcbArray[fd].bytesReadInBlock = 0;
			fcbArray[fd].prevKey = getKeyFromBlock(blockBuffer, BLOCK_SIZE);
			LBAread(blockBuffer, 1, fcbArray[fd].prevKey);
			//printf("FINAL KEY %d\n", fcbArray[fd].prevKey);
			int oldCountToCopy = countToCopy;
			countToCopy = bytesLeftInFileToRead - countToCopy;
			//printf("TOTAL BYTES READ: %d\n", fcbArray[fd].totalBytesRead + oldCountToCopy + countToCopy);
			memcpy(&buffer[oldCountToCopy], blockBuffer, countToCopy);
			return oldCountToCopy + countToCopy;

		}else{
			// count to copy = total filesize - bytes already read
			countToCopy = fcbArray[fd].entry.fileSizeBytes - fcbArray[fd].totalBytesRead;
			memcpy(buffer, &blockBuffer[fcbArray[fd].bytesReadInBlock], countToCopy);
			fcbArray[fd].totalBytesRead += countToCopy;
	
			//printf("TOTAL BYTES WRITTEN %d\n",fcbArray[fd].totalBytesRead);
			//printf("TOTAL SIZE OF FILE %d\n",fcbArray[fd].entry.fileSizeBytes);
	
			// free and exit
			free(blockBuffer);
			return countToCopy;
		}
	}
}
	
// Interface to Close the file	
void b_close (b_io_fd fd)
	{
		//printf("Closed\n");
		//printf("Initial Key: %d\n", fcbArray[fd].initialKey);
		//printf("Final Key: %d\n", fcbArray[fd].prevKey);
		//printf("Total File Size: %d\n", fcbArray[fd].fileSize);
		//printf("Total Actual Block Count %d\n", fcbArray[fd].blockCount);
		//printf("Total Calc Block Count From File Size %ld\n", (long) ceil((double)fcbArray[fd].fileSize/(BLOCK_SIZE - sizeof(int))));
		//printf("End Closed\n");
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;
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

			free(fcbArray[fd].readBlock);
			free(parentDir);
			free(entryToAdd);

			//printf("FILE SIZE: %d\n",entryToAdd->fileSizeBytes);
			//printf("FILE BLOCKS: %d\n", entryToAdd->entrySize);
		}else if(fcbArray[fd].type == 'r'){

		}
	}
