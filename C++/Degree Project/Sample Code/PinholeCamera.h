#pragma once

#ifndef PINHOLE_CAMERA_H
#define PINHOLE_CAMERA_H



#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix.h>

#include "common_structs.h"
#include "Timer.h"



enum Camera_State
{
	CAM_NONE,
	CAM_FREEZE,
	CAM_ORBIT,
	CAM_PAN,
	CAM_DOLLY,
	CAM_FOCUS
};

class PinholeCamera
{

	

public:

	PinholeCamera();
	~PinholeCamera();
	void cleanUp();

	void setViewport(int w, int h);
	void setBaseCoordinates(int x, int y);
	void setSpeedRatio(float f);
	void setFocusDistance(float f);

	void orbit(int x, int y);
	void pan(int x, int y , int prev_x, int prev_y);
	void dolly(int x, int y);
	void focus(int x, int y);
	void zoom(float x);

	bool  getFrustum(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w, bool reset = false);
	void  getFrustum();
	bool  getFrustum_Test(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w, bool reset = false);
	void  getFrustum_Test();
	
	void  getFrustum_Deprecated();

	bool  isChanged(bool reset = false);
	void  reset();
	float getAspectRatio() const;

	optix::float3 getU();
	optix::float3 getV();
	optix::float3 getW();

	optix::float3 getFocusCenter();
	optix::float3 getOrbitCenter();
	optix::float3 getCameraPos();
	void          getCameraBaseCoord(optix::float3 target, optix::float3 up, optix::float3& u, optix::float3 &v, optix::float3 &w);
	float         getDistance();
	float         getPhi();
	float         getTheta();

	void          setChanged(bool state);
	void          Target( class SceneObject* object );
	void          Target( optix::float3 center, float distance );
	void          resetOrbitCenter();
	void          setOrbitCenter(optix::float3 center, bool enable_free_roam = false , bool to_object = false , bool set_old_vars = true);
	void          setOrbitCenter(optix::float3 center, optix::float3 u, optix::float3 v, optix::float3 w, bool set_old_vars = true);
	void          setOrbitCenter_Deprecated(optix::float3 center);
	void          setCenter(optix::float3 center);
	void          setCenter_Deprecated(optix::float3 center);
	void          setCameraPos(optix::float3 pos);
	void          setCameraPos_Deprecated(optix::float3 pos);
	void          setDistance(float dist);

	float          getOrbitDistance();
	void          setOrbitDistance(float dist);
	void          setPhi(float phi);
	void          setTheta(float theta);

	void          calculatePhiTheta();
	void          Update(Io_Mouse mouse_data);
	void          Update_Interp_State();

	optix::Matrix4x4 getCameraMatrix();

	void          setState(Camera_State state);
	Camera_State  getState();

	float         getDistanceFrom(const optix::float3& p);
	float         getInvScaleFactor(const struct VIEWPORT& vp);
	float         getInvScaleFactor();

	void setViewport(int id);
	int getViewportId();
	void retargetingViewport();

	void set_interp_Origin(optix::float3 to);
	void set_interp_Target(optix::float3 to, bool activate = true);
	void set_interp_Camera_State(enum Camera_Look_At look_at);
	void set_interp_flip_Y_Axis();
	void set_interp_align_Axis( int axis_index, optix::float3 to_axis , optix::float3 rot_axis );
	void set_interp_State(bool state);
	bool is_Interp_Active();


	
	optix::Matrix4x4 getCamMatrix()
	{
		return m_camera_matrix;
	}
	optix::Matrix4x4 getCamOrbitMatrix()
	{
		return m_orbit_matrix;
	}
	void setCamMatrix(optix::Matrix4x4 mat)
	{
		m_camera_matrix = mat;
	}
	void setCamOrbitMatrix(optix::Matrix4x4 mat)
	{
		m_orbit_matrix = mat;
	}

public: // Just to be able to load and save them easily.

