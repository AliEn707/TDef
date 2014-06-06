npc* newNpc();

npc* addNpc(gnode* node,npc* n);

npc* getNpc(gnode* grid,npc* n);

int delNpc(gnode* grid,npc* n);

npc* diedCheckNpc(npc* n);

void setNpcBase(npc* n);

npc* spawnNpc(gnode* grid,int node_id,int isfriend,int type);

int findEnemyBase(int isfriend);

void tickMiscNpc(gnode* grid,npc* n);

void tickDiedCheckNpc(gnode* grid,npc* n);

void tickCleanNpc(gnode* grid,npc* n);

void tickTargetNpc(gnode* grid,npc* n);

void tickAttackNpc(gnode* grid,npc* n);

void tickMoveNpc(gnode* grid,npc* n);

void forEachNpc(gnode* grid, void (process)(gnode*g,npc*n));
