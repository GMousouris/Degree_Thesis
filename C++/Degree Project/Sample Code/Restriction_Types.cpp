#include "Restriction_Types.h"
#include "SceneObject.h"
#include "GeometryFunctions.h"


#ifdef Restriction_DISTANCE_R


Restriction_Distance::~Restriction_Distance()
{

}
Restriction_Distance::Restriction_Distance()
	: Restriction(0,0,DISTANCE_R)
{

}
Restriction_Distance::Restriction_Distance(SceneObject* subject, SceneObject* object)
	: Restriction(subject, object, DISTANCE_R)
{

}
Restriction_Distance::Restriction_Distance(SceneObject* subject, SceneObject* object, float min_dist, float max_dist)
	: Restriction(subject, object, optix::make_float2(min_dist,max_dist), DISTANCE_R)
{

}
Restriction_Distance::Restriction_Distance(SceneObject* subject, SceneObject* object, optix::float2 d)
	: Restriction(subject, object, d, DISTANCE_R)
{

}

bool Restriction_Distance::is_Valid( optix::float2* error, bool dist_too)
{
	float dist = length(subject->getCentroid_Transformed() - object->getCentroid_Transformed());

	if (error != 0)
	{
		error->x = dist;
	}

	if (dist <= m_dist_limit.y && dist >= m_dist_limit.x)
	{
		return true;
		//if (error != 0) error->x = 0.0f;
	}
	else
	{
		//if (error != 0) error->x = (dist - m_dist_limit.y);
		return false;
	}
}
float Restriction_Distance::get_Error()
{
	return 0.0f;
}


#endif


#ifdef Restriction_ALIGNEMENT_R

Restriction_Alignement::~Restriction_Alignement()
{

}
Restriction_Alignement::Restriction_Alignement()
	: Restriction(ALIGNEMENT_R),
	  subject_ff(optix::make_float3(0.0f)), object_ff(optix::make_float3(0.0f))
{

}
Restriction_Alignement::Restriction_Alignement(SceneObject* subject, SceneObject* object)
	: Restriction(subject, object, ALIGNEMENT_R),
	  subject_ff( optix::make_float3(0.0f)) , object_ff( optix::make_float3(0.0f))
{

}
Restriction_Alignement::Restriction_Alignement(SceneObject* subject, SceneObject* object, optix::float3 subject_ff, optix::float3 object_ff)
	: Restriction(subject, object, ALIGNEMENT_R),
	  subject_ff(subject_ff), object_ff(object_ff), angular_tolerance(0.0f)
	
{

}
Restriction_Alignement::Restriction_Alignement(SceneObject* subject, SceneObject* object, optix::float3 subject_ff, optix::float3 object_ff, float ang_tol)
	: Restriction(subject, object, ALIGNEMENT_R), 
	  subject_ff(subject_ff) , object_ff(object_ff), angular_tolerance(ang_tol)

{

}


void Restriction_Alignement::set_AngularTolerance(float tol)
{
	angular_tolerance = tol;
}
void Restriction_Alignement::set_SubjectFF(optix::float3 ff)
{
	subject_ff = ff;
}
void Restriction_Alignement::set_ObjectFF(optix::float3 ff)
{
	object_ff = ff;
}

float Restriction_Alignement::get_AngularTolerance()
{
	return angular_tolerance;
}
optix::float3 Restriction_Alignement::get_SubjectFF()
{
	return subject_ff;
}
optix::float3 Restriction_Alignement::get_ObjectFF()
{
	return object_ff;
}

#endif


#ifdef Restriction_FOCUS_R

Restriction_Focus::~Restriction_Focus()
{

}
Restriction_Focus::Restriction_Focus()
	: Restriction(0, 0, FOCUS_R)
{

}

Restriction_Focus::Restriction_Focus(SceneObject* subject, SceneObject* object)
	: Restriction(subject, object, FOCUS_R)
{

}
Restriction_Focus::Restriction_Focus(SceneObject* subject, SceneObject* object, optix::float2 delta)
	: Restriction(subject, object, optix::make_float2(0.0f), delta, FOCUS_R)
{

}

