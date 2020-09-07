#version 330 core
layout(location = 0) out vec4 out_color;

uniform vec3 uniform_diffuse;
uniform vec3 uniform_specular;
uniform float uniform_shininess;
uniform float uniform_has_texture;
uniform sampler2D diffuse_texture;
uniform int uniform_focused;
uniform int uniform_shading_mode;
uniform int uniform_testing_mode;

uniform int uniform_damage_texture = 0;
uniform float uniform_life_time = 1.0f;
uniform int uniform_dizzy_state = 0;
uniform int uniform_coin_collection = 0;
uniform int uniform_cannon_ball = 0;
uniform int uniform_damage_value = 0;
uniform int uniform_meteorite = 0;
// Camera Properties
uniform vec3 uniform_camera_position;

// Light Source Properties
uniform mat4 uniform_light_projection_matrix;
uniform mat4 uniform_light_view_matrix;
uniform vec3 uniform_light_position;
uniform vec3 uniform_light_direction;
uniform vec3 uniform_light_color;
uniform float uniform_light_umbra;
uniform float uniform_light_penumbra;
uniform int uniform_cast_shadows;
uniform sampler2D shadowmap_texture;
uniform sampler2D shadowmap_color_texture;

float uniform_constant_bias = 0.00004;



in vec2 f_texcoord;
in vec3 f_position_wcs;
in vec4 position_wcs;
in vec3 f_normal;


#define NEAR 0.1
#define PI 3.14159
#define uvLightSize 2.9 //2.9
#define samplesNumber 16
#define blockersNumber 16



vec2 poissonDisk[16] = vec2[](
	vec2(-0.94201624, -0.39906216),
	vec2(0.94558609, -0.76890725),
	vec2(-0.094184101, -0.92938870),
	vec2(0.34495938, 0.29387760),
	vec2(-0.91588581, 0.45771432),
	vec2(-0.81544232, -0.87912464),
	vec2(-0.38277543, 0.27676845),
	vec2(0.97484398, 0.75648379),
	vec2(0.44323325, -0.97511554),
	vec2(0.53742981, -0.47373420),
	vec2(-0.26496911, -0.41893023),
	vec2(0.79197514, 0.19090188),
	vec2(-0.24188840, 0.99706507),
	vec2(-0.81409955, 0.91437590),
	vec2(0.19984126, 0.78641367),
	vec2(0.14383161, -0.14100790)
	);

int blockers = 0;

