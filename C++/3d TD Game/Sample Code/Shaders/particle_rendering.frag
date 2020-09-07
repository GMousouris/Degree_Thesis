#version 330 core
layout(location = 0) out vec4 out_color;

uniform  float uniform_time;
uniform  int uniform_effect_id;

in float life;
in vec3 velocity;

void main(void) 
{
	vec3 color;
	if(uniform_effect_id == 0)
	{
		color = vec3(1.0f , 0.2f , 0.1f);
	    color = mix(color , vec3(1,1,1) , abs(1 - life) );
	}
	else if(uniform_effect_id == 1)
	{
		color = vec3(0.1,0.1,1);
		color = mix(color , vec3(1,1,1) , abs(1 - life) );

	}

	for(int i = 0; i<3; i++)
		color[i] = clamp(color[i] , 0.1f , 1.0f);
	
	out_color = vec4(color , 1);
}

