
tower* damageTower(tower* t,bullet* b);

tower* newTower();

int findEnemyBase(int group);

tower* diedCheckTower(tower* t);

void setTowerBase(tower* t);

tower* spawnTower(gnode * grid,int node_id,int owner,int type);
int removeTower(gnode * grid,tower* t);

int tickMiscTower(gnode* grid,tower* t);

int tickDiedCheckTower(gnode* grid,tower* t);

int tickAttackTower(gnode* grid,tower* t);

int tickCleanTower(gnode* grid,tower* t);

int forEachTower(gnode* grid, int (process)(gnode*g,tower*t));

void setTowersMax(int size);
//create array for towers
void allocTowers();
//free array of towers
void realizeTowers();