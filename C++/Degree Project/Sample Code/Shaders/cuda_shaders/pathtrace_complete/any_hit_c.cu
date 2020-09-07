#include <optix.h>
#include "per_ray_data.h"

using namespace optix;



rtDeclareVariable(PerRayData_shadow_c, thePrdShadow, rtPayload, );
RT_PROGRAM void any_hit()
{
    thePrdShadow.visible = false;
    rtTerminateRay();
}
