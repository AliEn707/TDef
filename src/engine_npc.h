// set damage to npc
npc* damageNpc(npc* n,bullet* b);
// check npc health
npc* diedCheckNpc(npc* n);
// set basic parameters
void setNpcBase(npc* n);
// add npc to world
npc* spawnNpc(gnode* grid,int node_id,int owner,int type);

int tickMiscNpc(gnode* grid,npc* n);

int tickDiedCheckNpc(gnode* grid,npc* n);

int tickCleanNpc(gnode* grid,npc* n);
//find target
int tickTargetNpc(gnode* grid,npc* n);
//attack target if in attack
int tickAttackNpc(gnode* grid,npc* n);

int tickMoveNpc(gnode* grid,npc* n);

int forEachNpc(gnode* grid, int (process)(gnode*g,npc*n));
int forEachNpcRemove(gnode* grid, int (process)(gnode*g,npc*n));

npc* getNpcById(int id);

int setNpcTargetByNode(gnode * grid,npc* n, int node);

void setNpcsMax(int size);
//create array for npcs
void allocNpcs();
//free array of npcs
void realizeNpcs();