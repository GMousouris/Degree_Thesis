#version 330 core
layout(location = 0) in vec3 coord3d;
layout(location = 1) in float particle_life;
layout(location = 2) in vec3 particle_velocity;

uniform mat4 uniform_view_matrix;
uniform mat4 uniform_projection_matrix;

out float life;
out vec3 velocity;


void main(void) 
{
    life = particle_life;
    velocity = vec3(particle_velocity);
	gl_Position = uniform_projection_matrix * uniform_view_matrix * vec4(coord3d, 1.0);
}
