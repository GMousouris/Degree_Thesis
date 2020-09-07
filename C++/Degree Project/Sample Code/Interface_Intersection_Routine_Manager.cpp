#pragma once
#include "InterfaceManager.h"
#include "SceneObject.h"
#include "PinholeCamera.h"
#include "GeometryFunctions.h"
#include "Transformations.h"

bool InterfaceManager::get_active_utility_intersectionPoint(
	GLFWwindow * target_window, 
	PinholeCamera* target_camera, 
	Object_Package * target_obj_properties, 
	IntersectionPoint * isec_p,
	UtilityProperties interaction_utility
)
{


	bool is_obj_valid  = m_selected_object_pack.raw_object != nullptr;
	bool is_type_valid = true;
	bool is_create     = m_selected_utility.group == g_CREATE;
	if (is_obj_valid)
	{
		Type obj_type = m_selected_object_pack.raw_object->getType();
		is_type_valid &= (obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT) || (m_selected_utility.group == g_TRANSFORMATIONS);
	}

	bool hit = false;
	if ((is_obj_valid && is_type_valid) 
		|| is_create
		)
	{
		switch (interaction_utility.group)
		{

		case g_TRANSFORMATIONS: hit = get_Utility_Interaction_g_TRANSFORMATIONS(target_window, target_camera, target_obj_properties, isec_p, interaction_utility); break;
		case g_ATTACH:          hit = get_Utility_Interaction_g_ATTACH(target_window, target_camera, target_obj_properties, isec_p, interaction_utility); break;
		case g_CREATE:          hit = get_Utility_Interaction_g_CREATE(target_window, target_camera, target_obj_properties, isec_p, interaction_utility); break;

		}
	}
	//



	// light button utility interaction
	{
		SceneObject* light_obj = 0;
		optix::float3 light_hit_p = ZERO_3f;
		int obj_id = -1;
		bool light_hit = get_Light_toggleButton_Interaction(target_camera, light_hit_p, light_obj, obj_id);
		if (light_hit)
		{
			light_obj = Mediator::RequestSceneObjects()[obj_id];
			float di = hit ? target_camera->getDistanceFrom(isec_p->p) : 99999999.9f;
			float dl = target_camera->getDistanceFrom(light_hit_p);

			if (dl <= di)
			{
				isec_p->is_lightButton_hit = true;
				isec_p->light_obj_id = light_obj->getId();
				light_obj->set_LightButtonHovered(true);
				if (mouse_data.m0_clicked)
					m_m0_down_on_LightButton = light_obj->getId();

				return false;
			}
		}
	}

	return hit;

}


