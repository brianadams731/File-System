#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Directory.h"
#include "parsePath.h"

fs_Path * parsePath(char* originalPath){
    char* path = malloc(sizeof(path));
    if(originalPath[0] == '/'){
        strcpy(path,&originalPath[1]);
    }else{
        strcpy(path, originalPath);
    }

    printf("In Path: %s\n", path);
    fs_Path* fsPath = NULL;
    fsDir* rootDir = fetchRootDir();

    char* token = strtok(path, "/");
    fsDirEntry* traversedDir = findDirEntry(rootDir, token);
    while(token && traversedDir){
        token = strtok(NULL,"/");
        if(token == NULL){
            fsPath = malloc(sizeof(fs_Path));
            fsPath->currentPath = malloc(sizeof(path));
            strcpy(fsPath->currentPath, path);
            fsPath->entry = traversedDir;
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
    return fsPath;
}

int freePath(fs_Path* fsPath){
    if(fsPath){
        free(fsPath->currentPath);
        free(fsPath);
        return 1;
    }
    return 0;
}