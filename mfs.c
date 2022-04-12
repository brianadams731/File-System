#include <stdio.h>
#include <stdlib.h>
#include "Directory.h"
#include "fsLow.h"
#include "mfs.h"
#include "parsePath.h"

int fs_setcwd(char *buf){
    fs_Path* path = parsePath(buf);
    
    if(path){
        printf("%s\n",path->entry->filename);
        printf("%d\n",path->entry->fileBlockLocation);
    }else{
        printf("Null\n");
    }
    freePath(path);
    return 0;
}