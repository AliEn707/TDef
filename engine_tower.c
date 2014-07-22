#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "gridmath.h"


tower* newTower(){
	int i;
	for(i=0;i<config.tower_max;i++)
		if (config.tower_array[i].id==0){
			config.tower_array[i].id=getGlobalId();
			return &config.tower_array[i];
		}	
	return 0;
}



tower* diedCheckTower(tower* n){
	if (n==0)
		return 0;
	if (n->type!=BASE)
		if (n->health<=0)
			return 0;
	return n;
}


void setTowerBase(tower* t){
	{
		t->health=config.tower_types[t->type].health;
		t->energy=config.tower_types[t->type].energy;
	}
}


tower* spawnTower(gnode * grid,int node_id,int owner,int type){
	tower* t;
	gnode* node=&grid[node_id];
	if((t=newTower())==0)
		perror("newTower spawnTower");
	t->owner=owner;
	t->position=node_id;
	t->type=type;
	t->bit_mask=0;
	setTowerBase(t);
	node->tower=t;
	return t;
}


int removeTower(gnode * grid,tower* t){
	gnode * node=&grid[t->position];
	node->tower=0;
	t->health=-1;
	return 0;
}

void tickMiscTower(gnode* grid,tower* t){
	if (t->attack_count<config.tower_types[t->type].attack_speed)
		t->attack_count++;
	t->bit_mask=0;
}

void tickDiedCheckTower(gnode* grid,tower* t){
	t->target=diedCheckNpc(t->target);
	/**/
}



void tickAttackTower(gnode* grid,tower* t){
	if (t->type==BASE)
		return;
	if (t->target==0){
		//find near npc
		int x=idtox(t->position);
		int y=idtoy(t->position);
		int i,j,k;
		int yid,xid;
		npc* tmp;
		for (i=0;i<config.tower_types[t->type].distanse;i++)
			for(j=0;j<config.area_size[i];j++)
				if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
						((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize))
					for (k=0;k<MAX_PLAYERS;k++)
						if (k!=config.players[t->owner].isfriend)
							for(tmp=grid[to2d(xid,yid)].npcs[k];
									tmp!=0;tmp=tmp->next)
								if (canSee(grid,&(vec){x+0.5,y+0.5},&tmp->position)>0){
									t->target=tmp;
									if (rand()%100<60)
										return;
								}
	}else{
		printf("\t|%d %d\n",t->attack_count,config.tower_types[t->type].attack_speed);
		if (t->attack_count>=config.tower_types[t->type].attack_speed){
				t->attack_count=0;
				bullet* b;//set params of bullet
				if ((b=newBullet())==0){
					perror("newBullet tickAttackBullet");
					return;
				}
				vec position={getGridx(t->position),getGridy(t->position)};
				memcpy(&b->position,&position,sizeof(vec));
				memcpy(&b->source,&position,sizeof(vec));
				b->destination.x=t->target->position.x;
				b->destination.y=t->target->position.y;
				b->type=config.tower_types[t->type].bullet_type;
				b->damage=config.tower_types[t->type].damage;
				b->support=config.tower_types[t->type].support;
				b->isfriend=config.players[t->owner].isfriend;
				b->owner=t->id;
//				b->target=NPC;
				getDir(&b->position,&b->destination,&b->direction);
//				memcpy(&b->effects,&config.npc_types[n->type].effects,sizeof(effects));
			}
	}
	/**/
}

void tickCleanTower(gnode* grid,tower* t){
	if (t->type==BASE)
		return;
	if (t->health>0)
		return;
	grid[t->position].tower=0;
	memset(t,0,sizeof(tower));
	/**/
}



void forEachTower(gnode* grid, void (process)(gnode*g,tower*t)){//add function
	int i;
	for(i=0;i<config.tower_max;i++)
		if(config.tower_array[i].id>0)
			process(grid,&config.tower_array[i]);
}


