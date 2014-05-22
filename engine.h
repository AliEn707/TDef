


void initArrays();

void realizeArrays();

npc* newNpc();

tower* newTower();

bullet* newBullet();

npc* addNpc(gnode* node,npc* n);

npc* getNpc(gnode* grid,npc* n);

int delNpc(gnode* grid,npc* n);

void setNpcBase(npc* n);

npc* spawnNpc(gnode* grid,int node_id,int isfriend,int type);

void tickMoveNpc(gnode* grid,npc* n);

void forEachNpc(gnode* grid, void (process)(gnode*g,npc*n));

void setTowerBase(tower* t);

tower* spawnTower(gnode * grid,int node_id,int owner,int type);

int findEnemyBase(int isfriend);