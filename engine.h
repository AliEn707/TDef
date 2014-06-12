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
tickTargetNpc
tickAtackNpc
tickAtackTower
tickProcessBullet
tickMoveNpc

some user stuff
-remove tower set it to died

*/


//player



