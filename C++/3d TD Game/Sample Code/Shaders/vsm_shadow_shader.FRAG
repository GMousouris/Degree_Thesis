#version 330 core


in vec4 position_wcs;

out vec4 out_color;

void main(void) 
{
    vec4 v_position = position_wcs;	
/*
	
	
	float depth = v_position.z/v_position.w;
	depth = depth*0.5 + 0.5;

	float moment1 = depth;
	float moment2 = depth * depth;

	float dx = dFdx(depth);
	float dy = dFdy(depth);
	moment2 += 0.25*(dx*dx+dy*dy);
*/
	out_color = vec4( gl_FragCoord.z , gl_FragCoord.z * gl_FragCoord.z , 0.0 , 0.0);
	
}
