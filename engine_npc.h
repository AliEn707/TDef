// set damage to npc
npc* damageNpc(npc* n,bullet* b);
//get new npc
npc* newNpc();
//add npc to gnode list
npc* addNpc(gnode* node,npc* n);
//get npc from gnode list
npc* getNpc(gnode* grid,npc* n);
// delete npc !not used!
int delNpc(gnode* grid,npc* n);
// check npc health
npc* diedCheckNpc(npc* n);
// set basic parameters
void setNpcBase(npc* n);
// add npc to world
npc* spawnNpc(gnode* grid,int node_id,int group,int type);

int findEnemyBase(int group);

int tickMiscNpc(gnode* grid,npc* n);

int tickDiedCheckNpc(gnode* grid,npc* n);

int tickCleanNpc(gnode* grid,npc* n);
//find target
int tickTargetNpc(gnode* grid,npc* n);
//attack target if in attack
int tickAttackNpc(gnode* grid,npc* n);

int tickMoveNpc(gnode* grid,npc* n);

int forEachNpc(gnode* grid, int (process)(gnode*g,npc*n));
