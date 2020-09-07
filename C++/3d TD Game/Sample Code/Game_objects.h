#pragma once
#include "GameObject.h"
#include <iostream>
#include <math.h>



//irrklang::ISoundEngine * soundEngine2;


//Pirate parts
class Pirate_body : public GameObject
{

public:

	Pirate_body(class GeometryNode * node, class GameObject * parent);
	virtual void Update(float dt);
	
	

};

class Pirate_arm : public GameObject
{
public:

	Pirate_arm(class GeometryNode * node, class GameObject * parent);
	virtual void Update(float dt);

};

class Pirate_left_foot : public GameObject
{
public:

	Pirate_left_foot(class GeometryNode * node, class GameObject * parent);
	virtual void Update(float dt);

};

class Pirate_right_foot : public GameObject
{
public:

	Pirate_right_foot(class GeometryNode * node, class GameObject * parent);
	virtual void Update(float dt);

};
/* */

//Class Pirate holds the pirate data
class Pirate : public GameObject
{

public:
	

	Pirate(class GeometryNode * node[4]);

	virtual void Update(float dt);
	virtual void ReceiveDamage(int dmg , GameObject * object);
	void getDazzed();
	virtual bool isDazzed();
	virtual bool isAlive();
	virtual bool isPushBack();
	virtual float getPushBackFactor();
	virtual float getVelocity();
	virtual bool isResetting();

	virtual void setTiles(std::vector<glm::vec2> * tiles);
	virtual void setTilePosition(glm::vec2 pos);
	virtual void setWorldPosition(glm::vec3 pos);
	virtual void setWorldPositionBasedOnTile();
	virtual bool tryPushBack(glm::vec3 direction, float dist);
	virtual void ResetMovement(bool push);
	virtual void slowDown(float factor);
	virtual void adjustVelocity(float speed);
	virtual bool re_adjustPos();

	virtual void deCompose(glm::vec3 col_direction);
	virtual bool reachedEndPoint();
	virtual int collectBones();
	virtual bool collectedBones();
	virtual void setHealthPoints(float value);

private:

	void UpdatePosition();

private:

	Pirate_body * body;
	Pirate_arm  * arm;
	Pirate_left_foot * left_foot;
	Pirate_right_foot * right_foot;

	// struct Tiles -> data of road tiles of current level
	struct Tiles
	{
	public:
		std::vector<glm::vec2> * tiles;
	
		Tiles() {}
		Tiles(std::vector<glm::vec2> * & s_tiles)
		{
			tiles = s_tiles;
		}

		glm::vec2 & operator[](int index)
		{
			
			if (index < 0 || index >= tiles->size())
			{
				std::cout << "-Error : requested Tile index[" << index << "] : out of bounds ( 0 ,"<<(*tiles).size()-1<<" ) "<< std::endl;
				return glm::vec2(-125, -125);
			}
			
			return (*tiles)[index];
		}

		int size()
		{
			return (*tiles).size();
		}
	};
	Tiles RoadTiles;
	//

	/*
	//
	struct tile
	{
	public:
		tile(glm::vec2 pos)
			:pos(pos)
		{ 

		}
		glm::vec2 pos;
		glm::vec2 prev;
		glm::vec2 next;

		bool isEqual(glm::vec2 pos) 
		{
			return (this->pos == pos) ? true : false;
		}
	};
	std::vector<tile> visited_tiles;
	//

	std::vector<glm::vec2> tiles_visited;
	*/

	float VELOCITY = 1.0f;
	float time_passed = 0.0f;
	float SPEED = 1.0f;
	bool push_back_delay = false;

	bool alive = true;
	bool back_from_tile = false;
	bool adjust_pos = true;
	bool re_adjust_timer = false;
	bool reseting_movement = false;
	bool reset_pos = false;
	bool dazzed = false;
	bool after_dazzed = false;
	float dazzed_timer = 0.0f;
	float dazzed_cooldown_timer = 0.0f;
	
	bool reached_endpoint = false;
	int bones = 5;

	int pain_sound = 0;
};


//Class Tower holds the Tower data
class Tower : public GameObject
{
public:

	Tower(class GeometryNode * node , class GeometryNode * pointer_node);

	virtual void Update(float dt);
	virtual void UpdateParts(float dt);
	virtual void shootAt(GameObject * object);
	virtual bool canShoot();
	virtual float getDamageMultiplier();
	virtual float getPenetrationMultiplier();
	bool canMove();

private:

	float dmg;
	float penetration;
	float shoot_timer = 1.5f;
	bool can_shoot = false;
};


class Projectile : public GameObject
{
public:

	Projectile(GeometryNode * node ,GameObject * parent, GameObject * target);
	Projectile(GeometryNode * node, int id ,GameObject * parent, GameObject * target);
	virtual void Update(float dt);
	virtual bool projectileMiss();

private:

	GameObject * target;
	bool miss = false;
	float speed;
	float dmg;
	float penetration;
	bool hit = false;
	glm::vec3 scale_vector;

};

//Class Terrain holds the Terrain data
class Terrain : public GameObject
{

public:

	Terrain(class GeometryNode * node);
	virtual void Update(float dt);
	virtual glm::mat4 & getCubeTransformationMatrix();
	virtual glm::mat4 & getCubeNormalMatrix();

private:
	glm::mat4 cube_transformation_matrix;
	glm::mat4 cube_normal_matrix;

};


//Class Tile holds the Tile data (road)
class Tile : public GameObject
{
public:
	
	Tile(class GeometryNode * node , int id);

	virtual void Update(float dt);
	virtual bool isOccupied();
	virtual void Occupy(bool enable);
	virtual glm::mat4 & getCubeTransformationMatrix();
	virtual glm::mat4 & getCubeNormalMatrix();
	virtual void setWorldPositionBasedOnTile();

private:
	
	glm::mat4 cube_transformation_matrix;
	glm::mat4 cube_normal_matrix;

	bool occupied;
};


//Class Chest
class Chest : public GameObject
{
public:
	Chest(class GeometryNode * node);
	virtual void Update(float dt);
};


//Class Asteroid
class Asteroid : public GameObject
{
public:
	Asteroid(class GeometryNode * node);
	Asteroid(class GeometryNode * node , int id);
	virtual void Update(float dt);
	virtual void setRadiusXYZ(glm::vec3 xyz);
	virtual void adjustVelocity(float speed);
	virtual float getVelocity();

	virtual void shootAt(GameObject * object);
	virtual bool canShoot();


private:

	bool can_shoot = false;
	float shoot_timer = 10.0f;
	float dmg;

	float radiusX;
	float radiusY;
	float radiusZ;
	glm::vec3 xyz;

	glm::vec3 rot_direction;

	float VELOCITY = 1.0f;
};


