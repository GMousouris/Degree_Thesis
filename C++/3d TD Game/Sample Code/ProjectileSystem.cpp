#pragma once
#include "ProjectileSystem.h"
#include <iostream>




ProjectileSystem::ProjectileSystem()
{
	program = 0;
}



ProjectileSystem::~ProjectileSystem()
{
	delete sphere;
}


void ProjectileSystem::setShaderProgram(ShaderProgram * program)
{
	this->program = program;
}

void ProjectileSystem::CastShadowStatus(bool state)
{
	cast_shadows = state;
}

bool ProjectileSystem::init()
{
	
	OBJLoader loader;
	// load geometric object 1

	auto mesh = loader.load("../Data/Sphere/Sphere.obj");
	if (mesh != nullptr)
	{
		sphere = new GeometryNode();
		sphere->Init(mesh);
	}
	else
		return false;

	return true;
}



void ProjectileSystem::createProjectile(glm::vec3 & pos, glm::vec3 & dir)
{
	projectile proj(pos, dir, 0.1f, 0.2f);
	projectiles.push_back(proj);
}

void ProjectileSystem::Update()
{
	for (unsigned int i = 0; i < projectiles.size(); i++)
	{
		if (!projectiles[i].isAlive())
		{
			projectiles[i] = projectiles.back();
			projectiles.pop_back();
		}
	}

	for (unsigned int i = 0; i < projectiles.size(); i++)
	{
		projectiles[i].update();
	}
}

void ProjectileSystem::Render()
{

	glBindVertexArray(sphere->m_vao);
	for (unsigned int i = 0; i < projectiles.size(); i++)
	{
		
		glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(projectiles[i].getTransformationMatrix()));
		if (!cast_shadows) {
			glUniformMatrix4fv((*program)["uniform_model_normal_matrix"], 1, GL_FALSE, glm::value_ptr(projectiles[i].getNormalMatrix()));
		}
		for (int j = 0; j < sphere->parts.size(); j++)
		{
			if (!cast_shadows)
			{
				glm::vec3 diffuseColor = glm::vec3(1, 0, 0);
				glm::vec3 specularColor = sphere->parts[j].specularColor;
				float shininess = sphere->parts[j].shininess;

				glUniform3f((*program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform3f((*program)["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
				glUniform1f((*program)["uniform_shininess"], shininess);
				glUniform1f((*program)["uniform_has_texture"], (sphere->parts[j].textureID > 0) ? 1.0f : 0.0f);
				glBindTexture(GL_TEXTURE_2D, sphere->parts[j].textureID);

			}
			glDrawArrays(GL_TRIANGLES, sphere->parts[j].start_offset, sphere->parts[j].count);
		}
	}

	glBindVertexArray(0);
}

int ProjectileSystem::getClosesetProjectile()
{
	int min = 80;
	for (int i = 0; i < projectiles.size(); i++)
	{
		if (projectiles[i].get_dt() < min)
		{
			min = projectiles[i].get_dt();
		}
	}

	return min;
}

//

projectile::projectile(glm::vec3 &pos, glm::vec3 &dir, float vel, float off)
{
	this->pos = pos;
	this->dir = dir;
	this->velocity = vel;
	this->offset = off;
	this->size = 1;
	this->dt = 0;
}

projectile::~projectile()
{

}

void projectile::update()
{

	this->transformation_matrix =
		  glm::translate(glm::mat4(1.0f), glm::vec3(pos.x + dir.x*offset, pos.y+2, pos.z + dir.z*offset))
		* glm::scale(glm::mat4(1.0f), glm::vec3(0.08));

	this->normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(this->transformation_matrix))));

	this->dt++;
	this->offset += this->velocity;
}

bool projectile::isAlive()
{
	if (this->dt > 100)
	{
		return false;
	}
	
	return true;
}

int projectile::get_dt()
{
	return this->dt;
}

glm::mat4 & projectile::getTransformationMatrix()
{
	return this->transformation_matrix;
}

glm::mat4 & projectile::getNormalMatrix()
{
	return this->normal_matrix;
}

