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
* Description:
* This file implements the functions for Directory
**************************************************************/
#include "Directory.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// TODO: Add correct root location
#define rootLocation 1;

fsDir* initRootDir(){
    fsDir* root = malloc(sizeof(fsDir));
    strcpy(root->name, "root");
    root->currentBlockLocation = rootLocation;
    root->parentBlockLocation =  rootLocation;

    int i = 0;
    for(i; i<sizeof(root->directryEntries)/sizeof(fsDirEntry);i++){
        if(i<2){
        strcpy(root->directryEntries[i].filename, root->name);
        root->directryEntries[i].entrySize = 1;
        root->directryEntries[i].isADir = 'T';
        root->directryEntries[i].fileBlockLocation = root->currentBlockLocation;
        strcpy(root->directryEntries[i].author, "OS");
        // TODO: Permissions if in project scope
        // TODO: Date created
        }else{
            strcpy(root->directryEntries[i].filename, "");
            root->directryEntries[i].id = -1;
        }
    }

    return root;
}

fsDir* findDir(const char* name){

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