#include "optixu/optixpp_namespace.h"
#include "optix.h"

using namespace optix;

struct ParallelogramLight
{

	float3 pos;
	float3 v1, v2;
	float3 normal;
	float3 color;
	bool casts_shadow;
};