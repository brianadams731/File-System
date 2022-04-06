/**************************************************************
* Class:  CSC-415-0# - Fall 2021
* Names: Salaar Karimzadeh,
         Brandon Cruz-Youll,
         Brian Adams,
         Jasmine Thind
* Student IDs:920765141, 902899041, 921039987, 920767229
* GitHub Name: BeeSeeWhy
* Group Name: Zombies
* Project: Basic File System
*
* File: VolumeControlBlock.c
*
* Description:
**************************************************************/

#include "VolumeControlBlock.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


VolumeControlBlock* vcbInit(int bSize,int totalB){
    VolumeControlBlock* vcb = malloc(sizeof(VolumeControlBlock));
    vcb->totalBlocks = totalB;
    vcb->blockSize = bSize;
    vcb->magicNumber = 16;
    strcpy(vcb->name, "vcb");
    
    
    return vcb;    
}
/*
int main(){
    VolumeControlBlock* test = vcbInit(1,30,30);
    printf("Blocks: %d\n", test->totalBlocks);
    return 0;
}
*/