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
* File: mfs.c
*
* Description: This is the file system interface implementation file.
*	This is the interface needed by the driver to interact with
*	your filesystem.
* 
**************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Directory.h"
#include "fsLow.h"
#include "mfs.h"
#include "parsePath.h"
#include "FreeSpace.h"

//TODO: Does these need to be longer?
char currentPath[300] = "/";
char currentDirectory[25];

void addEntryToCurrentPath(char* entry){
    if(strcmp(currentPath,"/") != 0){
        strcat(currentPath,"/");
    }
    strcat(currentPath, entry);
    strcpy(currentDirectory, entry);
}

void addCurrentDirFromPath(){
    int indexOfLastSlash = strlen(currentPath);
    for(indexOfLastSlash; indexOfLastSlash > 0; indexOfLastSlash--){
        if(currentPath[indexOfLastSlash] == '/'){
            break;
        }
    }
    indexOfLastSlash++;
    strcpy(currentDirectory,&currentPath[indexOfLastSlash]);
}

void popEntryFromCurrentPath(){
    int lengthOfPath = strlen(currentPath);
    int lengthOfDir = strlen(currentDirectory);
    
    int lengthOfNewPath = lengthOfPath - lengthOfDir;
    
    if(lengthOfNewPath > 1){
        // setting currentPath
        lengthOfNewPath--; // to account for the extra /
        strncpy(currentPath, currentPath, lengthOfNewPath);
        currentPath[lengthOfNewPath] = '\0';
        //------
        // setting currentDir
        addCurrentDirFromPath();
    }else{
        strcpy(currentPath,"/");
        strcpy(currentDirectory,"");
    }
}
/*
* int fs_setcwd(char *buf)
* - Takes a path and checks if that path is valid
* and checks the name of the directory from the token
* of the path that exists. Then we set the cwd location
* of the found directory.
*/
int fs_setcwd(char *buf){
    if(strcmp(buf,".") == 0){
    }else if(strcmp(buf,"..") == 0){
        popEntryFromCurrentPath();
    }else if(buf[0] == '/'){
        // Absolute Path
        fs_Path* path = parsePath(buf);
        if(!path || path->entry->isADir != 'T'){
            return 1;
        }
        strcpy(currentPath, buf);
        addCurrentDirFromPath();
        freePath(path);
    }else{
        // Rel Path
        //Checking if buf is a valid directory
        parentPath* constructedPath = relPath(currentPath, buf);
        fs_Path* path = parsePath(constructedPath->path);
        if(!path){
            free(constructedPath);
            free(path);
            return 1;
        }
        fsDir* dir = loadDirFromBlock(path->entry->fileBlockLocation);
        fsDirEntry* newDir = findDirEntry(dir,constructedPath->name);

        freePath(path);
        free(dir);
        if(newDir == NULL || newDir->isADir != 'T'){
            free(constructedPath);
            return 1;
        }
        // If buf is valid, add to path
        if(strcmp(constructedPath->path,"") != 0){
            strcpy(currentPath,constructedPath->path);
        }
        
        addEntryToCurrentPath(constructedPath->name);
        free(constructedPath);
    }
    return 0;
}

/*
* char * fs_getcwd(char *buf, size_t size)
* - This functiion copies an absolute pathname
* of the current working directory to an array
* using a pointer called buf.
*/
char * fs_getcwd(char *buf, size_t size){
    if(size >= strlen(currentPath)){
        strcpy(buf, currentPath);
    }else{
        strncpy(buf, currentPath, size - 1);
        buf[size] = '\0';
    }
    return buf;
}

