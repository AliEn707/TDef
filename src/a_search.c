#include "grid.h"
#include "engine.h"
#include "gridmath.h"

typedef
struct set{
	gnode * gnode;
	struct set * next;
} set;

set* setInit(){
	set* tmp;
	if ((tmp=malloc(sizeof(set)))==0) 
		perror("malloc initSet");
	memset(tmp,0,sizeof(set));
	return tmp;
}

set* setAdd(set* s,gnode * node){
	for(;s->next!=0;s=s->next);
	if((s->next=malloc(sizeof(set)))==0) 
		perror("malloc setAdd");
	memset(s->next,0,sizeof(set));
	s->next->gnode=node;
	return s;
}

int setSize(set* s){
	set* tmp=s;
	int i;
	for(i=0;tmp->next!=0;tmp=tmp->next)i++;
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

void setRealize(set* s){
	set*t_p=s;
	set *t_c=t_p->next;
	if (t_c==0)
		free(t_p);
	for (;t_c!=0;t_c=t_c->next){
		free(t_p);
		t_p=t_c;
	}
}


float heuristic_cost_estimate(gnode * a,gnode * b){
#define  ax (a->id/config.gridsize)
#define  ay (a->id%config.gridsize)
#define  bx (b->id/config.gridsize)
#define  by (b->id%config.gridsize)
//	return ((ax-bx)*(ax-bx)+(ay-by)*(ay-by));
	return gsqrt(sqr(ax-bx)+sqr(ay-by));
#undef ax	
#undef ay	
#undef bx	
#undef by	
}

int cost(gnode * grid,gnode* a,gnode* b){
	if (b->tower!=0)
		return 50;
	int x1=idtox(a->id),
		y1=idtoy(a->id);
	int x2=idtox(b->id),
		y2=idtoy(b->id);
	if (abs(x1-x2)==1 && abs(y1-y2)==1)
		if(grid[to2d(x1,y2)].tower!=0 || grid[to2d(x2,y1)].tower!=0 ||
				grid[to2d(x1,y2)].walkable<=0 || grid[to2d(x2,y1)].walkable<=0){
			return 10;
		}
	
	return 1;
	return -1;
	
}

int * neighbor_nodes(gnode* grid,gnode* n){
	int out=8;	
	int * z;
	if((z=malloc(sizeof(int)*out+sizeof(int)))==0)
		perror("malloc neighbor_nodes");
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
			if (grid[a[i]].walkable>0)
				z[++j]=a[i];
	*z=j;
	return z;
}

int reconstruct_path(gnode *grid,gnode *goal,path* p){
	gnode* tmp=goal;
	int i;
	if (p!=0){
		for(i=0;i<NPC_PATH;i++){
			p[i].node=tmp->next;
			tmp=&grid[p[i].node];
			if (grid[p[i].node].tower!=0)
				p[i].tower=1;
		}
		char * t_t;
		int grid2=config.gridsize*config.gridsize;
		if ((t_t=malloc(sizeof(*t_t)*grid2))==0){
			perror("malloc t_t path");
			return -1;
		}
		memset(t_t,0,sizeof(*t_t)*grid2);
		//rem loops
		for(i=0;i<NPC_PATH && p[i].node>=0;i++){
			if (p[i].node>-1){
				if (t_t[p[i].node]!=0){
					int j;
					for(j=i;p[j].node!=p[i].node;j--)
						p[j].node=-1;
					p[i].node=-1;
//					p[i+1].node=-1;
//					break;
				}else
					t_t[p[i].node]=1;
			}
		}
		free(t_t);
	}
	return 0;
}

int aSearch(gnode* grid,gnode* start,gnode* goal, path* path){
	set* closedset;   
	set* openset;
	
	if (start->id<0 || goal->id<0){
		printDebug("not correct nodes for path\n");
		path[0].node=-1;
		return -1;
	}
	if (start->id== goal->id){
		path[0].node=-1;
		return -1;
	}
	
	openset = setInit();
	closedset = setInit(); 
	setAdd(openset,start);

	start->g = 0;   
	start->h = heuristic_cost_estimate(start, goal); 
	start->f = start->g + start->h;      
 
	while (setSize(openset)>0){
		
		gnode* x =0;
		x=setMinf(openset);
		if (x->id == goal->id) {
		//	start->next=stat->id;
			setRealize(openset);
			setRealize(closedset);
			return reconstruct_path(grid,goal,path); 
		}
		setDel(openset,x); 
		setAdd(closedset,x); 
		int *y=neighbor_nodes(grid,x);
		int i;
		int tentative_is_better;
		for(i=1;i<=*y;i++){
			if (setFind(closedset,grid+y[i])>0)      
				continue;
	 
			int tentative_g_score = x->g +cost(grid,x,grid+y[i]);  
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
	setRealize(openset);
	setRealize(closedset);
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
			printDebug("{%d}[%d]%d ",grid[to2d(i,j)].buildable,grid[to2d(i,j)].id,grid[to2d(i,j)].next);
		printDebug("\n");
	}
	
	printDebug("%d\n",a);
}
*/
