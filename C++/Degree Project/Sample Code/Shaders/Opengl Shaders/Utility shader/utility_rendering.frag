#version 330 core
layout(location = 0) out vec4 out_color;
layout(pixel_center_integer) in vec4 gl_FragCoord;
uniform sampler2D samplerHDR;
uniform sampler2D depthSampler;
uniform int uniform_rendering_scene_origin;

uniform vec3 uniform_diffuse;
uniform float uniform_alpha;
uniform int uniform_is_arc;
uniform vec3 uniform_camera_pos;
uniform vec3 uniform_camera_w;
uniform int uniform_circle;
uniform int uniform_selected_circle;
uniform vec3 uniform_sphere_center;

uniform int outline_1st_pass;
uniform int outline_2nd_pass;
uniform int draw_sphere;
uniform int draw_sphere_wireframe;
uniform int inner_radius_hit;

uniform int uniform_test_depth;


uniform int   uniform_rendering_gradient_grid;
uniform vec3  uniform_gradient_grid_o;
uniform float uniform_gradient_grid_dist;


uniform int uniform_mix_color;
uniform vec3 uniform_mix_p0;
uniform vec3 uniform_mix_p1;
uniform vec3 uniform_mcolor_a;
uniform vec3 uniform_mcolor_b;


// in  // 
in vec3 f_position_default;
in vec3 f_position_wcs;
in vec3 f_parent_center;
in vec3 f_normal;
in vec3 f_pos;
in vec3 f_gradient_grid_origin;

void main(void) 
{	
	

	if(uniform_mix_color == 1)
	{
		vec3 colora = uniform_mcolor_a;
		vec3 colorb = uniform_mcolor_b;
		vec3 pa  = uniform_mix_p0;
		vec3 pb  = uniform_mix_p1;
		vec3 p   = f_pos.xyz;
		float d  = length(pa-pb) + 0.00001f;
		float dp = length(p-pb);
		float t  = dp / d;
		t = t > 1 ? 1 : t;
		t = t < 0 ? 0 : t;
		vec3 color = mix( colorb, colora, t );
		out_color  = vec4( color, uniform_alpha );
		return;
	}

    vec3 color  = uniform_diffuse;
	bool behind = false;

	if( uniform_test_depth == 1 )
	{
	     ivec2 uv     = ivec2(gl_FragCoord.xy);
	     float depth  = texelFetch(depthSampler, uv , 0).r;
		 float z      = gl_FragCoord.z;
		 behind  = z >= depth + 0.0000001f ? true : false;

		 if ( behind && uniform_rendering_gradient_grid == 1 )
		 {
		    return;
		 }
		 else if ( behind && uniform_rendering_gradient_grid != 1 )
		 {
		    out_color = vec4(color.rgb, 0.4f);
		    return;
	     }
	}

	if( uniform_rendering_gradient_grid == 1 )
	{
	    float dist = length(f_pos - uniform_gradient_grid_o);
		float a    = dist / uniform_gradient_grid_dist;
		a = a > 1? 1 : a;
		float alpha_v = (1 - a);
		
		out_color = vec4(uniform_diffuse, alpha_v);
		return;
	}


	if( uniform_rendering_scene_origin == 1 )
	{
	    ivec2 screen     = ivec2(gl_FragCoord.xy);
		float miss_value = texelFetch(samplerHDR, screen , 0).r;
		if(miss_value == 1.0f)
		   out_color = vec4(uniform_diffuse, 1.0f);
		else
		   out_color = vec4(uniform_diffuse, 1.0f);
		return;
	}

    if( draw_sphere == 1 )
    {
        float alpha_value = uniform_alpha;
        vec3 n = normalize(uniform_camera_pos - f_pos);
        
        vec3 a = vec3(0.25f);
        vec3 b = vec3(0.25f);
		//alpha_value = 0.0f;

        if(inner_radius_hit == 1)
        {
            a = vec3(0.3f); // 0.55
            b = vec3(0.3f); // 0.55
            alpha_value = 0.7f; // 0.4
        }

        float d = dot(n, normalize(f_position_default));
        d = clamp(d,0,1);
        out_color = vec4( d*a + (1-d)*b, alpha_value);
        return;
    }
    else if(draw_sphere_wireframe == 1)
    {
        out_color = vec4( uniform_diffuse, uniform_alpha);
        return;
    }

    if(uniform_is_arc == 1)
    {
    	
    	if(f_position_wcs.z > f_parent_center.z + 0.1f && uniform_circle != uniform_selected_circle)
    		discard;
    	else if( f_position_wcs.z > f_parent_center.z + 0.1f && uniform_circle == uniform_selected_circle)
    		color = vec3(0.34f);
    }

    float alpha_value = uniform_alpha;
    if(outline_1st_pass == 1 )
	{
		alpha_value = 0.0f;
	}
	if(outline_2nd_pass == 1 )
	{
		//alpha_value = 0.0f;
	}


	
	
	out_color = vec4( color , alpha_value );	
}