/*
* int fs_isDir(char * path)
* - takes a given path, checks if the path is valid
* Checks to see if the path is an existing directory or not.
* Directory marked as T.
*/
int fs_isDir(char * path){
    int isDir = 0;
    char usePath[325];
    if(path[0] != '/'){ 
        /*      
        strcpy(usePath, currentPath);
        if(strcmp(usePath, "/") != 0){
            strcpy(&usePath[strlen(currentPath)],"/");
        }
        strcpy(&usePath[strlen(usePath)],path);
        */
        
        parentPath* constructedRelPath = relPath(currentPath, path);
        snprintf(usePath, sizeof(usePath),"%s/%s", constructedRelPath->path, constructedRelPath->name);
        free(constructedRelPath);
        
    }else{
        strcpy(usePath,path);
    }

    fs_Path* fileEntry = parsePath(usePath);
    if(fileEntry){
        isDir = (fileEntry->entry->isADir == 'T');
    }
    freePath(fileEntry);
    return isDir;
}
/*
* int fs_isFile(char * path)
* - Takes a given path, if the path is valid
* checks to see if it is a file in the directory.
* File is marked as F
*/
int fs_isFile(char * path){
    int isFile = 0;
    char usePath[326];

    if(path[0] != '/'){
        /*
        strcpy(usePath, currentPath);
        if(strcmp(usePath, "/") != 0){
            strcpy(&usePath[strlen(currentPath)],"/");
        }
        strcpy(&usePath[strlen(usePath)],path);
        */
        parentPath* constructedRelPath = relPath(currentPath, path);
        snprintf(usePath, sizeof(usePath),"%s/%s", constructedRelPath->path, constructedRelPath->name);
        free(constructedRelPath);
    }else{
        strcpy(usePath,path);
    }

    fs_Path* fileEntry = parsePath(usePath);
    if(fileEntry){
        isFile = (fileEntry->entry->isADir == 'F');
    }
    freePath(fileEntry);
    return isFile;
}
/*
* int fs_mkdir(const char *pathname, mode_t mode)
* - Creates a new directory that has a name
* of the last token given in the path.
* - Will update the parent directory information
* into disk.
*/
int fs_mkdir(const char *pathname, mode_t mode){
    if(strcmp(pathname," ")==0){
        printf("Error: Dir must have a name\n");
        return 1;
    }
    char pathToParent[300];
    char dirName[25];

    if(pathname[0] == '/'){
        parentPath* parentData = getParentPath(pathname);
        if(strcmp(parentData->name,"") == 0){
            printf("Error: Invalid name\n");
            return 1;
        }
        strcpy(pathToParent, parentData->path);
        strcpy(dirName, parentData->name);
        free(parentData);
    }else{
        parentPath* constructedRel = relPath(currentPath, pathname);
        strcpy(pathToParent, constructedRel->path);
        strcpy(dirName, constructedRel->name);
        free(constructedRel);
    }

    fs_Path* path = parsePath(pathToParent);
    if(!path || path->entry->isADir != 'T'){
        printf("Error: Invalid Dir Location\n");
        freePath(path);
        return 1;
    }
    fsDir* dir = loadDirFromBlock(path->entry->fileBlockLocation);
    int freeEntryIndex = -1;
    int i = 0;
    for(i;i<MAX_DIR_ENTRIES;i++){
        if(strcmp(dir->directryEntries[i].filename, dirName) == 0){
            printf("Error: Duplicate Dir Name\n");
            freePath(path);
            return 1;
        }
    }

    i=0;
    for(i;i<MAX_DIR_ENTRIES;i++){
        if(strcmp(dir->directryEntries[i].filename,"")==0){
            freeEntryIndex = i;
            break;
        }
    }
    if(freeEntryIndex == -1){
        printf("There are no free entries\n");
        freePath(path);
        return 1;
    }
    freeData freeBlock = getFreeSpace(DIR_SIZE);
    if(freeBlock.end == 0){
        printf("Error: Insufficent Free Space");
        freePath(path);
        return 1;
    }
    fsDir* newDir = makeDir(dirName, freeBlock.start, dir->directryEntries[0]);
    addDirEntryFromDir(dir, newDir, freeEntryIndex);
    LBAwrite(dir,DIR_SIZE, dir->currentBlockLocation);
    LBAwrite(newDir,DIR_SIZE,newDir->currentBlockLocation);
    markUsedSpace(freeBlock);
    
    free(newDir);
    freePath(path);
    return 0;
}

