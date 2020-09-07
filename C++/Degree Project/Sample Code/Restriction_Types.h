#pragma once
#include "Restrictions.h"
#include "optixu/optixu_math_namespace.h"

#define Restriction_DISTANCE_R
class Restriction_Distance : public Restriction
{

public:

	~Restriction_Distance();
	Restriction_Distance();
	Restriction_Distance(class SceneObject* subject, class SceneObject* object);
	Restriction_Distance(class SceneObject* subject, class SceneObject* object, float min_dist, float max_dist);
	Restriction_Distance(class SceneObject* subject, class SceneObject* object, optix::float2 dist);

	virtual bool is_Valid( optix::float2* error = 0, bool dist_too = false);
	virtual float get_Error();
	
};


#define Restriction_FOCUS_R
class Restriction_Focus : public Restriction
{

public:

	~Restriction_Focus();
	Restriction_Focus();
	Restriction_Focus(class SceneObject* subject, class SceneObject* object);
	Restriction_Focus(class SceneObject* subject, class SceneObject* object, optix::float2 delta);

	virtual bool is_Valid( optix::float2* error = 0, bool dist_too = false);
	virtual float get_Error();

	virtual bool checkForValidState();

};

#define Restriction_CONVERSATION_R
class Restriction_Conversation : public Restriction
{


public:

	~Restriction_Conversation();
	Restriction_Conversation();
	Restriction_Conversation(class SceneObject* subject, class SceneObject* object);
	Restriction_Conversation(class SceneObject* subject, class SceneObject* object, optix::float2 delta);

	bool is_dist_Valid(float * error = 0);

	virtual bool is_Valid( optix::float2* error = 0, bool dist_too = false );
	virtual float get_Error();
	optix::float2 get_Error_2f();
	virtual bool checkForValidState();
};

#define Restriction_ALIGNEMENT_R
class Restriction_Alignement : public Restriction
{

public:
	~Restriction_Alignement();
	Restriction_Alignement();
	Restriction_Alignement(class SceneObject* subject, class SceneObject* object);
	Restriction_Alignement(class SceneObject* subject, class SceneObject* object, optix::float3 u, optix::float3 v);
	Restriction_Alignement(class SceneObject* subject, class SceneObject* object, optix::float3 u, optix::float3 v , float ang_tolerance);

	void set_SubjectFF(optix::float3 ff);
	void set_ObjectFF(optix::float3 ff);
	optix::float3 get_SubjectFF();
	optix::float3 get_ObjectFF();
	void set_AngularTolerance(float tol);
	float get_AngularTolerance();
	

protected:
	optix::float3 subject_ff;
	optix::float3 object_ff;
	float angular_tolerance;

};



