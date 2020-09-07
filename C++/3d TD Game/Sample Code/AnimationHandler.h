#pragma once


#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>
#include "ParticleSystem.h"
#include "ShaderProgram.h"
#include "GeometryNode.h"
#include "AssetManager.h"
#include "GameObject.h"



class Animation
{

public:

	static enum ID 
	{
		DIZZY,
		FALLING,
		DAMAGE,
		COIN_COLLECTION,
		COIN_STEALING
	};

	Animation(glm::vec3 world_position, int id , void * object);

	~Animation();

	void Update(float dt);
	void Render();
	void Pause(bool state);
	bool isEnd();


private:

	int Id;
	
	float scale_factor = 1.0f;

	float dt = 0;
	float timer = 0;
	float life = 1.0f;
	float speed = 1.0f;
	bool end = false;

	bool pause = false;
	glm::vec3 world_position;
	glm::mat4 transformation_matrix;
	glm::mat4 normal_matrix;
	std::vector<glm::mat4> parts;
	std::vector<glm::vec2> random_pars;


	GeometryNode * node = 0;
	class GameObject * object = 0;
	int damage;
	int digits[3];
	bool scale_anim = false;

	glm::mat4 & getTransformationMatrix();
	glm::mat4 & getNormalMatrix();
	GeometryNode * getNode();

	static std::uniform_real_distribution<> dir_random;
	static std::mt19937 rng;

};


class AnimationHandler
{
public:

	AnimationHandler();
	~AnimationHandler();

	static bool Init();
	static void Update(float dt);
	static void Render();
	static void Pause(bool state);
	static bool isEmpty();

	static void addAnimation(glm::vec3 world_pos , int id , void * object);
	//static void addAnimation(glm::vec3 world_pos, int id, int damage);

	
	static void setViewMatrix(glm::mat4 & view_matrix);
	static void setProjectionMatrix(glm::mat4 & projection_matrix);
	static void setFrameBuffer(GLuint & framebuffer);

	static void setProgram(ShaderProgram * program);
	static ShaderProgram * getProgram();
	static GLuint * getQuadVao();

private:

	static std::vector <Animation *> animations;
	static GLuint FrameBuffer;
	static glm::mat4 & m_view_matrix;
	static glm::mat4 & m_projection_matrix;
	static bool pause;

	static GLuint quad_vao, quad_vbo, quad_texcoords;

	static ShaderProgram * program;



};

