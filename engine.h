//wait tto set need tps
void syncTPS();

void initArrays();

void realizeArrays();

//helper
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
void setupPlayer(int id,int isfriend,int base_health,tower* base);


