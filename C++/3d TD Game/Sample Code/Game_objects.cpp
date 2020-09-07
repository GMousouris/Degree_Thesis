
#include "Game_objects.h"
#include "Defines.h"
//#include <cstdlib>
//#include <ctime>
//#include <iostream>
//#include <stdio.h>






/*PIRATE PARTS*/
Pirate_body::Pirate_body(class GeometryNode * node, class GameObject * parent)
	:GameObject(node, parent, GameObject::PIRATE_BODY)
{
	scale_factor = parent->getScaleFactor();
}

Pirate_arm::Pirate_arm(class GeometryNode * node, class GameObject * parent)
	: GameObject(node, parent, GameObject::PIRATE_ARM)
{
	scale_factor = parent->getScaleFactor();
}

Pirate_left_foot::Pirate_left_foot(class GeometryNode * node, class GameObject * parent)
	: GameObject(node, parent, GameObject::PIRATE_L_FOOT)
{
	scale_factor = parent->getScaleFactor();
}

Pirate_right_foot::Pirate_right_foot(class GeometryNode * node, class GameObject * parent)
	: GameObject(node, parent, GameObject::PIRATE_R_FOOT)
{
	scale_factor = parent->getScaleFactor();
}
//


//Update pirate Parts
void Pirate_body::Update(float dt)
{
	direction = parent->getDirection();
	rotation = parent->getInterpolatedRotation();
	VELOCITY = parent->getVelocity();

	float rot_angle;
	float rot_VELOCITY = VELOCITY;
	glm::vec3 rot_direction;


	
	if (parent->isDazzed() && !parent->isDecomposing())
	{

		world_position = parent->getInterpolatedPosition();
		interpolated_world_position = glm::vec3(world_position.x, world_position.y + abs(0.05*sin(5.0f * this->dt)), world_position.z);

		transformation_matrix =

			glm::translate(glm::mat4(1.0f), interpolated_world_position - glm::vec3(0,1,0))

			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 1, 0)) // rotate object around Y axis (strafe)

			* glm::rotate(glm::mat4(1.0f), 0.2f * cos(this->dt_counter * 0.01f) , glm::vec3(0,0,0.01))

			* glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.5, 0))

			* glm::rotate(glm::mat4(1.0f) , 0.2f * sin(this->dt_counter * 0.01f) , glm::vec3(0.01, 0, 0))

			* glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.5, 0))

			* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));

		normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));

		this->dt_counter += 8.0f;
		this->dt += dt;

		return;

	}
	else if(parent->isDecomposing())
	{
		world_position = parent->getWorldPosition();
		rot_angle = this->dt * -20.1f;
		rot_VELOCITY = 1.0f;
		rot_direction = glm::vec3(1,0,1);

		interpolated_world_position = glm::mix(world_position, target_position, timer * VELOCITY);
		timer += dt;
	}
	else
	{
		world_position = parent->getInterpolatedPosition();
		interpolated_world_position = glm::vec3(world_position.x, world_position.y + abs(0.05*sin(5.0f * this->dt)), world_position.z);
		rot_angle = glm::radians(20.0f) * sin(10.0f * this->dt);
		rot_direction = direction;
		dt_counter = 0.0f;
	}
	
	transformation_matrix =

		glm::translate(glm::mat4(1.0f), interpolated_world_position)

		* glm::rotate(glm::mat4(1.0f), rot_VELOCITY * rot_angle , rot_direction)

		* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 1, 0)) // rotate object around Y axis (strafe)

		* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));

	normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));


	if (!parent->isPushBack() && !parent->getCollision() )
		this->dt += dt;


}

void Pirate_arm::Update(float dt)
{
	//if(!parent->isDecomposing())
	
	direction = parent->getDirection();
	rotation = parent->getInterpolatedRotation();
	VELOCITY = parent->getVelocity();

	float rot_angle;
	glm::vec3 rot_direction;

	
	if (parent->isDazzed() && !parent->isDecomposing()) 
	{
		return;
	}
	else if (parent->isDecomposing())
	{

		world_position =  parent->getWorldPosition();
		glm::vec3 offset;

		/*
		if (direction.z * -1.0f < 0)
			offset = glm::vec3(abs(world_position.z) * 0.5, 0, 0);
		else if (direction.z * -1.0f > 0)
			offset = glm::vec3(-abs(world_position.z) * 0.5, 0, 0);
		else if (direction.x * -1.0f < 0)
			offset = glm::vec3(0, 0, abs(world_position.x) * 0.5);
		else if (direction.x * -1.0f > 0)
			offset = glm::vec3(0, 0, -abs(world_position.x) * 0.5);
		*/

		interpolated_world_position = glm::mix( world_position  , target_position , timer * VELOCITY);
		//std::cout << "arm_pos : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " )" << std::endl;

		
		rot_angle = this->dt * -20.f;
		rot_direction = glm::vec3(1, 0, 1);

		timer += dt;
	}
	else 
	{
		world_position = parent->getInterpolatedPosition();
		interpolated_world_position = glm::vec3(world_position.x, world_position.y + ((12)*scale_factor) + abs(0.05*sin(5 * this->dt)), world_position.z);
		rot_angle = glm::radians(rotation);
		rot_direction = glm::vec3(0, 1, 0);
	}

	transformation_matrix =

		glm::translate(glm::mat4(1.0f), interpolated_world_position)

		* glm::rotate(glm::mat4(1.0f), rot_angle , rot_direction ) // rotate object around Y axis (strafe)
		* glm::translate(glm::mat4(1.0f), glm::vec3((4.5*scale_factor), 0, 0)) // with radius =  0.09*4.5(x)

		* glm::rotate(glm::mat4(1.0f), VELOCITY * sin(9 * this->dt), glm::vec3(1, 0, 0)) // rotate object around X axis (moving hand animation)
		* glm::translate(glm::mat4(1.0f), glm::vec3(0, (-2.25*scale_factor), (-2.2*scale_factor))) // with radius =  -2.25 * 0.09(y)  &  -2.25 * 0.09(z)

		//* glm::rotate(glm::mat4(1.0f), VELOCITY * glm::radians(40.0f) * sin(-10 * this->dt), direction )
		//* glm::translate(glm::mat4(1.0f), glm::vec3( -2.25 * scale_factor*dir.x , 0 , -2.25 * scale_factor*dir.z) ) // with radius =  -2.25 * 0.09(y)  &  -2.25 * 0.09(z)


		* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));


	normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));


	if (!parent->isPushBack() && !parent->getCollision())
		this->dt += dt;


	
}

void Pirate_left_foot::Update(float dt)
{

	
	direction = parent->getDirection();
	rotation = parent->getInterpolatedRotation();
	VELOCITY = parent->getVelocity();

	float rot_angle;
	glm::vec3 rot_direction;

	
	if (parent->isDazzed() && !parent->isDecomposing()) 
	{
		world_position = parent->getInterpolatedPosition();
		interpolated_world_position = glm::vec3(world_position.x, (6 * scale_factor) + world_position.y + abs(0.05*sin(5 * this->dt)), world_position.z);
		rot_angle = glm::radians(rotation);
		rot_direction = glm::vec3(0, 1, 0);
		this->dt = 0.0f;
	}
	else if (parent->isDecomposing())
	{

		world_position = parent->getWorldPosition();
		glm::vec3 offset;

		/*
		if (direction.z * -1.0f < 0)
			offset = glm::vec3(abs(world_position.z) * 0.8, 0, 0);
		else if (direction.z * -1.0f > 0)
			offset = glm::vec3(-abs(world_position.z) * 0.8, 0, 0);
		else if (direction.x * -1.0f < 0)
			offset = glm::vec3(0, 0, abs(world_position.x) * 0.8);
		else if (direction.x * -1.0f > 0)
			offset = glm::vec3(0, 0, -abs(world_position.x) * 0.8);

		*/
		interpolated_world_position = glm::mix(world_position, target_position, timer * VELOCITY);
		//std::cout << "left_foot_pos : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " )" << std::endl;


		rot_angle = this->dt * -20.f;
		rot_direction = glm::vec3(1, 0, 1);

		timer += dt;
	}
	else
	{
		world_position = parent->getInterpolatedPosition();
		interpolated_world_position = glm::vec3(world_position.x, (6 * scale_factor) + world_position.y + abs(0.05*sin(5 * this->dt)), world_position.z);
		rot_angle = glm::radians(rotation);
		rot_direction = glm::vec3(0, 1, 0);
	}

	transformation_matrix =


		glm::translate(glm::mat4(1.0f), interpolated_world_position )

		* glm::rotate(glm::mat4(1.0f), rot_angle , rot_direction ) // rotate object around Y axis (strafe)
		* glm::translate(glm::mat4(1.0f), glm::vec3((-4 * scale_factor), 0, (-2 * scale_factor))) // with radius =  -4 * 0.09(x)  &  -2* 0.09(z)

		* glm::rotate(glm::mat4(1.0f), VELOCITY * sin(10 * this->dt) + 0.2f, glm::vec3(1, 0, 0)) // rotate object around X axis (moving foot animation)
		* glm::translate(glm::mat4(1.0f), glm::vec3(0, -(5.5*scale_factor), 0)) // with radius =  -6.5 * 0.09(y)
		* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));



	normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));

	if (!parent->isPushBack() && !parent->getCollision())
		this->dt += dt;

}

