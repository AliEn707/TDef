#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "bintree.h"



/*
╔══════════════════════════════════════════════════════════════╗
║ 		functions for binary tree with data 			                       ║
║ created by Dennis Yarikov						                       ║
║ sep 2014									                       ║
╚══════════════════════════════════════════════════════════════╝
*/



int bintreeAdd(bintree* root, bintree_key key,void* data){
	bintree* tmp=root;
	register int next;
//	printDebug("add key %d\n",key);
	if (data==0)
		return 0;
	while(key>0){
		next=(int)(key&1);
		if(tmp->next[next]==0){
			if((tmp->next[next]=malloc(sizeof(bintree)))==0){
				perror("malloc bintreeAdd");
				return 0;
			}
			memset(tmp->next[next],0,sizeof(bintree));
		}
		tmp=tmp->next[next];
		key>>=1;
	}
	if (tmp->data!=0)
		return 0;
	tmp->data=data;
	return 1;
}

void * bintreeGet(bintree* root, bintree_key key){
	bintree* tmp=root;
	bintree_key next;
	while(key>0){
		next=(int)(key&1);
		if(tmp->next[next]==0)
			return 0;
		tmp=tmp->next[next];
		key>>=1;
	}
	return tmp->data;
}

//get or add object size of "size"
void * bintreeFetch(bintree* root,bintree_key key,int size){
	void * a=bintreeGet(root,key);
	if (a==0){
		a=malloc(size);
		bintreeAdd(root,key,a);
	}
	return a;
}

int _bintreeDel(bintree* root, bintree_key key, void (f)(void*v)){
	int get;
	int next;
	if (root==0)
		return 0;
	if (key==0){
		if (f!=0)
			f(root->data);
//		free(root->data); //may be need
		root->data=0;
		if (root->next[0]==0 && root->next[1]==0){
			free(root);
//			printDebug("free\n");
			return 1;
		}
		return 0;
	}
	next=key&1;
	get=_bintreeDel(root->next[next],key>>1,f);
//	free(root->next[next]);
	if (get!=0){
		root->next[next]=0;
		if (root->next[(next+1)%2]==0){
//			printDebug("%d %d\n",next,(next+1)%2);
			free(root);
			return 1;
		}
	}
	return 0;
}

int bintreeDel(bintree* root, bintree_key key, void (f)(void*v)){
	int get;
	bintree_key next=key&1;
	get=_bintreeDel(root->next[next],key>>1,f);
	if (get!=0)
		root->next[next]=0;
	return get;
}

/*
int bintreeDel(bintree* root, int key){
	bintree* tmp=root;
	int next;
	while(key>1){
		next=(int)(key&1);
		if(tmp->next[next]==0)
			return 0;
		tmp=tmp->next[next];
		key>>=1;
	}
	free(tmp->next[next]);
	tmp->next[next]=0;
	return 1;
}
*/
void bintreeErase(bintree * root,void (f)(void*v)){
	if (root==0)
		return;
//	printDebug("y\n");
	bintreeErase(root->next[0],f);
	bintreeErase(root->next[1],f);
	if (f!=0)
		f(root->data);
	free(root->next[0]);
	root->next[0]=0;
	free(root->next[1]);
	root->next[1]=0;
}

/*
int main(){
	bintree r;
	memset(&r,0,sizeof(r));
	bintreeAdd(&r,3,100);
	bintreeAdd(&r,7,132);
	printDebug("%d\n",bintreeGet(&r,3));
	bintreeDel(&r,3);
//	bintreeDel(&r,3);
	printDebug("%d\n",bintreeGet(&r,3));
	printDebug("%d\n",bintreeGet(&r,7));
	bintreeErase(&r);
}
*/
