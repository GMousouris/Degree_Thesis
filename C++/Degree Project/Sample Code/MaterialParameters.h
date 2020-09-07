#pragma once
#include <unordered_map>
#include <string>

static const float DEFAULT_Kd[3] = { 0.7f , 0.7f , 0.7f };
static const float DEFAULT_Ks[3] = { 0.7f , 0.7f , 0.7f };
static const float DEFAULT_Kr[3] = { 0.25f, 0.25f, 0.25f };
static const float DEFAULT_Ka[3] = { 0.0f , 0.0f , 0.0f };
static const float DEFAULT_Phong_exp = 0.0f;

struct MaterialParameters
{
	std::string  name;
	float   Kd[3];
	float   Ks[3];
	float   Kr[3];
	float   Ka[3];
	float   phong_exp;


	MaterialParameters()
	{
		for (int i = 0; i < 3; i++)
		{
			Kd[i] = DEFAULT_Kd[i];
			Ks[i] = DEFAULT_Ks[i];
			Kr[i] = DEFAULT_Kr[i];
			Ka[i] = DEFAULT_Ka[i];
		}

		phong_exp = DEFAULT_Phong_exp;
	}
	MaterialParameters(optix::Material &mat)
	{
		//par_count = 5;

		getData(mat["Kd"]->getFloat3(),  Kd);
		getData(mat["Ks"]->getFloat3(),  Ks);
		getData(mat["Kr"]->getFloat3(),  Kr);
		getData(mat["Ka"]->getFloat3(),  Ka);
		phong_exp = mat["phong_exp"]->getFloat();
		
	}

	void getData(optix::float3 v, float * s)
	{
		//float data[3];
		s[0] = v.x;
		s[1] = v.y;
		s[2] = v.z;
	}
	const char * par_name(const int index)
	{
		switch (index)
		{
		case 0: return "Kd";
		case 1: return "Ks";
		case 2: return "Kr";
		case 3: return "Ka";
		case 4: return "phong_exp";

		}

		return "";
	}
	float * par(const int index)
	{
		switch (index)
		{
		case 0: return Kd;
		case 1: return Ks;
		case 2: return Kr;
		case 3: return Ka;
		case 4: return &phong_exp;
		}

		return 0;
	}

	void setKd(optix::float3& t)
	{
		Kd[0] = t.x;
		Kd[1] = t.y;
		Kd[2] = t.z;
	}
	void setKs(optix::float3& t)
	{
		Ks[0] = t.x;
		Ks[1] = t.y;
		Ks[2] = t.z;
	}
	void setKr(optix::float3& t)
	{
		Kr[0] = t.x;
		Kr[1] = t.y;
		Kr[2] = t.z;
	}
	void setKa(optix::float3& t)
	{
		Ka[0] = t.x;
		Ka[1] = t.y;
		Ka[2] = t.z;
	}
	void setPhongExp(float p)
	{
		phong_exp = p;
	}
	

	


};