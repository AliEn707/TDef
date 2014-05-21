#include "grid.h"
#include "gridmath.h"
#include "engine.h"
#include "file.h"
//Test main file

int main(){
	
//	gnode grid[100];
	gnode* grid;
//	gridsize=10;
//	memset(grid,0,sizeof(grid));
	
	initGridMath();
	loadConfig("test.cfg");
	grid=loadMap("test.mp");
	loadTypes("types.cfg");
	initArrays();
	npc* n=spawnNpc(grid,3,0,1);
	npc* n2=spawnNpc(grid,3,0,2);
	spawnTower(grid,25,1,1);
	printf("%p %p\n",grid[3].enpcs,grid[3].fnpcs);
	
	n->status=IN_MOVE;
	n2->status=IN_MOVE;
	
	{vec a={1,2},b={2,3},c;
		getDir(&n->position,&n->destination,&c);
	}
	
	int i;
/*	npc_type* t=config.npc_types;
	for(i=1;i<3;i++){
		printf("%d %d %d  %d %d %f %d\n",
						t[i].health,
						t[i].damage,
						t[i].shield,
						t[i].distanse,
						t[i].attack_speed,
						t[i].move_speed,
						t[i].cost
				);
				printf(" %f %f %f %d %d\n",
						t[i].effects.speed,
						t[i].effects.shield,
						t[i].effects.damage,
						t[i].effects.time,
						t[i].effects.status
				);
	}
*/	
/*	for(i=0;i<100;i++){
		grid[i].buildable=1;
		grid[i].id=i;
		if(rand()%100<10)
			grid[i].buildable=0;
	}
*/	int j;
	
	int a=aSearch(grid,grid+80,grid+3,0);
	
	while(1){
	
	for(i=0;i<10;i++){
		for(j=0;j<10;j++)
//			printf("{%d}[%d]%d ",grid[to2d(i,j)].buildable,grid[to2d(i,j)].id,grid[to2d(i,j)].next);
			printf("%c ",grid[to2d(i,j)].tower!=0?'T':grid[to2d(i,j)].enpcs==0?'O':'X');
		printf("\n");
	}
	tickMoveNpc(grid,n);
	tickMoveNpc(grid,n2);
	sleep(1);
	printf("\n");
	}
//	vec v={1.5,1.3};
//	printf("%d\n",getGridId(v));
	printf("%d\n",a);


	realizeMap(grid);
	realizeTypes();
	realizeArrays();
	return 0;
}	