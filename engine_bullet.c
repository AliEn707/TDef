#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"

bullet* newBullet(){
	int i;
	for(i=0;i<config.bullet_max;i++)
		if (config.bullet_array[i].id==0){
			config.bullet_array[i].id=getGlobalId();
			return &config.bullet_array[i];
		}
	return 0;
}

void tickMiscBullet(gnode * grid,bullet * b){
	b->bit_mask=0;
}

void tickCleanBullet(gnode * grid,bullet * b){
	if (b->detonate>0)
		memset(b,0,sizeof(bullet));
}

void tickProcessBullet(gnode * grid,bullet * b){
	if (b->detonate==0){
		//vec dir={0,0};
//		printf("!!%g %g\n",b->position.x,b->position.y);
		//float length=getDir(&b->position,&b->destination,&dir);
		float delta;
		if (config.bullet_types[b->type].move_type!=SHOT){
			b->position.x+=b->direction.x*config.bullet_types[b->type].speed;
			b->position.y+=b->direction.y*config.bullet_types[b->type].speed;
			delta=config.bullet_types[b->type].speed;
		}else{
			b->position.x=b->destination.x;
			b->position.y=b->destination.y;
			delta=0.1;
		}
		if (eqInD(b->position.x,b->destination.x,delta)&&
			eqInD(b->position.y,b->destination.y,delta)){
			int i,j;
			int multiple=0;
			if (b->target==TOWER){
				tower * tmp;
				if ((tmp=grid[to2d((int)b->position.x,(int)b->position.y)].tower)>0)
					if(config.players[tmp->owner].isfriend!=b->isfriend){
						if(tmp->type==BASE)
							config.players[tmp->owner].base_health-=b->damage;
						else
							tmp->health-=b->damage;
						multiple++;
					}
				
			}
			if (config.bullet_types[b->type].attack_type==SINGLE && multiple>0)
				goto out;
			if (b->target==NPC){  
				npc* tmp;
				for(j=0;j<MAX_PLAYERS;j++)
					for(tmp=grid[to2d((int)b->position.x,(int)b->position.y)].npcs[j];
						tmp!=0;tmp=tmp->next)
							if (tmp->isfriend!=b->isfriend)
								if (eqInD(tmp->position.x,b->position.x,0.1)&&
									eqInD(tmp->position.y,b->position.y,0.1)){
									tmp->health-=b->damage;
									multiple++;
									if (config.bullet_types[b->type].attack_type==SINGLE)
										goto out;
									if (config.bullet_types[b->type].attack_type==MULTIPLE && 
										multiple>config.bullet_types[b->type].area)
										goto out;
								}
				
			}
			//add area damage to Npc
			//add area damage to towers
			//if (config.bullet_types[b->type].attack_type==AREA ||
			// 	config.bullet_types[b->type].attack_type==AREA_FF)
			
out:
			b->detonate++;
		}
	}
}



void forEachBullet(gnode* grid, void (process)(gnode*g,bullet*b)){
	int i;
	for(i=0;i<config.bullet_max;i++)
		if(config.bullet_array[i].id>0)
			process(grid,&config.bullet_array[i]);
}