void Pirate_right_foot::Update(float dt)
{

	
	direction = parent->getDirection();
	rotation = parent->getInterpolatedRotation();
	VELOCITY = parent->getVelocity();

	float rot_angle;
	glm::vec3 rot_direction;

	
	if (parent->isDazzed() && !parent->isDecomposing())
	{
		world_position = parent->getInterpolatedPosition();
		interpolated_world_position = glm::vec3(world_position.x, (6 * scale_factor) + world_position.y + abs(0.05*sin(5 * this->dt)), world_position.z);
		rot_angle = glm::radians(rotation);
		rot_direction = glm::vec3(0, 1, 0);
		this->dt = 0.0f;
	}
	else if (parent->isDecomposing())
	{
		world_position = parent->getWorldPosition();
		glm::vec3 offset;

		/*
		if (direction.z * -1.0f > 0)
			offset = glm::vec3(abs(world_position.z) * 0.8, 0, 0);
		else if (direction.z * -1.0f < 0)
			offset = glm::vec3(-abs(world_position.z) * 0.8, 0, 0);
		else if (direction.x * -1.0f > 0)
			offset = glm::vec3(0, 0, abs(world_position.x) * 0.8);
		else if (direction.x * -1.0f < 0)
			offset = glm::vec3(0, 0, -abs(world_position.x) * 0.8);

		*/
		interpolated_world_position = glm::mix(world_position, target_position, timer * VELOCITY);
		//std::cout << "right_foot : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " )" << std::endl;


		rot_angle = this->dt * -20.f;
		rot_direction = glm::vec3(1, 0, 1);

		timer += dt;
	}
	else
	{
		world_position = parent->getInterpolatedPosition();
		interpolated_world_position = glm::vec3(world_position.x, (6 * scale_factor) + world_position.y + abs(0.05*sin(5 * this->dt)), world_position.z);
		rot_angle = glm::radians(rotation);
		rot_direction = glm::vec3(0, 1, 0);
	}


	transformation_matrix =

		glm::translate(glm::mat4(1.0f), interpolated_world_position) // + (2 * 0.09)

		* glm::rotate(glm::mat4(1.0f), rot_angle , rot_direction ) //rotate object around Y (strafe)
		* glm::translate(glm::mat4(1.0f), glm::vec3((4 * scale_factor), 0, (-2 * scale_factor))) // with Radius =  4*0.09(x)  &  -2 *0.09(z)

		* glm::rotate(glm::mat4(1.0f), VELOCITY * (0.2f - sin(10 * this->dt)), glm::vec3(1, 0, 0)) //rotate object around X axis (moving foot animation)
		* glm::translate(glm::mat4(1.0f), glm::vec3(0, -(5.5*scale_factor), 0)) // with radius =  -6.5 * 0.09(y)
		* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));


	normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));


	if (!parent->isPushBack() && !parent->getCollision())
		this->dt += dt;

}
/* */








/*PIRATE*/
Pirate::Pirate(class GeometryNode * node[4])
	:GameObject(nullptr, GameObject::PIRATE)
{
	rotation = 180.0f;

	scale_factor = 0.09;

	Dimensions.CENTER = glm::vec3(-0.5957, 11.683, -2.274);
	Dimensions.RADIUS = 10.87075;

	parts.push_back(new Pirate_body(node[0], this));
	parts.push_back(new Pirate_arm(node[1], this));
	parts.push_back(new Pirate_left_foot(node[2], this));
	parts.push_back(new Pirate_right_foot(node[3], this));


	//HP = 200.0f;
	VELOCITY = 1.0f;
	alive = true;
	drawable = false;

	rng.seed(std::random_device()());
	pain_sound = rand(rng);

	

}

