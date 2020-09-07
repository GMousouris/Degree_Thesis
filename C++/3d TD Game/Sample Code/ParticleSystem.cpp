#include "ParticleSystem.h"
#include "GeometryNode.h"
#include "Tools.h"
#include <algorithm>
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OBJLoader.h"
#include "TextureManager.h"
#include <iostream>


std::vector<ParticleEffect *> ParticleSystem::particle_effects;
ShaderProgram  ParticleSystem::m_particle_rendering_program;

glm::mat4 & ParticleSystem::m_view_matrix = glm::mat4(1.0f);
glm::mat4 & ParticleSystem::m_proj_matrix = glm::mat4(1.0f);

ParticleSystem::~ParticleSystem()
{
	for (int i = 0; i < particle_effects.size(); i++)
		delete particle_effects[i];
}

bool ParticleSystem::Init()
{

	// Create and Compile Particle Shader
	const char * vertex_shader_path = "../Data/Shaders/particle_rendering.vert";
	const char * fragment_shader_path = "../Data/Shaders/particle_rendering.frag";
	m_particle_rendering_program.LoadVertexShaderFromFile(vertex_shader_path);
	m_particle_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path);
	bool initialized =  m_particle_rendering_program.CreateProgram();
	m_particle_rendering_program.LoadUniform("uniform_view_matrix");
	m_particle_rendering_program.LoadUniform("uniform_projection_matrix");
	m_particle_rendering_program.LoadUniform("uniform_time");
	m_particle_rendering_program.LoadUniform("uniform_effect_id");



	return initialized;
}

bool ParticleSystem::isEmpty()
{
	return particle_effects.size() > 0 ? false : true;
}

void ParticleSystem::Update(float dt)
{
	for (int i = 0; i < particle_effects.size(); i++)
	{
		if (particle_effects[i]->isEnd())
		{
			delete particle_effects[i];
			particle_effects[i] = 0;
			particle_effects[i] = particle_effects.back();
			particle_effects.pop_back();

		}
		
	}

	for (int i = 0; i < particle_effects.size(); i++)
	{
		particle_effects[i]->Update(dt);
	}
}

void ParticleSystem::Render()
{
	m_particle_rendering_program.Bind();

	glUniformMatrix4fv(m_particle_rendering_program["uniform_view_matrix"],1 , GL_FALSE , glm::value_ptr(m_view_matrix));
	glUniformMatrix4fv(m_particle_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_proj_matrix));

	for (int i = 0; i < particle_effects.size(); i++)
		particle_effects[i]->Render();

	m_particle_rendering_program.Unbind();
}

void ParticleSystem::addParticleEffect(glm::vec3 pos, int id)
{
	particle_effects.push_back(new ParticleEffect(pos, id));
}

void ParticleSystem::addParticleEffect(GameObject * obj1, GameObject * obj2, int id)
{
	particle_effects.push_back(new ParticleEffect(obj1, obj2, id));
}


ShaderProgram * ParticleSystem::getProgram()
{
	return &m_particle_rendering_program;
}

void ParticleSystem::setViewMatrix(glm::mat4 & view_matrix)
{
	m_view_matrix = view_matrix;
}

void ParticleSystem::setProjectionMatrix(glm::mat4 & proj_matrix)
{
	m_proj_matrix = proj_matrix;
}

bool ParticleSystem::reloadShaders()
{
	return m_particle_rendering_program.ReloadProgram();
}

//

ParticleEffect::ParticleEffect(glm::vec3 pos, int id)
	:world_pos(pos), effect_id(id)
{
	Init();
}


ParticleEffect::ParticleEffect(GameObject * source,GameObject * target, int id)
	: source_obj(source),target_obj(target), effect_id(id)
{
	Init();
}


ParticleEffect::~ParticleEffect()
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_life);
	glDeleteBuffers(1, &m_velocity);
}


