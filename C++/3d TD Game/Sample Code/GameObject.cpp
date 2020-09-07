#include "GameObject.h"
#include <iostream>



int GameObject::number_of_objects = 0;
bool GameObject::showInfo = false;
std::uniform_int_distribution<> GameObject::rand(1, 3);
std::uniform_int_distribution<> GameObject::stunRand(1, 10);
std::uniform_int_distribution<> GameObject::dmgRand(8, 25);
std::uniform_int_distribution<> GameObject::dmgRand_2(0,1);
std::uniform_int_distribution<> GameObject::dmgRand_3(0, 5);



std::mt19937 GameObject::rng;



GameObject::GameObject()
{

}

GameObject::GameObject(class GeometryNode * node, int id)
	:id(id)
{
	this->node = node;

	if (id == GameObject::PIRATE)
	{
		object_index = number_of_objects;
		number_of_objects++;
	}
	

}

GameObject::GameObject(class GeometryNode * node, class GeometryNode * pointer_node, int id)
	:id(id)
{
	this->node = node;
	this->pointer_node = pointer_node;
}

GameObject::GameObject(class GeometryNode * node, class GameObject * parent, int id)
	:id(id)
{
	this->node = node;
	this->parent = parent;
}

GameObject::~GameObject()
{

}
/* */

void GameObject::UpdateParts(float dt) {}

//Returns the GeometryNode of the object
class GeometryNode * GameObject::getNode()
{
	return this->node;
}


//Returns the Transformation matrix of the object
glm::mat4 & GameObject::getTransformationMatrix()
{
	return transformation_matrix;
}


//Returns the Normal matrix of the object
glm::mat4 & GameObject::getNormalMatrix()
{
	return normal_matrix;
}


glm::mat4 & GameObject::getCubeTransformationMatrix()
{
	return glm::mat4(1.0f);
}

//
glm::mat4 & GameObject::getCubeNormalMatrix()
{
	return glm::mat4(1.0f);
}


//returns the scale factor
float & GameObject::getScaleFactor()
{
	return scale_factor;
}


//Returns Object's Dimensions
GameObject::dimensions GameObject::getDimensions()
{
	return Dimensions;
}


//Returns the tile position of the object
glm::vec2 GameObject::getTilePosition()
{
	return tilePosition;
}


//Sets the tile the position of the object
void GameObject::setTilePosition(glm::vec2 pos)
{
	last_tile_position = tilePosition;
	tilePosition = glm::vec2(pos);
}


//Sets the target tile position of the Object
void GameObject::setTargetTilePosition(glm::vec2 pos)
{
	target_tilePosition = pos;
	target_position = getWorldPositionBasedOntile(target_tilePosition)+glm::vec3(0,world_position.y - 0.1,0);
	
	//setDirection(computeDirection(target_tilePosition));
	//computeRotation();

	/*
	std::cout << "\nPirate (" << object_index << ") : computedRotation()" << std::endl;
	std::cout << "Pirate (" << object_index << ") : Rotation : " << interpolated_rotation << std::endl;
	std::cout << "Pirate (" << object_index << ") : Target rotation : " << target_rotation << std::endl;
	std::cout << "Pirate (" << object_index << ") : Position : [" << tilePosition.x << " , " << tilePosition.y << "] " << std::endl;
	std::cout << "Pirate (" << object_index << ") : Target pos : [" << pos.x << " , " << pos.y << "]" << std::endl;
	*/
}


//Returns a reference to object's tile ID
int & GameObject::getTileId()
{
	return tile_index;
}


//Returns object's world position
glm::vec3 GameObject::getWorldPosition()
{
	return world_position;
}


//Sets the world position of the object
void GameObject::setWorldPosition(glm::vec3 pos)
{
	world_position = glm::vec3(pos);
}


//Returns object's interpolated position
glm::vec3 GameObject::getInterpolatedPosition()
{
	
	return interpolated_world_position;
}


//Sets object's world position based on its tile position
void GameObject::setWorldPositionBasedOnTile()
{
	float y = 0.1;
	float z = -18 + 4 * tilePosition.y;
	float x = -18 + 4 * tilePosition.x;
	
	world_position = glm::vec3(world_center_position.x + x, 0.1, world_center_position.z + z);
	interpolated_world_position = world_position;
}


