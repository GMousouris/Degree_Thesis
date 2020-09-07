#pragma once

#include <optixu/optixu_math_namespace.h>

#include "random_number_generators.h"
#include "per_ray_data.h"
#include "shader_common.h"
#include "RT_app_config.h"

using namespace optix;

rtDeclareVariable(int, sys_write_depth, , );
rtDeclareVariable(int, sys_only_write_depth, , );

rtDeclareVariable(float3, sys_CameraPosition, , );
rtDeclareVariable(float3, sys_CameraU, , );
rtDeclareVariable(float3, sys_CameraV, , );
rtDeclareVariable(float3, sys_CameraW, , );
rtDeclareVariable(float, sys_SceneEpsilon, , );

rtBuffer<float, 2>                 sys_DepthBuffer;
rtBuffer<float4, 2>                sys_OutputBuffer;
//rtBuffer<float, 2>               sys_MissOutputBuffer;              
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(rtObject, top_shadower, , );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );

rtDeclareVariable(unsigned int, frame_number, , );
rtDeclareVariable(unsigned int, sqrt_num_samples, , );
rtDeclareVariable(unsigned int, rr_begin_depth, , );

rtDeclareVariable(unsigned int, min_depth, , );
rtDeclareVariable(unsigned int, max_depth, , );

rtDeclareVariable(unsigned int , sys_abort, , );


//-----------------------------------------------------------------------------
//
//  Camera program -- main ray tracing loop
//
//-----------------------------------------------------------------------------


RT_FUNCTION void integrator(PerRayData_c& prd, float3& radiance)
{

  radiance          = make_float3(0.0f); // Start with black.
  float3 throughput = make_float3(1.0f); // The throughput for the next radiance, starts with 1.0f.
  prd.flags         = 0;
  prd.flags        &= FLAG_CLEAR_MASK;   // Clear all non-persistent flags. In this demo only the last diffuse surface interaction stays.
  int depth         = 0;

  while ( prd.depth < max_depth)
  {

    prd.wo        = -prd.wi;           // Direction to observer.
    optix::Ray ray = optix::make_Ray(prd.pos, prd.wi, 0, sys_SceneEpsilon, RT_DEFAULT_MAX);
    rtTrace(top_object, ray, prd);

    radiance += throughput * prd.radiance;

    // Path termination by miss shader or sample() routines.
    // If terminate is true, f_over_pdf and pdf might be undefined.
    if ((prd.flags & FLAG_TERMINATE) || prd.pdf <= 0.0f || isNull(prd.f_over_pdf))
    {
      break;
    }

    // PERF f_over_pdf already contains the proper throughput adjustment for diffuse materials: f * (fabsf(optix::dot(prd.wi, state.normal)) / prd.pdf);
    throughput *= prd.f_over_pdf;

    // Unbiased Russian Roulette path termination.
    if ( min_depth <= prd.depth) // Start termination after a minimum number of bounces.
    {
      const float probability = fmaxf(throughput); // DAR Other options: // intensity(throughput); // fminf(0.5f, intensity(throughput));
      if (probability < rng(prd.seed)) // Paths with lower probability to continue are terminated earlier.
      {
        break;
      }
      throughput /= probability; // Path isn't terminated. Adjust the throughput so that the average is right again.
    }

    ++prd.depth; // Next path segment.
	//++depth;
  }
}







RT_PROGRAM void path_trace()
{

  PerRayData_c prd;
  prd.depth = 0;
  prd.mouse_ray = 0;
  prd.miss = 0;
  
  // Initialize the random number generator seed from the linear pixel index and the iteration index.
  prd.seed = tea2<8>(launch_index.y * launch_dim.x + launch_index.x, frame_number);
  
  //float2 jitter = make_float2(0.0f);
  //if (max_depth > 1)

  float2 jitter   = rng2(prd.seed);
  //float2 jitter = make_float2(0.0f);

  const float2 pixel = make_float2(launch_index);
  const float2 fragment = pixel + jitter; // Random jitter of the fragment location in this pixel.
  const float2 screen = make_float2(launch_dim);
  const float2 ndc = (fragment / screen) * 2.0f - 1.0f;

  // The integrator expects the next path segments ray.origin in prd.pos and the next ray.direction in prd.wi.
  prd.pos = sys_CameraPosition;
  prd.wi  = optix::normalize(ndc.x * sys_CameraU + ndc.y * sys_CameraV + sys_CameraW);

  float3 radiance;
  integrator(prd, radiance); // In this case a unidirectional path tracer.

#if USE_DEBUG_EXCEPTIONS
  // DAR DEBUG Highlight numerical errors.
  if (isnan(radiance.x) || isnan(radiance.y) || isnan(radiance.z))
  {
    radiance = make_float3(1000000.0f, 0.0f, 0.0f); // super red
  }
  else if (isinf(radiance.x) || isinf(radiance.y) || isinf(radiance.z))
  {
    radiance = make_float3(0.0f, 1000000.0f, 0.0f); // super green
  }
  else if (radiance.x < 0.0f || radiance.y < 0.0f || radiance.z < 0.0f)
  {
    radiance = make_float3(0.0f, 0.0f, 1000000.0f); // super blue
  }
#else
  // NaN values will never go away. Filter them out before they can arrive in the output buffer.
  // This only has an effect if the debug coloring above is off!
  if (!(isnan(radiance.x) || isnan(radiance.y) || isnan(radiance.z)))
#endif
  {
    

	if ( sys_write_depth == 1 )
	{
	    sys_DepthBuffer[launch_index] = prd.depth_value;
	}

	if ( sys_only_write_depth == 1 )
	{
	    return;
  }
   
  float4 dst    = sys_OutputBuffer[launch_index]; 
  float4 output = ( frame_number > 1 )? optix::lerp(dst, make_float4(radiance, 1.0f), 1.0f / (float) (frame_number + 1)) 
                                        : make_float4(radiance, 1.0f);
  
  //output.x = output.x > 1 ? 1: output.x;
  //output.y = output.y > 1 ? 1: output.y;
  //output.z = output.z > 1 ? 1: output.z;
  sys_OutputBuffer[launch_index] = output;
  
	
  }
}




