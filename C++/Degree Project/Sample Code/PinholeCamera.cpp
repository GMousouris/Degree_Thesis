#include "PinholeCamera.h"
#include <iostream>
#include "Utilities.h"
#include "Mediator.h"
#include "SceneObject.h"
#include "ViewportManager.h"
#include "GeometryFunctions.h"

//#define CAMERA_DEBUG_PRINT
//#define CAMERA_PANNING_DEBUG_PRINT
//#define CAMERA_UPDATE_DEBUG_PRINT

PinholeCamera::PinholeCamera()
//: m_distance(10.0f) // Some camera defaults for the demo scene.
//, m_phi(0.70f) // 0.75f
//, m_theta(0.0f) // 0.6f
//, m_fov(60.0f) // 60.0f
	: m_distance(300.0f) // 20.0f
	, m_phi(-0.75f)  // positive z-axis
	, m_theta(0.75f) // equator
	, m_fov(45.0f)
	, m_near(0.1f)
	, m_width(1)
	, m_height(1)
	, m_aspect(1.f)
	, m_baseX(0)
	, m_baseY(0)
	, m_speedRatio(20.f) // 10.0f
	, m_dx(0)
	, m_dy(0)
	, m_changed(true)
	, m_state(CAM_NONE)
{

	m_cameraU = optix::make_float3(1.0f, 0.0f, 0.0f);
	m_cameraV = optix::make_float3(0.0f, 1.0f, 0.0f);
	m_cameraW = optix::make_float3(0.0f, 0.0f, 1.0f);



	// custom camera model
	m_camera_position = optix::make_float3(0, 200, 400) * 3.0f;
	m_camera_temp_position = m_camera_position;
	m_cameraPosition = m_camera_position;
	m_camera_orbit_center = ZERO_3f;
	m_camera_focus_center = ZERO_3f;

	{
		m_camera_w = normalize(m_camera_orbit_center - m_cameraPosition);
		m_camera_u = normalize(cross(m_camera_w, GLOBAL_AXIS[1]));
		m_camera_v = normalize(cross(m_camera_u, m_camera_w));

		m_orbit_dir = normalize(m_camera_orbit_center - m_cameraPosition);
		m_focus_dir = m_orbit_dir;
		m_camera_distance = optix::length(m_cameraPosition);

	}

	{
		m_orbit_u = m_camera_u;
		m_orbit_v = m_camera_v;
		m_orbit_w = m_camera_w;
	}

	m_camera_matrix = optix::Matrix4x4::fromBasis(m_camera_u, m_camera_v, m_camera_w, ZERO_3f);
	m_orbit_matrix = optix::Matrix4x4::fromBasis(m_orbit_u, m_orbit_v, m_orbit_w, ZERO_3f);

	// m_relative_pos = m_camera_orbit_center - m_camera_w * m_camera_distance;
	m_relative_pos = optix::make_float3((optix::Matrix4x4::translate(m_camera_orbit_center) * m_orbit_matrix).inverse() * optix::make_float4(m_cameraPosition, 1.0f));

	m_position_offset = ZERO_3f;

	m_camera_movement = optix::make_float2(0.0f);

	m_orbit_dx = 0.0f;
	m_orbit_dy = 0.0f;

	du = ZERO_3f;
	dv = ZERO_3f;
	dw = ZERO_3f;

	sign = 1.0f;
}
PinholeCamera::~PinholeCamera()
{

}
void PinholeCamera::cleanUp()
{

}
void PinholeCamera::setViewport(int w, int h)
{

	if (m_width != w || m_height != h)
	{
		// Never drop to zero viewport size. This avoids lots of checks for zero in other routines.
		m_width = (w) ? w : 1;
		m_height = (h) ? h : 1;
		m_aspect = float(m_width) / float(m_height);
		m_changed = true;
	}

}
void PinholeCamera::setBaseCoordinates(int x, int y)
{
	m_baseX = x;
	m_baseY = y;
}
void PinholeCamera::clamp(float & value, float min, float max, float d)
{

	if (value < min)
		value += d;
	else if (value > max)
		value -= d;
}
void PinholeCamera::clamp(float & value, float min, float max, float d_min, float d_max)
{

	if (value < min)
		value = d_min;
	else if (value > max)
		value = d_max;
}
void PinholeCamera::orbit(int x, int y)
{

	if (setDelta(x, y))
	{
		

		if (m_free_roam_state)
			m_orbited_on_free_roam = true;

		float vp_scale_factor = ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id).scale_factor;
		vp_scale_factor = vp_scale_factor > 2.1f ? 0.75f : 1.0f;
		////std::cout << " - vp_Scale_factor : " << vp_scale_factor << std::endl;

		float scale_factor = 1.0f;
		float ratio   = m_height > 0.0f ? (m_width / (float)m_height) : 1.0f;
		float ratio_x = ratio;
		float ratio_y = 1.0f / ratio;
		optix::float2 r;
		r.x = ratio_x > 1.5f ? 1.5f : ratio_x;


		int c1 = 0;
		int c2 = 0;
		if (ratio_x > 2.0f)
		{
			ratio_x = 1.5f;
			ratio_y = 0.6666f;
		}
		else
		{
			ratio_x = 1.0f;
			ratio_y = 1.0f;
		}
		r.x = ratio_x;
		r.y = ratio_y;

		
		m_orbit_dx = -float(m_dx) / (m_width * scale_factor) * r.x * vp_scale_factor;
		m_camera_movement.x -= (m_orbit_dx);

		m_orbit_dy = float(m_dy) / (m_height * scale_factor) * sign * r.y * vp_scale_factor;
		m_camera_movement.y += (m_orbit_dy);

		m_changed = true;

	}

}
void PinholeCamera::pan(int x, int y, int prev_x, int prev_y)
{

#ifdef CAMERA_PANNING_DEBUG_PRINT
	////std::cout << "\n - PinholeCamera::Pan( Io_Mouse ): " << std::endl;
#endif

	if ( setDelta(x, y) )
	{

		VIEWPORT vp = ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id);

		optix::float3 plane_origin = m_panning_center; //ZERO_3f;
		optix::float3 plane_normal = -normalize(m_camera_w);

		Plane proj_plane = Geometry::SHAPES::createPlane(plane_origin, plane_normal);
		Ray ray, prev_ray;
		optix::float3 I1, I0;

		ray = Geometry::SHAPES::createMouseRay(glm::vec2(x, y), vp);
		prev_ray = Geometry::SHAPES::createMouseRay(glm::vec2(prev_x, prev_y), vp);
		Geometry::RAY::Intersect_Plane(ray, proj_plane, I1);
		Geometry::RAY::Intersect_Plane(prev_ray, proj_plane, I0);
		optix::float3 distance = I1 - I0;

		optix::float3 u = normalize(m_camera_u);
		optix::float3 v = normalize(m_camera_v);
		
		float du = dot(u, distance);
		float dv = dot(v, distance);
		int sign_x = m_dx == 0 ? 0 : 1;
		int sign_y = m_dy == 0 ? 0 : 1;

		optix::float3 delta = -u * du * sign_x - v * dv * sign_y;
		
		if (!m_free_roam_state)
		{
			setOrbitCenter(m_camera_orbit_center - delta);
			m_camera_orbit_center += delta;
		}
		else
		{
			m_camera_orbit_center += delta;
		}

		m_dx = 0.0f;
		m_dy = 0.0f;
		m_camera_movement = optix::make_float2(0.0f);
		m_changed = true;

	}


}
void PinholeCamera::dolly(int x, int y)
{
	
	if (setDelta( x, y ))
	{
		float w = float(m_dy) / m_speedRatio;
		if (!m_zooming_stable)
		{
			m_zooming_stable = true;
			//m_zooming_stable_activating = true;
			SceneObject* focused_obj  = Mediator::RequestFocusedObject();
			SceneObject* selected_obj = Mediator::RequestSelectedObject();
			bool in_frustum = selected_obj != nullptr ? !Utilities::frustumCull( selected_obj->getCentroid_Transformed( true ) , ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id)) : false;
			
			if (focused_obj != nullptr)
				m_stable_zooming_target = focused_obj->getCentroid_Transformed( true );
			else if (in_frustum)
				m_stable_zooming_target = selected_obj->getCentroid_Transformed( true );
			else
			{
				Ray m_ray = Geometry::SHAPES::createMouseRay();
				optix::float3 ip;
				Geometry::RAY::Intersect_Plane(m_ray, Geometry::SHAPES::createPlane(ZERO_3f, GLOBAL_AXIS[1]), ip);
				m_stable_zooming_target = ip;
			}
			
			zoom( w );
			//m_zooming_stable_activating = false;
		}
		else
			zoom( w );

		//return zoom( w );
		m_changed = true;
	}


}
void PinholeCamera::focus(int x, int y)
{

	if (setDelta(x, y))
	{
		// m_speedRatio pixels will move one vector length.
		float w = float(m_dy) / m_speedRatio;

		// Adjust the center of interest.
		setFocusDistance(m_distance - w * length(m_cameraW));
	}

}
void PinholeCamera::setFocusDistance(float f)
{
	
	if (m_distance != f && 0.001f < f) // Avoid swapping sides.
	{
		m_distance = f;
		m_center = m_cameraPosition + m_distance * m_cameraW; // Keep the camera position fixed and calculate a new center of interest which is the focus plane.
		m_changed = true; // m_changed is only reset when asking for the frustum
	}
}
void PinholeCamera::zoom(float x)
{

#ifdef CAMERA_DEBUG_PRINT
	//std::cout << "\n - - - - - - PinholeCamera::zoom(): - - - - - - - - - -" << std::endl; 
#endif

	float speed = 0.15f * m_camera_distance; // [ 0.1 - 0.2 ]
	float sign  = (x > 0) ? -1 : 1;


	bool zoom_in  = false;
	bool zoom_out = false;
	optix::float3 old_m_camera_orbit_center = m_camera_orbit_center_old;
	float old_m_camera_distance  = m_camera_distance;
	bool was_in_frustum          = m_was_in_frustum;
	bool was_in_frustum_selected = m_was_in_frustum_selected;
	bool was_free_roam           = m_free_roam_state;
	bool was_zoom_stable         = m_zooming_stable;

	// change camera position along mouse direction //
	{
		optix::float3 prev_cameraPos = m_cameraPosition;
		SceneObject* selected_object = Mediator::RequestSelectedObject();
		SceneObject* focused_object  = Mediator::RequestFocusedObject();
		bool is_selected = (selected_object == nullptr) ? false : true;
		bool is_focused  = (focused_object  == nullptr) ? false : true;
		if (is_focused)
			if (focused_object->getType() == APP_OBJECT)
				is_focused     = false;
		
		Ray mouse_ray     = Geometry::SHAPES::createMouseRay();
		if ( m_zooming_stable && !m_zooming_stable_activating ) 
			mouse_ray.direction = normalize(m_stable_zooming_target - m_cameraPosition);
		optix::float3 dir = mouse_ray.direction;
		float delta       = sign * optix::length(dir * x) * speed;
		m_cameraPosition  = m_cameraPosition + dir * -delta;
		optix::float3 pos_delta     = m_cameraPosition - prev_cameraPos;
		optix::float3 target_center = m_camera_orbit_center; //is_selected && !m_zooming_stable ? selected_object->getCentroid_Transformed( true ) : m_camera_orbit_center;
		
		if ( m_zooming_stable ) 
			target_center = m_stable_zooming_target;

		bool in_frustum = ( !Utilities::frustumCull( m_camera_orbit_center, ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id) ) );
		
#ifdef CAMERA_DEBUG_PRINT
		//std::cout << "  - in_frustum : "      << in_frustum << std::endl;
		//std::cout << "  - is_focused : "      << is_focused << std::endl;
		//std::cout << "  - is_selected : "     << is_selected << std::endl;
		//std::cout << "  - zooming_stable : "  << m_zooming_stable << std::endl;
		//std::cout << "  - free_roam_state : " << m_free_roam_state << std::endl;
#endif

		if (    in_frustum 
			&& !m_orbited_on_free_roam 
			&& !m_free_roam_state 
			&&  m_should_retarget_to_center 
			&& !is_focused /* an 8elw na kanw prioritize to selected object */
			&& !m_zooming_stable
			//&&  false
			)
		{
			
#ifdef CAMERA_DEBUG_PRINT
			//std::cout << "  -> [a]::setOrbitCenter( .. , false, true ); " << std::endl;
#endif

			setOrbitCenter( target_center, false, true );
		}
		else
		{
			float delta_dot                 = dot(normalize(pos_delta), dir);
			optix::float3 selected_Centroid = is_selected ? selected_object->getCentroid_Transformed(true) : ZERO_3f;
			bool selected_in_frustum        = is_selected ? (!Utilities::frustumCull(selected_Centroid, ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id))) : false;
			optix::float3 delta      = is_selected ? (selected_Centroid - m_camera_orbit_center) : optix::make_float3(1.0f);
			bool selected_is_orbit   = length(delta) < 1e-5f;

			zoom_in  = (delta_dot > 0.0f);
			zoom_out = (delta_dot < 0.0f);
			
			if ( selected_in_frustum && !is_focused && !m_zooming_stable
				&& selected_is_orbit
				)
			{
#ifdef CAMERA_DEBUG_PRINT
				//std::cout << "  > [0]setting : target_center => selected_Centroid!" << std::endl;
#endif

				target_center = selected_Centroid;
			}
			else if ( is_focused
					 // && !selected_in_frustum  /* an 8elw na kanw prioritize to selected object parolo poy yparxe focused */
					 )
			{


#ifdef CAMERA_DEBUG_PRINT
				//std::cout << "  > [1]:setting : target_center => focused_object.Centroid " << std::endl;
#endif
				target_center = focused_object->getCentroid_Transformed(true);
			}
			else if ( !is_focused && !m_zooming_stable )
			{
				Plane origin_plane = Geometry::SHAPES::createPlane(ZERO_3f, GLOBAL_AXIS[1]);
				optix::float3 Ip; Geometry::RAY::Intersect_Plane(mouse_ray, origin_plane, Ip);
				optix::float3 u = Ip - m_cameraPosition;
				float Ip_dist   = length( u );


				float clamp_value;
				if (delta_dot > 0.0f) // zoom in
				{
					clamp_value = ( /* is_selected && */ is_focused ) ? 500.0f : 1000.0f; // 500 : 1000.0f
				}
				else
				{
					clamp_value = ( is_selected  &&  is_focused ) ? 2000.0f : 3500.0f; // 2 : 4
				}

				if (Ip_dist > clamp_value) Ip_dist = clamp_value;
				target_center = m_cameraPosition + normalize(u) * Ip_dist;

#ifdef CAMERA_DEBUG_PRINT
				//std::cout << "   - is_focused : " << is_focused << std::endl;
				//std::cout << "   - is_selected : " << is_selected << std::endl;
				//std::cout << "  - Ip_dist : " << Ip_dist << std::endl;
				//std::cout << "  > setting : target_center => [ m_cameraPosition + normalize(u) * ( " << Ip_dist << " ) ]" << std::endl;
#endif

			}

			if ( m_zooming_stable )
			{
				target_center = m_stable_zooming_target;

#ifdef CAMERA_DEBUG_PRINT
				//std::cout << "  - m_zooming_stable!" << std::endl;
				//std::cout << "  > setting : target_center => [ m_stable_zooming_target ] " << std::endl;
#endif
			}


			optix::float3 new_orbit_center;
			optix::float3 from_center = target_center - m_cameraPosition;
			float dist_proj           = dot(normalize(m_camera_w), from_center);
			float dist_forward_offset = 0.0f;
			float old_camera_dist     = m_camera_distance;

#ifdef CAMERA_DEBUG_PRINT
			//std::cout << "  - dist_proj : " << dist_proj << std::endl;
			//std::cout << "  ~ m_camera_distance : " << m_camera_distance << std::endl;
#endif

			if (delta_dot < 0.0f) // zoom - out 
			{
				if ( dist_proj <= 9.f )
				{
					dist_proj = 200.0f;//speed;
					//m_camera_distance += optix::length(m_camera_w * x) * sign * dist_proj;

#ifdef CAMERA_DEBUG_PRINT
					//std::cout << "  > setting : m_camera_distance => += [ ( " << length(m_camera_w * x) * sign << " ) * ( " << dist_proj << " ) ] " << std::endl;
#endif
				}

#ifdef CAMERA_DEBUG_PRINT
				//std::cout << "\n  > setting : new_orbit_center => [m_cameraPosition] + normalize(cam_w) * ( " << dist_proj << " ) " << std::endl;
#endif
				new_orbit_center = m_cameraPosition + normalize(m_camera_w) * (dist_proj + dist_forward_offset);
			}
			if ( delta_dot > 0.0f ) // zoom - in
			{
				if (dist_proj < 40.0f) // otan ftasw poso konta 9elw na kanw jump
					dist_proj = 200.0f; // poso jump 9elw na kanw
					
				new_orbit_center = m_cameraPosition + normalize(m_camera_w) * (dist_proj + dist_forward_offset);

#ifdef CAMERA_DEBUG_PRINT
				//std::cout << "    ~ proj_dist : " << dist_proj << std::endl;
				//std::cout << "    ~ current_camera_distance : " << m_camera_distance << std::endl;
				//std::cout << "    ~ old_camera_distance : " << old_camera_dist << std::endl;

				//std::cout << "  > setting : new_orbit_center => [m_cameraPosition] + normalize(cam_w) * ( " << dist_proj << " ) " << std::endl;
#endif

			}

			setOrbitCenter( new_orbit_center, true );
#ifdef CAMERA_DEBUG_PRINT
			//std::cout << "  -> [b]::setOrbitCenter( new_orbit_center, true ) ... " << std::endl;
#endif

		}


		if ( m_zooming_stable_activating )
			m_stable_zooming_target = target_center;

		if (m_zooming_stable)
		{
#ifdef CAMERA_DEBUG_PRINT
			//std::cout << "\n  - zooming_stable = ON! " << std::endl;
			//std::cout << "  -> [c]::setOrbitCenter( stable_zooming_target, true, false ) ... " << std::endl;
#endif
			setOrbitCenter(m_stable_zooming_target, true, false);
		}
		m_changed = true;


		if (m_changed)
		{
			
#ifdef CAMERA_DEBUG_PRINT

			//std::cout << "  - - - - - - - POST_PROCESS - - - - - - - " << std::endl;
			//std::cout << "  [~] zoom_in : "  << zoom_in  << std::endl;
			//std::cout << "  [~] zoom_out : " << !zoom_in << std::endl;
			//std::cout << "  [~] old_camera_distance : " << old_m_camera_distance << std::endl;
			//std::cout << "  [~] new_camera_distance : " << m_camera_distance << std::endl;
			//std::cout << "  [~] old orbit center : [ " << old_m_camera_orbit_center.x << " , " << old_m_camera_orbit_center.y << " , " << old_m_camera_orbit_center.z << " ] " << std::endl;
			//std::cout << "  [~] new orbit center : [ " << m_camera_orbit_center.x << " , " << m_camera_orbit_center.y << " , " << m_camera_orbit_center.z << " ] " << std::endl;
			//std::cout << "  [~] was_in_frustum : " << was_in_frustum << std::endl;
			
			//std::cout << "  [~] free_roam_state : " << m_free_roam_state << std::endl;
			//std::cout << "  [~] was_free_roam_state : " << was_free_roam << std::endl;
			//std::cout << "  [~] zooming_stable : " << m_zooming_stable << std::endl;
			//std::cout << "  [~] was_zooming_stable : " << was_zoom_stable << std::endl;

			
#endif

			bool in_frustum_focused  = false;
			bool in_frustum_selected = false;
			bool in_frustum          = false;
			bool was_selected_object_orbited = false;
			bool is_selected_object_orbited  = false;
			float e1, e2;
			
			if ( is_focused )
			{
				////std::cout << "  [~] was_focused!" << std::endl;
				////std::cout << "  [~] Checking : if focused object is still in frustum!" << std::endl;
				in_frustum_focused = !Utilities::frustumCull( focused_object->getCentroid_Transformed(true), ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id));
			}
			
			if ( in_frustum )
			{
				////std::cout << "  [~] was_in_frustum!" << std::endl;
				////std::cout << "  [~] Checking : if old_orbit_center is still in frustum!" << std::endl;
				//in_frustum_focused = !Utilities::frustumCull( old_m_camera_orbit_center, ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id));
			}

			if ( is_selected )
			{
				optix::float3 selected_object_centroid = selected_object->getCentroid_Transformed(true);
				in_frustum_selected = !Utilities::frustumCull(selected_object_centroid, ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id));

				optix::float3 delta1 = old_m_camera_orbit_center - selected_object_centroid;
				optix::float3 delta2 = m_camera_orbit_center - selected_object_centroid;
				was_selected_object_orbited = (   old_m_camera_orbit_center.x == selected_object_centroid.x 
											   && old_m_camera_orbit_center.y == selected_object_centroid.y
											   && old_m_camera_orbit_center.z == selected_object_centroid.z);
				is_selected_object_orbited = (    m_camera_orbit_center.x == selected_object_centroid.x
											   && m_camera_orbit_center.y == selected_object_centroid.y
											   && m_camera_orbit_center.z == selected_object_centroid.z);

				was_selected_object_orbited = (length(delta1) < 1e-5);
				is_selected_object_orbited  = (length(delta2) < 1e-5);