float random(vec3 seed, int i) {
	vec4 seed4 = vec4(seed, i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}


float reduceLightBleeding(float p_max, float amount)
{
    return clamp((p_max-amount)/ (1.0-amount), 0.0, 1.0);
}

float linstep(float low, float high , float v)
{
	return clamp( (v - low )/(high - low) , 0.0 ,1.0);
}



float FindBlockerDistance_v1(vec3 light_space_xyz , float dist)
{


	blockers = 0;
	float avgBlockerDistance = 0;
	float zReceiver = light_space_xyz.z;
	float searchWidth =   uvLightSize  * ( (zReceiver - NEAR) / light_space_xyz.z );

	//float bias = light_space_xyz.z - BIAS;
	float dotNL = max(dot(normalize(uniform_light_position - light_space_xyz.xyz), normalize(f_normal)), 0.0001);
	uniform_constant_bias = 0.000000117* tan(cos(dotNL));
	//uniform_constant_bias = clamp(uniform_constant_bias, 0, 0.0001);
	float bias = light_space_xyz.z - uniform_constant_bias;

	ivec2 shadow_map_size = textureSize(shadowmap_texture, 0);
	float xOffset = 1.f / shadow_map_size.x;
	float yOffset = 1.f / shadow_map_size.y;



	for (int i = 0; i < blockersNumber; i++)
	{

		
		int index = int(16.0*random(f_position_wcs.xyy, i))%16;

		vec2 offset1 = vec2(xOffset,yOffset)*i  +  poissonDisk[i] * searchWidth;
		vec2 offset2 = poissonDisk[index] * searchWidth; //80.0f
		vec2 offset3 = (vec2(xOffset,yOffset)*i + poissonDisk[index])  *  searchWidth;

		float z = texture(shadowmap_texture , light_space_xyz.xy + poissonDisk[i]*searchWidth , 0).r;
		
		if (z < bias) // z < light_space_xyz.z
		{
			blockers++;
			avgBlockerDistance += z;
		}
	}
	
	
	return avgBlockerDistance / blockers;
	
}

float FindBlockerDistance_v2(vec3 light_space_xyz , float dist)
{


	blockers = 0;
	float avgBlockerDistance = 0;
	float zReceiver = light_space_xyz.z;
	float searchWidth = uvLightSize * ( zReceiver - NEAR )/(zReceiver);

	/*
	vec3 vertex_to_light_direction = (uniform_light_position - light_space_xyz.xyz);
	float distance = distance(uniform_light_position, light_space_xyz.xyz) + 0.000001;
	float searchWidth2 = uvLightSize * ( (light_space_xyz.z )/ distance );
	*/
	


	int start = -2;
	int end = -start-1;

	//float dotNL = max(dot(normalize(uniform_light_position - light_space_xyz.xyz), normalize(f_normal)), 0.0001);
	uniform_constant_bias = 0.00000117;//* tan(cos(dotNL));
	float bias = uniform_constant_bias;


	ivec2 shadow_map_size = textureSize(shadowmap_texture, 0);
	float xOffset = 1.0f / shadow_map_size.x;
	float yOffset = 1.0f / shadow_map_size.y;

	vec2  uv = light_space_xyz.xy;

	int poisson = 0;
	vec2 poisson_var;


	int c = 0;

	for(int i = start; i<= end; i++)
	{
		for(int j = start; j<= end; j++)
		{

			/* poisson var */
			int index = int(16.0*random(light_space_xyz.xyy, c))%16;
			if(poisson == 1)
				poisson_var = poissonDisk[c]; // abs(i+j)
			else
				poisson_var = vec2(0.0f);
			//

			vec2 offset1 = vec2(i , j) * 1.0f  + poisson_var;
			vec2 offset2 = vec2(i , j) * vec2(xOffset,yOffset)*1.f + poisson_var / 950; // 450

			vec2 offset = offset2;

			float z = texture2D(shadowmap_texture , uv + offset * searchWidth * 1.1f   , 0).r;

			if(z < zReceiver - bias)
			{
				blockers++;
				avgBlockerDistance += z;
			}

			c++;
		}

	}

	if(blockers == 0)
		return -1;
	return avgBlockerDistance/(blockers);
	
}


float PCF_FILTER(vec3 light_space_xyz , float uvRadius , int version , float blockersDist)
{


	int start;
	int end;
	int pivot;

	if(version == 1) // 6 x 6 pcf kernel
	{
		start = -3;
		end = -start -1;
		pivot = 1;
	}
	else if(version == 2) // 6 x 6 pcf kernel
	{
		start = -3;
		end = -start -1;
		pivot = 1;

	}
	else if(version == 3) // 12 x 12 pcf kernel
	{
		start = -6; // -4
		end = -start -1;
		pivot = 1;
	}

	float dotNL = max(dot(normalize(uniform_light_position - light_space_xyz.xyz), normalize(f_normal)), 0.0001);
	uniform_constant_bias = 0.0000145;//* tan(cos(dotNL));
	float z = light_space_xyz.z - uniform_constant_bias;


	ivec2 shadow_map_size = textureSize(shadowmap_texture, 0);
	float xOffset = 1.0f / shadow_map_size.x;
	float yOffset = 1.0f / shadow_map_size.y;

	vec2  uv = light_space_xyz.xy;

	float u_ratio = mod(uv.x, xOffset) / xOffset;
	float v_ratio = mod(uv.y, yOffset) / yOffset;
	float u_opposite = 1.0 - u_ratio;
	float v_opposite = 1.0 - v_ratio;

	int poisson = 1;
	vec2 poisson_var;


	float sum = 0.0f;
	float count = 0;

	int c = 0;
	
	float shadow_color; //= 0.0075 * blockers;
	
	/*
	shadow_color = 4.50075 * (1.0f/(blockers));
	shadow_color = 65.0f * (1.0/((blockers * blockers) ));
	shadow_color =  1.0f / (0.009f * (blockers * blockers) );
	*/

	//shadow_color = 0.0075 * blockers;
	//shadow_color = (4.5f /(blockers));
	//shadow_color = 65.5f /(blockers*blockers);
	shadow_color = 0.0f;
	shadow_color = clamp( shadow_color , 0.0 , 0.35);
	if(uniform_testing_mode == 1) 
	    shadow_color = 0.0f;

	
	for(int i = start; i<= end; i += pivot)
	{
		for(int j = start; j<= end; j += pivot)
		{

			/* poisson var */
			int index = int(16.0*random(light_space_xyz.xyy, c ))%16;
			if(poisson == 1)
			{
				if(version == 1)
					poisson_var = poissonDisk[c];
				else if (version == 2)
					poisson_var = poissonDisk[abs(i+j)];
				else  if (version == 3)
					poisson_var = poissonDisk[index]; // random indexed Poisson var
				
			}
			else
				poisson_var = vec2(0.0f);

			
			// offsets
			vec2 offset1 = vec2(i , j) *1.0f + poisson_var / 0.5f; //0.5
			vec2 offset2 = vec2(i , j) * 1.0f + poisson_var / 1.f;
			vec2 offset3 = vec2(i,j) * vec2(xOffset , yOffset) + poisson_var;
			
			vec2 offset;
			if(version == 1)
				offset = offset2;
			else if(version == 2)
				offset = offset1;
			else if(version == 3)
				offset = offset1;


			/* sample shadow map method 1 
			float bottomLeft = (texture(shadowmap_texture, uv + (offset ) * uvRadius   ).r > z) ? u_opposite : shadow_color;
			float bottomRight = (texture(shadowmap_texture,  uv + (offset ) * uvRadius + vec2(xOffset,0)    ).r > z) ? u_ratio : shadow_color;
			float topLeft = (texture(shadowmap_texture, uv + (offset ) * uvRadius + vec2(0 , yOffset)    , 0 ).r > z) ? u_opposite : shadow_color;
			float topRight = texture(shadowmap_texture, uv + (offset ) * uvRadius + vec2(xOffset , yOffset)  ).r > z ? u_ratio : shadow_color;
			float factor = (bottomLeft + bottomRight) * v_opposite + (topLeft + topRight) * v_ratio;
			/* */

			// sample shadow map method 2
		    float factor = (texture(shadowmap_texture , uv + offset * uvRadius  ,0).r > z ) ? 1.0f : shadow_color;

		    sum += factor;
			count++;
		}

		c++;

	}

	return sum / count;

}


float shadow_PCF_test(vec3 light_space_xyz , float uvRadius)
{
	int vers1 = 1;
	int vers2 = 0;
	int interpolation = 0;

	float dotNL = max(dot(normalize(uniform_light_position - light_space_xyz.xyz), normalize(f_normal)), 0.0001);
	uniform_constant_bias = 0.00001947;
	uniform_constant_bias = clamp(uniform_constant_bias, 0, 0.001);
	float z = light_space_xyz.z - uniform_constant_bias;

	float pivot = 1.f;

	int begin = -3;
	int end = -begin;
	int dx = 1;

	int poison = 1;
	int poison2 = 1;


	ivec2 shadow_map_size = textureSize(shadowmap_texture, 0);
	float xOffset = pivot / shadow_map_size.x;
	float yOffset = pivot / shadow_map_size.y;

	vec2 uv = light_space_xyz.xy;

	float sum = 0;
	float count = 0;
	int c = 0;

	float shadow_color = 0.0f;

	for (int i = begin; i <= end; i += dx)
	{
		for (int j = begin; j <= end; j += dx)
		{

			int index = int(16.0*random(f_position_wcs.xyy, c))%16;

			vec2 poisson_var = vec2(1.0f);
			if (poison == 1)
				poisson_var = poissonDisk[c];
			else
				poisson_var = vec2(0.0f);

			float u_ratio = mod(uv.x, xOffset) / xOffset;
			float v_ratio = mod(uv.y, yOffset) / yOffset;
			float u_opposite = 1.0 - u_ratio;
			float v_opposite = 1.0 - v_ratio;

			vec2 o = mod(floor(uv), 2.0);

			
			vec2 offset1 = vec2(i,j);
			vec2 offset2 = vec2(i,j) * vec2(xOffset , yOffset);
			vec2 offset = offset2;

			float bottomLeft = (texture(shadowmap_texture, uv + offset + o).r > z) ? u_opposite : shadow_color;
			float bottomRight = (texture(shadowmap_texture, uv + offset + vec2(xOffset, 0 ) + o).r > z) ? u_ratio : shadow_color;
			float topLeft = (texture(shadowmap_texture, uv + offset + vec2(0 , yOffset) + o, 0 ).r > z) ? u_opposite : shadow_color;
			float topRight = texture(shadowmap_texture, uv + offset + vec2(xOffset, yOffset) + o ).r > z ? u_ratio : shadow_color;
			float factor = (bottomLeft + bottomRight) * v_opposite + (topLeft + topRight) * v_ratio;

			// factor = texture(shadowmap_texture , uv + offset  , 0 ).r > z ? 1.0f : shadow_color;
			
			sum += factor;
			count++;

		}

		c++;
	}
	// compute the weights of the neighboring pixels


	return sum / count;
}

float shadow_PCF(vec3 light_space_xyz , float dist)
{
	int vers1 = 1;
	int vers2 = 0;
	int interpolation = 0;

	float pivot = 1.f;

	


	int begin = -3;
	int end = -begin-1;
	int dx = 1;

	int poison = 1;
	int poison2 = 1;


	ivec2 shadow_map_size = textureSize(shadowmap_texture, 0);
	float xOffset = pivot / shadow_map_size.x;
	float yOffset = pivot / shadow_map_size.y;

	float sum = 0;
	int c = 0;
	float count = 0;

	vec2 uv = light_space_xyz.xy;

	for (int i = begin; i <= end; i += dx)
	{
		for (int j = begin; j <= end; j += dx)
		{

			int index = int(16.0*random(f_position_wcs.xyy, c))%16;

			vec2 poisson_var = vec2(1.0f);
			if (poison == 1)
				poisson_var = poissonDisk[abs(i+j)];


			//vec2 uv = light_space_xyz.xy + vec2(xOffset*(i), yOffset*(j)) + poisson_var / .9f;

			float u_ratio = mod(uv.x, xOffset) / xOffset;
			float v_ratio = mod(uv.y, yOffset) / yOffset;
			float u_opposite = 1.0 - u_ratio;
			float v_opposite = 1.0 - v_ratio;


			float dotNL = max(dot(normalize(uniform_light_position - light_space_xyz.xyz), normalize(f_normal)), 0.0001);
			uniform_constant_bias = 0.00002;
			//uniform_constant_bias = clamp(uniform_constant_bias, 0, 0.001);
			
			float z = light_space_xyz.z - uniform_constant_bias;

			float shadow_color = 0.25;

			vec2 offset = vec2(i ,j) * vec2(xOffset , yOffset) * poisson_var / 1.5f;
			
			// compute the shadow percentage
			/*
			float bottomLeft = (texture(shadowmap_texture, uv + offset).r > z) ? u_opposite : shadow_color;
			float bottomRight = (texture(shadowmap_texture, uv + offset + vec2(xOffset, 0 )).r > z) ? u_ratio : shadow_color;
			float topLeft = (texture(shadowmap_texture, uv + offset + vec2(0 , yOffset), 0 ).r > z) ? u_opposite : shadow_color;
			float topRight = texture(shadowmap_texture, uv + offset + vec2(xOffset, yOffset)).r > z ? u_ratio : shadow_color;
			float factor = (bottomLeft + bottomRight) * v_opposite + (topLeft + topRight) * v_ratio;
			*/

			float factor = texture(shadowmap_texture , uv + offset , 0 ).r > z ? 1.0f : shadow_color;			
			sum += factor;
			count++;
		}

		c++;
	}
	// compute the weights of the neighboring pixels


	return sum / count;
}


float shadow_PCSS(vec3 light_space_xyz , int version , float dist)
{


	float blockerDistance = FindBlockerDistance_v2(light_space_xyz , dist);
	
	if(blockerDistance < 0)
		return 1.0f;

	
    float penumbraWidth =  (light_space_xyz.z - blockerDistance)*uvLightSize  /(blockerDistance);
    //float uvRadius =  penumbraWidth * uvLightSize  * NEAR /(  light_space_xyz.z  );
    float uvRadius = penumbraWidth;

    return PCF_FILTER(light_space_xyz , uvRadius , version - 1 , blockerDistance);

}


float shadow_VSM(vec3 light_space_xyz)
{

	vec2 moments = texture2D(shadowmap_texture, light_space_xyz.xy).rg;

	float depth = texture2D(shadowmap_color_texture , light_space_xyz.xy).r;

	float bias = 0.000015;
	
	float distance = light_space_xyz.z - bias;

		
		// Surface is fully lit. as the current fragment is before the light occluder
	if (distance <= depth)
		return 1.0 ;
	
		
	float variance = moments.y - (moments.x*moments.x);
	variance = max(variance,0.0000005);
	
	float d = distance - moments.x;
	float p_max = variance / (variance + d*d);
	

	return reduceLightBleeding(p_max ,0.8);
	

}


float shadow_nearest(vec3 light_space_xyz)
{
	// sample shadow map
	float shadow_map_z = texture2D(shadowmap_texture, light_space_xyz.xy).r;

	// + shaded -> 0.0 
	// - lit -> 1.0


	return (light_space_xyz.z - uniform_constant_bias < shadow_map_z) ? 1.0 : 0.15;

}
/* */

// 1 sample per pixel
float shadow(vec3 pwcs)
{
	// project the pwcs to the light source point of view
	vec4 plcs = uniform_light_projection_matrix * uniform_light_view_matrix * vec4(pwcs, 1.0);
	// perspective division
	
	plcs /= plcs.w;

	// convert from [-1 1] to [0 1]
	plcs.xy = (plcs.xy + 1.0f) * 0.5; //0.5

	// check that we are inside light clipping frustum
	if (plcs.x < 0.0) return 0.0;
	if (plcs.y < 0.0) return 0.0;
	if (plcs.x > 1.0) return 0.0;
	if (plcs.y > 1.0) return 0.0;

	// set scale of light space z vaule to [0, 1]
	plcs.z = 0.5 * plcs.z + 0.5; //0.5 0.5

	plcs.xyz += normalize(f_normal)*0.000006;

	vec3 vertex_to_light_direction = normalize(uniform_light_position - f_position_wcs.xyz);
	float dist = distance(uniform_light_position, f_position_wcs.xyz) + 0.000001;

	// sample shadow map
	if(uniform_shading_mode == 0)
		return shadow_nearest(plcs.xyz);
	else if(uniform_shading_mode == 1)
		return shadow_PCF(plcs.xyz , dist);
	else if(uniform_shading_mode == 2 || uniform_shading_mode == 3 || uniform_shading_mode == 4 || uniform_shading_mode == 6)
		return shadow_PCSS( plcs.xyz , uniform_shading_mode , dist);
	
	
}

float compute_spotlight(vec3 vertex_to_light_direction)
{
	float light_cosine_umbra = cos(radians(0.5 * uniform_light_umbra));
	float light_cosine_penumbra = cos(radians(0.5 * uniform_light_penumbra));
	float spoteffect = 1;
	float angle_vertex_spot_dir = dot(normalize(-vertex_to_light_direction), uniform_light_direction);

	if (angle_vertex_spot_dir > light_cosine_umbra)
	{
		spoteffect = 1;
	}
	else if (angle_vertex_spot_dir > light_cosine_penumbra)
	{
		spoteffect = smoothstep(light_cosine_penumbra, light_cosine_umbra, angle_vertex_spot_dir);
	}
	else
		spoteffect = 0;

	return spoteffect;
}

void main(void)
{
    if(uniform_dizzy_state == 1)
		out_color = vec4(1,1,0,1);

	else if(uniform_damage_texture == 1 )
	{
		vec4 color = texture2D( diffuse_texture , f_texcoord);
		color.g += 1.0 - uniform_damage_value/25.0f;
		color.g = clamp(color.g , 0.0f , 1.0f);
		out_color = vec4(color.rgb ,  color.a * uniform_life_time * uniform_life_time * uniform_life_time);	

	}
	else if(uniform_coin_collection == 1)
	{
		 vec4 diffuseColor = vec4(uniform_diffuse.rgb, 1);
	     diffuseColor = mix(diffuseColor, diffuseColor * texture(diffuse_texture, f_texcoord), uniform_has_texture);
	     out_color = diffuseColor;
	}
	//else if(uniform_cannon_ball == 1) // or meteorite
	//{
		// use 2x2 pcf only
	//}
	else
	{

	        vec3 normal = normalize(f_normal);
	        vec4 diffuseColor = vec4(uniform_diffuse.rgb, 1);
	        // if we provide a texture, multiply color with the color of the texture
	        diffuseColor = mix(diffuseColor, diffuseColor * texture(diffuse_texture, f_texcoord), uniform_has_texture);
        
	        if (uniform_focused == 1)
	        	diffuseColor += vec4(0.0, 0.3, 0.0, 0.0);
        
        
	        // compute the direction to the light source
	        vec3 vertex_to_light_direction = normalize(uniform_light_position - f_position_wcs.xyz);
	        // compute the distance to the light source
	        float dist = distance(uniform_light_position, f_position_wcs.xyz) + 0.000001; // add small offset to avoid division with zero
        
	        // direction to the camera
	        vec3 viewDirection = normalize(uniform_camera_position - f_position_wcs.xyz);
	        vec3 halfVector = normalize(viewDirection + vertex_to_light_direction);
        
	        float NdotL = max(dot(vertex_to_light_direction, normal), 0.0);
	        float NdotH = max(dot(halfVector, normal), 0.0);
        
	        // check if we have shadows
	        float shadow_value = 1.0f;
        
	        if (uniform_cast_shadows == 1)
	        	shadow_value = shadow(f_position_wcs);
        
        
	        float nD_dist = NdotL / dist;
	        float a = 1.0f;
	        // compute the spot effect
	        float spotEffect = compute_spotlight(vertex_to_light_direction);
        
	        if (uniform_focused == 1)
	        {
	        	spotEffect += 0.45f;
	        	if (spotEffect > 1.0f)spotEffect = 1.0f;
        
	        	shadow_value += 0.5f;
	        	
	        	if (shadow_value > 1.0f)shadow_value = 1.0f;
	        	nD_dist += 0.0075;
        
	        }
	        // compute the incident radiance
	        vec3 light_color = uniform_light_color;
        
        
	        if (shadow_value <= 0.122999)
	        {
	        	//diffuseColor += vec4(0.2, 0.1, 0, 1)*4.f;
	        }
        
        
	        float x = f_position_wcs.x;
	        float y = f_position_wcs.y;
	        float z = f_position_wcs.z;
	        if (x <= -50 || x > 50 || y > 40 || y < -30 || z > 60 || z < -40)//|| f_position_wcs.x > 50)
	        {
	        	/*
	        	a = mix(1, 0.0, abs(x) / 90.0f);
	        	if (a < 0.1)
	        		discard;
	        	*/
	        }
	        else
	        {
        
	        	a = 1.0f;
	        	if(shadow_value <= 0.9)
	        		diffuseColor += vec4(0.001, 0.005, 0.0025, 0);
	        	else
	        		diffuseColor += vec4(0.0 , 0.055 , 0.05 , 0);
	        }
        
        
	        vec3 irradiance = shadow_value * (spotEffect)* light_color * nD_dist;
        
	        if (uniform_testing_mode == 1)
	        	diffuseColor.rgb = vec3(0.5);
        
	        vec3 diffuseReflection = irradiance * diffuseColor.rgb / PI;
	        float specularNormalization = (uniform_shininess + 8) / (8 * PI);
	        vec3 specularReflection = (NdotL > 0.0) ? irradiance * specularNormalization * uniform_specular * pow(NdotH, uniform_shininess + 0.001) : vec3(0.0);

	        out_color = vec4( diffuseReflection + specularReflection , 1.0f);
	
	}



}

