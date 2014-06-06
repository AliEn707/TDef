tower* newTower();

tower* diedCheckTower(tower* t);

void setTowerBase(tower* t);

tower* spawnTower(gnode * grid,int node_id,int owner,int type);

void tickMiscTower(gnode* grid,tower* t);

void tickDiedCheckTower(gnode* grid,tower* t);

void tickAttackTower(gnode* grid,tower* t);

void tickCleanTower(gnode* grid,tower* t);

void forEachTower(gnode* grid, void (process)(gnode*g,tower*t));

