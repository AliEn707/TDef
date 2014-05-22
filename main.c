#include "grid.h"
#include "gridmath.h"
#include "engine.h"
#include "file.h"
//Test main file

void pinfo(npc* n){
	int i;
	printf("%g %g -> %g %g\n",n->position.x,n->position.y,n->destination.x,n->destination.y);
	for(i=0;i<NPC_PATH;i++)
		printf("%d ",n->path[i]);
	printf("\n");
		
}

void drawGrid(gnode* grid){
	int i,j;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++)
//			printf("{%d}[%d]%d ",grid[to2d(i,j)].buildable,grid[to2d(i,j)].id,grid[to2d(i,j)].next);
			printf("%c ",
					grid[to2d(i,j)].tower!=0?
						grid[to2d(i,j)].tower->type==1?
							'B':
						'T':
						grid[to2d(i,j)].enpcs==0?
							grid[to2d(i,j)].walkable<1?
								'X':
							'O':
						'N');
		printf("\n");
	}		
}


int main(){
	timePassed();
//	gnode grid[100];
	gnode* grid;
//	gridsize=10;
//	memset(grid,0,sizeof(grid));
	
	initGridMath();
	loadConfig("test.cfg");
	grid=loadMap("test.mp");
	loadTypes("types.cfg");
	
	config.player_max=4;
	initArrays();
	initAreaArray();
	
	npc* n=spawnNpc(grid,3,0,1);
	npc* n2=spawnNpc(grid,3,0,2);
	spawnNpc(grid,5,0,3);
	setupPlayer(0,1,0);
	setupPlayer(1,0,0);
	spawnTower(grid,75,0,1);
	printf("%d\n",timePassed());
	printf("%p %p\n",grid[3].enpcs,grid[3].fnpcs);
	
	
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
	//for (i=0;i<NPC_PATH;i++)
	while(1){
	printf("%d\n",timePassed());
	drawGrid(grid);
	
	forEachNpc(grid,tickTargetNpc);
	forEachNpc(grid,tickMoveNpc);
	
	
	sleep(1);
	printf("\n");
	}
//	pinfo(n);
//	pinfo(n2);
	
	
//	vec v={1.5,1.3};
//	printf("%d\n",getGridId(v));
	printf("%d\n",a);

	realizeAreaArray();
	realizeMap(grid);
	realizeTypes();
	realizeArrays();
	return 0;
}	