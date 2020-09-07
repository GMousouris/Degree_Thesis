#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "per_ray_data.h"

using namespace optix;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//        Phong shader : Any_hit_shadow
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );
static __device__ void phong_shadow()
{

	// this material is opaque, so it fully attenuates all shadow rays
	prd_shadow.attenuation = optix::make_float3(0.0f);
	rtTerminateRay();

}

