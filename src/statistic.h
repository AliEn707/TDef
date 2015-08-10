void statisticsPrint();

void statisticsInit();
void statisticsClear();

void statisticAddLevel(int player);
void statisticAddNpcSpawned(int player,int type);
void statisticAddNpcKilled(int player,int type, int level);
void statisticAddNpcLost(int player,int type, int level);
void statisticAddTowerBuilt(int player,int type);
void statisticAddTowerDestroed(int player,int type, int level);
void statisticAddTowerLost(int player,int type, int level);