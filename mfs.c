#include <stdio.h>
#include <stdlib.h>
#include "Directory.h"
#include "fsLow.h"
#include "mfs.h"
#include "parsePath.h"

int fs_setcwd(char *buf){
    FS_Path* path = parsePath(buf);

    fsDir* rootDir = fetchRootDir();
    fsDir* testGood = findDirFrom(rootDir, "root Entry");
    fsDir* testBad = findDirFrom(rootDir, "not here");
    
    printf("%s\n",rootDir->name);
    printf("%s\n",testGood->name);
    if(!testBad){
        printf("NULL\n");
    }
    
    free(rootDir);
    freePath(path);
    return 0;
}