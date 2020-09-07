#include "Game.h"
#include <fstream>
#include "OBJLoader.h"
#include "Game_objects.h"
#include <iostream>
#include <chrono>
#include <random>

std::uniform_int_distribution<> Game::dmgRand(100 , 180);
std::mt19937 Game::rng;

/*constructors - destructors*/
Game::GameLevel::GameLevel()
{
	
}

Game::GameLevel::GameLevel(int ** w_d, int s_tiles[10][10], std::vector<glm::vec2> road_tiles_s , glm::vec2 spawn_point , glm::vec2 chest_point)
	:wave_difficulty(w_d) , spawn_point(spawn_point) , chest_point(chest_point)
{
	
	tilesSizeX = 10;
	tilesSizeY = 10;
	memcpy(&tiles[0][0], &s_tiles[0][0], 100*sizeof(int));
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			//std::cout << "tiles[" << i << "][" << j << "] = " << tiles[i][j] << std::endl;
			if (tiles[i][j] == 0)
			{
				free_tiles.push_back(glm::vec2(i, j));
			}
		}
	}
	road_tiles = road_tiles_s;
}

int Game::GameLevel::getTileId(glm::vec2 pos)
{
	for (int i = 0; i < road_tiles.size(); i++)
	{
		if (road_tiles[i] == pos)
		{
			return i;
		}
	}
}

Game::Game()
{
	
	//const char * gamelevel_file = "../Data/GameLevels/TestLevel.level";
	//gameLevel = LoadLevel(gamelevel_file);
	
	renderer = new Renderer(this);

}

Game::Game(const char * gameLevelFile)
{
	

	gameLevel = LoadLevel(gameLevelFile);
	renderer = new Renderer(this);
}

Game::~Game()
{
	
	delete renderer;
	AssetManager::clearData();
}
/* */


//Loads GameLevel data from specific file
Game::GameLevel Game::LoadLevel(const char * gameLevelFile)
{
	std::ifstream file(gameLevelFile);
	std::string str;

	bool reading_tiles = false;
	bool first_encounter = true;
	glm::vec2 spawn_point;
	glm::vec2 chest_point;
	int tiles[10][10];
	std::vector<glm::vec2> road_tiles;

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			tiles[i][j] = 0;
		}
	}


	while (std::getline(file, str))
	{

		char * line = new char[strlen(str.c_str())];
		strcpy(line, str.c_str());

		//printf("\n line : ");
		char * tokens = strtok(line, ",");

		while (tokens)
		{

			//printf(" %s ,", tokens);
			if (strcmp(tokens, "tiles{") == 0)
			{
				reading_tiles = true;
			}

			//printf(" %s \n", tokens);
			else if (reading_tiles && strcmp(tokens, "}") != 0)
			{
				int x = atoi(tokens);
				tokens = strtok(NULL, ",");
				int y = atoi(tokens);

				if(x >= 0 && x < 10 && y >= 0 && y < 10)
					tiles[x][y] = 1;
				road_tiles.push_back(glm::vec2(x, y));
				chest_point.x = x; chest_point.y = y;

				if (first_encounter)
				{
					spawn_point = glm::vec2(x, y);
					first_encounter = false;
				}
			}
			else
			{
				reading_tiles = false;
			}

			tokens = strtok(NULL, ",");

		}
	}
	

	GameLevel level(nullptr, tiles , road_tiles , spawn_point , chest_point);
	return level;
}


/*
  Initializing Renderer object plus all the resources needed
  to construct current level.(Tiles , chests , etc..)
*/
bool Game::Init(int screen_width , int screen_height)
{
	
	// initializing gameLevel
	gameLevel = LoadLevel("../Data/GameLevels/GameLevel0.level");
	for (int i = 0; i < 10; i++) // initialize wave's pirate numbers && wave's pirate's health points
	{
		waves[i] = 10 + i * 2;
		waves_difficulty[i] = 200 + i * 50; //10
	}

	// Loading Assets into memory
	bool load = AssetManager::loadAssets() && AssetManager::loadTextures();

	// Sound Engine
	SoundEngine::InitEngine();

	// AnimationHandler
	AnimationHandler::Init();

	// ParticleSystem
	ParticleSystem::Init();

	// Initializing Terrain
	terrain = new Terrain(AssetManager::GetIndex("terrain"));
	terrain->Update(0.0f);

	// initializing Tiles
	Tile * tile = nullptr;
	
	for (int i = 0; i < gameLevel.road_tiles.size(); i++)
	{
		tile = new Tile(AssetManager::GetIndex("road") , GameObject::ROAD_TILE);
		tile->setTilePosition(glm::vec2(gameLevel.road_tiles[i]));
		tile->setWorldCenterPosition(terrain->getWorldPosition());
		tile->setWorldPositionBasedOnTile();
		tile->Update(0.0f);
		road_tiles.push_back(tile);
		tiles.push_back(tile);
	}
	for (int i = 0; i < gameLevel.free_tiles.size(); i++)
	{
		tile = new Tile( nullptr , GameObject::FREE_TILE);
		tile->setTilePosition(glm::vec2(gameLevel.free_tiles[i]));
		tile->setWorldCenterPosition(terrain->getWorldPosition());
		tile->setWorldPositionBasedOnTile();
		tile->Update(0.0f);
		free_tiles.push_back(tile);
		tiles.push_back(tile);

		// std::cout << "free_tile[" << i << "] : [ " << tile->getTilePosition().x << " , " << tile->getTilePosition().y << " ] " << std::endl;
	}
	//

	
	// init chest
	spawnObject(GameObject::CHEST, gameLevel.road_tiles[gameLevel.road_tiles.size()-1]);
	chests[chests.size() - 1]->setRotation(55.0f);
	chests[chests.size() - 1]->Update(0.0f);

	spawnObject(GameObject::CHEST, gameLevel.road_tiles[gameLevel.road_tiles.size() - 1]);
	chests[chests.size() - 1]->setRotation(-55.0f);
	chests[chests.size() - 1]->Update(0.0f);

	// init asteroids
	for (int i = 0; i < 10; i++)
	{
		spawnObject(GameObject::ASTEROID, glm::vec2(i));
	}

	spawnObject(GameObject::CASSINI, glm::vec2(0));

#ifdef DEBUGGING_TOWERS

	for (int i = 0; i < free_tiles.size(); i++)
	{
		spawnObject(GameObject::TOWER, free_tiles[i]->getTilePosition());
	}

#endif

	
	SoundEngine::PlaySound("theme", true);

	//initializing renderer
	bool state = renderer->Init(screen_width, screen_height);

	return state & load;
}


