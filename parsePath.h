
typedef struct fs_Path{
    char* currentPath;
    fsDirEntry* entry;
} fs_Path;

/*
* WARNING: Use freePath when deallocating this!
* WARNING: If path is not viable (either the file does not exist or invalid path) this will return null
* @ owns: nothing, it is up to the caller to deallocate this with freePath!
* @ param path: full path of file you want to access, ie dir1/dir2/file
* @ returns fsPath: this struct contains the path to the resource, as well as the dirEntry of the resource
* use the appropriate loader to load the resource
*/
fs_Path*  parsePath(char* path);
int freePath(fs_Path* fs_path); // 1 indicates path was deallocated correctly, 0 indicates error