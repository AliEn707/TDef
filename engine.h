//wait tto set need tps
void syncTPS();

void initArrays();

void realizeArrays();
//npc
npc* newNpc();

npc* addNpc(gnode* node,npc* n);

npc* getNpc(gnode* grid,npc* n);

int delNpc(gnode* grid,npc* n);

npc* diedCheckNpc(npc* n);

void setNpcBase(npc* n);

npc* spawnNpc(gnode* grid,int node_id,int isfriend,int type);

int findEnemyBase(int isfriend);

void tickCleanNpc(gnode* grid,npc* n);

void tickTargetNpc(gnode* grid,npc* n);

void tickAttackNpc(gnode* grid,npc* n);

void tickMoveNpc(gnode* grid,npc* n);

void forEachNpc(gnode* grid, void (process)(gnode*g,npc*n));
//tower
tower* newTower();

tower* diedCheckTower(tower* t);

void setTowerBase(tower* t);

tower* spawnTower(gnode * grid,int node_id,int owner,int type);

void tickMiscTower(gnode* grid,tower* t);

void tickDiedCheckTower(gnode* grid,tower* t);

void tickAttackTower(gnode* grid,tower* t);

void tickCleanTower(gnode* grid,tower* t);

void forEachTower(gnode* grid, void (process)(gnode*g,tower*t));
//bullet
bullet* newBullet();

void tickMiscBullet(gnode * grid,bullet * b);

void tickCleanBullet(gnode * grid,bullet * b);

void tickProcessBullet(gnode * grid,bullet * b);

void forEachBullet(gnode* grid, void (process)(gnode*g,bullet*b));
//player