#ifdef CAMERA_DEBUG_PRINT
				//std::cout << "   ~ delta1 : " << length(delta1) << std::endl;
				//std::cout << "   ~ delta2 : " << length(delta2) << std::endl;
#endif
			}

			////std::cout << "  [~] in_frustum_focused : " << in_frustum_focused << std::endl;
			////std::cout << "  [~] in_frustum_old_orbit_center : " << in_frustum<< std::endl;

			//bool jump_above_trsh = (fabsf(m_camera_distance - old_m_camera_distance) > fabsf(old_m_camera_distance));
			bool jump_above_trsh = (fabsf(m_camera_distance) > fabsf(old_m_camera_distance));  
#ifdef CAMERA_DEBUG_PRINT
			//std::cout << "  [~] jump_above_treshold : " << jump_above_trsh << std::endl;
			//std::cout << "  [~] in_frustum_focused : " << in_frustum_focused << std::endl;
			//std::cout << "  [~] in_frustum_selected : " << in_frustum_selected << std::endl;
			//std::cout << "  [~] was_in_frustum_selected : " << was_in_frustum_selected << std::endl;
			//std::cout << "  [~] was_selected_object_orbited : " << was_selected_object_orbited << std::endl;
			//std::cout << "  [~] is_selected_object_orbited : " << is_selected_object_orbited << std::endl;
			//std::cout << "  [~] zoom_in : " << zoom_in << std::endl;
