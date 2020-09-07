#pragma once

#include "optixu/optixpp_namespace.h"
#include "optixu/optixu_math_namespace.h"
#include <iostream>
#include "ViewportManager_Defines.h"

enum Shading_Mode
{
	DEFAULT_SHADING,
	GRAY_SHADING,
	WIREFRAME_SHADING
};

enum Camera_Look_At
{
	LOOK_NONE,
	LOOK_FRONT,
	LOOK_BACK,
	LOOK_TOP,
	LOOK_BOTTOM,
	LOOK_LEFT,
	LOOK_RIGHT
};

struct Render_Settings
{
	bool wireframe_mode = false;
	Shading_Mode shading_mode = DEFAULT_SHADING;
	bool draw_bbox            = false;
	bool draw_bbox_only = false;
	bool draw_bbox_and_object = false;
	bool draw_restrictions    = false;
	bool draw_relations       = false;
	bool draw_relations_only_selected = false;
	bool draw_relations_only_selected_tree = false;
	bool draw_relations_all = false;

	bool draw_objects_all = true;
	bool draw_groups_all = true;

	bool draw_sampler_all = false;
	bool draw_sampler_only_selected = true;

	bool draw_restrictions_only_selected = false;
	bool draw_restrictions_only_face_vector_selected = false;
	//bool draw_restrictions_only_ui_selected = false;
	bool draw_restrictions_all = false;
	
	bool draw_face_vectors_only_selected = true;
	bool draw_face_vectors_all = false;
	bool draw_restriction_details = true;
	bool draw_restriction_fv = true;
	bool draw_restriction_fv_lines = true;
	bool draw_restriction_cones = true;
	bool draw_restriction_dist_area = true;

	bool draw_lights_all = true;
	bool draw_lights_quad = true;
	bool draw_lights_spot = true;
	bool draw_lights_sphere = true;
	bool draw_lights_point = true;

	bool draw_lights_fill_polygon = true;
	bool draw_lights_fill_poly[4] = { true,true,true,true };
};



struct Viewport_Camera_Settings
{
	Camera_Look_At set_camera_look_at = LOOK_NONE;
};

enum FRAMEBUFFER_ID
{
	MAIN_VIEWPORT_FB,
	TOP_BOTTOM_VIEWPORT_FB,
	LEFT_RIGHT_VIEWPORT_FB,
	ONE_TWO_VIEWPORT_FB,
	ONE_THREE_VIEWPORT_FB
};

enum VIEWPORT_PRESET_
{
	// c0
	DEFAULT_PRESET,
	UP_DOWN_PRESET,
	LEFT_RIGHT_PRESET,
	FOUR_TO_ONE,
	
	// c1
	MAIN_2_LEFT_PRESET,
	MAIN_2_RIGHT_PRESET,
	MAIN_2_DOWN_PRESET,
	MAIN_2_UP_PRESET,

	// c2
	MAIN_3_LEFT_PRESET,
	MAIN_3_RIGHT_PRESET,
	MAIN_3_DOWN_PRESET,
	MAIN_3_UP_PRESET
};

enum VIEWPORT_ID
{
	MAIN_VIEWPORT,

	TOP_LEFT_VIEWPORT,
	TOP_RIGHT_VIEWPORT,
	BOT_LEFT_VIEWPORT,
	BOT_RIGHT_VIEWPORT,

	TOP_VIEWPORT,
	BOTTOM_VIEWPORT,

	LEFT_VIEWPORT,
	RIGHT_VIEWPORT,

	MAIN_VIEWPORT_LEFT,
	LEFT_VIEWPORT_1,
	LEFT_VIEWPORT_2,
	LEFT_VIEWPORT_3,

	MAIN_VIEWPORT_LEFT_P2,
	RIGHT_VIEWPORT_P2_UP,
	RIGHT_VIEWPORT_P2_DOWN,

	MAIN_VIEWPORT_RIGHT,
	RIGHT_VIEWPORT_1,
	RIGHT_VIEWPORT_2,
	RIGHT_VIEWPORT_3,


	MAIN_VIEWPORT_RIGHT_P2,
	LEFT_VIEWPORT_P2_UP,
	LEFT_VIEWPORT_P2_DOWN,

	UP_LEFT_VIEWPORT_1,
	UP_RIGHT_VIEWPORT_1,
	DOWN_LEFT_VIEWPORT_1,
	DOWN_RIGHT_VIEWPORT_1,

	UP_LEFT_VIEWPORT_2,
	UP_MID_VIEWPORT_2,
	UP_RIGHT_VIEWPORT_2,
	DOWN_LEFT_VIEWPORT_2,
	DOWN_MID_VIEWPORT_2,
	DOWN_RIGHT_VIEWPORT_2,

	MAIN_VIEWPORT_2_DOWN,
	MAIN_VIEWPORT_2_UP,
	MAIN_VIEWPORT_3_DOWN,
	MAIN_VIEWPORT_3_UP


};

