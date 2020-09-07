#version 330
layout(location = 0) out vec4 out_color;

uniform int background = 0;
uniform int uniform_apply;
uniform int uniform_blur_enabled = 0;
uniform sampler2D uniform_texture;
uniform sampler2D uniform_depth;
uniform float uniform_time;
uniform mat4 uniform_projection_inverse_matrix;
uniform int uniform_camera_move;

uniform vec2 uniform_scaleU;

in vec2 f_texcoord;

#define DOF_RADIUS 4 // 0.1
#define BLOOM_RADIUS 3 // 3

#define PI 3.14159


vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) );

vec2 gaussFilter[7] = vec2[](

	vec2(-3.0,	0.015625),
	vec2(-2.0,	0.09375),
	vec2(-1.0,	0.234375),
	vec2(0.0,	0.3125),
	vec2(1.0,	0.234375),
	vec2(2.0,	0.09375),
	vec2(3.0,	0.015625)
);

float gaussian(vec2 uv)
{
	const float sigma = 6;
	const float sigma2 = 2 * sigma * sigma;
	
	return 4*exp(-dot(uv,uv) / sigma2) / (PI * sigma2);
}

float gaussian(vec2 uv, float sigma)
{
	float sigma2 = 2 * sigma * sigma;
	
	return 4*exp(-dot(uv,uv) / sigma2) / (PI * sigma2);
}