#endif
			if ( zoom_in && jump_above_trsh 
				&& is_focused
				&& !in_frustum_focused
				//&& false
				)
			{
#ifdef CAMERA_DEBUG_PRINT
				//std::cout << "  [ ZOOM - IN ] -> Jumped too much! Need to re-adjust Camera!" << std::endl;
#endif
				float adjusted_camera_distance      = old_m_camera_distance * 1.f;
				optix::float3 w = (m_camera_orbit_center - m_cameraPosition);
				float len = length(w);
				w /= len;

				optix::float3 adjusted_orbit_center = m_cameraPosition + w * adjusted_camera_distance;
				setOrbitCenter(adjusted_orbit_center, true);
			}
			else if ( zoom_out && jump_above_trsh && !in_frustum_selected 
					 && is_selected 
					 && was_selected_object_orbited
					 )
			{
#ifdef CAMERA_DEBUG_PRINT
				//std::cout << "  [ ZOOM - OUT ] -> Jumped too much! Need to re-adjust Camera!" << std::endl;
#endif
				float adjusted_camera_distance = old_m_camera_distance * 1.f;
				optix::float3 w = (m_camera_orbit_center - m_cameraPosition);
				float len = length(w);
				w /= len;

				optix::float3 adjusted_orbit_center = m_cameraPosition + w * adjusted_camera_distance;
				setOrbitCenter(adjusted_orbit_center, true);
			}

#ifdef CAMERA_DEBUG_PRINT
			//std::cout << "  - - - - - - - - - - - - - - - - - - - -" << std::endl;
#endif

		}
	}

	

}


void PinholeCamera::setChanged(bool state)
{
	m_changed = state;
}
bool PinholeCamera::setDelta(int x, int y)
{
	if (m_baseX != x || m_baseY != y)
	{
		m_dx = x - m_baseX;
		m_dy = y - m_baseY;

		m_baseX = x;
		m_baseY = y;

		m_changed = true; // m_changed is only reset when asking for the frustum.
		return true; // There is a delta.
	}
	return false;
}
void PinholeCamera::setSpeedRatio(float f)
{
	m_speedRatio = f;
	if (m_speedRatio < 0.01f)
	{
		m_speedRatio = 0.01f;
	}
	else if (100.0f < m_speedRatio)
	{
		m_speedRatio = 100.0f;
	}
}


void PinholeCamera::setCameraPos(optix::float3 pos)
{
	m_cameraPosition = pos;
}
void PinholeCamera::setCameraPos_Deprecated(optix::float3 pos)
{
	m_cameraPosition = pos;
}

