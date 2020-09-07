#version 330 core
layout(location = 0) in vec3 coord3d;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

layout(location = 4)  in uint material_id;
layout(location = 5)  in uint has_tex_id;

layout(location = 10) in uint draw_id;

uniform mat4 uniform_model_matrix;
uniform mat4 uniform_normal_matrix;

uniform mat4 uniform_view_matrix;
uniform mat4 uniform_projection_matrix;
uniform int  uniform_is_app_grid;

uniform int  uniform_rendering_utility;

//uniform int uniform_rendering_distance_sphere;
//uniform vec3 uniform_ds_a;
//uniform vec3 uniform_ds_b;

out VS_OUT
{
   vec2 f_texcoord;
   vec3 f_position_wcs;
   vec3 f_normal;
   flat uint draw_id;
   flat uint material_id;
   flat uint has_tex_id;

} vs_out;

void main(void)
{
	
	vec4 position_wcs = uniform_model_matrix * vec4(coord3d, 1.0);

	vs_out.f_position_wcs = position_wcs.xyz;
	vs_out.f_normal       = (uniform_normal_matrix * vec4(normal, 0)).xyz;
	vs_out.f_texcoord     = texcoord;
	vs_out.material_id    = material_id;
	vs_out.has_tex_id     = has_tex_id;
	vs_out.draw_id        = draw_id;
	gl_Position = uniform_projection_matrix * uniform_view_matrix * position_wcs;

}