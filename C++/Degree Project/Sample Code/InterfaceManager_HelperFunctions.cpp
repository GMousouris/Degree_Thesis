#include "InterfaceManager_HelperFunctions.h"
#include "InterfaceManager.h"
#include "Mediator.h"
#include "PinholeCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Transformations.h"
#include "GeometryFunctions.h"
#include <optixu/optixu_matrix_namespace.h>


std::string AxisOrientationToString(AXIS_ORIENTATION orientation)
{
	switch (orientation)
	{
	case WORLD:  return "world";
	case SCREEN: return "screen";
	case VIEW:   return "view";
	case LOCAL:  return "local";
	case PARENT: return "parent";
	case PICK:   return "pick";
	}

	//std::cout << " ~AxisOrientationToString() : returning nullptr!" << std::endl;
	system("pause");
	
}
std::vector<int> getSelectedObjectsFromArea(Rect2D rect, PinholeCamera* camera_instance)
{
	// b.x = 2.0f*((b.x / w)) - 1.0f;
	// b.y = 2.0f*(1 - (b.y / h)) - 1.0f;
	int w = Mediator::RequestWindowSize(0);
	int h = Mediator::RequestWindowSize(1);

	optix::float2 a = optix::make_float2(rect.a);
	optix::float2 b = optix::make_float2(rect.b);

	//float s_min_x, s_min_y, s_max_x, s_max_y;
	//s_min_x = std::fminf(min.x, max.x);
	//s_max_x = std::fmaxf(min.x, max.x);
	//s_min_y = std::fminf(min.y, max.y);
	//s_max_y = std::fmaxf(min.y, max.y);

	// travaw apo de3ia pros ta aristera
	float s_min_x, s_min_y;
	float s_max_x, s_max_y;

	s_min_x = a.x;
	s_min_y = a.y;
	s_max_x = b.x;
	s_max_y = b.y;

	if (a.x > b.x)
	{
		s_min_x = b.x;
		s_max_x = a.x;
		// travaw apo katw pros ta panw 
		if (a.y > b.y)
		{
			s_min_y = b.y;
			s_max_y = a.y;
		}
	}
	
	//min.x = 2.0f*((min.x / w)) - 1.0f;
	//min.y = 2.0f*(1 - (min.y / h)) - 1.0f;
	//max.x = 2.0f*((max.x / w)) - 1.0f;
	//max.y = 2.0f*(1 - (max.y / h)) - 1.0f;

	

	std::vector<SceneObject*>& scene_objects = Mediator::RequestSceneObjects();
	// scene_lights
	// scene_cameras
	// etc..
	optix::float3 cam_pos = camera_instance->getCameraPos();
	optix::float3 camera_up = camera_instance->getV();
	optix::float3 center = camera_instance->getOrbitCenter();

	glm::vec3 pos = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
	glm::vec3 up = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
	glm::vec3 target = glm::vec3(center.x, center.y, center.z);
	glm::mat4 view_matrix = glm::lookAt(pos, target, up);
	glm::mat4 projection_matrix = glm::perspective(glm::radians(M_FOV), w / (float)h, NEAR, FAR);
	glm::mat4 vp = projection_matrix * view_matrix;


	for (SceneObject* obj : scene_objects)
	{
		if ( !obj->isActive() || obj->getType() == APP_OBJECT || obj->getType() == FACE_VECTOR )
			continue;

		glm::vec3 bbox_min     = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getGeomProperties().bbox_min);
		glm::vec3 bbox_max     = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getGeomProperties().bbox_max);
		optix::Matrix4x4 trs   = obj->getTransformationMatrix();

		glm::mat4 mvp = vp * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(trs);
		//mvp = projection_matrix * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(camera_instance->getCameraMatrix()) * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(trs);

		glm::vec4 bbox_min_ = (mvp * glm::vec4(bbox_min, 1.0f));
		glm::vec4 bbox_max_ = (mvp * glm::vec4(bbox_max, 1.0f));

		glm::vec3 ndc_min = glm::vec3(bbox_min_ / bbox_min_.w);
		glm::vec3 ndc_max = glm::vec3(bbox_max_ / bbox_max_.w);


		glm::vec2 screen_min; //= glm::vec2((ndc_min + 1.0f) / 2.0f) * glm::vec2(w, h);
		glm::vec2 screen_max; //= glm::vec2((ndc_max + 1.0f) / 2.0f) * glm::vec2(w, h);
		
		screen_min.x = ((ndc_min.x + 1.0f) / 2.0f) * w;
		screen_min.y = ((1.0f - ndc_min.y) / 2.0f) * h;

		screen_max.x = ((ndc_max.x + 1.0f) / 2.0f) * w;
		screen_max.y = ((1.0f - ndc_max.y) / 2.0f) * h;


		float min_x = std::fminf(screen_min.x, screen_max.x);
		float max_x = std::fmaxf(screen_min.x, screen_max.x);
		float min_y = std::fminf(screen_min.y, screen_max.y);
		float max_y = std::fmaxf(screen_min.y, screen_max.y);

		//std::cout << "\n X : [" << min_x << " , " << max_x << "]" << std::endl;
		//std::cout << " Y : [" << min_y << " , " << max_y << "]" << std::endl;
		if (s_min_x < min_x && s_min_y < min_y)
		{
			if (s_max_x > max_x && s_max_y > max_y)
			{
				//std::cout << "       - - - -OBJ[" << obj->getId() << "]::SELECTED!" << std::endl;
			}
		}
		//glm::vec2 b_min = glm::vec2(bbox_min_ / bbox_min_.z);
		//glm::vec2 b_max = glm::vec2(bbox_max_ / bbox_max_.z);

		//b_min.x = 2.0f*((b_min.x / w)) - 1.0f;
		//b_min.y = 2.0f*(1 - (b_min.y / h)) - 1.0f;

		//b_max.x = 2.0f*((b_max.x / w)) - 1.0f;
		//b_max.y = 2.0f*(1 - (b_max.y / h)) - 1.0f;
		//a.x = 2.0f*((a.x / w)) - 1.0f;
		//a.y = 2.0f*(1 - (a.y / h)) - 1.0f;

		////std::cout << "\n      - bbox_min[" << obj->getId() << "] : ( " << screen_min.x << " , " << screen_min.y << " ) " << std::endl;
		////std::cout << "      - bbox_max[" << obj->getId() << "] : ( " << screen_max.x << " , " << screen_max.y << " ) " << std::endl;

		
	
	}

	std::vector<int> return_value;
	return return_value;
}
std::vector<SceneObject*> getSelectedObjectsFromFrustum(Rect2D rect, PinholeCamera* camera_instance)
{
	
	int w = ViewportManager::GetInstance().getActiveViewport().renderSize.x; //Mediator::RequestWindowSize(0);
	int h = ViewportManager::GetInstance().getActiveViewport().renderSize.y; //Mediator::RequestWindowSize(1);

	optix::float2 a = optix::make_float2(rect.a);
	optix::float2 b = optix::make_float2(rect.b);

	float s_min_x, s_min_y;
	float s_max_x, s_max_y;

	s_min_x = a.x;
	s_min_y = a.y;
	s_max_x = b.x;
	s_max_y = b.y;

	if (a.x > b.x)
	{
		s_min_x = b.x;
		s_max_x = a.x;
		// travaw apo katw pros ta panw 
		if (a.y > b.y)
		{
			s_min_y = b.y;
			s_max_y = a.y;
		}
	}

	glm::vec2 rec_a = glm::vec2(s_min_x, s_min_y);
	glm::vec2 rec_b = glm::vec2(s_max_x, s_min_y);
	glm::vec2 rec_c = glm::vec2(s_max_x, s_max_y);
	glm::vec2 rec_d = glm::vec2(s_min_x, s_max_y);

	Ray ray_a = Geometry::SHAPES::createMouseRay(rec_a);
	Ray ray_b = Geometry::SHAPES::createMouseRay(rec_b);
	Ray ray_c = Geometry::SHAPES::createMouseRay(rec_c);
	Ray ray_d = Geometry::SHAPES::createMouseRay(rec_d);

	Plane plane_a; // ray_a + ray_d
	Plane plane_b; // ray_a + ray_b
	Plane plane_c; // ray_b + ray_c
	Plane plane_d; // ray_c + ray_d

	float t = 10.0f;
	optix::float3 origin = camera_instance->getCameraPos();
	optix::float3 p_a = ray_a.origin + ray_a.direction * t;
	optix::float3 p_b = ray_b.origin + ray_b.direction * t;
	optix::float3 p_c = ray_c.origin + ray_c.direction * t;
	optix::float3 p_d = ray_d.origin + ray_d.direction * t;

	optix::float3 U = normalize(camera_instance->getV());
	optix::float3 a_normal = -optix::cross(ray_a.direction, ray_d.direction);//optix::cross( optix::cross(ray_a.direction, U), ray_a.direction);
	optix::float3 b_normal = optix::cross(ray_a.direction, ray_b.direction); //optix::cross( optix::cross(ray_b.direction, U), ray_b.direction);
	optix::float3 c_normal = optix::cross(ray_b.direction, ray_c.direction);  //optix::cross( optix::cross(ray_c.direction, U), ray_c.direction);
	optix::float3 d_normal = optix::cross(ray_c.direction, ray_d.direction);  //optix::cross( optix::cross(ray_d.direction, U), ray_d.direction);

	optix::float3 normals[4] = { a_normal, b_normal, c_normal, d_normal };
	optix::float3 dir[4]     = { ray_a.direction, ray_b.direction, ray_c.direction, ray_d.direction };

	std::vector<SceneObject*>& scene_objects = Mediator::RequestSceneObjects();
	
	optix::float3 cam_pos   = camera_instance->getCameraPos();
	optix::float3 camera_up = camera_instance->getV();
	optix::float3 center    = camera_instance->getOrbitCenter();

	glm::vec3 pos               = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
	glm::vec3 up                = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
	glm::vec3 target            = glm::vec3(center.x, center.y, center.z);
	glm::mat4 view_matrix       = glm::lookAt(pos, target, up);
	glm::mat4 projection_matrix = glm::perspective(glm::radians(M_FOV), w / (float)h, NEAR, FAR);
	glm::mat4 vp                = projection_matrix * view_matrix;

	std::vector<SceneObject*> selected_objs;
	for (SceneObject* obj : scene_objects)
	{
		if (!obj->isActive() || obj->getId() == -100)
			continue;

		glm::vec3 bbox_min = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getGeomProperties().bbox_min);
		glm::vec3 bbox_max = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getGeomProperties().bbox_max);

		glm::vec3 b_a = glm::vec3(bbox_min.x, bbox_min.y, bbox_min.z);
		glm::vec3 b_b = glm::vec3(bbox_min.x, bbox_min.y, bbox_max.z);
		glm::vec3 b_c = glm::vec3(bbox_min.x, bbox_max.y, bbox_max.z);
		glm::vec3 b_d = glm::vec3(bbox_min.x, bbox_max.y, bbox_min.z);

		glm::vec3 b_e = glm::vec3(bbox_max.x, bbox_max.y, bbox_min.z);
		glm::vec3 b_f = glm::vec3(bbox_max.x, bbox_max.y, bbox_max.z);
		glm::vec3 b_g = glm::vec3(bbox_max.x, bbox_min.y, bbox_max.z);
		glm::vec3 b_h = glm::vec3(bbox_max.x, bbox_min.y, bbox_min.z);

		optix::Matrix4x4 trs = obj->getTransformationMatrix();
		glm::mat4 mvp; //= vp * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(trs);
		mvp = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(trs);
		//mvp = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(camera_instance->getCameraMatrix()) * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(trs);

		glm::vec4 b_min_world = (mvp * glm::vec4(bbox_min, 1.0f));
		glm::vec4 b_max_world = (mvp * glm::vec4(bbox_max, 1.0f));

		glm::vec3 b_a_ = mvp * glm::vec4(b_a, 1.0f);
		glm::vec3 b_b_ = mvp * glm::vec4(b_b, 1.0f);
		glm::vec3 b_c_ = mvp * glm::vec4(b_c, 1.0f);
		glm::vec3 b_d_ = mvp * glm::vec4(b_d, 1.0f);
		glm::vec3 b_e_ = mvp * glm::vec4(b_e, 1.0f);
		glm::vec3 b_f_ = mvp * glm::vec4(b_f, 1.0f);
		glm::vec3 b_g_ = mvp * glm::vec4(b_g, 1.0f);
		glm::vec3 b_h_ = mvp * glm::vec4(b_h, 1.0f);

		optix::float3 b_[8] =
		{
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_a_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_b_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_c_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_d_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_e_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_f_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_g_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_h_)
		};

		optix::float3 bbox_min_world = optix::make_float3(b_min_world.x, b_min_world.y, b_min_world.z);
		optix::float3 bbox_max_world = optix::make_float3(b_max_world.x, b_max_world.y, b_max_world.z);

		optix::float3 O = camera_instance->getCameraPos();
		float t_min = optix::length(bbox_min_world - O);
		float t_max = optix::length(bbox_max_world - O);

		bool in = true;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				float t = length(b_[i] - O);
				optix::float3 p = O + dir[j] * t;

				float dot = optix::dot(normals[j], b_[i] - p);
				if (dot < 0.0f)
				{
					in = false;
				}
			}
		}

		/*
		for (int i = 0; i < 4; i++)
		{
			optix::float3 p_min = O + dir[i] * t_min;
			optix::float3 p_max = O + dir[i] * t_max;

			float dot_min = optix::dot(normals[i], bbox_min_world - p_min );
			float dot_max = optix::dot(normals[i], bbox_max_world - p_max );

			if (dot_min < 0.0f || dot_max < 0.0f)
			{
				in = false;
				//break;
			}
			////std::cout << "p_min : ( " << p_min.x << " , " << p_min.y << " , " << p_min.z << " ) " << std::endl;
			////std::cout << "p_max : ( " << p_max.x << " , " << p_max.y << " , " << p_max.z << " ) " << std::endl;

			////std::cout << "            - dot_min[" << i << "]: " << dot_min << std::endl;
			////std::cout << "            - dot_max[" << i << "]: " << dot_max << std::endl;
			////std::cout << "\n";
		}
		*/

		if (in == true)
		{
			selected_objs.push_back(obj);
		}

	}

	
	////std::cout << "\n - selected : " << std::endl;
	//for (int i = 0; i < selected_objs.size(); i++)
	//{
	//	//std::cout << "     - obj : " << selected_objs[i] << std::endl;
	//}
	
	return selected_objs;
}
std::vector<SceneObject*> getSelectedObjectsFromFrustum(Rect2D rect, const VIEWPORT& viewport)
{
	// local coords
	////std::cout << "\n" << std::endl;
	////std::cout << "  - AREA_SELECTION:" << std::endl;
	////std::cout << "    - rect.a : [ " << rect.a.x << " , " << rect.a.y << " ] " << std::endl;
	////std::cout << "    - rect.b : [ " << rect.b.x << " , " << rect.b.y << " ] " << std::endl;

	int w = viewport.renderSize.x; 
	int h = viewport.renderSize.y; 

	optix::float2 a = optix::make_float2(rect.a);
	optix::float2 b = optix::make_float2(rect.b);

	float s_min_x, s_min_y;
	float s_max_x, s_max_y;

	s_min_x = a.x;
	s_min_y = a.y;
	s_max_x = b.x;
	s_max_y = b.y;

	if (a.x > b.x)
	{
		s_min_x = b.x;
		s_max_x = a.x;
		
		if (a.y > b.y)
		{
			s_min_y = b.y;
			s_max_y = a.y;
		}
	}

	glm::vec2 rec_a = glm::vec2(s_min_x, s_min_y);
	glm::vec2 rec_b = glm::vec2(s_max_x, s_min_y);
	glm::vec2 rec_c = glm::vec2(s_max_x, s_max_y);
	glm::vec2 rec_d = glm::vec2(s_min_x, s_max_y);

	Ray ray_a = Geometry::SHAPES::createMouseRay(rec_a, viewport);
	Ray ray_b = Geometry::SHAPES::createMouseRay(rec_b, viewport);
	Ray ray_c = Geometry::SHAPES::createMouseRay(rec_c, viewport);
	Ray ray_d = Geometry::SHAPES::createMouseRay(rec_d, viewport);

	Plane plane_a; // ray_a + ray_d
	Plane plane_b; // ray_a + ray_b
	Plane plane_c; // ray_b + ray_c
	Plane plane_d; // ray_c + ray_d

	float t = 10.0f;
	PinholeCamera camera_instance = ViewportManager::GetInstance().getViewportCamera( viewport.id );
	optix::float3 origin = camera_instance.getCameraPos();
	optix::float3 p_a = ray_a.origin + ray_a.direction * t;
	optix::float3 p_b = ray_b.origin + ray_b.direction * t;
	optix::float3 p_c = ray_c.origin + ray_c.direction * t;
	optix::float3 p_d = ray_d.origin + ray_d.direction * t;

	optix::float3 U        = normalize( camera_instance.getV() );
	optix::float3 a_normal = -optix::cross(ray_a.direction, ray_d.direction);
	optix::float3 b_normal =  optix::cross(ray_a.direction, ray_b.direction); 
	optix::float3 c_normal =  optix::cross(ray_b.direction, ray_c.direction);  
	optix::float3 d_normal =  optix::cross(ray_c.direction, ray_d.direction); 

	optix::float3 normals[4] = { a_normal, b_normal, c_normal, d_normal };
	optix::float3 dir[4]     = { ray_a.direction, ray_b.direction, ray_c.direction, ray_d.direction };

	std::vector<SceneObject*>& scene_objects = Mediator::RequestSceneObjects();

	optix::float3 cam_pos   = camera_instance.getCameraPos();
	optix::float3 camera_up = camera_instance.getV();
	optix::float3 center    = camera_instance.getOrbitCenter();

	
	std::vector<SceneObject*> selected_objs;
	for (SceneObject* obj : scene_objects)
	{
		if (
			  !obj->isActive() 
			|| obj->getType() == APP_OBJECT
			|| obj->getType() == GROUP_PARENT_DUMMY
			|| obj->getType() == FACE_VECTOR
			|| obj->getType() == SAMPLER_OBJECT
			)
			continue;

		glm::vec3 bbox_min = VECTOR_TYPE_TRANSFORM::TO_GLM_3f( obj->getGeomProperties().bbox_min );
		glm::vec3 bbox_max = VECTOR_TYPE_TRANSFORM::TO_GLM_3f( obj->getGeomProperties().bbox_max );

		glm::vec3 b_a = glm::vec3(bbox_min.x, bbox_min.y, bbox_min.z);
		glm::vec3 b_b = glm::vec3(bbox_min.x, bbox_min.y, bbox_max.z);
		glm::vec3 b_c = glm::vec3(bbox_min.x, bbox_max.y, bbox_max.z);
		glm::vec3 b_d = glm::vec3(bbox_min.x, bbox_max.y, bbox_min.z);

		glm::vec3 b_e = glm::vec3(bbox_max.x, bbox_max.y, bbox_min.z);
		glm::vec3 b_f = glm::vec3(bbox_max.x, bbox_max.y, bbox_max.z);
		glm::vec3 b_g = glm::vec3(bbox_max.x, bbox_min.y, bbox_max.z);
		glm::vec3 b_h = glm::vec3(bbox_max.x, bbox_min.y, bbox_min.z);

		optix::Matrix4x4 trs = obj->getTransformationMatrix();
		glm::mat4 mvp        = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX( trs );
		
		glm::vec4 b_min_world = (mvp * glm::vec4(bbox_min, 1.0f));
		glm::vec4 b_max_world = (mvp * glm::vec4(bbox_max, 1.0f));

		glm::vec3 b_a_ = mvp * glm::vec4(b_a, 1.0f);
		glm::vec3 b_b_ = mvp * glm::vec4(b_b, 1.0f);
		glm::vec3 b_c_ = mvp * glm::vec4(b_c, 1.0f);
		glm::vec3 b_d_ = mvp * glm::vec4(b_d, 1.0f);
		glm::vec3 b_e_ = mvp * glm::vec4(b_e, 1.0f);
		glm::vec3 b_f_ = mvp * glm::vec4(b_f, 1.0f);
		glm::vec3 b_g_ = mvp * glm::vec4(b_g, 1.0f);
		glm::vec3 b_h_ = mvp * glm::vec4(b_h, 1.0f);

		optix::float3 b_[8] =
		{
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_a_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_b_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_c_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_d_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_e_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_f_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_g_) ,
			VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(b_h_)
		};

		optix::float3 bbox_min_world = optix::make_float3(b_min_world.x, b_min_world.y, b_min_world.z);
		optix::float3 bbox_max_world = optix::make_float3(b_max_world.x, b_max_world.y, b_max_world.z);

		optix::float3 O = camera_instance.getCameraPos();
		float t_min = optix::length(bbox_min_world - O);
		float t_max = optix::length(bbox_max_world - O);

		bool in = true;

		////std::cout << "  - OBJ : " << obj->getId() << std::endl;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				float t = length(b_[i] - O);
				optix::float3 p = O + dir[j] * t;

				float dot = optix::dot(normals[j], b_[i] - p);
				if (dot < 0.0f)
				{
					in = false;
				}

				////std::cout << "     - dot[ " << j << " ] = " << (dot >= 0.0f) << std::endl;
			}
		}

		if (in == true)
		{
			selected_objs.push_back(obj);
		}

	}


	////std::cout << "\n - selected : " << std::endl;
	//for (int i = 0; i < selected_objs.size(); i++)
	//{
	//	//std::cout << "     - obj : " << selected_objs[i]->getId() << std::endl;
	//}

	return selected_objs;
}

