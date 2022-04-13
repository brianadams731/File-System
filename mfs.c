#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Directory.h"
#include "fsLow.h"
#include "mfs.h"
#include "parsePath.h"

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
        int indexOfLastSlash = strlen(currentPath);
        for(indexOfLastSlash; indexOfLastSlash > 0; indexOfLastSlash--){
            if(currentPath[indexOfLastSlash] == '/'){
                break;
            }
        }
        indexOfLastSlash++;
        strcpy(currentDirectory,&currentPath[indexOfLastSlash]);
    }else{
        strcpy(currentPath,"/");
        strcpy(currentDirectory,"");
    }
}

int fs_setcwd(char *buf){
    if(strcmp(buf,".") == 0){
    }else if(strcmp(buf,"..") == 0){
        popEntryFromCurrentPath();
    }else{
        // TODO: Add checking to make sure buf is a dir once mkdir is added
        // if not a dir return 1; Use parsePath and checkDir!
        addEntryToCurrentPath(buf);
    }

    /*fs_Path* path = parsePath(currentPath);
    if(parsePath){
        printf("Parsed %s", path->entry->filename);
    }
    freePath(path);
    */

    return 0;
}

char * fs_getcwd(char *buf, size_t size){
    return currentPath;
}

int fs_isDir(char * path){
    int isDir = 0;
    fs_Path* fileEntry = parsePath(path);
    if(fileEntry){
        isDir = fileEntry->entry->isADir;
    }
    freePath(fileEntry);
    return isDir;
}

int fs_isFile(char * path){
    int isFile = 0;
    fs_Path* fileEntry = parsePath(path);
    if(fileEntry){
        isFile = (fileEntry->entry->isADir == 0);
    }
    freePath(fileEntry);
    return isFile;
}

int fs_mkdir(const char *pathname, mode_t mode){

}
int fs_rmdir(const char *pathname){

}
// ------------
fdDir * fs_opendir(const char *name){
    //TODO: Should we use the name? or our current path, both seem identical
    fs_Path* path = parsePath(currentPath);
    fsDir* dir = loadDirFromBlock(path->entry->fileBlockLocation);
    
    fdDir* openDir = malloc(sizeof(fdDir));
    
    openDir->directoryStartLocation = dir->currentBlockLocation;
    openDir->dirEntryPosition = 0;
    memcpy(openDir->directryEntries, dir->directryEntries, sizeof(dir->directryEntries));
    strcpy(openDir->pathToDir, currentPath);

    freePath(path);
    free(dir);
    return openDir;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp){    
    dirp->directryEntries[dirp->dirEntryPosition];
    if(dirp->directryEntries[dirp->dirEntryPosition].fileBlockLocation == -1){
        return NULL;
    }

    printf("%s",dirp->directryEntries[0].filename);
    dirp->dirEntryPosition = dirp->dirEntryPosition + 1;
    return NULL;
}

int fs_closedir(fdDir *dirp){
    free(dirp);
    return 0;
}
// -------------


int fs_delete(char* filename){

}
// -------------
int fs_stat(const char *path, struct fs_stat *buf){

}