void PinholeCamera::setOrbitDistance(float dist)
{
	m_camera_distance = dist; 
}
void PinholeCamera::setDistance(float dist)
{

	m_distance = dist;
}
void PinholeCamera::setPhi(float phi)
{
	m_phi = phi;
}
void PinholeCamera::setTheta(float theta)
{
	m_theta = theta;
}
void PinholeCamera::calculatePhiTheta()
{

}
void PinholeCamera::getCameraBaseCoord(optix::float3 target, optix::float3 up, optix::float3& u, optix::float3 &v, optix::float3 &w)
{

	optix::float3 orbit_dir = normalize(m_camera_position - target);
	//optix::float3 up = optix::make_float3(0, 1, 0);
	optix::float3 new_orbit_dir = orbit_dir;
	optix::float3 right = normalize(cross(new_orbit_dir, up));
	up = normalize(cross(right, new_orbit_dir));

	w = -normalize(orbit_dir);
	u = normalize(cross(orbit_dir, up));
	v = normalize(cross(orbit_dir, m_camera_u));
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

void PinholeCamera::Update(Io_Mouse mouse_data)
{
	if (m_interp_active)
		return;
	////std::cout << "\n - PinholeCamera::Update( Io_Mouse ):" << std::endl;
	////std::cout << " - mouse_delta : [ " << mouse_data.dx << " , " << mouse_data.dy << " ] " << std::endl;
	SceneObject* selected_object = Mediator::RequestSelectedObject(); //int selected_id = selected_object == nullptr ? -1 : selected_object->getId();
	SceneObject* focused_object  = Mediator::RequestFocusedObject();   //int focused_id  = focused_object  == nullptr ? -1 : focused_object->getId();
	//bool same_obj = ((selected_id == focused_id) && selected_id != -1) ? true : false;
	bool is_selected = (selected_object == nullptr) ? false : true;
	bool is_focused  = (focused_object  == nullptr) ? false : true;
	bool action_on_selected = mouse_data.m2_down_on_selected_object;
	bool action_on_focused  = mouse_data.m2_down_on_focused_object;
	

	// orbit_center -> in frustum ?
	bool in_frustum           = !Utilities::frustumCull(m_camera_orbit_center, ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id));
	bool in_frustum_panning   = is_selected ? !Utilities::frustumCull( selected_object->getCentroid_Transformed(true) , ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id)) : false;
	m_was_in_frustum          = in_frustum;
	m_was_in_frustum_selected = in_frustum_panning;
	

	// if not in frustum and state == none -> change orbit center
	float jumping_cam_center_offset = 0.75f; // 0.75f
	/*
	if (m_state == CAM_NONE
		&& m_free_roam_state
		&& fabsf(m_camera_distance) > 200.0f)
	{
		
		//std::cout << "\n - was_in_frustum : " << m_was_in_frustum << std::endl;
		//std::cout << " - was_in_frustum_selected : " << m_was_in_frustum_selected << std::endl;
		//std::cout << " - is_in_frustum : " << in_frustum << std::endl;
		//std::cout << " - in_frustum_panning : " << in_frustum_panning << std::endl;
		//std::cout << " - re-adjusting camera distance!" << std::endl;

		float adjusted_camera_distance = 200.0f;//m_camera_distance_old * 0.5f;
		optix::float3 w = (m_camera_orbit_center - m_cameraPosition);
		float len = length(w);
		w /= len;
		optix::float3 adjusted_orbit_center = m_cameraPosition + w * adjusted_camera_distance;
		setOrbitCenter(adjusted_orbit_center, true, false, false);
	}
	*/
	if      ( !in_frustum && m_state == CAM_NONE )
	{
		//std::cout << " - b " << std::endl;
		//system("pause");

		optix::float3 new_orbit_center = m_cameraPosition + normalize(m_camera_w) * m_camera_distance * jumping_cam_center_offset;
		setOrbitCenter( new_orbit_center, true );

		
		if ( fabsf(m_camera_distance) > 200.0f 
			//fabsf(m_camera_distance) > fabsf(m_camera_distance_old)
			&& m_free_roam_state
			)
		{
			// re - adjust camera //
			float adjusted_camera_distance = 200.0f;//m_camera_distance_old * 0.5f;
			optix::float3 w = (m_camera_orbit_center - m_cameraPosition);
			float len = length(w);
			w /= len;
			optix::float3 adjusted_orbit_center = m_cameraPosition + w * adjusted_camera_distance;
			setOrbitCenter(adjusted_orbit_center, true, false, false);
		}

#ifdef CAMERA_UPDATE_DEBUG_PRINT
		//std::cout << "\n - PinholeCamera::Update( Io_Mouse ):" << std::endl;
		//std::cout << "  - m_camera_distance : " << m_camera_distance << std::endl;
		//std::cout << "  - m_camera_distance_old : " << m_camera_distance_old << std::endl;
		//std::cout << "  - free_roam_state : " << m_free_roam_state << std::endl;
		//std::cout << "  - [a] " << std::endl;
#endif

	}
	else if (in_frustum_panning && m_state == CAM_NONE && !m_orbited_on_free_roam && m_free_roam_state)
	{
		if ( m_should_retarget_to_center )
		{
			setOrbitCenter( selected_object->getCentroid_Transformed(true), false, true );


#ifdef CAMERA_UPDATE_DEBUG_PRINT
			//std::cout << "\n - PinholeCamera::Update( Io_Mouse ):" << std::endl;
			//std::cout << "  - m_camera_distance : " << m_camera_distance << std::endl;
			//std::cout << "  - m_camera_distance_old : " << m_camera_distance_old << std::endl;
			//std::cout << "  - free_roam_state : " << m_free_roam_state << std::endl;
			//std::cout << "  - [b] " << std::endl;
#endif

		}
		else if( m_setting_orbit_center_to_object )
		{

			optix::float3 new_orbit_center = m_cameraPosition + normalize(m_camera_w) * m_camera_distance * jumping_cam_center_offset;
			setOrbitCenter( new_orbit_center, true );
			m_setting_orbit_center_to_object = false;


#ifdef CAMERA_UPDATE_DEBUG_PRINT
			//std::cout << "\n - PinholeCamera::Update( Io_Mouse ):" << std::endl;
			//std::cout << "  - m_camera_distance : " << m_camera_distance << std::endl;
			//std::cout << "  - m_camera_distance_old : " << m_camera_distance_old << std::endl;
			//std::cout << "  - free_roam_state : " << m_free_roam_state << std::endl;
			//std::cout << "  - [c] " << std::endl;
#endif

		}

	}
	
	if ( m_state == CAM_NONE || m_re_update_panning_state )
	{

		// pan on focused object
		if ( action_on_focused )
		{
			m_panning_center = focused_object->getCentroid_Transformed(true); //focused_object->getTranslationAttributes();
			////std::cout << " - panning : focused object!" << std::endl;
		}

		// in frustum and state == none
		// case : selected object and not picked focused
		// pan on selected object
		else if (in_frustum_panning && selected_object)
		{
			m_panning_center = selected_object->getCentroid_Transformed(true); //selected_object->getTranslationAttributes();
		}

		// pan on scene origin
		else if (!Utilities::frustumCull( ZERO_3f , ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id)))
		{
			m_panning_center = ZERO_3f;
		}

		// pan on orbiting center
		else
		{
			m_panning_center = m_camera_orbit_center;
		}

		if (m_state == CAM_NONE)
			m_zooming_stable = false;

	}


	switch ( m_state )
	{

	case CAM_NONE:

		if (mouse_data.m0_down) // LMB down event?
		{
			setBaseCoordinates(mouse_data.x, mouse_data.y);
			m_state = Camera_State::CAM_ORBIT;
		}
		else if (mouse_data.m1_down) // RMB down event?
		{
			setBaseCoordinates(mouse_data.x, mouse_data.y);
			m_state = Camera_State::CAM_DOLLY;
		}
		else if (mouse_data.m2_down) // MMB down event?
		{
			setBaseCoordinates(mouse_data.x, mouse_data.y);
			m_state = Camera_State::CAM_PAN;
		}
		else if (mouse_data.m_wheel != 0.0f) // Mouse wheel zoom.
		{
			zoom(mouse_data.m_wheel);
		}

		break;

	case CAM_ORBIT:
		if (mouse_data.m0_released) // LMB released? End of orbit mode.
		{
			m_state = Camera_State::CAM_NONE;
		}
		else
		{
			orbit(mouse_data.x, mouse_data.y);
		}
		break;

	case CAM_DOLLY:
		if (mouse_data.m1_released) // RMB released? End of dolly mode.
		{
			m_state = Camera_State::CAM_NONE;
		}
		else
		{
			//dolly(mouse_data.x, mouse_data.y);
		}
		break;

	case CAM_PAN:
		if (mouse_data.m2_released) // MMB released? End of pan mode.
		{
			m_state = Camera_State::CAM_NONE;
		}
		else
		{
			pan(mouse_data.x, mouse_data.y , mouse_data.prev_x, mouse_data.prev_y);
		}
		break;
	}


	
}
void PinholeCamera::Update_Interp_State()
{
	float speed = (m_interp_look_at == LOOK_LEFT || m_interp_look_at == LOOK_RIGHT
				   || m_interp_look_at == LOOK_TOP || m_interp_look_at == LOOK_BOTTOM
				   || m_interp_look_at == LOOK_FRONT || m_interp_look_at == LOOK_BACK) ? 4.0f : 2.0f;
	if (m_interp_flip_Y_axis_active || m_interp_align_axis_active)
		speed = 4.f;

	float time = m_interp_timer.getTime();
	float dt = time - m_interp_t_start;
	m_interp_dt += speed * dt;
	float t = m_interp_dt;

	if (t > 1.0f) t = 1.0f;

	m_interp_t_start = time;
	optix::float3 interp_pos;
	float M = 1.f;

	if      (m_interp_flip_Y_axis_active)
	{
		if (t <= M)
		{
			optix::Matrix4x4 rot = optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(t * 180.0f), GLOBAL_AXIS[2]);
			optix::Matrix4x4 mat = m_interp_FROM_matrix * rot;

			m_camera_u = optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[0], 0.0f));
			m_camera_v = optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[1], 0.0f));
			m_camera_w = optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[2], 0.0f));

			m_camera_u = normalize(m_camera_u);
			m_camera_v = normalize(m_camera_v);
			m_camera_w = normalize(m_camera_w);

			setOrbitCenter(m_interp_orbit_TO, m_camera_u, m_camera_v, m_camera_w);
		}

		if (t == 1.0f)
		{

			//interp_pos = m_interp_TO;
			//m_cameraPosition = interp_pos;

			m_interp_timer.reset();
			m_interp_timer.stop();
			m_state = CAM_NONE;
			m_interp_active = false;
			m_interp_flip_Y_axis_active = false;

		}
	}
	else if (m_interp_align_axis_active)
	{

		if (t <= M)
		{
			optix::Matrix4x4 rot = optix::Matrix4x4::rotate((t * m_interp_align_axis_delta), GLOBAL_AXIS[1]);
			optix::Matrix4x4 mat = rot * m_interp_FROM_matrix;

			m_camera_u = optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[0], 0.0f));
			m_camera_v = optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[1], 0.0f));
			m_camera_w = optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[2], 0.0f));

			m_camera_u = normalize(m_camera_u);
			m_camera_v = normalize(m_camera_v);
			m_camera_w = normalize(m_camera_w);

			setOrbitCenter(m_interp_orbit_TO, m_camera_u, m_camera_v, m_camera_w);
		}

		if (t == 1.0f)
		{
			m_interp_timer.reset();
			m_interp_timer.stop();
			m_state = CAM_NONE;
			m_interp_active = false;
			m_interp_align_axis_active = false;
		}

	}
	else
	{
		if (t < M)
		{
			interp_pos = m_interp_FROM * (M - t) + m_interp_TO * t;
			m_cameraPosition = interp_pos;
			m_camera_w = normalize((m_interp_axis_FROM[2] * (M - t) + m_interp_axis_TO[2] * t));

			if (m_interp_look_at == LOOK_TOP || m_interp_look_at == LOOK_BOTTOM)
			{
				m_camera_u = cross(m_camera_w, optix::make_float3(0.0f, 1.0f, 0.0f));
				if (dot(m_camera_u, m_interp_axis_FROM[0]) < 0.0f)// && dot(m_camera_v, m_interp_axis_FROM[1]) < 0.0f)
					m_camera_u *= -1.0f;
				m_camera_v = cross(m_camera_u, m_camera_w);

			}
			else if (m_interp_look_at == LOOK_LEFT || m_interp_look_at == LOOK_RIGHT)
			{

				m_camera_u = cross(m_camera_w, optix::make_float3(0.0f, 1.0f, 0.0f));
				m_camera_v = cross(m_camera_u, m_camera_w);

				float d1, d2;
				float d3, d4;
				if (m_interp_look_at == LOOK_LEFT)
				{
					d1 = dot(m_interp_axis_FROM[2], GLOBAL_AXIS[0]);
					d2 = dot(m_interp_axis_FROM[1], -GLOBAL_AXIS[1]);
					if (d1 > 0 && d2 > 0)
					{
						m_camera_u *= -1.0f;
						m_camera_v *= -1.0f;
					}
				}
				else
				{
					d1 = dot(m_interp_axis_FROM[2], GLOBAL_AXIS[0]);
					d2 = dot(m_interp_axis_FROM[1], -GLOBAL_AXIS[1]);

					d3 = dot(m_interp_axis_FROM[2], -GLOBAL_AXIS[0]);
					d4 = dot(m_interp_axis_FROM[0], GLOBAL_AXIS[2]);

					if (d1 > 0 && d2 > 0 || (d1 <= 0.0f && d2 > 0 && d4 > 0.0f))
					{

						m_camera_u *= -1.0f;
						m_camera_v *= -1.0f;
					}
					else
					{

					}
				}


			}
			else if (m_interp_look_at == LOOK_FRONT || m_interp_look_at == LOOK_BACK)
			{

				m_camera_u = cross(m_camera_w, optix::make_float3(0.0f, 1.0f, 0.0f));
				m_camera_v = cross(m_camera_u, m_camera_w);

				float d1, d2;
				float d3, d4;
				if (m_interp_look_at == LOOK_FRONT)
				{
					d1 = dot(m_interp_axis_FROM[2], GLOBAL_AXIS[0]);
					d2 = dot(m_interp_axis_FROM[1], -GLOBAL_AXIS[1]);
					if (d1 > 0 && d2 > 0)
					{
						m_camera_u *= -1.0f;
						m_camera_v *= -1.0f;
					}
				}
				else
				{
					d1 = dot(m_interp_axis_FROM[2], GLOBAL_AXIS[0]);
					d2 = dot(m_interp_axis_FROM[1], -GLOBAL_AXIS[1]);

					d3 = dot(m_interp_axis_FROM[2], -GLOBAL_AXIS[0]);
					d4 = dot(m_interp_axis_FROM[0], GLOBAL_AXIS[2]);

					if (d1 > 0 && d2 > 0 || (d1 <= 0.0f && d2 > 0 && d4 > 0.0f))
					{

						m_camera_u *= -1.0f;
						m_camera_v *= -1.0f;
					}
					else
					{

					}
				}


			}


			m_camera_u = normalize(m_camera_u);
			m_camera_v = normalize(m_camera_v);
			m_camera_w = normalize(m_camera_w);
			setOrbitCenter(m_interp_orbit_TO, m_camera_u, m_camera_v, m_camera_w);
		}
		else
		{
			interp_pos = m_interp_TO;
			m_cameraPosition = interp_pos;

			m_interp_timer.reset();
			m_interp_timer.stop();
			m_state = CAM_NONE;
			m_interp_active = false;

			if (m_interp_look_at == LOOK_LEFT || m_interp_look_at == LOOK_RIGHT || m_interp_look_at == LOOK_FRONT || m_interp_look_at == LOOK_BACK)
			{

				if (dot(m_camera_v, GLOBAL_AXIS[1]) < 0.0f)
					set_interp_flip_Y_Axis();

			}
			else
			{

#ifdef align_axis_after_BOT_TOP_cam_interp
				float angle_d = acosf(dot(normalize(m_camera_u), GLOBAL_AXIS[0]));
				if (angle_d != 0)
				{
					//set_interp_align_Axis(0, GLOBAL_AXIS[0], GLOBAL_AXIS[1]);
				}
#endif
			}


		}

	}


	m_changed = true;
	ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id).markDirty();

	optix::float3 p, u, v, w;
	getFrustum(p, u, v, w, false);
}
bool PinholeCamera::getFrustum(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w, bool reset)
{
	return getFrustum_Test(pos, u, v, w, reset);
}
void PinholeCamera::getFrustum()
{
	getFrustum_Test();
}
bool PinholeCamera::getFrustum_Test(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w, bool reset)
{


	bool changed = m_changed;

#define Camera_Model_3dsMax_Test
#ifdef Camera_Model_3dsMax_Test

	

	if ( changed  && !m_interp_active )
	{
		////std::cout << "\n - getFrustum( ... )( default ):" << std::endl;
		float speed = 2.0f * M_PIf;
		optix::Matrix4x4 rotY;
		optix::Matrix4x4 rotX;

		//
		rotY = optix::Matrix4x4::rotate(m_orbit_dx * speed * 0.8f, GLOBAL_AXIS[1]);
		rotX = optix::Matrix4x4::rotate(m_orbit_dy * speed, GLOBAL_AXIS[0]);
		m_camera_matrix = rotY * m_camera_matrix;
		m_camera_matrix = m_camera_matrix * rotX;

		m_orbit_matrix = rotY * m_orbit_matrix;
		m_orbit_matrix = m_camera_matrix * rotX * m_camera_matrix.inverse() * m_orbit_matrix;
		optix::float3 orbit_normal = optix::make_float3(m_orbit_matrix * optix::make_float4(GLOBAL_AXIS[2], 0.0f));

		// extract basis //
		{
			m_camera_u = optix::make_float3(m_camera_matrix * optix::make_float4(GLOBAL_AXIS[0], 0.0f));// + du;
			m_camera_v = optix::make_float3(m_camera_matrix * optix::make_float4(GLOBAL_AXIS[1], 0.0f));// + dv;
			m_camera_w = optix::make_float3(m_camera_matrix * optix::make_float4(GLOBAL_AXIS[2], 0.0f));// + dw;

			m_camera_u = normalize(m_camera_u);
			m_camera_v = normalize(m_camera_v);
			m_camera_w = normalize(m_camera_w);
		}

		optix::Matrix4x4 mat = optix::Matrix4x4::translate(m_camera_orbit_center) * m_orbit_matrix * optix::Matrix4x4::translate(m_relative_pos);
		//m_cameraPosition = optix::make_float3(mat * optix::make_float4(ZERO_3f, 1.0f));
		m_cameraPosition = m_position_offset + m_camera_orbit_center - orbit_normal * m_camera_distance;



		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		// set perspective projection
		float tanFov = m_near * tanf((m_fov * 0.5f) * M_PIf / 180.0f);
		u = m_camera_u * m_aspect * tanFov;
		v = m_camera_v * tanFov;
		w = m_camera_w * m_near;
		pos = m_cameraPosition;

		m_cameraU = u;
		m_cameraV = v;
		m_cameraW = w;

		////std::cout << " - u : [ " << m_camera_u.x << " , " << m_camera_u.y << " , " << m_camera_u.z << " ] " << std::endl;
		////std::cout << " - v : [ " << m_camera_v.x << " , " << m_camera_v.y << " , " << m_camera_v.z << " ] " << std::endl;
		////std::cout << " - w : [ " << m_camera_w.x << " , " << m_camera_w.y << " , " << m_camera_w.z << " ] " << std::endl;


		ViewportManager::GetInstance().getActiveViewport().markDirty();
	}
	else if ( changed && m_interp_active )
	{
		////std::cout << "\n - getFrustum( ... )( interp ):" << std::endl;

		float tanFov = m_near * tanf((m_fov * 0.5f) * M_PIf / 180.0f);
		u = m_camera_u * m_aspect * tanFov;
		v = m_camera_v * tanFov;
		w = m_camera_w * m_near;
		pos = m_cameraPosition;

		m_cameraU = u;
		m_cameraV = v;
		m_cameraW = w;

		////std::cout << " - u : [ " << u.x << " , " << u.y << " , " << u.z << " ] " << std::endl;
		////std::cout << " - v : [ " << v.x << " , " << v.y << " , " << v.z << " ] " << std::endl;
		////std::cout << " - w : [ " << w.x << " , " << w.y << " , " << w.z << " ] " << std::endl;
	}

#endif


	if ( reset )
		m_changed = false;


	m_orbit_dx = 0.0f;
	m_orbit_dy = 0.0f;

	return changed;
}
void PinholeCamera::getFrustum_Test()
{

	bool changed = m_changed;

#define Camera_Model_3dsMax_Test
#ifdef Camera_Model_3dsMax_Test

	if      ( changed && !m_interp_active )
	{
		////std::cout << "\n - getFrustum( default ):" << std::endl;
		float speed = 2.0f * M_PIf;


		optix::Matrix4x4 rotY;
		optix::Matrix4x4 rotX;

		//
		rotY = optix::Matrix4x4::rotate(m_orbit_dx * speed, GLOBAL_AXIS[1]);
		rotX = optix::Matrix4x4::rotate(m_orbit_dy * speed, GLOBAL_AXIS[0]);
		m_camera_matrix = rotY * m_camera_matrix;
		m_camera_matrix = m_camera_matrix * rotX;

		m_orbit_matrix = rotY * m_orbit_matrix;
		m_orbit_matrix = m_camera_matrix * rotX * m_camera_matrix.inverse() * m_orbit_matrix;
		optix::float3 orbit_normal = optix::make_float3(m_orbit_matrix * optix::make_float4(GLOBAL_AXIS[2], 0.0f));

		// extract basis //
		{
			m_camera_u = optix::make_float3(m_camera_matrix * optix::make_float4(GLOBAL_AXIS[0], 0.0f));// + du;
			m_camera_v = optix::make_float3(m_camera_matrix * optix::make_float4(GLOBAL_AXIS[1], 0.0f));// + dv;
			m_camera_w = optix::make_float3(m_camera_matrix * optix::make_float4(GLOBAL_AXIS[2], 0.0f));// + dw;

			m_camera_u = normalize(m_camera_u);
			m_camera_v = normalize(m_camera_v);
			m_camera_w = normalize(m_camera_w);
		}
		m_cameraPosition = m_camera_orbit_center - orbit_normal * m_camera_distance;

		////std::cout << " - u : [ " << m_camera_u.x << " , " << m_camera_u.y << " , " << m_camera_u.z << " ] " << std::endl;
		////std::cout << " - v : [ " << m_camera_v.x << " , " << m_camera_v.y << " , " << m_camera_v.z << " ] " << std::endl;
		////std::cout << " - w : [ " << m_camera_w.x << " , " << m_camera_w.y << " , " << m_camera_w.z << " ] " << std::endl;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


	}
	else if ( changed && m_interp_active )
	{
		////std::cout << "\n - getFrustum( interp ):" << std::endl;


		////std::cout << " - interp_TO_u : [ " << m_interp_axis_TO[0].x << " , " << m_interp_axis_TO[0].y << " , " << m_interp_axis_TO[0].z << " ] " << std::endl;
		////std::cout << " - interp_TO_v : [ " << m_interp_axis_TO[1].x << " , " << m_interp_axis_TO[1].y << " , " << m_interp_axis_TO[1].z << " ] " << std::endl;
		////std::cout << " - interp_TO_w : [ " << m_interp_axis_TO[2].x << " , " << m_interp_axis_TO[2].y << " , " << m_interp_axis_TO[2].z << " ] " << std::endl;

		////std::cout << " - u : [ " << m_camera_u.x << " , " << m_camera_u.y << " , " << m_camera_u.z << " ] " << std::endl;
		////std::cout << " - v : [ " << m_camera_v.x << " , " << m_camera_v.y << " , " << m_camera_v.z << " ] " << std::endl;
		////std::cout << " - w : [ " << m_camera_w.x << " , " << m_camera_w.y << " , " << m_camera_w.z << " ] " << std::endl;
	}

#endif






}
void PinholeCamera::getFrustum_Deprecated()
{


#define CUSTOM_CAMERA_MODEL
#ifdef CUSTOM_CAMERA_MODEL


	{

		float speed = 2.0f * M_PIf;
		optix::float3 dir = normalize(m_camera_position - m_camera_orbit_center);
		optix::float3 up = optix::make_float3(0, 1, 0);
		optix::Matrix4x4 rotY = optix::Matrix4x4::rotate(m_camera_movement.x * speed, up);
		optix::float3 new_dir = optix::make_float3(rotY * optix::make_float4(dir, 0.0f));
		optix::float3 right = normalize(cross(new_dir, up));
		optix::Matrix4x4 rotX = optix::Matrix4x4::rotate(m_camera_movement.y * speed / 1.4f, right);
		dir = optix::make_float3(rotX * rotY * optix::make_float4(dir, 0.0f));
		up = normalize(cross(right, dir));
		m_camera_temp_position = m_camera_orbit_center + dir * m_camera_distance;

		m_camera_w = -normalize(m_camera_temp_position - m_camera_orbit_center);
		m_camera_u = normalize(cross(m_camera_w, up));
		m_camera_v = normalize(cross(m_camera_w, m_camera_u));

		float dot_w = optix::dot(prev_w, m_camera_w);
		float dot_u = optix::dot(prev_u, m_camera_u);
		float dot_v = optix::dot(prev_v, m_camera_v);
		diff_u = length(m_camera_u - prev_u * sign);
		diff_v = length(m_camera_v - prev_v * sign);

		////std::cout << "\n\n dot_u : " << dot_u << std::endl;
		////std::cout << " dot_v : " << dot_v << std::endl;
		////std::cout << " dot_w : " << dot_w << std::endl;
		////std::cout << " diff_u : " << (diff_u) << std::endl;
		////std::cout << " diff_v : " << (diff_v) << std::endl;

		bool is_inv_u = (fabsf(dot_u + 1.0f) < 0.3f);
		bool is_inv_v = (fabsf(dot_v + 1.0f) < 0.3f);
		////std::cout << " is_inv_u : " << is_inv_u << std::endl;
		////std::cout << " is_inv_v : " << is_inv_v << std::endl;

		if (
			1
			//1.0f - fabsf(dot_w) < 0.01f 
			&& is_inv_u
			&& is_inv_v
			)
		{
			////std::cout << " 1 " << std::endl;
			m_camera_u *= -1.0f;
			m_camera_v *= -1.0f;
			is_inv = true;
			sign = -1.0f;
		}
		else
		{
			////std::cout << " 2 " << std::endl;
			is_inv = false;
			sign = 1.0f;
		}



		float tanFov = m_near * tanf((m_fov * 0.5f) * M_PIf / 180.0f);
		optix::float3 u = m_camera_u * m_aspect * tanFov;
		optix::float3 v = -m_camera_v * tanFov;
		optix::float3 w = m_camera_w * m_near;
		optix::float3 pos = m_camera_temp_position;


		m_cameraPosition = pos;
		m_distance = m_camera_distance;
		m_cameraU = u;
		m_cameraV = v;
		m_cameraW = w;

		prev_u = m_camera_u;
		prev_v = m_camera_v;
		prev_w = m_camera_w;
	}

#endif





}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