//Returns a world position based on a tile
glm::vec3 GameObject::getWorldPositionBasedOntile(glm::vec2 tile)
{
	float y = 0.1;
	float z = -18 + 4 * tile.y;
	float x = -18 + 4 * tile.x;

	world_center_position  = glm::vec3(18, 0, 16); 
	return glm::vec3(world_center_position.x + x, 0.1, world_center_position.z + z);

}


//
glm::vec3 GameObject::getWorldPositionBasedOnTile(glm::vec2 tile)
{

	float y = 0.1;
	float z = -18 + 4 * tile.y;
	float x = -18 + 4 * tile.x;

	glm::vec3 world_center_position = glm::vec3(18, 0, 16);
	return glm::vec3(world_center_position.x + x, 0.1, world_center_position.z + z);

}


//Sets object's world's center position
void GameObject::setWorldCenterPosition(glm::vec3 pos)
{
	world_center_position = pos;
}


//Returns a reference to Object's target position
glm::vec3 & GameObject::getTargetPosition()
{
	return target_position;
}


//Sets the Target position of the object
void GameObject::setTargetPosition(glm::vec3 pos)
{
	last_target_pos = target_position;
	target_position = pos;
	target_pos_acquired = false;
}


//Returns true if Target position of the object has been acquired
bool GameObject::target_position_acquired()
{
	return target_pos_acquired;
}


//Sets the direction of the Object
void GameObject::setDirection(glm::vec3 dir)
{
	last_direction = direction;
	direction = dir;
}


//Returns a reference to object's direction
glm::vec3 & GameObject::getDirection()
{
	return direction;
}


float GameObject::getVelocity()
{
	return VELOCITY;
}


//Returns a reference to object's rotation
float & GameObject::getRotation()
{
	return rotation;
}


void GameObject::setRotation(float r)
{
	rotation = r;
}

//Returns object's interpolated rotation
float GameObject::getInterpolatedRotation()
{
	return interpolated_rotation;
}


//Computes object's rotation based on its direction
void GameObject::computeRotation()
{
	if (direction == last_direction)
		return;
	
	glm::vec2 diff = target_tilePosition - tilePosition;
	diff.x *= -1.0f;
	
	float rot_scale_factor = 1.0f;

	if (tilePosition == spawn_point)
	{
		if(glm::length(last_direction) == 0.0f)
			last_direction = glm::vec3(1);
		rot_scale_factor = 2.0f;
	}

	if (diff.x == 0 && diff.y == 0)
		return;

	if (diff.y == 1)
	{
		target_rotation = 90 * (-1.0f*last_direction.x);
	}
	else if (diff.y == -1)
	{
		target_rotation = 90 * (last_direction.x);
	}
	else if (diff.x == 1)
	{
		target_rotation = 90 * (-1.0f*last_direction.z);
	}
	else if (diff.x == -1)
	{
		target_rotation = 90 * (last_direction.z);
	}

	target_rotation *= rot_scale_factor;
	target_rotation += rotation;
	rotating = true;
}


//Computes Object's direction based on target tile position
glm::vec3 GameObject::computeDirection(glm::vec2 target_tile_pos)
{

	glm::vec3 direction;
	
	// 
	
	if (target_tile_pos.y > tilePosition.y)
		direction.z = +1.0f;
	else if (target_tile_pos.y < tilePosition.y)
		direction.z = -1.0f;
	else
		direction.z = 0.0f;
	if (target_tile_pos.x > tilePosition.x)
		direction.x = +1.0f;
	else if (target_tile_pos.x < tilePosition.x)
		direction.x = -1.0f;
	else
		direction.x = 0.0f;

	if (object_index == 0 && showInfo)
	{ 
		std::cout << "\n..computeDirection()::tilePos : [" << tilePosition.x << " , " << tilePosition.y << "]" << std::endl;
		std::cout << "..computeDirection()::target_TilePos : [" << target_tilePosition.x << " , " << target_tilePosition.y << "] " << std::endl;
		std::cout << "..computeDirection()::direction before : ( " << this->direction.x <<" , "<<this->direction.y<<" , "<<this->direction.z<<" ) " << std::endl;
		std::cout << "..computeDirection()::direction after : ( " << direction.x << " , " << direction.y << " , " << direction.z << " ) " << std::endl;

	}


	return direction;
}


