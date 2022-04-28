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
* Description: This file provides the parse path utility used throughout the
*  file system. 
**************************************************************/
typedef struct fs_Path{
    char* currentPath;
    fsDirEntry* entry;
} fs_Path;

typedef struct parentPath{
    char path[300];
    char name[25];
} parentPath;

/*
* @ owns: nothing, it is up to the caller to deallocate this with freePath!
* @ param path: full path of file you want to access, ie dir1/dir2/file
*/
parentPath* getParentPath(const char* path);

/*
* WARNING: Use freePath when deallocating this!
* WARNING: If path is not viable (either the file does not exist or invalid path) this will return null
* @ owns: nothing, it is up to the caller to deallocate this!
* @ returns fsPath: this struct contains the path to the resource, as well as the dirEntry of the resource
* use the appropriate loader to load the resource
*/
fs_Path*  parsePath(char* path);


/*
*   @ returns: 1 indicates path was deallocated correctly, 0 indicates error
*/
int freePath(fs_Path* fs_path);

parentPath* relPath(const char* currentDir,const char* relPath);
