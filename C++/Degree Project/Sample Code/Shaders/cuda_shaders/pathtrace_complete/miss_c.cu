
#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "RT_app_config.h"
#include "per_ray_data.h"
#include "shader_common.h"


rtDeclareVariable(float3, bg_color, , );
rtDeclareVariable(float3, bg_color2, , );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );

rtDeclareVariable(PerRayData_c, thePrd, rtPayload, );
rtDeclareVariable(int, cast_shadow_ray, , );

rtBuffer<float3>       sys_mouseHit_buffer_Output;


RT_PROGRAM void miss()
{
    if(thePrd.mouse_ray == 1)
	{
	    sys_mouseHit_buffer_Output[2] = make_float3(0.0f);
	}
	
	thePrd.selected = -1;
	
	if (thePrd.depth == 0)
	{
	    thePrd.miss = 1;
	    thePrd.depth_value = 1;
		//float u = (float)launch_index.x / (float)launch_dim.x;
	    float v = (float)launch_index.y / (float)launch_dim.y;

		
		float3 final_color = bg_color * v + bg_color2 * (1.0f - v);
		thePrd.radiance    = final_color;
	}
	else
	{
	    if( cast_shadow_ray == 1 )
		    thePrd.radiance = make_float3(0.0f);
		else
		   thePrd.radiance = make_float3(1.0f);
		    
	}

    thePrd.flags |= FLAG_TERMINATE;



}
