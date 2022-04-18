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
* File: Directory.c
*
* Description: This file provides the implementation for the initialization of the 
* root directory. The root directory contains it's block location, and its parent block location 
* (which is it self). The root directory also contains a magic number for identification, as well as 
* an array of directory entries. These directory entries contain metadata about the files that the root
* directory tracks. These directory entries contain the file id's, block location, size, author, date created etc.
* The first two indexes represent the parent and self. Index 0 would represent .. (the parent), and index 1 would
* represent . (self). Because this is the root index the parent and self both point the root (self). 
* 
**************************************************************/
#include "Directory.h"
#include <stdlib.h>
#include "fsLow.h"
#include <stdio.h>
#include <string.h>
#include <time.h>


fsDir* initRootDir(){
    fsDir* root = malloc(sizeof(fsDir));
    strcpy(root->name, "root");
    root->currentBlockLocation = ROOT_DIR_LOCATION;
    root->parentBlockLocation =  ROOT_DIR_LOCATION;

    int i = 0;
    for(i; i<sizeof(root->directryEntries)/sizeof(fsDirEntry);i++){
        if(i<2){
        strcpy(root->directryEntries[i].filename, i==0?".":"..");
        root->directryEntries[i].entrySize = DIR_SIZE;
        root->directryEntries[i].isADir = 'T';
        root->directryEntries[i].fileBlockLocation = root->currentBlockLocation;
        strcpy(root->directryEntries[i].permissions,"---");
        strcpy(root->directryEntries[i].author, "OS");
        root->directryEntries[i].dateCreated = getCurrentDateTime();
        // TODO: Permissions if in project scope
        // TODO: Date created
        }else{
            strcpy(root->directryEntries[i].filename, "");
            root->directryEntries[i].id = -1;
        }
    }
    return root;
}

fsDir* makeDir(const char* name, int blockLocation, fsDirEntry parentDirEntry ){
    fsDir* dir = malloc(sizeof(fsDir));
    strcpy(dir->name, name);
    dir->currentBlockLocation = blockLocation;
    dir->parentBlockLocation =  parentDirEntry.fileBlockLocation;

    int i = 0;
    for(i; i<sizeof(dir->directryEntries)/sizeof(fsDirEntry);i++){
        if(i==0){
            strcpy(dir->directryEntries[i].filename,".");
            dir->directryEntries[i].entrySize = DIR_SIZE;
            dir->directryEntries[i].isADir = 'T';
            dir->directryEntries[i].fileBlockLocation = dir->currentBlockLocation;
            strcpy(dir->directryEntries[i].permissions,"rwd");
            strcpy(dir->directryEntries[i].author, "user");
            dir->directryEntries[i].dateCreated = getCurrentDateTime();
        }else if(i==1){
            strcpy(dir->directryEntries[i].filename, "..");
            dir->directryEntries[i].entrySize = DIR_SIZE;
            dir->directryEntries[i].isADir = 'T';
            dir->directryEntries[i].fileBlockLocation = parentDirEntry.fileBlockLocation;
            strcpy(dir->directryEntries[i].permissions,"---");
            strcpy(dir->directryEntries[i].author, "user");
            dir->directryEntries[i].dateCreated = getCurrentDateTime();
        }else{
            strcpy(dir->directryEntries[i].filename, "");
            dir->directryEntries[i].id = -1;
        }
    }
    return dir;
}

void addDirEntryFromDir(fsDir* targetDir, fsDir* sourceDir, int targetIndex){
    strcpy(targetDir->directryEntries[targetIndex].filename, sourceDir->name);
    targetDir->directryEntries[targetIndex].entrySize = sourceDir->directryEntries[0].entrySize;
    targetDir->directryEntries[targetIndex].isADir = sourceDir->directryEntries[0].isADir;
    targetDir->directryEntries[targetIndex].fileBlockLocation = sourceDir->currentBlockLocation;
    strcpy(targetDir->directryEntries[targetIndex].permissions, sourceDir->directryEntries[0].permissions);
    strcpy(targetDir->directryEntries[targetIndex].author, sourceDir->directryEntries[0].author);
}

int addExistingDirEntry(fsDir* targetDir, fsDirEntry* entry){
    int position = 0;
    int found = 0;
    for(position; position<MAX_DIR_ENTRIES; position++){
        if(strcmp(targetDir->directryEntries[position].filename, "") == 0){
            found = 1;
            break;
        }
    }
    if(!found){
        return 0;
    }
    memcpy(&targetDir->directryEntries[position], entry, sizeof(fsDirEntry));
    return 1;
}