void PinholeCamera::Target( SceneObject* object )
{
	optix::float3 pos      = object->getTranslationAttributes();
	optix::float3 centroid = object->getCentroid_Transformed();
	optix::float3 bbox_min;// = object->getBBoxMin();
	optix::float3 bbox_max;// = object->getBBoxMax();
	optix::float3 * bbox = object->getBBOX_Transformed();

	bbox_min = bbox[0];
	bbox_max = bbox[1];
	
	float dist = length(  bbox_max - bbox_min ) * 2.0f;

	m_cameraPosition = centroid - normalize(m_camera_w) * dist;
	setCenter( centroid );
	setOrbitCenter( centroid );
	
}
void PinholeCamera::Target(optix::float3 center, float distance)
{
	optix::float3 centroid = center;
	float dist = distance;


	m_cameraPosition = centroid - normalize(m_camera_w) * dist;
	setCenter(centroid);
	setOrbitCenter(centroid);
}

void PinholeCamera::resetOrbitCenter()
{
	////std::cout << "\n - resetOrbitCenter():" << std::endl;
	setOrbitCenter(m_camera_orbit_center);
}
void PinholeCamera::setOrbitCenter(optix::float3 orbit_center, bool enable_free_roam, bool to_object, bool set_old_vars )
{
	if (set_old_vars)
	{
		m_camera_orbit_center_old = m_camera_orbit_center;
		m_camera_distance_old = m_camera_distance;
	}

	////std::cout << "\n - PinholeCamera::setOrbitCenter( " << orbit_center.x << " , " << orbit_center.y << " , " << orbit_center.z << " ) " << std::endl;
	m_camera_orbit_center = orbit_center;
	m_orbit_dir           = normalize(m_cameraPosition - orbit_center);
	m_camera_distance     = optix::length(m_camera_orbit_center - m_cameraPosition);
	m_camera_movement     = optix::make_float2(0.0f);
	m_orbit_matrix        = optix::Matrix4x4::identity();
	m_relative_pos        = m_cameraPosition - m_camera_orbit_center;
	
	m_orbit_w = normalize(orbit_center - m_cameraPosition);
	m_orbit_u = normalize(cross(m_orbit_w, GLOBAL_AXIS[1]));
	m_orbit_v = normalize(cross(m_orbit_u, m_orbit_w));

	m_orbit_matrix = optix::Matrix4x4::fromBasis(m_orbit_u, m_orbit_v, m_orbit_w, ZERO_3f);
	m_relative_pos = optix::make_float3((optix::Matrix4x4::translate(m_camera_orbit_center) * m_orbit_matrix).inverse() * optix::make_float4(m_cameraPosition, 1.0f));
	
	if ( to_object )
	{
		m_setting_orbit_center_to_object = true;
		bool in_frustum = !Utilities::frustumCull( m_camera_orbit_center, ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id) );
		if (!in_frustum)
			m_should_retarget_to_center = false;
		else
			m_should_retarget_to_center = true;
	}
	
	if (enable_free_roam)
		m_free_roam_state = true;
	else
	{
		m_free_roam_state = false;
		m_orbited_on_free_roam = false;
	}

}
void PinholeCamera::setOrbitCenter(optix::float3 orbit_center, optix::float3 u, optix::float3 v, optix::float3 w, bool set_old_vars )
{

	if (set_old_vars)
	{
		m_camera_orbit_center_old = m_camera_orbit_center;
		m_camera_distance_old = m_camera_distance;
	}

	m_camera_orbit_center = orbit_center;
	m_orbit_dir = normalize(m_cameraPosition - orbit_center);
	m_camera_distance = optix::length(m_camera_orbit_center - m_cameraPosition);
	m_camera_movement = optix::make_float2(0.0f);
	m_orbit_matrix = optix::Matrix4x4::identity();
	m_relative_pos = m_cameraPosition - m_camera_orbit_center;

	m_camera_matrix = optix::Matrix4x4::fromBasis(u, v, w, ZERO_3f);
	m_orbit_matrix  = optix::Matrix4x4::fromBasis(u, v, w, ZERO_3f);
	
	//m_orbit_w = normalize(orbit_center - m_cameraPosition);
	//m_orbit_u = normalize(cross(m_orbit_w, GLOBAL_AXIS[1]));
	//m_orbit_v = normalize(cross(m_orbit_u, m_orbit_w));
	//m_orbit_matrix = optix::Matrix4x4::fromBasis(m_orbit_u, m_orbit_v, m_orbit_w, ZERO_3f);

	
	{
		m_free_roam_state      = false;
		m_orbited_on_free_roam = false;
	}
}
void PinholeCamera::setOrbitCenter_Deprecated(optix::float3 orbit_center)
{
	m_camera_orbit_center = orbit_center;
	m_camera_focus_delta = m_camera_focus_center - m_camera_orbit_center;
	m_camera_position = m_camera_temp_position;
	m_camera_distance = optix::length(m_camera_orbit_center - m_camera_position);
	m_camera_movement = optix::make_float2(0.0f);

	m_changed = true;
}
void PinholeCamera::setCenter(optix::float3 orbit_center)
{
	
	m_camera_orbit_center = orbit_center;
	m_orbit_dir = normalize(m_cameraPosition - orbit_center);
	m_camera_distance = optix::length(m_camera_orbit_center - m_cameraPosition);
	m_camera_movement = optix::make_float2(0.0f);
	m_orbit_matrix = optix::Matrix4x4::identity();
	m_relative_pos = m_cameraPosition - m_camera_orbit_center;
	m_changed = true;

	m_orbit_w = normalize(orbit_center - m_cameraPosition);
	m_orbit_u = normalize(cross(m_orbit_w, GLOBAL_AXIS[1]));
	m_orbit_v = normalize(cross(m_orbit_u, m_orbit_w));

	m_orbit_matrix = optix::Matrix4x4::fromBasis(m_orbit_u, m_orbit_v, m_orbit_w, ZERO_3f);

	du = m_camera_u - m_orbit_u;
	dv = m_camera_v - m_orbit_v;
	dw = m_camera_w - m_orbit_w;

}
void PinholeCamera::setCenter_Deprecated(optix::float3 center)
{

	m_center = center;


	m_camera_position = m_camera_temp_position;
	m_camera_movement = optix::make_float2(0.0f);
	m_camera_orbit_center = center;
	m_camera_focus_center = center;
	m_camera_distance = optix::length(m_camera_position - m_camera_orbit_center);
	m_changed = true;


}



