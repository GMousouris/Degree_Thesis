#include "AnimationHandler.h"

#include <iostream>




/* AnimationHandler*/
std::vector <Animation *> AnimationHandler::animations;
bool AnimationHandler::pause = false;

ShaderProgram * AnimationHandler::program = 0;
GLuint AnimationHandler::FrameBuffer = 0;
glm::mat4 & AnimationHandler::m_view_matrix = glm::mat4(1.0f);
glm::mat4 & AnimationHandler::m_projection_matrix = glm::mat4(1.0f);

GLuint AnimationHandler::quad_vao = 0;
GLuint AnimationHandler::quad_vbo = 0;
GLuint AnimationHandler::quad_texcoords = 0;

std::uniform_real_distribution<> Animation::dir_random(-5,5);
std::mt19937 Animation::rng;


AnimationHandler::AnimationHandler()
{

}


AnimationHandler::~AnimationHandler()
{
	glDeleteVertexArrays(1, &quad_vao);
	glDeleteBuffers(1, &quad_vbo);
}


bool AnimationHandler::Init()
{


	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	GLfloat vertices[] = {
			-1, -1, 0,
			1, -1, 0,
			-1, 1, 0,
			1, 1, 0
	};

	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER , quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	GLfloat quad_texcoord[] = {
			0, 0,
			1, 0,
			0, 1,
			1, 1
	};
	

	glGenBuffers(1, &quad_texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, quad_texcoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_texcoord), quad_texcoord, GL_STATIC_DRAW);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	
	glBindVertexArray(0);

	return true;
}


void AnimationHandler::Update(float dt)
{
	for (int i = 0; i < animations.size(); i++)
	{

		if (animations[i]->isEnd())
		{
			delete animations[i];
			animations[i] = 0;
			animations[i] = animations.back();
			animations.pop_back();
		}
		
	}

	for (int i = 0; i < animations.size(); i++)
	{
		animations[i]->Update(dt);
	}
}


void AnimationHandler::Render()
{
	for (int i = 0; i < animations.size(); i++)
		animations[i]->Render();
}


void  AnimationHandler::Pause(bool state)
{

}


bool AnimationHandler::isEmpty()
{
	return (animations.size() > 0 ? false : true);
}


void AnimationHandler::addAnimation(glm::vec3 world_pos, int id , void * object)
{
	Animation * animation = new Animation(world_pos, id , object);
	animations.push_back(animation);
}

void AnimationHandler::setViewMatrix(glm::mat4 & view_matrix)
{
	m_view_matrix = view_matrix;
}

void AnimationHandler::setProjectionMatrix(glm::mat4 & projection_matrix)
{
	m_projection_matrix = projection_matrix;
}

void AnimationHandler::setFrameBuffer(GLuint & framebuffer)
{
	FrameBuffer = framebuffer;
}

void AnimationHandler::setProgram(ShaderProgram * s_program)
{
	program = s_program;
}

ShaderProgram * AnimationHandler::getProgram()
{
	return program;
}

GLuint * AnimationHandler::getQuadVao()
{
	return &quad_vao;
}
/* */



/* Animation */

Animation::Animation(glm::vec3 world_pos, int id , void * s_object)
	:world_position(world_pos), Id(id)
{
	switch (Id)
	{
	case DIZZY:

		object = (GameObject *)s_object;
		node = AssetManager::GetIndex("star");
		parts.resize(3);
		life = 1.0f;
		scale_factor = 0.15f;
		this->dt = 0.0f;
		timer = 2.0f;
		break;

	case DAMAGE:
		
		damage = *(int *)s_object;
		node = 0;
		parts.resize(3);
		life = 1.0f;
		scale_factor = 0.18f;
		this->dt = 0.0f;
		timer = 2.0f;
		
		if (damage / 100 == 0)
		{
			digits[0] = 0;
			digits[1] = damage / 10;
			digits[2] = damage % 10;
		}
		else
		{
			digits[0] = damage / 100;
			damage = damage % 100;
			digits[1] = damage / 10;
			digits[2] = damage % 10;
			/*

		int a = bones / 100;

		bones = bones % 100;

		int b = bones / 10;

		int c = bones % 10;
			*/
		}
		if (damage >= 22 && damage <= 25)
			scale_factor = 0.25f;
		else if (damage > 25)
			scale_factor = 0.45f;
		

		break;

	case COIN_COLLECTION:

		
		node = AssetManager::GetIndex("coin");
		//parts.resize(2);
		life = 0.23f;
		scale_factor = 1.0f;
		this->dt = 0.0f;
		timer = 2.0f;
		break;

	case COIN_STEALING:

		node = AssetManager::GetIndex("coin");
		life = 1.0f;
		scale_factor = 0.25f;
		parts.resize(10);
		this->dt = 0.0f;
		timer = 1.5f;

		for (int i = 0; i < parts.size(); i++)
		{
			rng.seed(std::random_device()());
			float dir_x = dir_random(rng);
			rng.seed(std::random_device()());
			float dir_z = dir_random(rng);

			random_pars.push_back(glm::vec2(dir_x, dir_z));
		}

		break;

	}

	

}

