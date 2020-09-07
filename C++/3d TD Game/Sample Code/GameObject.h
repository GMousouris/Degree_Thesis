#pragma once
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "GeometryNode.h"
#include "AssetManager.h"
#include "SoundEngine.h"
#include <random>
#include "AnimationHandler.h"


class GameObject
{

public:
	
	//Object Ids
	enum ID
	{
		PIRATE,
		PIRATE_BODY,
		PIRATE_ARM,
		PIRATE_L_FOOT,
		PIRATE_R_FOOT,
		TOWER,
		PROJECTILE,
		CASSINI_PROJECTILE,
		ROAD_TILE,
		FREE_TILE,
		TERRAIN,
		CHEST,
		ASTEROID,
		PLANET,
		CASSINI
	};
	

public:
	int object_index;
	static int number_of_objects;
	static bool showInfo;
	static std::uniform_int_distribution<> rand;
	static std::uniform_int_distribution<> stunRand;
	static std::uniform_int_distribution<> dmgRand;
	static std::uniform_int_distribution<> dmgRand_2;
	static std::uniform_int_distribution<> dmgRand_3;
	static std::mt19937 rng;




protected:
	
	
	class GeometryNode * node = 0; // Object's GeometryNode
	class GeometryNode * pointer_node = 0; // Object's child GeometryNode ( Tower -> Cannonball ) 
	class GameObject * parent = 0; // Object's parent ( cannonball -> Tower , { pirate body , arm , feet } -> pirate , etc )
	
	std::vector < class GameObject * > parts; // Object's parts ( Tower -> cannonball(s) , pirate -> { pirate body , arm , feet } , etc )
	
	//Object's Matrices
	glm::mat4 transformation_matrix;
	glm::mat4 normal_matrix;
	glm::vec3 collision_sphere;

	//object's scale factor
	float scale_factor = 1.0f;

	//Object's Dimensions
	struct dimensions
	{
	public:
		glm::vec3 MIN; 
		glm::vec3 MAX; 
		glm::vec3 SIZE;
		glm::vec3 CENTER; 
		float RADIUS;
	};

	dimensions Dimensions;

	//collision check
	bool collision = false;
	
	//object's direction
	glm::vec3 direction;
	glm::vec3 last_direction;

	//object's rotation
	float rotation = 0.0f;
	float target_rotation = 0.0f;
	float interpolated_rotation = 0.0f;
	bool rotating = false;

	glm::vec2 tilePosition; //Object's tile position
	glm::vec2 target_tilePosition; // Object's target tile Position
	glm::vec2 last_tile_position; // Object's last tile visited
	glm::vec3 world_position; // Object's world position coords
	glm::vec3 last_target_pos;
	glm::vec3 interpolated_world_position = glm::vec3(0); // Object's interpolated world position coords
	glm::vec3 world_center_position; // Object's world center position
	glm::vec3 target_position; // Object's target's world position

	glm::vec2 spawn_point; // Object's spawn point

	bool target_pos_acquired = true; // true - false if object reaches target position
	int tile_index = 0; // Object's tile index e (0 , N) { an Object can be on a road tile or on a free tile }

	int id; // Object's id ( PIRATE , ROAD_TILE , FREE_TILE , TOWER , etc )
	bool drawable = true; // if Object is not drawable it wont get rendered
	bool can_move = true;
	bool focused = false;

	//Timers used to update objects
	float timer = 0.0f;
	float random_movement_speed_multiplier = 1.0f;
	float dt = 0.0f;
	float dt_m = 0.0f;
	float dt_counter = 0.0f;
	float vel_readjust_counter = 0.0f;

	bool taking_damage = false; // can become Pirate's local variable 
	bool push_back = false; // can become Pirate's local variable
	glm::vec3 push_back_direction; // can become Pirate's local variable
	float push_back_factor; // can become Pirate's local variable

	float HP = 100.0f;
	float VELOCITY = 1.0f;
	
	float decompose_timer = 0.0f;
	bool decomposing = false;

	bool amplifiedDamage = false;
	bool remove_after_collision = false;

public:

	//Constructors
	GameObject();
	GameObject(class GeometryNode * node , int id);
	GameObject(class GeometryNode * node, class GeometryNode * pointer_node, int id);
	GameObject(class GeometryNode * node, class GameObject * parent , int id);
	~GameObject();


	//Update Object
	virtual void Update(float dt) = 0;


	virtual void UpdateParts(float dt);

	//Object shootAt -> Object ( Object must be : Tower )
	virtual void shootAt(GameObject * object);

	//
	virtual bool canShoot();

	//Object receive damage incoming from a projectile ( Object must be : Pirate )
	virtual void ReceiveDamage(int dmg , GameObject * object);