bool PinholeCamera::isChanged(bool reset)
{
	if (reset)
	{
		bool return_state = m_changed;
		m_changed = false;
		return return_state;
	}
	else
		return m_changed;
}
void PinholeCamera::reset()
{
	m_changed = false;
}
float PinholeCamera::getAspectRatio() const
{
	return m_aspect;
}
optix::float3 PinholeCamera::getFocusCenter()
{
	return m_camera_focus_center;
}
optix::float3 PinholeCamera::getOrbitCenter()
{
	return m_camera_orbit_center;
}
optix::float3 PinholeCamera::getCameraPos()
{
	return m_cameraPosition;
}
float PinholeCamera::getDistance()
{
	return m_distance;
}
float PinholeCamera::getOrbitDistance()
{
	return m_camera_distance;
}
float PinholeCamera::getPhi()
{
	return m_phi;
}
float PinholeCamera::getTheta()
{
	return m_theta;
}
optix::float3 PinholeCamera::getU()
{
	return m_cameraU;
}
optix::float3 PinholeCamera::getV()
{
	return m_cameraV;
}
optix::float3 PinholeCamera::getW()
{
	return m_cameraW;
}
void PinholeCamera::setState(Camera_State state)
{
	m_state = state;
}
Camera_State PinholeCamera::getState()
{
	return m_state;
}
optix::Matrix4x4 PinholeCamera::getCameraMatrix()
{
	return optix::Matrix4x4::fromBasis(m_camera_u, m_camera_v, m_camera_w, m_cameraPosition);
}
float PinholeCamera::getDistanceFrom(const optix::float3& p)
{
	return optix::length(m_cameraPosition - p);
}