//Returns the object's ID
int GameObject::getObjectID()
{
	return id;
}


int GameObject::getObjectNumberId()
{
	return object_index;
}


//Sets the spawn point of the object
void GameObject::setSpawnPoint(glm::vec2 pos)
{
	spawn_point = pos;
}


//Returns the i-th part of the object
GameObject * GameObject::getIndex(int i)
{
	if (i < 0 || i >= parts.size())
		return nullptr;

	return parts[i];
}


//Returns size of parts
int GameObject::getPartsSize()
{
	return parts.size();
}


//Returns true if object's is drawable
bool GameObject::isDrawable()
{
	return drawable;
}


bool GameObject::isOccupied()
{
	return false;
}


void GameObject::Occupy(bool enable)
{

}


bool GameObject::isAlive()
{
	return true;
}


bool GameObject::isDazzed()
{
	return false;
}


bool & GameObject::canMove()
{
	return can_move;
}


void GameObject::shootAt(GameObject * object)
{

}


bool GameObject::canShoot()
{
	return false;
}


void GameObject::ReceiveDamage(int dmg , GameObject * object)
{

}


float GameObject::getDamageMultiplier()
{
	return 0.0f;
}


float GameObject::getPenetrationMultiplier()
{
	return 0.0f;
}


bool GameObject::hasCollision()
{
	return collision;
}


//Returns the collision matrix of the object
glm::vec3 & GameObject::getCollisionSphere()
{
	return interpolated_world_position + Dimensions.CENTER*scale_factor;
}


bool GameObject::hasCollisionWith(GameObject * object)
{

	glm::vec3 collA = getCollisionSphere();
	glm::vec3 collB = object->getCollisionSphere();

	if (object->getObjectID() != GameObject::TERRAIN && object->getObjectID() != GameObject::ROAD_TILE)
	{
		
		float radiusA = Dimensions.RADIUS * scale_factor;
		float radiusB = object->getDimensions().RADIUS * object->getScaleFactor();

		if (pow((collA.x - collB.x), 2) + pow((collA.y - collB.y), 2)
			+ pow((collA.z - collB.z), 2) < pow((radiusA + radiusB), 2))
		{
			
			return true;
		}
	}
	else
	{
		float radiusA = 1.0f * scale_factor;
		float radiusB = object->getDimensions().RADIUS * object->getScaleFactor();
		
		float x_offset = 0.0f;
		float y_offset = 0.0f;
		float z_offset = 0.0f;

		if (object->getObjectID() == GameObject::TERRAIN)
			y_offset = -25.0f;
		else if (object->getObjectID() == GameObject::ROAD_TILE)
			y_offset = -1.0f;

		

		float planeX = abs(interpolated_world_position.x - (object->getWorldPosition().x + x_offset) );
		float planeY = abs(interpolated_world_position.y - (object->getWorldPosition().y + y_offset) );
		float planeZ = abs(interpolated_world_position.z - (object->getWorldPosition().z + z_offset) );

		if ( planeX < radiusA  + radiusB)
		{
			if ( planeY < radiusA + radiusB )
			{
				if ( planeZ < radiusA + radiusB)
				{
					
					glm::vec3 next_move = direction * glm::vec3(0.5) + interpolated_world_position;
					glm::vec3 base_pos = object->getWorldPosition() + glm::vec3(x_offset, y_offset, z_offset);

					glm::vec3 pos_diff1 = next_move - base_pos;
					glm::vec3 pos_diff2 = interpolated_world_position - base_pos;

					/*
					std::cout << "\n\ndirection : ( " << direction.x << " , " << direction.y << " , " << direction.z << " ) " << std::endl;
					std::cout << "next_move - pos :  ( " << pos_diff1.x << " , " << pos_diff1.y << " ," << pos_diff1.z << " ) " << std::endl;
					std::cout << "inteprolated_pos - pos :  ( " << pos_diff2.x << " , " << pos_diff2.y << " ," << pos_diff2.z << " ) " << std::endl;

					std::cout << "length(pos_diff1) : " << glm::length(pos_diff1) << std::endl;
					std::cout << "length(pos_diff2) : " << glm::length(pos_diff2) << std::endl;
					*/
					
					if (glm::length(pos_diff1) < glm::length(pos_diff2))
					{

						float planes[] = { planeX , planeY , planeZ };
						int _max = 0;
						float max = planeX;
						for (int i = 1; i < 3; i++)
						{
							if (planes[i] > max)
							{
								max = planes[i];
								_max = i;
							}
							else
								planes[i] = 1.0f;
						}

						//std::cout << "dir before : (" << direction.x << " , " << direction.y << " , " << direction.z << " ) " << std::endl;
						direction[_max] *= -1.0f;
						//std::cout << "dir after : (" << direction.x << " , " << direction.y << " , " << direction.z << " ) " << std::endl;

						return true;
					}
					else
					{
						return false;
					}
					
				}
			}
		}
	}
	

	return false;
}


