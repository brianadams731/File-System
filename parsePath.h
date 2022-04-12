
typedef struct fs_Path{
    char* currentPath;
    fsDirEntry* entry;
} fs_Path;

fs_Path*  parsePath(char* path);
int freePath(fs_Path* fs_path); // 1 indicates path was deallocated correctly, 0 indicates error