void Pirate::Update(float dt)
{

	/*
	   Interpolate object from current position to target position based on next road tile
	   Interpolation from A -> B happens within maximum 1 sec ( error = O(10^-2) )
	*/

	/*
	   check if time_passed between interpolation is < 1 sec
	 o if TRUE :
	   interpolates from A -> B with factor a = time_passed  where :
	   interpolated_world_position = world_position * (1-a) + target_position * a ,
	   hence while : time_passed -> 1 (second) ==> interpolated_world_position -> target_position

	 o if FALSE :
	   fix current object's position to target position ( error = O(10^-2) )
	   in order to avoid possible unexpected movement errors in the future
	*/


	if (showInfo && object_index == 0)
	{
		std::cout << "\npirate(" << object_index << ") pos : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " )" << std::endl;
		std::cout << "pirate(" << object_index << ") target_pos : ( " << target_position.x << " , " << target_position.y << " , " << target_position.z << " )" << std::endl;
		std::cout << "pirate(" << object_index << ") tile : [" << tilePosition.x << " , " << tilePosition.y << "]" << std::endl;
		std::cout << "pirate(" << object_index << ") target_Tile : [" << target_tilePosition.x << " , " << target_tilePosition.y << "]" << std::endl;
		std::cout << "pirate(" << object_index << ") tile index : " << tile_index << std::endl;
		std::cout << "pirate(" << object_index << ") RoadTile[" << tile_index << "] : [" << RoadTiles[tile_index].x << " , " << RoadTiles[tile_index].y << "]" << std::endl;
		std::cout << "pirate(" << object_index << ") RoadTile[" << tile_index << "] pos : ( " << getWorldPositionBasedOntile(RoadTiles[tile_index]).x
	    << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).y << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).z << " )" << std::endl;
		std::cout << "pirate(" << object_index << ") Velocity : " << VELOCITY << std::endl;
		std::cout << "pirate(" << object_index << ") timer : " << timer << std::endl;
		std::cout << "pirate(" << object_index << ") Push_back : " << push_back << std::endl;
		std::cout << "pirate(" << object_index << ") Back_from_tile : " << back_from_tile << std::endl;
		std::cout << "pirate(" << object_index << ") Reseting : " << reseting_movement << std::endl;
		std::cout << "pirate(" << object_index << ") Collision : " << collision << std::endl;
		std::cout << "pirate(" << object_index << ") vel_readjust_counter : " << vel_readjust_counter << std::endl;
		std::cout << "pirate(" << object_index << ") interpolated_rotation : " << interpolated_rotation << std::endl;
		std::cout << "pirate(" << object_index << ") Target rotation : " << target_rotation << std::endl;
		std::cout << "pirate(" << object_index << ") Direction : (" << direction.x << " , " << direction.y << " , " << direction.z << " )" << std::endl;

	}



	if (VELOCITY < 1.0f && vel_readjust_counter > 0.05/3.5 && !decomposing && !dazzed) // edw to resetting_movmeent
	{
		if (VELOCITY <= 0.01f && !collision || !push_back && !collision || !dazzed && !collision)// || !reseting_movement && !collision) //(VELOCITY <= 0.01f || !push_back)
		{
			adjustVelocity(0.125/4.0f); //0.125f
			vel_readjust_counter = 0.0f;
		}

	}
	



	if (dt_counter >= 1.2f && push_back_delay) //1.5
		push_back_delay = false;
	else if (push_back_delay)
		dt_counter += dt;

	if (VELOCITY <= 0.01f  && !decomposing && !dazzed)
	{
		if (!collision)
		{
			vel_readjust_counter += dt;
		}

		return;
	}

	if (!decomposing && !dazzed)
	{

		float diff = glm::length(glm::abs(interpolated_world_position - target_position));
		float tresh = 0.0f;

		if (!push_back && timer < 1.0f / VELOCITY && diff > tresh) //diff > tresh
			interpolated_world_position = glm::mix(world_position, target_position, timer * VELOCITY);

		else if (push_back && timer < 0.5f && interpolated_world_position != target_position) // 0.5f
			interpolated_world_position = glm::mix(world_position, target_position, timer * 2.0f);  // 1/0.5f;

		else
		{

			timer = 0.0f;
			world_position = target_position;
			interpolated_world_position = world_position;
			reseting_movement = false;

			if (push_back)
			{
				push_back_delay = true;
				dt_counter = 0.0f;

				direction *= -1.0f;

				if (!back_from_tile)
				{

					push_back = false;


					setTilePosition(RoadTiles[tile_index]);
					setTargetTilePosition(RoadTiles[tile_index + 1]);

					if (re_adjust_timer)
					{

						glm::vec3 current_pos = interpolated_world_position;
						glm::vec3  target_pos = getWorldPositionBasedOntile(RoadTiles[tile_index + 1]);
						glm::vec3 begin_pos = getWorldPositionBasedOntile(RoadTiles[tile_index]); //glm::vec3 begin_pos = getWorldPositionBasedOntile(visited_tiles[tile_index].pos);

						glm::vec3 diff = abs((target_pos)-(current_pos))*abs(direction);
						//std::cout << "push bacck diff : ( " << diff.x << " , " << diff.y << " , " << diff.z << " )" << std::endl;

						if (direction.x != 0)
							timer = (abs(1 - (diff.x / 4.0f)));
						else if (direction.z != 0)
							timer = (abs(1 - (diff.z / 4.0f)));


						timer /= VELOCITY;

						re_adjust_timer = false;


					}
					setWorldPosition(getWorldPositionBasedOntile(RoadTiles[tile_index]));

				}
				else // back from tile 
				{

					push_back = false;
					back_from_tile = false;


					setWorldPosition(getWorldPositionBasedOntile(RoadTiles[tile_index]));
					//world_position = getWorldPositionBasedOntile(RoadTiles[tile_index])+glm::vec3(0,2,0);

					setTilePosition(RoadTiles[tile_index]);
					setTargetTilePosition(RoadTiles[tile_index + 1]);


					glm::vec3 current_pos = interpolated_world_position; //x
					glm::vec3 target_pos = target_position;//getWorldPositionBasedOntile(RoadTiles[tile_index + 1]);
					glm::vec3 begin_pos = getWorldPositionBasedOntile(tilePosition); //A

					target_pos_acquired = false;

					glm::vec3 diff = abs((target_pos)-(current_pos))*abs(direction);
					//std::cout << "push bacck diff : ( " << diff.x << " , " << diff.y << " , " << diff.z << " )" << std::endl;

					if (diff.x != 0)
						timer = (abs(1.0f - (diff.x / 4.0f)));
					else if (diff.z != 0)
						timer = (abs(1.0f - (diff.z / 4.0f)));


					timer /= VELOCITY;


				}

				// slowDown(-1.0f);
				adjustVelocity(-0.95f); //0.45

				setDirection(computeDirection(target_tilePosition));
				computeRotation();
			}
			else
			{
				timer = 0.0f;
				push_back = false;
				back_from_tile = false;
				tile_index++;


				setTilePosition(RoadTiles[tile_index]);
				setTargetTilePosition(RoadTiles[tile_index + 1]);

				setDirection(computeDirection(target_tilePosition));
				computeRotation();


			}

			if (tile_index == RoadTiles.size()-1)
			{
				reached_endpoint = true;
				AnimationHandler::addAnimation(getWorldPositionBasedOntile(RoadTiles[RoadTiles.size() - 1]), Animation::COIN_STEALING, nullptr);
				SoundEngine::PlayRandomSoundFrom("coin_cluster", 1, 6, false);
			}
		}


	}
	else if(decomposing)
	{ 
	     //interpolated_world_position += glm::vec3(0.1)*direction;
	     interpolated_world_position = glm::mix(world_position, target_position, timer * VELOCITY );
		 decompose_timer += dt;
	}
	
	
	


	/*
	   Interpolate from current rotation to target rotation based on object's direction
	   in order to animate smoothly the object strafing
	   Interpolation happens in max 1/8 seconds

	   If object is at spawn point rotation interpolates to target rotation instantly
	*/
	if (tilePosition == spawn_point)
	{
		interpolated_rotation = target_rotation;
		rotation = target_rotation;
	}
	else if (timer < 0.25f && interpolated_rotation != target_rotation)
	{
		interpolated_rotation = glm::mix(rotation, target_rotation, timer * 1/0.25f);
	}
	else
	{
		rotation = target_rotation;
		interpolated_rotation = target_rotation;
		rotating = false;
	}

	// Update each pirate's part ( world_position , direction , rotation )
	for (int i = 0; i < 4; i++)
	{
		getIndex(i)->Update(dt);
	}


	if (VELOCITY < 1.0f && !decomposing && !dazzed)
		vel_readjust_counter += dt;
	
	if (dazzed)
		dazzed_timer += dt;

	if (after_dazzed)
		dazzed_cooldown_timer += dt;

	if (dazzed && dazzed_timer > 1.50f)
	{
		dazzed_timer = 0.0f;
		dazzed = false;
		dazzed_cooldown_timer = 0.0f;
		after_dazzed = true;
	}
	
	if (after_dazzed && dazzed_cooldown_timer > 1.0f)
	{
		after_dazzed = false;
		dazzed_cooldown_timer = 0.0f;
	}

	if(!push_back)
		this->dt += dt;

	if(!dazzed)
		timer += dt;
	
	
}

void Pirate::UpdatePosition()
{


}

