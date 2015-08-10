
void allocArrays();

int parseArgv(int argc,char * argv[]);
//wait tto set need tps
int syncTPS(int z,int _TPS);
//tick per second limiter
int timePassed(struct timeval * t);
//init primary arrays 
void initArrays();

void realizeArrays();

//check node between a and b
int canSee(gnode* grid,vec* a,vec* b);

int canWalkThrough(gnode* grid,vec* a,vec* b);

/*
must be this

tickDiedCheckNpc
tickDiedCheckTower
tickDiedCheckBullet

tickCleanNpc
tickCleanTower
tickCleanBullet

tickMoveNpc
tickTargetNpc
tickAtackNpc

tickAtackTower
tickProcessBullet

some user stuff

tickMiscNpc
tickMiscTower
tickMiscBullet

-remove tower == set it to died

*/

void processWaves(gnode* grid);


//player
void setupPlayer(int id,int group);

void setPlayerBase(int id,tower* base);
void setPlayerHero(int id,npc* hero);

void forEachPlayer(gnode * grid);
void playersClearBitMasks();

void printStats();
void printDebug(const char* format, ...); 

