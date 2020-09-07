#pragma once

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "RT_app_config.h"
#include "per_ray_data.h"
#include "shader_common.h"
#include "random_number_generators.h"
#include "sampling_functions.h"

#include "light_structs.h"
#include "light_sample.h"

using namespace optix;




//rtDeclareVariable(Matrix4x4, sys_view_mat, , );
//rtDeclareVariable(Matrix4x4, sys_proj_mat, , );
rtDeclareVariable(Matrix4x4, sys_vp_mat, , );

rtDeclareVariable(int, sys_write_depth, , );
rtDeclareVariable(int, sys_only_write_depth, , );

rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(int, geometry_id, , );
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(rtObject, top_shadower, , );

rtDeclareVariable(float,    sys_SceneEpsilon, , );

rtDeclareVariable(int, sys_NumLights, , );
rtBuffer<LightDefinition> sys_LightDefinitions;
rtDeclareVariable(int, cast_shadow_ray, , );

// Semantic variables.
rtDeclareVariable(optix::Ray, ray,                  rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance,  rtIntersectionDistance, );

rtDeclareVariable(PerRayData_c, thePrd,                  rtPayload, );

 

rtDeclareVariable(float3, varGeoNormal, attribute geometric_normal, );
rtDeclareVariable(float3, varNormal, attribute shading_normal, );




//


rtDeclareVariable(int, sys_cast_reflections, , );
rtDeclareVariable(int, sys_use_fresnel_equations, , );
rtDeclareVariable(unsigned int, max_depth, , );
rtDeclareVariable(float3, sys_CameraW, ,);
rtDeclareVariable(float3, sys_CameraPosition, ,);
rtDeclareVariable(unsigned int, sys_sample_glossy_index, ,);
rtDeclareVariable(unsigned int, sys_sample_light_refl, ,);
rtDeclareVariable(unsigned int, sys_bsdf_light_refl, ,);
rtDeclareVariable(int, sys_const_light_model, , );

rtDeclareVariable(int, is_light, , );
rtDeclareVariable(int, is_sampler, , );

rtDeclareVariable(uint2, launch_index, rtLaunchIndex, ); // temp //

RT_FUNCTION void lambertian_surface_sample(float3 const& Kd, float3 const& Kr, float3 const& Ka, float const& exp )
{
  State state; // All in world space coordinates!

  state.geoNormal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varGeoNormal));
  state.normal    = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));

  thePrd.pos      = ray.origin + ray.direction * theIntersectionDistance; // Advance the path to the hit position in world coordinates.
  thePrd.distance = theIntersectionDistance; // Return the current path segment distance, needed for absorption calculations in the integrator.


  if ( sys_write_depth == 1 && thePrd.depth == 0 )
  {
        float4 p      = make_float4(thePrd.pos.x, thePrd.pos.y, thePrd.pos.z, 1.0f);
	    float4 proj_p = sys_vp_mat * p;
	    float3 ndc_p  = make_float3(proj_p.x,proj_p.y,proj_p.z) / proj_p.w;
	    float  depth  = ndc_p.z;

	    float f_depth;
	    f_depth = depth * 0.5f + 0.5f;
	    thePrd.depth_value = f_depth;
  }




  thePrd.flags |= (0.0f <= optix::dot(thePrd.wo, state.geoNormal)) ? FLAG_FRONTFACE : 0;
  if ((thePrd.flags & FLAG_FRONTFACE) == 0) // Looking at the backface?
  {
      state.geoNormal = -state.geoNormal;
      state.normal    = -state.normal;
  }

  
  thePrd.radiance   = make_float3(0.0f);
  thePrd.f_over_pdf = make_float3(0.0f);
  thePrd.pdf        = 0.0f;
  thePrd.flags      = (thePrd.flags & ~FLAG_DIFFUSE); // 

  float refl       = Kr.x;
  float ior        = Ka.x;
  float rand_value = 1.0f;
  float pattern    = 1.0f;

  if ( sys_cast_reflections == 1 &&  geometry_id != -100)
  {
     rand_value = rng2( thePrd.seed ).x;
  }

  float fresn = sys_use_fresnel_equations == 1? fresnel(thePrd.wi, state.normal, ior, refl) : 1.0f;
  
  
  int DIFFUSE_IND = 0;
  if (  refl  >= rand_value 
     && sys_cast_reflections == 1 )
  {
     
     if      ( sys_sample_glossy_index == 0 )
        sample_bsdf_specular_reflection(Kd, refl, exp, state, thePrd);
     else if ( sys_sample_glossy_index == 1 )
	    sample_bsdf_specular_glossy_reflection  ( Kd, refl, ior, exp,  state, thePrd );
     else if ( sys_sample_glossy_index == 2 )
	    sample_bsdf_specular_glossy_reflection2 ( Kd, refl, ior, exp,  state, thePrd );
     else if ( sys_sample_glossy_index == 3 )
        sample_bsdf_specular_glossy_reflection2 ( Kd, refl, ior, exp,  state, thePrd );

	 if( sys_use_fresnel_equations == 1 )
	      thePrd.f_over_pdf *= fresn;

  }
  else
  {
     DIFFUSE_IND = 1;
	   sample_bsdf_diffuse_reflection( Kd, state, thePrd );
     if(    sys_cast_reflections == 1 
         && geometry_id != -100
	       && sys_use_fresnel_equations == 1 )
		 thePrd.f_over_pdf *= (1.0f - fresn);

  }

  

