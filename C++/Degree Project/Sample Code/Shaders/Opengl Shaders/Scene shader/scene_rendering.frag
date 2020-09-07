#version 330 core
layout(location = 0) out vec4 out_color;


uniform sampler2D      diffuse_texture;
uniform sampler2DArray uniform_Material_textureArray;

uniform vec3      uniform_diffuse;
uniform float     uniform_alpha;

uniform vec3      uniform_specular;
uniform float     uniform_shininess;
uniform float     uniform_has_texture;
uniform vec3      uniform_camera_position;
uniform vec3      uniform_camera_w;

uniform int       uniform_shading_mode;
uniform int       uniform_rendering_utility;

uniform int       uniform_is_app_grid;
uniform int       uniform_is_instance;
uniform int       uniform_is_camera_orbit;

uniform int uniform_rendering_distance_sphere;
uniform vec3  uniform_ds_a;
uniform vec3  uniform_ds_b;
uniform float uniform_ds_r;
uniform int   uniform_ds_inv;


in VS_OUT
{
   vec2 f_texcoord;
   vec3 f_position_wcs;
   vec3 f_normal;
   flat uint draw_id;
   flat uint material_id;
   flat uint has_tex_id;

} fs_in;


float uniform_constant_bias = 0.0001;
#define PI 3.14159
#define M_1_PIf 0.31830988618
#define _8_M_1_PIf 0.03978873577

float modulo(float x)
{
	return x - floor(x);
}

float get_checkers_pattern()
{
    vec2 tex = fs_in.f_texcoord;
	

    float angle = 0.0f;
	float s = tex.x * cos(angle) - tex.y * sin(angle);
	float t = tex.y * cos(angle) + tex.x * sin(angle);

	float scaleS = 24.0f; // 24
	float scaleT = 24.0f; // 24

	
	float pattern = 1.0f;
	bool m1 = modulo(t*scaleS) < 0.94f; // 0.945
	bool m2 = modulo(s*scaleT) < 0.94f; // 0.945

	if( m1 && m2 )
	    pattern = 0.0f;
	else
	    pattern = 1.0f;

	pattern = clamp(pattern, 0.40f, 0.9f);
	return pattern;
}

float compute_spotlight(vec3 vertex_to_light_direction , vec3 light_direction)
{
	float light_cosine_umbra    = cos(radians(0.5 * 500));
	float light_cosine_penumbra = cos(radians(0.5 * 450));
	float spoteffect = 1;
	float angle_vertex_spot_dir = dot(normalize(-vertex_to_light_direction), light_direction);

	if (angle_vertex_spot_dir > light_cosine_umbra) 
	{
		spoteffect = 1;
	}
	else if(angle_vertex_spot_dir > light_cosine_penumbra) 
	{
		spoteffect = smoothstep(light_cosine_penumbra, light_cosine_umbra, angle_vertex_spot_dir);
	}
	else 
		spoteffect = 0;
	
	return spoteffect;
}



void main(void) 
{	

    vec4 diffuseColor = vec4( uniform_diffuse.rgb, 1.0f );	
    if( uniform_rendering_utility == 1 )
	{
	    if(uniform_rendering_distance_sphere == 1)
	    {
	    	vec3 a     = uniform_ds_a; // circle origin
	    	vec3 b     = uniform_ds_b; // to
	    	float r    = uniform_ds_r + 0.00001f;
	    	vec3 p     = fs_in.f_position_wcs.xyz;
	    	int is_inv = uniform_ds_inv;

	    	float dist = length(b-a);

	    	vec3 u = normalize(b - a);
	    	vec3 v = normalize(p - a);
	    	vec3 p0 = a - (u*r);

            float d     = is_inv == 1? length(p - p0) / (4*r) : dot(v,u);
	    	float alpha = is_inv == 1? d * d * 2.0f  : d * d * uniform_alpha;


	    	if(d < 0.05f) //0.05
	    		discard;

	    	
	    	//vec3 ca = vec3(0.3,1,0.3);
	    	//vec3 cb = vec3(1,0.25,0);
	    	//vec3 color = is_inv == 1? uniform_diffuse : mix(ca, cb, dist/r);
	    	out_color = vec4(uniform_diffuse, alpha);
	    	return;
	    }

		out_color = vec4( uniform_diffuse.rgb , uniform_alpha);
		return;
	}
	float pattern = 1.0f;
	if ( uniform_is_instance == 1 )
	{
	     int mat_id = int(fs_in.material_id);
	     //diffuseColor  = mix( diffuseColor, diffuseColor * texture( diffuse_texture, fs_in.f_texcoord ), uniform_has_texture );
	     diffuseColor     = texture( uniform_Material_textureArray, vec3(0, 0, mat_id) );
	}
	else
	{
	       
	}
	

    float dist     = uniform_is_camera_orbit == 1 ? 7.50f : 1.0f;
	vec3 normal    = normalize( fs_in.f_normal );
	vec3 light_pos = uniform_camera_position;
	vec3 light_dir = normalize( uniform_camera_w );
	
	vec3 vertex_to_light_direction = normalize(light_pos   - fs_in.f_position_wcs.xyz);
	vec3 viewDirection = normalize(uniform_camera_position - fs_in.f_position_wcs.xyz);	
	vec3 halfVector    = normalize(viewDirection + vertex_to_light_direction);
	float NdotL        = max( dot(vertex_to_light_direction, normal), 0.0 );
	
	vec3   fade   = vec3(1.0f);
    float  fade_s = 0.65f;
    if( uniform_is_app_grid != 1 && NdotL < fade_s  )
	{
		float t = (fade_s - NdotL) / fade_s;
	    t       = clamp(t, 0.0f, 1.0f);

	    fade.x = (0.0f) + (1.0f - t);
		fade.y = (0.0f) + (1.0f - t);
		fade.z = (0.0f) + (1.0f - t);
	}
	

	
	float NdotH      = max( dot(halfVector, normal), 0.0 );
	float spotEffect = 1.0f; 
	spotEffect       = dot( uniform_camera_w, normal );
	spotEffect      *= spotEffect;
	spotEffect       = uniform_is_camera_orbit == 1 ? clamp( spotEffect, 1.f, 100.0f ) : clamp( spotEffect, 0.07f, 100.0f );

	vec3  irradiance        = spotEffect * NdotL * vec3(1.0f) * 0.5f / dist;
	vec3  diffuseReflection = irradiance * diffuseColor.rgb * M_1_PIf;


	float shininess             = 0.01f; // 0.001f;
	float specularNormalization = (shininess + 8) * _8_M_1_PIf; // (8 * PI);
	vec3  specular              = uniform_is_app_grid == 1 ? vec3(0.0f) : diffuseColor.rgb;
	vec3  specularReflection    = (NdotL > 0.0)? irradiance * specularNormalization * specular * pow( NdotH, shininess + 0.001) : vec3(0.0f);

	if ( uniform_is_camera_orbit == 1 )
	{
	     fade *= 0.7f;
	     //fade = vec3(0.5f);
	     //specularReflection = specularReflection * 0.5f;
	}
	


	vec3  final_color = 100.0f * pattern * fade * diffuseReflection + specularReflection;
	if( uniform_shading_mode == 1 )
	{ 
	     final_color.r *= 0.3f;
		 final_color.g *= 0.59f;
		 final_color.b *= 0.11f;
	     float g        = final_color.r + final_color.g + final_color.b;
	     final_color    = vec3(g);
	}
	out_color = vec4( final_color , 1.0f );	
}