void main(void)
{
	vec3 _c = texture2D(uniform_texture, f_texcoord).rgb;
	vec3 color = texture2D(uniform_texture, f_texcoord).rgb;
	vec3 bloom2_color = texture2D(uniform_texture , f_texcoord).rgb;
	vec3 dof_color = texture2D(uniform_texture , f_texcoord).rgb;
	vec3 blur_color = texture2D(uniform_texture , f_texcoord).rgb;
	vec3 effect_color = texture2D(uniform_texture , f_texcoord).rgb;

	vec2 uv = f_texcoord;
	vec2 texSize = textureSize(uniform_texture, 0);

	float brightness = (color.r + color.g + color.b)/3.0f;

	float depth = texture(uniform_depth, f_texcoord.xy).r;
	vec4 pos = vec4(f_texcoord, depth, 1);
	pos.xyz = 2 * pos.xyz - 1;
	pos = uniform_projection_inverse_matrix * pos;
	pos /= pos.w;
	// convert it to a positive number
	float z_coord = -pos.z;
	int dof = 0;

	vec3 diff = abs(_c - vec3(0.2,0.1,0));

if(uniform_apply == 1)
{


	if ( uniform_camera_move == 1 && diff.x > 0.0001 && diff.y > 0.0001 && diff.z > 0.0001 && false)
    {
    	vec2  uv = f_texcoord;
    	vec2 p  = 2 * uv - 1.0;
    	float len = length(p);
    	float freq = 12.0;
    	float speed = 2.0;

    	vec2 dir = p/len;
    	uv += dir * atan(cos(4 * len * freq - uniform_time * speed))*0.01;
    	effect_color = texture2D(uniform_texture , uv).xyz;

    }

    if ( uniform_camera_move == 1 && diff.x > 0.0001 && diff.y > 0.0001 && diff.z > 0.0001 && false)
    {
    	vec2 uv = f_texcoord;
    	float dist = 1.0 - mod(0.3 * uniform_time , 1.0) - uv.y;
    	dist *= dist;
    	float time = exp(1 - 200 * dist)/2.8;
    	float power = time;
    	power *= 0.6;

    	float r = texture(uniform_texture , uv + power * vec2(-0.1 , 0)).r;
    	float g = texture(uniform_texture , uv + power * vec2(-0.05 , 0)).g;
    	float b = texture(uniform_texture , uv + power * vec2(-0.025,  0)).b;

    	effect_color = vec3(r , g , b);

    	//vec2  uv = f_texcoord;
    	vec2 p  = 2.0 * uv - 1.0;
    	float len = length(p);
    	float freq = 12.0;
    	float speed = 2.0;

    	vec2 dir = p/len;
    	uv += dir * cos(len * freq - uniform_time * speed)*0.01;
    	effect_color += texture2D(uniform_texture , uv).xyz *0.50f;

    }



	float RADIUS = 1.1;
	float var = 0.7;

	if(uniform_camera_move == 1)
		var = 1.1f;

	int state = 1;
    if(diff.x > 0.0001 && diff.y >  0.0001 && diff.z > 0.0001)
    {
		for(int i=0;i<16; i++)
	    {
	    	
		    vec3 color2 = texture(uniform_texture, f_texcoord + RADIUS * poissonDisk[i] / texSize).rgb;
		    float lum =  dot(vec3(0.2126, 0.7152, 0.0722), color2);//dot(vec3(0.30, 0.59, 0.11), color2);
		    if( lum < 1 )
		    {
		    		bloom2_color += var*gaussian(poissonDisk[i], RADIUS) * color2 * z_coord/850.0f; //0.3
		    }
	    }
	}
     			

     // dof
     if(diff.x > 0.0001 && diff.y > 0.0001 && diff.z > 0.0001)
     {
	// get the value of the depth buffer
	float depth = texture(uniform_depth, f_texcoord.xy).r;
	vec4 pos = vec4(f_texcoord, depth, 1);
	pos.xyz = 2 * pos.xyz - 1;
	pos = uniform_projection_inverse_matrix * pos;
	pos /= pos.w;
	
	// convert it to a positive number
	float z_coord = -pos.z;
	
	float focus = 32; // focus distance
	float blurclamp = 50; // max blur amount
	float focalRange = 60; // focus range

	if(uniform_camera_move == 1)
	{
		focus = 10;
		focalRange = 20;
	}
	

	float factor = abs(z_coord - focus) / focalRange;

	// clamp the blur intensity
	vec2 dofblur = vec2 (clamp( factor, 0, blurclamp ));
	
	// get the size of the intermediate texture
	vec2 texSize = textureSize(uniform_texture, 0);
	
	float sum = 0.0;
	
	// clear the color value
	//color = vec3(0);
	dof_color = vec3(0);

	// check each corner
	for(int x = -DOF_RADIUS; x <= DOF_RADIUS; ++x)
	{
		for(int y = -DOF_RADIUS; y <= DOF_RADIUS; ++y)
		{
			vec2 offset = 2.25 * vec2(x,y);
			vec3 color2 = texture(uniform_texture, f_texcoord + dofblur * offset / texSize).rgb;
			
			float weight_value = gaussian(offset);
			dof_color += weight_value * color2;
			
			sum += weight_value;
		}
	}

	dof_color /= sum;
     
    }
	



    if(uniform_camera_move == 1 && diff.x > 0.0001 && diff.y > 0.0001 && diff.z > 0.0001) //uniform_blurr == 1 && 
    {   
    	
        //vec2 ScaleU = uniform_scaleU;
    
        vec2 ScaleU = vec2(0.001 , 0.001);
        blur_color = vec3(0);
        blur_color  += vec3(texture2D( uniform_texture , f_texcoord + vec2( -3.0*ScaleU.x, -3.0*ScaleU.y ) ) * 0.015625);
    	blur_color  += vec3(texture2D( uniform_texture , f_texcoord + vec2( -2.0*ScaleU.x, -2.0*ScaleU.y ) )*0.09375);
    	blur_color  += vec3(texture2D( uniform_texture , f_texcoord + vec2( -1.0*ScaleU.x, -1.0*ScaleU.y ) )*0.234375);
    	blur_color  += vec3(texture2D( uniform_texture , f_texcoord + vec2( 0.0 , 0.0) )*0.3125);
    	blur_color  += vec3(texture2D( uniform_texture , f_texcoord + vec2( 1.0*ScaleU.x,  1.0*ScaleU.y ) )*0.234375);
    	blur_color  += vec3(texture2D( uniform_texture , f_texcoord + vec2( 2.0*ScaleU.x,  2.0*ScaleU.y ) )*0.09375);
    	blur_color  += vec3(texture2D( uniform_texture , f_texcoord + vec2( 3.0*ScaleU.x, -3.0*ScaleU.y ) ) * 0.015625);
    	
    	//color = vec3(1);
    
    }




}


//#define VISUALIZE_DEPTH_BUFFER
#ifdef VISUALIZE_DEPTH_BUFFER
	// get the value of the depth buffer
	
	// scale down to make it visible
	z_coord *= 0.01; //0.05
	vec3 depth_color = vec3(z_coord);
#endif


vec3 bloom_color = vec3(color);
//#define BLOOM
#ifdef BLOOM
	
	texSize = textureSize(uniform_texture, 0);
	
	// check each corner
	for(int x = -BLOOM_RADIUS; x <= BLOOM_RADIUS; ++x)
	{
		for(int y = -BLOOM_RADIUS; y <= BLOOM_RADIUS; ++y)
		{
			vec3 color2 = texture(uniform_texture, f_texcoord + 2 * vec2(x,y) / texSize).rgb;
			float lum = dot(vec3(0.30, 0.59, 0.11), color2);
			if(lum > 1.0)
				bloom_color += 0.45*gaussian(vec2(x,y)) * color2;
		}
	}	
#endif	


//#define LSD
#ifdef LSD
//color = z_coord * dof_color + (1-z_coord)*color;
#endif


//#define LSD2
#ifdef LSD2
    z_coord *= 40;
    if(diff.x > 0.001 && diff.y > 0.001 && diff.z > 0.001)
    {
       color = (z_coord)*color + ( 1.0 - z_coord) *depth_color;
    }
#endif




for(int x = 0; x<3; x++)
{

	color[x] = clamp(color[x] , 0 , 1);

}

vec3 final_color;

if(uniform_apply == 0)
	final_color = color;
else
	final_color = dof_color*0.45f + bloom2_color * 0.55f;

if(uniform_blur_enabled == 1)
{
	final_color = blur_color;
}

if(uniform_camera_move == 1)
	final_color = vec3( blur_color * 0.4 + bloom2_color * 0.2 + dof_color * 0.4 );

//if(true)
	//final_color = vec3(effect_color);

out_color = vec4(final_color ,1.0f);
/*
if(true)
{
	out_color = vec4( vec3(1) , 1.0);
	//out_color = vec4(texture2D(uniform_texture , f_texcoord , 0));
}

if(uniform_apply == 0)
{
	out_color = vec4(color , 1.0);
}
else if(uniform_apply == 1)
{
	out_color = vec4(bloom2_color*0.7 + dof_color*0.3 ,1.0);
}
*/

//out_color = vec4(color*0.8 + dof_color*0.2 , 1.0);

//else
//out_color = vec4(color , 1.0f);

//out_color = vec4(color*0.8 +  dof_color*0.2 , 1.0);
//out_color = vec4(color*0.8 + dof_color * 0.5, 1.0);	

}