float PinholeCamera::getInvScaleFactor(const VIEWPORT& vp)
{

	return vp.scale_factor;

	float a = Mediator::RequestWindowSize(0) + Mediator::RequestWindowSize(1);
	VIEWPORT_PRESET_ preset = ViewportManager::GetInstance().getActiveViewportPreset();
	int c;


	if (preset == VIEWPORT_PRESET_::DEFAULT_PRESET)
	{
		c = 1;
	}
	else if (preset == VIEWPORT_PRESET_::FOUR_TO_ONE)
	{
		c = 4;
	}
	else if (preset == VIEWPORT_PRESET_::MAIN_3_RIGHT_PRESET
			 || preset == VIEWPORT_PRESET_::MAIN_3_LEFT_PRESET
			 || preset == VIEWPORT_PRESET_::MAIN_2_RIGHT_PRESET
			 )
	{
		c = 3;
	}

	float fact = 1.0f;

	if (c == 3)
	{
		if (vp.renderer == OPTIX_RENDERER)
		{
			fact = 2.0f / 2.5f;
		}
		else
			fact = 1.0f / 1.25f;
	}
	else if (c == 4)
		fact = 1 / 1.5f;

	float scale = a / (fact * 2.0f * (m_width + m_height)); // 2000.0f // 8a prepei na einai sta9ero gia opoiodhpote resolution wstoso
	////std::cout << " scale : " << scale << std::endl;
	if (scale < 1.f)
		scale = 1.f;
	else if (scale > 6.0f)
		scale = 6.0f;



	return scale;
}
float PinholeCamera::getInvScaleFactor()
{

	VIEWPORT vp = ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id);
	return vp.scale_factor;

	float a = Mediator::RequestWindowSize(0) + Mediator::RequestWindowSize(1);
	VIEWPORT_PRESET_ preset = ViewportManager::GetInstance().getActiveViewportPreset();
	int c;


	if (preset == VIEWPORT_PRESET_::DEFAULT_PRESET)
	{
		c = 1;
	}
	else if (preset == VIEWPORT_PRESET_::FOUR_TO_ONE)
	{
		c = 4;
	}
	else if (preset == VIEWPORT_PRESET_::MAIN_3_RIGHT_PRESET
			 || preset == VIEWPORT_PRESET_::MAIN_3_LEFT_PRESET
			 || preset == VIEWPORT_PRESET_::MAIN_2_RIGHT_PRESET
			 )
	{
		c = 3;
	}

	float fact = 1.0f;

	if (c == 3)
	{
		if (vp.renderer == OPTIX_RENDERER)
		{
			fact = 2.0f / 2.5f;
		}
		else
			fact = 1.0f / 1.25f;
	}
	else if (c == 4)
		fact = 1 / 1.5f;

	float scale = a / (fact * 2.0f * (m_width + m_height)); // 2000.0f // 8a prepei na einai sta9ero gia opoiodhpote resolution wstoso
	////std::cout << " scale : " << scale << std::endl;
	if (scale < 1.f)
		scale = 1.f;
	else if (scale > 6.0f)
		scale = 6.0f;



	return scale;
}