/*fsDir* findDir(const char* name){

}*/

/*
* @ owns: nothing, you need to deallocate this memory!
*/
fsDir* fetchRootDir(){
    return loadDirFromBlock(ROOT_DIR_LOCATION);
}

fsDir* loadDirFromBlock(int blockLocation){
    fsDir* dir = malloc(MINBLOCKSIZE * DIR_SIZE);
    LBAread(dir, DIR_SIZE, blockLocation);
    return dir;
}

fsDirEntry* findDirEntry(fsDir* src, char* dirEntryName){
    int i = 0;
    fsDirEntry* dirEntry = NULL;
    for(i;i< MAX_DIR_ENTRIES;i++){
        if(strcmp(src->directryEntries[i].filename, dirEntryName) == 0){
            return &src->directryEntries[i];
        }
    }
    return dirEntry;
}

int rmDirEntry(fsDir* src, char* dirname){
    int i = 0;
    int found = 0;
    fsDirEntry* dirEntry = NULL;
    /*
    printf("SOURCE BLOCK: %d\n", src->currentBlockLocation);
    printf("SOURCE NAME: %s\n", src->name);
    */
    for(i;i< MAX_DIR_ENTRIES;i++){
        if(strcmp(src->directryEntries[i].filename, dirname) == 0){
            found = 1;
            dirEntry = &src->directryEntries[i];
            break;
        }
    }
    if(found){
        // MOVES EVERY ENTRY DOWN ONE, THEN DELETES THE LAST RECORD
        if(i != MAX_DIR_ENTRIES - 1){
            i++;
            for(i;i<MAX_DIR_ENTRIES;i++){
                src->directryEntries[i-1] = src->directryEntries[i];
            } 
            i--;
        }

        strcpy(src->directryEntries[i].filename,"");
        strcpy(src->directryEntries[i].author, "");
        strcpy(src->directryEntries[i].permissions, "---");
        src->directryEntries[i].dateCreated = 0;
        src->directryEntries[i].id = -1;
        src->directryEntries[i].entrySize = 0;
        src->directryEntries[i].fileBlockLocation = 0;
        src->directryEntries[i].isADir = 'F';
        return 1;
    }
    return 0;
}

time_t getCurrentDateTime(){
    time_t timeNow;
    time(&timeNow);
    return timeNow;
}

void setRead(fsDirEntry* targetEntry, int status){
    if(status){
        targetEntry->permissions[0] = 'r';
    }else{
        targetEntry->permissions[0] = '-';
    }
}
void setWrite(fsDirEntry* targetEntry, int status){
    if(status){
        targetEntry->permissions[1] = 'w';
    }else{
        targetEntry->permissions[1] = '-';
    }
}
void setDelete(fsDirEntry* targetEntry, int status){
    if(status){
        targetEntry->permissions[2] = 'd';
    }else{
        targetEntry->permissions[2] = '-';
    }
}

int canRead(fsDirEntry* targetEntry){
    if(!targetEntry){
        return 0;
    }
    return targetEntry->permissions[0] == 'r';
}
int canWrite(fsDirEntry* targetEntry){
    if(!targetEntry){
        return 0;
    }
    return targetEntry->permissions[1] == 'w';
}
int canDelete(fsDirEntry* targetEntry){
    if(!targetEntry){
        return 0;
    }
    if(targetEntry->permissions[2] == 'd'){
        if(targetEntry->isADir == 'T'){
            fsDir* dir = loadDirFromBlock(targetEntry->fileBlockLocation);
            int count = 0;
            int i = 0;
            for(i;i<MAX_DIR_ENTRIES;i++){
                if(strcmp(dir->directryEntries[i].filename,"") != 0){
                    ++count;
                }
            }
            free(dir);
            if(count > 2){ // accounting for self, and parrent
                printf("Error: Directory cannot delete directories that are not empty");
                return 0;
            }
            return 1;
        }else{
            return 1;
        }
    }else{
        printf("Error: Invalid Permission\n");
    }
    return 0;
}

// For debug only, used to make sure dir is being initalized correctly
/*
int main(){
    fsDir* test = initRootDir();
    printf("Name: %s\n", test->name);
    printf("Location: %d\n", test->currentBlockLocation);
    printf("Parent: %d\n", test->parentBlockLocation);
    printf("DirEntry: 0 %s\n",test->directryEntries[0].author);
    printf("DirEntry: 1 %s\n",test->directryEntries[0].author);
    return 0;
}
*/