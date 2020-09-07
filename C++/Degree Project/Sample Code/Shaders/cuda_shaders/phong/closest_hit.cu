#pragma once
#include "closest_hit_functions.h"


using namespace optix;

rtDeclareVariable(float3, Ka, , );
rtDeclareVariable(float3, Kd, , );
rtDeclareVariable(float3, Ks, , );
rtDeclareVariable(float3, Kr, , );
rtDeclareVariable(float, phong_exp, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(int, geometry_id, , );
rtDeclareVariable(int, sys_focusedObject, , );
rtDeclareVariable(int, sys_selectedObject, , );



RT_PROGRAM void closest_hit()
{

	
	if (prd.mouse_ray == 1)
	{
		if (geometry_id >= 0)
		{ 
			sys_SelectedObjectBuffer[0] = geometry_id;
			prd.selected = geometry_id;
			return;
			
		}
		else
		{
			prd.selected = -1;
			return;
		}
	}
	else
	{
		if ((sys_focusedObject == geometry_id) || (sys_selectedObject == geometry_id) && geometry_id >= 0)
			prd.selected = geometry_id;
		else
			prd.selected = -1;
	}

	

	
	float3 world_shading_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 world_geometric_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));
	float3 ffnormal = faceforward(world_shading_normal, -ray.direction, world_geometric_normal);

	
	phong_closest_hit(Kd, Ka, Ks, Kr, phong_exp, ffnormal);
}


rtTextureSampler<float4, 2> Kd_map;
rtDeclareVariable(float3, texcoord, attribute texcoord, );

RT_PROGRAM void closest_hit_textured()
{

	if (prd.mouse_ray == 1)
	{
		if (geometry_id >= 0)
		{
			sys_SelectedObjectBuffer[0] = geometry_id;
			prd.selected = geometry_id;
			return;
		}
		else
		{
			prd.selected = -1;
			return;
		}
	}
	else
	{
		if ((sys_focusedObject == geometry_id) || (sys_selectedObject == geometry_id) && geometry_id >= 0)
		{
			prd.selected = geometry_id;
		}
		else
		{
			prd.selected = -1;
		}
	}


	
	float3 world_shading_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 world_geometric_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));
	float3 ffnormal = faceforward(world_shading_normal, -ray.direction, world_geometric_normal);
	const float3 Kd_val = make_float3(tex2D(Kd_map, texcoord.x, texcoord.y));

	
	phong_closest_hit(Kd_val, Ka, Ks, Kr, phong_exp, ffnormal);
	
}
