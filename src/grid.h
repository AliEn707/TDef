#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/sem.h>
//#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h> 
#include <pthread.h>
#include <signal.h>

#include "t_sem.h"

#define BIT_1 1
#define BIT_2 2
#define BIT_3 4
#define BIT_4 8
#define BIT_5 16
#define BIT_6 32
#define BIT_7 64
#define BIT_8 128
#define BIT_9 256
#define BIT_10 512
#define BIT_11 1024
#define BIT_12 2048
#define BIT_13 4096
#define BIT_14 8192
#define BIT_15 16384
#define BIT_16 32768
#define BIT_17 65536
#define BIT_18 131072
#define BIT_19 262144
#define BIT_20 524288
#define BIT_21 1048576
#define BIT_22 2097152
#define BIT_23 4194304
#define BIT_24 8388608
#define BIT_25 16777216
#define BIT_26 33554432
#define BIT_27 67108864
#define BIT_28 134217728
#define BIT_29 268435456
#define BIT_30 536870912
#define BIT_31 1073741824
#define BIT_32 2147483648


#define TPS 8


#define NPC_PATH 20
#define MAX_AREA 33

//time constants on game start
#define START_WAITING_TIME 30000000
#define START_WAITING_STEP 1000000

#define PLAYER_MAX 8
#define TOWER_SET_NUM 9
#define NPC_SET_NUM 9

#define SHIELD_RECOVERY (TPS*3)

//player types
#define PC 0

//tower types
#define BASE 0

//npc types
#define HERO 0

//group types
#define ENEMY 0


//bullet damage types
#define SINGLE 1
#define MULTIPLE 2
#define AREA 3
#define AREA_FF 4

//bullet move types
#define SHOT 1

//bullet target types  !!not used
#define NPC 1
#define TOWER 2

//npc statuses
#define IN_ATTACK 1
#define IN_MOVE 2
#define IN_TARGET 3  //not used
#define IN_MOVE_ATTACK 4  //not used
#define IN_IDLE 0


//gnode type (buildable component)
#define NOTHING -1
//>1 may walk and build
#define MAX_GROUPS 20

//msg to client
#define MSG_TEST 0
#define MSG_NPC 1
#define MSG_TOWER 2
#define MSG_BULLET 3
#define MSG_PLAYER 4
#define MSG_INFO 5
//additional messages to client
#define MSG_INFO_WAITING_TIME 1

//msg to server
#define MSG_SPAWN_TOWER 1
#define MSG_SPAWN_NPC 2
#define MSG_DROP_TOWER 3
#define MSG_MOVE_HERO 4
#define MSG_SET_TARGET 5

///////bit mask
#define setMask(z,x) (z)->bit_mask|=x
#define checkMask(z,x) z&x

#define PLAYER_BASE BIT_1
#define PLAYER_MONEY BIT_2
#define PLAYER_CREATE BIT_3
#define PLAYER_LEVEL BIT_4
#define PLAYER_HERO BIT_5
#define PLAYER_HERO_COUNTER BIT_6
#define PLAYER_TARGET BIT_7
#define PLAYER_FAIL BIT_8
#define PLAYER_SETS BIT_9

#define NPC_HEALTH BIT_1
#define NPC_POSITION BIT_2
#define NPC_CREATE BIT_3
#define NPC_LEVEL BIT_4
#define NPC_SHIELD BIT_5
#define NPC_STATUS BIT_6

#define TOWER_HEALTH BIT_1
#define TOWER_TARGET BIT_2
#define TOWER_CREATE BIT_3
#define TOWER_LEVEL BIT_4
#define TOWER_SHIELD BIT_5

#define BULLET_POSITION BIT_1
#define BULLET_DETONATE BIT_2
#define BULLET_CREATE BIT_3



typedef 
struct vector2{
	float x;
	float y;
} vec;

typedef 
struct vector2i{
	int x;
	int y;
} veci;

typedef 
struct vector2c{
	char x;
	char y;
} vecc;

typedef 
struct workerarg{
	int sock;
	int id;
	struct gnode* grid;
} worker_arg;


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
	float armor;
	int distanse;
	int attack_speed; //ticks to attack
	int cost;
	int receive;
	int ignor_type;
	int prior_type;
	int bullet_type;
	int support;
	effect effects;   //наносимые эффекты
}tower_type;

typedef
struct npc_type{
	int id;
	int health;
	int damage;
	int shield;
	int energy;
	float armor;
	int attack_distanse;
	int see_distanse;
	int attack_speed;
	float move_speed;
	int cost;
	int type;
	short ignor_type;
	short prior_type;
	int bullet_type;
	int support;
	int receive;
	effect effects;  //наносимые эффекты
	short attack_tower;
}npc_type;

typedef
struct bullet_type{
	int id;
	float speed;
	int attack_type;
	int move_type;
	int area;
}bullet_type;

