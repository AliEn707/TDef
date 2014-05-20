


void initArrays();

void realizeArrays();

npc* newNpc();

tower* newTower();

bullet* newBullet();

npc* addNpc(gnode* node,npc* n);

npc* getNpc(gnode* grid,npc* n);

int delNpc(gnode* grid,npc* n);

void setNpcBase(npc* n);

int spawnNpc(gnode* grid,int node_id,int isfriend,int type);
