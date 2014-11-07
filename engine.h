
int parseArgv(int argc,char * argv[]);
//wait tto set need tps
void syncTPS(int z,int _TPS);
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
tickMiscNpc
tickMiscTower
tickMiscBullet
tickDiedCheckNpc
tickDiedCheckTower

tickCleanNpc
tickCleanTower
tickCleanBullet

tickProcessBullet
tickMoveNpc
tickTargetNpc
tickAtackNpc
tickAtackTower

some user stuff
-remove tower set it to died

*/

void processWaves(gnode* grid);


//player
void setupPlayer(int id,int group,int base_health);

void setPlayerBase(int id,tower* base);

