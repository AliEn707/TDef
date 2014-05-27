#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>

#define TPS 40

#define NPC_PATH 5

//tower types
#define BASE 1


//npc statuses
#define IN_ATTACK 1
#define IN_MOVE 2
#define IN_TARGET 3
#define IN_MOVE_ATTACK 4
#define IN_IDLE 0


//gnode type (buildable component)
#define NOTHING -1
//>1 may walk and build



typedef 
struct vector2i{
	float x;
	float y;
} vec;

typedef 
struct vector2{
	int x;
	int y;
} veci;


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
	int distanse;
	int attack_speed; //ticks to attack
	int cost;
	char ignor_type;
	char prior_type;
	effect effects;   //наносимые эффекты
}tower_type;

typedef
struct npc_type{
	int id;
	int health;
	int damage;
	int shield;
	int attack_distanse;
	int see_distanse;
	int attack_speed;
	float move_speed;
	int cost;
	char type;
	char ignor_type;
	char prior_type;
	effect effects;  //наносимые эффекты
}npc_type;


typedef
struct tower{
	int id;
	int position;//id of node
	int type;
	int health;
	int shield;
	int energy;
	int attack_count;
	int owner;
	effect effects;  //полученные эффекты
	struct npc* target;
}tower;

typedef
struct npc{
	char status;
	char isfriend;
	vec position;
	vec destination;
	int id;
	int type;
	int health;
	int shield;
	int attack_count;
	effect effects; //полученные эффекты
	struct npc* ntarget;
	struct tower* ttarget;
	int path_count;
	int path[NPC_PATH];
	struct npc * next; //for list in gnode
}npc;


typedef
struct bullet{
	int id;
	vec position;
	vec destination;
	vec source;
	int type;
	int speed;
	union {
		struct npc* ntarget;
		struct tower* ttarget;
	};
}bullet;



typedef
struct gnode{
	float f;
	float g;
	float h;
	int id;
	int next;
	char walkable; //-1 no see, 0 no walk, 1 walk
	tower * tower;
	npc * enpcs;
	npc * fnpcs;
	char buildable; //<=0 no build, >0 build 
	
} gnode;

typedef
struct player{
	int id;
	int isfriend;
	int base_health;
} player;

typedef
struct config{
	int gridsize;
	veci* area_array[30];
	int area_size[30];
	unsigned int tower_max;
	unsigned int tower_types_size;
		tower_type* tower_types;
		struct tower* tower_array;
	unsigned int npc_max;
	unsigned int npc_types_size;
		npc_type* npc_types;
		struct npc* npc_array;
	unsigned int bullet_max;
		struct bullet* bullet_array;
	unsigned int global_id;
	int player_max;
		player* players;
	struct timeval time;
} engine_config;

///////
#define setGridSize(size) (config.gridsize=size)
engine_config config;

///////
#define getGridx(id) (1.0*idtox(id)+0.5f)
#define getGridy(id) (1.0*idtoy(id)+0.5f)

#define getGridId(v) to2d(((int)v.x),((int)v.y))
#define to2d(x,y)  ((x)*config.gridsize+(y))

#define idtox(id)  (id/config.gridsize)
#define idtoy(id)  (id%config.gridsize)

#define sqr(x)  ((x)*(x))
#define eqInD(a,b,eq) (fabs(a-b)<=eq)
//////
#define setVecto0(v) memset(&v,0,sizeof(vec))


int aSearch(gnode* grid,gnode* start,gnode* goal, int* path);//start-куда, goal-откуда
#define setNPCPath(grid,start,goul) aSearch(grid,grid+goul,grid+start)

#define getGlobalId() (++config.global_id!=0?config.global_id:++config.global_id)