void Pirate::ReceiveDamage(int dmg , GameObject * object)
{

	AnimationHandler::addAnimation(interpolated_world_position + glm::vec3(0, 3, 0), Animation::DAMAGE, &dmg);

	this->HP -= dmg;
	if (HP <= 0.0f)
	{
		SoundEngine::PlaySound("coin", false);

		alive = false;
		glm::vec3 decompose_dir = interpolated_world_position - object->getWorldPosition();
		deCompose(decompose_dir);

		AnimationHandler::addAnimation(interpolated_world_position + glm::vec3(0, 3.5 , 0), Animation::COIN_COLLECTION, nullptr);
		return;
	}

	if (tile_index == 0)
		return;

	if (!after_dazzed && !dazzed)
	{
		/*
		std::mt19937 rng;
		rng.seed(std::random_device()());
		if (stunRand(rng) == 5)
		{

			getDazzed();
			return;
		}
		*/

		if (dmg >= 24 && HP <= 200)
		{
			getDazzed();
			return;
		}
		else if (dmg >= 24 && HP <= 300 && dmgRand_2(rng) == 1 || dmg > 100)
		{
			getDazzed();
			return;
		}
		else if (dmg >= 24 &&  dmgRand_3(rng) == 2 || dmg > 100)
		{
			getDazzed();
			return;
		}
	}

	if (push_back || back_from_tile || push_back_delay || reseting_movement ||  after_dazzed || dazzed || collision || VELOCITY <= 0.01f)
		return;


	glm::vec2 pos_diff = object->getTilePosition() - tilePosition;


	push_back_factor = glm::length(pos_diff);
	push_back_direction = object->computeDirection(tilePosition);

	if (push_back_direction != this->direction * -1.0f)
		return;

	glm::vec2 push_back_pos = tilePosition + glm::vec2(push_back_direction.x, push_back_direction.z);



	glm::vec3 diffA = abs(interpolated_world_position - target_position);
	glm::vec3 diffB = abs(interpolated_world_position - world_position);
	bool too_close = false;
	bool too_closeB = false;

	float treshold = 0.0000001f; //0.05
	if (this->direction.x != 0)
	{
		if (diffA.x <= treshold || diffB.x <= treshold) // 0.1
			too_close = true;
		if (diffB.x <= treshold)
			too_closeB = true;
	}
	else if (this->direction.z != 0)
	{
		if (diffB.z <= treshold || diffA.z <= treshold)
			too_close = true;
		if (diffB.z <= treshold)
			too_closeB = true;
	}

	bool push = false;

	//too_close = false;
	//too_closeB = false;

	if (tile_index >= 1 && push_back_pos == RoadTiles[tile_index - 1] && !too_close) // back 1 tile distance from current position // >1
	{

#ifdef DEBUGGING_PUSH_BACK

		std::cout << "\n ..ReceiveDamage() : pushing back by 4 units tile Pirate[" << object_index << "]" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] interpolated_position : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " )" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] world_position : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << " )" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] Direction = (" << this->direction.x <<
			" , " << this->direction.y << " , " << this->direction.z << ")" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] interpolated_pos - world_pos = ("
			<< (interpolated_world_position - world_position).x << " , " << (interpolated_world_position - world_position).y
			<< " , " << (interpolated_world_position - world_position).z << ")" << std::endl;
#endif
		this->direction *= -1.0f;
		world_position = interpolated_world_position;
		last_target_pos = target_position;
		target_position = interpolated_world_position + glm::vec3(4.0f)*push_back_direction;
		target_tilePosition = tilePosition;

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] tileIndex before : [" << tile_index << "] " << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] world_tile_pos[" << tile_index << "] : ( " << getWorldPositionBasedOntile(RoadTiles[tile_index]).x << " , "
			<< getWorldPositionBasedOntile(RoadTiles[tile_index]).y << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).z << " ) " << std::endl;
#endif

		//if (tile_index >= 1) // >1
		tile_index--;

		back_from_tile = true;
		push_back = true;
		timer = 0.0f;

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] world_position after : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << " )" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] tileIndex after : [" << tile_index << "] " << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] world_tile_pos[" << tile_index << "] : ( " << getWorldPositionBasedOntile(RoadTiles[tile_index]).x << " , "
			<< getWorldPositionBasedOntile(RoadTiles[tile_index]).y << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).z << " ) " << std::endl;

		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] last_Target_position : ( " << last_target_pos.x << " , " << last_target_pos.y << " , " << last_target_pos.z << " )" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] Target_position : ( " << target_position.x << " , " << target_position.y << " , " << target_position.z << " )" << std::endl;
#endif

		//tilePosition = last_tile_position;
		tilePosition = RoadTiles[tile_index];
		if (tile_index > 1)
			last_tile_position = RoadTiles[tile_index - 2];//last_tile_position = visited_tiles[tile_index - 1].prev;    //last_tile_position = tiles_visited[tile_index - 2];
		else
			last_tile_position = RoadTiles[0];  // last_tile_position = visited_tiles[0].pos; //last_tile_position = tiles_visited[0];

		push = true;
	}

	else if (interpolated_world_position - world_position != glm::vec3(0) && !too_closeB) // push back to last tile
	{

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "\n  ..ReceiveDamage() : pushing back to tile Pirate[" << object_index << "]" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] interpolated_position : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " )" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] world_position : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << " )" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] Direction = (" << this->direction.x <<
			" , " << this->direction.y << " , " << this->direction.z << ")" << std::endl;
		std::cout << " : Pirate[" << object_index << "] interpolated_pos - world_pos = ("
			<< (interpolated_world_position - world_position).x << " , " << (interpolated_world_position - world_position).y
			<< " , " << (interpolated_world_position - world_position).z << ")" << std::endl;
#endif
		this->direction *= -1.0f;
		last_target_pos = target_position;
		target_position = world_position;
		target_tilePosition = tilePosition; // **
		// tilePosition = RoadTiles[tile_index-1];
		world_position = interpolated_world_position;

		target_pos_acquired = false;

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "\n  ..ReceiveDamage() : Pirate[" << object_index << "] tileIndex before : [" << tile_index << "] " << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] world_pos[" << tile_index << "] : ( " << getWorldPositionBasedOntile(RoadTiles[tile_index]).x << " , "
			<< getWorldPositionBasedOntile(RoadTiles[tile_index]).y << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).z << " ) " << std::endl;
#endif
		//target_pos_acquired = false;

		//tile_index--;
		back_from_tile = false;
		push_back = true;
		timer = 0.0f;

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] world_position after : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << " )" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] tileIndex after : [" << tile_index << "] " << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] world_tile_pos[" << tile_index << "] : ( " << getWorldPositionBasedOntile(RoadTiles[tile_index]).x << " , "
			<< getWorldPositionBasedOntile(RoadTiles[tile_index]).y << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).z << " ) " << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] last_Target_position : ( " << last_target_pos.x << " , " << last_target_pos.y << " , " << last_target_pos.z << " )" << std::endl;
		std::cout << "  ..ReceiveDamage() : Pirate[" << object_index << "] Target_position : ( " << target_position.x << " , " << target_position.y << " , " << target_position.z << " )" << std::endl;
#endif

		push = true;
	}


	if (push)
	{
		
		SoundEngine::PlaySound("pushback", false);
		// slowDown(-0.5f);
		push_back_delay = true;
		dt_counter = 0.0f;
	}


	SoundEngine::PlaySound("pain", pain_sound, false);

	
}

void Pirate::getDazzed()
{
	dazzed_timer = 0.0f;
	dazzed = true;

	AnimationHandler::addAnimation(interpolated_world_position + glm::vec3(0, 3, 0), Animation::DIZZY , this);
	SoundEngine::PlaySound("dizzy", false);

}

bool Pirate::isDazzed()
{
	return dazzed;
}

bool Pirate::tryPushBack(glm::vec3 direction, float dist)
{
	/*
	std::cout << "\n-----try push back | Pirate(" << object_index << ") <<0>>" << std::endl;
	std::cout << "    ----Pirate(" << object_index << ") : push : " << push << std::endl;
	std::cout << "    ----Pirate(" << object_index << ") : too_closeA : " << too_close << std::endl;
	std::cout << "    ----Pirate(" << object_index << ") : too_closeB : " << too_closeB << std::endl;
	*/

	if (push_back || back_from_tile || reseting_movement || tile_index == 0 || after_dazzed || VELOCITY <= 0.01f || dazzed || collision) // || push_back_delay
	{
		/*
		std::cout << "    ----Pirate(" << object_index << ") : push_back : " << push_back << std::endl;
		std::cout << "    ----Pirate(" << object_index << ") : back_from_tile : " << back_from_tile << std::endl;
		std::cout << "    ----Pirate(" << object_index << ") : resetting_mov : " << reseting_movement << std::endl;
		*/
		return false;
	}


	push_back_factor = dist;
	push_back_direction = direction;


	glm::vec2 push_back_pos = tilePosition + glm::vec2(push_back_direction.x, push_back_direction.z);


	if (push_back_direction != -1.0f * this->direction)
	{

		//std::cout << "    ----Pirate(" << object_index << ") : push_back_direction : ( " << push_back_direction.x << " , " << push_back_direction.y << " , " << push_back_direction.z << " )" << std::endl;
		//std::cout << "    ----Pirate(" << object_index << ") : direction :  ( " << this->direction.x << " , " << this->direction.y << " , " << this->direction.z << " ) " << std::endl;

		return false;
	}


	glm::vec3 diffA = abs(interpolated_world_position - target_position);
	glm::vec3 diffB = abs(interpolated_world_position - world_position);

	bool too_close = false;
	bool too_closeB = false;

	float treshold = 0.0000001f;
	if (this->direction.x != 0)
	{
		if (diffA.x <= treshold || diffB.x <= treshold) // 0.1
			too_close = true;
		if (diffB.x <= treshold)
			too_closeB = true;
	}
	else if (this->direction.z != 0)
	{
		if (diffB.z <= treshold || diffA.z <= treshold)
			too_close = true;
		if (diffB.z <= treshold)
			too_closeB = true;
	}

	bool push = false;
	
	//too_close = false;
	//too_closeB = false;

	if (tile_index >= 1 && push_back_pos == RoadTiles[tile_index - 1] && !too_close) // back 1 tile distance from current position // >1
	{

#ifdef DEBUGGING_PUSH_BACK

		std::cout << "\n ..tryPushBack() : pushing back by 4 units tile Pirate[" << object_index << "]" << std::endl;
		std::cout << "  ...tryPushBack() : Pirate[" << object_index << "] interpolated_position : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " )" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] world_position : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << " )" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] Direction = (" << this->direction.x <<
			" , " << this->direction.y << " , " << this->direction.z << ")" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] interpolated_pos - world_pos = ("
			<< (interpolated_world_position - world_position).x << " , " << (interpolated_world_position - world_position).y
			<< " , " << (interpolated_world_position - world_position).z << ")" << std::endl;
#endif
		this->direction *= -1.0f;
		world_position = interpolated_world_position;
		last_target_pos = target_position;
		target_position = interpolated_world_position + glm::vec3(4.0f)*push_back_direction;
		target_tilePosition = tilePosition;

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] tileIndex before : [" << tile_index << "] " << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] world_tile_pos[" << tile_index << "] : ( " << getWorldPositionBasedOntile(RoadTiles[tile_index]).x << " , "
			<< getWorldPositionBasedOntile(RoadTiles[tile_index]).y << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).z << " ) " << std::endl;