//Spawns a pirate
void Game::spawnPirate(int number)
{
	spawnObject(GameObject::PIRATE, gameLevel.spawn_point);

	//spawnObject(GameObject::ASTEROID, glm::vec2(2));
}


//Spawns an object on given position
void Game::spawnObject(const int object_id, glm::vec2 pos)
{
	//GameObject * object = new GameObject(object_id);
	switch (object_id)
	{
	case GameObject::PIRATE:
	{

		std::string Mesh_array[4] = { "pirate_body" , "pirate_arm" , "pirate_left_foot" , "pirate_right_foot" };
		GameObject * pirate = new Pirate(AssetManager::GetArray(Mesh_array, 4));

		pirate->setHealthPoints(waves_difficulty[wave_index]);
		pirate->setTiles(&gameLevel.road_tiles);
		pirate->setTilePosition(pos);
		pirate->setSpawnPoint(pos);
		pirate->setWorldCenterPosition(terrain->getWorldPosition());
		pirate->setWorldPositionBasedOnTile();
		pirate->getTileId() = gameLevel.getTileId(pos);
		int TileId = pirate->getTileId();

		//pirate->setTargetPosition(road_tiles[TileId+1]->getWorldPosition());
		pirate->setTargetTilePosition(gameLevel.road_tiles[TileId + 1]);
		pirate->setDirection(pirate->computeDirection(gameLevel.road_tiles[TileId + 1]));
		pirate->computeRotation();

		glm::vec2 target_tile_pos = gameLevel.road_tiles[TileId + 1];

		/*
		//glm::vec2 current_tile_pos = pos;
		glm::vec3 direction;

		if (target_tile_pos.y > current_tile_pos.y)
			direction.z = +1.0f;
		else if (target_tile_pos.y < current_tile_pos.y)
			direction.z = -1.0f;
		else
			direction.z = 0.0f;

		if (target_tile_pos.x > current_tile_pos.x)
			direction.x = +1.0f;
		else if (target_tile_pos.x < current_tile_pos.x)
			direction.x = -1.0f;
		else
			direction.x = 0.0f;
		*/
		//pirate->setDirection(direction);


		//pirate->setDirection(pirate->computeDirection(target_tile_pos));
		//pirate->computeRotation();
		pirates.push_back(pirate);
		SoundEngine::PlaySound("portal_3", false);
		ParticleSystem::addParticleEffect(pirate->getWorldPosition(), ParticleEffect::swirl);

		break;
	}
	case GameObject::TOWER:
	{
		GameObject * tower = new Tower(AssetManager::GetIndex("tower"), AssetManager::GetIndex("cannon_ball"));
		tower->setTilePosition(pos);
		tower->setWorldCenterPosition(terrain->getWorldPosition());
		tower->setWorldPositionBasedOnTile();
		tower->getTileId() = gameLevel.getTileId(pos);
		tower->Update(0.0f);
		towers.push_back(tower);
		break;
	}

	case GameObject::CHEST:
	{
		GameObject * chest = new Chest(AssetManager::GetIndex("chest"));
		chest->setTilePosition(pos);
		chest->setWorldCenterPosition(terrain->getWorldPosition());
		chest->setWorldPositionBasedOnTile();
		chest->getTileId() = gameLevel.getTileId(pos);
		chests.push_back(chest);
		break;

	}

	case GameObject::ASTEROID:
	{
		std::mt19937 rng;
		rng.seed(std::random_device()());
		std::uniform_real_distribution<> dist(-60, 60);
		std::uniform_real_distribution<> distY1(-50, -45);
		std::uniform_real_distribution<> distY2(20, 50);

		std::uniform_real_distribution<> dist2(-0.8, 0.8); // -1 , 1
		std::uniform_real_distribution<> scale(2, 8.0f);
		std::uniform_real_distribution<> vel(0.01, 0.3);
		std::uniform_int_distribution<> dist3(1, 3);
		std::uniform_int_distribution<> rand(0, 1);

		int rng_asteroid = dist3(rng);
		char * asteroid_mesh;
		if (rng_asteroid == 1)
			asteroid_mesh = "asteroid01";
		else if (rng_asteroid == 2)
			asteroid_mesh = "asteroid02";
		else
			asteroid_mesh = "asteroid03";


#define asteroids_project
		//    #define asteroids_test

#ifdef asteroids_project

		GameObject * asteroid = new Asteroid(AssetManager::GetIndex(asteroid_mesh));


#ifdef SCRIPTED_ASTEROID

		if (pos == glm::vec2(2))
		{
			asteroid->setWorldPosition(terrain->getWorldPosition() + glm::vec3(-12 , 30 , 0));
			asteroid->setDirection(glm::vec3(0 , -1 , -1));
			asteroid->adjustVelocity(0.2);
			asteroid->getScaleFactor() = (scale(rng));
			asteroid->Update(0.0f);
			asteroids.push_back(asteroid);
			return;
		}
#endif

		float rngY;

		if (rand(rng) == 0)
			rngY = distY1(rng);
		else
			rngY = distY2(rng);

		asteroid->setWorldPosition(terrain->getWorldPosition() + glm::vec3(dist(rng), rngY, dist(rng)));
		asteroid->setDirection(glm::vec3(dist2(rng), dist2(rng), dist2(rng)));
		asteroid->adjustVelocity(vel(rng));
		asteroid->getScaleFactor() = (scale(rng));
		asteroid->Update(0.0f);
		

		asteroids.push_back(asteroid);

#endif

#ifdef asteroids_test


		/* plane collision
		GameObject * asteroid3 = new Asteroid(AssetManager[asteroid_mesh]);
		asteroid3->setWorldPosition(terrain->getWorldPosition() + glm::vec3(10, 10, 5));
		asteroid3->setDirection(glm::vec3(-1, 0, 0));
		asteroid3->Update(0.0f);
		asteroid3->adjustVelocity((dist2(rng) + 2.0f) / 30.0f);
		asteroid3->getScaleFactor() = (dist2(rng) + 6.0f);
		asteroids.push_back(asteroid3);
		/* */

		/* terrain collision
		GameObject * asteroid2 = new Asteroid(AssetManager[asteroid_mesh]);
		asteroid2->setWorldPosition(terrain->getWorldPosition() + glm::vec3(-30, -30, 10));
		asteroid2->setDirection(glm::vec3( 1, 0, 0));
		asteroid2->Update(0.0f);
		asteroid2->adjustVelocity((dist2(rng) + 2.0f) / 30.0f);
		asteroid2->getScaleFactor() = (dist2(rng) + 5.0f);
		asteroids.push_back(asteroid2);
		/**/

		/* left right
		GameObject * asteroid2 = new Asteroid(AssetManager[asteroid_mesh]);
		asteroid2->setWorldPosition(terrain->getWorldPosition() + glm::vec3(-30, 10, 5));
		asteroid2->setDirection(glm::vec3(1, 0, 0));
		asteroid2->Update(0.0f);
		asteroid2->adjustVelocity((dist2(rng) + 2.0f) / 30.0f);
		asteroid2->getScaleFactor() = (dist2(rng) + 6.0f);
		asteroids.push_back(asteroid2);

		GameObject * asteroid3 = new Asteroid(AssetManager[asteroid_mesh]);
		asteroid3->setWorldPosition(terrain->getWorldPosition() + glm::vec3(10, 10, 5));
		asteroid3->setDirection(glm::vec3(-1, 0, 0));
		asteroid3->Update(0.0f);
		asteroid3->adjustVelocity((dist2(rng) + 2.0f) / 30.0f);
		asteroid3->getScaleFactor() = (dist2(rng) + 6.0f);
		asteroids.push_back(asteroid3);
		/* */


		/* up down
		GameObject * asteroid2 = new Asteroid(AssetManager[asteroid_mesh]);
		asteroid2->setWorldPosition(terrain->getWorldPosition() + glm::vec3(10, 30, 5));
		asteroid2->setDirection(glm::vec3(0, -1, 0));
		asteroid2->Update(0.0f);
		asteroid2->adjustVelocity((dist2(rng) + 2.0f) / 30.0f);
		asteroid2->getScaleFactor() = (dist2(rng) + 6.0f);
		asteroids.push_back(asteroid2);

		GameObject * asteroid3 = new Asteroid(AssetManager[asteroid_mesh]);
		asteroid3->setWorldPosition(terrain->getWorldPosition() + glm::vec3(10, 5, 5));
		asteroid3->setDirection(glm::vec3(0, -1, 0));
		asteroid3->Update(0.0f);
		asteroid3->adjustVelocity((dist2(rng) + 2.0f) / 30.0f);
		asteroid3->getScaleFactor() = (dist2(rng) + 6.0f);
		asteroids.push_back(asteroid3);
		/* */
#endif

		break;
	}

	case GameObject::PLANET:
	{
		GameObject * asteroid = new Asteroid(AssetManager::GetIndex("moon") , GameObject::PLANET);

		asteroid->setWorldPosition(terrain->getWorldPosition() + glm::vec3(0, 20, 0));
		asteroid->setDirection(glm::vec3(0.1, 1, 0.1));
		asteroid->adjustVelocity(1.f);
		asteroid->getScaleFactor() = 0.04f;
		asteroid->Update(0.0f);
		asteroids.push_back(asteroid);
		break;
	}

	case GameObject::CASSINI:
	{
		GameObject * asteroid = new Asteroid(AssetManager::GetIndex("cassini"), GameObject::CASSINI);

		asteroid->setWorldPosition(terrain->getWorldPosition() + glm::vec3(0, 20, 0));
		asteroid->setDirection(glm::vec3(0.1, 1, 0.1));
		asteroid->adjustVelocity(1.f);
		asteroid->getScaleFactor() = 3.f;
		asteroid->Update(0.0f);
		asteroids.push_back(asteroid);
		cassini = asteroid;
		break;
	}

	}

	
}


