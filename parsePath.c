#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parsePath.h"

FS_Path * parsePath(char* path){
    FS_Path* fsPath = malloc(sizeof(FS_Path));
    fsPath->currentPath = malloc(strlen(path) + 1);

    strcpy(fsPath->currentPath,path);
    
    printf("%s\n", path);
    printf("%s\n", fsPath->currentPath);
    return fsPath;
}

int freePath(FS_Path* fs_path){
    free(fs_path->currentPath);
    free(fs_path);
    return 1;
}