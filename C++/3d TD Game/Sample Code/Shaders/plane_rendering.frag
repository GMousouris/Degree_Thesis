#version 330 core
layout(location = 0) out vec4 out_color;


uniform float uniform_has_texture;
uniform sampler2D diffuse_texture;
uniform vec3 uniform_diffuse;
uniform int has_uniform_diffuse = 0;
uniform int uniform_focused;


// Light Source Properties
uniform mat4 uniform_light_projection_matrix;
uniform mat4 uniform_light_view_matrix;

uniform int uniform_cast_shadows;
uniform sampler2D shadowmap_texture;

float uniform_constant_bias = 0.0001;

in vec2 f_texcoord;
in vec3 f_position_wcs;


#define PI 3.14159

float shadow_nearest(vec3 light_space_xyz)
{
	// sample shadow map
	float shadow_map_z = texture2D(shadowmap_texture, light_space_xyz.xy).r;

	// + shaded -> 0.0 
	// - lit -> 1.0
	return (light_space_xyz.z - uniform_constant_bias < shadow_map_z) ? 1.0 : 0.3;

}

// 1 sample per pixel
float shadow(vec3 pwcs)
{
	
	vec4 plcs = uniform_light_projection_matrix * uniform_light_view_matrix * vec4(pwcs, 1.0);
	
	plcs /= plcs.w;
	
	plcs.xy = (plcs.xy + 1) * 0.5;
	
	if (plcs.x < 0.0) return 0.0;
	if (plcs.y < 0.0) return 0.0;
	if (plcs.x > 1.0) return 0.0;
	if (plcs.y > 1.0) return 0.0;

	plcs.z = 0.5 * plcs.z + 0.5;

	
	// sample shadow map
	return shadow_nearest(plcs.xyz);
}



void main(void) 
{	


	vec4 diffuseColor = texture(diffuse_texture, f_texcoord);

	vec3 vec = f_position_wcs.xyz;
	
	if(uniform_focused == 1)
	{
		diffuseColor += vec4(0.0,1,0.0f , 0.0f);
		out_color = vec4( diffuseColor.rgb  , 0.3);	
	}
	
	
	
	float shadow_value = shadow(vec3(vec.x , vec.y , vec.z));
	shadow_value += abs(f_position_wcs.z) * 0.005;
	//shadow_value = 1.0f;

	//if(f_position_wcs.z> 28.0f)
	//{
	     //shadow_value = 1.0f;
	  //   shadow_value = shadow_value/(1 - f_position_wcs.z/35.0f);
    //}

    

	shadow_value = shadow_value/(1 - f_position_wcs.z/50.0f);

	if(shadow_value < 0.2f)
	{
	    shadow_value = 0.2f;
	}
	if(shadow_value > 1.0f)
	{
	    shadow_value = 1.0f;
    }


    float z = 0.45;
    float depth_par = 1.0f;
    if(has_uniform_diffuse == 1)
    {  
        diffuseColor += vec4(uniform_diffuse,0.0)*0.6; //0.8
        //diffuseColor = vec4(uniform_diffuse , 0.0);
        z = 0.65;
        depth_par = (1 - f_position_wcs.z/65.0f);
    }
    depth_par = (1 - 0.3*f_position_wcs.z/60.0f);

    vec3 final_color = vec3(diffuseColor.rgb * shadow_value*1.5 * depth_par);

    for(int i = 0; i<3; i++)
    {
         if(final_color[i]>1.0f)
              final_color[i] = 1.0f;
    }

	out_color = vec4( final_color , z);	
	
	
	
}