//Resets pirate's position
void Game::ResetPirate(class GameObject * pirate)

{
	
	pirate->setTilePosition(gameLevel.road_tiles[0]);
	pirate->setWorldPositionBasedOnTile();
	pirate->getTileId() = 0;
	pirate->setTargetPosition(road_tiles[1]->getWorldPosition());
	pirate->setTargetTilePosition(gameLevel.road_tiles[1]);

	glm::vec2 current_tile_pos = pirate->getTilePosition();
	glm::vec2 target_tile_pos = gameLevel.road_tiles[1];

	pirate->setSpawnPoint(gameLevel.spawn_point);
	pirate->setDirection(pirate->computeDirection(target_tile_pos));
}


//Starts game logic + rendering.
void Game::Start()
{

	

}


//Updates game's logic & Objects before rendering
void Game::Update(float dt)
{

#ifdef GAME_RULES
	if (waves[wave_index] > 0)
	{
		if (pirate_spawn_dt > pirate_spawn_timer)
		{
			spawnObject(GameObject::PIRATE, gameLevel.spawn_point);
			pirate_spawn_dt = 0.0f;
			waves[wave_index]--;
		}
		else
			pirate_spawn_dt += dt;
	}
#endif

	if (pirates_alive && dt_timer > 0.38)
	{
		SoundEngine::PlaySound("footsteps", false);
		dt_timer = 0.0f;
	}
	else
		dt_timer += dt;
	

	if (pause)
	{
		timer += dt;
		renderer->Update(dt);
		return;
	}
	

	/*
	(terrain->Update(dt);
	*/

	// update road tiles
	/*
	for (int i = 0; i < road_tiles.size(); i++)
	{
		//road_tiles[i]->Update(dt);
	}
	*/

	// Update ~ Remove dead pirates
	pirates_alive = false;
	for (int i = 0; i < pirates.size(); i++)
	{
		if (pirates[i]->isAlive())
			pirates_alive = true;

#ifdef GAME_RULES
		if (pirates[i]->reachedEndPoint())
		{
			coins -= 10;
			delete pirates[i];
			pirates[i] = 0;
			pirates[i] = pirates.back();
			pirates.pop_back();
		}
#endif
		else if (!pirates[i]->isAlive())
		{

#ifdef GAME_RULES
		
			if (!pirates[i]->collectedBones())
				bones += pirates[i]->collectBones();
#endif
			
			if (pirates[i]->get_decompose_timer() > 2.0f || pirates[i]->reachedEndPoint())
			{
				delete pirates[i];
				pirates[i] = 0;
				pirates[i] = pirates.back();
				pirates.pop_back();

			}
		}
		
	}


#ifdef CASSINI_SHOOTER
	int lucky_pirate = -1;
	if (cassini_shoot_dt > cassini_shoot_timer)
	{
		std::uniform_int_distribution<> pirateRand(0, pirates.size() - 1);
		rng.seed(std::random_device()());
		lucky_pirate = pirateRand(rng);
	}
#endif

	// Update Pirates
	for (int i = 0; i < pirates.size(); i++)
	{
		
		pirates[i]->Update(dt);
		//Check for collisions between them
		
		bool collision = false;
		for (int j = 0; j < pirates.size(); j++)
		{

			if (i == j || pirates[i]->isDecomposing() || pirates[j]->isDecomposing())
				continue;

			if (pirates[i]->hasCollisionWith(pirates[j]))
			{ 
				HandleCollision(pirates[i], pirates[j]);
				collision = true;
			}

		}

		if (!collision)
		{
			pirates[i]->getCollision() = false;
		}
		
#ifdef CASSINI_SHOOTER
		if (cassini_shoot_dt > cassini_shoot_timer && lucky_pirate == i)
		{
			cassini->shootAt(pirates[i]);
			cassini_shoot_dt = 0.0f;
		}
#endif

	}


	// Update towers
	for (int i = 0; i < towers.size(); i++)
	{
		glm::vec3 tower_pos = towers[i]->getWorldPosition();

		if (towers[i]->canShoot())
		{
			
			float min = 16;
			GameObject * closest_pirate = nullptr;
			for (int j = 0; j < pirates.size(); j++)
			{
				if (pirates[j]->isDecomposing())
					continue;

				/*
				glm::vec2 pos = pirates[j]->getTilePosition();
				pos -= towers[i]->getTilePosition();
				if (glm::length(pos) < min)
				{
					min = glm::length(pos);
					closest_pirate = pirates[j];
				}
				*/

				glm::vec3 pirate_tile_pos = pirates[j]->getInterpolatedPosition();
				glm::vec3 pos_diff = glm::abs(pirate_tile_pos - tower_pos);

				if (pos_diff.x <= 12 && pos_diff.z <= 12)
				{
					if (glm::length(pos_diff) < min)
					{
						min = glm::length(pos_diff);
						closest_pirate = pirates[j];
					}
				}
			}
			if (closest_pirate != nullptr)
				towers[i]->shootAt(closest_pirate);



		}
		
		//towers[i]->Update(dt);
		towers[i]->UpdateParts(dt);
	}
	

	// Update asteroids
	if (remove_asteroid != -1)
	{
		int index = remove_asteroid;
		delete asteroids[index];
		asteroids[index] = 0;
		asteroids[index] = asteroids.back();
		asteroids.pop_back();

		remove_asteroid = -1;
	}
	for (int i = 0; i < asteroids.size(); i++)
	{

		/*
		if (asteroids[i]->getObjectID() == GameObject::CASSINI && asteroids[i]->canShoot())
		{
			for (int p = 0; p < pirates.size(); p++)
			{
				if (!pirates[p]->isAlive() || pirates[p]->isDecomposing())
					continue;
				asteroids[i]->shootAt(pirates[p]);
				break;
			}
		}
		*/

		for (int j = 0; j < asteroids.size(); j++)
		{
			if (i == j)
				continue;
			if (asteroids[i]->hasCollisionWith(asteroids[j]))
				HandleCollision(asteroids[i], asteroids[j]);


		}

		

		asteroids[i]->hasCollisionWith(glm::vec3(100, 120, 100));
	
		for (int j = 0; j < road_tiles.size(); j++)
		{
			

			if (asteroids[i]->hasCollisionWith(road_tiles[j]))
			{
				SoundEngine::PlaySound("expl", false);

				glm::vec3 pos;
				bool col = false;

				for (int p = 0; p < pirates.size(); p++)
				{

					glm::vec3 diff = abs(pirates[p]->getInterpolatedPosition() - asteroids[i]->getInterpolatedPosition());
					float tresh;
					float dmg;
					if (!asteroids[i]->amplifyDamage())
					{
						tresh = 4.0f;
						dmg = 500.0f;
					}
					else
					{
						tresh = 8.0f;
						dmg = 150.0f;
					}

					if (diff.x <= tresh && diff.z <= tresh)
					{
						pos = pirates[p]->getInterpolatedPosition();
						rng.seed(std::random_device()());
						pirates[p]->ReceiveDamage( dmgRand(rng), asteroids[i]);
						//pirates[p]->deCompose(asteroids[i]->getDirection());
						col = true;
					}
				}

				SoundEngine::PlaySound("expl_double", false);
				if (col)
				{
					ParticleSystem::addParticleEffect(pos + glm::vec3(0, 0, 0), ParticleEffect::emit);
					if (asteroids[i]->removableAfterCollision())
						remove_asteroid = i;
				}

			}
		}

		if (asteroids[i]->hasCollisionWith(terrain))
			SoundEngine::PlaySound("expl", false);


		asteroids[i]->Update(dt);
	}




	

	
#ifdef GAME_RULES
	if (wave_respawn_dt > wave_respawn_timer + wave_index * 1.0f)
	{
		wave_index++;
		wave_respawn_dt = 0.0f;
	}
	else
		wave_respawn_dt += dt;

	if (!tower_can_move && tower_move_dt < tower_move_timer)
		tower_move_dt += dt;
	else if (!tower_can_move && tower_move_dt >= tower_move_timer)
	{
		tower_can_move = true;
	}
#endif

	if (!meteorite_sp_usable && meteorite_sp_dt < meteorite_sp_timer)
		meteorite_sp_dt += dt;
	else if (!meteorite_sp_usable && meteorite_sp_dt >= meteorite_sp_timer)
		meteorite_sp_usable = true;
	//

	cassini_shoot_dt += dt;

	//update game's renderer
	renderer->Update(dt);

	m_continuous_time += dt;
	timer += dt;	

}


