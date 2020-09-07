#include <optix_world.h>
#include <common.h>
#include "helpers.h"
#include "per_ray_data.h"

using namespace optix;


rtDeclareVariable(float3,        sys_CameraPosition, , );
rtDeclareVariable(float3,        sys_CameraU, , );
rtDeclareVariable(float3,        sys_CameraV, , );
rtDeclareVariable(float3,        sys_CameraW, , );
rtDeclareVariable(float3,        bad_color, , );
rtDeclareVariable(float,         sys_SceneEpsilon, , );

rtDeclareVariable(float2,        sys_mousePos, , );


rtBuffer<float4, 2>              sys_OutputBuffer;
rtDeclareVariable(rtObject,      top_object, , );

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim,   rtLaunchDim, );
rtDeclareVariable(float, time_view_scale, , ) = 1e-6f;
rtDeclareVariable(int, geometry_id, , );
rtDeclareVariable(int, sys_selectedObject, , );
rtDeclareVariable(int, sys_highlightPass, , );
rtDeclareVariable(int, sysIterationIndex, , );


//#define TIME_VIEW

RT_PROGRAM void pinhole_camera()
{



#ifdef TIME_VIEW
  clock_t t0 = clock(); 
#endif

  float scene_epsilon = sys_SceneEpsilon;
  float3 ray_origin;
  float3 ray_direction;

#define METHOD1
#ifdef METHOD1

  const float2 pixel = make_float2(launch_index);
  const float2 fragment = pixel + make_float2(0.5f);
  const float2 screen = make_float2(launch_dim);
  const float2 ndc = (fragment / screen) * 2.0f - 1.0f;

  ray_origin = sys_CameraPosition;
  ray_direction = optix::normalize(ndc.x * sys_CameraU + ndc.y * sys_CameraV + sys_CameraW);

#endif

//#define METHOD2
#ifdef METHOD2

  float3 eye = sys_CameraPosition;
  float3 U   = sys_CameraU;
  float3 V   = sys_CameraV;
  float3 W   = sys_CameraW;
  float2 d = make_float2(launch_index) / make_float2(launch_dim) * 2.f - 1.f;

  ray_origin = sys_CameraPosition;
  ray_direction = normalize(d.x*sys_CameraU + d.y*sys_CameraV + sys_CameraW);

#endif



 
  optix::Ray ray = optix::make_Ray(ray_origin, ray_direction, RADIANCE_RAY_TYPE, scene_epsilon, RT_DEFAULT_MAX);

  PerRayData_radiance prd;
  prd.importance = 1.f;
  prd.depth = 0;
  prd.mouse_ray = 0;
  prd.selected = 0;
  prd.miss = 0;


  rtTrace(top_object, ray, prd);


#ifdef TIME_VIEW
  clock_t t1 = clock(); 
 
  float expected_fps   = 1.0f;
  float pixel_time     = ( t1 - t0 ) * time_view_scale * expected_fps;
  output_buffer[launch_index] = make_color( make_float3(  pixel_time ) ); 
#else

  float a;

  if (prd.selected == 1)
	  a = 1.0f;
  else if (prd.selected == 0)
	  a = 0.0f;
  else
	  a = 0.0f;
  
  if (sys_highlightPass == 0)
	  sys_OutputBuffer[launch_index] = make_float4(prd.radiance, prd.selected);
 
#endif
}