/*
* int fs_rmdir(const char *pathname)
* - Removes the directory with a given valid path
* Error checks for an invalid path or name.
*/
int fs_rmdir(const char *pathname){
    // THIS HAS ALREADY CHECKED IF PATH EXISTS AND IS A DIR
    char pathToParent[300];
    char dirName[30];
    if(strcmp(pathname,".") == 0){
        printf("Error: Invalid Path\n");
        return 1;
    }
    if(pathname[0] == '/'){
        parentPath* parentData = getParentPath(pathname);
        if(strcmp(parentData->name,"") == 0){
            printf("Error: Invalid name\n");
            free(parentData);
            return 1;
        }
        strcpy(pathToParent, parentData->path);
        strcpy(dirName, parentData->name);
        free(parentData);
    }else{
        parentPath* constructedRelPath = relPath(currentPath, pathname);
        strcpy(pathToParent, constructedRelPath->path);
        strcpy(dirName, constructedRelPath->name);
        free(constructedRelPath);

        /*
        strcpy(pathToParent, currentPath);
        strcpy(dirName, pathname);
        */
    }

    fs_Path* parentDirEntry = parsePath(pathToParent);
    fsDir* parentDir = loadDirFromBlock(parentDirEntry->entry->fileBlockLocation);
    fsDirEntry* dirEntryToDelete = findDirEntry(parentDir, dirName);


    if(!dirEntryToDelete){
        printf("ERROR: Dir entry cannot be located\n");
        freePath(parentDirEntry);
        free(parentDir);
        return 1;
    }
    if(canDelete(dirEntryToDelete) == 0){
        freePath(parentDirEntry);
        free(parentDir);
        return 1;
    }

    int deleteBlockLoc = dirEntryToDelete->fileBlockLocation;
    int deleteBlockSize = dirEntryToDelete->entrySize;

    int didDelete = rmDirEntry(parentDir, dirEntryToDelete->filename);
    if(didDelete){
        LBAwrite(parentDir, DIR_SIZE, parentDir->currentBlockLocation);
        markFreeSpace(deleteBlockLoc, deleteBlockSize);
    }else{
        printf("Error: Failed to delete");
    }

    freePath(parentDirEntry);
    free(parentDir);

    // TODO: Is 0 success?
    return didDelete?0:1;
}

/*
* int fs_delete(char* filename)
* - Takes a given path and will check if that file
* or directory exists using the parsepath functions.
* - If the path is valid we load that block and
* release the free space and set the data entries 
* to default values.
*/
int fs_delete(char* filename){
    // THIS HAS ALREADY CHECKED IF PATH EXISTS AND IS FILE
    char pathToParent[300];
    char dirName[25];

    if(filename[0] == '/'){
        parentPath* parentData = getParentPath(filename);
        if(strcmp(parentData->name,"") == 0){
            printf("Error: Invalid name\n");
            free(parentData);
            return 1;
        }
        strcpy(pathToParent, parentData->path);
        strcpy(dirName, parentData->name);
        free(parentData);
    }else{
        parentPath* constructedRelPath = relPath(currentPath, filename);
        strcpy(pathToParent, constructedRelPath->path);
        strcpy(dirName, constructedRelPath->name);
        free(constructedRelPath);
        /*
        strcpy(pathToParent, currentPath);
        strcpy(dirName, filename);
        */
    }

    fs_Path* parsedPath = parsePath(pathToParent);
    fsDir* parentDir = loadDirFromBlock(parsedPath->entry->fileBlockLocation);
    freePath(parsedPath);
    fsDirEntry* entryToRemove = findDirEntry(parentDir, dirName);
    //printf("ENTRY TO REMOVE %s\n", entryToRemove->filename);
    long checkBlock = (long) ceil(((double)entryToRemove->fileSizeBytes)/(BLOCK_SIZE - sizeof(int)));
    int totalBlock = entryToRemove->entrySize;
    //printf("REMOVE\n");
    //printf("REAL BLOCK CONSUMPTION: %ld\n", checkBlock);
    //printf("RECORDED BLOCK CONSUMPTION: %d\n", totalBlock);

    freeData freeCount = getFreeSpace(1);
    //printf("Free Block Count Before Remove: %ld\n", freeCount.freeBlockCount);
    char* bufferBlock = malloc(BLOCK_SIZE);
    int key = entryToRemove->fileBlockLocation;
    int i = 0;
    for(i;i< checkBlock;i++){
        //printf("KEY %d\n",key);
        LBAread(bufferBlock, 1, key);
        markFreeSpace(key,1);
        key = (i != (checkBlock - 1))? getKeyFromBlock(bufferBlock, BLOCK_SIZE):-1;
    }
    free(bufferBlock);

    freeCount = getFreeSpace(1);
    //printf("Free Block Count After: %ld\n", freeCount.freeBlockCount);
    
    rmDirEntry(parentDir, entryToRemove->filename);
    LBAwrite(parentDir, parentDir->directryEntries[0].entrySize, parentDir->currentBlockLocation);

    free(parentDir);
    return 0;
}