void Game::HandleCollision(GameObject * A, GameObject * B)
{

	glm::vec3 direction_A = A->getDirection();
	glm::vec3 direction_B = B->getDirection();


	glm::vec3 interpolated_position_A = A->getInterpolatedPosition();
	glm::vec3 interpolated_position_B = B->getInterpolatedPosition();

	int tileIdA = A->getTileId();
	int tileIdB = B->getTileId();

	glm::vec3 world_position_A = A->getWorldPosition();
	glm::vec3 world_position_B = B->getWorldPosition();

	float VELOCITY_A = A->getVelocity();
	float VELOCITY_B = B->getVelocity();


	// pirates collision
	if (A->getObjectID() == GameObject::PIRATE && B->getObjectID() == GameObject::PIRATE)
	{

		glm::vec3 dir;

		if (A->isPushBack())
			dir = direction_B;
		else
			dir = direction_A;

		glm::vec3 pos_diff1 = (interpolated_position_A - interpolated_position_B)*dir; // * direction
		if (glm::length(glm::abs(pos_diff1)) < 3.5f)
		{
			if (direction_A == -1.0f * direction_B) // objects collide with opposite directions
			{
				glm::vec3 pos_diff = (interpolated_position_A - interpolated_position_B)*dir;
				
				{
					/*
					std::cout << "\n ______Collision < Pirate(" << A->object_index << ") - Pirate(" << B->object_index << ") >______" << std::endl;
					std::cout << "        o pirate(" << A->object_index << ") pos : ( " << interpolated_position_A.x << " , " << interpolated_position_A.y << " , " << interpolated_position_A.z << " ) " << std::endl;
					std::cout << "        o pirate(" << A->object_index << ") isPushBack() : " << A->isPushBack() << std::endl;
					std::cout << "        o pirate(" << A->object_index << ") isResetting() : " << A->isResetting() << std::endl;

					std::cout << "\n        o pirate(" << B->object_index << ") pos : ( " << interpolated_position_B.x << " , " << interpolated_position_B.y << " , " << interpolated_position_B.z << " ) " << std::endl;
					std::cout << "        o pirate(" << B->object_index << ") isPushBack() : " << B->isPushBack() << std::endl;
					std::cout << "        o pirate(" << B->object_index << ") isResetting() : " << B->isResetting() << std::endl;
					std::cout << " _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _" << std::endl;
					*/
				}
				if (pos_diff.x > 0 || pos_diff.z > 0 && A->isPushBack()) // A ahead of B
				{

					/*
						std::cout << "A:\n pirate(" << A->object_index << ") ahead of pirate(" << B->object_index << ") : trying push back pirate(" << B->object_index << ")  " << std::endl;
						std::cout << " pirate(" << A->object_index << ") pos : ( " << interpolated_position_A.x << " , " << interpolated_position_A.y << " , " << interpolated_position_A.z << " ) " << std::endl;
						std::cout << " pirate(" << B->object_index << ") pos : ( " << interpolated_position_B.x << " , " << interpolated_position_B.y << " , " << interpolated_position_B.z << " ) " << std::endl;
						std::cout << " posA - posB : ( " << pos_diff.x << " , " << pos_diff.y << " , " << pos_diff.z << " ) " << std::endl;
						*/

					if (A->isResetting() || (A->isPushBack() && B->isPushBack()) || B->isDazzed() )
					{
						B->getCollision() = true;
						A->ResetMovement(false); 

						//A->setCollision(true);
						//A->adjustVelocity(-1.0f);
					}
					else if (B->tryPushBack(direction_B*-1.0f, A->getVelocity()))
					{

						//std::cout << "     pirate(" << B->object_index << ") : pushing back " << std::endl;
						//std::cout << "     pirate(" << A->object_index << ") : resetting[true] " << std::endl;

						//A->getCollision() = true;
						A->ResetMovement(true);
						//B->getCollision() = true;
					}
					else
					{
						//std::cout << "     pirate(" << B->object_index << ") : can't push back" << std::endl;
						//std::cout << "     pirate(" << A->object_index << ") : resetting[false]" << std::endl;


					
						//A->adjustVelocity(-75.0f);


						// B->ResetMovement(false);
						// B->adjustVelocity(-0.5f);
						A->ResetMovement(false);
						B->getCollision() = true;

					}


				}
				
				else if (pos_diff.x < 0 || pos_diff.z < 0 && B->isPushBack()) // A ahead of B
				{

					/*
						std::cout << "A:\n pirate(" << A->object_index << ") ahead of pirate(" << B->object_index << ") : trying push back pirate(" << B->object_index << ")  " << std::endl;
						std::cout << " pirate(" << A->object_index << ") pos : ( " << interpolated_position_A.x << " , " << interpolated_position_A.y << " , " << interpolated_position_A.z << " ) " << std::endl;
						std::cout << " pirate(" << B->object_index << ") pos : ( " << interpolated_position_B.x << " , " << interpolated_position_B.y << " , " << interpolated_position_B.z << " ) " << std::endl;
						std::cout << " posA - posB : ( " << pos_diff.x << " , " << pos_diff.y << " , " << pos_diff.z << " ) " << std::endl;
						*/

					if (B->isResetting() || (B->isPushBack() && A->isPushBack()) || A->isDazzed())
					{

						A->getCollision() = true;
						B->ResetMovement(false);

						//B->setCollision(true);//B->getCollision() = true;
						//B->adjustVelocity(-1.f);
					}
					else if (A->tryPushBack(direction_A*-1.0f, B->getVelocity()))
					{

						//std::cout << "     pirate(" << B->object_index << ") : pushing back " << std::endl;
						//std::cout << "     pirate(" << A->object_index << ") : resetting[true] " << std::endl;
						//A->getCollision() = true;
						B->ResetMovement(true);
						//B->getCollision() = true;
						
					}
					else
					{
						//std::cout << "     pirate(" << B->object_index << ") : can't push back" << std::endl;
						//std::cout << "     pirate(" << A->object_index << ") : resetting[false]" << std::endl;


						
						//B->adjustVelocity(-0.75f);


						// B->ResetMovement(false);
						// B->adjustVelocity(-0.5f);
						B->ResetMovement(false);
						A->getCollision() = true;
						//B->setCollision(true);
						//A->setCollision(true);//A->getCollision() = true;

					}


				}

			}


			//////////////////////////////////////////////////
			{

				if (pos_diff1.x > 0 || pos_diff1.z > 0)
				{
					if (A->isDazzed() && !B->isPushBack())
					{
						B->adjustVelocity(-B->getVelocity() / 2.0f);
						B->getCollision() = true;
					}

					if (!B->isPushBack() && !A->isPushBack())
					{

						B->adjustVelocity(-B->getVelocity() / 8.f);
						B->getCollision() = true;
						A->getCollision() = false;
						//A->getCollision() = false; //EDW
					}
					//else
						//B->getCollision() = false;

					else if (A->isPushBack() && !B->isPushBack())
					{
						if (direction_A != direction_B * -1.0f)
						{
							A->ResetMovement(false);
							A->getCollision() = true;
						}
					}
					//A->getCollision() = false; // edw


				}

				else if (pos_diff1.x < 0 || pos_diff1.z < 0)
				{
					if (B->isDazzed() && !A->isPushBack())
					{
						A->adjustVelocity(-A->getVelocity() / 2.0f);
						A->getCollision() = true;
					}

					if (!A->isPushBack() && !B->isPushBack())
					{

						A->adjustVelocity(-A->getVelocity() / 8.f);
						A->getCollision() = true; 
						B->getCollision() = false; // EDW
					}
					//else
						//A->getCollision() = false; // edw

					else if (B->isPushBack() && !A->isPushBack())
					{
						if (direction_A != direction_B * -1.0f)
						{
							B->ResetMovement(false);
							B->getCollision() = true; // edw
						}
					}
					//B->getCollision() = false;

				}


			}
			//

		}
		//

	}


	// asteroids collision
	if (A->getObjectID() == GameObject::ASTEROID && B->getObjectID() == GameObject::ASTEROID)
	{

		glm::vec3 next_moveA = direction_A * glm::vec3(0.5) + interpolated_position_A;
		glm::vec3 next_moveB = direction_B * glm::vec3(0.5) + interpolated_position_B;

		//direction *= -1.0f;
		if (glm::length(next_moveA - interpolated_position_B) < glm::length(interpolated_position_A - interpolated_position_B) ||
			glm::length(next_moveB - interpolated_position_A) < glm::length(interpolated_position_B - interpolated_position_A))
		{
			float momentumA = VELOCITY_A * A->getScaleFactor();
			float momentumB = VELOCITY_B * B->getScaleFactor();

			



				/* version 1 
				glm::vec3 v1 = direction_A * momentumA;
				glm::vec3 v2 = direction_B * momentumB;
				/* */

				/* version 2 */
				glm::vec3 v1 = direction_A;
				glm::vec3 v2 = direction_B;
				/* */

				glm::vec3 n = glm::normalize(interpolated_position_A - interpolated_position_B);

				float a1 = glm::dot(v1, n);
				float a2 = glm::dot(v2, n);

				float optimizedP = (2.0 * (a1 - a2)) / 2.0f;

				glm::vec3 _v1 = v1 - optimizedP *  n;
				glm::vec3 _v2 = v2 + optimizedP * n;

				A->setDirection(_v1);
				B->setDirection(_v2);
				
				
				
				
				/*
				std::cout << "asteroid A vel : " << VELOCITY_A << std::endl;
				std::cout << "asteroid A scale : " << A->getScaleFactor() << std::endl;
				std::cout << "asteroid A dir : ( " << direction_A.x << " , " << direction_A.y << " , " << direction_A.z << " ) " << std::endl;
				std::cout << "momentum A : " << momentumA << std::endl;
				std::cout << "new dirA :  ( " << new_dirA.x << " , " << new_dirA.y << " , " << new_dirA.z << " )" << std::endl;

				std::cout << "\nasteroid B vel : " << VELOCITY_B << std::endl;
				std::cout << "asteroid B scale : " << B->getScaleFactor() << std::endl;
				std::cout << "asteroid B dir : ( " << direction_B.x << " , " << direction_B.y << " , " << direction_B.z << " ) " << std::endl;
				std::cout << "momentum B : " << momentumB << std::endl;
				std::cout << "new dirB :  ( " << new_dirB.x << " , " << new_dirB.y << " , " << new_dirB.z << " )" << std::endl;
				*/
				//system("pause");
			

		}


	}

	



}


