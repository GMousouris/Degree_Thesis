#include <optix_world.h>
#include "per_ray_data.h"

using namespace optix;




rtDeclareVariable(PerRayData_radiance, current_prd, rtPayload, ,);
rtDeclareVariable(float3, bg_color, , );
rtDeclareVariable(float3, bg_color2, , );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );



RT_PROGRAM void miss()
{
	
	
	/* experimenting bg color */
	float u = (float)launch_index.x / (float)launch_dim.x;
	float v = (float)launch_index.y / (float)launch_dim.y;
	float3 final_color = bg_color * u + bg_color2 * v;
	
	//current_prd.result = final_color;
	current_prd.selected = -1;
	current_prd.miss = 1;
	
	current_prd.done = true;
	if (current_prd.depth == 0)
	{
		current_prd.radiance = final_color;
	}
	else
	{
	    float b = current_prd.depth/5.0f;
		float a = 1.0f - b;
		current_prd.radiance = a * make_float3(0.0f) + make_float3(0.25f)*b;
	}

	

}
