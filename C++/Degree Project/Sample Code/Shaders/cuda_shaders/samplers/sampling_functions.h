#include <math.h>
#include "RT_app_config.h"
#include "random_number_generators.h"
#include <math.h>


using namespace optix;

RT_FUNCTION float modulo(float const& x)
{
	return x - floor(x);
}
RT_FUNCTION float rad(float const& d)
{
	return d * 0.0174532925f;
}

RT_FUNCTION void unitSquareToSphere(const float u, const float v, float3& p)
{
	p.z = 1.0f - 2.0f * u;
	float r = 1.0f - p.z * p.z;
	r = (0.0f < r) ? sqrtf(r) : 0.0f;

	const float phi = v * 2.0f * M_PIf;
	p.x = r * cosf(phi);
	p.y = r * sinf(phi);

}
RT_FUNCTION void unitSquareToSphere(const float u, const float v, float3& p, float& pdf)
{
	p.z = 1.0f - 2.0f * u;
	float r = 1.0f - p.z * p.z;
	r = (0.0f < r) ? sqrtf(r) : 0.0f;

	const float phi = v * 2.0f * M_PIf;
	p.x = r * cosf(phi);
	p.y = r * sinf(phi);

	pdf = 0.25f * M_1_PIf;  // == 1.0f / (4.0f * M_PIf)
}
RT_FUNCTION void alignVector(float3 const& axis, float3& w)
{
	// Align w with axis.
	const float s = copysign(1.0f, axis.z);
	w.z *= s;
	const float3 h = make_float3(axis.x, axis.y, axis.z + s);
	const float  k = optix::dot(w, h) / (1.0f + fabsf(axis.z));
	w = k * h - w;
}
RT_FUNCTION void unitSquareToCosineHemisphere(const float2 sample, float3 const& axis, float3& w)
{
	// Choose a point on the local hemisphere coordinates about +z.
	const float theta = 2.0f * M_PIf * sample.x;
	const float r = sqrtf(sample.y);
	w.x = r * cosf(theta);
	w.y = r * sinf(theta);
	w.z = 1.0f - w.x * w.x - w.y * w.y;
	w.z = (0.0f < w.z) ? sqrtf(w.z) : 0.0f;

	// Align with axis.
	alignVector(axis, w);
}
RT_FUNCTION void unitSquareToCosineHemisphere(const float2 sample, float3 const& axis, float3& w, float& pdf)
{
	// Choose a point on the local hemisphere coordinates about +z.
	const float theta = 2.0f * M_PIf * sample.x;
	const float r = sqrtf(sample.y);
	w.x = r * cosf(theta);
	w.y = r * sinf(theta);
	w.z = 1.0f - w.x * w.x - w.y * w.y;
	w.z = (0.0f < w.z) ? sqrtf(w.z) : 0.0f;

	pdf = w.z * M_1_PIf;

	// Align with axis.
	alignVector(axis, w);
}


RT_FUNCTION float getReflProbability(float3 const& Kd, float const& Kr)
{
	float p_d = Kd.x + Kd.y + Kd.z;
	float p_r = Kr;
	float P = 1.0f - p_d / (p_d + p_r);
	return P;
}

RT_FUNCTION float fresnel(const float3& I, const float3& N, const float ior,const float& kr)
{
	float cosi = dot(I, N);
	cosi = clamp( cosi , -1.0f, 1.0f);
	float etai = 1.0f;
	float etat = ior;

	if (cosi > 0.0f)
	{
		float t = etai;
		etai = etat;
		etat = t;
	}
	
	// Compute sini using Snell's law
	float sint = etai / etat * sqrtf( max(0.f, 1.0f - cosi * cosi));
	
	// Total internal reflection
	if (sint >= 1) 
	{
		return 1.0f;
	}
	else 
	{
		float cost = sqrtf( max(0.f, 1 - sint * sint));
		cosi = fabsf(cosi);
		float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
		float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
		return ((Rs * Rs + Rp * Rp) * 0.5f);
	}
	
}



RT_FUNCTION void sample_bsdf_diffuse_reflection(float3 const& Kd, State const& state, PerRayData_c& prd)
{

	// Cosine weighted hemisphere sampling for Lambert material.
	unitSquareToCosineHemisphere(rng2(prd.seed), state.normal, prd.wi, prd.pdf);

	//float z1 = rng2(prd.seed).x;
	//float z2 = rng2(prd.seed).y;
	//float3 p;
	//optix::cosine_sample_hemisphere(z1, z2, p);
	//optix::Onb onb(state.normal);
	//onb.inverse_transform(p);
	//prd.wi = p;
	//prd.pdf = 1.0f;

	if (prd.pdf <= 0.0f || optix::dot(prd.wi, state.geoNormal) <= 0.0f)
	{
		prd.flags |= FLAG_TERMINATE;
		return;
	}

	// This would be the universal implementation for an arbitrary sampling of a diffuse surface.
	// prd.f_over_pdf = parameters.albedo * (M_1_PIf * fabsf(optix::dot(prd.wi, state.normal)) / prd.pdf); 

	// PERF Since the cosine-weighted hemisphere distribution is a perfect importance-sampling of the Lambert material,
	// the whole term ((M_1_PIf * fabsf(optix::dot(prd.wi, state.normal)) / prd.pdf) is always 1.0f here!
	//prd.f_over_pdf = Kd * fabsf( dot(prd.wi, state.normal));
	prd.f_over_pdf = Kd;//*M_1_PIf * fabsf(dot(prd.wi, state.normal)) / prd.pdf;
	prd.flags |= FLAG_DIFFUSE; // Direct lighting will be done with multiple importance sampling.
}