void Game::End()
{

}


bool Game::isRunning()
{
	return true;
}


bool & Game::Selecting()
{
	return selecting;
}


void Game::setSelection(bool enable)
{

	selecting = enable;

	
	//std::cout << "\nenable : " << enable << std::endl;
	//std::cout << "isActiveSuperPower : " << isActiveSuperPower() << std::endl;
	if (!selecting && isActiveSuperPower())
	{
		useSuperPower(active_super_power);
		return;
	}
	

#ifdef GAME_RULES
	
	
	

#endif


	if (!selecting)
	{
		if (focused_object != nullptr)
		{
			

			if (!focused_object->isOccupied())
			{

#ifdef GAME_RULES
				if (moving_tower == nullptr)
					bones -= tower_value;
#endif
				spawnObject(GameObject::TOWER, focused_object->getTilePosition());
				focused_object->Occupy(true);
				moving_tower = nullptr;

			}
			else if (moving_tower!=nullptr)
			{

				tower_can_move = true;
				spawnObject(GameObject::TOWER, moving_tower->getTilePosition()); 
				getTile(moving_tower->getTilePosition())->Occupy(true);
				moving_tower = nullptr;
			}

			focused_object = nullptr;
			
		}
	}
}


void Game::cancelSelection()
{
	selecting = false;
	focused_object = nullptr;
	active_super_power = -1;
	if (moving_tower != nullptr)
		bones += tower_value;
}


