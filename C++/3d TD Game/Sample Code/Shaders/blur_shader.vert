#version 330 core
layout(location = 0) in vec3 coord3d;
layout(location = 2) in vec2 texcoord;

out vec2 f_texcoord;

void main()
{
		gl_Position = vec4(coord3d , 1.0);
	    f_texcoord = texcoord;
	    //f_texcoord = (coord3d.xy + 1.0) / 2.0;
};