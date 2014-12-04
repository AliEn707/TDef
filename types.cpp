#include <map>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib> 


/*
╔══════════════════════════════════════════════════════════════╗
║ 										                       ║
║ created by Dennis Yarikov						                       ║
║ dec 2014									                       ║
╚══════════════════════════════════════════════════════════════╝
*/


using namespace std;

typedef map <int, void*>::iterator it;

map <int, void*> npc_types_map; 
//npc
extern "C" {
	
	int typesNpcAdd(int _key, void* value){
		if (npc_types_map.find(_key) == npc_types_map.end()) {
			npc_types_map[_key] = value;
			return 0;
		}
		return 1;
	}
	
	void * typesNpcGet(int _key){
		if (npc_types_map.find(_key) != npc_types_map.end()) {
			return npc_types_map[_key];
		}
		return 0;
	}
	
	int typesNpcForEach(void (function)(void*v)) {
//		cout << "map size before"<<npc_types_map.size()<< endl;
		for (it i = npc_types_map.begin(); i != npc_types_map.end(); i++)
			function(i->second);
//		cout << "map size "<<npc_types_map.size()<< endl;
		return 0;
	}
	
	void typesNpcClear(){
		typesNpcForEach(free);
		npc_types_map.clear();
	}

}


map <int, void*> tower_types_map; 
//tower
extern "C" {
	
	int typesTowerAdd(int _key, void* value){
		if (tower_types_map.find(_key) == tower_types_map.end()) {
			tower_types_map[_key] = value;
			return 0;
		}
		return 1;
	}
	
	void * typesTowerGet(int _key){
		if (tower_types_map.find(_key) != tower_types_map.end()) {
			return tower_types_map[_key];
		}
		return 0;
	}
	
	int typesTowerForEach(void (function)(void*v)) {
//		cout << "map size before"<<tower_types_map.size()<< endl;
		for (it i = tower_types_map.begin(); i != tower_types_map.end(); i++)
			function(i->second);
//		cout << "map size "<<tower_types_map.size()<< endl;
		return 0;
	}
	
	void typesTowerClear(){
		typesTowerForEach(free);
		tower_types_map.clear();
	}

}

map <int, void*> bullet_types_map; 
//Bullet
extern "C" {
	
	int typesBulletAdd(int _key, void* value){
		if (bullet_types_map.find(_key) == bullet_types_map.end()) {
			bullet_types_map[_key] = value;
			return 0;
		}
		return 1;
	}
	
	void * typesBulletGet(int _key){
		if (bullet_types_map.find(_key) != bullet_types_map.end()) {
			return bullet_types_map[_key];
		}
		return 0;
	}
	
	int typesBulletForEach(void (function)(void*v)) {
//		cout << "map size before"<<bullet_types_map.size()<< endl;
		for (it i = bullet_types_map.begin(); i != bullet_types_map.end(); i++)
			function(i->second);
//		cout << "map size "<<bullet_types_map.size()<< endl;
		return 0;
	}
	
	void typesBulletClear(){
		typesBulletForEach(free);
		bullet_types_map.clear();
	}

}

map <int, void*> splash_types_map; 
//Splash
extern "C" {
	
	int typesSplashAdd(int _key, void* value){
		if (splash_types_map.find(_key) == splash_types_map.end()) {
			splash_types_map[_key] = value;
			return 0;
		}
		return 1;
	}
	
	void * typesSplashGet(int _key){
		if (splash_types_map.find(_key) != splash_types_map.end()) {
			return splash_types_map[_key];
		}
		return 0;
	}
	
	int typesSplashForEach(void (function)(void*v)) {
//		cout << "map size before"<<splash_types_map.size()<< endl;
		for (it i = splash_types_map.begin(); i != splash_types_map.end(); i++)
			function(i->second);
//		cout << "map size "<<splash_types_map.size()<< endl;
		return 0;
	}
	
	void typesSplashClear(){
		typesSplashForEach(free);
		splash_types_map.clear();
	}

}