bool ParticleEffect::Init()
{

	switch (effect_id)
	{
	case emit:
	{
		timer = 1.f;
	
		m_particles_position.resize(400);
		m_particles_velocity.resize(400);
		m_particles_life.resize(400, 0.f);

		break;
	}

	case swirl:
	{
		timer = 2.0f;

		m_particles_position.resize(400);
		m_particles_life.resize(400);
		m_particles_velocity.resize(400);

		for (int i = 0; i < m_particles_position.size(); ++i)
		{
			m_particles_position[i] = glm::vec3(0);
			m_particles_life[i] = (float)i;
		}

		break;
	}

	case swirl_1:
	{
		timer = 2.0f;

		m_particles_position.resize(400);
		m_particles_life.resize(400);
		m_particles_velocity.resize(400);

		for (int i = 0; i < m_particles_position.size(); ++i)
		{
			m_particles_position[i] = glm::vec3(0);
			m_particles_life[i] = (float)i;
		}

		break;
	}

	case LASER:
	{
		timer = 7.0f;

		m_particles_position.resize(4000);
		m_particles_velocity.resize(4000);
		m_particles_life.resize(4000, 0.f);

		break;
	}

	}


	{
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		glGenBuffers(1, &m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, m_particles_position.size() * sizeof(glm::vec3), &m_particles_position[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,				// attribute index
			3,              // number of elements per vertex, here (x,y,z)
			GL_FLOAT,       // the type of each element
			GL_FALSE,       // take our values as-is
			0,		         // no extra data between each position
			0				// pointer to the C array or an offset to our buffer
		);

		glGenBuffers(1, &m_life);
		glBindBuffer(GL_ARRAY_BUFFER, m_life);
		glBufferData(GL_ARRAY_BUFFER, m_particles_life.size() * sizeof(float), &m_particles_life[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,				// attribute index
			1,              // number of elements per vertex, here (x,y,z)
			GL_FLOAT,       // the type of each element
			GL_FALSE,       // take our values as-is
			0,		         // no extra data between each position
			0				// pointer to the C array or an offset to our buffer
		);

		glGenBuffers(1, &m_velocity);
		glBindBuffer(GL_ARRAY_BUFFER, m_velocity);
		glBufferData(GL_ARRAY_BUFFER, m_particles_velocity.size() * sizeof(glm::vec3), &m_particles_velocity[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,				// attribute index
			3,              // number of elements per vertex, here (x,y,z)
			GL_FLOAT,       // the type of each element
			GL_FALSE,       // take our values as-is
			0,		         // no extra data between each position
			0				// pointer to the C array or an offset to our buffer
		);

		//If there was any errors
		if (Tools::CheckGLError() != GL_NO_ERROR)
		{
			printf("Exiting with error at Renderer::Init\n");
			return false;
		}

	}

	return true;
}


void ParticleEffect::Update(float dt)
{

	switch (effect_id)
	{

	case swirl:
	{
		float movement_speed = 4.f;
		const glm::vec3 center = world_pos;

		for (int i = 0; i < m_particles_position.size(); ++i)
		{
			m_particles_life[i] += dt;
			float life = m_particles_life[i];

			if (life > 2.f)
			{
				float random_pos = rand() / (float)RAND_MAX;
				random_pos *= 2 * 3.14159f;
				m_particles_position[i] = 0.1f * glm::vec3(sin(random_pos), 0, cos(random_pos)) + center;
				m_particles_life[i] = rand() / (float)RAND_MAX;
			}
			else
			{
				float life = m_particles_life[i] + 0 * m_continous_time;

				float random_pos = (i / 20) / 5.f;
				random_pos *= 2.f * 3.14159f;
				m_particles_position[i] = life * glm::vec3(sin(random_pos), 0, cos(random_pos)) + center;
				m_particles_position[i].x = cos(life * this->dt) * sin(random_pos * this->dt) + sin(life * this->dt) * cos(random_pos * this->dt);
				m_particles_position[i].x *= 1.f;
				m_particles_position[i].y = 3.f;
				m_particles_position[i].z = -sin(life * this->dt) * sin(random_pos * this->dt) + cos(life * this->dt) * cos(random_pos * this->dt);
				m_particles_position[i].z *= 1.f;
				m_particles_position[i] = m_particles_life[i] * m_particles_position[i] + center;

				

			}

			if (m_particles_position[i].y > 40)
				m_particles_position[i].y = 40;
		
		}
		
		if (m_continous_time > timer)
		{
			end = true;
		}

		m_continous_time += dt;
		this->dt += dt;
		break;
	}

	case swirl_1:
	{

		float movement_speed = 2.f;
		const glm::vec3 center = glm::vec3(15, 4, 10);

		for (int i = 0; i < m_particles_position.size(); ++i)
		{
			m_particles_life[i] += dt;
			float life = m_particles_life[i];

			if (life > 2.f)
			{
				float random_pos = rand() / (float)RAND_MAX;
				random_pos *= 2 * 3.14159f;
				m_particles_position[i] = 0.1f * glm::vec3(sin(random_pos), 0, cos(random_pos)) + center;
				m_particles_life[i] = rand() / (float)RAND_MAX;
			}
			else
			{
				float life = m_particles_life[i] + 0 * m_continous_time;

				float random_pos = (i / 20) / 5.f;
				random_pos *= 2.f * 3.14159f;
				m_particles_position[i] = life * glm::vec3(sin(random_pos), 0, cos(random_pos)) + center;
				m_particles_position[i].y = cos(life) * sin(random_pos) + sin(life) * cos(random_pos);
				m_particles_position[i].z = 0.f;
				m_particles_position[i].x = -sin(life) * sin(random_pos) + cos(life) * cos(random_pos);
				m_particles_position[i] = m_particles_life[i] * m_particles_position[i] + center;

				
			
				
			}

			if (m_particles_position[i].z > 10)
				m_particles_position[i].z = 1;

		}

		if (m_continous_time > timer)
		{
			end = true;
		}

		m_continous_time += dt;
		this->dt += dt;
		break;
	}

	case emit:
	{

		float movement_speed = 0.5f;


		for (int i = 0; i < m_particles_position.size(); ++i)
		{
			if (!state)
			{
				// the life will be a random value between [0.5 ... 1.0]
				m_particles_life[i] = 0.5f * rand() / float(RAND_MAX) + 0.5f;
				// Create a random velocity. XZ components of velocity will be random numbers between [0...1].
				m_particles_velocity[i] = glm::vec3(rand(), RAND_MAX, rand()) / float(RAND_MAX);
				// Change velocity (X,Y,Z) from [0...1] range to [-1...1]
				m_particles_velocity[i] = 2.f * m_particles_velocity[i] - 1.f;
				// Make the velocity cone smaller
				m_particles_velocity[i] *= 1.f;
				m_particles_velocity[i].y = 0.5f;
				m_particles_velocity[i].z *= 10.0f;
				m_particles_velocity[i].x *= 10.0f;
				// normalize the velocity vector
				//m_particles_velocity[i] = glm::normalize(m_particles_velocity[i]);

				// we have 120 particles that will be emitted from 3 points.
				float pos = (i / 40) / 5.f; //* 3.14159f;
				// each emitter will be positioned on a circle with radius 1.5

				m_particles_position[i] = 2.5f * glm::vec3(sin(pos), 1, cos(pos)) + world_pos;



			}
			else
			{
				m_particles_position[i] += m_particles_velocity[i] * movement_speed * this->dt;
				m_particles_life[i] -= dt;
			}

		}

		state = true;



		if (m_continous_time > timer)
		{
			end = true;
		}

		this->dt += dt;
		m_continous_time += dt;
		break;
	}

	case LASER:
	{
		glm::vec3 dir = target_obj->getInterpolatedPosition() - source_obj->getInterpolatedPosition();

		float movement_speed = 4.f;
		const glm::vec3 center = source_obj->getInterpolatedPosition();

		for (int i = 0; i < m_particles_position.size(); ++i)
		{
			m_particles_life[i] += dt;
			float life = m_particles_life[i];

			if (life > 1.f)
			{
				float random_pos = rand() / (float)RAND_MAX ;
				random_pos *= 2 * 3.14159f;
			    
				m_particles_position[i] = 0.1f *  glm::vec3(sin(random_pos), 0, cos(random_pos)) + center + dir * this->dt;
				m_particles_life[i] = rand() / (float)RAND_MAX;
			}
			else
			{
				float life = m_particles_life[i] + 0 * m_continous_time;

				float random_pos = (i / 20) / 5.f;
				random_pos *= 2.f * 3.14159f;
				m_particles_position[i] = life * glm::vec3(sin(random_pos), 0, cos(random_pos)) + center;
				m_particles_position[i].x = cos(life * this->dt) * sin(random_pos * this->dt) + sin(life * this->dt) * cos(random_pos * this->dt);
				m_particles_position[i].x *= 1.f;
				m_particles_position[i].y = 1.f;
				m_particles_position[i].z = sin(life * this->dt) * sin(random_pos * this->dt) + cos(life * this->dt) * cos(random_pos * this->dt);
				m_particles_position[i].z *= 1.f;
				
				glm::vec3 pos_diff = glm::abs(m_particles_position[i] - target_obj->getInterpolatedPosition());
				if (pos_diff.x < 9 && pos_diff.y < 9 && pos_diff.z < 9)
				{
					
					state = true;
				}
				
				m_particles_position[i] += m_particles_life[i] * dir * this->dt + center;

				

				



			}

			//if (m_particles_position[i].y > 40)
				//m_particles_position[i].y = 40;

		}

		if (m_continous_time > timer || target_obj->isDecomposing() || !target_obj->isAlive())
		{
			end = true;
		}
		else
		{
			std::uniform_int_distribution<> Rand(1 , 4);
			static std::mt19937 rng;
			rng.seed(std::random_device()());
			if(Rand(rng) == 2)
				target_obj->ReceiveDamage(2, source_obj);
		}

		if (!state)
		{
			m_continous_time += dt;
			this->dt += dt;
		}
	
		break;
	}
	}



	// Reupload data to the GPU
	{

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_particles_position.size() * sizeof(glm::vec3), &m_particles_position[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_life);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_particles_life.size() * sizeof(float), &m_particles_life[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_velocity);
		glBufferSubData(GL_ARRAY_BUFFER, 0, m_particles_velocity.size() * sizeof(glm::vec3), &m_particles_velocity[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	}
}


void ParticleEffect::Render()
{
	

	switch (effect_id)
	{

	case swirl:
	{
		glPointSize(2);
		glBindVertexArray(m_vao);
		glUniform1i((*ParticleSystem::getProgram())["uniform_effect_id"], 1);
		glDrawArrays(GL_POINTS, 0, (GLsizei)m_particles_position.size());
		glBindVertexArray(0);

		break;
	}
	case swirl_1:
	{
		glPointSize(2);
		glBindVertexArray(m_vao);
		glUniform1i((*ParticleSystem::getProgram())["uniform_effect_id"], 1);
		glDrawArrays(GL_POINTS, 0, (GLsizei)m_particles_position.size());
		glBindVertexArray(0);

		break;
	}
	case emit:
	{
		glPointSize(2);
		glBindVertexArray(m_vao);
		glUniform1i((*ParticleSystem::getProgram())["uniform_effect_id"], 0);
		glDrawArrays(GL_POINTS, 0, (GLsizei)m_particles_position.size());
		glBindVertexArray(0);

		break;
	}

	case LASER:
	{
		glPointSize(2);
		glBindVertexArray(m_vao);
		glUniform1i((*ParticleSystem::getProgram())["uniform_effect_id"], 1);
		glDrawArrays(GL_POINTS, 0, (GLsizei)m_particles_position.size());
		glBindVertexArray(0);

		break;
	}

	}
	
}


bool ParticleEffect::isEnd()
{
	return end;
}