//Returns a pointer to game's renderer
Renderer * Game::getRenderer()
{
	return renderer;
}


//Returns a reference to game's Terrain
GameObject * Game::getTerrain()
{
	return terrain;
}


std::vector <class GameObject * > & Game::getChests()
{
	return chests;

}


//Returns a reference to game's Objects
std::vector<class GameObject *> & Game::getObjects()
{
	return gameObjects;
}


//Returns a reference to game's road tiles
std::vector<class GameObject *> &  Game::getRoadTiles()
{
	return road_tiles;
}


//Returs a reference to game's free tiles
std::vector<class GameObject*> & Game::getFreeTiles()
{
	return free_tiles;
}


//Returns a reference to game's tiles
std::vector<class GameObject*> & Game::getTiles()
{
	return tiles;
}


std::vector<class GameObject *> & Game::getPirates()
{
	return pirates;
}


std::vector<class GameObject *> & Game::getTowers()
{
	return towers;
}


std::vector <class GameObject * > & Game::getAsteroids()
{
	return asteroids;
}


void Game::setFocusedObject(GameObject * object)
{
	focused_object = object;
}


//Returns a reference to game's focused tiles
GameObject * Game::getFocusedObject()
{
	return focused_object;
}


void Game::setMovingObject(GameObject * object)
{

	moving_tower = object;
#ifdef GAME_RULES
	tower_move_dt = 0.0f;
	tower_can_move = false;
#endif
}