Animation::~Animation()
{

}

void Animation::Update(float dt)
{

	switch (Id)
	{
		case DIZZY:
		{
			if (!object || object->isDecomposing())
			{
				end = true;
				return;
			}
			
			if (!object->isDazzed())
			{
				end = true;
				return;
			}

			for (int i = 0; i < parts.size(); i++)
			{

				parts[i] =
					glm::translate(glm::mat4(1.0f), world_position)
					* glm::rotate(glm::mat4(1.0f), this->dt * 8.5f + i * 2.0f, glm::vec3(0, 1, 0))
					* glm::translate(glm::mat4(1.0f), glm::vec3(1, 0, 0))
					//* glm::rotate(glm::mat4(1.0f) ,  this->dt * 5.0f , glm::vec3(0,1,0))
					* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))
					* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor * life ));
			}

			life -= dt / 3.0f; // 2.0f
			if (life < 0.001)
				life = 0.001;

			this->dt += dt;

		}
		break;


		case DAMAGE:
		{

			for (int i = 0; i < parts.size(); i++)
			{
				
				parts[i] =
					glm::translate(glm::mat4(1.0f), world_position + glm::vec3(-i * 3.4f * scale_factor, 7.0f * this->dt , 0))
					* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0))
					//* glm::translate(glm::mat4(1.0f), glm::vec3(1, 0, 0))
					//* glm::rotate(glm::mat4(1.0f) ,  this->dt * 5.0f , glm::vec3(0,1,0))
					//* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))
					* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor * 3.0f * life));
			}

			if (damage >= 24 && scale_factor < 0.6f  && !scale_anim)
				scale_factor += 0.02;

			else if(damage >= 24 && scale_factor > 0.18)
			{
				scale_anim = true;
				scale_factor -= 0.02;
				
				if (scale_factor < 0.18f)
					scale_factor = 0.18f;
			}

			life -= dt / 3.5f; // 2.0f
			if (life < 0.0001)
				life = 0.0001;


			

			if (this->dt > 0.8f)
				end = true;

			this->dt += dt;
		}
		break;


		case COIN_COLLECTION:
		{
			transformation_matrix =
				glm::translate(glm::mat4(1.0f), world_position + glm::vec3(0, this->dt * 4.0f, 0))
				* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0))
				* glm::rotate(glm::mat4(1.0f), this->dt * 4.0f, glm::vec3(0, 1, 0))
				//* glm::translate(glm::mat4(1.0f), glm::vec3(1, 0, 0))
				//* glm::rotate(glm::mat4(1.0f) ,  this->dt * 5.0f , glm::vec3(0,1,0))
				//* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))
				* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor * 3.0f * life));

			normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));

			life -= dt / 3.0f;
			if (life < 0.0001)
				life = 0.0001;

			if (this->dt > timer)
				end = true;

			this->dt += dt;
			break;
		}

		case COIN_STEALING:
		{
			for (int i = 0; i < parts.size(); i++)
			{
				glm::vec3 pos_offset = glm::vec3(this->dt  * random_pars[i].x  , 8.0f * sin(this->dt * 2.5f ) , this->dt  * random_pars[i].y  )  * 1.0f;

				parts[i] =

					glm::translate(glm::mat4(1.0f), world_position  + pos_offset)
					* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0))
					* glm::rotate(glm::mat4(1.0f), this->dt * 4.0f, glm::vec3(0, 1, 0))
					* glm::rotate(glm::mat4(1.0f), this->dt * 4.0f * random_pars[i].x , glm::vec3(1, 0, 0))
					* glm::rotate(glm::mat4(1.0f), this->dt * 4.0f * random_pars[i].y , glm::vec3(0, 0, 1))

					//* glm::translate(glm::mat4(1.0f), glm::vec3(1, 0, 0))
					//* glm::rotate(glm::mat4(1.0f) ,  this->dt * 5.0f , glm::vec3(0,1,0))
					//* glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))
					* glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor * 3.0f * life));
			}

			
			life -= dt / 1.5f;
			if (life < 0.0001)
				life = 0.0001;
			

			if (this->dt > 2.0f)
				end = true;

			this->dt += dt;
			break;
		}
	}

	// this->dt += dt;
}