#if USE_NEXT_EVENT_ESTIMATION
  
  if ( ( DIFFUSE_IND == 1 ) && ( 0 < sys_NumLights && cast_shadow_ray == 1 ) )
  {
      // generate a random float2 //
      const float2 sample    = rng2(thePrd.seed); 
      const float3 sample3f  = make_float3( sample.x, sample.y, rng(thePrd.seed) );

      int num_lights = 0;
      for(int i = 0; i< sys_NumLights; i++)
      {
          if(sys_LightDefinitions[i].is_on) 
          {
		      num_lights++;
          }  
      }
      
	  
	  
      LightSample lightSample;
	  LightDefinition rand_light;
      lightSample.index          = optix::clamp( static_cast<int>(floorf(rng(thePrd.seed) * sys_NumLights)), 0, sys_NumLights - 1 );  // picking a random light
	  rand_light = sys_LightDefinitions[ lightSample.index ];
	  sample_Light( 
	       thePrd.pos,
           sample3f,
		   lightSample,
		   rand_light,
		   num_lights);
	  
      if ( lightSample.pdf > 0.0f ) // Useful light sample?
      {
      
         float4 bsdf_pdf;
         bsdf_pdf = ( DIFFUSE_IND == 1 ) ? eval_bsdf_diffuse_reflection(  Kd, state, thePrd,       lightSample.direction ) 
                                         : eval_bsdf_specular_reflection( Kd, refl, state, thePrd, lightSample.direction );
         
         if ( 0.0f < bsdf_pdf.w && isNotNull(make_float3(bsdf_pdf)) )
         {
        
             PerRayData_shadow_c prdShadow;
             prdShadow.visible = true; 
             optix::Ray ray    = optix::make_Ray(thePrd.pos, lightSample.direction, 1, sys_SceneEpsilon, lightSample.distance - sys_SceneEpsilon); // Shadow ray.
             rtTrace( top_shadower , ray, prdShadow );

             if ( prdShadow.visible )
             {
                 const float weight = powerHeuristic( lightSample.pdf, bsdf_pdf.w );
                 thePrd.radiance   += pattern 
                  * make_float3(bsdf_pdf)
                  * lightSample.emission 
                  * ( weight * optix::dot(lightSample.direction, state.normal) / lightSample.pdf );
             }
        }
    }

  }
  else if ( cast_shadow_ray == 0 && DIFFUSE_IND == 1 && max_depth > 1 && geometry_id != -100 )
  {

    const float2 sample = rng2(thePrd.seed);
    LightSample lightSample; // Sample one of many lights. 
    //sample_light_constant(thePrd.pos, sample, lightSample, 1);
	  sample_light_camera_constant(thePrd.pos, sys_CameraW, sample, lightSample, 1);

    float4 bsdf_pdf;
	  bsdf_pdf = eval_bsdf_diffuse_reflection(Kd, state, thePrd, lightSample.direction );
    const float weight = powerHeuristic(lightSample.pdf, bsdf_pdf.w);
	  float dot_weight = dot(lightSample.direction, state.normal);
	  dot_weight = clamp(dot_weight, 0.1f, 100.0f);
	  thePrd.radiance += pattern * make_float3(bsdf_pdf) * lightSample.emission * ( weight * dot_weight / lightSample.pdf );
    

  }
  else if ( cast_shadow_ray == 0 && DIFFUSE_IND == 1 && max_depth == 1 || geometry_id == -100 )
  {   

 
	  if (sys_const_light_model == 0)
	  {
	      
		  float const_dist = 150.0f;
		  float3 light_pos = thePrd.pos - sys_CameraW * const_dist;
		  float3 light_dir = normalize(sys_CameraPosition - thePrd.pos);
		  float3 viewDir = normalize(sys_CameraPosition - thePrd.pos);
		  float3 halfV = normalize(viewDir + light_dir);
		  float NdotL   = fabsf(dot(light_dir, state.normal));
		  
		  float3 fade  = make_float3(1.0f);
		  float fade_s = 0.45f;
		  if(    geometry_id != -100
		      && NdotL < fade_s 
			  )
		  {
		      float t = (fade_s - NdotL) / fade_s;
			  t       = clamp(t, 0.0f, 1.0f);

			  fade.x = t*(0.0f) + (1.0f - t)*1.0f;
			  fade.y = t*(0.0f) + (1.0f - t)*1.0f;
			  fade.z = t*(0.0f) + (1.0f - t)*1.0f;
		  }
		  float NdotH = fabsf(dot(halfV, state.normal));

		  float spotEffect;
		  spotEffect  = 1.f * (dot(sys_CameraW, -state.normal));
		  spotEffect *= spotEffect;
		  spotEffect  = clamp(spotEffect, 0.07f, 100.0f); // 0.1 - 100.0f

		  float3 irr       = spotEffect * NdotL / const_dist * make_float3(1.0f);
		  float3 diff_refl = irr * Kd;
		  float specularNorm = (0.01f + 8.0f) / (8.0f * M_PIf);

		  float3 spec_refl = make_float3(0.0f);
		  if (NdotL > 0.0f)
			  spec_refl = irr * specularNorm * powf(NdotH, 10.0f + 0.001f);
		  thePrd.radiance += pattern * 2500.0f * diff_refl * fade + spec_refl;
	  }
	  else if (sys_const_light_model == 1)
	  {
		  float3 specular;
		  float phong_exp;
		  float Ldist;
		  float3 emission;

		  if( geometry_id == -100 )
		  {
			 phong_exp = 10.0f;
			 Ldist     = 5000.0f;
			 emission  = make_float3(0.7f);
			 specular  = make_float3(0.2f);
		  }
		  else
		  {
		     phong_exp = 10.0f;
			 Ldist     = 4000.0f;
			 emission  = make_float3(0.9f);
			 specular  = make_float3(0.5f);
		  }
		  
		  float3 L = normalize(sys_CameraPosition - thePrd.pos);
		  float nDl = dot(state.normal, L);
		  //nDl *= nDl;
		  //nDl  = clamp(nDl, 0.0f, 1.0f);
		  float d = dot(sys_CameraW, -state.normal);
		  //d = clamp(d, 0.5f, 1.0f);
		  d = 1.0f;

		  float3 Lc        = emission;
		  thePrd.radiance += pattern * Kd * nDl * d * Lc;
		  float3 H = normalize(L - ray.direction);
		  float nDh = dot(state.normal, H);

		  if (nDh > 0)
		  {
			  float power = pow(nDh, phong_exp);
			  thePrd.radiance += specular * power * Lc;
		  }

	  }

	 
  }
    
  
