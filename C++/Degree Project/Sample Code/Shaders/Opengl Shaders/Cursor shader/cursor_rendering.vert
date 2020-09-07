#version 330
layout(location = 0) in vec4 coord3d;
layout(location = 8) in vec2 texcoord;

uniform mat4 uniform_model_matrix;
uniform mat4 uniform_view_matrix;
uniform mat4 uniform_projection_matrix;

uniform int transform;

out vec2 f_texcoord;

void main(void)
{
    //if( transform == 0 )
    {
	  gl_Position = coord3d;
	  f_texcoord  = texcoord;
	}
	//else
	//{
	   //gl_Position = (uniform_model_matrix * vec4(coord3d,1.0f)).xyz;
	   //gl_Position = uniform_projection_matrix  * uniform_model_matrix * vec4(coord3d , 1.0f);
	//}
}