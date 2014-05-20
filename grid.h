#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>




//npc statuses
#define IN_ATTACK 1
#define IN_MOVE 2
#define IN_IDLE 3


//gnode type (buildable component)
#define NOTHING -1
#define MAY_BUILD 0
#define MAY_WALK 1 //not build
#define MAY_WALK_AND_BUILD 2
//>1 may walk and build



typedef 
struct vector2{
	float x;
	float y;
} vec;


typedef 
struct effect{ //модификаторы свойств > 0
	float speed;
	float shield;
	float damage;
	int time;
	int status; //вероятность срабатывания в %
} effect;

typedef
struct tower_type{
	int id;
	int health;
	int damage;
	int energy;
	int shield;
	int attack_speed; //ticks to attack
	int cost;
	effect effects;   //наносимые эффекты
}tower_type;

typedef
struct npc_type{
	int id;
	int health;
	int damage;
	int shield;
	int attack_speed;
	float move_speed;
	int cost;
	effect effects;  //наносимые эффекты
}npc_type;


typedef
struct tower{
	int x;
	int y;
	int id;
	int position;
	int type;
	int health;
	int energy;
	int attack;
	effect effects;  //полученные эффекты
	struct npc* target;
}tower;

typedef
struct npc{
	char status;
	char iffriend;
	vec position;
	vec direction;
	int id;
	int type;
	int health;
	int attack;
	effect effects; //полученные эффекты
	union {
		struct npc* ntarget;
		struct tower* ttarget;
	};
	struct npc * next; //for list in gnode
}npc;


typedef
struct bullet{
	vec position;
	vec destination;
	int type;
	int speed;
	union {
		struct npc* ntarget;
		struct tower* ttarget;
	};
}bullet;



typedef
struct gnode{
	char buildable;
	int id;
	int next;
	tower * tower;
	npc * enpcs;
	npc * fnpcs;
	float f;
	float g;
	float h;
	
} gnode;


typedef
struct config{
	int gridsize;
	int tower_max;
	int tower_types_size;
		tower_type* tower_types;
		struct tower* tower_array;
	int npc_max;
	int npc_types_size;
		npc_type* npc_types;
		struct npc* npc_array;
	int bullet_max;
		struct bullet* bullet_array;
} engine_config;

///////
#define setGridSize(size) (config.gridsize=size)
engine_config config;

///////
#define to2d(x,y)  ((x)*config.gridsize+(y))
#define idtox(id)  (id/config.gridsize)
#define idtoy(id)  (id%config.gridsize)

#define setvecto0(v) v.x=0;v.y=0


int aSearch(gnode* grid,gnode* start,gnode* goal);//start-куда, goal-откуда
#define setNPCPath(grid,start,goul) aSearch(grid,grid+goul,grid+start)