bool InterfaceManager::get_Utility_Interaction_g_TRANSFORMATIONS(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility)
{


#ifdef CANCER_DEBUG
	if (target_obj_properties->raw_object == nullptr || target_obj_properties->object_index < 0)
	{
		SceneObject* obj = target_obj_properties->raw_object;

		//std::cout << "\n InterfaceManager::get_active_utility_IntersectionPoint():" << std::endl;
		//std::cout << " object = " << obj << std::endl;
		//std::cout << " object is null ? = " << (obj == nullptr) << std::endl;
		//std::cout << " object id : " << (obj == nullptr ? -1 : obj->getId()) << std::endl;
		//std::cout << " obj index : " << target_obj_properties->object_index << std::endl;
		
		////std::cout << " raw_object == nullptr!" << std::endl;
		////std::cout << " object_index < 0 " << std::endl;
		system("pause");
	}
#endif


	const VIEWPORT vp = ViewportManager::GetInstance().getActiveViewport();
	SceneObject * m_selected_object = target_obj_properties->raw_object;
	optix::float3 object_position = m_selected_object->getTranslationAttributes();
	optix::float3 c = object_position;
	optix::float3 W = target_camera->getW();
	//Ray ray = Geometry::SHAPES::createMouseRay(); // CreateMouseRay( from_specific_camera )
	Ray ray = Geometry::SHAPES::createMouseRay(vp);

	optix::float3 delta = m_selected_object->getAttributesDelta(0);
	float obj_dist = ViewportManager::GetInstance().getViewportCamera(vp.id).getDistanceFrom(object_position); //m_selected_object->getDistanceFromCamera(); // getDistanceFromCamera( specific camera )
	float radius;
	float m_camera_inv_scale_factor;
	float renderSize_factor = ViewportManager().GetInstance().getActiveViewportCamera().getInvScaleFactor(vp);
	//float renderSize_factor = optix::length(ViewportManager::GetInstance().getActiveViewport().renderSize);

	if (interaction_utility.id == Utility::u_TRANSLATION && (delta.x == 0 && delta.y == 0 && delta.z == 0))
	{
		m_camera_inv_scale_factor = obj_dist * TRANSLATION_AXIS_SCALE_FACTOR * renderSize_factor;
		radius = UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor * 3.6f; //3.3
	}
	else if (interaction_utility.id == Utility::u_ROTATION)
	{
		m_camera_inv_scale_factor = obj_dist * ROTATION_AXIS_SCALE_FACTOR * renderSize_factor;
		radius = UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor;
	}
	else if (interaction_utility.id == Utility::u_SCALE && (delta.x == 0 && delta.y == 0 && delta.z == 0))
	{
		m_camera_inv_scale_factor = obj_dist * TRANSLATION_AXIS_SCALE_FACTOR * renderSize_factor;
		radius = UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor * 5.6f; //3.3
	}
	m_renderer_camera_inv_scale_factor = m_camera_inv_scale_factor;

	// cancel check in case mouse is already dragging ( camera movement ) or object reselection
	if (mouse_data.m0_dragging || m_reselecting 
		//|| m_cloning_finished // EDW //
		)
	{
		return false;
	}

	bool inner_radius_hit = false;
	UtilityPlane m_selected_plane = UtilityPlane::NONE;
	// check if mouse position is in utility area before going in intersection tests
	{
		float dist = optix::length(ray.origin - c);
		optix::float3 mouse_pos = ray.origin + ray.direction * dist;
		float eq = (mouse_pos.x - c.x) * (mouse_pos.x - c.x) + (mouse_pos.y - c.y) * (mouse_pos.y - c.y) + (mouse_pos.z - c.z) * (mouse_pos.z - c.z);
		if (eq > radius * radius + 0.45f * m_camera_inv_scale_factor * m_camera_inv_scale_factor)
		{
			return false;
		}


		// inner circle check
		if (interaction_utility.id == u_ROTATION)
		{
			float rad = UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor * 0.7f;
			if (eq <= rad * rad + 0.35f * m_camera_inv_scale_factor * m_camera_inv_scale_factor)
			{
				inner_radius_hit = true;
			}
			m_interface_state.inner_radius_hit = inner_radius_hit;
		}




	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - -



	// transform Selected object's base axis according to selected axis_orientation
	AXIS_ORIENTATION axis_orientation = ( (m_selected_object->getType() == SAMPLER_OBJECT || m_selected_object->getType() == LIGHT_OBJECT) && interaction_utility.id == u_SCALE) ? LOCAL : m_selected_axis_orientation; // force local axis

	BASE_TRANSFORM::AXIS(m_selected_object, axis_orientation, target_obj_properties->base_axis);
	std::vector<IntersectionPoint> potentialIntersectionPoints;

	// continue with intersection routines based on selected utility [ Translation, Rotation, Scale, etc.. ]
	if (interaction_utility.id == Utility::u_TRANSLATION)
	{
		bool hit = get_translation_utility_intersectionPoint(&ray, target_obj_properties, W, m_camera_inv_scale_factor, potentialIntersectionPoints);
		if (hit)
		{
			////std::cout << " Translation_Hit " << std::endl;
			//return true;
		}
		else
		{
			////std::cout << std::endl;
			return false;
		}
	}
	else if (interaction_utility.id == Utility::u_ROTATION)
	{
		bool hit = get_rotation_utility_intersectionPoint(&ray, target_obj_properties, W, m_camera_inv_scale_factor, potentialIntersectionPoints);
		if (hit)
		{
			////std::cout << " Rotation_Hit : " << std::endl;
			//return true;
		}
		else
		{
			////std::cout << std::endl;
			if (inner_radius_hit)
			{
				////std::cout << " renderSize_factor : " << renderSize_factor << std::endl;
				float r = 100.0f * 0.6 * m_renderer_camera_inv_scale_factor * m_renderer_camera_inv_scale_factor / renderSize_factor;

				optix::float3 p;
				Geometry::RAY::Intersect_Sphere(ray, c, r, p);
				isec_p->p = p;
				isec_p->center = c;
				isec_p->normal = optix::normalize(p - c);
				isec_p->plane_id = UtilityPlane::XYZ;
				isec_p->inner_radius_hit = true;
				return true;

			}
			return false;
		}
	}
	else if (interaction_utility.id == Utility::u_SCALE)
	{
		bool hit = get_scale_utility_intersectionPoint(&ray, target_obj_properties, W, m_camera_inv_scale_factor, potentialIntersectionPoints);
		if (hit)
		{
			////std::cout << " Rotation_Hit : " << std::endl;
			//return true;
		}
		else
		{
			////std::cout << std::endl;
			return false;
		}
	}


	// getting the closest intersectionPoint from all potential intersections
	if (potentialIntersectionPoints.size() > 0)
	{
		optix::float3 cam_pos = Mediator::RequestCameraInstance().getCameraPos();

		float z_min = optix::length(potentialIntersectionPoints[0].p - cam_pos);
		int index = 0;
		for (int i = 1; i < potentialIntersectionPoints.size(); i++)
		{
			float z = optix::length(potentialIntersectionPoints[i].p - cam_pos);
			if (interaction_utility.id == u_TRANSLATION) // if utility = Translation && potentialIntersectionPoints > 0 -> we choose the closest plane intersected
			{
				if (z > z_min)
				{
					z_min = z;
					index = i;
				}
			}
			else if (z < z_min)
			{
				z_min = z;
				index = i;
			}
		}


		// pass data about the intersection point
		isec_p->p = potentialIntersectionPoints[index].p;
		isec_p->plane_id = potentialIntersectionPoints[index].plane_id;
		isec_p->intersection_plane = potentialIntersectionPoints[index].intersection_plane;
		isec_p->normal = potentialIntersectionPoints[index].normal;
		// if utility.id == ROTATION : pass extra information about the intersection

		if (interaction_utility.id == Utility::u_ROTATION)
		{

			isec_p->normal = potentialIntersectionPoints[index].normal;
			isec_p->plane_id = potentialIntersectionPoints[index].plane_id;
			isec_p->tangent = Geometry::VECTOR_SPACE::getCircleTangentOnPlane(*isec_p, target_obj_properties->base_axis, isec_p->plane_id);
			isec_p->out_normal = optix::cross(isec_p->normal, isec_p->tangent);
			isec_p->center = c;
			m_interface_state.inner_radius_hit = false;
			//isec_p->inner_radius_hit = inner_radius_hit;
			//m_focused_arc_point = true;

		}


		potentialIntersectionPoints.clear();
		return true;
	}

	return false;
}
bool InterfaceManager::get_translation_utility_intersectionPoint(Ray * ray, Object_Package * obj_prop, optix::float3 &cam_w, float m_camera_inv_scale_factor, std::vector<IntersectionPoint>& potentialIntersectionPoints)
{
	
	optix::float3 * M_WORKING_AXIS;// = obj_prop->base_axis;
	M_WORKING_AXIS = obj_prop->base_axis;

	optix::float3 plane_n;
	optix::float3 projected_point;
	optix::float3 c = obj_prop->raw_object->getTranslationAttributes();
	float x, y, z;

	float d_u = optix::dot(cam_w, M_WORKING_AXIS[0]);
	float d_y = optix::dot(cam_w, M_WORKING_AXIS[1]);
	float d_z = optix::dot(cam_w, M_WORKING_AXIS[2]);

	float e_dist = 2.9f * m_camera_inv_scale_factor; //2.76
	float e = 0.1f * m_camera_inv_scale_factor;
	float rayDotLine_e = 0.003f;

	/* XYZ - plane */
	{
		
		plane_n = Geometry::VECTOR_SPACE::getPerpendicularBasePlane(ray->direction);
		Geometry::RAY::Intersect_Plane(*ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);
		x = projected_point.x;
		y = projected_point.y;
		z = projected_point.z;
		

		float ee = 0.16f * m_camera_inv_scale_factor;
		bool hit = false;
		if (plane_n.x == 1)
		{
			if (fabs(z - c.z) < ee && fabs(y - c.y) < ee)
				hit = true;
		}
		else if (plane_n.y == 1)
		{
			if (fabs(x - c.x) < ee && fabs(z - c.z) < ee)
				hit = true;
		}
		else if (plane_n.z == 1)
		{

			if (fabs(x - c.x) < ee && fabs(y - c.y) < ee)
				hit = true;
		}

		if (hit)
		{
			IntersectionPoint iP;
			Geometry::RAY::Intersect_Plane(*ray, Geometry::SHAPES::createPlane(c, -Mediator::RequestCameraInstance().getW()), projected_point);
			iP.p = projected_point;
			iP.plane_id = UtilityPlane::XYZ;
			iP.intersection_plane = -Mediator::RequestCameraInstance().getW();
			potentialIntersectionPoints.push_back(iP);
			return true;
		}


	}

	/* X - axis */
	{
		if (fabs(d_y) < fabs(d_z))
			plane_n = M_WORKING_AXIS[2];
		else
			plane_n = M_WORKING_AXIS[1];

		Geometry::RAY::Intersect_Plane(*ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);

		float length = optix::length(projected_point - c);
		optix::float3 _dir = optix::normalize(projected_point - c);
		float _dot = optix::dot(_dir, M_WORKING_AXIS[0]);

		if (1.0f - _dot < rayDotLine_e && length < e_dist)
		{
			IntersectionPoint iP;
			iP.p = projected_point;
			iP.plane_id = UtilityPlane::X;
			iP.intersection_plane = plane_n;
			potentialIntersectionPoints.push_back(iP);
			return true;
		}
	}

	/* Y - axis */
	{
		if (fabs(d_u) < fabs(d_z))
			plane_n = M_WORKING_AXIS[2];
		else
			plane_n = M_WORKING_AXIS[0];

		Geometry::RAY::Intersect_Plane(*ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);

		float length = optix::length(projected_point - c);
		optix::float3 _dir = optix::normalize(projected_point - c);
		float _dot = optix::dot(_dir, M_WORKING_AXIS[1]);
		////std::cout << "\n length : " << length << std::endl;
		////std::cout << " dot : " << _dot << std::endl;
		if (1.0f - _dot < rayDotLine_e && length < e_dist)
		{
			IntersectionPoint iP;
			iP.p = projected_point;
			iP.plane_id = UtilityPlane::Y;
			iP.intersection_plane = plane_n;
			potentialIntersectionPoints.push_back(iP);
			return true;
		}
	}

	/* Z - axis */
	{
		if (fabs(d_u) < fabs(d_y))
			plane_n = M_WORKING_AXIS[1];
		else
			plane_n = M_WORKING_AXIS[0];

		Geometry::RAY::Intersect_Plane(*ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);

		float length = optix::length(projected_point - c);
		optix::float3 _dir = optix::normalize(projected_point - c);
		float _dot = optix::dot(_dir, M_WORKING_AXIS[2]);
		////std::cout << "\n length : " << length << std::endl;
		////std::cout << " dot : " << _dot << std::endl;
		if (1.0f - _dot < rayDotLine_e && length < e_dist)
		{
			IntersectionPoint iP;
			iP.p = projected_point;
			iP.plane_id = UtilityPlane::Z;
			iP.intersection_plane = plane_n;
			potentialIntersectionPoints.push_back(iP);
			return true;
		}
	}

	/* Planes */
	{
		float scale = 1.2 * UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor;//UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor;
		optix::float3 isec_point;

		Plane plane_xy = Geometry::SHAPES::createPlane(c, M_WORKING_AXIS[0] * scale, M_WORKING_AXIS[1] * scale);
		Plane plane_xz = Geometry::SHAPES::createPlane(c, M_WORKING_AXIS[0] * scale, M_WORKING_AXIS[2] * scale);
		Plane plane_yz = Geometry::SHAPES::createPlane(c, M_WORKING_AXIS[1] * scale, M_WORKING_AXIS[2] * scale);

		bool hit = false;
		if (Geometry::RAY::Intersect_Parallelogram(*ray, plane_xz, isec_point) == 1)
		{
			hit = true;
			IntersectionPoint iP;
			iP.p = isec_point;
			iP.plane_id = UtilityPlane::XZ;
			iP.intersection_plane = M_WORKING_AXIS[1];
			potentialIntersectionPoints.push_back(iP);
			
		}
		if (Geometry::RAY::Intersect_Parallelogram(*ray, plane_xy, isec_point) == 1)
		{
			hit = true;
			IntersectionPoint iP;
			iP.p = isec_point;
			iP.plane_id = UtilityPlane::XY;
			iP.intersection_plane = M_WORKING_AXIS[2];
			potentialIntersectionPoints.push_back(iP);
		}
		if (Geometry::RAY::Intersect_Parallelogram(*ray, plane_yz, isec_point) == 1)
		{
			hit = true;
			IntersectionPoint iP;
			iP.p = isec_point;
			iP.plane_id = UtilityPlane::YZ;
			iP.intersection_plane = M_WORKING_AXIS[0];
			potentialIntersectionPoints.push_back(iP);
		}

		if (hit)
			return true;
	}

	return false;
}
bool InterfaceManager::get_rotation_utility_intersectionPoint(Ray * ray, Object_Package * obj_prop, optix::float3 &cam_w, float m_camera_inv_scale_factor, std::vector<IntersectionPoint>& potentialIntersectionPoints)
{
	optix::float3 c = obj_prop->raw_object->getTranslationAttributes();
	optix::float3 * M_WORKING_AXIS = obj_prop->base_axis;
	float e_r = 1.1 * 0.6 * m_camera_inv_scale_factor * m_camera_inv_scale_factor;
	float real_r = 1.0f * 0.6 * m_camera_inv_scale_factor;

	Geometry::RAY::Intersect_SphereArcs(*ray, c, e_r, real_r, &M_WORKING_AXIS[0], m_selected_axis_orientation, potentialIntersectionPoints);
	if (potentialIntersectionPoints.size() > 0)
		return true;

	return false;
		
}
bool InterfaceManager::get_scale_utility_intersectionPoint(Ray * ray, Object_Package * obj_prop, optix::float3 &cam_w, float m_camera_inv_scale_factor, std::vector<IntersectionPoint>& potentialIntersectionPoints)
{
	optix::float3 * M_WORKING_AXIS = obj_prop->base_axis;
	optix::float3 plane_n;
	optix::float3 projected_point;
	optix::float3 c = obj_prop->raw_object->getTranslationAttributes();
	float d_u = optix::dot(cam_w, M_WORKING_AXIS[0]);
	float d_y = optix::dot(cam_w, M_WORKING_AXIS[1]);
	float d_z = optix::dot(cam_w, M_WORKING_AXIS[2]);
	float e_dist = 4.4f * m_camera_inv_scale_factor; // axis scale_factor
	float e_dist2 = 1.2f * m_camera_inv_scale_factor; // plane - triangles scale_factor
	float e = 0.1f * m_camera_inv_scale_factor;
	float rayDotLine_e = 0.003f;

	bool hit = false;

	{

		float scale = 0.9f * m_camera_inv_scale_factor * UTILITY_SCALE_FACTOR;
		float scale_factor1 = 2.35f;
		float scale_factor2 = 1.25f;
		optix::float3 p1, p2, p3,p11, p22;

		optix::float3 ip1, ip2, ip3, ip4, p;
		Triangle xy1, xy2, xz1, xz2, yz1, yz2, xyz_tri;
	

		p1 = M_WORKING_AXIS[0] * scale_factor1 * scale; p11 = p1 + M_WORKING_AXIS[0] * scale_factor2 * scale;
		p2 = M_WORKING_AXIS[1] * scale_factor1 * scale; p22 = p2 + M_WORKING_AXIS[1] * scale_factor2 * scale;
		xy1 = Geometry::SHAPES::createTriangle(c, p1, p2);
		xy2 = Geometry::SHAPES::createTriangle(c, p11, p22);
		

		p1 = M_WORKING_AXIS[0] * scale_factor1 * scale; p11 = p1 + M_WORKING_AXIS[0] * scale_factor2 * scale;
		p2 = M_WORKING_AXIS[2] * scale_factor1 * scale; p22 = p2 + M_WORKING_AXIS[2] * scale_factor2 * scale;
		xz1 = Geometry::SHAPES::createTriangle(c, p1, p2);
		xz2 = Geometry::SHAPES::createTriangle(c, p11, p22);

		p1 = M_WORKING_AXIS[1] * scale_factor1 * scale; p11 = p1 + M_WORKING_AXIS[1] * scale_factor2 * scale;
		p2 = M_WORKING_AXIS[2] * scale_factor1 * scale; p22 = p2 + M_WORKING_AXIS[2] * scale_factor2 * scale;
		yz1 = Geometry::SHAPES::createTriangle(c, p1, p2);
		yz2 = Geometry::SHAPES::createTriangle(c, p11, p22);

		p1 = M_WORKING_AXIS[0] * scale_factor1 * scale;
		p2 = M_WORKING_AXIS[1] * scale_factor1 * scale;
		p3 = M_WORKING_AXIS[2] * scale_factor1 * scale;
		xyz_tri = Geometry::SHAPES::createTriangle(c + p1, p2 - p1, p3 - p1 );

		int xy = !Geometry::RAY::Intersect_Triangle_(*ray, xy1, ip1) && Geometry::RAY::Intersect_Triangle_(*ray, xy2, ip1);
		int xz = !Geometry::RAY::Intersect_Triangle_(*ray, xz1, ip2) && Geometry::RAY::Intersect_Triangle_(*ray, xz2, ip2);
		int yz = !Geometry::RAY::Intersect_Triangle_(*ray, yz1, ip3) && Geometry::RAY::Intersect_Triangle_(*ray, yz2, ip3);
		int xyz = Geometry::RAY::Intersect_Triangle_(*ray, xyz_tri, ip4);

		if (xy)
		{
			p = ip1;
			IntersectionPoint iP;
			iP.p = p;
			iP.plane_id = UtilityPlane::XY;
			iP.intersection_plane = M_WORKING_AXIS[2];
			potentialIntersectionPoints.push_back(iP);
			hit = true;
		}
		if (xz)
		{
			p = ip2;
			IntersectionPoint iP;
			iP.p = p;
			iP.plane_id = UtilityPlane::XZ;
			iP.intersection_plane = M_WORKING_AXIS[1];
			potentialIntersectionPoints.push_back(iP);
			hit = true;
		}
		if (yz)
		{
			p = ip3;
			IntersectionPoint iP;
			iP.p = p;
			iP.plane_id = UtilityPlane::YZ;
			iP.intersection_plane = M_WORKING_AXIS[0];
			potentialIntersectionPoints.push_back(iP);
			hit = true;
		}
		if (xyz)
		{
			p = ip4;
			IntersectionPoint iP;
			optix::float3 w = -Mediator::RequestCameraInstance().getW();
			Geometry::RAY::Intersect_Plane(*ray, Geometry::SHAPES::createPlane(c, w), iP.p);
			//iP.p = p;
			iP.plane_id = UtilityPlane::XYZ;
			iP.intersection_plane = w;
			iP.normal = xyz_tri.normal;
			potentialIntersectionPoints.push_back(iP);
			hit = true;
		}

		if (hit)
			return true;
	}

	/*
	// Intersect Planes
	{
		Triangle XY = Geometry::SHAPES::createTriangle(c, M_WORKING_AXIS[0] * e_dist2, M_WORKING_AXIS[1] * e_dist2);
		Triangle XZ = Geometry::SHAPES::createTriangle(c, M_WORKING_AXIS[0] * e_dist2, M_WORKING_AXIS[2] * e_dist2);
		Triangle YZ = Geometry::SHAPES::createTriangle(c, M_WORKING_AXIS[1] * e_dist2, M_WORKING_AXIS[2] * e_dist2);
		Triangle XYZ_1 = Geometry::SHAPES::createTriangle(c,  GLOBAL_AXIS[0] * e_dist2 * 0.3f, GLOBAL_AXIS[1] * e_dist2 * 0.7f);
		Triangle XYZ_2 = Geometry::SHAPES::createTriangle(c, -GLOBAL_AXIS[0] * e_dist2 * 0.3f, GLOBAL_AXIS[1] * e_dist2 * 0.7f);

		optix::float3 ip1, ip2, ip3, ip4, ip5;
		int xz = Geometry::RAY::Intersect_Triangle_(*ray, XZ, ip2);
		int xy = Geometry::RAY::Intersect_Triangle_(*ray, XY, ip1);
		int yz = Geometry::RAY::Intersect_Triangle_(*ray, YZ, ip3);
		int xyz_1 = Geometry::RAY::Intersect_Triangle_(*ray, XYZ_1, ip4);
		int xyz_2 = Geometry::RAY::Intersect_Triangle_(*ray, XYZ_1, ip5);
		
		if (xy)
		{
			IntersectionPoint iP;
			iP.p = ip1;
			iP.plane_id = UtilityPlane::XY;
			iP.intersection_plane = M_WORKING_AXIS[2];
			potentialIntersectionPoints.push_back(iP);
			hit = true;
		}
		if (xz)
		{
			IntersectionPoint iP;
			iP.p = ip2;
			iP.plane_id = UtilityPlane::XZ;
			iP.intersection_plane = M_WORKING_AXIS[1];
			potentialIntersectionPoints.push_back(iP);
			hit = true;
		}
		if (yz)
		{
			IntersectionPoint iP;
			iP.p = ip3;
			iP.plane_id = UtilityPlane::YZ;
			iP.intersection_plane = M_WORKING_AXIS[0];
			potentialIntersectionPoints.push_back(iP);
			hit = true;
		}
		if (xyz_1)
		{
			IntersectionPoint iP;
			iP.p = ip4;
			iP.plane_id = UtilityPlane::XYZ;
			iP.intersection_plane = -Mediator::RequestCameraInstance().getW();
			potentialIntersectionPoints.push_back(iP);
			//hit = true;
		}
		if (xyz_2)
		{
			IntersectionPoint iP;
			iP.p = ip5;
			iP.plane_id = UtilityPlane::XYZ;
			iP.intersection_plane = -Mediator::RequestCameraInstance().getW();
			potentialIntersectionPoints.push_back(iP);
			//hit = true;
		}
	}
	*/

	/* X - axis */
	{
		if (fabs(d_y) < fabs(d_z))
			plane_n = M_WORKING_AXIS[2];
		else
			plane_n = M_WORKING_AXIS[1];

		Geometry::RAY::Intersect_Plane(*ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);

		float length = optix::length(projected_point - c);
		optix::float3 _dir = optix::normalize(projected_point - c);
		float _dot = optix::dot(_dir, M_WORKING_AXIS[0]);

		if (1.0f - _dot < rayDotLine_e && length < e_dist)
		{
			IntersectionPoint iP;
			iP.p = projected_point;
			iP.plane_id = UtilityPlane::X;
			iP.intersection_plane = plane_n;
			potentialIntersectionPoints.push_back(iP);
			return true;
		}
	}

	/* Y - axis */
	{
		if (fabs(d_u) < fabs(d_z))
			plane_n = M_WORKING_AXIS[2];
		else
			plane_n = M_WORKING_AXIS[0];

		Geometry::RAY::Intersect_Plane(*ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);

		float length = optix::length(projected_point - c);
		optix::float3 _dir = optix::normalize(projected_point - c);
		float _dot = optix::dot(_dir, M_WORKING_AXIS[1]);
		////std::cout << "\n length : " << length << std::endl;
		////std::cout << " dot : " << _dot << std::endl;
		if (1.0f - _dot < rayDotLine_e && length < e_dist)
		{
			IntersectionPoint iP;
			iP.p = projected_point;
			iP.plane_id = UtilityPlane::Y;
			iP.intersection_plane = plane_n;
			potentialIntersectionPoints.push_back(iP);
			return true;
		}
	}

	/* Z - axis */
	{
		if (fabs(d_u) < fabs(d_y))
			plane_n = M_WORKING_AXIS[1];
		else
			plane_n = M_WORKING_AXIS[0];

		Geometry::RAY::Intersect_Plane(*ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);

		float length = optix::length(projected_point - c);
		optix::float3 _dir = optix::normalize(projected_point - c);
		float _dot = optix::dot(_dir, M_WORKING_AXIS[2]);
		////std::cout << "\n length : " << length << std::endl;
		////std::cout << " dot : " << _dot << std::endl;
		if (1.0f - _dot < rayDotLine_e && length < e_dist)
		{
			IntersectionPoint iP;
			iP.p = projected_point;
			iP.plane_id = UtilityPlane::Z;
			iP.intersection_plane = plane_n;
			potentialIntersectionPoints.push_back(iP);
			return true;
		}
	}

	

	return hit;
}



bool InterfaceManager::get_Utility_Interaction_g_ATTACH(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties,
														IntersectionPoint * isec_p, UtilityProperties interaction_utility)
{
	
	Utility ut = interaction_utility.id;
	bool is_attach_sampler = ut == u_ATTACH_SAMPLER_DIRECTIONAL || ut == u_ATTACH_SAMPLER_PLANAR || ut == u_ATTACH_SAMPLER_VOLUME;
	return is_attach_sampler ? get_Utility_Interaction_g_ATTACH_SAMPLER(target_window, target_camera, target_obj_properties, isec_p, interaction_utility)
		: get_Utility_Interaction_g_ATTACH_FACE_VECTOR(target_window, target_camera, target_obj_properties, isec_p, interaction_utility);
}
bool InterfaceManager::get_Utility_Interaction_g_ATTACH_FACE_VECTOR(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility)
{
	SceneObject* obj = m_selected_object_pack.raw_object;
	if (obj == nullptr)
		return false;

	Utility ut_id = interaction_utility.id;
	bool is_group_n_Normal_aligned = (ut_id == u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED && m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY);
	if (ut_id == u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED && !is_group_n_Normal_aligned)
		return false;



	//
	VIEWPORT vp = ViewportManager::GetInstance().getActiveViewport();
	float obj_dist = target_camera->getDistanceFrom(obj->getTranslationAttributes());
	float renderSize_factor = target_camera->getInvScaleFactor(vp);
	float m_camera_inv_scale_factor = obj_dist * LINKING_SCALE_FACTOR * renderSize_factor;
	float scale_factor = optix::length(obj->getScaleAttributes());
	optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
	optix::float3 centroid = obj->getCentroid();
	if (obj->getType() != GROUP_PARENT_DUMMY)
		centroid = optix::make_float3(def_transformation * optix::make_float4(centroid, 1.0f));

	optix::float3 bbox_min = obj->getBBoxMin();
	optix::float3 bbox_max = obj->getBBoxMax();
	float xm = bbox_min.x;
	float ym = bbox_min.y;
	float zm = bbox_min.z;
	float xM = bbox_max.x;
	float yM = bbox_max.y;
	float zM = bbox_max.z;

	optix::float3 p[8];
	p[0] = bbox_min;
	p[1] = optix::make_float3(xm, yM, zm);
	p[2] = optix::make_float3(xM, yM, zm);
	p[3] = optix::make_float3(xM, ym, zm);

	p[4] = bbox_max;
	p[5] = optix::make_float3(xm, yM, zM);
	p[6] = optix::make_float3(xm, ym, zM);
	p[7] = optix::make_float3(xM, ym, zM);

	if (obj->getType() != GROUP_PARENT_DUMMY)
	{
		for (int i = 0; i < 8; i++)
			p[i] = optix::make_float3(def_transformation * optix::make_float4(p[i], 1.0f));
	}
	// Define the [6] faces //
	Plane xy, xy_, yz, yz_, xz, xz_;

	optix::float3 r_normals[6];

	float const_dist = 100.0f;
	Plane planes[6];
	Plane r_planes[6];
	{
		// xy           // xy_
		xy.p0 = p[0];   xy_.p0 = p[6];
		xy.p1 = p[3];   xy_.p1 = p[7];
		xy.p2 = p[2];   xy_.p2 = p[4];
		xy.p3 = p[1];   xy_.p3 = p[5];

		xy.u = (xy.p1 - xy.p0);
		xy.v = (xy.p3 - xy.p0);
		xy.normal = ((xy.p0 + xy.p2) * 0.5f - centroid);
		planes[0] = xy;
		r_normals[0] = -normalize(cross(planes[0].u, planes[0].v));


		xy_.u = (xy_.p1 - xy_.p0);
		xy_.v = (xy_.p3 - xy_.p0);
		xy_.normal = ((xy_.p0 + xy_.p2) * 0.5f - centroid);
		planes[1] = xy_;
		r_normals[1] = normalize(cross(planes[1].u, planes[1].v));


		// yz           // yz_
		yz.p0 = p[3];   yz_.p0 = p[0];
		yz.p1 = p[7];   yz_.p1 = p[6];
		yz.p2 = p[4];   yz_.p2 = p[5];
		yz.p3 = p[2];   yz_.p3 = p[1];

		yz.u = (yz.p1 - yz.p0);
		yz.v = (yz.p3 - yz.p0);
		yz.normal = ((yz.p0 + yz.p2) * 0.5f - centroid);
		planes[2] = yz;
		r_normals[2] = -normalize(cross(planes[2].u, planes[2].v));

		yz_.u = (yz_.p1 - yz_.p0);
		yz_.v = (yz_.p3 - yz_.p0);
		yz_.normal = ((yz_.p0 + yz_.p2) * 0.5f - centroid);
		planes[3] = yz_;
		r_normals[3] = normalize(cross(planes[3].u, planes[3].v));


		// xz           // xz_
		xz.p0 = p[0];   xz_.p0 = p[1];
		xz.p1 = p[6];   xz_.p1 = p[5];
		xz.p2 = p[7];   xz_.p2 = p[4];
		xz.p3 = p[3];   xz_.p3 = p[2];

		xz.u = (xz.p1 - xz.p0);
		xz.v = (xz.p3 - xz.p0);
		xz.normal = ((xz.p0 + xz.p2) * 0.5f - centroid);
		planes[4] = xz;
		r_normals[4] = -normalize(cross(planes[4].u, planes[4].v));


		xz_.u = (xz_.p1 - xz_.p0);
		xz_.v = (xz_.p3 - xz_.p0);
		xz_.normal = ((xz_.p0 + xz_.p2) * 0.5f - centroid);
		planes[5] = xz_;
		r_normals[5] = normalize(cross(planes[5].u, planes[5].v));
	}

	int i_min = 0;
	int i_max = 0;
	float d_min = length(planes[0].normal);
	float d_max = d_min;

	for (int i = 0; i < 6; i++)
	{
		planes[i].centroid = (planes[i].p0 + planes[i].p2) * 0.5f;

		Ray to_plane;
		to_plane.origin = planes[i].centroid;
		to_plane.direction = r_normals[i];
		Geometry::RAY::Intersect_Plane
		(to_plane,
		 Geometry::SHAPES::createPlane(planes[i].p0 + planes[i].normal, r_normals[i]), planes[i].p_normal);

		float d = length(planes[i].normal);
		if (d < d_min)
		{
			d_min = d;
			i_min = i;
		}
		if (d > d_max)
		{
			d_max = d;
			i_max = i;
		}

	}


	float inv_scale_factor = m_camera_inv_scale_factor;

	if (inv_scale_factor < 8.0f)
		inv_scale_factor = 8.0f;

	float dist = 2.5f * inv_scale_factor;
	float size = 1.4f * inv_scale_factor;
	dist = 0.0f;
	size = 0.0f;
	const float offset = is_group_n_Normal_aligned ? 0.1f : (d_max + d_min) * 0.5f;
	optix::float2 scale = optix::make_float2(offset * 0.1f);


	// Ray 
	Ray ray = Geometry::SHAPES::createMouseRay();
	bool hitted[6];
	optix::float3 p_isec_p[6];
	optix::float3 t_isec_p[4];
	int hit_i = -1;
	for (int i = 0; i < 6; i++)
	{


		optix::float3 p0 = planes[i].centroid + r_normals[i] * offset;
		optix::float3 du = normalize(planes[i].u) * scale.x;
		optix::float3 dv = normalize(planes[i].v) * scale.y;
		optix::float3 u = planes[i].u * 0.5f + du;
		optix::float3 v = planes[i].v * 0.5f + dv;

		Plane p1 = Geometry::SHAPES::createPlane(p0, u, v);
		Plane p2 = Geometry::SHAPES::createPlane(p0, -u, -v);
		Plane p3 = Geometry::SHAPES::createPlane(p0, u, -v);
		Plane p4 = Geometry::SHAPES::createPlane(p0, -u, v);

		hitted[i] = false;
		int t_hit_i[4] = { false, false, false, false };

		t_hit_i[0] = Geometry::RAY::Intersect_Parallelogram(ray, p1, t_isec_p[0]);
		t_hit_i[1] = Geometry::RAY::Intersect_Parallelogram(ray, p2, t_isec_p[1]);
		t_hit_i[2] = Geometry::RAY::Intersect_Parallelogram(ray, p3, t_isec_p[2]);
		t_hit_i[3] = Geometry::RAY::Intersect_Parallelogram(ray, p4, t_isec_p[3]);

		hitted[i] = t_hit_i[0] || t_hit_i[1] || t_hit_i[2] || t_hit_i[3];
		for (int j = 0; j < 4; j++)
			if (t_hit_i[j] == 1)
				p_isec_p[i] = t_isec_p[j];

	}

	float nearest = 9999999999.0f;
	int nearest_hit = -1;
	for (int i = 0; i < 6; i++)
	{
		if (!hitted[i])
			continue;

		float d = target_camera->getDistanceFrom(p_isec_p[i]);
		if (d < nearest)
		{
			nearest = d;
			nearest_hit = i;
		}
	}



	isec_p->hit_index = nearest_hit;
	isec_p->p = p_isec_p[nearest_hit];
	return false;
	//return nearest_hit >= 0 ? true : false;
}
bool InterfaceManager::get_Utility_Interaction_g_ATTACH_SAMPLER(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility)
{

	SceneObject* object = m_selected_object_pack.raw_object;
	bool use_bbox = m_sampler_init_isec_object_bbox;
	Type obj_type = object->getType();
	bool is_group = obj_type == GROUP_PARENT_DUMMY;
	bool return_value = false;

	

	if ( use_bbox  )
	{
		SceneObject* obj = object;
		VIEWPORT vp = ViewportManager::GetInstance().getActiveViewport();
		float obj_dist = target_camera->getDistanceFrom(obj->getTranslationAttributes());
		float renderSize_factor = target_camera->getInvScaleFactor(vp);
		float m_camera_inv_scale_factor = obj_dist * LINKING_SCALE_FACTOR * renderSize_factor;
		float scale_factor = optix::length(obj->getScaleAttributes());
		optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
		optix::float3 centroid = obj->getCentroid();
		if ( !is_group )
			centroid = optix::make_float3(def_transformation * optix::make_float4(centroid, 1.0f));

		optix::float3 bbox_min = obj->getBBoxMin();
		optix::float3 bbox_max = obj->getBBoxMax();
		float xm = bbox_min.x;
		float ym = bbox_min.y;
		float zm = bbox_min.z;
		float xM = bbox_max.x;
		float yM = bbox_max.y;
		float zM = bbox_max.z;

		optix::float3 p[8];
		p[0] = bbox_min;
		p[1] = optix::make_float3(xm, yM, zm);
		p[2] = optix::make_float3(xM, yM, zm);
		p[3] = optix::make_float3(xM, ym, zm);

		p[4] = bbox_max;
		p[5] = optix::make_float3(xm, yM, zM);
		p[6] = optix::make_float3(xm, ym, zM);
		p[7] = optix::make_float3(xM, ym, zM);

		if ( !is_group )
		{
			for (int i = 0; i < 8; i++)
				p[i] = optix::make_float3(def_transformation * optix::make_float4(p[i], 1.0f));
		}
		// Define the [6] faces //
		Plane xy, xy_, yz, yz_, xz, xz_;

		optix::float3 r_normals[6];

		float const_dist = 100.0f;
		Plane planes[6];
		Plane r_planes[6];
		{
			// xy           // xy_
			xy.p0 = p[0];   xy_.p0 = p[6];
			xy.p1 = p[3];   xy_.p1 = p[7];
			xy.p2 = p[2];   xy_.p2 = p[4];
			xy.p3 = p[1];   xy_.p3 = p[5];

			xy.u = (xy.p1 - xy.p0);
			xy.v = (xy.p3 - xy.p0);
			xy.normal = ((xy.p0 + xy.p2) * 0.5f - centroid);
			planes[0] = xy;
			r_normals[0] = -normalize(cross(planes[0].u, planes[0].v));


			xy_.u = (xy_.p1 - xy_.p0);
			xy_.v = (xy_.p3 - xy_.p0);
			xy_.normal = ((xy_.p0 + xy_.p2) * 0.5f - centroid);
			planes[1] = xy_;
			r_normals[1] = normalize(cross(planes[1].u, planes[1].v));


			// yz           // yz_
			yz.p0 = p[3];   yz_.p0 = p[0];
			yz.p1 = p[7];   yz_.p1 = p[6];
			yz.p2 = p[4];   yz_.p2 = p[5];
			yz.p3 = p[2];   yz_.p3 = p[1];

			yz.u = (yz.p1 - yz.p0);
			yz.v = (yz.p3 - yz.p0);
			yz.normal = ((yz.p0 + yz.p2) * 0.5f - centroid);
			planes[2] = yz;
			r_normals[2] = -normalize(cross(planes[2].u, planes[2].v));

			yz_.u = (yz_.p1 - yz_.p0);
			yz_.v = (yz_.p3 - yz_.p0);
			yz_.normal = ((yz_.p0 + yz_.p2) * 0.5f - centroid);
			planes[3] = yz_;
			r_normals[3] = normalize(cross(planes[3].u, planes[3].v));


			// xz           // xz_
			xz.p0 = p[0];   xz_.p0 = p[1];
			xz.p1 = p[6];   xz_.p1 = p[5];
			xz.p2 = p[7];   xz_.p2 = p[4];
			xz.p3 = p[3];   xz_.p3 = p[2];

			xz.u = (xz.p1 - xz.p0);
			xz.v = (xz.p3 - xz.p0);
			xz.normal = ((xz.p0 + xz.p2) * 0.5f - centroid);
			planes[4] = xz;
			r_normals[4] = -normalize(cross(planes[4].u, planes[4].v));


			xz_.u = (xz_.p1 - xz_.p0);
			xz_.v = (xz_.p3 - xz_.p0);
			xz_.normal = ((xz_.p0 + xz_.p2) * 0.5f - centroid);
			planes[5] = xz_;
			r_normals[5] = normalize(cross(planes[5].u, planes[5].v));
		}

		int i_min = 0;
		int i_max = 0;
		float d_min = length(planes[0].normal);
		float d_max = d_min;

		for (int i = 0; i < 6; i++)
		{
			planes[i].centroid = (planes[i].p0 + planes[i].p2) * 0.5f;

			Ray to_plane;
			to_plane.origin = planes[i].centroid;
			to_plane.direction = r_normals[i];
			Geometry::RAY::Intersect_Plane
			(to_plane,
			 Geometry::SHAPES::createPlane(planes[i].p0 + planes[i].normal, r_normals[i]), planes[i].p_normal);

			float d = length(planes[i].normal);
			if (d < d_min)
			{
				d_min = d;
				i_min = i;
			}
			if (d > d_max)
			{
				d_max = d;
				i_max = i;
			}

		}


		float inv_scale_factor = m_camera_inv_scale_factor;

		if (inv_scale_factor < 8.0f)
			inv_scale_factor = 8.0f;

		float dist = 2.5f * inv_scale_factor;
		float size = 1.4f * inv_scale_factor;
		dist = 0.0f;
		size = 0.0f;
		const float offset = !is_group ? (d_max + d_min) * 0.05f : 3.5f;
		optix::float2 scale = optix::make_float2(offset * 0.1f);


		// Ray 
		Ray ray = Geometry::SHAPES::createMouseRay();
		bool hitted[6];
		optix::float3 p_isec_p[6];
		optix::float3 t_isec_p[4];
		int hit_i = -1;
		for (int i = 0; i < 6; i++)
		{


			optix::float3 p0 = planes[i].centroid + r_normals[i] * offset;
			optix::float3 du = normalize(planes[i].u) * scale.x;
			optix::float3 dv = normalize(planes[i].v) * scale.y;
			optix::float3 u = planes[i].u * 0.5f + du;
			optix::float3 v = planes[i].v * 0.5f + dv;

			Plane p1 = Geometry::SHAPES::createPlane(p0, u, v);
			Plane p2 = Geometry::SHAPES::createPlane(p0, -u, -v);
			Plane p3 = Geometry::SHAPES::createPlane(p0, u, -v);
			Plane p4 = Geometry::SHAPES::createPlane(p0, -u, v);

			hitted[i] = false;
			int t_hit_i[4] = { false, false, false, false };

			t_hit_i[0] = Geometry::RAY::Intersect_Parallelogram(ray, p1, t_isec_p[0]);
			t_hit_i[1] = Geometry::RAY::Intersect_Parallelogram(ray, p2, t_isec_p[1]);
			t_hit_i[2] = Geometry::RAY::Intersect_Parallelogram(ray, p3, t_isec_p[2]);
			t_hit_i[3] = Geometry::RAY::Intersect_Parallelogram(ray, p4, t_isec_p[3]);

			hitted[i] = t_hit_i[0] || t_hit_i[1] || t_hit_i[2] || t_hit_i[3];
			for (int j = 0; j < 4; j++)
				if (t_hit_i[j] == 1)
					p_isec_p[i] = t_isec_p[j];

		}

		float nearest = 9999999999.0f;
		int nearest_hit = -1;
		for (int i = 0; i < 6; i++)
		{
			if (!hitted[i])
				continue;

			float d = target_camera->getDistanceFrom(p_isec_p[i]);
			if (d < nearest)
			{
				nearest = d;
				nearest_hit = i;
			}
		}

		if (nearest_hit != -1) 
			return_value = true;

		isec_p->hit_index = nearest_hit;
		isec_p->p         = p_isec_p[nearest_hit];
		isec_p->normal    = r_normals[nearest_hit];
		
	}
	


    {
		optix::float3 local_axis[3];
		BASE_TRANSFORM::AXIS(object, LOCAL, local_axis);

		optix::float3 * bbox = object->getBBOX_Transformed( true );
		optix::float3 b_max = bbox[1];
		optix::float3 b_min = bbox[0];
		delete bbox;
		float bbox_size = length((b_max - b_min)) * 0.1f * 2.0f;
		optix::float2 size = m_interface_state.m_sampler_params.dim_2f; //length(b_max - b_min) * 0.15f;
		float scale_factor = 1.0f;
		optix::float3 m_axis[3] = { GLOBAL_AXIS[0],GLOBAL_AXIS[1],GLOBAL_AXIS[2] };

		optix::float3 * mouse_data = 0;
		optix::float3 mouse_hit_p;
		optix::float3 mouse_hit_n;
		
		if (use_bbox)
		{
			mouse_hit_p = isec_p->p;
			mouse_hit_n = isec_p->normal;
		}
		else
		{
			mouse_data = Mediator::request_MouseHit_Buffer_Data();
			mouse_hit_p = mouse_data[0];
			mouse_hit_n = mouse_data[1];
			delete mouse_data;
		}
		

		float offset = 0.0f;
		optix::float3 offset_u = mouse_hit_n;

		SAMPLER_ALIGNEMENT SA = m_interface_state.m_sa;
		optix::float3 t_axis[3];
		OnB onb;
		optix::float3 origin;

		Utility ut = interaction_utility.id;
		if      (ut == u_ATTACH_SAMPLER_PLANAR)
		{
			if (SA == SA_GLOBAL)
			{
				t_axis[0] = GLOBAL_AXIS[0];
				t_axis[1] = GLOBAL_AXIS[1];
				t_axis[2] = GLOBAL_AXIS[2];
				float du = fabsf(1.0f - fabsf(optix::dot(t_axis[0], mouse_hit_n)));
				float dv = fabsf(1.0f - fabsf(optix::dot(t_axis[1], mouse_hit_n)));
				float dw = fabsf(1.0f - fabsf(optix::dot(t_axis[2], mouse_hit_n)));

				int min_i = 0;
				float min = (du);
				if ((dv) < min)
				{
					min_i = 1;
					min = (dv);
				}
				if ((dw) < min)
				{
					min_i = 2;
					min = (dw);
				}

				int sign = dot(mouse_hit_n, t_axis[min_i]) >= 0.0f ? 1 : -1;
				mouse_hit_n = t_axis[min_i] * sign;

				onb.w = mouse_hit_n;
				onb.u = min_i == 0 ? t_axis[1] : t_axis[0];
			}
			else if (SA == SA_LOCAL)
			{
				BASE_TRANSFORM::AXIS(object, LOCAL, t_axis);
				float du = fabsf(1.0f - fabsf(optix::dot(t_axis[0], mouse_hit_n)));
				float dv = fabsf(1.0f - fabsf(optix::dot(t_axis[1], mouse_hit_n)));
				float dw = fabsf(1.0f - fabsf(optix::dot(t_axis[2], mouse_hit_n)));

				int min_i = 0;
				float min = (du);
				if ((dv) < min)
				{
					min_i = 1;
					min = (dv);
				}
				if ((dw) < min)
				{
					min_i = 2;
					min = (dw);
				}

				int sign = dot(mouse_hit_n, t_axis[min_i]) >= 0.0f ? 1 : -1;
				mouse_hit_n = t_axis[min_i] * sign;

				onb.w = mouse_hit_n;
				onb.u = min_i == 0 ? t_axis[1] : t_axis[0];
			}
			else if (SA == SA_NORMAL)
			{
				bool local = false;
				optix::float3 local_axis[3];
				BASE_TRANSFORM::AXIS(object, LOCAL, local_axis);

				onb.w = mouse_hit_n;
				int j = -1;
				for (int i = 0; i < 3; i++)
					if (onb.w.x == GLOBAL_AXIS[i].x && onb.w.y == GLOBAL_AXIS[i].y && onb.w.z == GLOBAL_AXIS[i].z)
					{
						j = i;
						break;
					}

				if (j == -1)
				{
					for (int i = 0; i < 3; i++)
					{
						optix::float3 d = onb.w - local_axis[i];
						optix::float3 d_ = onb.w + local_axis[i];
						if (fabsf(d.x) < 1e-4 && fabsf(d.y) < 1e-4 && fabsf(d.z) < 1e-4)
						{
							j = i;
							local = true;
							break;
						}
						else if (fabsf(d_.x) < 1e-4 && fabsf(d_.y) < 1e-4 && fabsf(d_.z) < 1e-4)
						{
							j = i;
							local = true;
							//onb.w *= -1.0f;
							break;
						}
					}
				}

				if (j != -1)
				{
					if (local)
						onb.u = j == 0 ? local_axis[1] : local_axis[0];
					else
						onb.u = j == 0 ? GLOBAL_AXIS[1] : GLOBAL_AXIS[0];
				}
				else
				{
					bool is_v = ((mouse_hit_n.x == GLOBAL_AXIS[1].x) && (mouse_hit_n.y == GLOBAL_AXIS[1].y) && (mouse_hit_n.z == GLOBAL_AXIS[1].z))
						|| ((mouse_hit_n.x == -GLOBAL_AXIS[1].x) && (mouse_hit_n.y == -GLOBAL_AXIS[1].y) && (mouse_hit_n.z == -GLOBAL_AXIS[1].z));
					optix::float3 v_axis = (is_v) ? GLOBAL_AXIS[2] : GLOBAL_AXIS[1];

					onb.u = normalize(cross(onb.w, v_axis));
				}
			}


			onb.v = normalize(cross(onb.u, onb.w));
			origin = mouse_hit_p;

			////std::cout << "\n - onb.u : [ " << onb.u.x << " , " << onb.u.y << " , " << onb.u.z << " ] " << std::endl;
			////std::cout << " - onb.v : [ " << onb.v.x << " , " << onb.v.y << " , " << onb.v.z << " ] " << std::endl;
			////std::cout << " - onb.w : [ " << onb.w.x << " , " << onb.w.y << " , " << onb.w.z << " ] " << std::endl;
		}
		else if (ut == u_ATTACH_SAMPLER_VOLUME)
		{
			if (SA == SA_GLOBAL)
			{

				t_axis[0] = GLOBAL_AXIS[0];t_axis[1] = GLOBAL_AXIS[1];t_axis[2] = GLOBAL_AXIS[2];
				float du = fabsf(1.0f - fabsf(optix::dot(t_axis[0], mouse_hit_n)));
				float dv = fabsf(1.0f - fabsf(optix::dot(t_axis[1], mouse_hit_n)));
				float dw = fabsf(1.0f - fabsf(optix::dot(t_axis[2], mouse_hit_n)));

				int min_i = 0;
				float min = (du);
				if ((dv) < min)
				{
					min_i = 1;
					min = (dv);
				}
				if ((dw) < min)
				{
					min_i = 2;
					min = (dw);
				}

				int sign = dot(mouse_hit_n, t_axis[min_i]) >= 0.0f ? 1 : -1;

				float * v = (float *)&m_interface_state.m_sampler_params.dim_3f;
				offset = v[min_i] * 0.5f;
				offset_u = t_axis[min_i] * sign;

				BASE_TRANSFORM::AXIS(object, WORLD, t_axis);
				onb.u = -t_axis[0];
				onb.v = t_axis[1];
				onb.w = t_axis[2];
			}
			else if (SA == SA_LOCAL)
			{
				t_axis[0] = GLOBAL_AXIS[0];t_axis[1] = GLOBAL_AXIS[1];t_axis[2] = GLOBAL_AXIS[2];
				float du = fabsf(1.0f - fabsf(optix::dot(t_axis[0], mouse_hit_n)));
				float dv = fabsf(1.0f - fabsf(optix::dot(t_axis[1], mouse_hit_n)));
				float dw = fabsf(1.0f - fabsf(optix::dot(t_axis[2], mouse_hit_n)));

				int min_i = 0;
				float min = (du);
				if ((dv) < min)
				{
					min_i = 1;
					min = (dv);
				}
				if ((dw) < min)
				{
					min_i = 2;
					min = (dw);
				}

				int sign = dot(mouse_hit_n, t_axis[min_i]) >= 0.0f ? 1 : -1;


				float * v = (float *)&m_interface_state.m_sampler_params.dim_3f;
				offset = v[min_i] * 0.5f;
				offset_u = t_axis[min_i] * sign;

				BASE_TRANSFORM::AXIS(object, LOCAL, t_axis);
				onb.u = -t_axis[0];
				onb.v = t_axis[1];
				onb.w = t_axis[2];

			}
			else if (SA == SA_NORMAL)
			{
				bool local = false;
				optix::float3 local_axis[3];
				BASE_TRANSFORM::AXIS(object, LOCAL, local_axis);

				onb.w = mouse_hit_n;
				int j = -1;
				for (int i = 0; i < 3; i++)
					if (onb.w.x == GLOBAL_AXIS[i].x && onb.w.y == GLOBAL_AXIS[i].y && onb.w.z == GLOBAL_AXIS[i].z)
					{
						j = i;
						break;
					}

				if (j == -1)
				{
					for (int i = 0; i < 3; i++)
					{
						optix::float3 d = onb.w - local_axis[i];
						optix::float3 d_ = onb.w + local_axis[i];
						if (fabsf(d.x) < 1e-4 && fabsf(d.y) < 1e-4 && fabsf(d.z) < 1e-4)
						{
							j = i;
							local = true;
							break;
						}
						else if (fabsf(d_.x) < 1e-4 && fabsf(d_.y) < 1e-4 && fabsf(d_.z) < 1e-4)
						{
							j = i;
							local = true;
							//onb.w *= -1.0f;
							break;
						}
					}
				}

				if (j != -1)
				{
					if (local)
						onb.u = j == 0 ? local_axis[1] : local_axis[0];
					else
						onb.u = j == 0 ? GLOBAL_AXIS[1] : GLOBAL_AXIS[0];
				}
				else
				{
					bool is_v = ((mouse_hit_n.x == GLOBAL_AXIS[1].x) && (mouse_hit_n.y == GLOBAL_AXIS[1].y) && (mouse_hit_n.z == GLOBAL_AXIS[1].z))
						|| ((mouse_hit_n.x == -GLOBAL_AXIS[1].x) && (mouse_hit_n.y == -GLOBAL_AXIS[1].y) && (mouse_hit_n.z == -GLOBAL_AXIS[1].z));
					optix::float3 v_axis = (is_v) ? GLOBAL_AXIS[2] : GLOBAL_AXIS[1];

					onb.u = normalize(cross(onb.w, v_axis));
				}
				onb.v = normalize(cross(onb.u, onb.w));

				t_axis[0] = onb.u; t_axis[1] = onb.v; t_axis[2] = onb.w;
				offset = m_interface_state.m_sampler_params.dim_3f.z * 0.5f;
			}

			origin = mouse_hit_p + offset_u * offset;
		}
		else if (ut == u_ATTACH_SAMPLER_DIRECTIONAL)
		{
			scale_factor = 0.1f;
			if (SA == SA_GLOBAL)
			{
				t_axis[0] = GLOBAL_AXIS[0];t_axis[1] = GLOBAL_AXIS[1];t_axis[2] = GLOBAL_AXIS[2];
				float du = fabsf(1.0f - fabsf(optix::dot(t_axis[0], mouse_hit_n)));
				float dv = fabsf(1.0f - fabsf(optix::dot(t_axis[1], mouse_hit_n)));
				float dw = fabsf(1.0f - fabsf(optix::dot(t_axis[2], mouse_hit_n)));

				int min_i = 0;
				float min = (du);
				if ((dv) < min)
				{
					min_i = 1;
					min = (dv);
				}
				if ((dw) < min)
				{
					min_i = 2;
					min = (dw);
				}

				int sign = dot(mouse_hit_n, t_axis[min_i]) >= 0.0f ? 1 : -1;
				mouse_hit_n = t_axis[min_i] * sign;

			}
			else if (SA == SA_LOCAL)
			{
				BASE_TRANSFORM::AXIS(object, LOCAL, t_axis);
				float du = fabsf(1.0f - fabsf(optix::dot(t_axis[0], mouse_hit_n)));
				float dv = fabsf(1.0f - fabsf(optix::dot(t_axis[1], mouse_hit_n)));
				float dw = fabsf(1.0f - fabsf(optix::dot(t_axis[2], mouse_hit_n)));

				int min_i = 0;
				float min = (du);
				if ((dv) < min)
				{
					min_i = 1;
					min = (dv);
				}
				if ((dw) < min)
				{
					min_i = 2;
					min = (dw);
				}

				int sign = dot(mouse_hit_n, t_axis[min_i]) >= 0.0f ? 1 : -1;
				mouse_hit_n = t_axis[min_i] * sign;
			}
			else if (SA == SA_NORMAL)
			{

			}

			bool is_v = ((mouse_hit_n.x == GLOBAL_AXIS[1].x) && (mouse_hit_n.y == GLOBAL_AXIS[1].y) && (mouse_hit_n.z == GLOBAL_AXIS[1].z))
				|| ((mouse_hit_n.x == -GLOBAL_AXIS[1].x) && (mouse_hit_n.y == -GLOBAL_AXIS[1].y) && (mouse_hit_n.z == -GLOBAL_AXIS[1].z));
			optix::float3 v_axis = (is_v) ? GLOBAL_AXIS[2] : GLOBAL_AXIS[1];

			onb.w = mouse_hit_n;
			onb.u = normalize(cross(onb.w, v_axis));
			onb.v = normalize(cross(onb.u, onb.w));

			optix::float3 temp = onb.v;
			onb.v = onb.w;
			onb.w = temp;
			onb.u *= -1.0f;

			//optix::Matrix4x4 mat = optix::Matrix4x4::fromBasis(onb.u, onb.v, onb.w, ZERO_3f);
			//onb.u = normalize(optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[0], 0.0f)));
			//onb.v = normalize(optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[1], 0.0f)));
			//onb.w = normalize(optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[2], 0.0f)));

			origin = mouse_hit_p;

		}

		m_interface_state.m_sampler_params.basis[0] = onb.u;
		m_interface_state.m_sampler_params.basis[1] = onb.v;
		m_interface_state.m_sampler_params.basis[2] = onb.w;
		m_interface_state.m_sampler_params.scale = optix::make_float3(bbox_size * scale_factor);
		m_interface_state.m_sampler_params.origin = origin;
		//m_interface_state.m_sampler_params.origin = mouse_hit_p;
		
	}

	//return return_value;
	return false;
}


bool InterfaceManager::get_Utility_Interaction_g_CREATE(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties,
														IntersectionPoint * isec_p, UtilityProperties interaction_utility)
{

	Utility ut = interaction_utility.id;
	bool is_create_light = ut == u_CREATE_LIGHT;
	if (is_create_light)
		return get_Utility_Interaction_g_CREATE_LIGHT(target_window, target_camera, target_obj_properties, isec_p, interaction_utility);
	else
		return false;
}
bool InterfaceManager::get_Utility_Interaction_g_CREATE_LIGHT(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility)
{
	//std::cout << "\n - get_Utility_Interaction_g_CREATE_LIGHT():" << std::endl;

	Plane isec_plane;
	optix::float3 plane_p0;
	optix::float3 plane_n = optix::make_float3(0, 1, 0);
	int is_hit = 1;

	int isec_plane_id          = m_light_init_isec_plane;
	SAMPLER_ALIGNEMENT s_align = m_interface_state.m_sa;
	//std::cout << "   - s_algin : " << m_interface_state.m_sa << std::endl;

	m_interface_state.m_sampler_params.basis[0] = GLOBAL_AXIS[0];
	m_interface_state.m_sampler_params.basis[1] = GLOBAL_AXIS[1];
	m_interface_state.m_sampler_params.basis[2] = GLOBAL_AXIS[2];

	// plane isec = 1 -> isec zero plane
	if      (s_align == SAMPLER_ALIGNEMENT::SA_GLOBAL)
	{
		m_interface_state.m_sampler_params.basis[0] = GLOBAL_AXIS[0];
		m_interface_state.m_sampler_params.basis[1] = GLOBAL_AXIS[1] * (m_interface_state.flip_side ? -1 : 1);
		m_interface_state.m_sampler_params.basis[2] = GLOBAL_AXIS[2] * (m_interface_state.flip_side ? -1 : 1);

		plane_p0 = ZERO_3f;

		// isec plane
		isec_plane = Geometry::SHAPES::createPlane(plane_p0, plane_n);
		Ray mouse_ray = Geometry::SHAPES::createMouseRay();
		optix::float3 Ip;
		Geometry::RAY::Intersect_Plane(mouse_ray, isec_plane, Ip);

		// store isec point //
		isec_p->hit_index = is_hit;
		isec_p->p = Ip;
		isec_p->p1 = plane_p0;
		isec_p->normal = plane_n * (m_interface_state.flip_side? -1 : 1);
		m_interface_state.m_light_params.position = isec_p->p;
	}

	else if (s_align == SAMPLER_ALIGNEMENT::SA_BBOX)
	{
		//std::cout << "  - SA_BBOX:" << std::endl;
		SceneObject* obj    = m_selected_object_pack.raw_object;
		bool is_null = obj == nullptr;
		Type obj_type = is_null ? Type::DUMMY : obj->getType();
		bool is_valid_type = obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT;
		if (!is_null && is_valid_type)
		{
			bool is_group = obj_type == GROUP_PARENT_DUMMY;
			VIEWPORT vp = ViewportManager::GetInstance().getActiveViewport();
			float obj_dist = target_camera->getDistanceFrom(obj->getTranslationAttributes());
			float renderSize_factor = target_camera->getInvScaleFactor(vp);
			float m_camera_inv_scale_factor = obj_dist * LINKING_SCALE_FACTOR * renderSize_factor;
			float scale_factor = optix::length(obj->getScaleAttributes());
			optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
			optix::float3 centroid = obj->getCentroid();
			if (!is_group)
				centroid = optix::make_float3(def_transformation * optix::make_float4(centroid, 1.0f));

			optix::float3 bbox_min = obj->getBBoxMin();
			optix::float3 bbox_max = obj->getBBoxMax();
			float xm = bbox_min.x;
			float ym = bbox_min.y;
			float zm = bbox_min.z;
			float xM = bbox_max.x;
			float yM = bbox_max.y;
			float zM = bbox_max.z;

			optix::float3 p[8];
			p[0] = bbox_min;
			p[1] = optix::make_float3(xm, yM, zm);
			p[2] = optix::make_float3(xM, yM, zm);
			p[3] = optix::make_float3(xM, ym, zm);

			p[4] = bbox_max;
			p[5] = optix::make_float3(xm, yM, zM);
			p[6] = optix::make_float3(xm, ym, zM);
			p[7] = optix::make_float3(xM, ym, zM);

			if (!is_group)
			{
				for (int i = 0; i < 8; i++)
					p[i] = optix::make_float3(def_transformation * optix::make_float4(p[i], 1.0f));
			}
			// Define the [6] faces //
			Plane xy, xy_, yz, yz_, xz, xz_;

			optix::float3 r_normals[6];

			float const_dist = 100.0f;
			Plane planes[6];
			Plane r_planes[6];
			{
				// xy           // xy_
				xy.p0 = p[0];   xy_.p0 = p[6];
				xy.p1 = p[3];   xy_.p1 = p[7];
				xy.p2 = p[2];   xy_.p2 = p[4];
				xy.p3 = p[1];   xy_.p3 = p[5];

				xy.u = (xy.p1 - xy.p0);
				xy.v = (xy.p3 - xy.p0);
				xy.normal = ((xy.p0 + xy.p2) * 0.5f - centroid);
				planes[0] = xy;
				r_normals[0] = -normalize(cross(planes[0].u, planes[0].v));


				xy_.u = (xy_.p1 - xy_.p0);
				xy_.v = (xy_.p3 - xy_.p0);
				xy_.normal = ((xy_.p0 + xy_.p2) * 0.5f - centroid);
				planes[1] = xy_;
				r_normals[1] = normalize(cross(planes[1].u, planes[1].v));


				// yz           // yz_
				yz.p0 = p[3];   yz_.p0 = p[0];
				yz.p1 = p[7];   yz_.p1 = p[6];
				yz.p2 = p[4];   yz_.p2 = p[5];
				yz.p3 = p[2];   yz_.p3 = p[1];

				yz.u = (yz.p1 - yz.p0);
				yz.v = (yz.p3 - yz.p0);
				yz.normal = ((yz.p0 + yz.p2) * 0.5f - centroid);
				planes[2] = yz;
				r_normals[2] = -normalize(cross(planes[2].u, planes[2].v));

				yz_.u = (yz_.p1 - yz_.p0);
				yz_.v = (yz_.p3 - yz_.p0);
				yz_.normal = ((yz_.p0 + yz_.p2) * 0.5f - centroid);
				planes[3] = yz_;
				r_normals[3] = normalize(cross(planes[3].u, planes[3].v));


				// xz           // xz_
				xz.p0 = p[0];   xz_.p0 = p[1];
				xz.p1 = p[6];   xz_.p1 = p[5];
				xz.p2 = p[7];   xz_.p2 = p[4];
				xz.p3 = p[3];   xz_.p3 = p[2];

				xz.u = (xz.p1 - xz.p0);
				xz.v = (xz.p3 - xz.p0);
				xz.normal = ((xz.p0 + xz.p2) * 0.5f - centroid);
				planes[4] = xz;
				r_normals[4] = -normalize(cross(planes[4].u, planes[4].v));


				xz_.u = (xz_.p1 - xz_.p0);
				xz_.v = (xz_.p3 - xz_.p0);
				xz_.normal = ((xz_.p0 + xz_.p2) * 0.5f - centroid);
				planes[5] = xz_;
				r_normals[5] = normalize(cross(planes[5].u, planes[5].v));
			}

			int i_min = 0;
			int i_max = 0;
			float d_min = length(planes[0].normal);
			float d_max = d_min;

			for (int i = 0; i < 6; i++)
			{
				planes[i].centroid = (planes[i].p0 + planes[i].p2) * 0.5f;

				Ray to_plane;
				to_plane.origin = planes[i].centroid;
				to_plane.direction = r_normals[i];
				Geometry::RAY::Intersect_Plane
				(to_plane,
				 Geometry::SHAPES::createPlane(planes[i].p0 + planes[i].normal, r_normals[i]), planes[i].p_normal);

				float d = length(planes[i].normal);
				if (d < d_min)
				{
					d_min = d;
					i_min = i;
				}
				if (d > d_max)
				{
					d_max = d;
					i_max = i;
				}

			}


			float inv_scale_factor = m_camera_inv_scale_factor;

			if (inv_scale_factor < 8.0f)
				inv_scale_factor = 8.0f;

			float dist = 2.5f * inv_scale_factor;
			float size = 1.4f * inv_scale_factor;
			dist = 0.0f;
			size = 0.0f;
			const float offset = !is_group ? (d_max + d_min) * 0.05f : 3.5f;
			optix::float2 scale = optix::make_float2(offset * 0.1f);


			// Ray 
			Ray ray = Geometry::SHAPES::createMouseRay();
			bool hitted[6];
			optix::float3 p_isec_p[6];
			optix::float3 t_isec_p[4];
			int hit_i = -1;
			for (int i = 0; i < 6; i++)
			{

				optix::float3 p0 = planes[i].centroid + r_normals[i] * offset;
				optix::float3 du = normalize(planes[i].u) * scale.x;
				optix::float3 dv = normalize(planes[i].v) * scale.y;
				optix::float3 u = planes[i].u * 0.5f + du;
				optix::float3 v = planes[i].v * 0.5f + dv;

				Plane p1 = Geometry::SHAPES::createPlane(p0, u, v);
				Plane p2 = Geometry::SHAPES::createPlane(p0, -u, -v);
				Plane p3 = Geometry::SHAPES::createPlane(p0, u, -v);
				Plane p4 = Geometry::SHAPES::createPlane(p0, -u, v);

				hitted[i] = false;
				int t_hit_i[4] = { false, false, false, false };

				t_hit_i[0] = Geometry::RAY::Intersect_Parallelogram(ray, p1, t_isec_p[0]);
				t_hit_i[1] = Geometry::RAY::Intersect_Parallelogram(ray, p2, t_isec_p[1]);
				t_hit_i[2] = Geometry::RAY::Intersect_Parallelogram(ray, p3, t_isec_p[2]);
				t_hit_i[3] = Geometry::RAY::Intersect_Parallelogram(ray, p4, t_isec_p[3]);

				hitted[i] = t_hit_i[0] || t_hit_i[1] || t_hit_i[2] || t_hit_i[3];
				for (int j = 0; j < 4; j++)
					if (t_hit_i[j] == 1)
						p_isec_p[i] = t_isec_p[j];

			}

			float nearest = 9999999999.0f;
			int nearest_hit = -1;
			for (int i = 0; i < 6; i++)
			{
				if (!hitted[i])
					continue;

				float d = target_camera->getDistanceFrom(p_isec_p[i]);
				if (d < nearest)
				{
					nearest = d;
					nearest_hit = i;
				}
			}

			if (nearest_hit != -1)
				is_hit = false;
			
			
			isec_p->hit_index = nearest_hit;
			isec_p->p         = p_isec_p[nearest_hit];
			isec_p->normal    = r_normals[nearest_hit];
			//isec_p->normal   *= m_interface_state.flip_side ? 1 : -1;
			//isec_p->p1 = isec_p->p;
			//m_interface_state.m_light_params.position = isec_p->p;
			//m_interface_state.m_light_params.n        = isec_p->normal;
			//m_interface_state.isec_p->hit_index       = nearest_hit;

		}
		else
		{
			isec_p->hit_index = -1;
			m_interface_state.isec_p->hit_index = -1;
		}
	}

	// plane isec = 2 -> isec camera - v plane
	if (s_align == SAMPLER_ALIGNEMENT::SA_NORMAL
		|| s_align == SAMPLER_ALIGNEMENT::SA_BBOX)
	{

		bool is_BBOX = s_align == SA_BBOX;
		if (!is_BBOX)
		{
			is_hit = (m_focused_object_pack.raw_object != nullptr) ? 1 : 0;
			optix::float3 * data = Mediator::request_MouseHit_Buffer_Data();
			plane_p0 = data[0];
			plane_n  = data[1];
			delete data;
		}
		else
		{
			is_hit = isec_p->hit_index != -1? true : false;
			if (is_hit)
			{
				plane_p0 = isec_p->p;
				plane_n  = isec_p->normal;
			}
		}

		
		//
		if (is_hit)
		{
			plane_n *= (m_interface_state.flip_side ? -1 : 1);
			optix::float3 mouse_hit_p = plane_p0;
			optix::float3 mouse_hit_n = plane_n;

			optix::float3 t_axis[3];
			OnB onb;

			SceneObject* object = is_BBOX? m_selected_object_pack.raw_object : m_focused_object_pack.raw_object;

			bool local = false;
			optix::float3 local_axis[3];
			BASE_TRANSFORM::AXIS(object, LOCAL, local_axis);
			
			onb.w = mouse_hit_n;
			int j = -1;
			
			for (int i = 0; i < 3; i++)
				if (onb.w.x == GLOBAL_AXIS[i].x && onb.w.y == GLOBAL_AXIS[i].y && onb.w.z == GLOBAL_AXIS[i].z)
				{
					j = i;
					break;
				}

			if (j == -1)
			{
				for (int i = 0; i < 3; i++)
				{
					optix::float3 d = onb.w - local_axis[i];
					optix::float3 d_ = onb.w + local_axis[i];
					if (fabsf(d.x) < 1e-4 && fabsf(d.y) < 1e-4 && fabsf(d.z) < 1e-4)
					{
						j = i;
						local = true;
						break;
					}
					else if (fabsf(d_.x) < 1e-4 && fabsf(d_.y) < 1e-4 && fabsf(d_.z) < 1e-4)
					{
						j = i;
						local = true;
						//onb.w *= -1.0f;
						break;
					}
				}
			}

			if (j != -1)
			{
				if (local)
					onb.u = j == 0 ? local_axis[1] : local_axis[0];
				else
					onb.u = j == 0 ? GLOBAL_AXIS[1] : GLOBAL_AXIS[0];
			}
			else
			{
				bool is_v = ((mouse_hit_n.x == GLOBAL_AXIS[1].x) && (mouse_hit_n.y == GLOBAL_AXIS[1].y) && (mouse_hit_n.z == GLOBAL_AXIS[1].z))
					|| ((mouse_hit_n.x == -GLOBAL_AXIS[1].x) && (mouse_hit_n.y == -GLOBAL_AXIS[1].y) && (mouse_hit_n.z == -GLOBAL_AXIS[1].z));
				optix::float3 v_axis = (is_v) ? GLOBAL_AXIS[2] : GLOBAL_AXIS[1];

				onb.u = normalize(cross(onb.w, v_axis));
			}
			onb.v = normalize(cross(onb.u, onb.w));
			m_interface_state.m_sampler_params.basis[0] = onb.u;
			m_interface_state.m_sampler_params.basis[1] = onb.w;
			m_interface_state.m_sampler_params.basis[2] = onb.v;

		}
		//

		//if (s_align == SAMPLER_ALIGNEMENT::SA_NORMAL)
		{
			
			isec_p->hit_index = is_BBOX ? isec_p->hit_index : is_hit;
			isec_p->p      = plane_p0;
			isec_p->p1     = plane_p0;
			isec_p->normal = plane_n;
			m_interface_state.m_light_params.position = isec_p->p;
		}

	}

	

	return false;
}


bool InterfaceManager::get_Light_toggleButton_Interaction(PinholeCamera* target_camera, optix::float3& ip, SceneObject* light_object, int& obj_id)
{

	struct tempStruct
	{
		SceneObject* obj = 0;
		optix::float3 hit;
	};
	std::vector<tempStruct> hits;

	for (SceneObject* obj : Mediator::RequestSceneObjects())
	{
		obj->set_LightButtonHovered(false);
		bool is_valid = obj->isActive() && obj->isVisible() && obj->getType() == LIGHT_OBJECT;
		VIEWPORT vp = ViewportManager::GetInstance().getActiveViewport();
		Light_Type ltype = obj->get_LightParameters().type;
		if (ltype == QUAD_LIGHT && !(vp.render_settings.draw_lights_all || vp.render_settings.draw_lights_quad))
		{
			is_valid = false;
			continue;
		}
		if (ltype == SPOTLIGHT && !(vp.render_settings.draw_lights_all || vp.render_settings.draw_lights_spot))
		{
			is_valid = false;
			continue;
		}
		if (ltype == SPHERICAL_LIGHT && !(vp.render_settings.draw_lights_all || vp.render_settings.draw_lights_sphere))
		{
			is_valid = false;
			continue;
		}
		if (ltype == POINT_LIGHT && !(vp.render_settings.draw_lights_all || vp.render_settings.draw_lights_point))
		{
			is_valid = false;
			continue;
		}

		if (is_valid)
		{
			optix::float3 c   = obj->getCentroid_Transformed();
			optix::float3 * b = obj->getBBOX_Transformed(true);
			optix::float3 bbox[2] = { b[0], b[1] };
			delete b;

			PinholeCamera cam = *target_camera;
			float dist = cam.getDistanceFrom(c);
			float scale_factor = dist * vp.scale_factor * UTILITY_SCALE_FACTOR * 0.001f;
			optix::float3 u = normalize(cam.getU());
			optix::float3 v = normalize(cam.getV());
			optix::float3 w = normalize(cam.getW());

			Shape plane;
			plane.shape_id = PLANE;
			float l = 15.0f;
			plane.axis[0] = u * scale_factor * l;
			plane.axis[1] = w * scale_factor * l;
			plane.axis[2] = v * scale_factor * l;
			plane.pos = c + v * 10.0f + plane.axis[0] * 0.5f;

			Ray ray     = Geometry::SHAPES::createMouseRay(vp);
			Plane PLANE = Geometry::SHAPES::createPlane(plane.pos, plane.axis[0], plane.axis[2]);
			PLANE.normal = -w;
			optix::float3 ipp;
			int is_hit = Geometry::RAY::Intersect_Parallelogram(ray, PLANE, ipp);

			if (is_hit)
			{
				tempStruct strct;
				strct.obj = obj;
				strct.hit = ipp;
				hits.push_back(strct);
			}
		}
	}

	if (hits.size() > 0)
	{
		float mdist = target_camera->getDistanceFrom(hits[0].hit);
		int mi = 0;
		for (int i = 1; i<hits.size(); i++)
		{
			float d = target_camera->getDistanceFrom(hits[i].hit);
			if (d < mdist)
			{
				mdist = d;
				mi    = i;
			}
		}

		//std::cout << " - mi : " << mi << std::endl;
		//std::cout << " - hits[mi].obj : " << hits[mi].obj->getName() << std::endl;

		light_object = hits[mi].obj;
		ip           = hits[mi].hit;
		obj_id       = Mediator::requestObjectsArrayIndex(light_object->getId());
		return true;
	}

	return false;
}