	//Check if Object is colliding with another one
	bool hasCollisionWith(GameObject * object);


	bool hasCollisionWith(glm::vec3 plane);

	//
	bool hasCollision();

	//Check if projectile is miss ( Object must be : Projectile )
	virtual bool projectileMiss();

	//
	virtual bool tryPushBack(glm::vec3 direction, float dist);
	

	//Check if Object is alive ( Object must be : Pirate )
	virtual bool isAlive();


	virtual bool isDazzed();


	//Check if Object is Occupied ( Object must be : Tile )
	virtual bool isOccupied();

	//Occupies an Object ( Object must be  : Tile )
	virtual void Occupy(bool enable);

	// true if object can move
	bool & canMove();

	//Returns Object's damage multiplier ( Object must be : Tower )
	virtual float getDamageMultiplier();

	//Returns Object's penetration multiplier ( Object must be : Tower )
	virtual float getPenetrationMultiplier();

	//Returns Object's GeometryNode
	class GeometryNode * getNode();

	//Returns a reference to Object's Transformation matrix
	glm::mat4 & getTransformationMatrix();

	//Returns a reference to Object's Normal matrix
	glm::mat4 & getNormalMatrix();

	//
	virtual glm::mat4 & getCubeTransformationMatrix();

	//
	virtual glm::mat4 & getCubeNormalMatrix();


	//Returns a reference to Object's Collision sphere
	glm::vec3 & getCollisionSphere();

	//Returns Object's tile position
	glm::vec2 getTilePosition();

	//Sets Object's tile position
	virtual void setTilePosition(glm::vec2 pos);

	//Sets Object's target tile position
	void setTargetTilePosition(glm::vec2 pos);
	
	//Returns a reference to Object's tile index
	int & getTileId();

	//Returns true if Object has reached the target position ( Object must be : Pirate )
	bool target_position_acquired();

	//Returns Object's world position coords
	glm::vec3 getWorldPosition();

	//Returns Object's interpolated world position coords
	glm::vec3 getInterpolatedPosition();

	//Returns a reference to Object's scale factor
	float & getScaleFactor();

	struct dimensions getDimensions();

	//Sets Object's world position coords
	virtual void setWorldPosition(glm::vec3 pos);

	//Sets Object's world position coords based on its tile position
	virtual void setWorldPositionBasedOnTile();

	//Returns a world position based on a tile
	glm::vec3 getWorldPositionBasedOntile(glm::vec2 tile);


	static 	glm::vec3 getWorldPositionBasedOnTile(glm::vec2 tile);


	//Sets Object's world center Position
	virtual void setWorldCenterPosition(glm::vec3 pos);
	
	//Returns a reference to Object's target's world position coords
	glm::vec3 & getTargetPosition();

	//Sets Object's target's position world coords
	void setTargetPosition(glm::vec3 pos);

	//Returns Object's id
	int getObjectID();

	int GameObject::getObjectNumberId();

	//
	virtual float getVelocity();

	//Returns a reference to Object's direction
	glm::vec3 & getDirection();
	
	//Sets the direction of the Object
	void setDirection(glm::vec3 dir);

	//Returns a reference to Object's rotation
	float & getRotation();

	void setRotation(float r);

	//Returns Object's interpolated rotation
	float getInterpolatedRotation();

	//Computes Object's rotation based on target tile position
	void computeRotation();

	//Computes Object's direction based on target tile position
	glm::vec3 computeDirection(glm::vec2 target_pos);

	//Sets Objects spawn point
	void setSpawnPoint(glm::vec2 pos);

	//Returns a pointer to Object's i-th part if it has any
	class GameObject * getIndex(int i);
	
	//Returns Object's part's size
	int getPartsSize();

	//Returns true if Object is drawable
	bool isDrawable();

	//
	virtual bool isPushBack();

	//
	virtual float getPushBackFactor();

	//
	virtual bool re_adjustPos();

	//Resets Object's parameters
	void Reset();

	//
	virtual void ResetMovement(bool push);

	//
	virtual void adjustVelocity(float speed);

	//
	virtual void setTiles(std::vector<glm::vec2> * tiles);


	virtual void slowDown(float factor);


	bool & getCollision();


	void setCollision(bool state);


	virtual bool isResetting();
	

	static void setInfo() { showInfo = !showInfo; }


	bool & isFocused();


	virtual void setRadiusXYZ(glm::vec3 xyz);


	virtual void deCompose(glm::vec3 col_direction);


	bool isDecomposing();


	float get_decompose_timer();


	virtual void setHealthPoints(float value);


	virtual bool reachedEndPoint();


	virtual int collectBones();


	virtual bool collectedBones();


	GameObject * getParent();


	bool & amplifyDamage();


	bool & removableAfterCollision();

};