
#include <optix_world.h>
#include <common.h>
#include "helpers.h"
#include "per_ray_data.h"

using namespace optix;



rtDeclareVariable(float3, sys_m_CameraPosition, , );
rtDeclareVariable(float3, sys_m_CameraU, , );
rtDeclareVariable(float3, sys_m_CameraV, , );
rtDeclareVariable(float3, sys_m_CameraW, , );
rtDeclareVariable(float3, bad_color, , );
rtDeclareVariable(float,  sys_SceneEpsilon, , );

rtDeclareVariable(float2, sys_mousePos, , );
rtDeclareVariable(float3, sys_cast_dir, , );
rtDeclareVariable(int,    sys_is_custom_dir , ,);

rtDeclareVariable(int,    sys_utility_ray, ,);
rtDeclareVariable(float3, sys_utility_ray_origin, ,);
rtDeclareVariable(float3, sys_utility_ray_direction, ,);


rtBuffer<float4, 2>                sys_OutputBuffer;
//rtBuffer<uchar4, 2>              sys_OutputBuffer;
rtDeclareVariable(rtObject, top_object, , );

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );
rtDeclareVariable(uint,  sys_m_width, , );
rtDeclareVariable(uint,  sys_m_height, , );

rtDeclareVariable(float, time_view_scale, , ) = 1e-6f;
rtDeclareVariable(int, geometry_id, , );

rtDeclareVariable(unsigned int, shader_index, ,);

RT_PROGRAM void mouse_ray_generation()
{
    
      optix::uint2 dim = optix::make_uint2(sys_m_width, sys_m_height);
	  const float2 pixel = sys_mousePos;
	  const float2 fragment = pixel + make_float2(0.5f);
	  const float2 screen = make_float2(dim);
	  const float2 ndc = (fragment / screen) * 2.0f - 1.0f;

	  float3 ray = make_float3(ndc.x, ndc.y, 1.0f);
	  float3 U = sys_m_CameraU;
	  float3 V = sys_m_CameraV;
	  float3 W = sys_m_CameraW;

	   
	  float3 ray_origin;
	  float3 ray_dir;

	  if( sys_utility_ray == 0 )
	  {
	     ray_origin = sys_m_CameraPosition;
	     ray_dir = normalize(ndc.x * U - ndc.y * V + W);
	  }
	  else
	  {
	     ray_origin = sys_utility_ray_origin;
		 ray_dir    = sys_utility_ray_direction;
		 
	  }

	  optix::Ray mouse_ray = optix::make_Ray(ray_origin, ray_dir, RADIANCE_RAY_TYPE, sys_SceneEpsilon, RT_DEFAULT_MAX);
	  if( shader_index == 0 || shader_index == 1 )
      {
	     PerRayData_radiance prd;
	     prd.depth = 0;
	     prd.mouse_ray = 1;
	     prd.selected = 0;
	     rtTrace(top_object, mouse_ray, prd);
	  }
	  else if ( shader_index == 2 )
      {
	     PerRayData_c prd;
	     prd.depth     = 0;
	     prd.mouse_ray = 1;
	     prd.selected  = 0;
	     rtTrace(top_object, mouse_ray, prd);
	  }
	


}