#endif

		if (tile_index >= 1) // >1
			tile_index--;

		back_from_tile = true;
		push_back = true;
		timer = 0.0f;

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] world_position after : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << " )" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] tileIndex after : [" << tile_index << "] " << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] world_pos[" << tile_index << "] : ( " << getWorldPositionBasedOntile(RoadTiles[tile_index]).x << " , "
			<< getWorldPositionBasedOntile(RoadTiles[tile_index]).y << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).z << " ) " << std::endl;

		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] last_Target_position : ( " << last_target_pos.x << " , " << last_target_pos.y << " , " << last_target_pos.z << " )" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] Target_position : ( " << target_position.x << " , " << target_position.y << " , " << target_position.z << " )" << std::endl;
#endif

		//tilePosition = last_tile_position;
		tilePosition = RoadTiles[tile_index];
		if (tile_index > 1)
			last_tile_position = RoadTiles[tile_index - 2];//last_tile_position = visited_tiles[tile_index - 1].prev;    //last_tile_position = tiles_visited[tile_index - 2];
		else
			last_tile_position = RoadTiles[0];  // last_tile_position = visited_tiles[0].pos; //last_tile_position = tiles_visited[0];

		push = true;
	}

	else if (interpolated_world_position != world_position && !too_closeB) // push back to last tile
	{

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "  ..tryPushBack() : pushing back to tile Pirate[" << object_index << "]" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] interpolated_position : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " )" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] world_position : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << " )" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] Direction = (" << this->direction.x <<
			" , " << this->direction.y << " , " << this->direction.z << ")" << std::endl;
		std::cout << " : Pirate[" << object_index << "] interpolated_pos - world_pos = ("
			<< (interpolated_world_position - world_position).x << " , " << (interpolated_world_position - world_position).y
			<< " , " << (interpolated_world_position - world_position).z << ")" << std::endl;
#endif
		this->direction *= -1.0f;
		last_target_pos = target_position;
		target_position = world_position;
		world_position = interpolated_world_position;

		target_tilePosition = tilePosition;
		//tilePosition = RoadTiles[tile_index - 1];

		target_pos_acquired = false;

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "\n  ..tryPushBack() : Pirate[" << object_index << "] tileIndex before : [" << tile_index << "] " << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] world_tile_pos[" << tile_index << "] : ( " << getWorldPositionBasedOntile(RoadTiles[tile_index]).x << " , "
			<< getWorldPositionBasedOntile(RoadTiles[tile_index]).y << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).z << " ) " << std::endl;
#endif
		//target_pos_acquired = false;

		//tile_index--;
		back_from_tile = false;
		push_back = true;
		timer = 0.0f;

#ifdef DEBUGGING_PUSH_BACK
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] world_position after : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << " )" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] tileIndex after : [" << tile_index << "] " << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] world_pos[" << tile_index << "] : ( " << getWorldPositionBasedOntile(RoadTiles[tile_index]).x << " , "
			<< getWorldPositionBasedOntile(RoadTiles[tile_index]).y << " , " << getWorldPositionBasedOntile(RoadTiles[tile_index]).z << " ) " << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] last_Target_position : ( " << last_target_pos.x << " , " << last_target_pos.y << " , " << last_target_pos.z << " )" << std::endl;
		std::cout << "  ..tryPushBack() : Pirate[" << object_index << "] Target_position : ( " << target_position.x << " , " << target_position.y << " , " << target_position.z << " )" << std::endl;
#endif

		push = true;
	}



	if (push)
	{
		//slowDown(-0.5f);

		push_back_delay = true;
		dt_counter = 0.0f;
	}
	else
	{
		/*
		std::cout << "\nPirate(" << object_index << ") -> too closeA : " << too_close << std::endl;
		std::cout << "Pirate(" << object_index << ") -> too closeB : " << too_closeB << std::endl;
		std::cout << "Pirate(" << object_index << ") -> push_back_pos : ( " << push_back_pos.x << " , " << push_back_pos.y << " ) " << std::endl;
		std::cout << "Pirate(" << object_index << ") -> last_tile_pos  : ( " << RoadTiles[tile_index - 1].x << " , " << RoadTiles[tile_index - 1].y << " )" << std::endl;
		std::cout << "Pirate(" << object_index << ") -> interpolated_world_position - world_position != glm::vec3(0) = " << (interpolated_world_position - world_position != glm::vec3(0));
		*/
	}

	return push;
}

bool Pirate::isAlive()
{
	return alive;
}

bool Pirate::isPushBack()
{
	return push_back;
}

bool Pirate::isResetting()
{
	return reseting_movement;
}

float Pirate::getPushBackFactor()
{
	return push_back_factor;
}

float Pirate::getVelocity()
{
	return VELOCITY;
}