/*
* fdDir * fs_opendir(const char *name)
* - Will open a directory by giving it a directory name.
* - Will return a pointer that stores the info of the directory.
*/
fdDir * fs_opendir(const char *name){
    //TODO: Should we use the name? or our current path, both seem identical
    // WARNING: This is assuming that the current path points to a dir!
    fs_Path* path = parsePath(currentPath);
    if(!path || path->entry->isADir == 'F'){
        return NULL;
    }
    fsDir* dir = loadDirFromBlock(path->entry->fileBlockLocation);
    
    fdDir* openDir = malloc(sizeof(fdDir));
    openDir->dirInfo = malloc(sizeof(struct fs_diriteminfo));

    openDir->directoryStartLocation = dir->currentBlockLocation;
    openDir->dirEntryPosition = 0;
    memcpy(openDir->directryEntries, dir->directryEntries, sizeof(dir->directryEntries));
    strcpy(openDir->pathToDir, currentPath);


    freePath(path);
    free(dir);
    return openDir;
}
/*
* struct fs_diriteminfo *fs_readdir(fdDir *dirp)
* - Returns a pointer to a structure that stores
* the information for each directory entry
*/
struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    // THIS ASSUMES THAT ENTERIES OF "" MARK THE END OF THE ENTRY ARRAY
    if(dirp->dirEntryPosition > MAX_DIR_ENTRIES){
        return NULL;
    }
    if(strcmp(dirp->directryEntries[dirp->dirEntryPosition].filename,"")==0){
        return NULL;
    }
    strcpy(dirp->dirInfo->d_name, dirp->directryEntries[dirp->dirEntryPosition].filename);
    //TODO: Check file type char conventions
    dirp->dirInfo->fileType = dirp->directryEntries[dirp->dirEntryPosition].isADir?FT_DIRECTORY:FT_REGFILE;
    dirp->dirInfo->d_reclen = sizeof(struct fs_diriteminfo);

    dirp->dirEntryPosition = dirp->dirEntryPosition + 1;
    return dirp->dirInfo;
}
/*
* int fs_closedir(fdDir *dirp)
* - deallocates and free memory for the open directory.
* - resets the pointer to the directory information
*/
int fs_closedir(fdDir *dirp){
    free(dirp->dirInfo);
    free(dirp);
    return 0;
}
/*
* int fs_stat(const char *path, struct fs_stat *buf)
* - Takes a given path and finds the name of the 
* directory entry
* - Fills the data file of fs_stat
*/
int fs_stat(const char *path, struct fs_stat *buf){
    fs_Path* dirPath = parsePath(currentPath);
    int found = 0;
    if(!dirPath || dirPath->entry->isADir != 'T'){
        printf("ERROR: PATH NOT FOUND IN FS_STAT");
        freePath(dirPath);
        return 0;
    }
    fsDir* dir = loadDirFromBlock(dirPath->entry->fileBlockLocation);
    int i = 0;
    for(i; i<MAX_DIR_ENTRIES; i++){
        if(strcmp(dir->directryEntries[i].filename, path) == 0){
            found = 1;
            break;
        }
    }
    if(found){
        buf->st_size = dir->directryEntries[i].entrySize * BLOCK_SIZE;
        buf->st_blksize = BLOCK_SIZE;
        buf->st_blocks = dir->directryEntries[i].entrySize;
        buf->st_createtime = dir->directryEntries[i].dateCreated;
        strcpy(buf->permissions, dir->directryEntries[i].permissions);
        // TODO: add modtime
        // TODO: add accesstime
        buf->st_accesstime = getCurrentDateTime();
        buf->st_modtime = dir->directryEntries[i].dateCreated;
    }else{
        printf("NOT FOUND...Path: %s\n",path);
    }
    free(dir);
    freePath(dirPath);
    return found;
}