void Animation::Render()
{
	ShaderProgram * program = AnimationHandler::getProgram();

	if (Id == DIZZY)
	{
		glBindVertexArray(node->m_vao);
		for (int i = 0; i < parts.size(); i++)
		{
			glm::mat4 normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(parts[i]))));
			glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(parts[i]));
			glUniformMatrix4fv((*program)["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(normal_matrix));

			for (int j = 0; j < node->parts.size(); j++)
			{

				glm::vec3 diffuseColor = node->parts[j].diffuseColor;
				diffuseColor = glm::vec3(1, 1, 0);
				glm::vec3 specularColor = node->parts[j].specularColor;
				float shininess = node->parts[j].shininess;
				glUniform3f((*program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform3f((*program)["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
				glUniform1f((*program)["uniform_shininess"], shininess);
				glUniform1f((*program)["uniform_has_texture"], (node->parts[j].textureID > 0) ? 1.0f : 0.0f);
				glUniform1i((*program)["uniform_dizzy_state"], 1);
				glUniform1i((*program)["uniform_damage_texture"], 0);
				glUniform1i((*program)["uniform_wall"], 0);

				glBindTexture(GL_TEXTURE_2D, node->parts[j].textureID);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glGenerateMipmap(GL_TEXTURE_2D);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

				glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);

			}

		}
	}

	if (Id == DAMAGE)
	{
		glBindVertexArray(*AnimationHandler::getQuadVao());


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int i = 0; i < parts.size(); i++)
		{
			if (i == 0 && digits[i] == 0)
				continue;

			glm::mat4 normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(parts[i]))));
			glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(parts[i]));
			glUniformMatrix4fv((*program)["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(normal_matrix));

			//glUniform1f((*program)["uniform_has_texture"], 1.0f);
			glUniform1i((*program)["uniform_damage_texture"], 1);
			glUniform1i((*program)["uniform_dizzy_state"], 0);
			glUniform1f((*program)["uniform_life_time"], life);
			glUniform1i((*program)["uniform_damage_value"], damage);
			glUniform1i((*program)["uniform_cannon_ball"], 0);
			glUniform1i((*program)["uniform_meteorite"], 0);

			glUniform1i((*program)["uniform_wall"], 0);


			std::string number = "dmg_numb_" + std::to_string(digits[i]);
			//std::cout << digits[0] << " " << digits[1] << std::endl;
			glBindTexture(GL_TEXTURE_2D, *AssetManager::getTexture(number));


			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		}

		glDisable(GL_BLEND);

	}
	
	if (Id == COIN_COLLECTION)
	{
		glBindVertexArray(node->m_vao);


		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int j = 0; j < node->parts.size(); j++)
		{
			glm::vec3 diffuseColor = node->parts[j].diffuseColor;
			glm::vec3 specularColor = node->parts[j].specularColor;
			float shininess = node->parts[j].shininess;
			glUniform3f((*program)["uniform_diffuse"], diffuseColor.r + 0.2, diffuseColor.g + 0.2, diffuseColor.b);
			glUniform3f((*program)["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f((*program)["uniform_shininess"], shininess);
			glUniform1f((*program)["uniform_has_texture"], (node->parts[j].textureID > 0) ? 1.0f : 0.0f);

			glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(transformation_matrix));
			glUniformMatrix4fv((*program)["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(normal_matrix));
		
			glUniform1i((*program)["uniform_damage_texture"], 0);
			glUniform1i((*program)["uniform_dizzy_state"], 0);
			glUniform1i((*program)["uniform_coin_collection"], 1);
			glUniform1f((*program)["uniform_life_time"], life);
			glUniform1i((*program)["uniform_cannon_ball"], 0);
			glUniform1i((*program)["uniform_meteorite"], 0);
			glUniform1i((*program)["uniform_wall"], 0);
			glUniform1i((*program)["uniform_focused"], 0);

			glBindTexture(GL_TEXTURE_2D, node->parts[j].textureID);
			glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);

			glUniform1i((*program)["uniform_coin_collection"], 0);

		}

		
			

		//glDisable(GL_BLEND);

	}

	if (Id == COIN_STEALING)
	{
		glBindVertexArray(node->m_vao);


		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (int i = 0; i < parts.size(); i++)
		{
			glm::mat4 normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(parts[i]))));

			for (int j = 0; j < node->parts.size(); j++)
			{
				glm::vec3 diffuseColor = node->parts[j].diffuseColor;
				glm::vec3 specularColor = node->parts[j].specularColor;
				float shininess = node->parts[j].shininess;
				glUniform3f((*program)["uniform_diffuse"], diffuseColor.r + 0.2, diffuseColor.g + 0.2, diffuseColor.b);
				glUniform3f((*program)["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
				glUniform1f((*program)["uniform_shininess"], shininess);
				glUniform1f((*program)["uniform_has_texture"], (node->parts[j].textureID > 0) ? 1.0f : 0.0f);

				glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(parts[i]));
				glUniformMatrix4fv((*program)["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(normal_matrix));

				glUniform1i((*program)["uniform_damage_texture"], 0);
				glUniform1i((*program)["uniform_dizzy_state"], 0);
				glUniform1i((*program)["uniform_coin_collection"], 1);
				glUniform1f((*program)["uniform_life_time"], life);
				glUniform1i((*program)["uniform_cannon_ball"], 0);
				glUniform1i((*program)["uniform_meteorite"], 0);
				glUniform1i((*program)["uniform_wall"], 0);
				glUniform1i((*program)["uniform_focused"], 0);

				glBindTexture(GL_TEXTURE_2D, node->parts[j].textureID);
				glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);

				glUniform1i((*program)["uniform_coin_collection"], 0);

			}
		}
	}

	glBindVertexArray(0);

}

glm::mat4 & Animation::getTransformationMatrix()
{
	return transformation_matrix;
}

glm::mat4 & Animation::getNormalMatrix()
{
	return normal_matrix;
}

class GeometryNode * Animation::getNode()
{
	return node;
}

bool Animation::isEnd()
{
	return end;
}
/* */