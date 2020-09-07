
#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "light_definition.h"


#define MULT_INV_PROB_HIT_LIGHT


RT_FUNCTION float compute_spotlight(float3 light_dir, float3 normal, float umbra, float penumbra, bool is_soft)
{
	float rad = 0.01745329252;
	float light_cosine_umbra    = fabsf(cosf((1.f * umbra    )*rad));
	float light_cosine_penumbra = fabsf(cosf((1.f * penumbra )*rad));
	float spotE  = 1.0f;
	float dotP_L = fabsf(dot( -normal, normalize(light_dir) ));
	if (dotP_L > light_cosine_umbra)
		return 1.0f;
	else if (dotP_L > light_cosine_penumbra)
	{
		//return
		float v;
		v = is_soft? smoothstep(light_cosine_penumbra, light_cosine_umbra, dotP_L):
			         lerp(light_cosine_penumbra, light_cosine_umbra, dotP_L);
		return v;
		
	}
	else
		return 0.0f;//return smoothstep(light_cosine_penumbra, 0.0f, dotP_L);
}
RT_FUNCTION int is_in_umbra(float3 light_dir, float3 normal)
{
	float umbra = 40.0f;
	float light_cosine_umbra = cosf((0.5f * umbra) * 0.01745329252);
	float dotL = fabsf(dot(normalize(-light_dir), normal));
	if (dotL > light_cosine_umbra)
		return 1;
	else
		return 0;
}


RT_FUNCTION void sample_light_camera_constant(float3 const& point, float3 const& cam_w, const float2 sample, LightSample& lightSample, int sys_NumLights)
{
	//unitSquareToSphere(sample.x, sample.y, lightSample.direction, lightSample.pdf);

	// Environment lights do not set the light sample position!
	lightSample.distance = 2000.0f; // Environment light.

	// Explicit light sample. White scaled by inverse probabilty to hit this light.
	lightSample.position = point -cam_w * lightSample.distance;
	lightSample.direction = normalize(lightSample.position - point);
	//const float cosTheta = optix::dot(-lightSample.direction, make_float3(0,-1,0));
	lightSample.emission = make_float3(3500.0f); //10500
	lightSample.pdf = (lightSample.distance * lightSample.distance) / (200.0f * 2.0f * M_PIf);
}
RT_FUNCTION void sample_light_constant(float3 const& point, const float2 sample, LightSample& lightSample, int sys_NumLights)
{
  //unitSquareToSphere(sample.x, sample.y, lightSample.direction, lightSample.pdf);

  // Environment lights do not set the light sample position!
  lightSample.position = make_float3(-5, 1000.0f, 5);
  lightSample.distance = 2000.0f; // Environment light.

  // Explicit light sample. White scaled by inverse probabilty to hit this light.
  lightSample.direction = normalize(lightSample.position - point);
  //const float cosTheta = optix::dot(-lightSample.direction, make_float3(0,-1,0));
  lightSample.emission = make_float3( 1500.0f );
  lightSample.pdf = (lightSample.distance * lightSample.distance) / ( 200.0f * 2.0f * M_PIf );
}


