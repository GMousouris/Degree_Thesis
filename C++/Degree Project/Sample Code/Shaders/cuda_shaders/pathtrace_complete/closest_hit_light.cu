/*

#include "RT_app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>


#include "per_ray_data.h"
#include "light_structs.h"
#include "shader_common.h"

rtDeclareVariable(rtObject, sysTopObject, , );
rtDeclareVariable(optix::Ray, theRay,                  rtCurrentRay, );
rtDeclareVariable(float,      theIntersectionDistance, rtIntersectionDistance, );
rtDeclareVariable(PerRayData, thePrd,                  rtPayload, );
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );


rtBuffer<LightDefinition> sysLightDefinitions;
rtDeclareVariable(int,    parLightIndex, , );  // Index into the sysLightDefinitions array.

// Very simple closest hit program just for rectangle area lights.
RT_PROGRAM void closesthit_light()
{

  thePrd.pos      = theRay.origin + theRay.direction * theIntersectionDistance; // Advance the path to the hit position in world coordinates.
  thePrd.distance = theIntersectionDistance; // Return the current path segment distance, needed for absorption calculations in the integrator.

  const float3 geoNormal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varGeoNormal)); // PERF Not really needed when it's know that light geometry is not under Transforms.

  const float cosTheta = optix::dot(thePrd.wo, geoNormal);
  thePrd.flags |= (0.0f <= cosTheta) ? FLAG_FRONTFACE : 0;

  thePrd.radiance = make_float3(0.0f); // Backside is black.

  if (thePrd.flags & FLAG_FRONTFACE) // Looking at the front face?
  {
    const LightDefinition light = sysLightDefinitions[parLightIndex];
    thePrd.radiance = light.emission;

    const float pdfLight = (thePrd.distance * thePrd.distance) / (light.area * cosTheta); // Solid angle pdf. Assumes light.area != 0.0f.
    if ((thePrd.flags & FLAG_DIFFUSE) && DENOMINATOR_EPSILON < pdfLight)
    {
      thePrd.radiance *= powerHeuristic(thePrd.pdf, pdfLight);
    }

  }

  thePrd.flags |= FLAG_TERMINATE;
}


*/