bool Restriction_Focus::is_Valid( optix::float2* error, bool dist_too)
{
	if (isTempInactive()) return false;
	bool is_valid = true;

	optix::float3 p0 = ZERO_3f;
	optix::float3 p1 = p0 + GLOBAL_AXIS[2];
	optix::float4 p0_t = subject->getTransformationMatrix() * optix::make_float4(p0, 1.0f);
	optix::float4 p1_t = subject->getTransformationMatrix() * optix::make_float4(p1, 1.0f);
	optix::float4 subj_dir_t = p1_t - p0_t;
	optix::float3 subj_dir = optix::make_float3(subj_dir_t.x, subj_dir_t.y, subj_dir_t.z); // subject's direction ( faceVectors direction )
	optix::float3 r_direction = (object->getCentroid_Transformed() - subject->getTranslationAttributes()); // wanted direction //
	
	float du    = dot( normalize(subj_dir), normalize(r_direction) );
	float theta = acosf(du);
	theta       = Geometry::GENERAL::toDegrees(theta);
    
	if (error != 0)
	{
		error->x = theta;
	}

	if (theta <= m_angle_delta.x)
	{
		return true;
	}
	else
	{
		return false;
	}

}
float Restriction_Focus::get_Error()
{
	return 0.0f;
}
bool Restriction_Focus::checkForValidState()
{
	if (isTempInactive()) return false;
	
	if (is_Valid()) return true;

	//std::cout << "\n - checkForValidState()" << std::endl;
	//std::cout << "   - subj : " << subject->getName() << std::endl;
	//std::cout << "   - obj  : " << object->getName() << std::endl;
	
	SceneObject* prev_subj = subject;
	SceneObject* best_subj = 0;
	float min_err = 999999.0f;

	for (SceneObject* fv : owner_A->get_Face_Vectors())
	{
		if (!fv->isActive() || fv->isTempClone())
			continue;

		subject = fv;
		optix::float2 err = optix::make_float2(99999999.9f);
		bool is_valid = is_Valid(&err);

		//std::cout << "   - fv : " << fv->getName() << std::endl;
		//std::cout << "   - is_valid : " << is_valid << std::endl;
		//std::cout << "   - err.y : " << err.y << std::endl;
		//std::cout << "   - min_err : " << min_err << std::endl;

		if (is_valid && err.x < min_err)
		{
			//std::cout << "     -> best_subj = " << subject->getName() << std::endl;
			min_err   = err.x;
			best_subj = subject;
		}

	}

	if (best_subj != 0)
	{
		subject = best_subj;
		return true;
	}
	else
	{
		subject = prev_subj;
		return false;
	}

}


#endif


#ifdef Restriction_CONVERSATION_R



Restriction_Conversation::~Restriction_Conversation()
{

}
Restriction_Conversation::Restriction_Conversation()
	: Restriction(0, 0, CONVERSATION_R)
{

}

Restriction_Conversation::Restriction_Conversation(SceneObject* subject, SceneObject* object)
	: Restriction(subject, object, CONVERSATION_R)
{

}
Restriction_Conversation::Restriction_Conversation(SceneObject* subject, SceneObject* object, optix::float2 delta)
	: Restriction(subject, object, optix::make_float2(0.0f), delta, CONVERSATION_R)
{

}