// SAMPLE_LIGHT : PARALLELOGRAM //
RT_FUNCTION void sample_light_parallelogram(float3 const& point, const float3 sample, LightSample& lightSample, LightDefinition& light, int sys_NumLights)                // Num of Lights
{

	lightSample.pdf = 0.0f; // Default return, invalid light sample (backface, edge on, or too near to the surface)

	
	lightSample.position  = light.position + light.vecU * sample.x + light.vecV * sample.y; // The light sample position in world coordinates.
	lightSample.direction = (lightSample.position - point); // Sample direction from surface point to light sample position.
	lightSample.distance  = optix::length(lightSample.direction);
	if (DENOMINATOR_EPSILON < lightSample.distance)
	{
		lightSample.direction /= lightSample.distance; // Normalized direction to light.

		const float cosTheta = optix::dot(-lightSample.direction, (light.normal));
		if (DENOMINATOR_EPSILON < cosTheta) // Only emit light on the front side.
		{
			// Explicit light sample, must scale the emission by inverse probabilty to hit this light.
			lightSample.emission = light.emission 
#ifdef MULT_INV_PROB_HIT_LIGHT
				* float(sys_NumLights)
#endif
				;
			lightSample.pdf      = (lightSample.distance * lightSample.distance) / (light.area * cosTheta); // Solid angle pdf. Assumes light.area != 0.0f.
		}
	}
}
// SAMPLE_LIGHT : SPOTLIGHT //
RT_FUNCTION void sample_light_SpotLight(float3 const& point, const float3 sample, LightSample& lightSample, LightDefinition& light, int sys_NumLights)                // Num of Lights
{

	lightSample.pdf = 0.0f; // Default return, invalid light sample (backface, edge on, or too near to the surface)

	float3 sample3f = sample * 2.0f - make_float3(1.0f);
	lightSample.position  = light.position + light.vecU * sample3f.x + light.vecV * sample3f.y; // The light sample position in world coordinates.
	lightSample.direction = (lightSample.position - point); // Sample direction from surface point to light sample position.
	lightSample.distance  = optix::length(lightSample.direction);
	
	// disk
	bool is_in_disk = true;
	if (light.is_clip)
	{
		float dist = light.clipping_planes.y;
		float3 disk_center = light.position + light.normal * dist;
		float3 normal      = light.normal;

		float dot_ = dot(normalize(point - disk_center), light.normal);
		//float rad  = 0.01745329252;
		//float phi  = (light.cone_dim.y) * rad;
		//float c    = dist / sinf(phi);
		//float d2   = cosf(phi) * c;
		is_in_disk = dot_ <= 0.0f ? true : false;
	}

	if (DENOMINATOR_EPSILON < lightSample.distance 
		&& is_in_disk
		)
	{
		lightSample.direction /= lightSample.distance; // Normalized direction to light.

		const float cosTheta = optix::dot(-lightSample.direction, (light.normal));
		if (DENOMINATOR_EPSILON < cosTheta) // Only emit light on the front side.
		{
			float spotlight_t    = compute_spotlight(lightSample.direction, light.normal , light.cone_dim.x, light.cone_dim.y, light.is_soft);
			lightSample.emission = light.emission 
#ifdef MULT_INV_PROB_HIT_LIGHT
				* float(sys_NumLights)
#endif
				* spotlight_t;
			lightSample.pdf      = (lightSample.distance * lightSample.distance) / (light.area * cosTheta); // Solid angle pdf. Assumes light.area != 0.0f.
		}
	}
}
// SAMPLE_LIGHT : POINT_LIGHT //
RT_FUNCTION void sample_light_Point_Light(float3 const& point, const float3 sample, LightSample& lightSample, LightDefinition& light, int sys_NumLights)
{
	lightSample.pdf = 0.0f; // Default return, invalid light sample (backface, edge on, or too near to the surface)

	lightSample.position  = light.position; //light.position + light.vecU * sample.x + light.vecV * sample.y; // The light sample position in world coordinates.
	lightSample.direction = (lightSample.position - point); // Sample direction from surface point to light sample position.
	lightSample.distance  = optix::length(lightSample.direction);
	if (DENOMINATOR_EPSILON < lightSample.distance)
	{
		lightSample.direction /= lightSample.distance; // Normalized direction to light.

		//const float cosTheta = optix::dot(-lightSample.direction, (light.normal));
		//if (DENOMINATOR_EPSILON < cosTheta) // Only emit light on the front side.
		{
			// Explicit light sample, must scale the emission by inverse probabilty to hit this light.
			lightSample.emission = light.emission 
#ifdef MULT_INV_PROB_HIT_LIGHT
				* float(sys_NumLights)
#endif
				;
			lightSample.pdf      = (lightSample.distance * lightSample.distance) / (light.area); // Solid angle pdf. Assumes light.area != 0.0f.
		}
	}
}
// SAMPLE_LIGHT : SPHERICAL //
RT_FUNCTION void sample_light_Spherical(float3 const& point, const float3 sample, LightSample& lightSample, LightDefinition& light, int sys_NumLights)
{
	lightSample.pdf = 0.0f; // Default return, invalid light sample (backface, edge on, or too near to the surface)

	
	float3 sample3f = sample * 2.0f - make_float3(1.0f);
	float3 light_vecW     = normalize(cross(light.vecU, light.vecV)) * length(light.vecU);
	lightSample.position  = light.position + light.vecU * sample3f.x + light.vecV * sample3f.y + light_vecW * sample3f.z; // The light sample position in world coordinates.
	lightSample.direction = (lightSample.position - point); // Sample direction from surface point to light sample position.
	lightSample.distance  = optix::length(lightSample.direction);
	if (DENOMINATOR_EPSILON < lightSample.distance)
	{
		lightSample.direction /= lightSample.distance; // Normalized direction to light.

		//const float cosTheta = optix::dot(-lightSample.direction, (light.normal));
		//if (DENOMINATOR_EPSILON < cosTheta) // Only emit light on the front side.
		{
			// Explicit light sample, must scale the emission by inverse probabilty to hit this light.
			lightSample.emission = 
				light.emission 
#ifdef MULT_INV_PROB_HIT_LIGHT
				* float(sys_NumLights)
#endif
				;
			lightSample.pdf      = (lightSample.distance * lightSample.distance) / (light.area); // Solid angle pdf. Assumes light.area != 0.0f.
		}
	}
}



RT_FUNCTION void sample_Light(float3 const& point,              // ray hit position
							  const float3 sample,              // float2 random numb
							  LightSample& lightSample,         // rand Light's index
							  LightDefinition& light,           // rand Light
							  int sys_NumLights)
{

	LightType type = light.type;
	if (!light.is_on)
	{
		lightSample.pdf = 0.0f;
		return;
	}

	if      (type == LIGHT_PARALLELOGRAM)   sample_light_parallelogram(point, sample, lightSample, light, sys_NumLights);
	else if (type == LIGHT_SPOTLIGHT)       sample_light_SpotLight(point, sample, lightSample, light, sys_NumLights);
	else if (type == LIGHT_SPHERICAL_LIGHT) sample_light_Spherical(point, sample, lightSample, light, sys_NumLights);
	else if (type == LIGHT_POINT_LIGHT)     sample_light_Point_Light(point, sample, lightSample, light, sys_NumLights);
}



