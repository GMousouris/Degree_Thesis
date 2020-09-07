#pragma once

#include <optixu/optixu_math_namespace.h>
#include "common.h"
#include "helpers.h"
#include "per_ray_data.h"
#include "light_structs.h"

#include "sampling_functions.h"
#include "random_number_generators.h"
#include "random.h"

using namespace optix;

rtDeclareVariable(float3, sys_CameraPosition, , );
rtDeclareVariable(float3, sys_CameraU, , );
rtDeclareVariable(float3, sys_CameraV, , );
rtDeclareVariable(float3, sys_CameraW, , );
rtDeclareVariable(float, sys_SceneEpsilon, , );

rtBuffer<ParallelogramLight> lights;
rtBuffer<float4, 2>              sys_OutputBuffer;
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(rtObject, top_shadower, , );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );

rtDeclareVariable(PerRayData_radiance, current_prd, rtPayload, );
rtDeclareVariable(unsigned int, max_depth, ,);
rtDeclareVariable(unsigned int, frame_number, , );
rtDeclareVariable(unsigned int, sqrt_num_samples, , );
rtDeclareVariable(unsigned int, rr_begin_depth, , );


//-----------------------------------------------------------------------------
//
//  Camera program -- main ray tracing loop
//
//-----------------------------------------------------------------------------
RT_PROGRAM void path_trace()
{
	
	size_t2 screen = sys_OutputBuffer.size();

	float2 inv_screen = 1.0f / make_float2(screen) * 2.f;
	float2 pixel = (make_float2(launch_index)) * inv_screen - 1.f;

	float2 jitter_scale = inv_screen / sqrt_num_samples;
	unsigned int samples_per_pixel = sqrt_num_samples * sqrt_num_samples;
	float3 result = make_float3(0.0f);

	unsigned int seed = tea<16>(screen.x*launch_index.y + launch_index.x, frame_number);
	do
	{
		
		//
		// Sample pixel using jittering
		//
		unsigned int x = samples_per_pixel % sqrt_num_samples;
		unsigned int y = samples_per_pixel / sqrt_num_samples;
		float2 jitter = make_float2(x - rnd(seed), y - rnd(seed));
		float2 d = pixel + jitter * jitter_scale;
		float3 ray_origin = sys_CameraPosition;
		float3 ray_direction = normalize(d.x*sys_CameraU + d.y*sys_CameraV + sys_CameraW);

		// Initialze per-ray data
		PerRayData_radiance prd;
		prd.result = make_float3(0.f);
		prd.attenuation = make_float3(1.f);
		prd.countEmitted = true;
		prd.done = false;
		prd.seed = seed;
		prd.depth = 0;
		prd.mouse_ray = 0;

		// Each iteration is a segment of the ray path.  The closest hit will
		// return new segments to be traced here.
		for (;;)
		{

			Ray ray = make_Ray(ray_origin, ray_direction, RADIANCE_RAY_TYPE, sys_SceneEpsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, ray, prd);

			if (prd.done)
			{
				// We have hit the background or a luminaire
				prd.result += prd.radiance * prd.attenuation;
				break;
			}

			// Russian roulette termination 
			if (prd.depth >= rr_begin_depth)
			{
				float pcont = fmaxf(prd.attenuation);
				if (rnd(prd.seed) >= pcont || prd.depth > max_depth)
					break;
				prd.attenuation /= pcont;
			}

			prd.depth++;
			prd.result += prd.radiance * prd.attenuation;

			//if(prd.depth > max_depth)
			//   break;

			// Update ray data for the next path segment
			ray_origin = prd.origin;
			ray_direction = prd.direction;
		}

		result += prd.result;
		seed = prd.seed;
	} while (--samples_per_pixel);

	//
	// Update the output buffer
	//
	float3 pixel_color = result / (sqrt_num_samples*sqrt_num_samples);
	if (frame_number > 1)
	{
		float a = 1.0f / (float)frame_number;
		float3 old_color = make_float3(sys_OutputBuffer[launch_index]);
		sys_OutputBuffer[launch_index] = make_float4(lerp(old_color, pixel_color, a), 1.0f);
	}
	else
	{
		sys_OutputBuffer[launch_index] = make_float4(pixel_color, 1.0f);
	}
}



//-----------------------------------------------------------------------------
//
//  Emissive surface closest-hit
//
//-----------------------------------------------------------------------------
#ifdef EMISSIVE_HIT

