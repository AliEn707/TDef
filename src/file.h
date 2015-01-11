
void realizeMap(gnode* grid);

gnode * loadMap(char *filepath);
/* map file format
[size]
[build string]
[work string]
max_npc [max npc]
max_tower [max tower]
max_bullets [max bullets]
bases [bases num] [base health]
[base id] [base position] [base spaw point position]
....
pc_base [pc base id]
points [point num]
[point id] [point position]
waves [waves num]
parts [(types num)*((points num)] [delay]
[point id] [type] [num] [delay]
...
*/
//void loadTypes(char* filepath);
int loadNpcTypes();
int loadTowerTypes();
int loadBulletTypes();

void realizeTypes();


