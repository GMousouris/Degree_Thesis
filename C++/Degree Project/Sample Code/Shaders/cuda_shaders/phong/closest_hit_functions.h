
#include <optixu/optixu_math_namespace.h>
#include <optix_world.h>

#include "common.h"
#include "random.h"
#include "helpers.h"
#include "light_structs.h"
#include "per_ray_data.h"

using namespace optix;


	rtDeclareVariable(unsigned int, max_depth, , );
	rtBuffer<ParallelogramLight>  lights;
	rtBuffer<int> sys_SelectedObjectBuffer;


	rtDeclareVariable(float3, ambient_light_color, , );
	rtDeclareVariable(float, sys_SceneEpsilon, , );
	rtDeclareVariable(float3, sys_CameraPosition, , );

	rtDeclareVariable(rtObject, top_object, , );
	rtDeclareVariable(rtObject, top_shadower, , );

	rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
	rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
	rtDeclareVariable(PerRayData_radiance, prd, rtPayload, );
	rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );
	rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
	rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );



static __device__ void phong_closest_hit
(
	float3 p_Kd,
	float3 p_Ka,
	float3 p_Ks,
	float3 p_Kr,
	float  p_phong_exp,
	float3 p_normal
)
	{
		float scene_epsilon = sys_SceneEpsilon;
		float3 hit_point = ray.origin + t_hit * ray.direction;




		float3 radiance = p_Ka * ambient_light_color;

		// compute direct lighting
		unsigned int num_lights = lights.size();
		for (int i = 0; i < num_lights; ++i)
		{
			ParallelogramLight light = lights[i];
			float Ldist = optix::length(light.pos - hit_point);
			float3 L = optix::normalize(light.pos - hit_point);
			float nDl = optix::dot(p_normal, L);

			// cast shadow ray
			float3 light_attenuation = make_float3(static_cast<float>(nDl > 0.0f));
			if (nDl > 0.0f && light.casts_shadow)
			{

				PerRayData_shadow shadow_prd;
				shadow_prd.attenuation = make_float3(1.0f);
				optix::Ray shadow_ray = optix::make_Ray(hit_point, L, SHADOW_RAY_TYPE, scene_epsilon, Ldist);
				rtTrace(top_shadower, shadow_ray, shadow_prd);
				light_attenuation = shadow_prd.attenuation;
			

			}

			// If not completely shadowed, light the hit point
			if (fmaxf(light_attenuation) > 0.0f)
			{
				float3 Lc = light.color * light_attenuation;

				radiance += p_Kd * nDl * Lc;

				float3 H = optix::normalize(L - ray.direction);
				float nDh = optix::dot(p_normal, H);
				if (nDh > 0) {
					float power = pow(nDh, p_phong_exp);
					radiance += p_Ks * power * Lc;
				}
			}
		}

		if (fmaxf(p_Kr) > 0) {

			// ray tree attenuation
			PerRayData_radiance new_prd;
			new_prd.importance = prd.importance * optix::luminance(p_Kr);
			new_prd.depth = prd.depth + 1;
			new_prd.mouse_ray = 0;
			new_prd.miss = 0;

			// reflection ray
			if (new_prd.importance >= 0.01f && new_prd.depth <= max_depth)
			{
				float3 R = optix::reflect(ray.direction, p_normal);
				optix::Ray refl_ray = optix::make_Ray(hit_point, R, RADIANCE_RAY_TYPE, scene_epsilon, RT_DEFAULT_MAX);
				rtTrace(top_object, refl_ray, new_prd);
				radiance += p_Kr * new_prd.radiance;
			}
		}

		// pass the color back up the tree
		radiance.x = optix::clamp(radiance.x, 0.0f, 1.0f);
		radiance.y = optix::clamp(radiance.y, 0.0f, 1.0f);
		radiance.z = optix::clamp(radiance.z, 0.0f, 1.0f);

		prd.radiance = radiance;

	}


