#include "bintree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/*
╔══════════════════════════════════════════════════════════════╗
║ 										                       ║
║ created by Dennis Yarikov						                       ║
║ dec 2014									                       ║
╚══════════════════════════════════════════════════════════════╝
*/


//npc
static bintree npc_types_bintree;
	
int typesNpcAdd(int _key, void* value){
	return bintreeAdd(&npc_types_bintree,_key,value);
}

void * typesNpcGet(int _key){
	return bintreeGet(&npc_types_bintree,_key);
}


void typesNpcClear(){
	bintreeErase(&npc_types_bintree,free);
}




//tower
	
static bintree tower_types_bintree;
	
int typesTowerAdd(int _key, void* value){
	return bintreeAdd(&tower_types_bintree,_key,value);
}

void * typesTowerGet(int _key){
	return bintreeGet(&tower_types_bintree,_key);
}


void typesTowerClear(){
	bintreeErase(&tower_types_bintree,free);
}




//Bullet
static bintree bullet_types_bintree;
	
int typesBulletAdd(int _key, void* value){
	return bintreeAdd(&bullet_types_bintree,_key,value);
}

void * typesBulletGet(int _key){
	return bintreeGet(&bullet_types_bintree,_key);
}


void typesBulletClear(){
	bintreeErase(&bullet_types_bintree,free);
}