void Pirate::ResetMovement(bool push)
{


	if (push) // * EDW * //!reseting_movement
	{



		glm::vec3 distance = glm::abs((target_position - world_position)*direction);
		glm::vec3 distance_covered = glm::abs((distance - (interpolated_world_position - target_position))*direction);
		glm::vec3 distance_left = glm::abs((distance - distance_covered));

		/*
		std::cout << "\nold_target_pos : ( " << target_position.x << " , " << target_position.y << " , " << target_position.z << " ) " << std::endl;
		std::cout << "distance_covered : ( " << distance_covered.x << " , " << distance_covered.y << " , " << distance_covered.z << " )" << std::endl;
		std::cout << "whole distance : ( " << distance.x << " , " << distance.y << " , " << distance.z << " ) " << std::endl;
		std::cout << "distance left : ( " << distance_left.x << " , " << distance_left.y << " , " << distance_left.z << " ) " << std::endl;
		std::cout << "new_target_pos : ( " << target_position.x << " , " << target_position.y << " , " << target_position.z << " ) " << std::endl;
		*/

		distance_left *= push_back_factor * 0.02f; // 0.01f;
		target_position = interpolated_world_position + distance_left * direction;



		bool behind = false;
		glm::vec3 poss = getWorldPositionBasedOntile(RoadTiles[tile_index + 1]); // temporary

		if (back_from_tile)
		{
			re_adjust_timer = true;
			glm::vec3 diff = (target_position - getWorldPositionBasedOntile(RoadTiles[tile_index + 1]))*direction*-1.0f; // glm::vec3 diff = target_position - getWorldPositionBasedOntile(visited_tiles[tile_index].next);
			

			if (direction.x != 0)
			{
				if (diff.x < 0)
					behind = true;
			}
			else if (direction.z != 0)
			{
				if (diff.z < 0)
					behind = true;
			}

			if (behind)
			{

			}
			else
			{
				re_adjust_timer = true;
				back_from_tile = false;
				tile_index++;
				tilePosition = RoadTiles[tile_index];

			}
		}
		else if (push_back)
		{
			re_adjust_timer = true;
		}

		
	
		
		push_back = true;
		reseting_movement = true;
	}


	else if (!push)
	{
		

		
		world_position = interpolated_world_position;

		if (push_back && back_from_tile)
		{
			world_position = interpolated_world_position;

			glm::vec3 pos_diff = (interpolated_world_position - getWorldPositionBasedOntile(RoadTiles[tile_index + 1]))*direction*-1.0f;  // glm::vec3 pos_diff = (interpolated_world_position - getWorldPositionBasedOntile(visited_tiles[tile_index].next))*direction*(-1.f); 
			bool behind;

			if (pos_diff.x > 0 || pos_diff.z > 0)
				behind = false;
			else
				behind = true;

			if (behind)
			{
				setTilePosition(RoadTiles[tile_index]); ///////////////////////
				setTargetTilePosition(RoadTiles[tile_index + 1]); 
			}
			else
			{
				tile_index++;
				setTilePosition(RoadTiles[tile_index]);
				setTargetTilePosition(RoadTiles[tile_index + 1]);
			}

			world_position = getWorldPositionBasedOntile(RoadTiles[tile_index]) + glm::vec3(0, 2, 0);

			glm::vec3 begin_pos = world_position;
		    glm::vec3 diff = abs((target_position)-(interpolated_world_position))*abs(direction);

		    if (direction.x != 0)
			    timer = (abs(1.0f - (diff.x / 4.0f)));
		    else if (direction.z != 0)
			    timer = (abs(1.0f - (diff.z / 4.0f)));

		    timer /= VELOCITY; // *
			

			if (false)
			{
				std::cout << "\n -- Pirate(" << object_index << ") :: BackFromTile " << std::endl;
				std::cout << "interpolated world position : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " ) " << std::endl;
				std::cout << "target position : ( " << target_position.x << " , " << target_position.y << " , " << target_position.z << " ) " << std::endl;
				std::cout << "world position : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << "  ) " << std::endl;
				std::cout << " Tile index : " << tile_index << std::endl;
				std::cout << " Timer set to : " << timer << std::endl;
				std::cout << "Velocity : " << VELOCITY << std::endl;
			}


		}

		else if (push_back && !back_from_tile)
		{
			
			setTilePosition(RoadTiles[tile_index]); //////////////////
			setTargetTilePosition(RoadTiles[tile_index + 1]);
			
			world_position = getWorldPositionBasedOntile(RoadTiles[tile_index]) + glm::vec3(0, 2, 0);

			glm::vec3 begin_pos = world_position;
			glm::vec3 diff = abs((target_position)-(interpolated_world_position))*abs(direction);

			if (direction.x != 0)
				timer = (abs(1.0f - (diff.x / 4.0f)));
			else if (direction.z != 0)
				timer = (abs(1.0f  - (diff.z / 4.0f)));

			timer /= VELOCITY; // *
			

			if (false)
			{
				std::cout << "\n -- Pirate(" << object_index << ") :: PushBack " << std::endl;
				std::cout << "interpolated world position : ( " << interpolated_world_position.x << " , " << interpolated_world_position.y << " , " << interpolated_world_position.z << " ) " << std::endl;
				std::cout << "target position : ( " << target_position.x << " , " << target_position.y << " , " << target_position.z << " ) " << std::endl;
				std::cout << "world position : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << "  ) " << std::endl;
				std::cout << " Tile index : " << tile_index << std::endl;
				std::cout << " Timer set to : " << timer << std::endl;
				std::cout << "Velocity : " << VELOCITY << std::endl;
			}


		}

		//VELOCITY = 0.02f;
		adjustVelocity(-0.99f); // * ALLAZW AUTO EDW AN KOLLAEI
		reseting_movement = false;

		if (push_back)
			direction *= -1.0f;
		//else
			//timer = 0.0f;
		
		push_back = false;
		back_from_tile = false;
		setDirection(direction);

	}

	if (object_index == 0 && false)
	{
		std::cout << " ..ResetMovement() : Pirate[" << object_index << "]" << std::endl;
		std::cout << " ..ResetMovement() : Pirate[" << object_index << "] Push : " << push << std::endl;
		std::cout << " ..ResetMovement() : Pirate[" << object_index << "] Target Position set to : (" << target_position.x << " , " << target_position.y << " , " << target_position.z << " )" << std::endl;
		std::cout << " ..ResetMovement() : Pirate[" << object_index << "] Direction : ( " << direction.x << " , " << direction.y << " , " << direction.z << " )" << std::endl;
	}

#ifdef DEBUGGING_PUSH_BACK
	std::cout << " ..ResetMovement() : Pirate[" << object_index << "]" << std::endl;
	std::cout << " ..ResetMovement() : Pirate[" << object_index << "] Push : " << push << std::endl;
	std::cout << " ..ResetMovement() : Pirate[" << object_index << "] World Position set to : ( " << world_position.x << " , " << world_position.y << " , " << world_position.z << " )" << std::endl;
	std::cout << " ..ResetMovement() : Pirate[" << object_index << "] Target Position set to : (" << target_position.x << " , " << target_position.y << " , " << target_position.z << " )" << std::endl;
	std::cout << " ..ResetMovement() : Pirate[" << object_index << "] Timer set to  = " << timer << std::endl;
	std::cout << " ..ResetMovement() : Pirate[" << object_index << "] Direction : ( " << direction.x << " , " << direction.y << " , " << direction.z << " )" << std::endl;
#endif


}

void Pirate::slowDown(float factor)
{
	adjustVelocity(1.0f);
	adjustVelocity(factor);
}

void Pirate::adjustVelocity(float offset)
{
	//if (VELOCITY < 1.0f && offset < 0.0f)
		//return;


	//world_position = interpolated_world_position;
	float new_velocity;
	if (VELOCITY + offset > 1.0f)
		new_velocity = 1.0f;
	else if (VELOCITY + offset < 0.01f)
	{
		new_velocity = 0.01f;
		//std::cout << "********VELOCITY : "<<VELOCITY << std::endl;
	}
	else
		new_velocity = VELOCITY + offset;

	timer = timer * VELOCITY / (new_velocity);
	VELOCITY = new_velocity;


	//for (int i = 0; i < parts.size(); i++)
		//getIndex(i)->adjustVelocity(offset);
}

bool Pirate::re_adjustPos()
{
	return adjust_pos;
}

void Pirate::setTiles(std::vector<glm::vec2> * s_tiles)
{
	RoadTiles.tiles = s_tiles;
}

void Pirate::setTilePosition(glm::vec2 pos)
{
	if (!adjust_pos)
		adjust_pos = true;

	if (pos != tilePosition)
		last_tile_position = tilePosition;

	tilePosition = pos;

}

void Pirate::setWorldPosition(glm::vec3 pos)
{
	world_position = pos + glm::vec3(0, 2, 0);
}

void Pirate::setWorldPositionBasedOnTile()
{
	float y = 2.07;
	float z = -18 + 4 * tilePosition.y;
	float x = -18 + 4 * tilePosition.x;

	world_position = glm::vec3(world_center_position.x + x, y, world_center_position.z + z);
	interpolated_world_position = world_position;
}

