#include <stdio.h>
#include "mfs.h"
#include "parsePath.h"

int fs_setcwd(char *buf){
    FS_Path* path = parsePath(buf);
    freePath(path);
    return 0;
}