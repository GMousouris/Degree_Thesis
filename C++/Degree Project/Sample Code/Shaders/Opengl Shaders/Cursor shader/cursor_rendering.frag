#version 330

uniform sampler2D uniform_texture;
in vec2 f_texcoord;

layout(location = 0, index = 0) out vec4 out_color;


void main(void) 
{	
	vec4 color = texture(uniform_texture, f_texcoord);
	
	// alpha culling
	if(color.a < 0.3f)
		discard;
	
	float alpha = color.a;
	out_color = vec4(color.rgb, alpha );	
}

