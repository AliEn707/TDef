
int parseArgv(int argc,char * argv[]);
//wait tto set need tps
void syncTPS();
//tick per second limiter
int timePassed(int i);
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


//player
void setupPlayer(int id,int group,int base_health,tower* base);

void processWaves(gnode* grid);