rtDeclareVariable(float3, emission_color, , );
rtDeclareVariable(float, t_hit_occluder, rtIntersectionDistance, );


RT_PROGRAM void diffuse_Emitter()
{
	current_prd.radiance = current_prd.countEmitted ? emission_color : make_float3(0.f);
	current_prd.done = true;
}
#endif




//-----------------------------------------------------------------------------
//
//  Lambertian surface diffuse
//
//-----------------------------------------------------------------------------

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(int , cast_shadow_ray, , );

static __device__ void lambertian_surface_diffuse(float3 Kd, float3 ffnormal)
{
	Kd.x = clamp(Kd.x, 0.0f, 0.9f);
	Kd.y = clamp(Kd.y, 0.0f, 0.9f);
	Kd.z = clamp(Kd.z, 0.0f, 0.9f);
	
	float scene_epsilon = sys_SceneEpsilon;
	float3 hitpoint = ray.origin + t_hit * ray.direction;

	
	//
	// Generate a reflection ray.  This will be traced back in ray-gen.
	//
	current_prd.origin = hitpoint;

	//float z1 = rnd(current_prd.seed);
	//float z2 = rnd(current_prd.seed);
	//float3 p;
	//optix::cosine_sample_hemisphere(z1, z2, p);
	//optix::Onb onb(ffnormal);
	//onb.inverse_transform(p);
	//current_prd.direction = p;
	//

	float pdf;
	unitSquareToCosineHemisphere(rng2(current_prd.seed), ffnormal, current_prd.direction, pdf);


	
	// NOTE: f/pdf = 1 since we are perfectly importance sampling lambertian
	// with cosine density.
	if ( cast_shadow_ray == 0 && current_prd.depth > 0)
		current_prd.attenuation = current_prd.attenuation * Kd * (1.2f / current_prd.depth);
	else
	    current_prd.attenuation = current_prd.attenuation * Kd;
	current_prd.countEmitted = false;



	float3 result = make_float3(0.0f);
	if( cast_shadow_ray == 1)
	{
		unsigned int num_lights = lights.size();
		for (int i = 0; i < num_lights; ++i)
	    {
	    	ParallelogramLight light = lights[i];
		
		    const float z1 = 1.0f * rnd(current_prd.seed);
		    const float z2 = 1.0f * rnd(current_prd.seed);
		    const float3 light_pos = light.pos + light.v1 * z1 + light.v2 * z2;
    
    
		    // Calculate properties of light sample (for area based pdf)
		    const float  Ldist = optix::length(light_pos - hitpoint);
		    const float3 L = optix::normalize(light_pos - hitpoint);
		    const float  nDl = optix::dot(ffnormal, L);
		    const float  LnDl = optix::dot( light.normal , L);
    
    
		    // cast shadow ray
		    if (nDl > 0.0f && LnDl > 0.0f)
		    {
    
		    	PerRayData_shadow shadow_prd;
		    	shadow_prd.inShadow = false;
		    	// Note: bias both ends of the shadow ray, in case the light is also present as geometry in the scene.
		    	optix::Ray shadow_ray = optix::make_Ray(hitpoint, L, SHADOW_RAY_TYPE, scene_epsilon, Ldist);
		    	rtTrace(top_shadower, shadow_ray, shadow_prd);
		    	
		    	if (!shadow_prd.inShadow)
		    	{
		    		float size_factor = 1.0f;
		    		const float A = optix::length(optix::cross(light.v1 * size_factor , light.v2) * size_factor );
		    		// convert area based pdf to solid angle
		    		const float weight = nDl * LnDl * A / (M_PIf * Ldist * Ldist);
		    		result += light.color * weight;
		    	}
		    	
    
		    }
		

	    }
	}
	else
	{
	
		float3 env_light_pos = make_float3(100.0f,1000.0f, -100.0f);
		float3 env_light_normal = normalize(hitpoint - env_light_pos);
		const float3 L = normalize(env_light_pos - hitpoint);
		const float  nDl = dot(ffnormal, L);
		const float  LnDl = dot( env_light_normal , L);
		
		if(nDl > 0.0f)
		{
		    const float3 env_light_color = make_float3(1.0f);
			//const float  Ldist = optix::length(env_light_pos - hitpoint);
			const float  Ldist = 500.0f; // constant ?
		    
		    float size_factor = (20.0f);
			// const float A = optix::length(optix::cross(light.v1 * size_factor , light.v2) * size_factor ); // Area Light 
			const float A = 2.0f * M_PIf * size_factor * size_factor; 
		    const float weight = nDl  * A * fabsf(dot( ffnormal, normalize(sys_CameraW)))  / (M_PIf * Ldist);
			result += env_light_color * weight;
		}
		
	}

	
	
	current_prd.radiance = result;
	
}





