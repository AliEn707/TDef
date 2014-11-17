#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "gridmath.h"

/*
#define EPS 1e-5

static vec* getTargetPos(vec *pos, vec *dir, float v, vec * $pos, float $v){
	static vec target;
//	printf("pos->x = %g, pos->y = %g, dir->x = %g, dir->y = %g, v = %g, $pos->x = %g, $pos->y = %g, $v = %g\n", pos->x, pos->y, dir->x, dir->y, v, $pos->x, $pos->y, $v);
//	printf("f(%g, %g, %g, %g, %g, %g, %g, %g, &targetX, &targetY);\n", dir->x, dir->y, pos->x, pos->y, v, $pos->x, $pos->y, $v);
//	f(dirx, diry, posx, posy, v, $posx, $posy, $v, &x, &y);
//	int f(float dir->x, float dir->y, float pos->x, float pos->y, float v, float $pos->x, float $pos->y, float $v, float *target.x, float *target.y) {
//	int f(float dir->x, float dir->y, float pos->x, float pos->y, float v, float $pos->x, float $pos->y, float $v, float *target.x, float *target.y) {
//	printf("dir- {%g|%g}\n",dir->x,dir->y);
	target.x = pos->x;
	target.y = pos->y;		
	if (fabs(dir->x) < EPS && dir->y < EPS) {//unit is not moving
		printf("not moving\n");
		return &target;
	}
	if (fabs(dir->x) < EPS){
		printf("revert\n");
		vec _pos={pos->y,pos->x};
		vec _dir={dir->y,dir->x};
		vec _$pos={$pos->y,$pos->x};
		
		getTargetPos(&_pos,&_dir,v,&_$pos,$v);
		float tmp=target.x;
		target.x=target.y;
		target.y=tmp;
		return &target;
	}
	float k = dir->y / dir->x;
	float b = pos->y - k*pos->x;
	float v2 = $v*$v;
	float v1 = v*v;
	float a1 = v2 + v2*k*k - v1 - v1*k*k;
	float b1 = -2*v2*pos->x + 2*v2*k*b - 2*pos->y*v2*k + 2*v1*$pos->x + 2*$pos->y*v1*k - 2*v1*k*b;
	float c1 = v2*pos->x*pos->x + v2*b*b - 2*pos->y*v2*b + v2*pos->y*pos->y - v1*$pos->x*$pos->x - v1*$pos->y*$pos->y + 2*v1*$pos->y*b - v1*b*b;
//	printf("a = %g b = %g c = %g\n", a1, b1, c1);
	if (fabs(a1) < EPS) {
		target.x = -c1/b1;
		target.y = k*target.x + b;	
		return &target;
	}
	float D = b1*b1 - 4*a1*c1;
	if (D < 0){
		printf("D < 0\n");
		return &target;
	}

//	if (fabs(D) < EPS) {
//		printf("min D\n");
//		target.x = (-b1)/(2*a1);
//		target.y = k*target.x + b;
//		return &target;
//	}

	float _sqrtD=sqrt(D);
	float x1 = (-b1 + _sqrtD)/(2*a1);
	float x2 = (-b1 - _sqrtD)/(2*a1);
	float y1 = k*x1 + b;
	float y2 = k*x2 + b;
//	printf("x1 = %g y1 = %g\n", x1, y1);
//	printf("x2 = %g y2 = %g\n", x2, y2);	
	if (sqr(pos->x + dir->x - x2) + sqr(pos->y + dir->y - y2) < sqr(pos->x + dir->x - x1) + sqr(pos->y + dir->y - y1)) {
	//if (sqr(pos->x - x1) + sqr(pos->y - y1) > sqr(pos->x - x2) + sqr(pos->y - y2)) {//use x2
		target.x = x2;	
		target.y = y2;	
	} else { //use x1
		target.x = x1;	
		target.y = y1;
	}
	printf("yea the end\n");
	return &target;
}
*/
tower* damageTower(tower* t,bullet* b){
//	if(t->type==BASE){
//		config.players[t->owner].base_health-=b->damage;
//		setMask((&config.players[t->owner]),PLAYER_HEALTH);
//	}else{
		t->health-=b->damage;
		setMask(t,TOWER_HEALTH);
//	}
	return t;
}

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
	if (t!=0){
		if (t->type==BASE){
			t->health=config.players[t->owner].base_health;
		}else{
			t->health=config.tower_types[t->type].health;
		}
		t->energy=config.tower_types[t->type].energy;
	}
}