#endif // USE_NEXT_EVENT_ESTIMATION

}

















RT_FUNCTION void closest_hit_Light( float3 const& Kd )
{ 
     
     LightDefinition light;
     for(int i = 0; i< sys_NumLights; i++)
	 {
	      if( sys_LightDefinitions[i].id == geometry_id )
		  {
		       light = sys_LightDefinitions[i];
			   break;
		  }
	 }

     thePrd.pos      = ray.origin + ray.direction * theIntersectionDistance;
     thePrd.distance = theIntersectionDistance; 

     const float3 geoNormal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varGeoNormal)); // PERF Not really needed when it's know that light geometry is not under Transforms.
     thePrd.radiance = make_float3(0.0f);

	   const float cosTheta = optix::dot(thePrd.wo, geoNormal);
	   thePrd.flags |= (0.0f <= cosTheta) ? FLAG_FRONTFACE : 0;

     if (thePrd.flags & FLAG_FRONTFACE)
     {
         
		 thePrd.radiance = make_float3(1.0f);
		 //thePrd.radiance = light.emission;

		 float light_area = light.area;
         const float pdfLight = (thePrd.distance * thePrd.distance) / ( light_area * cosTheta); 
         if ((thePrd.flags & FLAG_DIFFUSE) && DENOMINATOR_EPSILON < pdfLight)
         {
              thePrd.radiance *= powerHeuristic(thePrd.pdf, pdfLight);
         }

      }

      thePrd.flags |= FLAG_TERMINATE;
}

