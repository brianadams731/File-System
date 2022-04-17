#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

int fs_setcwd(char *buf){
    if(strcmp(buf,".") == 0){
    }else if(strcmp(buf,"..") == 0){
        popEntryFromCurrentPath();
    }else if(buf[0] == '/'){
        fs_Path* path = parsePath(buf);
        if(!path || path->entry->isADir != 'T'){
            return 1;
        }
        strcpy(currentPath, buf);
        addCurrentDirFromPath();
    }else{
        //Checking if buf is a valid directory
        fs_Path* path = parsePath(currentPath);
        fsDir* dir = loadDirFromBlock(path->entry->fileBlockLocation);
        fsDirEntry* newDir = findDirEntry(dir, buf);
        freePath(path);
        free(dir);
        if(newDir == NULL || newDir->isADir != 'T'){
            return 1;
        }
        // If buf is valid, add to path
        addEntryToCurrentPath(buf);
    }
    return 0;
}

char * fs_getcwd(char *buf, size_t size){
    if(size >= strlen(currentPath)){
        strcpy(buf, currentPath);
    }else{
        strncpy(buf, currentPath, size - 1);
        buf[size] = '\0';
    }
    return buf;
}

int fs_isDir(char * path){
    int isDir = 0;
    char usePath[300];
    if(path[0] != '/'){

        strcpy(usePath, currentPath);
        if(strcmp(usePath, "/") != 0){
            strcpy(&usePath[strlen(currentPath)],"/");
        }
        strcpy(&usePath[strlen(usePath)],path);
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

int fs_isFile(char * path){
    int isFile = 0;
    char usePath[300];

    if(path[0] != '/'){
        strcpy(usePath, currentPath);
        if(strcmp(usePath, "/") != 0){
            strcpy(&usePath[strlen(currentPath)],"/");
        }
        strcpy(&usePath[strlen(usePath)],path);
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
    }else{
        strcpy(pathToParent, currentPath);
        strcpy(dirName, pathname);
    }
    fs_Path* path = parsePath(pathToParent);
    if(!path || path->entry->isADir != 'T'){
        printf("Error: Invalid Dir Location\n");
        return 1;
    }
    fsDir* dir = loadDirFromBlock(path->entry->fileBlockLocation);
    int freeEntryIndex = -1;
    int i = 0;
    for(i;i<MAX_DIR_ENTRIES;i++){
        if(strcmp(dir->directryEntries[i].filename, dirName) == 0){
            printf("Error: Duplicate Dir Name\n");
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
        return 1;
    }
    freeData freeBlock = getFreeSpace(DIR_SIZE);
    if(freeBlock.end == 0){
        printf("Error: Insufficent Free Space");
        return 1;
    }
    fsDir* newDir = makeDir(dirName, freeBlock.start, dir->directryEntries[0]);
    addDirEntryFromDir(dir, newDir, freeEntryIndex);
    LBAwrite(dir,DIR_SIZE, dir->currentBlockLocation);
    LBAwrite(newDir,DIR_SIZE,newDir->currentBlockLocation);
    markUsedSpace(freeBlock);
    
    free(newDir);
    return 0;
}
int fs_rmdir(const char *pathname){
    // THIS HAS ALREADY CHECKED IF PATH EXISTS AND IS A DIR
    char pathToParent[300];
    char dirName[25];
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
        strcpy(pathToParent, currentPath);
        strcpy(dirName, pathname);
    }

    fs_Path* parentDirEntry = malloc(sizeof(fs_Path));
    parentDirEntry = parsePath(pathToParent);

    fsDir* parentDir = malloc(sizeof(fsDir));
    parentDir = loadDirFromBlock(parentDirEntry->entry->fileBlockLocation);

    fsDirEntry* dirEntryToDelete = malloc(sizeof(fsDirEntry));
    dirEntryToDelete = findDirEntry(parentDir, dirName);


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
int fs_delete(char* filename){
    printf("IN FILE\n");
    // THIS HAS ALREADY CHECKED IF PATH EXISTS AND IS FILE
}
// ------------
fdDir * fs_opendir(const char *name){
    //TODO: Should we use the name? or our current path, both seem identical
    // WARNING: This is assuming that the current path points to a dir!
    fs_Path* path = parsePath(currentPath);
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

int fs_closedir(fdDir *dirp){
    free(dirp->dirInfo);
    free(dirp);
    return 0;
}
// -------------

// -------------
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
