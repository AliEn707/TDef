npc* newNpc();

npc* addNpc(gnode* node,npc* n);

npc* getNpc(gnode* grid,npc* n);

int delNpc(gnode* grid,npc* n);

npc* diedCheckNpc(npc* n);

void setNpcBase(npc* n);

npc* spawnNpc(gnode* grid,int node_id,int group,int type);

int findEnemyBase(int group);

int tickMiscNpc(gnode* grid,npc* n);

int tickDiedCheckNpc(gnode* grid,npc* n);

int tickCleanNpc(gnode* grid,npc* n);

int tickTargetNpc(gnode* grid,npc* n);

int tickAttackNpc(gnode* grid,npc* n);

int tickMoveNpc(gnode* grid,npc* n);

int forEachNpc(gnode* grid, int (process)(gnode*g,npc*n));
