bullet* newBullet();

void tickMiscBullet(gnode * grid,bullet * b);

void tickCleanBullet(gnode * grid,bullet * b);

void tickProcessBullet(gnode * grid,bullet * b);

void forEachBullet(gnode* grid, void (process)(gnode*g,bullet*b));