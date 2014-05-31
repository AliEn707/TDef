#include "grid.h"
#include "gridmath.h"
#include "engine.h"
#include "file.h"
//Test main file

void pinfo(){
	int i=0,
		j=0,
		k=0;
	printf("Towers\t\t\tNpcs\t\t\tBullets\n");
	while(i<config.tower_max||
		j<config.npc_max||
		k<config.bullet_max){
		for(;config.tower_array[i].id<=0 && i<config.tower_max;i++);
		if (i<config.tower_max){
			printf("%d(%d)%d ",config.tower_array[i].id,
					config.tower_array[i].position,
					config.tower_array[i].type!=BASE?
						config.tower_array[i].health:
						config.players[config.tower_array[i].owner].base_health
					);
			i++;
		}
		printf("|\t\t\t");
		for(;config.npc_array[j].id<=0 && j<config.npc_max;j++);
		if (j<config.npc_max){
			printf("%d(%g,%g)%d %d",config.npc_array[j].id,
					config.npc_array[j].position.x,
					config.npc_array[j].position.y,
					config.npc_array[j].health,
					config.npc_array[j].status
					);
			j++;
		}
		printf("|\t\t\t");
		for(;config.bullet_array[k].id<=0 && k<config.bullet_max;k++);
		if (k<config.bullet_max){
			printf("%d(%g,%g) ",config.bullet_array[k].id,
					config.bullet_array[k].position.x,
					config.bullet_array[k].position.y
					);
			k++;
		}
		printf("\n");
	}
	
		
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
						grid[to2d(i,j)].npcs[0]==0?
							grid[to2d(i,j)].walkable<1?
								'X':
							'O':
						'N');
		printf("\n");
	}		
}


int main(){
	
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
	timePassed(0);
	
	npc* n=spawnNpc(grid,4,0,1);
	npc* n2=spawnNpc(grid,5,0,2);
	spawnNpc(grid,6,0,3);
	setupPlayer(0,1,2000);
	setupPlayer(1,0,1800);
	spawnTower(grid,75,0,1);
	spawnTower(grid,22,1,2);
	
	npc* n3=spawnNpc(grid,42,0,2);
	
	
	
	printf("%d\n",timePassed(0));
	printf("%p %p\n",grid[3].npcs[0],grid[3].npcs[0]);
	
	
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
	timePassed(0);
	
	drawGrid(grid);
	
	forEachNpc(grid,tickMiscNpc);
	forEachNpc(grid,tickDiedCheckNpc);
		
	forEachNpc(grid,tickCleanNpc);
	forEachBullet(grid,tickCleanBullet);
		
	forEachNpc(grid,tickTargetNpc);
	forEachNpc(grid,tickAttackNpc);
	forEachNpc(grid,tickMoveNpc);
	forEachBullet(grid,tickProcessBullet);
	
	int z;
	z=timePassed(1);
	printf("time %d",z);
	
	pinfo();
	usleep(500000);
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