bool GameObject::hasCollisionWith(glm::vec3 plane)
{
	if (interpolated_world_position.x >= plane.x)
	{
		if(direction.x > 0.0f)
			direction.x *= -1.0f;
	}
	else if (interpolated_world_position.x <= -plane.x)
	{
		if (direction.x < 0.0f)
			direction.x *= -1.0f;
	}
	if (interpolated_world_position.y >= plane.y  )
	{
		if (direction.y > 0.0f)
			direction.y *= -1.0f;
	}
	else if (interpolated_world_position.y <= -plane.y)
	{
		if (direction.y < 0.0f)
			direction.y *= -1.0f;
	}
	if (interpolated_world_position.z >= plane.z )
	{
		if (direction.z > 0.0f)
			direction.z *= -1.0f;
	}
	else if (interpolated_world_position.z <= -plane.z)
	{
		if (direction.z < 0.0f)
			direction.z *= -1.0f;
	}
	
	return false;
}



bool GameObject::projectileMiss()
{
	return true;
}


bool GameObject::isPushBack()
{
	return false;
}


float GameObject::getPushBackFactor()
{
	return 0.0f;
}


bool GameObject::tryPushBack(glm::vec3 direction, float dist)
{
	return false;
}


//Resets Object
void GameObject::Reset()
{
	
}


void GameObject::ResetMovement(bool push)
{
	
}


bool GameObject::re_adjustPos()
{
	return true;

}


void GameObject::adjustVelocity(float offset)
{

	//world_position = interpolated_world_position;
	
	if (VELOCITY + offset > 1.0f)
		VELOCITY = 1.0f;
	else
		VELOCITY += offset;

	
}


void GameObject::setTiles(std::vector<glm::vec2> * tiles)
{

}

void GameObject::slowDown(float factor) {}

bool GameObject::isResetting()
{
	return false;
}

bool & GameObject::getCollision()
{
	return collision;
}


void GameObject::setCollision(bool state)

{
	collision = state;
	if (collision)
		adjustVelocity(-1.0f);


}


bool & GameObject::isFocused()
{
	return focused;
}


void GameObject::setRadiusXYZ(glm::vec3 xyz)
{

}


void GameObject::deCompose(glm::vec3 col_direction)
{

}


bool GameObject::isDecomposing()
{
	return decomposing;
}


float GameObject::get_decompose_timer()
{
	return decompose_timer;
}


void GameObject::setHealthPoints(float value)
{
	HP = value;
}

bool GameObject::reachedEndPoint()
{
	return false;
}


int GameObject::collectBones() 
{ 
	return 0; 
}


bool GameObject::collectedBones()
{
	return false;
}


GameObject * GameObject::getParent()
{
	return parent;
}


bool & GameObject::amplifyDamage()
{
	return amplifiedDamage;
}


bool & GameObject::removableAfterCollision()
{
	return remove_after_collision;
}