rtDeclareVariable(int, geometry_id, , );
rtDeclareVariable(int, sys_focusedObject, , );
rtDeclareVariable(int, sys_selectedObject, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtBuffer<int> sys_SelectedObjectBuffer;

rtDeclareVariable(float3, Kd, , );
//-----------------------------------------------------------------------------
//
//  closest-hit
//
//-----------------------------------------------------------------------------
RT_PROGRAM void closest_hit()
{

	if (current_prd.mouse_ray == 1)
	{
		if (geometry_id >= 0)
		{
			sys_SelectedObjectBuffer[0] = geometry_id;
			current_prd.selected = geometry_id;
			return;

		}
		else
		{
			current_prd.selected = -1;
			return;
		}
	}
	else
	{
		if ((sys_focusedObject == geometry_id) || (sys_selectedObject == geometry_id) && geometry_id >= 0)
			current_prd.selected = geometry_id;
		else
			current_prd.selected = -1;
	}


	
	float3 world_shading_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 world_geometric_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));
	float3 ffnormal = faceforward(world_shading_normal, -ray.direction, world_geometric_normal);

	lambertian_surface_diffuse(Kd, ffnormal);

}




rtTextureSampler<float4, 2> Kd_map;
rtDeclareVariable(float3, texcoord, attribute texcoord, );
//-----------------------------------------------------------------------------
//
//  closest-hit Textured
//
//-----------------------------------------------------------------------------
RT_PROGRAM void closest_hit_textured()

{

	if (current_prd.mouse_ray == 1)
	{
		if (geometry_id >= 0)
		{
			sys_SelectedObjectBuffer[0] = geometry_id;
			current_prd.selected = geometry_id;
			return;

		}
		else
		{
			current_prd.selected = -1;
			return;
		}
	}
	else
	{
		if ((sys_focusedObject == geometry_id) || (sys_selectedObject == geometry_id) && geometry_id >= 0)
			current_prd.selected = geometry_id;
		else
			current_prd.selected = -1;
	}


	float3 world_shading_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 world_geometric_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));
	float3 ffnormal = faceforward(world_shading_normal, -ray.direction, world_geometric_normal);

	const float3 Kd_val = make_float3(tex2D(Kd_map, texcoord.x, texcoord.y));
	lambertian_surface_diffuse(Kd_val, ffnormal);
	
}





//-----------------------------------------------------------------------------
//
//  Shadow any-hit
//
//-----------------------------------------------------------------------------
rtDeclareVariable(PerRayData_shadow, current_prd_shadow, rtPayload, );
RT_PROGRAM void any_hit()
{
	current_prd_shadow.inShadow = true;
	rtTerminateRay();
}



//-----------------------------------------------------------------------------
//
//  Miss
//
//-----------------------------------------------------------------------------


rtDeclareVariable(float3, bg_color, , );
rtDeclareVariable(float3, bg_color2, , );
RT_PROGRAM void miss()
{
	

	current_prd.selected = -1;
	current_prd.miss = 1;
	
	current_prd.done = true;
	if (current_prd.depth == 0)
	{
	    float u = (float)launch_index.x / (float)launch_dim.x;
	    float v = (float)launch_index.y / (float)launch_dim.y;
	    float3 final_color = bg_color * u + bg_color2 * v;
	
		current_prd.radiance = final_color;
	}
	else
	{
	    float b = current_prd.depth/5.0f;
		float a = 1.0f - b;
		current_prd.radiance = a * make_float3(0.0f) + make_float3(0.25f)*b;
	}

}




//-----------------------------------------------------------------------------
//
//  Exception 
//
//-----------------------------------------------------------------------------

rtDeclareVariable(float3, bad_color, , );
RT_PROGRAM void exception()
{
	sys_OutputBuffer[launch_index] = make_float4(bad_color , 1.0f);
}