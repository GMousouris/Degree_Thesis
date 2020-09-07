#version 330
layout(location = 0, index = 0) out vec4 outColor;

uniform sampler2D samplerHDR;

in vec2 varTexCoord0;

uniform vec3 color_a;
uniform vec3 color_b;
uniform vec2 uniform_size;

void main(void)
{   

	
    float u = varTexCoord0.x;
	float v = varTexCoord0.y;

	vec3  final_color = color_a * v + color_b * (1.0f - v);
	outColor = vec4( final_color, 1.0f );
	
}