enum VIEWPORT_RENDERER
{
	OPENGL_RENDERER,
	OPTIX_RENDERER
};

struct VIEWPORT_DIMENSIONS
{
	optix::float2 offset;
	optix::float2 size;

	VIEWPORT_DIMENSIONS()
		:
		offset( optix::make_float2(0.0f) ),
		size(   optix::make_float2(0.0f) )
	{

	}

	~VIEWPORT_DIMENSIONS() {}
};

struct VIEWPORT
{

	Render_Settings render_settings;
	Camera_Look_At camera_settings;
	VIEWPORT_ID id;
	VIEWPORT_RENDERER renderer;

	optix::float2 renderSize;
	optix::float2 renderSize_dynamic;
	VIEWPORT_DIMENSIONS dimensions;
	float renderSize_scaleFactor = 1.0f;
	float scale_factor = 1.0f;

	bool is_active = false;
	bool is_enabled = false;
	bool is_upper = false;

	bool is_dirty = false;
	bool skip_frame = false;
	bool was_skip_frame = false;
	bool display = true;

	int skip_frame_counter;
	int max_frames_skipped;
	int frames_count            = 0;
	float frames_per_second     = 0.0f;
	float sum_fps               = 0.0f;
	float avg_frames_per_second = 0.0f;
	float sum_frame_time        = 0.0f;
	float frame_time            = 0.0f;
	float avg_frame_time        = 0.0f;
	float frame_timer           = 0.0f;
	int   render_count          = 0;

	bool is_dynamically_scaled  = 0;

	int render_sides[4] = { 0,0,0,0 };



	VIEWPORT()
		:
		id(VIEWPORT_ID::MAIN_VIEWPORT),
		renderer(VIEWPORT_RENDERER::OPENGL_RENDERER),
		is_active(false),
		is_enabled(false),
		is_upper(false),
		scale_factor(1.0f),
		is_dirty(false),
		skip_frame(false),
		display(true),
		skip_frame_counter(0),
		max_frames_skipped(MAX_FRAMES_SKIPPED),
		renderSize(optix::make_float2(0.0f))
		//renderSize_scale(optix::make_float2(1.0f))
	{

	}

	~VIEWPORT() {}

	void VIEWPORT::markDirty()
	{
		is_dirty = true;
	}

	bool VIEWPORT::isDirty(bool reset = false)
	{
		bool dirty = is_dirty;
		//if(reset)
			is_dirty = false;
		return dirty;
	}

	bool VIEWPORT::isSkipFrame()
	{
		was_skip_frame = skip_frame;

		bool skip;
		if (skip_frame_counter < max_frames_skipped)
		{
			skip = true;
			skip_frame_counter++;
			skip_frame = true;
		}
		else
		{
			skip = false;
			skip_frame_counter = 0;
			skip_frame = false;
		}

		return skip;
	}

	bool VIEWPORT::isSkipFrame(bool &prev_state)
	{
		prev_state = skip_frame;
		bool skip;
		if (skip_frame_counter < max_frames_skipped)
		{
			skip = true;
			skip_frame_counter++;
			skip_frame = true;
		}
		else
		{
			skip = false;
			skip_frame_counter = 0;
			skip_frame = false;
		}

		return skip;
	}


	void VIEWPORT::reset_Frame_States()
	{
		////std::cout << " - vp[" << id << "].reset_states()!" << std::endl;
		render_count = 0;
		sum_fps = 0.0f;
		sum_frame_time = 0.0f;
		avg_frames_per_second = 0.0f;
		avg_frame_time = 0.0f;
	}

	void VIEWPORT::Update_Frame_States(float fps_dt, float ft_dt)
	{
		if ( render_count >= 50 )
			reset_Frame_States();
		
		//avg_frames_per_second = fps_dt;
		
		render_count++;
		sum_fps += fps_dt;
		sum_frame_time += ft_dt;
		avg_frames_per_second = sum_fps / (float)render_count;
		avg_frame_time = sum_frame_time / (float)render_count;
		
	}

	
	

    /*
	VIEWPORT operator =(const VIEWPORT& source)
	{
		VIEWPORT vp;
		
		vp.renderer          = source.renderer;
		vp.renderSize        = source.renderSize;

		vp.dimensions.offset = source.dimensions.offset;
		vp.dimensions.size   = source.dimensions.size;

		vp.scale_factor      = source.scale_factor;
		vp.is_dirty          = source.is_dirty;

		vp.id = source.id;
		vp.is_active = source.is_active;
		vp.is_enabled = source.is_enabled;
		vp.is_upper = source.is_upper;
		vp.renderer = source.renderer;
		vp.render_settings = source.render_settings;
		vp.skip_frame = source.skip_frame;
		vp.display = source.display;

		vp.skip_frame_counter = source.skip_frame_counter;
		vp.max_frames_skipped = source.max_frames_skipped;
		
		return vp;
	}
	*/
	
};