void Game::DeletePirates()
{
	for (int i = 0; i < pirates.size(); i++)
	{
		delete pirates[i];
		pirates[i] = 0;
		pirates[i] = pirates.back();
		pirates.pop_back();

	}

	/*
	for (int i = 0; i < asteroids.size(); i++)
	{
		delete asteroids[i];
		asteroids[i] = 0;
		asteroids[i] = asteroids.back();
		asteroids.pop_back();

	}
	*/
	//ParticleSystem::addParticleEffect(towers[0], towers[1], ParticleEffect::LASER);
	GameObject::number_of_objects = 0;
}


int Game::getBones()
{
	return bones;
}


int Game::getCoins()
{
	return coins;
}


void Game::removeObject(GameObject * object)
{
	switch (object->getObjectID())
	{
	case GameObject::TOWER :
		for (int i = 0; i < towers.size(); i++)
		{
			if (towers[i] == object)
			{
				getTile(object->getTilePosition())->Occupy(false);
				towers[i] = towers.back();
				towers.pop_back();
				return;
			}
		}
		break;
	}

	
}


GameObject * Game::getTile(glm::vec2 tile_pos)
{
	for (GameObject * tile : tiles)
	{
		if (tile->getTilePosition() == tile_pos)
			return tile;
	}

	return nullptr;
}


