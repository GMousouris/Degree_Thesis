#version 330 core
layout(location = 0) in vec3 coord3d;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

uniform mat4 uniform_model_matrix;
uniform mat4 uniform_normal_matrix;

uniform mat4 uniform_view_matrix;
uniform mat4 uniform_projection_matrix;

out vec2 f_texcoord;




void main(void)
{
	vec4 position_wcs = uniform_model_matrix * vec4(coord3d, 1.0);
	f_texcoord = texcoord;
	gl_Position = uniform_projection_matrix * uniform_view_matrix * position_wcs;

}

