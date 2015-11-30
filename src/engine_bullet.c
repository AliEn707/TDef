#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "gridmath.h"
#include "types.h"
#include "bintree.h"

static unsigned int bullet_num=0;
static unsigned int bullet_max=1000; //default value
static bullet** bullet_array;
static bintree bullet_tree;

bullet* newBullet(){
	if (bullet_num>=bullet_max)
		return 0;
	if ((bullet_array[bullet_num]=malloc(sizeof(bullet)))==0){
		perror("newBullet bullet malloc");
		return 0;
	}
	memset(bullet_array[bullet_num],0,sizeof(bullet));
	bullet_array[bullet_num]->id=getGlobalId();
	do{
		int *index;
		if ((index=malloc(sizeof(*index)))==0){
			perror("newBullet index malloc");
			break;
		}
		*index=bullet_num;
//		printf("%d newNpc index %d added %d\n",npc_array[npc_num]->id, npc_num,*index);
		if (bintreeAdd(&bullet_tree, bullet_array[bullet_num]->id, index)==0){
			perror("newBullet bintreeAdd malloc");
			break;
		}
		bullet_num++;
		return bullet_array[bullet_num-1];
	}while(0);
	delBullet(0, bullet_array[bullet_num]);
	return 0;
}

void setBulletsMax(int size){
	bullet_max=size;
}

void allocBullets(){
	if ((bullet_array=malloc(sizeof(*bullet_array)*bullet_max))==0)
		perror("malloc bullet initArrays");
	memset(bullet_array,0,sizeof(*bullet_array)*bullet_max);
}

void realizeBullets(){
	bintreeErase(&bullet_tree, free);
	free(bullet_array);
}

int delBullet(gnode* grid,bullet* b){
	int i=*(int*)bintreeGet(&bullet_tree, b->id);
//	for(i=0;i<bullet_num && bullet_array[i]!=b;i++);
	if (i==bullet_num)
		return -1;
	bintreeDel(&bullet_tree, bullet_array[i]->id, free);
	free(bullet_array[i]);
	bullet_num--;
	if (bullet_num!=i){
		bullet_array[i]=bullet_array[bullet_num];
		int *index=bintreeGet(&bullet_tree, bullet_array[i]->id);
		*index=i;
	}
	bullet_array[bullet_num]=0;
	return -1;	
}

int tickMiscBullet(gnode * grid,bullet * b){
	b->bit_mask=0;
	return 0;
}

int tickCleanBullet(gnode * grid,bullet * b){
	if (b->detonate>0)
		return 1;
	return 0;
}

int tickDiedCheckBullet(gnode* grid,bullet* b){
	b->ntarget=diedCheckNpc(b->ntarget);
	/**/
	return 0;
}

int tickProcessBullet(gnode * grid,bullet * b){
	if (b->detonate==0){
		bullet_type * type=typesBulletGet(b->type);
		if (type==0){
			b->detonate=1;
			return 0;
		}
		//vec dir={0,0};
//		printDebug("!!%g %g\n",b->position.x,b->position.y);
		//float length=getDir(&b->position,&b->destination,&dir);
		float delta=1;
		if (type->move_type!=SHOT){
			b->position.x+=b->direction.x*type->speed;
			b->position.y+=b->direction.y*type->speed;
			delta=type->speed;
			if (delta<0.2)
				delta=0.2;
		}else{
			b->position.x=b->destination.x;
			b->position.y=b->destination.y;
		}
		setMask(b,BULLET_POSITION);
		if (eqInD(b->position.x,b->destination.x,delta)&&
				eqInD(b->position.y,b->destination.y,delta)){
			int i,j,k;
			int multiple=0;
			int x=(int)b->position.x;
			int y=(int)b->position.y;
			int xid,yid;
			int _id=to2d(x,y);
			if ( x<0 || y<0 || x>=config.gridsize || y>=config.gridsize)
				goto out;
			//tower search
			{
				tower * tmp;
				if (_id<sqr(config.gridsize))
					if ((tmp=grid[_id].tower)>0)
						if(config.players[tmp->owner].group!=b->group){
							damageTower(tmp,b);
							multiple++;
						}
			}	
			
			if (type->attack_type==SINGLE && multiple>0)
				goto out;
			
			//npc search
			{
				npc* tmp;
				if (_id<sqr(config.gridsize))
					for(j=0;j<MAX_GROUPS;j++)
						for(tmp=grid[_id].npcs[j];
								tmp!=0;tmp=tmp->next)
							if (config.players[tmp->owner].group!=b->group)
								if (eqInD(tmp->position.x,b->position.x,delta)&&
									eqInD(tmp->position.y,b->position.y,delta))
								//attack npc near the destination 
								{	
									damageNpc(tmp,b);
									multiple++;
									if (type->attack_type==SINGLE)
										goto out;
									if (type->attack_type==MULTIPLE && 
										multiple>type->area)
										goto out;
								}
				//if npc not in node, see nodes near
				i=0;
				for(j=0;j<config.area_size[i];j++)
					if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
							((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize))
						for(k=0;k<MAX_GROUPS;k++)
							for(tmp=grid[to2d(xid,yid)].npcs[k];
								tmp!=0;tmp=tmp->next)
									if (config.players[tmp->owner].group!=b->group)
										if (eqInD(tmp->position.x,b->position.x,delta)&&
											eqInD(tmp->position.y,b->position.y,delta))
										//attack first npc in gnode, need to correct
										{	
											damageNpc(tmp,b);
											multiple++;
											if (type->attack_type==SINGLE)
												goto out;
											if (type->attack_type==MULTIPLE && 
												multiple>type->area)
												goto out;
										}
			}
			
			//add area damage to Npc
			//add area damage to towers
			if (type->attack_type==AREA ||
			 	type->attack_type==AREA_FF){
				//add area gamage	
				npc* tmp;
				for (i=0;i<type->area;i++)
					for(j=0;j<config.area_size[i];j++)
						if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
								((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize)){
							//tower
							if (grid[to2d(xid,yid)].tower!=0)
								if (canSee(grid,&b->position,&(vec){xid+0.5,yid+0.5})>0)
									if(type->attack_type==AREA?
											config.players[grid[to2d(xid,yid)].tower->owner].group!=b->group
											:1){
										damageTower(grid[to2d(xid,yid)].tower,b);
									}
							//npc
							for (k=0;k<MAX_GROUPS;k++)
								if (type->attack_type==AREA?k!=b->group:1)
									for(tmp=grid[to2d(xid,yid)].npcs[k];
											tmp!=0;tmp=tmp->next)
										if (canSee(grid,&b->position,&tmp->position)>0)
											damageNpc(tmp,b);
						}
			}	
out:
			b->detonate++;
			setMask(b,BULLET_DETONATE);
		}else{
			if (b->ntarget!=0){
				if (glength(&b->ntarget->position,&b->source)>b->max_dist){
					b->ntarget=0;
				}else{
					//TODO: add folow attr
					memcpy(&b->destination,&b->ntarget->position,sizeof(b->destination));
					getDir(&b->position,&b->destination,&b->direction);
				}
			}
		}
	}
	return 0;
}



int forEachBullet(gnode* grid, int (process)(gnode*g,bullet*b)){
	int i;
	for(i=0;i<bullet_num;i++)
		if (process(grid,bullet_array[i])!=0)
			return -1;
	return 0;
}

int forEachBulletRemove(gnode* grid, int (process)(gnode*g,bullet*b)){
	int i;
	for(i=0;i<bullet_num;i++)
		if (process(grid,bullet_array[i])!=0){
			delBullet(grid,bullet_array[i]);
			i--;
		}
	return 0;
}