int fs_mv(char* srcFile, char* destDirPath){
    char pathToDestDir[300];
    char pathToSrc[300];

    if(destDirPath[0] == '/'){
        // abs path
        strcpy(pathToDestDir, destDirPath);
    }else{
        // rel path
        if(strcmp(currentPath,"/")== 0){
            strcpy(pathToDestDir, destDirPath);
        }else{
            int len = snprintf(pathToDestDir, sizeof(pathToDestDir),"%s/%s",currentPath, destDirPath);
        }
        //printf("PATH TO DEST %s\n", pathToDestDir);
    }
    
    fs_Path* pathDest = parsePath(pathToDestDir);
    if(!pathDest){
        printf("Error: Destination not found\n");
        freePath(pathDest);
        return 0;
    }
    fsDir* destDir = loadDirFromBlock(pathDest->entry->fileBlockLocation);
    freePath(pathDest);
    int i = 0;
    int spaceForMoreDir = 0;
    for(i;i<MAX_DIR_ENTRIES;i++){
        if(strcmp(destDir->directryEntries[i].filename,"") == 0){
            spaceForMoreDir = 1;
            break;
        }
    }
    if(spaceForMoreDir != 1){
        free(destDir);
        return 0;
    }

    if(srcFile[0] == '/'){
        // absolute path
        strcpy(pathToSrc, srcFile);
    }else{
        // rel path
        if(strcmp(currentPath,"/") == 0){
            strcpy(pathToSrc, srcFile);
        }else{
            sprintf(pathToSrc, "%s/%s",currentPath, srcFile);
        }
    }

    fs_Path* pathToSrcFile = parsePath(pathToSrc);

    if(!pathToSrcFile || pathToSrcFile->entry->isADir == 'T'){
        freePath(pathToSrcFile);
        free(destDir);
        return 0;
    }

    parentPath* srcParentPath = getParentPath(pathToSrc);
    fs_Path* parent = parsePath(srcParentPath->path);
    free(srcParentPath);

    i = 0;
    for(i; i < MAX_DIR_ENTRIES; i++){
        if(strcmp(destDir->directryEntries[i].filename, pathToSrcFile->entry->filename) == 0){
            printf("ERROR: File name already taken\n");
            return 0;
        }
    }

    if(!parent){
        printf("Error: Parent to src not found");
        freePath(pathToSrcFile);
        freePath(parent);
        free(destDir);
        return 0;
    }
    fsDir* parentSrcDir = loadDirFromBlock(parent->entry->fileBlockLocation);
    addExistingDirEntry(destDir, pathToSrcFile->entry);
    rmDirEntry(parentSrcDir, pathToSrcFile->entry->filename);
    LBAwrite(destDir, DIR_SIZE, destDir->currentBlockLocation);
    LBAwrite(parentSrcDir, DIR_SIZE, parentSrcDir->currentBlockLocation);
    free(destDir);
    free(parentSrcDir);
    freePath(pathToSrcFile);        
    freePath(parent);
    return 1;
}