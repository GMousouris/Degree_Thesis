#version 330 core
layout(location = 0) out vec4 out_color;

uniform sampler2D uniform_texture;


in vec2 f_texcoord;


void main(void) 
{	

	vec4 color = texture2D(uniform_texture, f_texcoord);

	out_color = vec4(color.rgb, color.a);	
}

