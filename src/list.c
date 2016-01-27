#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "list.h"


/*
╔══════════════════════════════════════════════════════════════╗
║ 		functions for l1 list with data 				                       ║
║ created by Dennis Yarikov						                       ║
║ sep 2014									                       ║
╚══════════════════════════════════════════════════════════════╝
*/




worklist * worklistAdd(worklist * root, int id){
	register worklist * tmp;
	for(tmp=root;tmp->next!=0;tmp=tmp->next);
	if ((tmp->next=malloc(sizeof(worklist)))==0){
		perror("malloc worklistAdd");
		return 0;
	}
	memset(tmp->next,0,sizeof(worklist));
	tmp->next->id=id;
	return tmp->next;
}

worklist * worklistDel(worklist * root, int id){
	register worklist* tmp;
	worklist * tmp1;
	for(tmp=root;tmp->next!=0;tmp=tmp->next)
		if (tmp->next->id==id)
			break;
	if (tmp->next==0)
		return 0;
	tmp1=tmp->next;
	tmp->next=tmp->next->next;
	free(tmp1);
	return tmp;
}

void worklistErase(worklist* root, void(f)(void*)){
	if (root==0)
		return;
	worklistErase(root->next,f);
	if (f)
		f(root->data);
	free(root->next);
	root->next=0;
}

//must free data manualy before return if needed
void worklistForEachRemove(worklist* root, void*(f)(worklist* w, void* arg), void* arg){
	register worklist * tmp=root;
	for(tmp=tmp->next;tmp!=0;tmp=tmp->next){
		if (f(tmp, arg)!=0)
			tmp=worklistDel(root,tmp->id);
	}
}

void* worklistForEachReturn(worklist* root, void*(f)(worklist* w, void* arg), void* arg){
	register worklist * tmp=root;
	void *o;
	for(tmp=tmp->next;tmp!=0;tmp=tmp->next){
		if ((o=f(tmp, arg))!=0)
			return o;
	}
	return o;
}

/*
int main(){
	worklist l;
	memset(&l,0,sizeof(l));
	worklistAdd(&l,5);
	worklistAdd(&l,7);
	worklistDel(&l,7);
	worklistErase(&l);
}
*/