void PinholeCamera::setViewport(int id)
{
	vp_id = id;
}
int PinholeCamera::getViewportId()
{
	return vp_id;
}
void PinholeCamera::retargetingViewport()
{
	m_retargeting_viewport = true;
}


void PinholeCamera::set_interp_Origin(optix::float3 from)
{
	m_interp_FROM = from;
}
void PinholeCamera::set_interp_Target(optix::float3 to, bool activate)
{
	m_interp_TO = to;
	if (activate)
	{
		m_interp_FROM = m_cameraPosition;
		m_interp_active = true;
	}

}
void PinholeCamera::set_interp_Camera_State(enum Camera_Look_At look_at)
{
	if (m_interp_active || m_interp_flip_Y_axis_active)
		return;


	m_interp_look_at = look_at;
	m_interp_axis_FROM[0] = m_camera_u;
	m_interp_axis_FROM[1] = m_camera_v;
	m_interp_axis_FROM[2] = m_camera_w;

	m_interp_FROM     = m_cameraPosition;
	m_interp_orbit_TO = m_camera_orbit_center;


	switch (look_at)
	{

	    case LOOK_FRONT:
		{
			m_interp_TO = m_camera_orbit_center + m_camera_distance * GLOBAL_AXIS[2];
			m_interp_axis_TO[2] = -GLOBAL_AXIS[2];

		}break;
	    
		case LOOK_BACK:
		{
			m_interp_TO = m_camera_orbit_center + m_camera_distance * -GLOBAL_AXIS[2];
			m_interp_axis_TO[2] = GLOBAL_AXIS[2];

		}break;

		case LOOK_TOP:
		{
			m_interp_TO = m_camera_orbit_center + m_camera_distance * GLOBAL_AXIS[1];
			m_interp_axis_TO[2] = -GLOBAL_AXIS[1];

		}break;

		case LOOK_BOTTOM:
		{
			m_interp_TO = m_camera_orbit_center - m_camera_distance * GLOBAL_AXIS[1];
			m_interp_axis_TO[2] =  GLOBAL_AXIS[1];

		}break;

		case LOOK_LEFT:
		{
			m_interp_TO = m_camera_orbit_center - m_camera_distance * GLOBAL_AXIS[0];
			m_interp_axis_TO[2] =  GLOBAL_AXIS[0];

		}break;

		case LOOK_RIGHT:
		{
			m_interp_TO = m_camera_orbit_center + m_camera_distance * GLOBAL_AXIS[0];
			m_interp_axis_TO[2] = -GLOBAL_AXIS[0];

		}break;
	}

	m_interp_active = true;
	m_interp_timer.start();
	m_interp_t_start = m_interp_timer.getTime();
	m_interp_dt      = 0.0f;

}
void PinholeCamera::set_interp_flip_Y_Axis()
{
	m_interp_flip_Y_axis_active = true;
	m_interp_axis_FROM[1] = m_camera_v;
	m_interp_axis_TO[1]   = GLOBAL_AXIS[1];
	m_interp_FROM_matrix  = m_camera_matrix;

	m_interp_active = true;
	m_interp_timer.start();
	m_interp_t_start = m_interp_timer.getTime();
	m_interp_dt = 0.0f;
}
void PinholeCamera::set_interp_align_Axis(int axis_index, optix::float3 to_axis , optix::float3 rot_axis )
{
	m_interp_align_axis_active = true;

	m_interp_FROM_matrix = m_camera_matrix;
	if      (axis_index == 0) m_interp_align_axis_FROM = normalize(m_camera_u);
	else if (axis_index == 1) m_interp_align_axis_FROM = normalize(m_camera_v);
	else                      m_interp_align_axis_FROM = normalize(m_camera_w);
	m_interp_align_axis_TO    = to_axis;
	m_interp_align_ROT_axis   = rot_axis;

	float dot = optix::dot(m_interp_align_axis_FROM, to_axis);
	float d   = acosf(dot);

	int sign = 1;
	{
		optix::Matrix4x4 rot = optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(0.1f * d), rot_axis);
		optix::Matrix4x4 mat = rot * m_interp_FROM_matrix;

		optix::float3 u = normalize(m_camera_u);
		optix::float3 v = normalize(m_camera_v);
		optix::float3 w = normalize(m_camera_w);

		u = optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[0], 0.0f));
		v = optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[1], 0.0f));
		w = optix::make_float3(mat * optix::make_float4(GLOBAL_AXIS[2], 0.0f));

		u = normalize(u);
		v = normalize(v);
		w = normalize(w);

		optix::float3 d_axis;
		if      (axis_index == 0) d_axis = u;
		else if (axis_index == 1) d_axis = v;
		else                      d_axis = w;

		float d_delta = acosf(optix::dot(d_axis, to_axis));
		if (d_delta < d)
			sign = 1;
		else
			sign = -1;
	}

	m_interp_align_axis_delta = d * sign; //dot > 0.0f ? -d : d;
	

	m_interp_active = true;
	m_interp_timer.start();
	m_interp_t_start = m_interp_timer.getTime();
	m_interp_dt = 0.0f;
}
void PinholeCamera::set_interp_State(bool state)
{
	m_interp_active = state;
}
bool PinholeCamera::is_Interp_Active()
{
	return m_interp_active;
}