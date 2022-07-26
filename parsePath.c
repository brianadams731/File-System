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
* File: parsePath.h
*
* Description: This file provides the parse path utility implementation which 
*  is used throughout the file system. 
**************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Directory.h"
#include "parsePath.h"

fs_Path * parsePath(char* originalPath){
    char* path = malloc(strlen(originalPath) + 1);
    if(originalPath[0] == '/'){
        strcpy(path,&originalPath[1]);
    }else{
        strcpy(path, originalPath);
    }
    fs_Path* fsPath = NULL;
    fsDir* rootDir = fetchRootDir();

    // if root
    if(strcmp(path,"") == 0){
        fsPath = malloc(sizeof(fs_Path));
        fsPath->currentPath = malloc(strlen(originalPath) + 1);
        fsPath->entry = malloc(sizeof(fsDirEntry));

        fsDirEntry* traversedDir = findDirEntry(rootDir,".");

        strcpy(fsPath->currentPath,"");
        memcpy(fsPath->entry, traversedDir, sizeof(fsDirEntry));

        free(path);
        free(rootDir);
        return fsPath;
    }

    // not root
    char* token = strtok(path, "/");
    fsDirEntry* traversedDir = findDirEntry(rootDir, token);
    while(token && traversedDir){
        token = strtok(NULL,"/");
        if(token == NULL){
            fsPath = malloc(sizeof(fs_Path));
            fsPath->currentPath = malloc(strlen(originalPath) + 1);
            fsPath->entry = malloc(sizeof(fsDirEntry));

            strcpy(fsPath->currentPath, path);
            memcpy(fsPath->entry, traversedDir, sizeof(fsDirEntry));

            break;
        }else if(traversedDir->isADir == 'T'){
            fsDir* dir = loadDirFromBlock(traversedDir->fileBlockLocation);
            traversedDir = findDirEntry(dir, token);
            free(dir);
        }else{
            break;
        }
    }
    free(rootDir);
    free(path);
    return fsPath;
}

int freePath(fs_Path* fsPath){
    if(fsPath){
        //free(fsPath->currentPath);
        //free(fsPath->entry);
        //free(fsPath);
        return 1;
    }
    return 0;
}

/*
* @ owns: nothing, you will need to deallocate the memory returned
* @ params path: full path you want the parent of ie. /dirOne/dirTwo
*/
parentPath* getParentPath(const char* path){
    parentPath* retPath = malloc(sizeof(parentPath));

    int indexOfLastSlash = strlen(path);
    int found = 0;
    for(indexOfLastSlash; indexOfLastSlash >= 0; indexOfLastSlash--){
        if(path[indexOfLastSlash] == '/'){
            found = 1;
            break;
        }
    }

    if(found == 1){
        indexOfLastSlash++;
        strcpy(retPath->name,&path[indexOfLastSlash]);
        if(indexOfLastSlash > 0){
            strncpy(retPath->path, path, indexOfLastSlash - 1);
            retPath->path[indexOfLastSlash-1] = '\0';
        }else{
            strcpy(retPath->path, "");
        }
    }else{
        strcpy(retPath->name,path);
        strcpy(retPath->path,"");
    }

    return retPath;
}


parentPath* relPath(const char* currentDir, const char* relPath){
    char constructedPath[300];
    if(strcmp(currentDir,"/") == 0){
        snprintf(constructedPath, sizeof(constructedPath),"%s%s",currentDir, relPath);
    }else{
        snprintf(constructedPath, sizeof(constructedPath),"%s/%s",currentDir, relPath);
    }
    return getParentPath(constructedPath);
}