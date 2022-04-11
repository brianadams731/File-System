
typedef struct FS_Path{
    int exists;
    char* currentPath;
    int blockLocation;
} FS_Path;

FS_Path*  parsePath(char* path);
int freePath(FS_Path* fs_path); // 1 indicates path was deallocated correctly, 0 indicates error