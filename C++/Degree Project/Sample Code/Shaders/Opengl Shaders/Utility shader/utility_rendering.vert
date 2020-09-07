#version 330 core
layout(location = 0) in vec3 coord3d;
layout(location = 1) in vec3 v_normal;

uniform int uniform_is_arc;
uniform int uniform_stable_circle;
uniform int uniform_text_render;

uniform int uniform_use_model_matrix;
uniform int uniform_use_parent_matrix;
//uniform int uniform_use_parent_matrix_scaled;
//uniform int uniform_use_parent_matrix_rotated_scaled;

//uniform vec3 uniform_object_center;
uniform mat4 uniform_normal_matrix;
uniform mat4 uniform_model_matrix;
uniform mat4 uniform_parent_matrix;
//uniform mat4 uniform_parent_matrix_scaled;
//uniform mat4 uniform_parent_matrix_rotated_scaled;

uniform mat4 uniform_view_matrix;
uniform mat4 uniform_projection_matrix;

uniform float uniform_camera_scale_factor;
uniform int draw_sphere;


uniform int   uniform_rendering_gradient_grid;
uniform vec3  uniform_gradient_grid_o;
uniform float uniform_gradient_grid_dist;


out vec3 f_pos;
out vec3 f_position_default;
out vec3 f_position_wcs;
out vec3 f_parent_center;
out vec3 f_normal;
out vec3 f_gradient_origin;


mat4 active_model_matrix;

void main(void)
{
    
	mat4 mvp = uniform_projection_matrix * uniform_view_matrix  * uniform_model_matrix;

	vec4 position_wcs = mvp  * vec4( coord3d , 1.0f);
	
	vec4 parent_center = uniform_projection_matrix * uniform_view_matrix  * uniform_parent_matrix * vec4( vec3(0.0f) , 1.0f);
    f_position_wcs     = position_wcs.xyz; 
	f_parent_center    = parent_center.xyz;
	
	
	vec4 gradient_grid_o = uniform_rendering_gradient_grid == 1 ?  mvp * vec4( uniform_gradient_grid_o, 1.0f ) : vec4(0.0f);
	f_gradient_origin    = gradient_grid_o.xyz;
	f_pos                = (uniform_model_matrix * vec4(coord3d,1.0f)).xyz;

	if(draw_sphere == 1)
	{
		f_pos              = (uniform_model_matrix * vec4(coord3d,1.0f)).xyz;
		f_position_default = coord3d;
		f_normal    = (uniform_normal_matrix * vec4(v_normal, 0)).xyz;
		gl_Position = uniform_projection_matrix * uniform_view_matrix * uniform_model_matrix * vec4(coord3d , 1.0f);
	}
	else if(uniform_text_render == 1)
	{
		gl_Position = uniform_projection_matrix * uniform_view_matrix * uniform_model_matrix * vec4(coord3d , 1.0f);
	}
	else if(uniform_stable_circle == 0)
	{
		gl_Position = uniform_projection_matrix * uniform_view_matrix * uniform_model_matrix * vec4(coord3d , 1.0f);
	}
	else if(uniform_stable_circle == 1)
	{
		mat4 trns_matrix =  uniform_view_matrix * uniform_model_matrix;

		vec4 col = trns_matrix[0];
		float d = col.x * col.x + col.y * col.y + col.z * col.z;
		d = sqrt(d);

		float s = uniform_model_matrix[0].x * 1.3f / uniform_camera_scale_factor; 
		d = d * s;
		vec4 u = vec4( d,  0,  0,  0 );
		vec4 v = vec4( 0,  d,  0,  0 );
		vec4 w = vec4( 0,  0,  d,  0 );
		mat4 inv = mat4
		(
			u,
			v,
			w,
			trns_matrix[3]
		);

		gl_Position = uniform_projection_matrix * inv  * vec4(coord3d , 1.0f);
	}

}