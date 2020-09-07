#pragma once
#include <vector>
#include "Renderer.h"
#include "GameObject.h"
#include "AssetManager.h"
#include "SoundEngine.h"
#include <string.h>


class Game {


private:

	static std::uniform_int_distribution<> dmgRand;
	static std::mt19937 rng;
	/*
	   GameLevel struct holds the level data
	   Wave difficulty
	   Tile data
	   Number of coins in chest
	   Pirate spawn point
	   Chest spawn point
	*/
	struct GameLevel
	{
	public:

		GameLevel(int ** w_d, int tiles[10][10], std::vector<glm::vec2>road_tiles_s, glm::vec2 spawn_point , glm::vec2 chest_point);
		GameLevel();

		//level data
		int tilesSizeX;
		int tilesSizeY;
		int ** wave_difficulty;
		int tiles[10][10];
		std::vector<glm::vec2> road_tiles;
		std::vector<glm::vec2> free_tiles;
		int coins = 100;
		
		glm::vec2 spawn_point;
		glm::vec2 chest_point;

		//current state of level
		int current_wave = 0;
		int level_success = false;
		int current_coins = 0;
		int level = 0;

		void print();

		int getTileId(glm::vec2 pos);

	};

	

public:

	Game();
	Game(const char * gameLevel);
	~Game();

	static enum superPowerId
	{
		METEORITE,
		UFO
	};

	int active_super_power = -1;

	//Initializes game objects & game.renderer
	bool Init(int screen_width , int screen_height);

	//Initializes common geometry items ( quads , etc . . ) 
	void InitCommonItems();

	//Loads GameLevel data from specific file
	GameLevel LoadLevel(const char * gameLevelFile);

	//testing variables
	struct pairs
	{
	public:
		GameObject * a;
		GameObject * b;

		pairs(GameObject * a, GameObject * b)
		{
			this->a = a;
			this->b = b;
		}
	public:
		bool isEqual(GameObject * aa, GameObject * bb)
		{
			if (a == aa && this->b == bb || this->a == bb && this->b == aa)
				return true;
			else
				return false;
		}
	};
	std::vector<pairs> Pairs;

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - */
	

	void Start();
	
	void Pause();

	//Updates Game logic and objects before rendering
	void Update(float dt);
	
	void Render();
	
	void End();
	
	bool isRunning();

	//Spawns Pirates
	void spawnPirate(int wave_number);

	void DeletePirates();

	int getBones();

	int getCoins();

private:

	//Spawn Object
	void spawnObject(const int object_id, glm::vec2 pos);

	//Resets Pirate
	void ResetPirate(class GameObject * object);

	//
	void HandleCollision(class GameObject * A, class GameObject * B);

	
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

public:

	//Returns a pointer to game.renderer
	Renderer * getRenderer();

	//Returns a reference to game's roadTiles
	std::vector<class GameObject*> & getRoadTiles();

	//Returns a reference to game's freeTiles
	std::vector<class GameObject*> & getFreeTiles();

	//Returns a reference to game's tiles
	std::vector<class GameObject*> & getTiles();


	std::vector<class GameObject *> & Game::getPirates();


	std::vector<class GameObject *> & Game::getTowers();


	//Sets the focused tile
	void setFocusedObject(class GameObject * object);

	//Returns a pointer to the focused tile
	class GameObject * getFocusedObject();

	//Returns a pointer to game's Terrain
	class GameObject * getTerrain();

	//
	std::vector <class GameObject * > & getChests();

	//
	std::vector <class GameObject * > & getAsteroids();
	
	//Returns a reference to the GameObjects of the game ( pirates , towers , etc..)
	std::vector <class GameObject * > & getObjects();

	bool & Selecting();

	void setSelection(bool enable);

	void cancelSelection();

	void setMovingObject(GameObject * object);

	void removeObject(GameObject * object);

	GameObject * getTile(glm::vec2 tile_pos);

	int getTowerValue();

	bool towerCanMove();

	bool towerCanBuy();

	bool superPowerCanUse(int id);

	void useSuperPower(int id);

	void activateSuperPower(int id);

	bool isActiveSuperPower();


private:


	float m_continuous_time = 0.0f;
	float start_time = 0.0f;
	float end_time = 0.0f;
	float timer = 0.0f;
	float dt_timer = 0.0f;

	//
	bool spawn = true;
	int pirates_count = 0;
	bool pirates_alive = false;
	//

	// game rules //
	int tower_value = 50;
	float tower_move_dt = 0.0f;
	float tower_move_timer = 20.0f;
	bool tower_can_move = true;
	float wave_respawn_dt = 0.0f;
	float wave_respawn_timer = 38.5f;
	int waves[10];
	float waves_difficulty[10];
	int wave_index = 0;
	float pirate_spawn_dt = 0.0f;
	float pirate_spawn_timer = 0.7f;

	float meteorite_sp_dt = 0.0f;
	float meteorite_sp_timer = 70.0f;
	bool meteorite_sp_usable = true;
	float cassini_shoot_timer = 40.0f;
	float cassini_shoot_dt = 0.0f;

	int bones = 175;
	int coins = 100;
	//

	bool pause = false;

	GameLevel gameLevel;

	//GameObjects
	GameObject * terrain;
	std::vector<class GameObject*> tiles;
	std::vector<class GameObject*> road_tiles;
	std::vector<class GameObject*> free_tiles;
	std::vector<class GameObject*> pirates;
	std::vector<class GameObject*> towers;
	std::vector<class GameObject*> chests;
	std::vector<class GameObject*> asteroids;
	int remove_asteroid = -1;
	std::vector<class GameObject*> stars;
	GameObject * cassini = 0;
	GameObject * mercury = 0;
	

	class GameObject * focused_object = nullptr;
	std::vector<class GameObject*> gameObjects; //skellys & towers

	//state of game
	bool alive = true;

	bool selecting = false;

	GameObject * moving_tower = nullptr;

	//game's renderer
	Renderer * renderer;

	
	public:
		void showInfo()
		{
			GameObject::setInfo();
		}

};