void Pirate::deCompose(glm::vec3 col_direction)
{

	alive = false;
	decomposing = true;
	//tile_index = -1;

	//if (decomposing)
		//return;
	/*
	if (col_direction != glm::vec3(0))
		direction = col_direction * -1.0f;
	else if (!push_back)
	{
		direction = -direction + glm::vec3(0, 1, 0);
	}
	else
		direction += glm::vec3(0, 1, 0);
		
	*/

	col_direction = glm::normalize(col_direction);
	col_direction.y = 1;

	glm::vec3 distance = glm::vec3(20);
	setTargetPosition(interpolated_world_position + col_direction * distance);
	world_position = interpolated_world_position;
	timer = 0.0f;
	//this->dt = 0.0f;
	VELOCITY = 2.f; // 0.0015
	push_back = false;

	glm::vec3 dir;
	glm::vec3 left_dir;
	glm::vec3 right_dir;
	if (direction.x != 0)
	{
		dir = glm::vec3(0.5 * col_direction.x, 1, 1);
		left_dir = dir * glm::vec3(1, 1, -1);
		right_dir = dir * glm::vec3(1, 1, 1);

	}
		
	else
	{
		dir = glm::vec3(1, 1, 0.5 * col_direction.z);
		left_dir = dir * glm::vec3(1, 1, 1);
		right_dir = dir * glm::vec3(-1, 1, 1);
	}

	

	parts[0]->setTargetPosition(interpolated_world_position + col_direction * distance);
	parts[1]->setTargetPosition(interpolated_world_position + glm::vec3(1, 1, 1) * right_dir * distance);
	parts[2]->setTargetPosition(interpolated_world_position + left_dir * distance);
	parts[3]->setTargetPosition(interpolated_world_position + glm::vec3(1, 1, 1) * right_dir * distance);

	//for(int i = 0; i<parts)
	//slowDown(-0.5f);

	SoundEngine::PlaySound("pain", pain_sound, false);
	SoundEngine::PlayRandomSoundFrom("jump", 1 , 4 , false);




}

bool Pirate::reachedEndPoint()
{
	return reached_endpoint;
}

int Pirate::collectBones()
{
	int temp = bones;
	bones = 0;
	return temp;
}

bool Pirate::collectedBones()
{
	return (bones > 0) ? false : true;
}


void Pirate::setHealthPoints(float value)
{
	HP = value;
	if (HP <= 200)
		bones = 5;
	else if (HP <= 250)
		bones = 6;
	else if (HP <= 300)
		bones = 7;
	else if (HP <= 350)
		bones = 7;
	else
		bones = 8;
}
/*  */






/* TOWER */
Tower::Tower(class GeometryNode * node, class GeometryNode * pointer_node)
	:GameObject(node, pointer_node, GameObject::TOWER)
{
	scale_factor = 0.4; //0.4
	dmg = 0.f;
	penetration = 0.0f;
	shoot_timer = 1.0f;

	Dimensions.CENTER = glm::vec3(-0.0101, 6.3225, -0.0758);
	Dimensions.RADIUS = 6.3852;
	Dimensions.MIN = glm::vec3(-2.6035, -0.0626, -2.6373);
	Dimensions.MAX = glm::vec3(2.583, 12.7077, 2.4856);
	Dimensions.SIZE = glm::vec3(5.1868, 12.7704, 5.1229);
}

void Tower::Update(float dt)
{

	
	transformation_matrix =
		glm::translate(glm::mat4(1.0f), world_position)
		//* glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3())
		* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));

	normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));


	
	
	//updating Tower's projectiles
	for (int i = 0; i < parts.size(); i++)
	{
		if (parts[i]->hasCollision() || parts[i]->projectileMiss())
		{
			delete parts[i];
			parts[i] = 0;
			parts[i] = parts.back();
			parts.pop_back();
		}
	}

	for (int i = 0; i < parts.size(); i++)
	{
		parts[i]->Update(dt);
	}


	timer += dt;
	if (timer > shoot_timer)
		can_shoot = true;
}

void Tower::UpdateParts(float dt)
{

	//updating Tower's projectiles
	for (int i = 0; i < parts.size(); i++)
	{
		if (parts[i]->hasCollision() || parts[i]->projectileMiss())
		{
			delete parts[i];
			parts[i] = 0;
			parts[i] = parts.back();
			parts.pop_back();
		}
	}

	for (int i = 0; i < parts.size(); i++)
	{
		parts[i]->Update(dt);
	}


	timer += dt;
	if (timer > shoot_timer)
		can_shoot = true;
}

bool Tower::canMove()
{

	return true;
}

void Tower::shootAt(GameObject * object)
{
	//soundEngine->play2D("../data/Assets/Sounds/expl1.wav", GL_FALSE);
	GameObject * projectile = new Projectile(pointer_node, this, object);
	parts.push_back(projectile);
	can_shoot = false;
	timer = 0.0f;

}

bool Tower::canShoot()
{
	return can_shoot;
}

float Tower::getDamageMultiplier()
{
	return dmg;
}

float Tower::getPenetrationMultiplier()
{
	return penetration;
}
/* */


/*Cannon Ball*/
Projectile::Projectile(class GeometryNode * node, class GameObject * parent, class GameObject * object)
	:GameObject(node, GameObject::PROJECTILE)
{
	this->parent = parent;
	this->target = object;

	world_position = glm::vec3(parent->getWorldPosition() + glm::vec3(0, 4, 0));
	dmg = parent->getDamageMultiplier();
	penetration = parent->getPenetrationMultiplier();


	speed = 5.0f;
	scale_factor = 0.1f;

	Dimensions.CENTER = glm::vec3(0, 0, 0);
	Dimensions.RADIUS = 1.0;
}

Projectile::Projectile(class GeometryNode * node,int id, class GameObject * parent, class GameObject * object)
	:GameObject(node, id)
{
	this->parent = parent;
	this->target = object;

	world_position = glm::vec3(parent->getInterpolatedPosition() + glm::vec3(0, 0, 1));
	dmg = parent->getDamageMultiplier();
	penetration = parent->getPenetrationMultiplier();


	speed = 5.0f;
	scale_factor = 1.f;
	scale_vector = glm::vec3(0.0, 0.0, 0);

	Dimensions.CENTER = glm::vec3(0, 0, 0);
	Dimensions.RADIUS = 1.0;
}


bool Projectile::projectileMiss()
{
	return miss;
}


void Projectile::Update(float dt)
{
	
	
	if (!target || target->isDecomposing() || target->reachedEndPoint() || target == 0 )
	{
		//std::cout << "target miss" << std::endl;
		miss = true;
		return;
	}

	float cannonball_radius = scale_factor;
	float target_radius = target->getDimensions().RADIUS*target->getScaleFactor();

	if (id == GameObject::PROJECTILE)
	{
		//shooting position
		world_position = parent->getWorldPosition() + glm::vec3(parent->getDimensions().CENTER.x, 9.5, parent->getDimensions().CENTER.z)*parent->getScaleFactor();
		//target position
		target_position = target->getInterpolatedPosition() + target->getDimensions().CENTER*target->getScaleFactor();

		//interpolate to target position
		interpolated_world_position = glm::mix(world_position, target_position, speed*this->dt);
		
	}
	else if(id == GameObject::CASSINI_PROJECTILE)
	{
		target_position = target->getInterpolatedPosition() + target->getDimensions().CENTER * target->getScaleFactor();
		interpolated_world_position = glm::mix(world_position, target_position, speed * this->dt);
	}


	if (pow((interpolated_world_position.x - target_position.x), 2) + pow((interpolated_world_position.y -
		target_position.y), 2) + pow((interpolated_world_position.z - target_position.z), 2) < pow((cannonball_radius + target_radius), 2))
	{
		collision = true;
		rng.seed(std::random_device()());
		target->ReceiveDamage(dmgRand(rng) , parent);
	}
	

	glm::vec3 scale_vec;
	if (id == GameObject::PROJECTILE)
		scale_vec = glm::vec3(scale_factor);
	else
		scale_vec = scale_vector;

	if (id == GameObject::PROJECTILE)
	{
		transformation_matrix =
			glm::translate(glm::mat4(1.0f), interpolated_world_position)
			* glm::scale(glm::mat4(1.0f), scale_vec);

	}

	else if (id == GameObject::CASSINI_PROJECTILE)
	{

		float rot_angle = -atan2(parent->getInterpolatedPosition().y - target->getInterpolatedPosition().y, 
								 parent->getInterpolatedPosition().x - target->getInterpolatedPosition().x) * 180.0f / 3.14569f;

		transformation_matrix =
			glm::translate(glm::mat4(1.0f), interpolated_world_position)
			* glm::rotate(glm::mat4(1.0f) , rot_angle , glm::vec3(0,1,0))
			* glm::scale(glm::mat4(1.0f), scale_vec);

	}
	
	this->dt += dt;
	timer += dt;


	if (timer > 4.0f)
	{
		miss = true;
		collision = true;
	}
}
/* */


