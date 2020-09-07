#pragma once
#include <GL/glew.h>

#include "ViewportManager_structs.h"
#include "ViewportManager_Defines.h"
#include <unordered_map>
#include "common_structs.h"
#include "PinholeCamera.h"
#include <vector>

class ViewportManager
{
public:

	ViewportManager() {}
	~ViewportManager() {}


	static ViewportManager& GetInstance()
	{
		static ViewportManager vp_manager;
		return vp_manager;
	}


public:
	bool Init();
	void clear();

private:

	bool Init_Viewports( bool clear = false );
	bool Init_ViewportCameras();
	bool Init_Framebuffers();

public:

	void Update();
	void Update(bool utility_active, bool ui_focused);
	void Update_Viewports( bool re_init = true );
	void Update_Camera_Viewports(bool force_changed = false);
	//void set_Camera_Viewports_State();
	void Update_Viewports_Scale_Factors();

private:
	void adjust_Viewports_Resolution();
	void Update_Cameras();

public:
	void Update_Cameras(bool utility_active, bool ui_focused);

public:
	void disableViewports();
	void resetViewports();
	
	void loadDefaultPreset();
	void loadDefaultViewportConfig(VIEWPORT_ID id);

private:
	void  resizeBuffers();
	void  resizeBuffers(const VIEWPORT& vp, optix::float2 dim);
	float compute_Scale_Factor(const VIEWPORT& vp);

public:

	VIEWPORT_PRESET_ getViewportPreset(int index);
	void setActiveViewportPreset_byIndex(int index);
	void setActiveViewportPreset(VIEWPORT_PRESET_ vp_preset);
	void ActivateFocusedViewport();
	void setActiveViewport(VIEWPORT_ID id);
	void setFocusedViewport(VIEWPORT_ID id);
	void setViewport(VIEWPORT viewport);

	void set_Rendering_Resolution(VIEWPORT_ID id, optix::float2 dim);
	void adjust_Rendering_Resolution(VIEWPORT_ID id);
	void Reset_Rendering_Resolution(VIEWPORT_ID id);

	void setViewports_Render_Settings(Render_Settings settings, int id);
	void markViewportsDirty(int code = 0);

	//void markViewportsDirty()
	bool                  isActiveViewportPreset(int code);
	

	VIEWPORT_PRESET_      getActiveViewportPreset();
	VIEWPORT&             getActiveViewport();
	int                   getFocusedViewport();
	VIEWPORT&             getActiveOptiXViewport();
	int                   getActiveViewportCount();
	std::vector<VIEWPORT> getAllEnabledViewports();
	std::vector<VIEWPORT> getEnabledViewports(VIEWPORT_RENDERER id);
	std::vector<VIEWPORT> getViewports();
	VIEWPORT&             getViewport(VIEWPORT_ID id);
	

	PinholeCamera&        getViewportCamera(VIEWPORT_ID id);
	PinholeCamera&        getActiveViewportCamera();
	PinholeCamera&        getOptiXViewportCamera();

	GLuint                getFbo(VIEWPORT_ID id);
	GLuint                getFboTexture(VIEWPORT_ID id);

	GLuint                getFbo_2(VIEWPORT_ID id);
	GLuint                getFboTexture_2(VIEWPORT_ID id);

	GLuint                getFbo_Utility(VIEWPORT_ID id);
	GLuint                getFboTexture_Utility(VIEWPORT_ID id);

	GLuint                getFbo_Utility_2(VIEWPORT_ID id);
	GLuint                getFboTexture_Utility_2(VIEWPORT_ID id);

	GLuint                getFbo_DepthTexture(VIEWPORT_ID id);
	GLuint                getFbo_DepthTexture_2(VIEWPORT_ID id);

	GLuint                getFbo_Camera_Orbit(VIEWPORT_ID id);
	GLuint                getFboTexture_Camera_Orbit(VIEWPORT_ID id);

	GLuint                getFbo_Camera_Orbit_2(VIEWPORT_ID id);
	GLuint                getFboTexture_Camera_Orbit_2(VIEWPORT_ID id);

	Render_Settings       get_Viewports_Render_Settings_Accumulative();
	
	bool m_adjust_res = true;
private:

	std::unordered_map<VIEWPORT_ID, VIEWPORT>      m_Viewports;
	std::unordered_map<VIEWPORT_ID, PinholeCamera> m_Viewport_cameras;

	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_framebuffers;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_textures;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_depth_textures;

	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_framebuffers_2;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_textures_2;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_depth_textures_2;

	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_framebuffers_utility;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_utility_textures;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_utility_depth_textures;

	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_framebuffers_utility_2;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_utility_textures_2;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_utility_depth_textures_2;


	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_Camera_Orbit_framebuffers;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_Camera_Orbit_textures;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_Camera_Orbit_depth_textures;

	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_Camera_Orbit_framebuffers_2;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_Camera_Orbit_textures_2;
	std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_Camera_Orbit_depth_textures_2;

	//std::unordered_map<VIEWPORT_ID, GLuint>        m_Viewport_framebuffers_utility;
	//std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_utility_textures;
	//std::unordered_map<VIEWPORT_ID, GLuint>        m_framebuffers_utility_depth_textures;



	
	VIEWPORT_PRESET_                               m_active_Viewport_preset;
	VIEWPORT_ID                                    m_active_Viewport_id;
	VIEWPORT_ID                                    m_focused_Viewport_id;
	VIEWPORT_ID                                    m_active_OptiX_Viewport_id;
	Dimension_Constraints                          viewport_space;
	int                                            active_viewport_count;
	

	bool m_init = false;
	
};