tower* spawnTower(gnode * grid,int node_id,int owner,int type){
	tower* t;
//	printf("spawn tower %d on %d by %d\n",type,node_id,owner);
	if (node_id<0 || node_id>=config.gridsize*config.gridsize)
		return 0;
	if (grid[node_id].tower!=0)
		return 0;
	gnode* node=&grid[node_id];
	if((t=newTower())==0){
		perror("newTower spawnTower");
		return 0;
	}
	t->owner=owner;
	t->position=node_id;
	t->type=type;
	t->bit_mask=0;
	t->level=type;//1;
	setMask(t,TOWER_CREATE);
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

int tickMiscTower(gnode* grid,tower* t){
	if (t->attack_count<config.tower_types[t->type].attack_speed)
		t->attack_count++;
	t->bit_mask=0;
	return 0;
}

int tickDiedCheckTower(gnode* grid,tower* t){
	t->target=diedCheckNpc(t->target);
	/**/
	return 0;
}



int tickAttackTower(gnode* grid,tower* t){
	if (t->type==BASE)
		return 0;
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
					for (k=0;k<MAX_GROUPS;k++)
						if (k!=config.players[t->owner].group)
							for(tmp=grid[to2d(xid,yid)].npcs[k];
									tmp!=0;tmp=tmp->next)
								if (canSee(grid,&(vec){x+0.5,y+0.5},&tmp->position)>0)
									if(sqr(x+0.5-(tmp->position.x))+sqr(y+0.5-(tmp->position.y))<=sqr(config.tower_types[t->type].distanse)){
										t->target=tmp;
										setMask(t,TOWER_TARGET);
										if (rand()%100<60)
											return 0;
									}
	}else{
		if (t->target->id==0)
			return 0;
		if(rand()%100<8){
			t->target=0;
			return 0;
		}
		if (sqr(t->target->position.x-getGridx(t->position))+
				sqr(t->target->position.y-getGridy(t->position))>
				sqr(config.tower_types[t->type].distanse)){
			t->target=0;
			return 0;
		}
//		printf("\t|%d %d\n",t->attack_count,config.tower_types[t->type].attack_speed);
		if (t->attack_count>=config.tower_types[t->type].attack_speed){
				t->attack_count=0;
				bullet* b;//set params of bullet
				if ((b=newBullet())==0){
					perror("newBullet tickAttackBullet");
					return -1;
				}
				vec position={getGridx(t->position),getGridy(t->position)};
				// getTargetPos(vec *pos, vec *dir, float v, vec * $pos, float $v){
//				vec* target=getTargetPos(&t->target->position,
//									&t->target->direction,
//									config.npc_types[t->target->type].move_speed,
//									&position,
//									config.bullet_types[config.tower_types[t->type].bullet_type].speed
//									);
				memcpy(&b->position,&position,sizeof(vec));
				memcpy(&b->source,&position,sizeof(vec));
				//memcpy(&b->destination,target,sizeof(vec));
//				printf("aaaa = %g|%g {%g|%g}\n",target->x,target->y,t->target->position.x,t->target->position.y);
				b->ntarget=t->target;
				b->destination.x=t->target->position.x;
				b->destination.y=t->target->position.y;
				b->max_dist=config.tower_types[t->type].distanse;
				b->type=config.tower_types[t->type].bullet_type;
				b->damage=config.tower_types[t->type].damage;
				b->support=config.tower_types[t->type].support;
				b->group=config.players[t->owner].group;
				b->owner=t->id;
				setMask(b,BULLET_CREATE);
//				b->target=NPC;
				getDir(&b->position,&b->destination,&b->direction);
//				memcpy(&b->effects,&config.npc_types[n->type].effects,sizeof(effects));
			}
	}
	/**/
	return 0;
}

int tickCleanTower(gnode* grid,tower* t){
	if (t->type==BASE)
		return 0;
	if (t->health>0)
		return 0;
	grid[t->position].tower=0;
	memset(t,0,sizeof(tower));
	/**/
	return 0;
}



int forEachTower(gnode* grid, int (process)(gnode*g,tower*t)){//add function
	int i;
	for(i=0;i<config.tower_max;i++)
		if(config.tower_array[i].id>0)
			if(process(grid,&config.tower_array[i])<0)
				return -1;
	return 0;
}


