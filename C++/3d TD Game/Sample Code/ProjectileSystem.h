#pragma once

#include "GLEW\glew.h"
#include "glm\glm.hpp"

#include "GeometryNode.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OBJLoader.h"
#include "ShaderProgram.h"




class projectile {

private:

	glm::vec3 pos;
	glm::vec3 dir;
	float velocity;
	float offset;
	float dt;
	float size;

	glm::mat4 transformation_matrix;
	glm::mat4 normal_matrix;

public:
	projectile(glm::vec3 &pos, glm::vec3 &dir, float vel, float off);
	~projectile();
	bool isAlive();
	void update();
	void kill();
	int get_dt();

	glm::mat4 & getTransformationMatrix();
	glm::mat4 & getNormalMatrix();
};


class ProjectileSystem
{

private:

	ShaderProgram * program;
	class GeometryNode* sphere;
	int projectiles_count = 0;
	std::vector<projectile> projectiles;
	bool cast_shadows = false;
	



public:

	ProjectileSystem();
	~ProjectileSystem();

	void setShaderProgram(ShaderProgram * program);
	void CastShadowStatus(bool state);
	void createProjectile(glm::vec3 &pos, glm::vec3 &dir);
	int getClosesetProjectile();
	bool init();
	void Update();
	void Render();

	

private:
	
	


};