	optix::float3 m_center;   // Center of interest point, around which is orbited (and the sharp plane of a depth of field camera).
	float         m_distance; // Distance of the camera from the center of intest.
	float         m_phi;      // Range [0.0f, 1.0f] from positive x-axis 360 degrees around the latitudes.
	float         m_theta;    // Range [0.0f, 1.0f] from negative to positive y-axis.
	float         m_fov;      // In degrees. Default is 60.0f
	float         m_near;

private:

	bool setDelta(int x, int y);
	void clamp(float &value, float min, float max , float d);
	void clamp(float &value, float min, float max, float d_min , float d_max);

private:

	int   m_width;    // Viewport width.
	int   m_height;   // Viewport height.
	float m_aspect;   // m_width / m_height
	int   m_baseX;
	int   m_baseY;
	float m_speedRatio;

	// Derived values:
	int           m_dx;
	int           m_dy;
	float m_orbit_dx = 0.0f;
	float m_orbit_dy = 0.0f;

	optix::float3 m_cameraPosition;
	optix::float3 m_cameraU;
	optix::float3 m_cameraV;
	optix::float3 m_cameraW;
	
	optix::Matrix4x4 m_camera_matrix;
	optix::Matrix4x4 m_orbit_matrix;
	bool          m_changed;

	// camera model #2
	optix::float3 m_camera_position;
	optix::float3 m_camera_temp_position;
	optix::float3 m_camera_focus_center;
	optix::float3 m_camera_orbit_center;
	optix::float3 m_camera_orbit_center_old;
	optix::float3 m_camera_focus_delta;
	
	bool m_was_in_frustum          = false;
	bool m_was_in_frustum_selected = false;

	optix::float3 m_focus_dir;
	optix::float3 m_orbit_dir;

	optix::float3 m_camera_u, prev_u;
	optix::float3 m_camera_v, prev_v;
	optix::float3 m_camera_w, prev_w;

	optix::float3 m_orbit_u, m_orbit_v, m_orbit_w;
	optix::float3 m_orbit_delta;

	optix::float3 du, dv, dw;

	float sign;
	optix::float2 m_camera_movement;
	float m_camera_distance;
	float m_camera_distance_old;
	optix::float3 m_relative_pos;
	optix::float3 m_position_offset;

	float diff_u, diff_v, diff_w;
	bool is_inv = false;

	Camera_State m_state;

	int vp_id;

	bool m_free_roam_state = false;
	bool m_in_frustum             = false;
	bool m_in_frustum_panning     = false;
	bool m_in_frustum_orbiting    = false;
	bool m_focused_object_panning = false;
	bool m_orbited_on_free_roam   = false;
	optix::float3 m_panning_center = ZERO_3f;

	bool m_retargeting_viewport = false;
	bool m_re_update_panning_state = false;
	bool m_setting_orbit_center_to_object = false;
	bool m_should_retarget_to_center = true;

	enum Camera_Look_At m_interp_look_at;
	optix::float3 m_interp_cross_axis;
	optix::float3 m_interp_axis_FROM[3];
	optix::float3 m_interp_axis_TO[3];
	optix::float3 m_interp_FROM;
	optix::float3 m_interp_TO;
	optix::float3 m_interp_orbit_TO;
	float m_interp_speed = 1.0f;
	bool m_interp_active = false;
	Timer m_interp_timer;
	float m_interp_t_start;
	float m_interp_t_end;
	float m_interp_dt;
	bool second_interp_phase = false;
	bool m_interp_flip_Y_axis_active = false;
	optix::Matrix4x4 m_interp_FROM_matrix;

	optix::float3 m_interp_align_axis_TO;
	optix::float3 m_interp_align_axis_FROM;
	int m_interp_align_axis_index;
	float m_interp_align_axis_delta;
	bool m_interp_align_axis_active = false;
	optix::float3 m_interp_align_ROT_axis;

	bool m_zooming_stable_activating      = false;
	bool m_zooming_stable                 = false;
	optix::float3 m_stable_zooming_target = ZERO_3f;

};


#endif // PINHOLE_CAMERA_H