typedef
struct tower{
	int id;
	int bit_mask;
	int position;//id of node
	int type;
	int health;
	int shield;
	short $shield; //shield not attack counter
	int energy;
	int attack_count;
	int owner;
	short level;
	effect effects;  //полученные эффекты
	struct npc* target;
	int last_attack;
}tower;

typedef
struct{
	short node;
	char tower;
}path;

typedef
struct npc{
	char status;
	int owner;
	int bit_mask;
	vec position;
	vec destination;
	vec direction;
	int id;
	int type;
	int health;
	int shield;
	short $shield;  //shield not attack counter
	int energy;
	short level;
	effect effects; //полученные эффекты
	struct npc* ntarget;
	struct tower* ttarget;
	int finded_base;
	
	npc_type *type_data;
	
	int attack_count;
	int path_count;
	path path[NPC_PATH];
	struct npc * next; //for list in gnode
	int last_attack;
}npc;


typedef
struct bullet{
	int id;
	int bit_mask;
	vec position;
	vec destination;
	vec direction;
	vec source;
	char group;
	char support;
	int type;
	char detonate;
	int damage;
	effect effects;
	int owner;
	float max_dist;
	npc* ntarget;
}bullet;



typedef
struct gnode{
	float f;
	float g;
	float h;
	char loop;
	int id;
	int next;
	char walkable; //-1 no see, 0 no walk, 1 walk
	tower * tower;
	npc * npcs[MAX_GROUPS];
	char buildable; //<=0 no build, >0 build 
	
} gnode;

typedef
struct player{
	int id;
	int bit_mask;
	int group; //player number [0-7]
	int first_send:1;
	int target_changed:1;
	struct {
		int id;
		int num;
	} tower_set[TOWER_SET_NUM];
	struct {
		int id;
		int num;
	} npc_set[NPC_SET_NUM];
	int base_id; //id of base in list of all map bases
	tower * base;
	npc * hero;
	
	tower_type base_type;
	npc_type hero_type;
	npc $npc$; //fake npc need to click move
	int hero_counter;
	int _hero_counter;
	
	int money;
	int level; // TODO: change to short
	
	short target;
	struct {
		int npcs_spawned;
		int towers_built;
		int npcs_killed;
		int towers_destroyed;
		int npcs_lost;
		int towers_lost;
		int xp;//experience
	} stat;
} player;


typedef
struct wave_part{
	int point;
	int npc_type;
	int num;
	int level;
	unsigned int delay;
	int spawned;
} wave_part;

typedef
struct wave{
	unsigned int delay;
	int parts_num;
	wave_part * parts;
} wave;

typedef
struct wave_spawner{
	unsigned int wave_num;
	unsigned int wave_part_num;
	unsigned int wave_ticks;
} wave_spawner;

typedef 
struct base{
	int id;
	int position;
	int point_id;
} base;

typedef 
struct point{
	int id;
	int position;
} point;

typedef
struct netw{
	
}netw;

typedef
struct config{
	int gridsize;
	gnode* grid;
	
	vecc * area_array[MAX_AREA];
	int area_size[MAX_AREA];
	
	unsigned int global_id;
	int players_num;
		player players[PLAYER_MAX];
	struct timeval time;
	
	unsigned int waves_size;
		wave* waves;
	wave_spawner wave_current;
	unsigned int bases_size;
		base* bases;
	unsigned int points_size;
		point* points;
		
	int current_money_timer;
	int max_money_timer;
	
	int debug;	
		
	struct {
		int sock;
		int port;
		int token;
		short run;
		int wait_start;
		int players;
		char map[20];
		struct {
			char host[60];
			int port;
		} public;
	} game;
} engine_config;

///////
#define setGridSize(size) (config.gridsize=size)
engine_config config;

struct t_sem_struct{
	t_sem_t send;
	t_sem_t player;
} t_sem;

///////
#define getGridx(id) (1.0*idtox(id)+0.5f)
#define getGridy(id) (1.0*idtoy(id)+0.5f)

#define getGridId(v) to2d(((int)v.x),((int)v.y))
#define to2d(x,y)  ((x)*config.gridsize+(y))

#define idtox(id)  (id/config.gridsize)
#define idtoy(id)  (id%config.gridsize)

#define sqr(x) ({typeof(x) $x=(x); ($x)*($x);})//check this

#define eqInD(a,b,eq) (fabsf((a)-(b))<=(eq))
//////
#define setVecto0(v) memset(&v,0,sizeof(vec))


int aSearch(gnode* grid,gnode* start,gnode* goal, path* path);//start-куда, goal-откуда
#define setNPCPath(grid,start,goul) aSearch(grid,grid+goul,grid+start)

#define getGlobalId() (++config.global_id!=0?config.global_id:++config.global_id)

