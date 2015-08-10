// get new bullet
bullet* newBullet();
//set mask to 0
int tickMiscBullet(gnode * grid,bullet * b);

int tickDiedCheckBullet(gnode* grid,bullet* b);
// clean up detonated bullets
int tickCleanBullet(gnode * grid,bullet * b);
// move or attack bullet
int tickProcessBullet(gnode * grid,bullet * b);

int forEachBullet(gnode* grid, int (process)(gnode*g,bullet*b));

void setBulletsMax(int size);
//create array for npcs
void allocBullets();
//free array of npcs
void realizeBullets();