RT_FUNCTION void closest_hit_Sampler(float3 const& kd)
{

}










rtDeclareVariable(int, sys_focusedObject, , );
rtDeclareVariable(int, sys_selectedObject, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtBuffer<int> sys_SelectedObjectBuffer;

rtBuffer<float3>       sys_mouseHit_buffer_Output;
rtBuffer<float>        sys_custom_dir_Output;

rtDeclareVariable(float3, Kd, , );
rtDeclareVariable(float3, Kr, , );
rtDeclareVariable(float3, Ka, , );
rtDeclareVariable(float, phong_exp, , );

rtDeclareVariable(unsigned int, sys_abort, , );
rtDeclareVariable(int,    sys_is_custom_dir , ,);



rtDeclareVariable(float3, sys_m_cameraW, , );


//-----------------------------------------------------------------------------
//
//  closest-hit
//
//-----------------------------------------------------------------------------
RT_PROGRAM void closest_hit()
{
   
  if (thePrd.mouse_ray == 1)
  {
    if (geometry_id >= 0)
    {
      sys_SelectedObjectBuffer[0] = geometry_id;
      thePrd.selected = geometry_id;

	  
      float3 normal    = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));
	  
	  float3 o   =  ray.origin;//rtTransformNormal(RT_OBJECT_TO_WORLD, ray.origin);
	  float3 dir =  ray.direction; //rtTransformNormal(RT_OBJECT_TO_WORLD, ray.direction);
	  
	  float3 pos       = o + dir * theIntersectionDistance; 
	  sys_mouseHit_buffer_Output[0] = pos;
	  sys_mouseHit_buffer_Output[1] = normal;
	  sys_mouseHit_buffer_Output[2] = make_float3(1.0f);
	  
	  //
	  
      return;

    }
    else
    {
      thePrd.selected = -1;
      return;
    }
  }
  else
  {
    if ((sys_focusedObject == geometry_id) || (sys_selectedObject == geometry_id) && geometry_id >= 0)
      thePrd.selected = geometry_id;
    else
      thePrd.selected = -1;
  }
 
  if( is_light == 1 )
     closest_hit_Light( Kd );
  else
     lambertian_surface_sample(Kd, Kr, Ka, phong_exp);
}




rtTextureSampler<float4, 2> Kd_map;
//-----------------------------------------------------------------------------
//
//  closest-hit Textured
//
//-----------------------------------------------------------------------------
RT_PROGRAM void closest_hit_textured()
{
    
  if (thePrd.mouse_ray == 1)
  { 
    if (geometry_id >= 0)
    {
        
      sys_SelectedObjectBuffer[0] = geometry_id;
      thePrd.selected = geometry_id;

      float3 normal      = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));
	  
	  float3 pos       = ray.origin + ray.direction * theIntersectionDistance; 
	  sys_mouseHit_buffer_Output[0] = pos;
	  sys_mouseHit_buffer_Output[1] = normal;
	  sys_mouseHit_buffer_Output[2] = make_float3(1.0f);

      return;

    }
    else
    {
      thePrd.selected = -1;
      return;
    }
  }
  else
  {
    if ((sys_focusedObject == geometry_id) || (sys_selectedObject == geometry_id) && geometry_id >= 0)
      thePrd.selected = geometry_id;
    else
      thePrd.selected = -1;
  }


  const float3 Kd_val = make_float3(tex2D(Kd_map, texcoord.x, texcoord.y));
  lambertian_surface_sample(Kd_val, Kr, Ka,  phong_exp);
  
}