/*TERRAIN*/
Terrain::Terrain(class GeometryNode * node)
	:GameObject(node, GameObject::TERRAIN)
{
	world_position = glm::vec3(18, 0, 16); //+0.5
	scale_factor = 20.0f;

	Dimensions.CENTER = glm::vec3(0, 0, 0);
	Dimensions.RADIUS = 1.0f;
}

void Terrain::Update(float dt)
{

	transformation_matrix =
		glm::translate(glm::mat4(1.0f), world_position) //glm::translate(glm::mat4(1.0f), glm::vec3(16, 0, 18))
		* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));

	normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));


	cube_transformation_matrix =  glm::translate(glm::mat4(1.0f), glm::vec3(world_position) + glm::vec3(0, -20.02, 0))
		//* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0))
		* glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
	cube_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(cube_transformation_matrix))));

}

glm::mat4 & Terrain::getCubeTransformationMatrix()
{
	return cube_transformation_matrix;
}

glm::mat4 & Terrain::getCubeNormalMatrix()
{
	return cube_normal_matrix;
}
/* */


/*TILE*/
Tile::Tile(class GeometryNode * node, int id)
	: GameObject(node, id)
{

	if (id == GameObject::ROAD_TILE)
	{
		occupied = true;
		drawable = true;
	}
	else if (id == GameObject::FREE_TILE)
	{
		occupied = false;
		drawable = false;
	}

	Dimensions.CENTER = glm::vec3(0);
	Dimensions.RADIUS = 1.0f;

	scale_factor = 2.0f;
}

void Tile::Update(float dt)
{

	transformation_matrix =
		glm::translate(glm::mat4(1.0f), world_position + glm::vec3(0, 0, 0))
		* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor, 4 * scale_factor, scale_factor));

	normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));


	cube_transformation_matrix =
		glm::translate(glm::mat4(1.0f), world_position + glm::vec3(0, -1.06, 0))
		* glm::scale(glm::mat4(1.0f), glm::vec3(2, 1.05f, 2));

	cube_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(cube_transformation_matrix))));

}

bool Tile::isOccupied()
{
	return occupied;
}

void Tile::Occupy(bool enable)
{
	occupied = enable;
}

glm::mat4 & Tile::getCubeTransformationMatrix()
{
	return cube_transformation_matrix;
}

glm::mat4 & Tile::getCubeNormalMatrix()
{
	return cube_normal_matrix;
}

void Tile::setWorldPositionBasedOnTile()
{
	if (id == GameObject::FREE_TILE)
	{
		float y = 0.1;
		float z = -18 + 4 * tilePosition.y;
		float x = -18 + 4 * tilePosition.x;

		world_position = glm::vec3(world_center_position.x + x, y, world_center_position.z + z);
	}
	else
	{
		float y = 2.1;
		float z = -18 + 4 * tilePosition.y;
		float x = -18 + 4 * tilePosition.x;

		world_position = glm::vec3(world_center_position.x + x, y, world_center_position.z + z);
	}
	interpolated_world_position = world_position;
}

/* */



/*CHEST*/
Chest::Chest(class GeometryNode * node)
	: GameObject(node, GameObject::CHEST)
{
	//rotation = -45.0f;
	scale_factor = 0.09f;
}

void Chest::Update(float dt)
{

	transformation_matrix =
		glm::translate(glm::mat4(1.0f), world_position + glm::vec3(0, 2, 0))
		* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 1, 0))
		* glm::translate(glm::mat4(1.0f), glm::vec3(-0.5, 0, -0.5))
		* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));

	normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));

}


/*ASTEROID*/
Asteroid::Asteroid(class GeometryNode * node)
:GameObject(node , GameObject::ASTEROID)
{
	//scale_factor = 4.0f;

	Dimensions.CENTER = glm::vec3(0, 0, 0);
	Dimensions.RADIUS = 0.50;

	std::uniform_int_distribution<> dist(-1, 1);
	std::mt19937 rng;

	rot_direction = glm::vec3(dist(rng), dist(rng), dist(rng));
}

Asteroid::Asteroid(class GeometryNode * node , int id)
	:GameObject(node, id)
{
	//scale_factor = 4.0f;

	Dimensions.CENTER = glm::vec3(0, 0, 0);
	Dimensions.RADIUS = 0.50;

	std::uniform_int_distribution<> dist(-1, 1);
	std::mt19937 rng;

	if (id == GameObject::PLANET)
		rot_direction = glm::vec3(0.2, 1, 0.2);
	else
		rot_direction = glm::vec3(dist(rng), dist(rng), dist(rng));

}

void Asteroid::Update(float dt)
{
	

	if (id == GameObject::ASTEROID)
	{
		world_position += direction * VELOCITY;
		transformation_matrix =
			glm::translate(glm::mat4(1.0f), world_position)
			* glm::rotate(glm::mat4(1.0f), this->dt*0.5f, glm::vec3(rot_direction)) // random direction rotation too
			//* glm::translate(glm::mat4(1.0f), glm::vec3(30, 0, 0))
			//* glm::rotate(glm::mat4(1.0f), this->dt*0.50f, rot_direction)
			* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));

	}

	else if (id == GameObject::PLANET || id == GameObject::CASSINI)
	{

		transformation_matrix =

			glm::translate(glm::mat4(1.0f), world_position )
			* glm::rotate(glm::mat4(1.0f) , this->dt * 0.1f , glm::vec3(0,1,0))
			* glm::translate(glm::mat4(1.0f) , glm::vec3(60 , 0 , 0))
			//* glm::rotate(glm::mat4(1.0f), this->dt * 0.1f, glm::vec3(0, 1, 0))
			* glm::rotate(glm::mat4(1.0f) , glm::radians(90.f) , glm::vec3(0,-1,1))
			//* glm::rotate(glm::mat4(1.0f) , glm::radians(90.0f))
			* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));


		for (int i = 0; i < parts.size(); i++)
		{
			if (parts[i]->hasCollision() || parts[i]->projectileMiss())
			{
				delete parts[i];
				parts[i] = 0;
				parts[i] = parts.back();
				parts.pop_back();
			}
		}

		for (int i = 0; i < parts.size(); i++)
			parts[i]->Update(dt);

		if (timer > shoot_timer)
			can_shoot = true;
		else
			timer += dt;

	}
	
	normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));
	interpolated_world_position = glm::vec3(transformation_matrix[3][0], transformation_matrix[3][1], transformation_matrix[3][2]);
	
	 //std::cout << "\ndir : (" << direction.x << "," << direction.y << " , " << direction.z << " ) " << std::endl;
	 //std::cout << "vel : " << VELOCITY;
	

	
	
	this->dt += dt;

}

void Asteroid::setRadiusXYZ(glm::vec3 xyz)
{
	this->xyz = xyz;
}

void Asteroid::adjustVelocity(float offset)
{
	VELOCITY = offset;
	if (VELOCITY <= 0.02f)
		VELOCITY = 0.02f;
	else if (VELOCITY > 0.2f)
		VELOCITY = 0.2f;
}

float Asteroid::getVelocity()
{
	return VELOCITY;
}

void Asteroid::shootAt(GameObject * object)
{

	//soundEngine->play2D("../data/Assets/Sounds/expl1.wav", GL_FALSE);
	//GameObject * projectile = new Projectile(nullptr,GameObject::CASSINI_PROJECTILE, this, object);
	
	//parts.push_back(projectile);
	ParticleSystem::addParticleEffect(this, object, ParticleEffect::LASER);
	can_shoot = false;
	timer = 0.0f;

}

bool Asteroid::canShoot()
{
	return can_shoot;
}