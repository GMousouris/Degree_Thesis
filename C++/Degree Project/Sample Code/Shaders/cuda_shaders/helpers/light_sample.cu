

#include "RT_app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "per_ray_data.h"
#include "light_definition.h"
#include "shader_common.h"
#include "sampling_functions.h"

rtBuffer<LightDefinition> sysLightDefinitions;
rtDeclareVariable(int,    sysNumLights, , ); // PERF Used many times and faster to read than sysLightDefinitions.size().




// Note that all light sampling routines return lightSample.direction and lightSample.distance in world space!

RT_CALLABLE_PROGRAM void sample_light_constant(float3 const& point, const float2 sample, LightSample& lightSample)
{
  unitSquareToSphere(sample.x, sample.y, lightSample.direction, lightSample.pdf);

  // Environment lights do not set the light sample position!
  lightSample.distance = RT_DEFAULT_MAX; // Environment light.

  // Explicit light sample. White scaled by inverse probabilty to hit this light.
  lightSample.emission = make_float3(sysNumLights);
}


RT_CALLABLE_PROGRAM void sample_light_parallelogram(float3 const& point, const float2 sample, LightSample& lightSample)
{
  lightSample.pdf = 0.0f; // Default return, invalid light sample (backface, edge on, or too near to the surface)

  const LightDefinition light = sysLightDefinitions[lightSample.index]; // The light index is picked by the caller!

  lightSample.position  = light.position + light.vecU * sample.x + light.vecV * sample.y; // The light sample position in world coordinates.
  lightSample.direction = lightSample.position - point; // Sample direction from surface point to light sample position.
  lightSample.distance  = optix::length(lightSample.direction);
  if (DENOMINATOR_EPSILON < lightSample.distance)
  {
    lightSample.direction /= lightSample.distance; // Normalized direction to light.
 
    const float cosTheta = optix::dot(-lightSample.direction, light.normal);
    if (DENOMINATOR_EPSILON < cosTheta) // Only emit light on the front side.
    {
      // Explicit light sample, must scale the emission by inverse probabilty to hit this light.
      lightSample.emission = light.emission * float(sysNumLights); 
      lightSample.pdf      = (lightSample.distance * lightSample.distance) / (light.area * cosTheta); // Solid angle pdf. Assumes light.area != 0.0f.
    }
  }
}