bool Restriction_Conversation::is_Valid( optix::float2* error, bool dist_too )
{
	if (isTempInactive()) return false;
	bool is_valid = true;

	optix::float3 p0 = ZERO_3f;
	optix::float3 p1 = p0 + GLOBAL_AXIS[2];
	optix::float4 p0_t = subject->getTransformationMatrix() * optix::make_float4(p0, 1.0f);
	optix::float4 p1_t = subject->getTransformationMatrix() * optix::make_float4(p1, 1.0f);
	optix::float4 subj_dir_t = p1_t - p0_t;
	optix::float3 subj_dir = optix::make_float3(subj_dir_t.x, subj_dir_t.y, subj_dir_t.z); // subject's direction ( faceVectors direction )
	
	p0_t = object->getTransformationMatrix() * optix::make_float4(p0, 1.0f);
	p1_t = object->getTransformationMatrix() * optix::make_float4(p1, 1.0f);
	optix::float4 obj_dir_t = p1_t - p0_t;
	optix::float3 obj_dir = optix::make_float3(obj_dir_t.x, obj_dir_t.y, obj_dir_t.z); // object's direction ( faceVectors direction )
	
	optix::float3 u = object->getTranslationAttributes() - subject->getTranslationAttributes();
	optix::float3 v = -u;
	float du = dot(normalize(subj_dir), normalize(u));
	float theta = acosf(du);
	theta = Geometry::GENERAL::toDegrees(theta);

	float dv = dot(normalize(obj_dir), normalize(v));
	float phi = acosf(dv);
	phi = Geometry::GENERAL::toDegrees(phi);
	////std::cout << "\n - d : [ " << theta << " , " << phi << " ] " << std::endl;
	////std::cout << " - angle_limit : [ " << m_angle_delta.x << " , " << m_angle_delta.y << " ] " << std::endl;
	////std::cout << " - angle_limit_2 : [ " << m_angle_delta_2.x << " , " << m_angle_delta_2.y << " ] " << std::endl;


	bool b1 = theta <= m_angle_delta.x;
	bool b2 = phi <= m_angle_delta_2.x;

	if (error != 0)
	{
		float e1 = (theta - m_angle_delta.x);
		float e2 = (phi - m_angle_delta_2.x);
		e1 = e1 < 0.0f ? 0.0f : e1;
		e2 = e2 < 0.0f ? 0.0f : e2;

		e1 = theta;
		e2 = phi;

		error->x = e1;
		error->y = e2;
	}

	// dist factor
	//float max_dist = m_dist_limit.y;
	//float dist = optix::length(owner_A->getCentroid_Transformed() - owner_B->getCentroid_Transformed());
	bool b3 = dist_too ? is_dist_Valid() : true;
	
	if (b1 && b2
		&& b3 // dist factor //
		)
	{
		return true;
	}
	else
	{
		return false;
	}

}
bool Restriction_Conversation::is_dist_Valid(float * error)
{
	if (isTempInactive()) return false;
	float max_dist = m_dist_limit.y;
	float dist     = optix::length(owner_A->getCentroid_Transformed() - owner_B->getCentroid_Transformed());
	bool b3 = dist <= max_dist;
	if (error != 0)
		*error = fabsf(max_dist - dist);

	return b3;
}
float Restriction_Conversation::get_Error()
{
	return 0.0f;
}
optix::float2 Restriction_Conversation::get_Error_2f()
{

}
bool Restriction_Conversation::checkForValidState()
{
	if (isTempInactive()) return false;

	if (is_Valid()) return true;


	SceneObject* prev_subj = subject;
	SceneObject* prev_obj = object;

	SceneObject* n_subj = 0;
	SceneObject* n_obj  = 0;

	float min_err = 999999.9f;


	for (SceneObject* subj : owner_A->get_Face_Vectors())
	{
		bool is_valid = subj->isActive() && !subj->isTempClone();
		if (!is_valid) continue;

		subject = subj;
		for (SceneObject* obj : owner_B->get_Face_Vectors())
		{
			bool is_valid = obj->isActive() && !obj->isTempClone();
			if (!is_valid) continue;

			object = obj;

			optix::float2 err = optix::make_float2(99999.9f);
			bool valid = is_Valid(&err);
			if (valid && err.y < min_err)
			{
				min_err = err.y;
				n_subj = subject;
				n_obj  = object;
			}

		}
	}

	if (n_subj != 0 && n_obj != 0)
	{
		subject = n_subj;
		object  = n_obj;
		return true;
	}
	else
	{
		subject = prev_subj;
		object  = prev_obj;
		return false;
	}
}

#endif