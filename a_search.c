#include "grid.h"

typedef
struct set{
	gnode * gnode;
	struct set * next;
} set;

set* setInit(){
	set* tmp;
	if ((tmp=malloc(sizeof(set)))==0) 
		perror("malloc initSet\n");
	memset(tmp,0,sizeof(set));
	return tmp;
}

set* setAdd(set* s,gnode * node){
	set* tmp=s;
	for(;s->next!=0;s=s->next);
	if((s->next=malloc(sizeof(set)))==0) 
		perror("malloc setAdd\n");
	memset(s->next,0,sizeof(set));
	s->next->gnode=node;
	return s;
}

int setSize(set* s){
	set* tmp=s;
	int i;
	for(i=0;tmp->next!=0;tmp=tmp->next)i++;;
	return i;
}

gnode* setFind(set* s,gnode * n){
	set* tmp=s->next;
	for(;tmp!=0;tmp=tmp->next)
		if (tmp->gnode->id==n->id) 
			return tmp->gnode;
	return 0;
}

gnode * setMinf(set* s){
	set* tmp=s,*out=0;
	int min;
	
	min=s->next->gnode->f;
	out=s->next;
	
	for(tmp=s->next;tmp->next!=0;tmp=tmp->next)
		if(min>=tmp->gnode->f){
			min=tmp->gnode->f;
			out=tmp;
		}
	return out->gnode;
}

void setDel(set* s,gnode*n){
	set* tmp;
	for(tmp=s;tmp->next!=0;tmp=tmp->next)
		if (tmp->next->gnode->id==n->id) break;
	set* tmp1=tmp->next;
	tmp->next=tmp->next->next;
	free(tmp1);
}

float heuristic_cost_estimate(gnode * a,gnode * b){
	int ax=a->id/config.gridsize;
	int ay=a->id%config.gridsize;
	int bx=b->id/config.gridsize;
	int by=b->id%config.gridsize;
//	printf("%f\n",sqrt(ax*bx+ay*by));
//	printf("%d %d %d %d\n\n",ax,ay,bx,by);
	return sqrt((ax-bx)*(ax-bx)+(ay-by)*(ay-by));
	
}

int cost(gnode* x,gnode* y){
	return 1;
	return -1;
	
}

int * neighbor_nodes(gnode* grid,gnode* n){
	int out=8;	
	int * z;
	if((z=malloc(sizeof(int)*out+sizeof(int)))==0)
		perror("malloc neighbor_nodes\n");
	memset(z,0,sizeof(sizeof(int)*out+sizeof(int)));
	int x=n->id/config.gridsize;
	int y=n->id%config.gridsize;
	int a[8]={(y+1<config.gridsize)?to2d(x,y+1):-1,
			(x-1>=0)?to2d(x-1,y):-1,
			(x-1>=0&&y-1>=0)?to2d(x-1,y-1):-1,
			(x-1>=0&&y+1<config.gridsize)?to2d(x-1,y+1):-1,
			(x>=0&&y-1>=0)?to2d(x,y-1):-1,
			(x+1<config.gridsize&&y-1>=0)?to2d(x+1,y-1):-1,
			(x+1<config.gridsize)?to2d(x+1,y):-1,
			(x+1<config.gridsize&&y+1<config.gridsize)?to2d(x+1,y+1):-1};
	int i,j=0;
	for(i=0;i<8;i++)
		if (a[i]>0)
			if (grid[a[i]].buildable>0)
				z[++j]=a[i];
		//printf("%d\n",out);
	*z=j;
	return z;
}

int reconstruct_path(gnode *start,gnode *goal){
//	town* t=goal;
//	if (start->name!=goal->name)
//		reconstruct_path(start,goal->came_from);
//	printf("%c ",goal->name);
	return 0;
}

int aSearch(gnode* grid,gnode* start,gnode* goal){
	set* closedset = setInit();   
	set* openset = setInit();
	setAdd(openset,start);

	start->g = 0;   
	start->h = heuristic_cost_estimate(start, goal); 
	start->f = start->g + start->h;      
 
	while (setSize(openset)>0){
		
		gnode* x =0;
		x=setMinf(openset);
		if (x->id == goal->id) 
			return reconstruct_path(start,goal); 
	 
		setDel(openset,x); 
		setAdd(closedset,x); 
		int *y=neighbor_nodes(grid,x);
		int i;
		int tentative_is_better;
		for(i=1;i<=*y;i++){
			if (setFind(closedset,grid+y[i])>0)      
				continue;
	 
			int tentative_g_score = x->g +cost(x,grid+y[i]);  
			if (setFind(openset,grid+y[i])<=0){
				tentative_is_better = 1;
				setAdd(openset,&grid[y[i]]);
			}
			else               
				if (tentative_g_score <= grid[y[i]].g)  
					tentative_is_better = 1;   
				else

					tentative_is_better = -1;
					if (tentative_is_better == 1){
						
						grid[y[i]].next = x->id; 
						grid[y[i]].g = tentative_g_score;
						grid[y[i]].h = heuristic_cost_estimate(grid+y[i], goal);
						grid[y[i]].f = grid[y[i]].g + grid[y[i]].h;
						
					}
				
		}
		free(y);
	}
	return -1;
}


/* //test
int main(){
	gnode grid[100];
	config.gridsize=10;
	memset(grid,0,sizeof(grid));
	int i;
	for(i=0;i<100;i++){
		grid[i].buildable=1;
		grid[i].id=i;
		if(rand()%100<10)
			grid[i].buildable=0;
	}
	int j;
	
	int a=aSearch(grid,grid+80,grid+3);
	for(i=0;i<10;i++){
		for(j=0;j<10;j++)
			printf("{%d}[%d]%d ",grid[to2d(i,j)].buildable,grid[to2d(i,j)].id,grid[to2d(i,j)].next);
		printf("\n");
	}
	
	printf("%d\n",a);
}
/**/