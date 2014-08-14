bullet* newBullet();

int tickMiscBullet(gnode * grid,bullet * b);

int tickCleanBullet(gnode * grid,bullet * b);

int tickProcessBullet(gnode * grid,bullet * b);

int forEachBullet(gnode* grid, int (process)(gnode*g,bullet*b));