RT_FUNCTION void sample_bsdf_specular_reflection(       float3 const& Kd,  float const& refl, float const& exp, State const& state, PerRayData_c& prd)
{

	prd.wi = optix::reflect(-prd.wo, state.normal);

	if (optix::dot(prd.wi, state.geoNormal) <= 0.0f) // Do not sample opaque materials below the geometric surface.
	{
		prd.flags |= FLAG_TERMINATE;
		return;
	}

	prd.pdf = 1.0f;
	prd.f_over_pdf = Kd; //* fabsf(dot(state.normal, prd.wi));
	//prd.pdf = 1.0f;

}

RT_FUNCTION void sample_bsdf_specular_glossy_reflection(float3 const& Kd,  float const& Kr, float const& ior, float const& exp, State const& state, PerRayData_c& prd)
{
	float e = exp * 8.f;
	float3 dv;
	float pdf;
	unitSquareToCosineHemisphere(rng2(prd.seed), state.normal, dv,pdf);
	dv *= e;

	// fresnel test
	//float refl = Kr;
	//fresnel(prd.wi, state.normal, ior, refl);
	//refl *= 2.5f;
	//
	
	float3 r = optix::reflect(-prd.wo, state.normal);
	float3 r_ = r + make_float3(rng(prd.seed)* dv.x * 0.0724f, rng2(prd.seed).x * dv.y * 0.05f, rng2(prd.seed).y * dv.z * 0.0764f);
	prd.wi = normalize(r_);
	if (optix::dot(prd.wi, state.geoNormal) <= 0.0f) // Do not sample opaque materials below the geometric surface.
	{
		prd.flags |= FLAG_TERMINATE;
		return;
	}

	
	prd.pdf = 1.0f; //2.0f * M_1_PIf;
	prd.f_over_pdf = Kd;//refl * Kd * fabsf(dot(r, prd.wi)) / prd.pdf;
	
}
RT_FUNCTION void sample_bsdf_specular_glossy_reflection2(float3 const& Kd, float const& Kr, float const& ior, float const& exp, State const& state, PerRayData_c& prd)
{

	//
	//float refl = Kr;
	//fresnel(prd.wi, state.normal, ior, refl);
	//refl *= 2.5f;
	//

	float3 r = optix::reflect(-prd.wo, state.normal);
	float3 sp;
	float pdf;
	unitSquareToCosineHemisphere(rng2(prd.seed), state.normal, sp, pdf);
	float3 _r = r + rng(prd.seed) * exp * sp;
	_r = normalize(_r);
	prd.wi = _r;

	

	if (dot(state.geoNormal, prd.wi) <= 0)
	{
		prd.flags |= FLAG_TERMINATE;
		return;
	}

	prd.wi = _r;

	//prd.pdf = 1.0f;
	//prd.f_over_pdf = Kd;

	prd.pdf = 1.0f;
	prd.f_over_pdf = Kd; //* refl * fabsf(dot(r, prd.wi)) / prd.pdf;
	//prd.f_over_pdf = Kd * fabsf(dot(state.normal, prd.wi)) / prd.pdf; //* (fabsf(dot(prd.wi, r)));
	//prd.f_over_pdf = Kd * (M_1_PIf * fabsf(optix::dot(prd.wi, state.normal)) / prd.pdf);
}
RT_FUNCTION void sample_bsdf_specular_glossy_reflection3(float3 const& Kd, float const& Kr, float const& exp, State const& state, PerRayData_c& prd)
{
	
	
}



RT_FUNCTION float4 eval_bsdf_diffuse_reflection(float3 const& Kd, State const& state, PerRayData_c const& prd, float3 const& wiL)
{
	const float3 f   = Kd * M_1_PIf;
	const float  pdf = fmaxf(0.0f, optix::dot(wiL, state.normal) * M_1_PIf);
	return make_float4(f, pdf);
}
RT_FUNCTION float4 eval_bsdf_specular_reflection(float3 const& Kd,float const& refl, State const& state, PerRayData_c const& prd, float3 const& wiL)
{
	//const float3 f = Kd * M_1_PIf;
	//const float  pdf = fmaxf(0.0f, optix::dot(wiL, state.normal) * M_1_PIf);
	//return make_float4(f, pdf);
	return make_float4(0.0f);
}


RT_FUNCTION float compute_checkers_pattern(PerRayData_c const& prd, State const& state, float3 const& tex)
{
	float angle = rad(0.0f);
	float s = tex.x * cos(angle) - tex.y * sin(angle);
	float t = tex.y * cos(angle) + tex.x * sin(angle);

	float scaleS = 24.05f;
	float scaleT = 24.05f;

	float pattern = (modulo(t * scaleS) < 0.04f) ^ (modulo(s * scaleT) < 0.04f);
	pattern       = clamp(pattern, 0.35f, 1.0f);//0.45f);
	return pattern;
}