void Game::Pause()
{
	pause = !pause;
}


int Game::getTowerValue()
{
	return tower_value;
}


bool Game::towerCanMove()
{
#ifdef GAME_RULES
	return tower_can_move;
#endif
	return true;
}


bool Game::towerCanBuy()
{
	return bones >= tower_value ? true : false;
}


bool Game::superPowerCanUse(int id)
{
	switch (id)
	{
	case METEORITE:
		return meteorite_sp_usable;
	case UFO:
		break;
	}
}


void Game::useSuperPower(int id)
{
	switch (id)
	{
	case METEORITE:
	{
		if (focused_object != nullptr)
		{
			GameObject * asteroid = new Asteroid(AssetManager::GetIndex("asteroid02"));
			asteroid->setWorldPosition(GameObject::getWorldPositionBasedOnTile(focused_object->getTilePosition()) + glm::vec3(0,40,0));
			asteroid->setDirection(glm::vec3(0, -1, 0));
			asteroid->adjustVelocity(0.2f);
			asteroid->getScaleFactor() = 5.0f;
			asteroid->amplifyDamage() = true;
			asteroid->removableAfterCollision() = true;
			asteroid->Update(0.0f);
			asteroids.push_back(asteroid);
			meteorite_sp_dt = 0.0f;
			meteorite_sp_usable = false;
			active_super_power = -1;
		}
		break;
		
	}
	}
}


void Game::activateSuperPower(int id)
{
	active_super_power = id;
}


bool Game::isActiveSuperPower()
{
	return active_super_power != -1 ? true : false;
}