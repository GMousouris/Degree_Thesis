#include <optix_world.h>
#include <common.h>
#include "helpers.h"
#include "per_ray_data.h"

using namespace optix;




rtDeclareVariable(float3, bad_color, , );
rtBuffer<float4, 2>                sys_OutputBuffer;
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );


RT_PROGRAM void exception()
{
	sys_OutputBuffer[launch_index] = make_float4(bad_color , 1.0f);
}