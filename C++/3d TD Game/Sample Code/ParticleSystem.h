#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include <vector>
#include "ShaderProgram.h"
#include "GameObject.h"




class ParticleEffect
{
public:

	ParticleEffect(glm::vec3 pos, int id);
	ParticleEffect::ParticleEffect(class GameObject * source, class GameObject * target, int id);

	~ParticleEffect();

	bool Init();
	void Update(float dt);
	void Render();
	bool isEnd();


	static enum ID
	{
		swirl,
		emit,
		swirl_1,
		particle_effect_2,
		LASER,
		LOADING_LASER
	};

private:

	int effect_id;

	glm::vec3                                       world_pos;
	glm::vec3                                       target_pos;

	class GameObject *                              object = 0;
	class GameObject * source_obj = 0;
	class GameObject * target_obj = 0;

	std::vector<glm::vec3>							m_particles_position;
	std::vector<glm::vec3>							m_particles_velocity;
	std::vector<float>								m_particles_life;
	float                                           m_continous_time = 0.0f;

	GLuint											m_vbo = 0;
	GLuint                                          m_life = 0;
	GLuint                                          m_velocity = 0;
	GLuint											m_vao = 0;

	float dt = 0.0f;
	float timer = 0.0f;

	bool state = false;
	bool end = false;


};



class ParticleSystem
{
protected:

	

	static std::vector<ParticleEffect *> particle_effects;
	static ShaderProgram  m_particle_rendering_program;

	static glm::mat4 & m_view_matrix;
	static glm::mat4 & m_proj_matrix;


public:

	ParticleSystem();
	~ParticleSystem();

	
	static bool									Init();
	static bool                                 isEmpty();

	static void								    Update(float dt);
	static void								    Render();

	static void                                 addParticleEffect(glm::vec3 pos, int id);
	static void                                 addParticleEffect(glm::vec3 pos,class GameObject * object , int id);
	static void                                 addParticleEffect(glm::vec3 init_pos , glm::vec3 target_pos, int id);
	static void                                 addParticleEffect(GameObject * obj1, GameObject * obj2 , int id);


	static                                      ShaderProgram * getProgram();
	static                                      void setViewMatrix(glm::mat4 & view_matrix);
	static                                      void setProjectionMatrix(glm::mat4 & proj_matrix);

	static                                      bool reloadShaders();
};



#endif
