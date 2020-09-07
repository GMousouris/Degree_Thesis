#include "OpenGL_Renderer.h"

#include "AssetManager.h"
#include "GeometryNode.h"
#include "Tools.h"
#include <algorithm>
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OBJLoader.h"

#include "PinholeCamera.h"
#include "SceneObject.h"


#include "Mediator.h"
#include "GeometryFunctions.h"
#include "Transformations.h"
#include "Utilities.h"
#include "streamHandler.h"
#include "OpenGL_Render_Functions.h"
#include "InterfaceManager.h"
#include "Mesh_Pool.h"
#include "ViewportManager_structs.h"
#include <stack>
#include <stdio.h>
#include <time.h>
#include <chrono>
#include "Restriction_Types.h"

#include "Defines.h"

#include <iostream>


//#define UTILITY_INTERSECTION_DEBUGGING

void GLAPIENTRY
MessageCallback(GLenum source,
				GLenum type,
				GLuint id,
				GLenum severity,
				GLsizei length,
				const GLchar* message,
				const void* userParam)
{
	fprintf(stderr, "\n ------------------- \n ~GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
}



#ifdef UTILITY_RENDERER_CONSTRUCTORS

OpenGLRenderer::OpenGLRenderer()
{


	m_screen_width  = 0;
	m_screen_height = 0;

	for (int i = 0; i < 3; i++)
	{
		m_base_axis_node[i] = nullptr;
	}
	m_sphere_node = nullptr;
	m_cube_node   = nullptr;
	m_disk_node = nullptr;
	m_disk_nswe_node = nullptr;
	m_cube_text_node = nullptr;

	DEF_AXIS_COLOR[0] = DEF_AXIS_COLOR_X;
	DEF_AXIS_COLOR[1] = DEF_AXIS_COLOR_Y;
	DEF_AXIS_COLOR[2] = DEF_AXIS_COLOR_Z;


	AXIS_COLOR[0] = DEF_AXIS_COLOR_X;
	AXIS_COLOR[1] = DEF_AXIS_COLOR_Y;
	AXIS_COLOR[2] = DEF_AXIS_COLOR_Z;

	

	m_render_utility = 0;
	m_render_cam_center_indicator = 0;
	m_active_utility = 0;
	m_render_cursor = 0;


	//m_selected_object = nullptr;
	//m_selected_object_translation_attributes = optix::make_float3(0.0f);
	//m_utility_translation_attributes = optix::make_float3(0.0f);
	//m_camera_inv_scale_factor = 1.0f;
	//m_focused_utility = -1;
	m_active_arc = -1;

	//m_selected_plane = UtilityPlane::NONE;
	m_rendering_mode = RENDERING_MODE::TRIANGLES;
	interface_state = nullptr;
}

OpenGLRenderer::~OpenGLRenderer()
{
	for (int i = 0; i < 3; i++)
		delete m_base_axis_node[i];

	delete m_sphere_node;
	delete m_cube_node;
	delete m_disk_node;
	delete m_disk_nswe_node;
	delete m_cube_text_node;

	glDeleteTextures(1, &m_fbo_depth_texture);
	glDeleteTextures(1, &m_fbo_texture_utility);
	glDeleteFramebuffers(1, &m_fbo_utility);

	glDeleteTextures(1, &cursor_fbo_texture);
	glDeleteTextures(1, &view_axis_texture);
	glDeleteTextures(1, &view_axis_depth_texture);
	glDeleteFramebuffers(1, &cursor_fbo);
	glDeleteFramebuffers(1, &view_axis_fbo);
	
}

#endif

#ifdef UTILITY_RENDERER_INIT_FUNCTIONS

bool OpenGLRenderer::Init()
{

#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) || defined(INIT_STATE)
	streamHandler::Report("OpenGL_Renderer", "Init", "");
#endif

	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(MessageCallback, 0);

	m_focused_object_changed          = false;
	m_current_focused_object_index    = -1;
	m_prev_focused_object_index       = -1;
	m_selected_object_changed         = false;
	m_current_selected_object_index   = -1;
	m_prev_selected_object_index      = -1;
	timer = 0.0f;

	m_fbo                 = 0;
	m_fbo_texture         = 0;
	m_fbo_utility         = 0;
	m_fbo_texture_utility = 0;
	m_fbo_depth_texture   = 0;

	cursor_fbo = 0;
	cursor_fbo_texture = 0;
	view_axis_fbo = 0;
	view_axis_texture = 0;
	//m_vao_fbo     = 0;

	
	bool techniques_initialization = Init_Shaders();
	bool buffers_initialization    = InitDeferredShaderBuffers() && Init_Utility_FrameBuffers();
	bool viewports_initialization  = Init_Viewports();
	bool meshes_initialization     = InitGeometricMeshes();

	//If there was any errors
#ifdef GL_CHECKERROR_CATCH
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		return false;
	}
#endif



	char * * argv = nullptr;
	int argc = 0;
	glutInit(&argc, argv);

	g_timer.start();

	return techniques_initialization && buffers_initialization &&  meshes_initialization && viewports_initialization;
}

bool OpenGLRenderer::Init_Viewports()
{
	
	int width  = Mediator::RequestWindowSize(0);
	int height = Mediator::RequestWindowSize(1);
	optix::float2 renderSize = optix::make_float2(width, height);
	return true;
}

bool OpenGLRenderer::Init_Shaders()
{
	bool init = true;

	std::string vertex_shader_path;
	std::string fragment_shader_path;

	// Init Utility Rendering Shader
	vertex_shader_path   = "../Data/Shaders/Opengl Shaders/Utility shader/utility_rendering.vert";
	fragment_shader_path = "../Data/Shaders/Opengl Shaders/Utility shader/utility_rendering.frag";
	m_utility_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_utility_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	m_utility_rendering_program.CreateProgram();

	// Init Cursor Rendering Shader
	vertex_shader_path   = "../Data/Shaders/Opengl Shaders/Cursor shader/cursor_rendering.vert";
	fragment_shader_path = "../Data/Shaders/Opengl Shaders/Cursor shader/cursor_rendering.frag";
	m_cursor_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_cursor_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	m_cursor_rendering_program.CreateProgram();


	// Init PostProcEffects Shader
	vertex_shader_path   = "../Data/Shaders/Opengl Shaders/Postprocess shader/postproc.vert";
	fragment_shader_path = "../Data/Shaders/Opengl Shaders/Postprocess shader/postproc.frag";
	m_postprocess_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_postprocess_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	m_postprocess_program.CreateProgram();

	// Init PostProcFXAA Shader
	vertex_shader_path   = "../Data/Shaders/Opengl Shaders/Postprocess shader/fxaa.vert";
	fragment_shader_path = "../Data/Shaders/Opengl Shaders/Postprocess shader/fxaa.frag";
	m_postproc_fxaa_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_postproc_fxaa_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	m_postproc_fxaa_program.CreateProgram();


	// Init Cursor Rendering Shader
	vertex_shader_path   = "../Data/Shaders/Opengl Shaders/Scene shader/scene_rendering.vert";
	fragment_shader_path = "../Data/Shaders/Opengl Shaders/Scene shader/scene_rendering.frag";
	m_scene_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_scene_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	m_scene_rendering_program.CreateProgram();


	// Init BackGround Rendering Shader
	vertex_shader_path = "../Data/Shaders/Opengl Shaders/Postprocess shader/background.vert";
	fragment_shader_path = "../Data/Shaders/Opengl Shaders/Postprocess shader/background.frag";
	m_background_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_background_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	m_background_rendering_program.CreateProgram();


	return init && Init_Shader_Uniforms();
}

bool OpenGLRenderer::Init_Shader_Uniforms()
{
	bool initialized = true;

	m_utility_rendering_program.LoadUniform("uniform_projection_matrix");
	m_utility_rendering_program.LoadUniform("uniform_view_matrix");
	m_utility_rendering_program.LoadUniform("uniform_model_matrix");
	m_utility_rendering_program.LoadUniform("uniform_normal_matrix");
	m_utility_rendering_program.LoadUniform("uniform_parent_matrix");
	m_utility_rendering_program.LoadUniform("uniform_parent_matrix_scaled");
	m_utility_rendering_program.LoadUniform("uniform_parent_matrix_rotated_scaled");
	m_utility_rendering_program.LoadUniform("uniform_use_model_matrix");
	m_utility_rendering_program.LoadUniform("uniform_use_parent_matrix");
	m_utility_rendering_program.LoadUniform("uniform_use_parent_matrix_scaled");
	m_utility_rendering_program.LoadUniform("uniform_use_parent_matrix_rotated_scaled");
	m_utility_rendering_program.LoadUniform("uniform_object_center");
	m_utility_rendering_program.LoadUniform("uniform_is_arc");
	m_utility_rendering_program.LoadUniform("uniform_diffuse");
	m_utility_rendering_program.LoadUniform("uniform_alpha");
	m_utility_rendering_program.LoadUniform("uniform_stable_circle");
	m_utility_rendering_program.LoadUniform("uniform_camera_pos");
	m_utility_rendering_program.LoadUniform("uniform_text_render");
	m_utility_rendering_program.LoadUniform("uniform_camera_scale_factor");
	m_utility_rendering_program.LoadUniform("uniform_selected_circle");
	m_utility_rendering_program.LoadUniform("uniform_circle");
	m_utility_rendering_program.LoadUniform("uniform_camera_w");
	m_utility_rendering_program.LoadUniform("uniform_utility_pos");
	m_utility_rendering_program.LoadUniform("outline_1st_pass");
	m_utility_rendering_program.LoadUniform("outline_2nd_pass");
	m_utility_rendering_program.LoadUniform("draw_sphere");
	m_utility_rendering_program.LoadUniform("uniform_sphere_center");
	m_utility_rendering_program.LoadUniform("draw_sphere_wireframe");
	m_utility_rendering_program.LoadUniform("inner_radius_hit");
	m_utility_rendering_program.LoadUniform("samplerHDR");
	m_utility_rendering_program.LoadUniform("depthSampler");
	m_utility_rendering_program.LoadUniform("uniform_rendering_scene_origin");
	m_utility_rendering_program.LoadUniform("uniform_test_depth");
	m_utility_rendering_program.LoadUniform("uniform_mix_color");
	m_utility_rendering_program.LoadUniform("uniform_mix_p0");
	m_utility_rendering_program.LoadUniform("uniform_mix_p1");
	m_utility_rendering_program.LoadUniform("uniform_mcolor_a");
	m_utility_rendering_program.LoadUniform("uniform_mcolor_b");


	m_utility_rendering_program.LoadUniform("uniform_rendering_gradient_grid");
	m_utility_rendering_program.LoadUniform("uniform_gradient_grid_o");
	m_utility_rendering_program.LoadUniform("uniform_gradient_grid_dist");

	m_utility_rendering_program.Bind();
	glUniform1i(m_utility_rendering_program["samplerHDR"],   0);
	glUniform1i(m_utility_rendering_program["depthSampler"], 1);
	glUniform1i(m_utility_rendering_program["uniform_test_depth"], 0);
	glUniform1i(m_utility_rendering_program["uniform_rendering_gradient_grid"], 0);
	glUniform1i(m_utility_rendering_program["uniform_mix_color"],0);
	m_utility_rendering_program.Unbind();

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	m_cursor_rendering_program.LoadUniform("transform");
	m_cursor_rendering_program.LoadUniform("uniform_model_matrix");
	m_cursor_rendering_program.LoadUniform("uniform_view_matrix");
	m_cursor_rendering_program.LoadUniform("uniform_projection_matrix");
	m_cursor_rendering_program.LoadUniform("uniform_texture");
	//m_cursor_rendering_program.Bind();
	//glUniform1i(m_cursor_rendering_program["transform"], 1);
	//m_cursor_rendering_program.Unbind();

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	m_postprocess_program.LoadUniform("samplerHDR");
	m_postprocess_program.LoadUniform("colorBalance");
	m_postprocess_program.LoadUniform("invWhitePoint");
	m_postprocess_program.LoadUniform("burnHighlights");
	m_postprocess_program.LoadUniform("saturation");
	m_postprocess_program.LoadUniform("crushBlacks");
	m_postprocess_program.LoadUniform("invGamma");
	m_postprocess_program.LoadUniform("apply_postproc_effects");
	m_postprocess_program.LoadUniform("multisample");
	m_postprocess_program.LoadUniform("uniform_discard_alpha");
	m_postprocess_program.LoadUniform("uniform_visualize_depth");
	m_postprocess_program.LoadUniform("uniform_write_depth");
	m_postprocess_program.LoadUniform("Depth_Sampler");
	m_postprocess_program.LoadUniform("uniform_invert_pixels");
	m_postprocess_program.Bind();
	glUniform1i(m_postprocess_program["uniform_visualize_depth"], 0);
	glUniform1i(m_postprocess_program["uniform_write_depth"], 0);
	glUniform1i(m_postprocess_program["uniform_invert_pixels"], 0);
	m_postprocess_program.Unbind();

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	m_postproc_fxaa_program.LoadUniform("samplerHDR");

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

	m_background_rendering_program.LoadUniform("color_a");
	m_background_rendering_program.LoadUniform("color_b");
	m_background_rendering_program.LoadUniform("uniform_size");
	m_background_rendering_program.LoadUniform("samplerHDR");

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/



	m_scene_rendering_program.LoadUniform("uniform_model_matrix");
	m_scene_rendering_program.LoadUniform("uniform_normal_matrix");
	m_scene_rendering_program.LoadUniform("uniform_view_matrix");
	m_scene_rendering_program.LoadUniform("uniform_projection_matrix");
	m_scene_rendering_program.LoadUniform("diffuse_texture");
	//m_scene_rendering_program.LoadUniform("samplerHDR");
	m_scene_rendering_program.LoadUniform("uniform_diffuse");
	m_scene_rendering_program.LoadUniform("uniform_alpha");
	m_scene_rendering_program.LoadUniform("uniform_specular");
	m_scene_rendering_program.LoadUniform("uniform_shininess");
	m_scene_rendering_program.LoadUniform("uniform_has_texture");
	m_scene_rendering_program.LoadUniform("uniform_camera_position");
	m_scene_rendering_program.LoadUniform("uniform_camera_w");
	m_scene_rendering_program.LoadUniform("uniform_is_app_grid");
	m_scene_rendering_program.LoadUniform("uniform_shading_mode");
	m_scene_rendering_program.LoadUniform("uniform_rendering_utility");
	m_scene_rendering_program.LoadUniform("uniform_alpha");
	m_scene_rendering_program.LoadUniform("uniform_Material_textureArray");
	m_scene_rendering_program.LoadUniform("uniform_is_instance");
	m_scene_rendering_program.LoadUniform("uniform_is_camera_orbit");

	m_scene_rendering_program.LoadUniform("uniform_rendering_distance_sphere");
	m_scene_rendering_program.LoadUniform("uniform_ds_a");
	m_scene_rendering_program.LoadUniform("uniform_ds_b");
	m_scene_rendering_program.LoadUniform("uniform_ds_inv");
	m_scene_rendering_program.LoadUniform("uniform_ds_r");
	m_scene_rendering_program.Bind();
	glUniform1i(m_scene_rendering_program["uniform_is_instance"], 1);
	glUniform1i(m_scene_rendering_program["uniform_is_camera_orbit"], 0);
	glUniform1i(m_scene_rendering_program["uniform_rendering_distance_sphere"], 0);
	m_scene_rendering_program.Unbind();

	return initialized && Init_postproc_effects();
}

bool OpenGLRenderer::Init_postproc_effects()
{
	m_post_proc_parameters.m_gamma          = 2.2f;
	m_post_proc_parameters.m_colorBalance   = optix::make_float3(1.0f, 1.0f, 1.0f);
	m_post_proc_parameters.m_whitePoint     = 1.0f;
	m_post_proc_parameters.m_burnHighlights = 0.8f;
	m_post_proc_parameters.m_crushBlacks    = 0.2f;
	m_post_proc_parameters.m_saturation     = 1.2f;
	m_post_proc_parameters.m_brightness     = 0.8f;

	m_postprocess_program.Bind();
	glUniform1f(m_postprocess_program["invGamma"],       1.0f / m_post_proc_parameters.m_gamma);
	glUniform3f(m_postprocess_program["colorBalance"],   m_post_proc_parameters.m_colorBalance.x, m_post_proc_parameters.m_colorBalance.y, m_post_proc_parameters.m_colorBalance.z);
	glUniform1f(m_postprocess_program["invWhitePoint"],  m_post_proc_parameters.m_brightness / m_post_proc_parameters.m_whitePoint);
	glUniform1f(m_postprocess_program["burnHighlights"], m_post_proc_parameters.m_burnHighlights);
	glUniform1f(m_postprocess_program["crushBlacks"],    m_post_proc_parameters.m_crushBlacks + m_post_proc_parameters.m_crushBlacks + 1.0f);
	glUniform1f(m_postprocess_program["saturation"],     m_post_proc_parameters.m_saturation);
	m_postprocess_program.Unbind();

	return true;
}

bool OpenGLRenderer::InitDeferredShaderBuffers()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) || defined(INIT_STATE)
	streamHandler::Report("OpenGL_Renderer", "InitDeferredShaderBuffers", "");
#endif

	glGenTextures(1, &m_fbo_texture_utility);
	glGenTextures(1, &m_fbo_depth_texture);
	glGenFramebuffers(1, &m_fbo_utility);

	return true;
}

bool OpenGLRenderer::Init_Utility_FrameBuffers()
{
	glGenTextures(1, &cursor_fbo_texture);
	glGenTextures(1, &view_axis_texture);
	glGenTextures(1, &view_axis_depth_texture);
	glGenFramebuffers(1, &cursor_fbo);
	glGenFramebuffers(1, &view_axis_fbo);

	// cursor fbo //
	{
		
		glBindTexture(GL_TEXTURE_2D, cursor_fbo_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_cursor_size.x , m_cursor_size.y , 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		// framebuffer to link to everything together
		glBindFramebuffer(GL_FRAMEBUFFER, cursor_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cursor_fbo_texture, 0);
		
		GLenum status = Tools::CheckFramebufferStatus( cursor_fbo );
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			//std::cout << " ~Error : OpenGLRenderer::ResizeBuffers():: check status of : cursor_fbo " << std::endl;
			system("pause");
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// view axis fbo //
	{
		
		glBindTexture(GL_TEXTURE_2D, view_axis_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_view_axis_size.x , m_view_axis_size.y , 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, view_axis_depth_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_view_axis_size.x, m_view_axis_size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		// framebuffer to link to everything together
		glBindFramebuffer(GL_FRAMEBUFFER, view_axis_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, view_axis_texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, view_axis_depth_texture, 0);

		GLenum status = Tools::CheckFramebufferStatus(view_axis_fbo);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			//std::cout << " ~Error : OpenGLRenderer::ResizeBuffers():: check status of : view_axis_fbo " << std::endl;
			system("pause");
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	return true;
}

// TODO : na ta kanei load o assetManager k na ta kanw link apo ekei
bool OpenGLRenderer::InitGeometricMeshes()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) || defined(INIT_STATE)
	streamHandler::Report("OpenGL_Renderer", "InitGeometricMeshes", "");
#endif

	bool initialized = true;
	OBJLoader loader;

	auto mesh = loader.load("../Data/Assets/Axis/xAxis4.obj");
	if (mesh != nullptr)
	{
		m_base_axis_node[0] = new GeometryNode();
		m_base_axis_node[0]->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Assets/Axis/yAxis4.obj");
	if (mesh != nullptr)
	{
		m_base_axis_node[1] = new GeometryNode();
		m_base_axis_node[1]->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Assets/Axis/zAxis4.obj");
	if (mesh != nullptr)
	{
		m_base_axis_node[2] = new GeometryNode();
		m_base_axis_node[2]->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Assets/sphere2.obj");
	if (mesh != nullptr)
	{
		m_sphere_node = new GeometryNode();
		m_sphere_node->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Assets/unit_cube.obj");
	if (mesh != nullptr)
	{
		m_cube_node = new GeometryNode();
		m_cube_node->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;


	mesh = loader.load("../Data/Assets/unit_disk.obj");
	if (mesh != nullptr)
	{
		m_disk_node = new GeometryNode();
		m_disk_node->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Assets/unit_disk_nswe.obj");
	if (mesh != nullptr)
	{
		m_disk_nswe_node = new GeometryNode();
		m_disk_nswe_node->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Assets/cube_text.obj");
	if (mesh != nullptr)
	{
		m_cube_text_node = new GeometryNode();
		m_cube_text_node->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	return initialized;
}


#endif

#ifdef UTILITY_RENDERER_UPDATE_FUNCTIONS


void OpenGLRenderer::Update(float dt)
{

#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) 
	streamHandler::Report("OpenGL_Renderer", "Update", "");
#endif

	

	ResizeBuffers();
	interface_state = InterfaceManager::GetInstance().get_current_Interface_State();

	m_prev_focused_object_index = m_current_focused_object_index;
	m_current_focused_object_index = Mediator::RequestFocusedObjectIndex();
	if (m_current_focused_object_index != m_prev_focused_object_index)
		m_focused_object_changed = true;

}

void OpenGLRenderer::Update_Viewports()
{

}

void OpenGLRenderer::ResizeBuffers()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) 
	streamHandler::Report("OpenGL_Renderer", "ResizeBuffers", "");
#endif

#ifdef OPENGL_DEBUG

	//std::cout << " - OpenGLRenderer::ResizeBuffers():" << std::endl;

#endif

	//int width  = m_custom_viewports[m_active_viewport].width;
	//int height = m_custom_viewports[m_active_viewport].height;
	VIEWPORT vp = ViewportManager::GetInstance().getActiveOptiXViewport();
	int width   = vp.renderSize.x;
	int height  = vp.renderSize.y;
	
#ifdef OPENGL_DEBUG

	//std::cout << "    - vp : " << vp.id << std::endl;
	//std::cout << "    - width : " << width << std::endl;
	//std::cout << "    - height : " << height << std::endl;

#endif

	if ( (m_screen_width != width || m_screen_height != height)
		&& (width > 0 && height > 0) )
	{
		m_screen_width  = width;
		m_screen_height = height;


		glBindTexture(GL_TEXTURE_2D, m_fbo_texture_utility);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_screen_width, m_screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_screen_width, m_screen_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 , NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		// framebuffer to link to everything together
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_utility);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,         GL_TEXTURE_2D, m_fbo_texture_utility, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,  GL_TEXTURE_2D, m_fbo_depth_texture,   0);

		GLenum status = Tools::CheckFramebufferStatus(m_fbo_utility);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
#ifdef OPENGL_DEBUG
			//std::cout << " - OpenGLRenderer::ResizeBuffers()::CheckError:" << std::endl;
#endif
			//std::cout << " ~Error : OpenGLRenderer::ResizeBuffers():: check status of : m_fbo_utility" << std::endl;
			system("pause");
		}



#ifdef GL_CHECKERROR_CATCH
		if (Tools::CheckGLError() != GL_NO_ERROR)
		{
			//std::cout << " - OpenGLRenderer::ResizeBuffers()::CheckError:" << std::endl;
			system("pause");
		}
#endif

		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		//
		m_projection_matrix = glm::perspective(glm::radians(M_FOV), (width) / (float)(height), NEAR, FAR);
		m_utility_rendering_program.Bind();
		glUniformMatrix4fv(m_utility_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
		m_utility_rendering_program.Unbind();
	}

}

void OpenGLRenderer::ResizeBuffers(int width, int height)
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) 
	streamHandler::Report("OpenGL_Renderer", "ResizeBuffers", "");
#endif


#ifdef OPENGL_DEBUG

	//std::cout << " - OpenGLRenderer::ResizeBuffers( w = "<<width<<" , h = "<< height << " ):" << std::endl;

#endif


	if ( (m_screen_width != width || m_screen_height != height)
		&& (width > 0 && height > 0))
	{
		m_screen_width  = width;
		m_screen_height = height;

#ifdef OPENGL_DEBUG

		//std::cout << "    - width : " << width << std::endl;
		//std::cout << "    - height : " << height << std::endl;

#endif


		glBindTexture(GL_TEXTURE_2D, m_fbo_texture_utility);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_screen_width, m_screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_screen_width, m_screen_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		// framebuffer to link to everything together
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_utility);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture_utility, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_fbo_depth_texture, 0);

		GLenum status = Tools::CheckFramebufferStatus(m_fbo_utility);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
#ifdef OPENGL_DEBUG
			//std::cout << " - OpenGLRenderer::ResizeBuffers( width = " << width << " , height = " << height << " )::CheckError:" << std::endl;
#endif
			//std::cout << " ~Error : OpenGLRenderer::ResizeBuffers():: check status of : m_fbo_utility" << std::endl;
			system("pause");
		}

#ifdef GL_CHECKERROR_CATCH
		if (Tools::CheckGLError() != GL_NO_ERROR)
		{
			//std::cout << " - OpenGLRenderer::ResizeBuffers( w = "<<width<<" , h = "<<height << " )::CheckError:" << std::endl;
			system("pause");
		}
#endif
		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		//
		m_projection_matrix = glm::perspective(glm::radians(M_FOV), (width) / (float)(height), NEAR, FAR);
		// pass updated projection matrix to shaders
		m_utility_rendering_program.Bind();
		glUniformMatrix4fv(m_utility_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
		m_utility_rendering_program.Unbind();
	}

}


void OpenGLRenderer::Update_Camera(PinholeCamera& camera_instance, bool force_update)
{
	bool camera_changed = false;
	if (camera_changed = camera_instance.isChanged(false) || true)  //|| true)
	{
		m_render_cam_center_indicator = 1;
		optix::float3 cam_pos    = camera_instance.getCameraPos();
		optix::float3 center     = camera_instance.getOrbitCenter(); // or FocusCenter?
		optix::float3 camera_up  = camera_instance.getV();

		optix::float3 m_camera_w = camera_instance.getW();
		glm::vec3 pos            = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
		glm::vec3 up             = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
		glm::vec3 target         = pos + VECTOR_TYPE_TRANSFORM::TO_GLM_3f(normalize(m_camera_w)) * 10.0f;
		m_view_matrix            = glm::lookAt(pos, target, up);
		

		//Mediator::RequestCameraInstance().reset();
		glUniformMatrix4fv(m_utility_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
		glUniform3f(m_utility_rendering_program["uniform_camera_pos"], cam_pos.x, cam_pos.y, cam_pos.z);
		glUniform3f(m_utility_rendering_program["uniform_camera_w"], m_camera_w.x, m_camera_w.y, m_camera_w.z);
	}
	else
		m_render_cam_center_indicator = 0;
}
void OpenGLRenderer::Update_ColorVars()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) 
	streamHandler::Report("OpenGL_Renderer", "Update_ColorVars", "");
#endif

	
	AXIS_COLOR[0] = DEF_AXIS_COLOR[0];
	AXIS_COLOR[1] = DEF_AXIS_COLOR[1];
	AXIS_COLOR[2] = DEF_AXIS_COLOR[2];

	bool is_active = interface_state->active_utility->active;
	bool is_trns   = interface_state->active_utility->id == u_TRANSLATION;

	switch (interface_state->isec_p->plane_id)
	{
	case X:
		AXIS_COLOR[0] = HIGHLIGHT_COLOR;
		break;
	case Y:
		AXIS_COLOR[1] = HIGHLIGHT_COLOR;
		break;
	case Z:
		AXIS_COLOR[2] = HIGHLIGHT_COLOR;
		break;
	case XY:
		AXIS_COLOR[0] = HIGHLIGHT_COLOR;
		AXIS_COLOR[1] = HIGHLIGHT_COLOR;
		break;

	case XZ:
		AXIS_COLOR[0] = HIGHLIGHT_COLOR;
		AXIS_COLOR[2] = HIGHLIGHT_COLOR;
		break;

	case YZ:
		AXIS_COLOR[1] = HIGHLIGHT_COLOR;
		AXIS_COLOR[2] = HIGHLIGHT_COLOR;
		break;
	case XYZ:
		if (interface_state->active_utility->id == u_SCALE)
		{
			AXIS_COLOR[0] = HIGHLIGHT_COLOR;
			AXIS_COLOR[1] = HIGHLIGHT_COLOR;
			AXIS_COLOR[2] = HIGHLIGHT_COLOR;
		}
	}
}

#endif

#ifdef UTILITY_RENDERER_SHADER_FUNCTIONS

void OpenGLRenderer::Shader_start(const VIEWPORT& vp, bool enable_blend )
{
	GLuint vp_fbo_utility = ViewportManager::GetInstance().getFbo_Utility(vp.id);

	glBindFramebuffer(GL_FRAMEBUFFER, vp_fbo_utility );
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);


	// openGL parameters
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	
	if (enable_blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// render only points, lines, triangles
	switch (m_rendering_mode)
	{
	case RENDERING_MODE::TRIANGLES:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case RENDERING_MODE::LINES:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case RENDERING_MODE::POINTS:
		glPointSize(2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	};

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(UTILITY_LINE_WIDTH);
	m_utility_rendering_program.Bind();

#ifdef GL_CHECKERROR_CATCH
	GLenum error = Tools::CheckGLError();
	if (error != GL_NO_ERROR)
	{
		printf("OpenGLRenderer::Shader_Start()::Draw GL Error\n");
		system("pause");
	}
#endif

}
void OpenGLRenderer::Shader_start(const VIEWPORT& vp, GLuint fbo, bool enable_blend)
{
	GLuint vp_fbo_utility = fbo;

	glBindFramebuffer(GL_FRAMEBUFFER, vp_fbo_utility);
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);


	// openGL parameters
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);


	if (enable_blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// render only points, lines, triangles
	switch (m_rendering_mode)
	{
	case RENDERING_MODE::TRIANGLES:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case RENDERING_MODE::LINES:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case RENDERING_MODE::POINTS:
		glPointSize(2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	};

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(UTILITY_LINE_WIDTH);
	m_utility_rendering_program.Bind();

#ifdef GL_CHECKERROR_CATCH
	GLenum error = Tools::CheckGLError();
	if (error != GL_NO_ERROR)
	{
		printf("OpenGLRenderer::Shader_Start( ... )::Draw GL Error\n");
		system("pause");
	}
#endif

}


void OpenGLRenderer::Update_OBJECT_Matrices(Utility_RenderList_Object& OBJECT, PinholeCamera& camera, const VIEWPORT& vp)
{
	
	UtilityProperties utility = OBJECT.ut;
	if      ( utility.group == g_TRANSFORMATIONS )
	{

		SceneObject * raw_object = OBJECT.obj_pack->raw_object;
		AXIS_ORIENTATION m_selected_axis_orientation = InterfaceManager::GetInstance().get_active_Axis_Orientation();
		if ( (raw_object->getType() == SAMPLER_OBJECT || raw_object->getType() == LIGHT_OBJECT) && utility.id == u_SCALE) m_selected_axis_orientation = LOCAL; // force to LOCAL axis scope if SAMPLER_OBJECT && u_SCALE //

		// update selected object's base axis if rotation changed
		if (m_selected_axis_orientation != AXIS_ORIENTATION::SCREEN || (!interface_state->active_utility->active && !interface_state->m_gui_transforming))
		{
			BASE_TRANSFORM::AXIS(raw_object, m_selected_axis_orientation, &OBJECT.obj_pack->base_axis[0]);
		}


		glm::mat4 object_rotation_matrix = glm::mat4(1.0f);
		glm::vec3 world_pos = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(raw_object->getTranslationAttributes());
		float obj_dist = camera.getDistanceFrom( raw_object->getTranslationAttributes() );
		optix::float3 delta = interface_state->m_gui_transforming? ZERO_3f : raw_object->getAttributesDelta(0);

		switch (m_selected_axis_orientation)
		{
		case AXIS_ORIENTATION::WORLD:
			object_rotation_matrix = glm::mat4(1.0f);
			break;
		case AXIS_ORIENTATION::LOCAL:
		{
#ifdef CANCER_DEBUG
			bool s = (raw_object == nullptr || raw_object == 0);
			
			if (s)
			{
				//std::cout << "\n OpenGLRenderer::Update_OBJECT_MATRICES():" << std::endl;
				//std::cout << "          - OBJ.obj_pack1 : " << !s << std::endl;
				system("pause");
			}
#endif
			object_rotation_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getRotationMatrix_chain());
		}
			break;
		}



		float renderSize_factor = camera.getInvScaleFactor(vp);
		float m_camera_inv_scale_factor;

		if (utility.id == u_TRANSLATION)
		{
			if (optix::length(delta) == 0)
				m_camera_inv_scale_factor = obj_dist * TRANSLATION_AXIS_SCALE_FACTOR * renderSize_factor;
			else
				m_camera_inv_scale_factor = OBJECT.obj_pack->obj_dists[vp.id] * TRANSLATION_AXIS_SCALE_FACTOR * renderSize_factor;
		}
		else if (utility.id == u_SCALE)
			m_camera_inv_scale_factor = obj_dist * TRANSLATION_AXIS_SCALE_FACTOR * renderSize_factor;
		else if (utility.id == Utility::u_ROTATION)
			m_camera_inv_scale_factor = obj_dist * ROTATION_AXIS_SCALE_FACTOR * renderSize_factor;

		glm::mat4 object_transformation_matrix =
			glm::translate(glm::mat4(1.0f), world_pos)
			* object_rotation_matrix
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.6f * m_camera_inv_scale_factor));

		glm::mat4 object_def_transformation_matrix =
			glm::translate(glm::mat4(1.0f), world_pos)
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.6f * m_camera_inv_scale_factor));

		glm::mat4 object_translation_matrix_scaled =
			glm::translate(glm::mat4(1.0f), world_pos)
			* glm::scale(glm::mat4(1.0f), glm::vec3(1.3f * 0.6f * m_camera_inv_scale_factor));

		bool screen_axis = false;
		glm::mat4 screen_trs_matrix = glm::mat4(1.0f);
		if (interface_state->active_axis_orientation == AXIS_ORIENTATION::SCREEN)
		{
			screen_trs_matrix = glm::translate(glm::mat4(1.0f), world_pos) * BASE_TRANSFORM::ONB_glm(OBJECT.obj_pack->base_axis)
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.6f * m_camera_inv_scale_factor));
			//screen_axis = true;
		}

		
		OBJECT.obj_pack->transformation_matrix     = object_transformation_matrix;
		OBJECT.obj_pack->translation_matrix_scaled = object_translation_matrix_scaled;
		OBJECT.obj_pack->def_transformation_matrix = object_def_transformation_matrix;
		OBJECT.obj_pack->screen_trs_matrix         = screen_trs_matrix;
		OBJECT.obj_pack->cam_inv_scale_factor      = m_camera_inv_scale_factor;
		
		
		if (interface_state->active_utility->id != u_ROTATION)
			OBJECT.obj_pack->cam_inv_scale_factor = TRANSLATION_AXIS_SCALE_FACTOR * obj_dist * renderSize_factor;

	}
	else if ( utility.group == g_LINKING || utility.group == g_RESTRICTIONS || utility.group == g_ALIGN )
	{
		SceneObject * raw_object = OBJECT.obj_pack->raw_object;
		if (raw_object != 0)
		{
			float obj_dist = camera.getDistanceFrom(raw_object->getTranslationAttributes());
			float renderSize_factor = camera.getInvScaleFactor(vp);
			float m_camera_inv_scale_factor = obj_dist * LINKING_SCALE_FACTOR * renderSize_factor;
			OBJECT.obj_pack->cam_inv_scale_factor = m_camera_inv_scale_factor;
		}
	}
	else if ( utility.group == g_ATTACH )
	{
		SceneObject * raw_object = OBJECT.obj_pack->raw_object;
		float obj_dist           = camera.getDistanceFrom(raw_object->getTranslationAttributes());
		float renderSize_factor  = camera.getInvScaleFactor(vp);
		float m_camera_inv_scale_factor = obj_dist * LINKING_SCALE_FACTOR * renderSize_factor;


		glm::vec3 pos = VECTOR_TYPE_TRANSFORM::TO_GLM_3f( raw_object->getTranslationAttributes());
		glm::vec3 centroid = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(raw_object->getCentroid());
		glm::mat4 rot_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getRotationMatrix_chain());
		glm::mat4 scale_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getScaleMatrix_value());

		glm::mat4 object_transformation_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX( raw_object->getTransformationMatrix() );
		glm::mat4 object_scaled_matrix         = glm::translate(glm::mat4(1.0f), pos) * rot_matrix; //* glm::translate(glm::mat4(1.0f), -centroid ) * scale_matrix ;

		OBJECT.obj_pack->cam_inv_scale_factor      = m_camera_inv_scale_factor;
		OBJECT.obj_pack->transformation_matrix     = object_transformation_matrix;
		OBJECT.obj_pack->translation_matrix_scaled = object_scaled_matrix;
		BASE_TRANSFORM::AXIS(raw_object, AXIS_ORIENTATION::LOCAL , &OBJECT.obj_pack->base_axis[0]);
	}
	else if ( utility.group == g_CREATE )
	{
		
		float renderSize_factor         = camera.getInvScaleFactor(vp);
		float m_camera_inv_scale_factor = LINKING_SCALE_FACTOR * renderSize_factor;

		glm::mat4 object_transformation_matrix = glm::mat4(1.0f);
		glm::mat4 object_scaled_matrix         = glm::mat4(1.0f); //* glm::translate(glm::mat4(1.0f), -centroid ) * scale_matrix ;

		OBJECT.obj_pack->cam_inv_scale_factor      = m_camera_inv_scale_factor;
		OBJECT.obj_pack->transformation_matrix     = object_transformation_matrix;
		OBJECT.obj_pack->translation_matrix_scaled = object_scaled_matrix;

		if (interface_state->m_sa == SA_BBOX)
		{
			SceneObject * raw_object = OBJECT.obj_pack->raw_object;
			if (raw_object != 0)
			{
				float obj_dist                  = camera.getDistanceFrom(raw_object->getTranslationAttributes());
				float renderSize_factor         = camera.getInvScaleFactor(vp);
				float m_camera_inv_scale_factor = obj_dist * LINKING_SCALE_FACTOR * renderSize_factor;


				glm::vec3 pos = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(raw_object->getTranslationAttributes());
				glm::vec3 centroid = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(raw_object->getCentroid());
				glm::mat4 rot_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getRotationMatrix_chain());
				glm::mat4 scale_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getScaleMatrix_value());

				glm::mat4 object_transformation_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getTransformationMatrix());
				glm::mat4 object_scaled_matrix = glm::translate(glm::mat4(1.0f), pos) * rot_matrix; //* glm::translate(glm::mat4(1.0f), -centroid ) * scale_matrix ;

				OBJECT.obj_pack->cam_inv_scale_factor = m_camera_inv_scale_factor;
				OBJECT.obj_pack->transformation_matrix = object_transformation_matrix;
				OBJECT.obj_pack->translation_matrix_scaled = object_scaled_matrix;
				BASE_TRANSFORM::AXIS(raw_object, AXIS_ORIENTATION::LOCAL, &OBJECT.obj_pack->base_axis[0]);
			}
		}
	}
	else if ( utility.group == g_NONE   )
	{
		switch (OBJECT.render_type)
		{
		case RENDER_LIGHT_OBJECT:
			break;
		case RENDER_LIGHT_OBJECT_PROPERTIES:
		{
			//PinholeCamera cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
			SceneObject * raw_object = OBJECT.obj_pack->raw_object;
			float obj_dist           = camera.getDistanceFrom(raw_object->getTranslationAttributes());
			float renderSize_factor  = vp.scale_factor;
			float m_camera_inv_scale_factor = obj_dist * LINKING_SCALE_FACTOR * renderSize_factor;


			glm::vec3 pos          = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(raw_object->getTranslationAttributes());
			glm::vec3 centroid     = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(raw_object->getCentroid());
			glm::mat4 rot_matrix   = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getRotationMatrix_chain());
			glm::mat4 scale_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getScaleMatrix_value());

			glm::mat4 object_transformation_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getTransformationMatrix());
			glm::mat4 object_scaled_matrix         = glm::translate(glm::mat4(1.0f), pos) * rot_matrix; 

			OBJECT.obj_pack->cam_inv_scale_factor      = m_camera_inv_scale_factor;
			OBJECT.obj_pack->transformation_matrix     = object_transformation_matrix;
			OBJECT.obj_pack->translation_matrix_scaled = object_scaled_matrix;
			
		}
		break;

		}
	}


}

void OpenGLRenderer::Update_Shader_Uniforms(Utility_RenderList_Object& OBJECT)
{
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
	glUniform1i(m_utility_rendering_program["uniform_circle"], -2);
	glUniform1i(m_utility_rendering_program["uniform_selected_circle"], -4);
	glUniform1f(m_utility_rendering_program["uniform_camera_scale_factor"], OBJECT.obj_pack->cam_inv_scale_factor);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_parent_matrix"], 1, GL_FALSE, glm::value_ptr(OBJECT.obj_pack->transformation_matrix));
	Update_ColorVars();
}
void OpenGLRenderer::Update_Shader_Uniforms(Highlight_RenderList_Object& OBJECT)
{
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
	glUniform1i(m_utility_rendering_program["uniform_circle"], -2);
	glUniform1i(m_utility_rendering_program["uniform_selected_circle"], -4);
}

void OpenGLRenderer::Update_Shader_Utility_To_Render(Utility_RenderList_Object * utility_to_render)
{
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
	glUniform1i(m_utility_rendering_program["uniform_circle"], -2);
	glUniform1i(m_utility_rendering_program["uniform_selected_circle"], -4);

	//if (interface_state->isec_p->plane_id == XYZ && interface_state->active_utility->id == u_TRANSLATION)
	if (interface_state->active_utility->id != u_ROTATION)
		utility_to_render->obj_pack->cam_inv_scale_factor = TRANSLATION_AXIS_SCALE_FACTOR * utility_to_render->obj_pack->raw_object->getDistanceFromCamera();


	glUniform1f(m_utility_rendering_program["uniform_camera_scale_factor"], utility_to_render->obj_pack->cam_inv_scale_factor);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_parent_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_render->obj_pack->transformation_matrix));

	Update_ColorVars();
	Update_Camera(*utility_to_render->camera_instance);
}
void OpenGLRenderer::Update_Shader_Utility_To_Render(Utility_RenderList_Object * utility_to_render, const VIEWPORT& vp)
{
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
	glUniform1i(m_utility_rendering_program["uniform_circle"], -2);
	glUniform1i(m_utility_rendering_program["uniform_selected_circle"], -4);

	//if (interface_state->isec_p->plane_id == XYZ && interface_state->active_utility->id == u_TRANSLATION)
	if (interface_state->active_utility->id != u_ROTATION)
		utility_to_render->obj_pack->cam_inv_scale_factor = TRANSLATION_AXIS_SCALE_FACTOR * utility_to_render->obj_pack->raw_object->getDistanceFromCamera();


	glUniform1f(m_utility_rendering_program["uniform_camera_scale_factor"], utility_to_render->obj_pack->cam_inv_scale_factor);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_parent_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_render->obj_pack->transformation_matrix));

	Update_ColorVars();
	Update_Camera( ViewportManager::GetInstance().getViewportCamera(vp.id));
}

void OpenGLRenderer::Update_Shader_Highlight_To_Render(Highlight_RenderList_Object * highlight_to_render)
{
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
	glUniform1i(m_utility_rendering_program["uniform_circle"], -2);
	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	glUniform1i(m_utility_rendering_program["uniform_selected_circle"], -4);
	Update_Camera(*highlight_to_render->camera_instance);
}
void OpenGLRenderer::Update_Shader_Highlight_To_Render(Highlight_RenderList_Object * highlight_to_render, const VIEWPORT& vp)
{
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
	glUniform1i(m_utility_rendering_program["uniform_circle"], -2);
	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	glUniform1i(m_utility_rendering_program["uniform_selected_circle"], -4);

	PinholeCamera cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
	Update_Camera( cam );

	SceneObject* obj = highlight_to_render->obj_pack.raw_object;
	float dist_from_cam = cam.getDistanceFrom(obj->getTranslationAttributes());
	float scale_factor = cam.getInvScaleFactor(vp);
	highlight_to_render->obj_pack.cam_inv_scale_factor = LINKING_SCALE_FACTOR * dist_from_cam * scale_factor;
	// obj_dist * LINKING_SCALE_FACTOR * renderSize_factor;
}

void OpenGLRenderer::Shader_end()
{

	glBindVertexArray(0);

	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	m_utility_rendering_program.Unbind();
	if (m_rendering_mode != RENDERING_MODE::TRIANGLES)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_DEPTH_TEST);
	glPointSize(1.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// just for debugging reasons
	// RenderText(glm::vec3(0.5), glm::vec3(1.0f), FONT2, "");
	draw::Text(glm::vec3(0.5), glm::vec3(1.0f), FONT2, "");


#ifdef GL_CHECKERROR_CATCH
	GLenum error = Tools::CheckGLError();
	if (error != GL_NO_ERROR)
	{
		printf("OpenGLRenderer::Shader_End()::Draw GL Error\n");
		system("pause");
	}
#endif

	m_render_utility = 0;

}

#endif

#ifdef UTILITY_RENDERER_RENDER_FUNCTIONS


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// 
//                         RENDER
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void OpenGLRenderer::Render()
{
	////std::cout << "\n - OpenGLRenderer::Render():" << std::endl;

#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) 
	streamHandler::Report("OpenGL_Renderer", "Render", "");
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.27f, 0.27f, 0.27f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	Render_Viewports_Overlay();
	

#ifdef GET_RENDER_TIME_METRICS
	
	float t_start = g_timer.getTime();
	
#endif

	// gather scene's hierarchy - Bounding Information //
	gather_Scene_Data();

	
	// Render OpenGL Viewports
	Render_Viewports();

	
#ifdef GET_RENDER_TIME_METRICS

	
	float t_end      = g_timer.getTime();
	double time_span = t_end - t_start;

	global_render_count++;
	global_time_counter     = g_timer.getTime();
	global_avg_Render_time += time_span / global_render_count;

	// reset every 5 seconds
	bool print_render_time_info = false;
	if ( global_time_counter  >= 5.0f )
	{
		g_timer.reset();
		g_timer.start();

		print_render_time_info = true;
	}

	if ( print_render_time_info )
	{
		//std::cout << "\n\n ~ OpenGLRenderer::TIME_METRICS:" << std::endl;
		//std::cout << "  - - - - - - - - - - - - - - - - - - - - - " << std::endl;
		
		//std::cout << "      > avg_Scene_Render_time   : " << global_avg_render_scene_time * 1.0f << " (s) " << std::endl;
		//std::cout << "      > avg_State_Change_time   : " << global_avg_State_Change_Time * 1.0f << " (s) " << std::endl;
		//std::cout << "      > total_avg_Render_Time   : " << global_avg_Render_time       * 1.0f << " (s) "<< std::endl;
		//std::cout << "      > total_time_passed       : " << global_time_counter          * 1.0f << " (s) " << std::endl;
		////std::cout << "      > Render_count            : " << global_render_count << std::endl;
		//std::cout << "  - - - - - - - - - - - - - - - - - - - - - " << std::endl;

		global_avg_Render_time = 0.0f;
		global_render_count    = 0;

		global_avg_State_Change_Time = 0.0f;
		global_state_change_counter  = 0;

		global_avg_render_scene_time = 0.0f;
		global_render_scene_counter  = 0;
	}

#endif


	//Render_Viewports_Overlay();


	// Clear Render Lists // 
	m_SceneObject_RenderList.clear();
	m_Utility_RenderList.clear();
	m_Highlight_RenderList.clear();
	m_shapes2D_RenderList.clear();
	m_Shapes_perViewport_depth.clear();
	for ( auto& it : m_Shapes2D_per_Viewport )
		it.second.clear();
}
void OpenGLRenderer::Render_Viewports()
{
	
	// get Enabled ( for OpenGL to Render ) viewports 
	//std::vector<VIEWPORT> viewports = ViewportManager::GetInstance().getEnabledViewports( VIEWPORT_RENDERER::OPENGL_RENDERER );
	std::vector<VIEWPORT> viewports = ViewportManager::GetInstance().getAllEnabledViewports();
	bool scene_was_dirty;
	bool scene_is_dirty = Mediator::isSceneDirty( &scene_was_dirty );


	for (VIEWPORT vp : viewports)
	{
		bool is_dirty        = ViewportManager::GetInstance().getViewport(vp.id).isDirty();
		bool is_active       = ViewportManager::GetInstance().getViewport(vp.id).is_active;
		bool was_skipFrame;
		bool is_skipFrame    = ViewportManager::GetInstance().getViewport(vp.id).isSkipFrame( was_skipFrame ) && !is_active;
		bool is_force_render = ( scene_was_dirty && !scene_is_dirty ) && ( !is_active && was_skipFrame )  ? true : false;

		// 
		PRE_Viewport_Render_APPLY(vp);
		Render_Viewport( vp, is_dirty, is_active, is_skipFrame, scene_is_dirty , is_force_render );
		POST_Viewport_Render_APPLY(vp);
		//
	}


}
void OpenGLRenderer::Render_Viewport(const VIEWPORT& vp, bool is_dirty, bool is_active, bool is_skipFrame,  bool scene_is_dirty, bool force_render)
{


#define COMPUTE_SCENE_RENDER_TIME
#ifdef COMPUTE_SCENE_RENDER_TIME
	
	Timer t_timer;
	t_timer.start();
	float t_start = t_timer.getTime();

#endif

	setUp_RenderingViewport(vp);

	// if scene is dirty ( or viewport is dirty ) 
	// RENDER : Scene && background
	bool render_scene = vp.renderer == OPTIX_RENDERER ? true : (is_dirty || scene_is_dirty) && !is_skipFrame;
	bool rendered_scene = false;
	

	
	if (render_scene || force_render)
	{

		rendered_scene = true;

		// clear default fbo ( COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT )
		Clear_Fbo(ViewportManager::GetInstance().getFbo(vp.id), true, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));


		if (vp.renderer == OPTIX_RENDERER)
		{
			// Copy SceneTexture to ViewportManager's Fbo
			// containing only Optix Rendered Objects - { Light Objects , Bbox Objects }
			RenderToOutFB(
				ViewportManager::GetInstance().getFbo(vp.id),   // to_fbo
				Mediator::requestRenderedSceneTexture(),        // from_texture
				0,  // post_proc ?
				0,  // use_blend ?
				0,  // discard alpha ?
				1,  // scale
				0,  // visualize depth ?
				1,  // write depth ?
				Mediator::Request_Optix_DepthBuffer()  // from_Depth_Texture
			);

			// Render Viewport Scene ~ Only : { Light Objects, Bbox Objects }
#ifdef USE_OptiXViewport_Scene_Depth
			Render_Viewport_Scene( vp );
#endif

		}
		else
		{
			
			// Render Background
			// bind : default viewports Fbo
			Render_Background(vp, ViewportManager::GetInstance().getFbo(vp.id));

			// Render Scene
			Render_Viewport_Scene( vp );

			// Apply post-process effects to rendered scene
			// postProcessScene(ViewportManager::GetInstance().getFbo(vp.id), ViewportManager::GetInstance().getFboTexture(vp.id));
			
		}

	}
	
	// copy scene tex to second Fbo (Fbo_2)
	{
		int    write_depth   = 1;
		GLuint depth_texture = ViewportManager::GetInstance().getFbo_DepthTexture(vp.id);

		Clear_Fbo(ViewportManager::GetInstance().getFbo_2( vp.id ), true, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));		
		RenderToOutFB(
			ViewportManager::GetInstance().getFbo_2(vp.id),        // target Fbo
			ViewportManager::GetInstance().getFboTexture(vp.id),   // source Texture
			1,   // enable post_proc
			0,   // enable blend
			0,   // discard_alpha
			1,   // scale factor
			0,   // visualize depth
			write_depth,   // write depth
			depth_texture  // from_depth_Texture
		);
	}
	

	if( rendered_scene )
		ViewportManager::GetInstance().getViewport(vp.id).frames_count++;
	
//  #define Optix_DepthBuffer_Test
#ifdef Optix_DepthBuffer_Test

	GLuint depth_texture = ViewportManager::GetInstance().getFbo_DepthTexture_2(vp.id);
	//GLuint depth_texture = vp.renderer == OPTIX_RENDERER ? Mediator::Request_Optix_DepthBuffer() : ViewportManager::GetInstance().getFbo_DepthTexture(vp.id);
	RenderToOutFB(
		ViewportManager::GetInstance().getFbo_2(vp.id),  // target Fbo
		depth_texture,                                   // source Texture
		0,   // enable post_proc
		0,   // enable blend
		0,   // discard_alpha
		1,   // scale factor
		1    // visualize depth
	);

#endif

	
#ifdef COMPUTE_SCENE_RENDER_TIME
	
	float t_end = t_timer.getTime();
	t_timer.stop();
	float dt = (t_end - t_start);

	float frame_timer = ViewportManager::GetInstance().getViewport(vp.id).frame_timer;
	if (rendered_scene)
	{
		ViewportManager::GetInstance().getViewport(vp.id).frame_time += dt;
		if (ViewportManager::GetInstance().getViewport(vp.id).frames_count >= 50)
		{
			ViewportManager::GetInstance().getViewport(vp.id).avg_frames_per_second = (ViewportManager::GetInstance().getViewport(vp.id).frames_count) / ViewportManager::GetInstance().getViewport(vp.id).frame_time;
			ViewportManager::GetInstance().getViewport(vp.id).avg_frame_time = (ViewportManager::GetInstance().getViewport(vp.id).frame_time) / ViewportManager::GetInstance().getViewport(vp.id).frames_count;
			ViewportManager::GetInstance().getViewport(vp.id).frames_count = 0;
			ViewportManager::GetInstance().getViewport(vp.id).frame_time   = 0.0f;
		}
	}
#endif


	// RENDER: Utilities in any case
	Render_Utilities(vp);

	reset_Viewport();
}

//
void OpenGLRenderer::Render_Background(const VIEWPORT& vp, GLuint m_fbo)
{

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	m_background_rendering_program.Bind();


	optix::float3 color_a = Mediator::request_Background_Colors(0);
	optix::float3 color_b = Mediator::request_Background_Colors(1);
	glUniform3f(m_background_rendering_program["color_a"], color_a.x, color_a.y, color_a.z);
	glUniform3f(m_background_rendering_program["color_b"], color_b.x, color_b.y, color_b.z);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 1.0f);
	glEnd();

	m_background_rendering_program.Unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}
void OpenGLRenderer::Render_Viewport_Scene(const VIEWPORT& vp)
{

	Render_Settings vp_settings = vp.render_settings;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// bind scene rendering program //
	m_scene_rendering_program.Bind();

	glUniform1i(m_scene_rendering_program["diffuse_texture"], 0);
	glActiveTexture(GL_TEXTURE0);

	glUniform1i(m_scene_rendering_program["uniform_Material_textureArray"], 1);
	glActiveTexture(GL_TEXTURE1);

	switch (vp_settings.shading_mode)
	{
	case Shading_Mode::DEFAULT_SHADING:   {   glUniform1i(m_scene_rendering_program["uniform_shading_mode"], 0);  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);   } break;
	case Shading_Mode::GRAY_SHADING:      {   glUniform1i(m_scene_rendering_program["uniform_shading_mode"], 1);  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);   } break;
	//case Shading_Mode::WIREFRAME_SHADING: {   glUniform1i(m_scene_rendering_program["uniform_shading_mode"], 2);  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);   } break;
	}

	if (vp.render_settings.wireframe_mode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glLineWidth(1.0f);

	// bind Viewport's framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, ViewportManager::GetInstance().getFbo(vp.id));
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	

	// update shader's uniforms
	
		PinholeCamera camera = ViewportManager::GetInstance().getViewportCamera(vp.id);
		optix::float3 cam_pos,  camera_up, w;
		
		optix::float3 center = camera.getOrbitCenter();
		cam_pos   = camera.getCameraPos();
		camera_up = camera.getV();
		w         = camera.getW();
		glm::vec3 pos    = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
		glm::vec3 up     = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
		glm::vec3 target = pos + VECTOR_TYPE_TRANSFORM::TO_GLM_3f(normalize(w)) * 10.0f;
		glm::mat4 scene_view_matrix = glm::lookAt(pos, target, up);
		glm::mat4 m_projection_matrix = glm::perspective(glm::radians(M_FOV), vp.renderSize.x / vp.renderSize.y, NEAR, FAR);
		glUniformMatrix4fv(m_scene_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(scene_view_matrix));
		glUniformMatrix4fv(m_scene_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
		glUniform3f(m_scene_rendering_program["uniform_camera_position"], cam_pos.x, cam_pos.y, cam_pos.z);
		glUniform3f(m_scene_rendering_program["uniform_camera_w"], w.x, w.y, w.z);
		glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
	


	// Render Scene
	Render_Scene(vp, scene_view_matrix, m_projection_matrix);


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(UTILITY_LINE_WIDTH);


	// unbind scene rendering program //
	m_scene_rendering_program.Unbind();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void OpenGLRenderer::Render_Scene(const VIEWPORT& vp, glm::mat4 view_matrix, glm::mat4 proj_matrix)
{
	//glEnable(GL_NORMALIZE);

	int vp_id = vp.id;
	int r_relations = 0;
	int r_face_vectors = 0;
	int r_restrictions = 0;
	int r_lights = 0;
	int r_bbox   = 0;

	bool draw_objects = vp.render_settings.draw_objects_all;
	bool draw_groups  = vp.render_settings.draw_groups_all;

	Render_Settings render_settings = vp.render_settings;
	if      (render_settings.draw_bbox_only)       r_bbox = 1;
	else if (render_settings.draw_bbox_and_object) r_bbox = 2;
	if      (render_settings.draw_lights_all)    r_lights = 5;
	else if (render_settings.draw_lights_quad)   r_lights = 1;
	else if (render_settings.draw_lights_spot)   r_lights = 2;
	else if (render_settings.draw_lights_sphere) r_lights = 3;
	else if (render_settings.draw_lights_point)  r_lights = 4;


	SceneObject* selected_object = Mediator::Request_INTERFACE_Selected_Object(); //Mediator::RequestSelectedObject();
	int selected_id = selected_object == nullptr ? -1 : selected_object->getId();
	SceneObject* focused_object = Mediator::Request_INTERFACE_Focused_Object(); //Mediator::RequestFocusedObject();
	int focused_id = focused_object == nullptr ? -1 : focused_object->getId();
	bool selected_obj_is_group = selected_object == nullptr ? false : selected_object->getType() == GROUP_PARENT_DUMMY;
	bool focused_obj_is_group = focused_object == nullptr ? false : focused_object->getType() == GROUP_PARENT_DUMMY;
	int ui_focused_id = Mediator::Request_Ui_Focused_Object();

	bool is_Optix_Viewport = vp.renderer == OPTIX_RENDERER;
	bool PASS_OBJECT_RENDERING = false;


#ifdef USE_OptiXViewport_Scene_Depth
	PASS_OBJECT_RENDERING = is_Optix_Viewport;
#endif


#define SIMPLE_SCENE_RENDER
#ifdef SIMPLE_SCENE_RENDER

	glm::mat4 model_matrix;
	int is_app_grid = false;

	

	std::vector<SceneObject*> scene_objects = Mediator::RequestSceneObjects(); // SCENE_OBJECTS_NO_UTILITY //
	for (SceneObject* obj : scene_objects)
	{
		Type obj_type        = obj->getType();
		bool is_light_object = obj_type == LIGHT_OBJECT;
		bool is_group_object = obj_type == GROUP_PARENT_DUMMY;
		bool is_app_grid     = obj_type == APP_OBJECT;

		if (is_app_grid) 
			continue;

		if (   !obj->isActive()
			|| !obj->isVisible()
			|| obj_type == DUMMY
			//|| obj->getType() == GROUP_PARENT_DUMMY
			|| obj_type == FACE_VECTOR
			|| obj_type == SAMPLER_OBJECT
			|| obj->isTemporary()
			|| (obj_type == IMPORTED_OBJECT && !draw_objects)
			|| (obj_type == GROUP_PARENT_DUMMY && !draw_groups)
			)
			continue;

		int obj_id = obj->getId();

		Light_Type  l_type = obj->get_LightParameters().type;
		bool should_render_light = false;
		if (   (render_settings.draw_lights_quad   && l_type == QUAD_LIGHT)
			|| (render_settings.draw_lights_spot   && l_type == SPOTLIGHT)
			|| (render_settings.draw_lights_sphere && l_type == SPHERICAL_LIGHT)
			|| (render_settings.draw_lights_point  && l_type == POINT_LIGHT)
			|| (render_settings.draw_lights_all)
			)
		{
			should_render_light = true;
		}
		if (focused_id == obj_id || selected_id == obj_id) 
			should_render_light = true;

		//int should_render_geometry = is_Optix_Viewport ? is_light_object && should_render_light : 1;
		int should_render_geometry = !is_light_object || should_render_light;

		// Render Object's Geometry //
		if ( !is_group_object )
		{
			//if (is_light_object) Mediator::scale_Light(obj, vp.id, true);
			model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getTransformationMatrix());
			//if (is_light_object) Mediator::scale_Light(obj, vp.id, false);

			is_app_grid? 1 : 0; glUniform1i(m_scene_rendering_program["uniform_is_app_grid"], is_app_grid);

			if (is_app_grid) 
				glDisable(GL_CULL_FACE);

			if ( (is_light_object) || is_app_grid )
			{
				
				if (is_light_object)
				{
					

				}

				// disable temporarily wireframe 
				if (vp.render_settings.wireframe_mode)
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				
				if (should_render_geometry)
				{
					
					GLenum gl_type = GL_TRIANGLES;

					if (is_light_object)
					{
						
						GLenum pol_type = render_settings.draw_lights_fill_polygon || l_type == POINT_LIGHT ? GL_FILL : GL_LINE;
						glm::vec3 col_a = pol_type == GL_FILL ? glm::vec3(0.85) : glm::vec3(0.75, 0.75f, 0.45f);
						glm::vec3 col_b = glm::vec3(0.75, 0.75f, 0.45f)*0.75f;
						glPolygonMode(GL_FRONT_AND_BACK, pol_type);
						glLineWidth(1.0f);
						Render_GeometryNode_Arrays(obj,
												   Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj),
												   model_matrix,
												   m_scene_rendering_program,
												   //glm::vec3(1,1,1),
												   //glm::vec3(0.75f),
												   col_a,
												   gl_type);

						if (   l_type != POINT_LIGHT 
							&& render_settings.draw_lights_fill_polygon
							//&& false
							)
						{
							glPolygonMode(GL_FRONT, GL_LINE);
							glLineWidth(1.0f);
							Render_GeometryNode_Arrays(obj,
													   Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj),
													   model_matrix,
													   m_scene_rendering_program,
													   //glm::vec3(0.0f),
													   col_b,
													   gl_type);
						}

						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						glLineWidth(1.0f);


						if (l_type == POINT_LIGHT)
						{
							glLineWidth(3.0f);
							optix::float3 u = normalize(Mediator::RequestCameraInstance().getU());
							optix::float3 v = normalize(Mediator::RequestCameraInstance().getV());
							glUniform3f(m_scene_rendering_program["uniform_diffuse"], col_b.x, col_b.y, col_b.z);
							glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
							glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 1);
							draw::Arc_arbitr_plane(obj->getTranslationAttributes(), 5.f, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, 30, true, GL_LINE_LOOP);
							glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
							glLineWidth(1.0f);
						}

					}
					else
					{
						Render_GeometryNode_Arrays(obj,
												   Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj),
												   model_matrix,
												   m_scene_rendering_program,
												   gl_type);
					}
					
				}

				if (vp.render_settings.wireframe_mode)
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
			}
			else if ( r_bbox != 1
					 && !PASS_OBJECT_RENDERING
					 )
			{
				Render_GeometryNode(obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_matrix, m_scene_rendering_program);
			}

			if (is_app_grid) 
				glEnable(GL_CULL_FACE);
		}

		if (PASS_OBJECT_RENDERING) 
			continue;


		bool is_selected   = obj->getId() == selected_id;
		bool is_focused    = obj->getId() == focused_id;
		bool is_ui_focused = (ui_focused_id == obj->getId());
		bool obj_is_group  = is_group_object;

		// draw Bbox //
		glm::vec3 highlight_color = glm::vec3(1.0f);
		bool is_dashed  = ( /*!is_selected &&*/ obj_is_group) ? true : false;
		float thickness = ((obj_is_group) && is_focused && !is_selected) ? 2.0f : 1.0f;
		if ((r_bbox == 1 || (interface_state->active_utility->group == g_RESTRICTIONS && obj_is_group)) && !is_app_grid
			)
		{
			glm::mat4 trs_matrix = (obj_is_group) ? glm::mat4(1.0f) : model_matrix;
			glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(trs_matrix));
			{
				SceneObject* group_parent = obj->getGroupParent();
				bool has_group_parent = group_parent == nullptr ? false : true;
				bool is_grp_parent_selected = (!has_group_parent) ? false : (group_parent->getId() == selected_id);
				bool is_grp_parent_focused = (!has_group_parent) ? false : ((group_parent->getId() == focused_id) || (group_parent->getId() == ui_focused_id));
				bool is_grp_parent_ui_focused = (!has_group_parent) ? false : (group_parent->getId() == ui_focused_id);

				if (selected_obj_is_group && !is_selected && has_group_parent && !(is_ui_focused || is_grp_parent_ui_focused))
					is_selected |= Utilities::is_object_in_List(obj, selected_object->getGroupSelectionChilds_and_Groups());
				if (focused_obj_is_group && !is_focused && has_group_parent && !(is_ui_focused || is_grp_parent_ui_focused))
					is_focused |= Utilities::is_object_in_List(obj, focused_object->getGroupSelectionChilds_and_Groups());

				if (is_selected || is_grp_parent_selected && !(is_ui_focused || is_grp_parent_ui_focused))
					highlight_color = glm::vec3(1.0f);
				else if (is_focused && !is_grp_parent_focused && obj_type != GROUP_PARENT_DUMMY)
					highlight_color = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->get_Highlight_Color());
				else if (is_grp_parent_focused && obj_type != GROUP_PARENT_DUMMY)
					highlight_color = HIGHLIGHT_COLOR;
				else if (obj->getType() != GROUP_PARENT_DUMMY)
				{
					float * Kd = obj->getMaterialParams(0).Kd;
					highlight_color = glm::vec3(Kd[0], Kd[1], Kd[2]);
				}

			}

		}

		if ((r_bbox != 0 || (interface_state->active_utility->group == g_RESTRICTIONS && obj_is_group)) && !is_app_grid && !is_light_object
			&& (!obj_is_group || draw_groups)
			)
		{
			if (obj_is_group)
				glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
			glUniform3f(m_scene_rendering_program["uniform_diffuse"], highlight_color.r, highlight_color.g, highlight_color.b);
			glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 1);
			is_dashed ? draw::BBox_dashed(obj->getBBoxMin(), obj->getBBoxMax(), 0.275f, thickness) 
				: draw::BBox(obj->getBBoxMin(), obj->getBBoxMax(), ZERO_3f, thickness);
			glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
		}


	}


	
	// draw scene grid //
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 1);
	{

		glm::vec3 color_l = glm::vec3(0.27f);
		glm::vec3 color_d = glm::vec3(0.035f);
		glm::vec3 color = color_l; //glm::vec3(0.05f);
		glUniform3f(m_scene_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
		glUniform1f(m_scene_rendering_program["unfirom_alpha"], 1.0f);

		glm::mat4 model_matrix = glm::mat4(1.0f);
		glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

		optix::float3 plane_p0 = ZERO_3f - optix::make_float3(0,1,0) * 0.1f;
		optix::float3 u = GLOBAL_AXIS[0];
	

#define draw_scene_grid_based_on_UserParams
#ifdef draw_scene_grid_based_on_UserParams

		

		SceneParameters s_params    = Mediator::Request_SceneParameters();
		SceneGrid_Paramaters params = s_params.grid_params;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// scene grid
		if (params.render)
		{

			for (int x = -params.block_numb.x; x <= params.block_numb.x; x++)
			{


				float X = x * params.block_size.x;
				float Y = params.block_numb.y  * params.block_size.y;
				
				if (X == 0)
					continue;
				
				optix::float3 p0 = optix::make_float3(X, 0, 0);
				optix::float3 p1 = optix::make_float3(X, 0, Y);
				optix::float3 p2 = optix::make_float3(X, 0, -Y);
				draw::Line3D(p0, p1, 1.0f);
				draw::Line3D(p0, p2, 1.0f);
			}

			for (int y = -params.block_numb.y; y <= params.block_numb.y; y++)
			{

				float Y = y * params.block_size.y;
				float X = params.block_numb.x  * params.block_size.x;
				
				if (Y == 0)
					continue;

				optix::float3 p0 = optix::make_float3(0, 0, Y);
				optix::float3 p1 = optix::make_float3(X, 0, Y);
				optix::float3 p2 = optix::make_float3(-X, 0, Y);
				draw::Line3D(p0, p1, 1.0f);
				draw::Line3D(p0, p2, 1.0f);
			}

			// draw origin lines
			glUniform3f(m_scene_rendering_program["uniform_diffuse"], color_d.r, color_d.g, color_d.b);
			float far_ = 99999.f;
			optix::float3 p = optix::make_float3(far_, 0, far_);
			draw::Line3D(optix::make_float3(-far_, 0, 0), optix::make_float3(far_, 0, 0));
			draw::Line3D(optix::make_float3(0, 0, -far_), optix::make_float3(0, 0, far_));

		}

		if (s_params.visualize)
		{
			glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 1);
			if (s_params.vis_planes)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDisable(GL_CULL_FACE);
				glUniform3f(m_scene_rendering_program["uniform_diffuse"], s_params.vis_color.x, s_params.vis_color.y, s_params.vis_color.z);
				glUniform1f(m_scene_rendering_program["uniform_alpha"], s_params.alpha);
				draw::BBox_Planes(-s_params.max_dimensions, s_params.max_dimensions, ZERO_3f, 0,
								  true, 1.0f);
				glUniform1f(m_scene_rendering_program["uniform_alpha"], 1.0f);
				glEnable(GL_CULL_FACE);
				glDisable(GL_BLEND);
			}

			//glDisable(GL_DEPTH_TEST);
			glUniform3f(m_scene_rendering_program["uniform_diffuse"], 0.7,0.7,0.7);
			glUniform1f(m_scene_rendering_program["uniform_alpha"], 1.0f);
			draw::BBox(-s_params.max_dimensions, s_params.max_dimensions, ZERO_3f, s_params.vis_planes ? 2 : 1);
			//glEnable(GL_DEPTH_TEST);
			glLineWidth(1);

			glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
		}
		
		glUniform1f(m_scene_rendering_program["unfirom_alpha"], 1.0f);

#endif
		

	}
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
	glDisable(GL_LINE_SMOOTH);
	//


	// draw depth shapes //
	for (Shape shape : m_Shapes_perViewport_depth[vp.id])
	{

		optix::float3 a = shape.a;
		optix::float3 b = shape.b;
		glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 1);
		

		float dist = length(shape.b - shape.a);
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			
			bool draw_arcs   = true;
			bool draw_sphere = false;

			glm::mat4 model_matrix;
			glm::mat4 normal_matrix;

			glUniform1i(m_scene_rendering_program["uniform_rendering_distance_sphere"], 1);
			glUniform3f(m_scene_rendering_program["uniform_ds_a"], a.x, a.y, a.z);
			glUniform3f(m_scene_rendering_program["uniform_ds_b"], b.x, b.y, b.z);
			glUniform1f(m_scene_rendering_program["uniform_ds_r"], shape.length);
			glUniform1i(m_scene_rendering_program["uniform_ds_inv"], 0);

			// arc 
			if( draw_arcs )
			{
				optix::float3 u = (a - b);
				optix::float3 v = cross(u, GLOBAL_AXIS[1]); //ViewportManager::GetInstance().getViewportCamera(vp.id).getW());
				optix::float3 w = cross(u, v);

				// arc //
				glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
				glUniform3f(m_scene_rendering_program["uniform_diffuse"], shape.color.x,shape.color.y,shape.color.z);
				glUniform1f(m_scene_rendering_program["uniform_alpha"], 1.f);
				glUniform1i(m_scene_rendering_program["uniform_ds_inv"], 1);
				glLineWidth(3.0f);
				draw::Arc_arbitr_plane(shape.a,
									   shape.length + 0.001f, 0, Geometry::GENERAL::toRadians(360.0f),
									   normalize(u), normalize(v),
									   40, 0, GL_LINE_LOOP);
				glLineWidth(1.0f);
				glUniform1i(m_scene_rendering_program["uniform_ds_inv"], 0);
				//

				// plane //
				glUniform3f(m_scene_rendering_program["uniform_diffuse"], 0.4,0.4,0.4);
				glUniform1f(m_scene_rendering_program["uniform_alpha"], 1.f);
				draw::Arc_arbitr_plane(shape.a,
									   shape.length + 0.001f, 0, Geometry::GENERAL::toRadians(360.0f),
									   normalize(u), normalize(v),
									   40, 0, GL_POLYGON);
				glUniform1f(m_scene_rendering_program["uniform_alpha"], 1.f);
				//
			}

			// sphere
			if( draw_sphere )
			{
				
				//

				model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(shape.a.x, shape.a.y, shape.a.z))
					* glm::scale(glm::mat4(1.0f), glm::vec3(shape.length));
				normal_matrix = glm::transpose(glm::inverse(normal_matrix));

				glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
				glUniformMatrix4fv(m_scene_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(normal_matrix));

				//
				glUniform3f(m_scene_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
				glUniform1f(m_scene_rendering_program["uniform_alpha"], 0.25f);
				glBindVertexArray(m_sphere_node->m_vao);
				for (int i = 0; i < m_sphere_node->parts.size(); i++)
					glDrawArrays(GL_TRIANGLES, m_sphere_node->parts[i].start_offset, m_sphere_node->parts[i].count);
				glUniform1f(m_scene_rendering_program["uniform_alpha"], 1.f);
				//

				//
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glUniform3f(m_scene_rendering_program["uniform_diffuse"], 0, 0, 0);
				glUniform1f(m_scene_rendering_program["uniform_alpha"], 1.f);
				for (int i = 0; i < m_sphere_node->parts.size(); i++)
					glDrawArrays(GL_TRIANGLES, m_sphere_node->parts[i].start_offset, m_sphere_node->parts[i].count);
				glUniform1f(m_scene_rendering_program["uniform_alpha"], 1.f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				//
				glBindVertexArray(0);
			}


			

		}

		glUniform1f(m_scene_rendering_program["unfirom_alpha"], 1.0f);
		glUniform1i(m_scene_rendering_program["uniform_rendering_distance_sphere"], 0);
		glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);

		glDisable(GL_BLEND);
	}
	//
	
	if(vp.render_settings.wireframe_mode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#endif

	m_scene_first_pass = false;

}
void OpenGLRenderer::Render_Scene_Light_Objects(const VIEWPORT& vp )
{
	return;

	//if( init )
	{
		Render_Settings vp_settings = vp.render_settings;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		//if( bind ) 
			m_scene_rendering_program.Bind();

		glUniform1i(m_scene_rendering_program["diffuse_texture"], 0);
		glActiveTexture(GL_TEXTURE0);

		glUniform1i(m_scene_rendering_program["uniform_Material_textureArray"], 1);
		glActiveTexture(GL_TEXTURE1);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(3.0f);

		// bind Viewport's framebuffer
		//if ( bind_fbo )
		{
			glBindFramebuffer(GL_FRAMEBUFFER, ViewportManager::GetInstance().getFbo(vp.id));
			GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, drawBuffers);
		}

		// update shader's uniforms
		{
			PinholeCamera camera = ViewportManager::GetInstance().getViewportCamera(vp.id);
			optix::float3 cam_pos,  camera_up, w;
			//camera.getFrustum(cam_pos, u, camera_up, w, false);
			optix::float3 center = camera.getOrbitCenter();
			cam_pos = camera.getCameraPos();
			camera_up = camera.getV();
			w = camera.getW();
			glm::vec3 pos = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
			glm::vec3 up = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
			glm::vec3 target = pos + VECTOR_TYPE_TRANSFORM::TO_GLM_3f(normalize(w)) * 10.0f;
			glm::mat4 scene_view_matrix = glm::lookAt(pos, target, up);

			//glm::mat4 proj = glm::ortho( 0.0f , vp.renderSize.x , 0.0f, vp.renderSize.y , 0.0f , FAR );
			glm::mat4 m_projection_matrix = glm::perspective(glm::radians(M_FOV), vp.renderSize.x / vp.renderSize.y, NEAR, FAR);
			glUniformMatrix4fv(m_scene_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(scene_view_matrix));
			glUniformMatrix4fv(m_scene_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
			glUniform3f(m_scene_rendering_program["uniform_camera_position"], cam_pos.x, cam_pos.y, cam_pos.z);
			glUniform3f(m_scene_rendering_program["uniform_camera_w"], w.x, w.y, w.z);
			glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
		}


	}

	std::vector<SceneObject*> objects = Mediator::RequestSceneObjects();
	for (SceneObject* obj : objects)
	{
		if ( obj->getType() != LIGHT_OBJECT )
			continue;

		glm::mat4 model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getTransformationMatrix());
		Render_GeometryNode_Arrays( obj , Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_matrix, m_scene_rendering_program );

	}


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(UTILITY_LINE_WIDTH);

	//if (bind)
	{
		m_scene_rendering_program.Unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	

}
void OpenGLRenderer::Render_SceneObject_RenderList(const VIEWPORT& vp)
{

	{
		Render_Settings vp_settings = vp.render_settings;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// bind scene rendering program //
		m_scene_rendering_program.Bind();

		glUniform1i(m_scene_rendering_program["diffuse_texture"], 0);
		glActiveTexture(GL_TEXTURE0);

		glUniform1i(m_scene_rendering_program["uniform_Material_textureArray"], 1);
		glActiveTexture(GL_TEXTURE1);

		//switch (vp_settings.shading_mode)
		//{
		//case Shading_Mode::DEFAULT_SHADING: {   glUniform1i(m_scene_rendering_program["uniform_shading_mode"], 0);  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);   } break;
		//case Shading_Mode::GRAY_SHADING: {   glUniform1i(m_scene_rendering_program["uniform_shading_mode"], 1);  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);   } break;
		//case Shading_Mode::WIREFRAME_SHADING: {   glUniform1i(m_scene_rendering_program["uniform_shading_mode"], 2);  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);   } break;
		//}
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(3.0f);

		// bind Viewport's framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, ViewportManager::GetInstance().getFbo_Utility(vp.id));
		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);

		// update shader's uniforms
		{
			PinholeCamera camera = ViewportManager::GetInstance().getViewportCamera(vp.id);
			optix::float3 cam_pos, camera_up, w;
			//camera.getFrustum(cam_pos, u, camera_up, w, false);
			optix::float3 center = camera.getOrbitCenter();
			cam_pos = camera.getCameraPos();
			camera_up = camera.getV();
			w = camera.getW();
			glm::vec3 pos = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
			glm::vec3 up = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
			glm::vec3 target = pos + VECTOR_TYPE_TRANSFORM::TO_GLM_3f(normalize(w)) * 10.0f;
			glm::mat4 scene_view_matrix = glm::lookAt(pos, target, up);

			//glm::mat4 proj = glm::ortho( 0.0f , vp.renderSize.x , 0.0f, vp.renderSize.y , 0.0f , FAR );
			glm::mat4 m_projection_matrix = glm::perspective(glm::radians(M_FOV), vp.renderSize.x / vp.renderSize.y, NEAR, FAR);
			glUniformMatrix4fv(m_scene_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(scene_view_matrix));
			glUniformMatrix4fv(m_scene_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
			glUniform3f(m_scene_rendering_program["uniform_camera_position"], cam_pos.x, cam_pos.y, cam_pos.z);
			glUniform3f(m_scene_rendering_program["uniform_camera_w"], w.x, w.y, w.z);
			glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
		}


	}

	for (SceneObject_RenderList_Object obj : m_SceneObject_RenderList)
	{
		glm::mat4 model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj.obj_pack->raw_object->getTransformationMatrix());
		Render_GeometryNode(obj.obj_pack->raw_object, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj.obj_pack->raw_object), model_matrix, m_scene_rendering_program);
	}

	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(UTILITY_LINE_WIDTH);

		// unbind scene rendering program //
		m_scene_rendering_program.Unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}
void OpenGLRenderer::postProcessScene(GLuint m_fbo, GLuint m_scene_tex)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	//glPushAttrib(GL_ALL_ATTRIB_BITS);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	m_postprocess_program.Bind();

	glUniform1i(m_postprocess_program["apply_postproc_effects"], 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_scene_tex);
	glUniform1i(m_postprocess_program["samplerHDR"], 0);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_postprocess_program["apply_postproc_effects"], 0);


	m_postprocess_program.Unbind();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//glPopAttrib();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

}
void OpenGLRenderer::apply_FXAA(GLuint m_fbo, GLuint m_scene_tex)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	//glPushAttrib(GL_ALL_ATTRIB_BITS);

	//glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//glDisable(GL_BLEND);

	m_postproc_fxaa_program.Bind();


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_scene_tex);
	glUniform1i(m_postproc_fxaa_program["samplerHDR"], 0);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 1.0f);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);

	m_postproc_fxaa_program.Unbind();

	//glPopAttrib();
}

void OpenGLRenderer::Render_Cursor_toScreen()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (m_render_cursor == 1)
	{
		optix::float2 mouse_pos = optix::make_float2(Mediator::RequestMousePos(0), Mediator::RequestMousePos(1));
		optix::float2 window = optix::make_float2(Mediator::RequestWindowSize(0), Mediator::RequestWindowSize(1));
		glViewport(0, 0, window.x, window.y);

		draw::Cursor(mouse_pos, window, m_cursor_to_render, &m_cursor_rendering_program);
		glViewport(0, 0, 0, 0);
	}

	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void OpenGLRenderer::Render_Cursor_toScreen_()
{

}
void OpenGLRenderer::Render_Viewports_Overlay()
{

	glViewport(0, 0, Mediator::RequestWindowSize(0), Mediator::RequestWindowSize(1));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	float line_width = 2.0f;

	glm::vec3 active_color   = glm::vec3(0.49, 0.41, 0.17);
	glm::vec3 inactive_color = glm::vec3(0.25f); // 0.20

	
	glLineWidth(1.0f);

	int window_width  = Mediator::RequestWindowSize(0);
	int window_height = Mediator::RequestWindowSize(1);

	glm::vec3 outline_color;
	std::vector<VIEWPORT> viewports = ViewportManager::GetInstance().getAllEnabledViewports();
	
	for (VIEWPORT vp : viewports)
	{
		if (!vp.is_active)
			continue;

		glm::vec2 a = glm::vec2(vp.dimensions.offset.x, window_height - (vp.dimensions.offset.y + vp.renderSize.y)) - glm::vec2(1.0f, 0.0f);
		glm::vec2 b = a + glm::vec2(vp.renderSize.x, vp.renderSize.y) + glm::vec2(2.f, 1.0f);

		outline_color = inactive_color;
		draw::Rectangle2D(a, b, outline_color, line_width, false, 0);
	}
	

	glDisable(GL_DEPTH_TEST);
	VIEWPORT vp = ViewportManager::GetInstance().getActiveViewport();
	glm::vec2 a = glm::vec2(vp.dimensions.offset.x, window_height - (vp.dimensions.offset.y + vp.renderSize.y)) - glm::vec2(1.0f, 0.0f);
	glm::vec2 b = a + glm::vec2(vp.renderSize.x, vp.renderSize.y) + glm::vec2(2.f, 1.0f);
	outline_color = active_color;
	int render_sides[4] = { vp.render_sides[0] + line_width,vp.render_sides[1] + line_width,vp.render_sides[2] + line_width,vp.render_sides[3] + line_width };
	draw::Rectangle2D( a, b, outline_color, line_width, false, render_sides );

	glViewport(0, 0, 0, 0);
	glLineWidth(1.0f);
	
}

//
void OpenGLRenderer::Render_Utilities(const VIEWPORT& vp)
{
	
	Clear_Fbo(ViewportManager::GetInstance().getFbo_Utility(vp.id),   true, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	Clear_Fbo(ViewportManager::GetInstance().getFbo_Utility_2(vp.id), true, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	//Clear_Fbo(ViewportManager::GetInstance().getFbo_2(vp.id), true, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));

	Render_Viewport_Shapes(vp, true, true, true); // render shapes - 1st pass 
	Render_Viewport_Shapes(vp, true);             // render shapes - 1st pass ( spheres only )

	Render_Utility_RenderList(vp);     // Render Utility objects
	Render_Viewport_Shapes(vp, false); // render shapes - 2nd pass
	Render_Camera_Orbit_Center(vp);    // Render Camera Orbit Center 
	Render_Highlight_RenderList(vp);   // Render Highlighted objects

	Render_Shapes2D(vp);  // Render Windows 2d shapes ( area selection, etc ... )        
	Render_View_Axis(vp); // Render Camera axis
	Render_Viewport_Camera_Orbit_Handle(vp);

}
void OpenGLRenderer::Render_Utility_RenderList(const VIEWPORT& vp)
{
	
	Shader_start(vp, 0);

	PinholeCamera camera = ViewportManager::GetInstance().getViewportCamera(vp.id);
	Update_Camera(camera);

	glm::mat4 m_projection_matrix = glm::perspective(glm::radians(M_FOV), vp.renderSize.x / vp.renderSize.y, NEAR, FAR);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));

	

	for (Utility_RenderList_Object utility_to_Render : m_Utility_RenderList)
	{

		//Update_Shader_Utility_To_Render(&utility_to_Render, vp);
		Update_OBJECT_Matrices( utility_to_Render, ViewportManager::GetInstance().getViewportCamera(vp.id), vp );
		Update_Shader_Uniforms( utility_to_Render );

		
		// case : g_TRANSFORMATIONS
		if (utility_to_Render.ut.group == g_TRANSFORMATIONS)
		{

			switch (utility_to_Render.ut.id)
			{

			case u_TRANSLATION:
				Render_Utility_Translation(utility_to_Render, vp);
				break;

			case u_ROTATION:
				Render_Utility_Rotation(utility_to_Render, vp);
				break;

			case u_SCALE:
				Render_Utility_Scale(utility_to_Render, vp);
				break;

			}
		}
		
		// case : g_LINKING
		else if (utility_to_Render.ut.group == g_LINKING)
		{

			switch (utility_to_Render.ut.id)
			{

			case u_LINK:
				Render_Utility_Link(utility_to_Render, vp);
				break;

			case u_UNLINK:
				// Render_Utility_Unlink(utility_to_Render);
				break;

			}
		}

		// case : g_ATTACH
		else if (utility_to_Render.ut.group == g_ATTACH)
		{
			////std::cout << "\n - utility: g_ATTACH!" << std::endl;
			////std::cout << " - obj : " << utility_to_Render.obj_pack->raw_object->getName() << std::endl;
			////std::cout << " - is_temp : " << utility_to_Render.obj_pack->raw_object->isTemporary() << std::endl;
			switch (utility_to_Render.ut.id)
			{

			case u_ATTACH_SAMPLER_PLANAR:
				Render_Utility_Attach_Sampler_Planar(utility_to_Render, vp);
				break;

			case u_ATTACH_SAMPLER_VOLUME:
				Render_Utility_Attach_Sampler_Volume(utility_to_Render, vp);
				break;

			case u_ATTACH_SAMPLER_DIRECTIONAL:
				Render_Utility_Attach_Sampler_Directional(utility_to_Render, vp);
				break;

			case u_ATTACH_FACE_VECTOR_BBOX_ALIGNED:
				Render_Utility_Attach_Face_Vector(utility_to_Render, vp);
				break;

			case u_ATTACH_FACE_VECTOR_BBOX_ALIGNED_PARALLEL:
				Render_Utility_Attach_Face_Vector_Parallel(utility_to_Render, vp);
				break;

			case u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED:
				Render_Utility_Attach_Face_vector_Normal_Aligned(utility_to_Render, vp);
				break;

			case u_ATTACH_SAMPLER:
				Render_Utility_Attach_Sampler(utility_to_Render);
				break;
			}
		}
		
		else if (utility_to_Render.ut.group == g_CREATE)
		{
			switch (utility_to_Render.ut.id)
			{
			case u_CREATE_LIGHT:
				Render_Utility_Create_Light(utility_to_Render, vp);
				break;
			}
		}

		// case : Restrictions
		else if (utility_to_Render.ut.group == g_RESTRICTIONS)
		{
			switch (utility_to_Render.ut.id)
			{
			case u_ATTACH_RESTRICTION_FOCUS:
				Render_Utility_Attach_Restriction_Focus(utility_to_Render, vp);
				break;

				case u_ATTACH_RESTRICTION_CONVERSATION:
					Render_Utility_Attach_Restriction_Conversation(utility_to_Render, vp);
				break;

				case u_ATTACH_RESTRICTION_DISTANCE:
					Render_Utility_Attach_Restriction_Distance(utility_to_Render, vp);
				break;
			}
		}

		// case : Align
		else if (utility_to_Render.ut.group == g_ALIGN)
		{
			switch (utility_to_Render.ut.id)
			{

			case u_ALIGN:
				Render_Utility_Object_Align(utility_to_Render, vp);
				break;

			case u_ALIGN_CAMERA:
				Render_Utility_Camera_Align(utility_to_Render, vp);
				break;

			
			}
		}

		// case : g_NONE ( render types )
		else if (utility_to_Render.ut.group == g_NONE)
		{
			switch (utility_to_Render.render_type)
			{

			case RENDER_LIGHT_OBJECT:
				Render_Light_Object(utility_to_Render);
				break;

			case RENDER_LIGHT_OBJECT_PROPERTIES:
				Render_Light_Object_Properties(utility_to_Render,vp);
				break;

			}
		}
	}

	Shader_end();

}
void OpenGLRenderer::Render_Highlight_RenderList(const VIEWPORT& vp)
{
	Shader_start(vp, 1);

	for (Highlight_RenderList_Object outline_to_Render : m_Highlight_RenderList)
	{
#ifdef CANCER_DEBUG
		if (outline_to_Render.obj_pack.raw_object == nullptr || outline_to_Render.obj_pack.object_index < 0)
		{
			//std::cout << "\n Render_Highlight_RenderList( vp ):" << std::endl;
			//std::cout << " outline_to_Render.obj_pack->object_index < 0!" << std::endl;
			//std::cout << " outline_to_Render.obj_pack->raw_object == nullptr " << std::endl;
		}
#endif
		Object_Package obj_pack = outline_to_Render.obj_pack;
		int request_obj_index = obj_pack.object_index;
		int real_obj_index = Mediator::requestObjectsArrayIndex(obj_pack.raw_object->getId());
		if (real_obj_index != request_obj_index || real_obj_index == -1)
		{

   #define PRINT_DEBUG_INFO_CLONE_DELETE2
#ifdef PRINT_DEBUG_INFO_CLONE_DELETE2
			//std::cout << "\n\nRender_Highlight_RenderList():" << std::endl;
			//std::cout << "      - m_scene_objets.size()       = " << Mediator::RequestSceneObjects().size() << std::endl;
			//std::cout << "      - legal index bounds          : " << "[0," << (Mediator::RequestSceneObjects().size() - 1) << "]" << std::endl;
			//std::cout << "      - caught illegal object_index : " << request_obj_index << std::endl;
			//std::cout << "      - object's real index         : " << real_obj_index << std::endl;
			//std::cout << "      - object's id :               : " << obj_pack.raw_object->getId() << std::endl;
			////std::cout << "      - object->isNull()            = " << (obj_pack.raw_object == nullptr) << std::endl;
#endif
			continue;
		}

		Update_Shader_Highlight_To_Render(&outline_to_Render, vp);

		Type obj_type = obj_pack.raw_object->getType();
		bool is_s_directional = obj_type == SAMPLER_OBJECT ? obj_pack.raw_object->get_SamplerParameters().type == s_DIRECTIONAL : false;

		if (    obj_type == IMPORTED_OBJECT 
			||  obj_type == LIGHT_OBJECT 
			|| (obj_type == SAMPLER_OBJECT && !is_s_directional)
			)
			Render_Object_Outline(outline_to_Render.obj_pack.object_index, outline_to_Render.obj_pack.selected, vp);
		else if (obj_type == GROUP_PARENT_DUMMY || obj_type == DUMMY)
			Render_Group_Outline(outline_to_Render.obj_pack.object_index, outline_to_Render.obj_pack.selected, vp);
		else if ( obj_type == FACE_VECTOR )
			Render_FaceVector_Outline( outline_to_Render, outline_to_Render.obj_pack.object_index, outline_to_Render.obj_pack.selected , vp);
		else if (obj_type == SAMPLER_OBJECT && is_s_directional)
		{
			//Render_Sampler_Outline(outline_to_Render, outline_to_Render.obj_pack.object_index, outline_to_Render.obj_pack.selected, vp);
		}

	}

	Shader_end();


}
void OpenGLRenderer::Render_Camera_Orbit_Center(const VIEWPORT& vp)
{
	Shader_start(vp, 0);
	
	PinholeCamera camera = ViewportManager::GetInstance().getViewportCamera(vp.id);
	if (camera.isChanged(true) && camera.getState() == CAM_ORBIT)
		draw::CameraFocus(camera, vp, M_FOCUS_SPHERE_SIZE, glm::vec3(0.25f));
	ViewportManager::GetInstance().getViewportCamera(vp.id).reset();

	Shader_end();
}
void OpenGLRenderer::Render_View_Axis(const VIEWPORT& vp)
{
	GLuint fbo = ViewportManager::GetInstance().getFbo_Utility( vp.id );
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	PinholeCamera camera_instance = ViewportManager::GetInstance().getViewportCamera(vp.id);
	float vp_scale_Factor = vp.scale_factor;
	optix::float3 old_pos;
	// define the bottom left point of the current viewport //
	{

		glm::vec2 vp_size = glm::vec2(vp.renderSize.x, vp.renderSize.y);
		glm::vec2 vp_off = glm::vec2(vp.dimensions.offset.x, vp.dimensions.offset.y);

		optix::float3 cam_pos = camera_instance.getCameraPos();
		old_pos = cam_pos;
		cam_pos = ZERO_3f;
		camera_instance.setCameraPos(cam_pos);
		optix::float3 camera_up = normalize(camera_instance.getV());
		optix::float3 m_camera_w = normalize(camera_instance.getW());
		optix::float3 m_camera_u = normalize(camera_instance.getU());
		glm::vec3 pos = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
		glm::vec3 up = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
		glm::vec3 target = pos + VECTOR_TYPE_TRANSFORM::TO_GLM_3f(normalize(m_camera_w)) * 1.f;
		glm::mat4 view_matrix = glm::lookAt(pos, target, up);

		float aspect = vp.renderSize.x / vp.renderSize.y;
		glm::mat4 projection = glm::perspective(M_FOV, aspect, NEAR, FAR);


		//
		glm::mat4 proj_view = projection * view_matrix;
		glm::mat4 inv_proj_view = glm::inverse(proj_view);

		float offset = fabsf(vp_scale_Factor - 1.0f) / 2.f;
		glm::vec2 vp_rel_offset = glm::vec2(offset);
		vp_rel_offset = aspect > 0.0f ? offset * glm::vec2(1.0f, aspect) : offset * glm::vec2(aspect, aspect);


		glm::vec2 portion = glm::vec2(6.6f, 6.6f) - vp_rel_offset;
		glm::vec2 p = glm::vec2(vp.renderSize.x / portion.x, vp.renderSize.y - vp.renderSize.y / portion.y);

		p.x = 2.0f * (p.x / (vp.renderSize.x)) - 1.0f;
		p.y = -2.0f * (p.y / (vp.renderSize.y)) + 1.0f;
		glm::vec4 _p = glm::vec4(p.x, p.y, -0.7f, 1.0f);
		_p = inv_proj_view * _p;
		_p *= 1.0f / _p.w;

		//
		m_utility_rendering_program.Bind();


		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(_p)) * glm::scale(glm::mat4(1.0f), glm::vec3(vp_scale_Factor));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(view_matrix));

		glLineWidth(1.0f);


		for (int i = 0; i < 3; i++)
		{
			optix::float3 p0 = ZERO_3f;
			optix::float3 p1 = p0 + GLOBAL_AXIS[i] * 0.0023f; // *vp_scale_Factor;
			optix::float3 p2 = p0 + GLOBAL_AXIS[i] * 0.0025f; // *vp_scale_Factor;

			glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR[i].x*0.6f, DEF_AXIS_COLOR[i].y*0.6f, DEF_AXIS_COLOR[i].z*0.6f);
			draw::Line3D(p0, p1);

			glUniform1i(m_utility_rendering_program["uniform_text_render"], 1);
			draw::Text(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p2), glm::vec3(1), FONT3, GLOBAL_AXIS_NAME[i]);
			glUniform1i(m_utility_rendering_program["uniform_text_render"], 0);
		}


		glLineWidth(1.0f);

		m_utility_rendering_program.Unbind();
	}

	camera_instance.setCameraPos(old_pos);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void OpenGLRenderer::Render_Viewport_Camera_Orbit_Handle(const VIEWPORT& vp)
{
	//return;

	GLuint fbo = ViewportManager::GetInstance().getFbo_Camera_Orbit_2(vp.id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	
	glViewport(0, 0, 100, 100);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Clear_Fbo(fbo, true, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);
	//glEnable(GL_BLEND);
	//

	PinholeCamera camera_instance = ViewportManager::GetInstance().getViewportCamera( vp.id );
	float vp_scale_Factor     = vp.scale_factor;
	optix::float3 cam_old_pos = camera_instance.getCameraPos();
	optix::float3 cam_pos     = ZERO_3f;
	
	camera_instance.setCameraPos( cam_pos );
	camera_instance.setViewport(100.0f, 100.0f);
	camera_instance.getFrustum();
	optix::float3 camera_up  = normalize(camera_instance.getV());
	optix::float3 m_camera_w = normalize(camera_instance.getW());
	optix::float3 m_camera_u = normalize(camera_instance.getU());
	optix::float3 m_camera_v = normalize(camera_instance.getV());
	glm::vec3 pos = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
	glm::vec3 up  = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
	glm::vec3 target      = pos + VECTOR_TYPE_TRANSFORM::TO_GLM_3f(normalize(m_camera_w)) * 1.f;
	glm::mat4 view_matrix; // = glm::lookAt(pos, target, -up);
	glm::mat4 projection  = glm::perspective( glm::radians(42.0f) , 1.0f, 0.1f, 1500.0f ); //* glm::scale(glm::mat4(1.0f), glm::vec3(1,1,1));
	
	{
		glm::vec3 center = target;
		glm::vec3 eye    = pos;
		glm::vec3 f(normalize(center - eye));
		glm::vec3 s(normalize(cross(f, up)));
		glm::vec3 u(cross(s, f));
		view_matrix[0][0] = s.x;
		view_matrix[1][0] = s.y;
		view_matrix[2][0] = s.z;
		view_matrix[0][1] = u.x;
		view_matrix[1][1] = u.y;
		view_matrix[2][1] = u.z;
		view_matrix[0][2] = -f.x;
		view_matrix[1][2] = -f.y;
		view_matrix[2][2] = -f.z;
		view_matrix[3][0] = -dot(s, eye);
		view_matrix[3][1] = -dot(u, eye);
		view_matrix[3][2] = dot(f, eye);
	}

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 normal_matrix = glm::mat4(1.0f);
	glm::vec3 color;
	optix::float3 axis[3] = { GLOBAL_AXIS[0],GLOBAL_AXIS[1],GLOBAL_AXIS[2] };

	//bool is_vp_active  = vp.is_active;
	bool hovering_over = (InterfaceManager::GetInstance().is_Hovering_Over_Camera_Orbit() || InterfaceManager::GetInstance().is_Camera_Orbit_Active()) && vp.is_active;

  //#define draw_camera_orbit_cube_ImmediateMode
#ifdef draw_camera_orbit_cube_ImmediateMode
	

	//
	m_utility_rendering_program.Bind();
	glUniformMatrix4fv(m_utility_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(m_utility_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(projection));


	// cube
	glm::vec3 p = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(cam_pos + normalize(m_camera_w) * 20.0f);
	model       = glm::translate(glm::mat4(1.0f), p);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model));


	color = glm::vec3(0.89f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	draw::Cube(ZERO_3f + GLOBAL_AXIS[1] * 2.85f , 5.0f, axis, true);

	color = glm::vec3(0.1f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	glLineWidth(1.0f);
	draw::Cube(ZERO_3f + GLOBAL_AXIS[1] * 2.85f, 5.0f, axis, false);
	glLineWidth(1.0f);
	//

	// plane
	optix::float2 scale = optix::make_float2(5.0f);
	color = glm::vec3(0.71f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	draw::Plane_Centered(ZERO_3f, GLOBAL_AXIS[0], GLOBAL_AXIS[2], scale, 1.0f, GL_POLYGON);

	//
	m_utility_rendering_program.Unbind();

#else


	// bind scene rendering program //
	m_scene_rendering_program.Bind();
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
	glUniform1i(m_scene_rendering_program["uniform_is_instance"], 0);
	

	optix::float3 cpos = cam_pos - normalize(m_camera_w) * 1000.0f;
	glUniform3f(m_scene_rendering_program["uniform_camera_position"], cpos.x, cpos.y, cpos.z);
	glUniform3f(m_scene_rendering_program["uniform_camera_w"], m_camera_w.x, m_camera_w.y, m_camera_w.z);


	glUniformMatrix4fv(m_scene_rendering_program["uniform_view_matrix"],       1, GL_FALSE, glm::value_ptr(view_matrix) );
	glUniformMatrix4fv(m_scene_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(projection)  );


	// CUBE
	glUniform1i(m_scene_rendering_program["uniform_is_camera_orbit"], 1);
	glm::vec3 p   = VECTOR_TYPE_TRANSFORM::TO_GLM_3f( cam_pos + normalize(m_camera_w) * 18.0f) + VECTOR_TYPE_TRANSFORM::TO_GLM_3f(GLOBAL_AXIS[1] * 0.1f);
	model         = glm::translate(glm::mat4(1.0f), p ) * glm::scale(glm::mat4(1.0f), glm::vec3(5.f));
	normal_matrix = model;
	glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"],  1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(m_scene_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(normal_matrix))));
	color = hovering_over? glm::vec3(0.7f) : glm::vec3(0.45f);
	glUniform3f(m_scene_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	GeometryNode * node = m_cube_node;
	node->bind_Vao();
	node->Draw();
	node->unbind_Vao();
	glUniform1i(m_scene_rendering_program["uniform_is_camera_orbit"], 0);
	
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 1);
	glLineWidth(3.0f);
	color = glm::vec3(0.35f);
	glUniform3f(m_scene_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	draw::Cube(ZERO_3f + GLOBAL_AXIS[1] * 0.5f, optix::make_float3(1.0f), axis, false);
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
	glLineWidth(1.0f);

	
#ifdef immediate_mode_Plane
	// plane
	glDisable(GL_CULL_FACE);
	model = glm::translate(glm::mat4(1.0f), p - VECTOR_TYPE_TRANSFORM::TO_GLM_3f(GLOBAL_AXIS[1] * 0.1f));
	glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model));

	optix::float2 scale = optix::make_float2(5.0f);
	color = glm::vec3(0.81f);
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 1);
	glUniform3f(m_scene_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(m_scene_rendering_program["uniform_alpha"], 1.0f);
	draw::Plane_Centered(ZERO_3f, GLOBAL_AXIS[0], GLOBAL_AXIS[2], scale, 1.0f, GL_POLYGON);
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
	glEnable(GL_CULL_FACE);
	//
#else



	model = glm::translate(glm::mat4(1.0f), p - VECTOR_TYPE_TRANSFORM::TO_GLM_3f(GLOBAL_AXIS[1] * 0.1f)) * glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 1.0f, 5.0f));
	normal_matrix = model;
	glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(m_scene_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(normal_matrix))));

	glUniform1i(m_scene_rendering_program["uniform_is_camera_orbit"], 1);
	color = hovering_over ? glm::vec3(0.75f) : glm::vec3(0.45f);
	glUniform3f(m_scene_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	node = m_disk_node;
	node->bind_Vao();
	node->Draw();
	node->unbind_Vao();


	color = hovering_over ? glm::vec3(0.95f) : glm::vec3(0.65f);
	glUniform3f(m_scene_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	node = m_disk_nswe_node;
	node->bind_Vao();
	node->Draw();
	node->unbind_Vao();
	
	glUniform1i(m_scene_rendering_program["uniform_is_camera_orbit"], 0);

#endif

	//
	//glDisable(GL_DEPTH_TEST);
	model = glm::translate(glm::mat4(1.0f), p) * glm::scale(glm::mat4(1.0f), glm::vec3(1.f));
	normal_matrix = model;
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 1);
	glUniformMatrix4fv(m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(m_scene_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(normal_matrix))));
	color = hovering_over ? glm::vec3(0.0f) : glm::vec3(0.0f);
	glUniform3f(m_scene_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	node = m_cube_text_node;
	node->bind_Vao();
	node->Draw();
	node->unbind_Vao();
	glUniform1i(m_scene_rendering_program["uniform_is_camera_orbit"], 0);
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);



	//
	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
	glUniform1i(m_scene_rendering_program["uniform_is_camera_orbit"], 0);
	glUniform1i(m_scene_rendering_program["uniform_is_instance"], 1);
	m_scene_rendering_program.Unbind();



#endif
	
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	
	//



	// Invert Pixels //
	/*
	GLuint tex_from = ViewportManager::GetInstance().getFboTexture_Camera_Orbit_2(vp.id);
	GLuint tex_to   = ViewportManager::GetInstance().getFboTexture_Camera_Orbit(vp.id);
	
	const int width  = 100;
	const int height = 100;
	const int Bytes  = 4;
	const int SIZE   = width * height * Bytes * sizeof(BYTE);
	char * pixels = new char[ SIZE ];
	glBindTexture(GL_TEXTURE_2D, tex_from);
	glGetTexImage(GL_TEXTURE_2D,
				  0,
				  GL_RGBA,
				  GL_UNSIGNED_BYTE,
				  (void*)pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	//glTexSubImage2D()
	char * pixels_inverted = new char[ SIZE ];
	int k = 0;
	for (int i = SIZE - 1; i >= 0; i--)
	{
		unsigned char c = (unsigned char)pixels[i];
		pixels_inverted[k] = c;
		k++;
	}
	glBindTexture(GL_TEXTURE_2D, tex_to);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)pixels_inverted);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete pixels;
	delete pixels_inverted;
	// */
	GLuint tex_from = ViewportManager::GetInstance().getFboTexture_Camera_Orbit_2(vp.id);
	GLuint tex_to   = ViewportManager::GetInstance().getFboTexture_Camera_Orbit(vp.id);

	RenderToOutFB(
		ViewportManager::GetInstance().getFbo_Camera_Orbit(vp.id),
		tex_from,
		0,
		0,
		0,
		1,
		0,
		0,
		0,
		1);


	glViewport(0, 0, 0, 0);

	glDisable(GL_BLEND);
}

void OpenGLRenderer::Render_Shapes2D( const VIEWPORT& vp )
{
	if ( !vp.is_active )
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, ViewportManager::GetInstance().getFbo_Utility(vp.id));
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	for (Shape_RenderList_Object obj : m_shapes2D_RenderList)
	{
		Shape shape = obj.shape;
		switch (obj.shape_id)
		{


		case LINE:
		{
			if (shape.use_shader)
			{
				m_utility_rendering_program.Bind();
			}

			
			glLineWidth(shape.thickness);
			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr( model_matrix ));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			if (shape.stripped)
				draw::stripped_Line3D(shape.a, shape.b, 1.0f);
			else
				draw::Line3D(shape.a, shape.b);
			if (shape.draw_arrow)
				draw::Arrow(shape.a, shape.b, shape.arrow_scale_factor , false);

			
			if (shape.use_shader)
				m_utility_rendering_program.Unbind();

			glLineWidth(1.0f);


		}break;

		case TRIANGLE:
		{

		}break;

		case CIRCLE:
		{

		}break;

		case RECTANGLE:
		{
			draw::Rectangle2D( obj.shape.a, obj.shape.b, glm::vec3(1.0f), 1.0f, &m_cursor_rendering_program, vp, true );

		}break;

		case CUBE:
		{
			if (shape.use_shader)
				m_utility_rendering_program.Bind();

			glLineWidth(shape.thickness);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
			draw::Cube(shape.pos, shape.size, shape.axis, false);
			glLineWidth(1.0f);
			if (shape.use_shader)
				m_utility_rendering_program.Unbind();

		}break;


		}
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void OpenGLRenderer::Render_Viewport_Shapes( const VIEWPORT& vp, bool first_phase, bool only_trans_spheres, bool draw_trans_spheres )
{
	////std::cout << "\n - Render_Viewport_Shapes( vp = " << vp.id << " , first_phase = " << first_phase << " , spheres_phase = " << only_trans_spheres << " , draw_spheres = " << draw_trans_spheres << " ) " << std::endl;
	std::vector<Shape> vp_Shapes = m_Shapes2D_per_Viewport[vp.id];
	
	GLuint vp_Fbo;
	if (first_phase && !only_trans_spheres)
	{
		//if (vp.renderer == OPTIX_RENDERER) 
		//	vp_Fbo = ViewportManager::GetInstance().getFbo(vp.id);
		//else
			vp_Fbo = ViewportManager::GetInstance().getFbo_Utility_2( vp.id );
	}
	else
		vp_Fbo = ViewportManager::GetInstance().getFbo_Utility( vp.id );


	if ( first_phase )
	{
		Shader_start(vp, vp_Fbo, 0);
		// update camera //
		{
			Update_Camera(ViewportManager::GetInstance().getViewportCamera(vp.id), true);
			glm::mat4 projection_matrix = glm::perspective(glm::radians(M_FOV), vp.renderSize.x / vp.renderSize.y, NEAR, FAR);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(projection_matrix));
			m_utility_rendering_program.Unbind();
		}

	}

	//if ( !first_phase )
	{
		glBindFramebuffer(GL_FRAMEBUFFER, vp_Fbo);
		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	bool is_g_attach = interface_state->active_utility->group == g_ATTACH;

	for (Shape shape : vp_Shapes)
	{

		if ( first_phase && !only_trans_spheres )
		{
			if (
				   shape.shape_id != SPHERE 
				   
				&&((shape.shape_id != PLANE_CENTERED
				&& shape.shape_id != PLANE
				&& shape.shape_id != SAMPLER_CUBE
				&& shape.shape_id != SAMPLER_CUBE_NOT_CENTERED)
				|| is_g_attach
				)

				&& shape.shape_id != ARC
				&& (shape.shape_id == LINE && (!shape.is_first_pass && !shape.is_directional))
				&& (shape.shape_id == CONE_FRUSTUM && (!shape.is_directional))
				)
				continue;
		}
		else if (first_phase && only_trans_spheres)
		{
			if (shape.shape_id != SPHERE) continue;
		}
		else if (!first_phase)
		{
			if (
				   shape.shape_id == SPHERE

				||(( shape.shape_id == PLANE_CENTERED
				|| shape.shape_id == PLANE
				|| shape.shape_id == SAMPLER_CUBE
				|| shape.shape_id == SAMPLER_CUBE_NOT_CENTERED)
				&& !is_g_attach
				)

				|| shape.shape_id == ARC
				|| shape.is_first_pass
				|| shape.is_directional
				)
				continue;
		}

		switch (shape.shape_id)
		{

		case S_TEXT:
		{

			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			m_utility_rendering_program.Bind();
			
			
			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			
			glUniform1i(m_utility_rendering_program["uniform_text_render"], 1);
			draw::Text(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(shape.a), glm::vec3(1.0f), shape.font, (char*)shape.label.c_str());
			glUniform1i(m_utility_rendering_program["uniform_text_render"], 0);

			m_utility_rendering_program.Unbind();

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

		}break;

		case S_POINT:
		{

			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			m_utility_rendering_program.Bind();

			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);

			draw::Point(shape.a, shape.scale_factor);
			
			m_utility_rendering_program.Unbind();

		}break;

		case LINE:
		{
			//glEnable(GL_LINE_SMOOTH);
			if (shape.gl_line_smooth) glEnable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			m_utility_rendering_program.Bind();
			glLineWidth(shape.thickness);

			//glm::mat4 model_matrix = glm::scale( glm::mat4(1.0f), glm::vec3( shape.scale_factor ) );
			glm::mat4 model_matrix = shape.use_trns? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], shape.alpha);
			
			glUniform1i(m_utility_rendering_program["uniform_mix_color"], shape.mix_color);
			if (shape.mix_color)
			{
				glUniform3f(m_utility_rendering_program["uniform_mcolor_a"], shape.m_color_a.x, shape.m_color_a.y, shape.m_color_a.z);
				glUniform3f(m_utility_rendering_program["uniform_mcolor_b"], shape.m_color_b.x, shape.m_color_b.y, shape.m_color_b.z);
				glUniform3f(m_utility_rendering_program["uniform_mix_p0"], shape.mix_p0.x, shape.mix_p0.y, shape.mix_p0.z);
				glUniform3f(m_utility_rendering_program["uniform_mix_p1"], shape.mix_p1.x, shape.mix_p1.y, shape.mix_p1.z);
			}
			if (!shape.only_arrow)
			{
				if (shape.stripped)
					draw::stripped_Line3D(shape.a, shape.b, shape.arrow_scale_factor, false, shape.thickness, shape.stripple_factor);
				else
					draw::Line3D(shape.a, shape.b, shape.thickness);
			}
			if(shape.draw_arrow || shape.only_arrow)
				draw::Arrow( shape.a, shape.b, shape.arrow_scale_factor, vp, false );
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
			glUniform1i(m_utility_rendering_program["uniform_mix_color"], 0);

			glLineWidth(1.0f);
			m_utility_rendering_program.Unbind();
			if (shape.gl_line_smooth) glDisable(GL_LINE_SMOOTH);

		}break;

		case TRIANGLE:
		{

		}break;

		case CIRCLE:
		{

			m_utility_rendering_program.Bind();
			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			draw::Circle(shape.a, shape.size , 40, shape.vertical , shape.thickness, shape.stripped);
			m_utility_rendering_program.Unbind();

		}break;

		case ARC:
		{
			if (shape.test_depth)
				glEnable(GL_DEPTH_TEST);
			if (shape.enum_type == GL_POLYGON)
				glDisable(GL_CULL_FACE);

			m_utility_rendering_program.Bind();

			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], shape.alpha);

			draw::Arc_arbitr_plane(shape.pos, shape.r, Geometry::GENERAL::toRadians(shape.s_rad), Geometry::GENERAL::toRadians(shape.e_rad), shape.axis[0], shape.axis[2], shape.length, shape.is_arc, shape.stripped, shape.stripple_factor, shape.thickness,  shape.enum_type);
			glLineWidth(1.0f);

			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);

			m_utility_rendering_program.Unbind();


			if (shape.test_depth)
				glDisable(GL_DEPTH_TEST);
			if (shape.enum_type == GL_POLYGON)
				glEnable(GL_CULL_FACE);

		}break;

		case RECTANGLE:
		{
			draw::Rectangle2D(shape.a, shape.b, glm::vec3(1.0f), 1.0f, &m_cursor_rendering_program, vp, true);

		}break;

		case PLANE:
		{

			glEnable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if (shape.is_outline)
			{
				glDisable(GL_DEPTH_TEST);
				//glEnable(GL_DEPTH_TEST);

			}
			else
			{
				//glDisable(GL_DEPTH_TEST);
				//glEnable(GL_DEPTH_TEST);
			}

			m_utility_rendering_program.Bind();
			glLineWidth(shape.thickness);

			float a = is_g_attach ? 0.5f : 1.0f;
			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], shape.alpha * a);

			shape.is_outline ? draw::Plane_(shape.pos, shape.axis[0], shape.axis[2], optix::make_float2(1.0f), shape.thickness, GL_LINE_LOOP)
				: draw::Plane_(shape.pos, shape.axis[0], shape.axis[2], optix::make_float2(1.0f), shape.thickness, GL_POLYGON);
				
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);

			glLineWidth(1.0f);
			m_utility_rendering_program.Unbind();

			glEnable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			//glDisable(GL_BLEND);
			

		}break;

		case PLANE_CENTERED:
		{

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


			m_utility_rendering_program.Bind();
			glLineWidth(shape.thickness);
			
			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], shape.alpha);

			shape.is_outline ? draw::Plane_Centered(shape.pos, shape.axis[0], shape.axis[2], optix::make_float2(1.0f), shape.thickness, GL_LINE_LOOP)
				: draw::Plane_Centered(shape.pos, shape.axis[0], shape.axis[2], optix::make_float2(1.0f), shape.thickness);

			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);

			glLineWidth(1.0f);
			glDisable(GL_BLEND);
			//glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

			m_utility_rendering_program.Unbind();


		}break;

		case SAMPLER_CUBE:
		{
			glEnable(GL_DEPTH_TEST);
			if ( shape.is_outline || shape.is_directional )
			{
				glDisable(GL_DEPTH_TEST);
				//glEnable(GL_DEPTH_TEST);
			}
			else
			{
				//glDisable(GL_DEPTH_TEST);
				//glEnable(GL_DEPTH_TEST);
			}

			m_utility_rendering_program.Bind();
			glLineWidth(shape.thickness);
			
			float a = is_g_attach ? 0.5f : 1.0f;
			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], shape.alpha * a);
			draw::Cube(shape.pos, shape.scale_factor, shape.axis, !shape.is_outline);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);

			glLineWidth(1.0f);
			m_utility_rendering_program.Unbind();

			//glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);


		}break;

		case SAMPLER_CUBE_NOT_CENTERED:
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


			m_utility_rendering_program.Bind();
			glLineWidth(shape.thickness);

			//glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(shape.scale_factor));
			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], shape.alpha);
			draw::Cube(shape.pos, shape.scale_factor, shape.axis, !shape.is_outline);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);

			glLineWidth(1.0f);
			m_utility_rendering_program.Unbind();

			glDisable(GL_BLEND);

		}break;

		case CUBE:
		{

			m_utility_rendering_program.Bind();
			glLineWidth(shape.thickness);

			//glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(shape.scale_factor));
			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			draw::Cube(shape.pos, shape.scale_factor, shape.axis, !shape.is_outline);

			glLineWidth(1.0f);
			m_utility_rendering_program.Unbind();

		}break;

		case SPHERE:
		{
			if (!draw_trans_spheres)
				break;
			
			m_utility_rendering_program.Bind();

			glm::mat4 model_matrix = shape.use_trns ? shape.model_matrix : glm::mat4(1.0f);
			glm::mat4 normal_matrix = glm::inverse(glm::transpose(model_matrix));


			Render_Sphere_Outline(model_matrix, shape.color, false, vp);
			

			//glEnable(GL_CULL_FACE);
			//glDisable(GL_DEPTH_TEST); 
			//glDisable(GL_BLEND);
			//glEnable(GL_BLEND);
			//glEnable(GL_DEPTH_TEST);

#define FILL_INNER_SPHERE_DIST_RESTRICTION
#ifdef FILL_INNER_SPHERE_DIST_RESTRICTION

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"],  1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniformMatrix4fv(m_utility_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(normal_matrix));
			
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			glUniform1f( m_utility_rendering_program["uniform_alpha"], 0.25f );
			glBindVertexArray(m_sphere_node->m_vao);
			for (int i = 0; i < m_sphere_node->parts.size(); i++)
				glDrawArrays(GL_TRIANGLES, m_sphere_node->parts[i].start_offset, m_sphere_node->parts[i].count);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);

			glDisable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
#endif

			m_utility_rendering_program.Unbind();

		}break;

		case BoundingBox:
		{
			if ( vp.renderer != OPTIX_RENDERER )
				break;

			m_utility_rendering_program.Bind();

			glm::mat4 model_matrix = shape.model_matrix;
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y, shape.color.z);
			shape.bbox_dashed ? draw::BBox_dashed(shape.a, shape.b, 0.275f , shape.thickness ) : draw::BBox(shape.a, shape.b, ZERO_3f, shape.thickness);
			m_utility_rendering_program.Unbind();

		}break;

		case SolidAngle:
		{

			glEnable(GL_DEPTH_TEST);

			m_utility_rendering_program.Bind();
			glLineWidth(shape.thickness);

			//glm::mat4 model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(shape.scale_factor));
			glm::mat4 model_matrix = shape.model_matrix;
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 0.0f, 0.0f); //shape.color.x, shape.color.y, shape.color.z);
			draw::SolidAngle(shape.arrow_scale_factor, 20.0f);
			glLineWidth(1.0f);
			m_utility_rendering_program.Unbind();

			glDisable(GL_DEPTH_TEST);

		}break;

		case CONE_FRUSTUM:
		{

			if (shape.is_directional)
			{
				//glEnable(GL_CULL_FACE);
				//glEnable(GL_BLEND);
				//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_DEPTH_TEST);
			}
			m_utility_rendering_program.Bind();

			glm::mat4 model_matrix = shape.model_matrix;
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], shape.color.x, shape.color.y , shape.color.z );
			draw::Frustum_Cone(  shape.a, shape.fov , optix::make_float2(0.0f, shape.length), GLOBAL_AXIS[2], m_utility_rendering_program, shape.thickness , shape.color, shape.is_directional );
			
			m_utility_rendering_program.Unbind();
			if (shape.is_directional)
			{
				glDisable(GL_DEPTH_TEST);
				//glDisable(GL_CULL_FACE);
				//glDisable(GL_BLEND);
			}

		}break;


		}

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	// post process if first phase //
	if (first_phase && !only_trans_spheres)
	{
		//if (vp.renderer != OPTIX_RENDERER)
		{
			
			// copy Utility_2 to Fbo_2
			RenderToOutFB(
				ViewportManager::GetInstance().getFbo_2( vp.id ),                 // target Fbo
				ViewportManager::GetInstance().getFboTexture_Utility_2( vp.id ),  // source Texture
				0,  // enable post_proc
				1,  // enable blend
				0,  // discard_alpha
				1   // scale factor

			);

		}
	}


}

  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void OpenGLRenderer::Display()
{

	Display_Viewports();

	// Render decorations //
	Render_Cursor_toScreen();

}
void OpenGLRenderer::Display_Viewports()
{


	std::vector<VIEWPORT> viewports = ViewportManager::GetInstance().getAllEnabledViewports();
	for (VIEWPORT vp : viewports)
	{
		bool is_optix_renderer = 
			vp.renderer == OPTIX_RENDERER
			&& false
			;

		// Render Scene objects to Viewport //
		GLuint source_tex_a = is_optix_renderer ? ViewportManager::GetInstance().getFboTexture( vp.id ) : ViewportManager::GetInstance().getFboTexture_2( vp.id );
		setUp_OutputViewport( vp );
		RenderToOutFB( 
			0, 
			source_tex_a,
			0, 
			0, 
			0 
		);

		// Render Utility objects to Viewport //
		GLuint source_tex_b = ViewportManager::GetInstance().getFboTexture_Utility( vp.id );
		setUp_OutputViewport(vp);
		RenderToOutFB( 
			0, 
			source_tex_b,
			0, 
			1, 
			1 
		);

	}


}


void OpenGLRenderer::Render_Object_Outline(int scene_object_index, bool selected, const VIEWPORT& vp)
{
	
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) 
	streamHandler::Report("OpenGL_Renderer", "draw_Object_Outline", "");
#endif


	SceneObject * obj        = Mediator::RequestSceneObjects(  )[scene_object_index];
	int obj_id               = obj->getId();
	Type obj_type            = obj->getType();
	optix::float3 obj_pos    = obj->getTranslationAttributes();
	glm::vec3 obj_cam_offset = glm::vec3(0.0f);
	float distance_factor    = obj->getDistanceFromCamera() * 0.01f;
	//

	glm::vec3 diffuse = glm::vec3(1.0f);
	if (selected)
		diffuse = glm::vec3(0, 255, 195) / 255.0f;
	else
		diffuse = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->get_Highlight_Color()); //diffuse = glm::vec3(245, 225, 0) / 255.0f;
	

#ifdef CANCER_DEBUG
	bool s = (obj == nullptr || obj == 0);
	
	if (s)
	{
		//std::cout << "\n OpenGLRenderer::Render_Object_Outline():" << std::endl;
		//std::cout << "          - obj : " << !s << std::endl;
		system("pause");
	}

#endif

	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getTranslationAttributes()) - (obj_cam_offset))
		* MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->get_local_parent_rot_matrix_chain() * obj->getRotationMatrix());

	glm::mat4 model_1 = model * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getAbsoluteScaleMatrix());//glm::scale(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getScaleAttributes())); //MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getScaleMatrix());
	//glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_1));
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuse.r, diffuse.g, diffuse.b);
	// - - - - - - - - - - - - - - - - - - - - - - - - - - //

	glClearStencil(0);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// 1st render pass //
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	//
	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 1);
	
	Render_GeometryNode( obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_1, m_utility_rendering_program, true );

	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 0);
	//


	// 2nd render pass //
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0xFF);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(OUTLINE_LINE_WIDTH);
	glEnable(GL_LINE_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 1);
	Render_GeometryNode( obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_1, m_utility_rendering_program, true );

	
	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 0);
	//


	glBindVertexArray(0);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);

}
void OpenGLRenderer::Render_Sphere_Outline(glm::mat4 model_matrix, glm::vec3 color, bool selected, const VIEWPORT& vp)
{
	glm::vec3 diffuse = color;
	
	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuse.r, diffuse.g, diffuse.b);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	// - - - - - - - - - - - - - - - - - - - - - - - - - - //
	glClearStencil(0);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	// glEnable(GL_DEPTH_TEST);
	// glDisable(GL_DEPTH_TEST);
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// 1st render pass //
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	//
	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 1);
	glBindVertexArray(m_sphere_node->m_vao);
	for (int i = 0; i < m_sphere_node->parts.size(); i++)
		glDrawArrays(GL_TRIANGLES, m_sphere_node->parts[i].start_offset, m_sphere_node->parts[i].count);
	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 0);
	//


	// 2nd render pass //
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0xFF);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(OUTLINE_LINE_WIDTH);
	glEnable(GL_LINE_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 1);
	for (int i = 0; i < m_sphere_node->parts.size(); i++)
		glDrawArrays(GL_TRIANGLES, m_sphere_node->parts[i].start_offset, m_sphere_node->parts[i].count);
	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 0);


	glBindVertexArray(0);
	glDisable(GL_STENCIL_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);

}
void OpenGLRenderer::Render_Group_Outline(int scene_object_index, bool selected, const VIEWPORT& vp, bool use_bbox )
{

	bool draw_bbox_only = vp.render_settings.draw_bbox_only;
	
	bool outline_hierarchy = true;
	SceneObject* group_parent = Mediator::RequestSceneObjects()[scene_object_index];

#if defined(CANCER_DEBUG)
	if (group_parent == nullptr || group_parent == 0)
	{
		//std::cout << "\n - Group_parent = nullptr!" << std::endl;
		//std::cout << " - requested index : " << scene_object_index << std::endl;

		std::vector<SceneObject*> objects = Mediator::RequestSceneObjects();
		int index = 0;
		for (SceneObject* obj : objects)
		{
			int id = obj->getId();
			//std::cout << "      - sceneObject(" << index << "):" << std::endl;
			
			//std::cout << "      - object == nullptr? : " << (obj == nullptr || obj == 0) << std::endl;
			//std::cout << "      - object id : " << id << std::endl;
			//std::cout << "      - object.childs().size() = " << obj->getChilds().size() << std::endl;
			//std::cout << " \n";

			index++;
		}
		system("pause");

		return;
	}
#endif

	if (group_parent->isTemporary())
	{
		outline_hierarchy = false;
	}

	// traverse family tree
	// for each child node render_Outline
	if (outline_hierarchy)
	{
		if (!draw_bbox_only) Render_Object_BBOX(scene_object_index, selected);
		Render_Object_Outline_as_Group(group_parent, group_parent->getGroupSelectionChilds(), selected, vp);
	}
	// area selection //
	else
	{
		Render_Object_Outline_as_Group(group_parent, group_parent->getGroupSelectionChilds(), selected, vp);
	}

	//#define DRAW_OUTLINE_BY_CHILDS_ONLY
#ifdef DRAW_OUTLINE_BY_CHILDS_ONLY

	for (SceneObject* obj : group_parent->getChilds())
	{
		int array_index = Mediator::requestObjectsArrayIndex(obj->getId());
		Type obj_type = obj->getType();
		if (array_index >= 0)
		{
			if (
				obj_type != GROUP_PARENT_DUMMY
				&& obj_type != DUMMY
				)
				Render_Object_Outline(array_index, true);
		}
	}

#endif

}
void OpenGLRenderer::Render_Object_Outline_as_Group( std::vector<class SceneObject*> objects, bool selected, const VIEWPORT& vp, bool use_bbox )
{
	bool draw_bbox_only = vp.render_settings.draw_bbox_only;

	glm::vec3 diffuse = glm::vec3(1.0f);
	if ( selected )
		diffuse = glm::vec3(0, 255, 195) / 255.0f;
	else
		diffuse = glm::vec3(245, 225, 0) / 255.0f;



	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	glm::mat4 model;
	glm::mat4 model_1;
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuse.r, diffuse.g, diffuse.b);
	// - - - - - - - - - - - - - - - - - - - - - - - - - - //


	glClearStencil(0);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// 1st render pass //
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	//
	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 1);

	for (SceneObject* child : objects)
	{
#ifdef CANCER_DEBUG
		bool s = (child == nullptr || child == 0);

		if (s)
		{
			//std::cout << "\n OpenGLRenderer::Render_Object_Outline_as_Group():" << std::endl;
			//std::cout << "          - obj : " << !s << std::endl;
			system("pause");
		}
#endif
		if (child->getType() == GROUP_PARENT_DUMMY || !child->isVisible())
			continue;

		//if(draw_bbox_only)
		//{
		//	glm::mat4 model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->getTransformationMatrix());
		//	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		//	draw::BBox(child->getBBoxMin(), child->getBBoxMax(), ZERO_3f, 1.0f);
		//}

		model = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(child->getTranslationAttributes()))
			* MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->get_local_parent_rot_matrix_chain() * child->getRotationMatrix());
		model_1 = model * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->getAbsoluteScaleMatrix());
		Render_GeometryNode(child, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(child), model_1, m_utility_rendering_program, true);
	}
	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 0);
	//


	// 2nd render pass //
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0xFF);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(OUTLINE_LINE_WIDTH);
	glEnable(GL_LINE_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	//
	//glBindVertexArray(m_geometry_stored_objects[scene_object_index]->m_vao);
	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 1);
	for (SceneObject* child : objects)
	{
#ifdef CANCER_DEBUG
		bool s = (child == nullptr || child == 0);

		if (s)
		{
			//std::cout << "\n OpenGLRenderer::Render_Object_Outline_as_Group():" << std::endl;
			//std::cout << "          - obj : " << !s << std::endl;
			system("pause");
		}
#endif
		if (child->getType() == GROUP_PARENT_DUMMY || !child->isVisible())
			continue;

		model = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(child->getTranslationAttributes()))
			* MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->get_local_parent_rot_matrix_chain() * child->getRotationMatrix());
		model_1 = model * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->getAbsoluteScaleMatrix());
		Render_GeometryNode(child, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(child), model_1, m_utility_rendering_program, true);
	}
	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 0);
	//


	glBindVertexArray(0);
	glDisable(GL_STENCIL_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
}
void OpenGLRenderer::Render_Object_Outline_as_Group( SceneObject* parent, std::vector<class SceneObject*> objects, bool selected, const VIEWPORT& vp, bool use_bbox)
{
	bool draw_bbox_only = vp.render_settings.draw_bbox_only;

	glm::vec3 diffuse = glm::vec3(1.0f);
	if (selected)
		diffuse = glm::vec3(0, 255, 195) / 255.0f;
	else
	{
		//diffuse = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(parent->get_Highlight_Color());
		diffuse = glm::vec3(245, 225, 0) / 255.0f;
	}


	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	glm::mat4 model;
	glm::mat4 model_1;
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuse.r, diffuse.g, diffuse.b);
	// - - - - - - - - - - - - - - - - - - - - - - - - - - //


	glClearStencil(0);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// 1st render pass //
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	//
	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 1);

	for (SceneObject* child : objects)
	{
#ifdef CANCER_DEBUG
		bool s = (child == nullptr || child == 0);

		if (s)
		{
			//std::cout << "\n OpenGLRenderer::Render_Object_Outline_as_Group():" << std::endl;
			//std::cout << "          - obj : " << !s << std::endl;
			system("pause");
		}
#endif
		if (child->getType() == GROUP_PARENT_DUMMY || !child->isVisible())
			continue;

		//if(draw_bbox_only)
		//{
		//	glm::mat4 model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->getTransformationMatrix());
		//	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		//	draw::BBox(child->getBBoxMin(), child->getBBoxMax(), ZERO_3f, 1.0f);
		//}

		model = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(child->getTranslationAttributes()))
			* MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->get_local_parent_rot_matrix_chain() * child->getRotationMatrix());
		model_1 = model * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->getAbsoluteScaleMatrix());
		Render_GeometryNode(child, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(child), model_1, m_utility_rendering_program, true);
	}
	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 0);
	//


	// 2nd render pass //
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0xFF);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(OUTLINE_LINE_WIDTH);
	glEnable(GL_LINE_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	//
	//glBindVertexArray(m_geometry_stored_objects[scene_object_index]->m_vao);
	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 1);
	for (SceneObject* child : objects)
	{
#ifdef CANCER_DEBUG
		bool s = (child == nullptr || child == 0);

		if (s)
		{
			//std::cout << "\n OpenGLRenderer::Render_Object_Outline_as_Group():" << std::endl;
			//std::cout << "          - obj : " << !s << std::endl;
			system("pause");
		}
#endif
		if (child->getType() == GROUP_PARENT_DUMMY || !child->isVisible())
			continue;

		model = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(child->getTranslationAttributes()))
			* MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->get_local_parent_rot_matrix_chain() * child->getRotationMatrix());
		model_1 = model * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(child->getAbsoluteScaleMatrix());
		Render_GeometryNode(child, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(child), model_1, m_utility_rendering_program, true);
	}
	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 0);
	//


	glBindVertexArray(0);
	glDisable(GL_STENCIL_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
}


void OpenGLRenderer::Render_Object_BBOX(int scene_object_index , bool selected)
{
	

	// draw BBOX of group
	SceneObject* object = Mediator::RequestSceneObjects()[scene_object_index];
#ifdef CANCER_DEBUG
	bool s = (object == nullptr || object == 0);

	if (s)
	{
		//std::cout << "\n OpenGLRenderer::Render_Object_BBOX():" << std::endl;
		//std::cout << "          - obj : " << !s << std::endl;
		system("pause");
	}
#endif
	glEnable(GL_LINE_SMOOTH);
	bool group = false;
	glm::mat4 model;
	if ( object->getType() == GROUP_PARENT_DUMMY && !object->isTemporary() )
	{
		model = glm::mat4(1.0f);
		group = true;
	}
	else
	{
		return;
		model = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(object->getTransformationMatrix());
	}

	glm::vec3 diffuse = selected? glm::vec3(0.85f, 0.95f, 0.95f) : VECTOR_TYPE_TRANSFORM::TO_GLM_3f( object->get_Highlight_Color() );

	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuse.r, diffuse.g, diffuse.b);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model));
	if (group)
		draw::BBox_dashed(object->getBBoxMin(), object->getBBoxMax(), 0.275f, 1.0f);
	else
		draw::BBox(object->getBBoxMin(), object->getBBoxMax(), ZERO_3f);

	glDisable(GL_LINE_SMOOTH);
}
void OpenGLRenderer::Render_Object_BBOX(SceneObject* obj)
{
#ifdef CANCER_DEBUG
	bool s = (obj == nullptr || obj == 0);

	if (s)
	{
		//std::cout << "\n OpenGLRenderer::Render_Object_BBOX():" << std::endl;
		//std::cout << "          - obj : " << !s << std::endl;
		system("pause");
	}
#endif

	glEnable(GL_LINE_SMOOTH);
	bool group = false;
	glm::mat4 model;
	if (obj->getType() == GROUP_PARENT_DUMMY)
	{
		model = glm::mat4(1.0f);
		group = true;
	}
	else
	{
		model = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getTransformationMatrix());
	}
	glUniform1i(        m_scene_rendering_program["uniform_rendering_utility"], 1);
	glUniform3f(        m_scene_rendering_program["uniform_diffuse"], 0.85f, 0.95f, 0.95f );
	glUniformMatrix4fv( m_scene_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model) );

	//if ( group )
	//	draw::BBox_dashed(obj->getBBoxMin(), obj->getBBoxMax(), 0.275f, 1.0f);
	//else
		draw::BBox(obj->getBBoxMin(), obj->getBBoxMax(), ZERO_3f);

	glUniform1i(m_scene_rendering_program["uniform_rendering_utility"], 0);
	glDisable(GL_LINE_SMOOTH);
}
void OpenGLRenderer::Render_Object_FaceVectors( Utility_RenderList_Object utility_to_Render , bool is_creating_face_vector )
{
	SceneObject* object = utility_to_Render.obj_pack->raw_object;
	float scale_factor = utility_to_Render.obj_pack->cam_inv_scale_factor;


	std::vector<SceneObject*> face_vectors = object->get_Face_Vectors();
	glm::mat4 model_matrix;
	optix::float3 color;
	optix::float3 p0 = ZERO_3f;
	optix::float3 p1;
	

	for (SceneObject* face_vector : face_vectors)
	{
		bool is_valid = face_vector->isActive();
		if (!is_valid)
			continue;

		float vp_scale_factor = utility_to_Render.camera_instance->getInvScaleFactor();
		float obj_dist = utility_to_Render.camera_instance->getDistanceFrom(face_vector->getTranslationAttributes());
		float scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp_scale_factor;
		
		//color = face_vector->get_FaceVectorParameters().color;
		//model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(face_vector->getTransformationMatrix());
		//glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		//glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);

		//draw::Line3D( p0, p0 + GLOBAL_AXIS[2] * scale_factor * FACE_VECTOR_SCALE_FACTOR, FACE_VECTOR_LINE_WIDTH);
		//draw::Arrow(  p0, p0 + GLOBAL_AXIS[2] * scale_factor * FACE_VECTOR_SCALE_FACTOR, scale_factor * 1.3f , utility_to_Render.vp , false);

		Shape line;
		line.color = glm::vec3(1.0f);
		line.shape_id = LINE;
		line.a = p0; line.b = p0 + GLOBAL_AXIS[2] * scale_factor * FACE_VECTOR_SCALE_FACTOR;
		line.draw_arrow = 0;
		line.thickness = FACE_VECTOR_LINE_WIDTH;

		line.draw_arrow = 1;
		line.only_arrow = 1;
		line.arrow_scale_factor = scale_factor * FACE_VECTOR_ARROW_SCALE_FACTOR;
	}

	

}
void OpenGLRenderer::Render_Object_FaceVectors(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{
	SceneObject* object = utility_to_Render.obj_pack->raw_object;
	float scale_factor  = vp.scale_factor;


	std::vector<SceneObject*> face_vectors = object->get_Face_Vectors();
	glm::mat4 model_matrix;
	optix::float3 color;
	optix::float3 p0 = ZERO_3f;
	optix::float3 p1;


	for (SceneObject* face_vector : face_vectors)
	{
		bool is_valid = face_vector->isActive();
		if (!is_valid)
			continue;

		float vp_scale_factor = vp.scale_factor;
		float obj_dist        = ViewportManager::GetInstance().getViewportCamera(vp.id).getDistanceFrom(face_vector->getTranslationAttributes());
		float scale_factor    = LINKING_SCALE_FACTOR * obj_dist * vp_scale_factor;

		model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(face_vector->getTransformationMatrix());
	
		Shape line;
		line.model_matrix = model_matrix;
		line.use_trns = 1;
		line.color = glm::vec3(1.0f);
		line.shape_id = LINE;
		line.a = p0; line.b = p0 + GLOBAL_AXIS[2] * scale_factor * FACE_VECTOR_SCALE_FACTOR;
		line.draw_arrow = 0;
		line.thickness  = FACE_VECTOR_LINE_WIDTH;
		line.test_depth = 0;
		line.gl_line_smooth = true;
		m_Shapes2D_per_Viewport[vp.id].push_back(line);

		line.draw_arrow = 1;
		line.only_arrow = 1;
		line.arrow_scale_factor = scale_factor * FACE_VECTOR_ARROW_SCALE_FACTOR;
		line.gl_line_smooth = true;
		m_Shapes2D_per_Viewport[vp.id].push_back(line);
	}



}
void OpenGLRenderer::Render_FaceVector_Outline( Highlight_RenderList_Object ut, int scene_object_index, bool selected , VIEWPORT vp )
{
	
	SceneObject* obj = Mediator::RequestSceneObjects()[scene_object_index];
	optix::float3 focused_color = optix::make_float3(0, 205, 135) / 255.0f;
	float scale_factor = ut.obj_pack.cam_inv_scale_factor;
	float offset = 0.1f * scale_factor;
	optix::float3 p0 = ZERO_3f;
	


	glm::mat4 model_matrix;
	optix::float3 color;
	
	optix::float3 p1;
	float alpha = 1.0f;
	
	glDisable(GL_DEPTH_TEST);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
	color = selected ? SELECTED_COLOR_3f : obj->get_Highlight_Color();
	//color = optix::make_float3(1.0f, 0.0f, 0.0f);
	model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getTransformationMatrix());
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
	draw::Line3D(p0, p0 + GLOBAL_AXIS[2] * scale_factor * FACE_VECTOR_SCALE_FACTOR, FACE_VECTOR_LINE_WIDTH + 1);
	draw::Arrow(p0,  p0 + GLOBAL_AXIS[2] * scale_factor * FACE_VECTOR_SCALE_FACTOR, scale_factor * FACE_VECTOR_ARROW_SCALE_FACTOR, vp ,  false, FACE_VECTOR_LINE_WIDTH + 1);
	glEnable(GL_DEPTH_TEST);
	


}
void OpenGLRenderer::Render_Sampler_Outline(Highlight_RenderList_Object ut, int scene_object_index, bool selected, VIEWPORT vp)
{

	return;

	SceneObject* obj = Mediator::RequestSceneObjects()[scene_object_index];
	optix::float3 focused_color = optix::make_float3(0, 205, 135) / 255.0f;
	float scale_factor = ut.obj_pack.cam_inv_scale_factor;
	float offset = 0.1f * scale_factor;
	optix::float3 p0 = ZERO_3f;

	glm::mat4 model_matrix;
	optix::float3 color;

	optix::float3 p1 = p0 + GLOBAL_AXIS[2] * scale_factor * FACE_VECTOR_SCALE_FACTOR;
	float alpha = 1.0f;

	glDisable(GL_DEPTH_TEST);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
	color = selected ? SELECTED_COLOR_3f : obj->get_Highlight_Color();
	model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX( obj->getTransformationMatrix() );
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
	draw::Line3D(p0, p1, SAMPLER_OUTLINE_THICKNESS);
	
	{
		optix::float3 pos = obj->getTranslationAttributes();
		float dist = ut.camera_instance->getDistanceFrom(pos);
		float obj_scale_factor = LINKING_SCALE_FACTOR * dist  * ut.camera_instance->getInvScaleFactor(vp);
		// 
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(1.0f);

		float a = FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 0.155f;
		optix::float3 axis[3]  = { GLOBAL_AXIS[0]  *a, GLOBAL_AXIS[1] * a, GLOBAL_AXIS[2] * a };
		glm::mat4 model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getTransformationMatrix());
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.7f);
		draw::Cube(p1, 1.0f, axis, true);

		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.0f, 0.0f, 0.0f);
		draw::Cube(p1, 1.0f, axis, false);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);

		glLineWidth(1.0f);
		glDisable(GL_BLEND);
	}

	
	glEnable(GL_DEPTH_TEST);


}

void OpenGLRenderer::RenderToOutFB(GLuint m_fbo_texture, bool enable_blend)
{
	// Bind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (enable_blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// bind the post processing program
	m_postprocess_program.Bind();

	// Bind the intermediate color image to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
	glUniform1i(m_postprocess_program["samplerHDR"], 0);
	glUniform1i(m_postprocess_program["apply_postproc_effects"], 0);


	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 1.0f);
	glEnd();


	// Unbind the post processing program
	glBindTexture(GL_TEXTURE_2D, 0);
	m_postprocess_program.Unbind();


	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	if (enable_blend)
	{
		glDisable(GL_BLEND);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void OpenGLRenderer::RenderToOutFB(GLuint m_fbo_to, GLuint m_fbo_texture_from, bool post_proc , bool use_blend, bool discard_alpha , float s, bool visualize_depth, bool write_depth, GLuint depth_texture, bool invert_pixels )
{
	glBindFramebuffer(GL_FRAMEBUFFER,  m_fbo_to );
	
	if (m_fbo_to != 0)
	{
		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);
	}
	
	write_depth ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);
	if (use_blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	

	m_postprocess_program.Bind();

	glUniform1i(m_postprocess_program["uniform_invert_pixels"], invert_pixels ? 1 : 0);
	glUniform1i(m_postprocess_program["uniform_visualize_depth"], visualize_depth ? 1 : 0);
	glUniform1i(m_postprocess_program["uniform_write_depth"], write_depth ? 1 : 0);
	glUniform1i(m_postprocess_program["apply_postproc_effects"], post_proc ? 1 : 0 );
	glUniform1i(m_postprocess_program["uniform_discard_alpha"], discard_alpha ? 1 : 0);

	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fbo_texture_from);
	glUniform1i(m_postprocess_program["samplerHDR"], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glUniform1i(m_postprocess_program["Depth_Sampler"], 1);
	glActiveTexture(GL_TEXTURE0);

	
	{
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-1.0f, 1.0f);
		glEnd();
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	glUniform1i(m_postprocess_program["uniform_invert_pixels"], 0);
	glUniform1i(m_postprocess_program["apply_postproc_effects"], 0);
	glUniform1i(m_postprocess_program["uniform_discard_alpha"], 0);
	glUniform1i(m_postprocess_program["uniform_visualize_depth"], 0);
	glUniform1i(m_postprocess_program["uniform_write_depth"], 0);

	m_postprocess_program.Unbind();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (use_blend)
		glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
}


#ifdef UTILITY_RENDERER_UTILITY_RENDER_FUNCTIONS

void OpenGLRenderer::Render_Utility_Translation(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{
	glm::mat4 model_matrix;
	glm::vec3 diffuseColor;
	UtilityPlane selected_plane = interface_state->isec_p->plane_id;

	const optix::float3 * M_WORKING_AXIS;
	bool screen_axis = false;
	if (interface_state->active_axis_orientation == AXIS_ORIENTATION::SCREEN)
		screen_axis = true;
	M_WORKING_AXIS = GLOBAL_AXIS;

	bool is_active = interface_state->active_utility->active && !interface_state->m_gui_transforming;
	bool is_axis   = (selected_plane == X || selected_plane == Y || selected_plane == Z);
	bool is_plane  = (selected_plane == XY || selected_plane == XZ || selected_plane == YZ);

	if (screen_axis)
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->screen_trs_matrix));
	else 
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->transformation_matrix));
	
	// Render Translation Axis
	if( !is_active || selected_plane != XYZ) 
	{	
		// pass the model matrix to glsl , need it when object is on local space
		for (int i = 0; i < 3; i++)
		{
			glBindVertexArray(m_base_axis_node[i]->m_vao);
			for (int j = 0; j < m_base_axis_node[i]->parts.size(); j++)
			{
				
				float axis_delta = 0.77f;
				if ( (!is_axis && is_active) || (is_plane))
					axis_delta = 0.0f;
				optix::float3 p1 = optix::make_float3(0.0f) + M_WORKING_AXIS[i] * axis_delta;
				optix::float3 p2 = M_WORKING_AXIS[i] * 4.0f;
				optix::float3 p3 = p2 + M_WORKING_AXIS[i] * 0.97f;

				// render translation axis line segment
				{
					
					if (AXIS_COLOR[i] == HIGHLIGHT_COLOR)
						glLineWidth(1.5f);
					else
						glLineWidth(UTILITY_LINE_WIDTH);
					glLineWidth(2.0f);
					diffuseColor = AXIS_COLOR[i] * 0.9f + glm::vec3(0.1f) * GLM_GLOBAL_INV_AXIS[i];
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					draw::Line3D((float*)&p1, (float*)&p2);
				}

				// render translation axis cone parts
				if( !is_active || is_axis )
				{
					// cone's back darker color
					diffuseColor = DEF_AXIS_COLOR[i];
					if (j == 0)
						diffuseColor *= 0.5f;

					glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					glDrawArrays(GL_TRIANGLES, m_base_axis_node[i]->parts[j].start_offset, m_base_axis_node[i]->parts[j].count);
				}

				// render translation axis logo
				{
					diffuseColor = AXIS_COLOR[i];
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					glUniform1i(m_utility_rendering_program["uniform_text_render"], 1);
					draw::Text(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p3), glm::vec3(1), FONT6, GLOBAL_AXIS_NAME[i]);
					glUniform1i(m_utility_rendering_program["uniform_text_render"], 0);
				}

			}
		}
	}

	


	glLineWidth(UTILITY_LINE_WIDTH);
	// Render XYZ plane
	if (!is_plane || !is_active)
	{
	

		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->def_transformation_matrix));
		glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 1);
		diffuseColor = glm::vec3(0.2f);
		if (selected_plane == XYZ)
			diffuseColor = HIGHLIGHT_COLOR;
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);

		float size = 0.4;

		glBegin(GL_LINE_LOOP);
		glVertex3f(size, -size, 0.0f);
		glVertex3f(size, size, 0.0f);
		glVertex3f(-size, size, 0.0f);
		glVertex3f(-size, -size, 0.0f);
		glEnd();

		glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	}

	// Render XY , XZ , YZ Planes
	{
		

		if( screen_axis )
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->screen_trs_matrix));
		else
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->def_transformation_matrix));

		optix::float3 p1, p2;
		float dist_factor = 1.45f;

		if( !screen_axis )
			M_WORKING_AXIS = utility_to_Render.obj_pack->base_axis;

		// XY - plane line indicators
		if (selected_plane != XYZ || !is_active )
		{
			
			if (selected_plane == XY)
			{
				diffuseColor = PLANE_HIGHLIGHT_COLOR;
				//diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
				draw::Plane(optix::make_float3(0.0f), M_WORKING_AXIS[0] * dist_factor, M_WORKING_AXIS[1] * dist_factor);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);

			}


			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
			p1 = optix::make_float3(0.0f) + M_WORKING_AXIS[0] * dist_factor;
			p2 = p1 + M_WORKING_AXIS[1] * dist_factor;
			if( selected_plane == XY )
				diffuseColor = AXIS_COLOR[0] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[1];
			else
				diffuseColor = DEF_AXIS_COLOR[0] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[1];
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Line3D((float*)&p1, (float*)&(p2));

			p1 = optix::make_float3(0.0f) + M_WORKING_AXIS[1] * dist_factor;
			p2 = p1 + M_WORKING_AXIS[0] * dist_factor;
			if (selected_plane == XY)
				diffuseColor = AXIS_COLOR[1] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[1];
			else
				diffuseColor = DEF_AXIS_COLOR[1] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[1];
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Line3D((float*)&p1, (float*)&(p2));

			

		}

		
		

		// XZ - plane line indicators
		if (selected_plane != XYZ || !is_active )
		{

			if (selected_plane == XZ)
			{
				diffuseColor = PLANE_HIGHLIGHT_COLOR;
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
				draw::Plane(optix::make_float3(0.0f), M_WORKING_AXIS[0] * dist_factor, M_WORKING_AXIS[2] * dist_factor);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
			}

			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
			p1 = optix::make_float3(0.0f) + M_WORKING_AXIS[0] * dist_factor;
			p2 = p1 + M_WORKING_AXIS[2] * dist_factor;
			if (selected_plane == XZ)
				diffuseColor = AXIS_COLOR[0] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
			else
				diffuseColor = DEF_AXIS_COLOR[0] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Line3D((float*)&p1, (float*)&(p2));

			p1 = optix::make_float3(0.0f) + M_WORKING_AXIS[2] * dist_factor;
			p2 = p1 + M_WORKING_AXIS[0] * dist_factor;
			if (selected_plane == XZ)
				diffuseColor = AXIS_COLOR[2] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
			else
				diffuseColor = DEF_AXIS_COLOR[2] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Line3D((float*)&p1, (float*)&(p2));

			
		}

		// YZ - plane line indicators
		if (selected_plane != XYZ || !is_active )
		{

			if (selected_plane == YZ)
			{
				diffuseColor = PLANE_HIGHLIGHT_COLOR;
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
				draw::Plane(optix::make_float3(0.0f), M_WORKING_AXIS[1] * dist_factor, M_WORKING_AXIS[2] * dist_factor);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
			}

			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
			p1 = optix::make_float3(0.0f) + M_WORKING_AXIS[1] * dist_factor;
			p2 = p1 + M_WORKING_AXIS[2] * dist_factor;
			if (selected_plane == YZ)
				diffuseColor = AXIS_COLOR[1] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
			else
				diffuseColor = DEF_AXIS_COLOR[1] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Line3D((float*)&p1, (float*)&(p2));


			p1 = optix::make_float3(0.0f) + M_WORKING_AXIS[2] * dist_factor;
			p2 = p1 + M_WORKING_AXIS[1] * dist_factor;
			if (selected_plane == YZ)
				diffuseColor = AXIS_COLOR[2] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
			else
				diffuseColor = DEF_AXIS_COLOR[2] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Line3D((float*)&p1, (float*)&(p2));

			
		}

		
	}


	

	

	// Render Solid Angle ( TEST )
	{
		//SceneObject* obj = utility_to_Render.obj_pack->raw_object;
		//diffuseColor = glm::vec3(1.0f, 0.0f, 0.0f);
		//model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getTranslationMatrix()) * glm::scale(glm::mat4(1.0f), glm::vec3(4.0f));
		//glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix)); //glm::value_ptr(utility_to_Render.obj_pack->transformation_matrix));
		//glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		//draw::SolidAngle(3.0f, 20.0f);

		//optix::float3 p1 = ZERO_3f;
		//optix::float3 p2 = optix::make_float3(0, 0, 10);
		//draw::Arrow(p1, p2, 2.0f, false);
	}
	

	//glUniform1i(m_utility_rendering_program["uniform_use_model_matrix"], 0);
}
void OpenGLRenderer::Render_Utility_Rotation(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 model_matrix;
	glm::vec3 diffuseColor;


	glm::vec3 pos = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(utility_to_Render.obj_pack->raw_object->getTranslationAttributes());
	glm::vec3 scale = glm::vec3( utility_to_Render.obj_pack->cam_inv_scale_factor * 0.6);
	////std::cout << "scale : " << utility_to_Render.obj_pack->cam_inv_scale_factor;
	//float inv_cam_scale_factor = ROTATION_AXIS_SCALE_FACTOR * utility_to_Render.obj_pack->raw_object->getDistanceFromCamera();
	//glm::vec3 scale = glm::vec3( inv_cam_scale_factor * 0.6f);
	optix::float3 rot_dt = utility_to_Render.obj_pack->raw_object->getAttributesDelta(1);

	optix::float3 * M_WORKING_AXIS = utility_to_Render.obj_pack->base_axis;
	IntersectionPoint isec_p = *interface_state->isec_p;

	glDisable(GL_CULL_FACE);
	// outer stable circles
	{
		//glDisable(GL_DEPTH_TEST);
		//glDisable(GL_BLEND);

		int a = 34;
		int b = 35;
		int PI = 1;
		if (interface_state->isec_p->plane_id != UtilityPlane::NONE)
		{
			a = 25;
			b = 35;
			PI = 4;
		}

		glm::mat4 outer_matrix_1 = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), scale * 1.29f);
		glm::mat4 outer_matrix_2 = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), scale * 1.45f);
		glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 1);

		if (interface_state->isec_p->inner_radius_hit)
			glLineWidth(1.5f);
		else
			glLineWidth(UTILITY_LINE_WIDTH);

		diffuseColor = glm::vec3(0.65f);
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(outer_matrix_1));
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		draw::Circle(0.0, 0.0, 0.0, 1.0f, Utilities::getRand(a, b, PI), 1);
		
		glLineWidth(UTILITY_LINE_WIDTH);
		diffuseColor = glm::vec3(0.5f);
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(outer_matrix_2));
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
		draw::Circle(0.0, 0.0, 0.0, 1.0f, Utilities::getRand(a - 5, b, PI), 1);
		//glLineWidth(UTILITY_LINE_WIDTH);

		glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
		//glEnable(GL_BLEND);
	}


	glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->translation_matrix_scaled));
	// draw Rotation Arcs
	//if (!interface_state->inner_radius_hit || !interface_state->active_utility->active)
	{

		for (uint arc_i = 0; arc_i < 3; arc_i++)
		{

			
			float rot_dti = ((float*)&rot_dt)[arc_i];
			if (arc_i == interface_state->isec_p->plane_id - 1)
				glLineWidth(1.5f);
			else
				glLineWidth(UTILITY_LINE_WIDTH);



			if (rot_dti != 0 && !interface_state->inner_radius_hit)
			{

				

				glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.9f);
				glUniform1i(m_utility_rendering_program["uniform_selected_circle"], arc_i);

				diffuseColor = DEF_AXIS_COLOR[arc_i];
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);

				optix::float3 c = ZERO_3f;
				optix::float3 p0 = isec_p.p;
				optix::float3 plane_n = M_WORKING_AXIS[arc_i];
				optix::float3 u = isec_p.normal; //normalize(p0 - c);
				optix::float3 v = -cross(u, plane_n);
				draw::Arc_arbitr_plane(c, 1.0f, 0.0f, rot_dti, u, v, 50, 1, GL_POLYGON);

				glLineWidth(1.0f);
				draw::Tangent(isec_p, rot_dti);
				glLineWidth(1.5f);

				// display rot dt on screen above sphere
				{
					/* clamp rot_dti //
					float pi = Geometry::GENERAL::toRadians(360.0f);
					if (fabsf(rot_dti) > pi)
					{
						if (rot_dti < 0.0f)
							rot_dti += pi;
						else
							rot_dti -= pi; //(pi - rot_dti);

					}
					*/

					float deg = Geometry::GENERAL::toDegrees(rot_dti); std::string s = std::to_string(deg);
					int sign = deg >= 0.0f ? 1 : -1; if (s.size() > 7 + (1 - sign) / 2) s.erase(s.begin() + 6 + (1 - sign) / 2, s.end());
					
					if (arc_i == 0)      s = "[ "+s+" , 0.00, 0.00 ]";
					else if (arc_i == 1) s = "[ 0.00, "+s+", 0.00 ]";
					else if (arc_i == 2) s = "[ 0.00, 0.00, "+s+" ]";

					glDisable(GL_CULL_FACE);
					glDisable(GL_DEPTH_TEST);
					glDisable(GL_BLEND);
					glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 0.0f);

					PinholeCamera cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
					optix::float3 u = normalize(cam.getU()); //utility_to_Render.camera_instance->getU());
					optix::float3 v = normalize(cam.getV()); //utility_to_Render.camera_instance->getV());
					glm::vec3 p = pos + VECTOR_TYPE_TRANSFORM::TO_GLM_3f(v) * scale * 1.98f - VECTOR_TYPE_TRANSFORM::TO_GLM_3f(u) * scale * 1.04f;
					glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
					draw::Text(p, glm::vec3(1.0f, 1.0f, 0.0f), FONT6, (char*)s.c_str());
					glEnable(GL_DEPTH_TEST);
					glEnable(GL_CULL_FACE);
					glEnable(GL_BLEND);
					glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->translation_matrix_scaled));
				}

			}
			

			optix::float3 u, v;
			if (arc_i == 0)
			{
				u = M_WORKING_AXIS[1];
				v = M_WORKING_AXIS[2];
			}
			else if (arc_i == 1)
			{
				u = M_WORKING_AXIS[0];
				v = M_WORKING_AXIS[2];
			}
			else
			{
				u = M_WORKING_AXIS[0];
				v = M_WORKING_AXIS[1];
			}

			

			glUniform1i(m_utility_rendering_program["uniform_is_arc"], 1);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
			glUniform1i(m_utility_rendering_program["uniform_circle"], arc_i);
			diffuseColor = AXIS_COLOR[arc_i];
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Arc_arbitr_plane(ZERO_3f, 1.0f, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, CIRCLE_SEGMENTS, 0, GL_LINE_LOOP);
			glLineWidth(UTILITY_LINE_WIDTH);
		}
		glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);

	}
	
	

	// draw transparent hemi-sphere
	{
		glEnable(GL_CULL_FACE);
		//glEnable(GL_DEPTH_TEST);

#ifdef CANCER_DEBUG
		
		bool s = (utility_to_Render.obj_pack->raw_object == nullptr || utility_to_Render.obj_pack->raw_object == 0);
		
		if (s)
		{
			//std::cout << "\n OpenGLRenderer::Render_Utility_Rotation():" << std::endl;
			//std::cout << "          - utility_to_Render.obj_pack->raw_object = nullptr! "<< std::endl;
			system("pause");
		}
#endif
		// auto edw prepei na ginei  : utility_to_Render.obj_pack1->raw_object;
		m_sphere_transformation_matrix = glm::translate(glm::mat4(1.0f), pos) * MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(utility_to_Render.obj_pack->raw_object->getRotationMatrix()) * glm::scale(glm::mat4(1.0f), scale * 1.26f); // 1.29
		m_sphere_normal_matrix = glm::inverse(glm::transpose(m_sphere_transformation_matrix));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_sphere_transformation_matrix));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(m_sphere_normal_matrix));
		diffuseColor = glm::vec3(0.35f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_utility_rendering_program["uniform_sphere_center"], pos.x, pos.y, pos.z);


		glBindVertexArray(m_sphere_node->m_vao);
		glUniform1i(m_utility_rendering_program["draw_sphere"], 1);
		glUniform1i(m_utility_rendering_program["inner_radius_hit"], interface_state->inner_radius_hit);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.4f);
		for (int i = 0; i < m_sphere_node->parts.size(); i++)
		{
			glDrawArrays(GL_TRIANGLES, m_sphere_node->parts[i].start_offset, m_sphere_node->parts[i].count);
		}
		glUniform1i(m_utility_rendering_program["draw_sphere"], 0);


		glBindVertexArray(0);

	}


	// draw XYZ - axis inside roation sphere
	if(!interface_state->inner_radius_hit || !interface_state->active_utility->active || true)
	{
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->translation_matrix_scaled));
		optix::float3 p1 = ZERO_3f;
		optix::float3 p2, p3;
		float size = 0.4f;
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);

		for (int i = 0; i < 3; i++)
		{

			p2 = M_WORKING_AXIS[i] * size;
			if (AXIS_COLOR[i] != HIGHLIGHT_COLOR)
				diffuseColor = glm::vec3(0.65f); // axis color when not selected
			else
				diffuseColor = HIGHLIGHT_COLOR;
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.x, diffuseColor.y, diffuseColor.z);
			glBegin(GL_LINE_LOOP);
			glVertex3f(p1.x, p1.y, p1.z);
			glVertex3f(p2.x, p2.y, p2.z);
			glEnd();

			p3 = p2 + M_WORKING_AXIS[i] * 0.07f;
			glUniform1i(m_utility_rendering_program["uniform_text_render"], 1);
			draw::Text(glm::vec3(p3.x, p3.y, p3.z), glm::vec3(1), FONT5, GLOBAL_AXIS_NAME[i]);
			glUniform1i(m_utility_rendering_program["uniform_text_render"], 0);

		}

		glEnable(GL_BLEND);
	}


	glDisable(GL_BLEND);
}
void OpenGLRenderer::Render_Utility_Scale(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{
	glm::mat4 model_matrix;
	glm::vec3 diffuseColor, diffuseColor2;

	UtilityPlane selected_plane = interface_state->isec_p->plane_id;
	bool is_active = interface_state->active_utility->active;
	float m_cam_inv_scale_factor = interface_state->m_camera_inv_scale_factor;
	optix::float3 delta = ZERO_3f;
	

	const optix::float3 * M_WORKING_AXIS = GLOBAL_AXIS;
	bool screen_axis = false;
	if (interface_state->active_axis_orientation == AXIS_ORIENTATION::SCREEN)
		screen_axis = true;

	if (screen_axis)
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->screen_trs_matrix));
	else
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->transformation_matrix));

	for (int i = 0; i < 3; i++)
	{
		optix::float3 ds = interface_state->selected_object_pack->raw_object->getAttributesDelta(2);
		((float*)&delta)[i] = ((float*)&ds)[i] / m_cam_inv_scale_factor;
		
	}

	// Render Translation Axis
	{
		
		// pass the model matrix to glsl , need it when object is on local space
		for (int i = 0; i < 3; i++)
		{
			
			glBindVertexArray(m_base_axis_node[i]->m_vao);
			for (int j = 0; j < m_base_axis_node[i]->parts.size(); j++)
			{
				
				optix::float3 p1 = optix::make_float3(0.0f); //+ GLOBAL_AXIS[i] * 0.75f;
				optix::float3 p2 = GLOBAL_AXIS[i] * (5.2f + 50.0f * ((float*)&delta)[i]);
				optix::float3 p3 = p2 + GLOBAL_AXIS[i] * 0.57f; // 0.77f

				
				
				// render translation axis line segment
				{
					if (AXIS_COLOR[i] == HIGHLIGHT_COLOR)
						glLineWidth(1.5f);
					else
						glLineWidth(UTILITY_LINE_WIDTH);
					diffuseColor = AXIS_COLOR[i] * 0.9f + glm::vec3(0.1f) * GLM_GLOBAL_INV_AXIS[i];
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					draw::Line3D((float*)&p1, (float*)&p2);
					draw::Cube(p2, 0.3f, (optix::float3*)GLOBAL_AXIS );
				}

				// render translation axis cone parts
				{

				}

				// render translation axis logo
				{
					diffuseColor = AXIS_COLOR[i];

					glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					glUniform1i(m_utility_rendering_program["uniform_text_render"], 1);
					draw::Text(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p3), glm::vec3(1), FONT6, GLOBAL_AXIS_NAME[i]);
					glUniform1i(m_utility_rendering_program["uniform_text_render"], 0);
				}

			}
		}
	}
	glLineWidth(UTILITY_LINE_WIDTH);


	// Render XY , XZ , YZ Planes
	{
		//glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(utility_to_Render.obj_pack->transformation_matrix));
		optix::float3 p1, p2, p3;
		float dist_factor = 2.7f; //2.5
		float dist_factor2 = 1.45f; // 1.35
		float ds_factor = 100.6f;
		float div_factor = 1/5.0f;

		// XY - plane line indicators
		{
			p1 = ZERO_3f + GLOBAL_AXIS[0] * (dist_factor + ds_factor * ((float*)&delta)[0] * div_factor );
			p2 = ZERO_3f + GLOBAL_AXIS[1] * (dist_factor + ds_factor * ((float*)&delta)[1] * div_factor );
			p3 = (p1 + p2) / 2.0f;

			if (selected_plane == XY || selected_plane == XYZ)
			{
				diffuseColor  = AXIS_COLOR[0];
				diffuseColor2 = AXIS_COLOR[1];
			}
			else
			{
				diffuseColor = DEF_AXIS_COLOR[0];
				diffuseColor2 = DEF_AXIS_COLOR[1];
			}

			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Line3D((float*)&(p1), (float*)&(p3));
			draw::Line3D((float*)&(p1*dist_factor2), (float*)&(p3*dist_factor2));

		
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor2.r, diffuseColor2.g, diffuseColor2.b);
			draw::Line3D((float*)&(p3), (float*)&(p2));
			draw::Line3D((float*)&(p3*dist_factor2), (float*)&(p2*dist_factor2));

			if (selected_plane == XY)
			{
				diffuseColor = PLANE_HIGHLIGHT_COLOR;
				glDisable(GL_CULL_FACE);
				
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
				glBegin(GL_POLYGON);
				glVertex3f(p1.x, p1.y, p1.z);
				glVertex3f(p2.x, p2.y, p2.z);
				glVertex3f(p2.x * dist_factor2 , p2.y * dist_factor2 , p2.z * dist_factor2);
				glVertex3f(p1.x* dist_factor2, p1.y* dist_factor2, p1.z* dist_factor2);
				glEnd();
				
			}

		}

		// XZ - plane line indicators
		{
			p1 = ZERO_3f + GLOBAL_AXIS[0] * (dist_factor + ds_factor * ((float*)&delta)[0] * div_factor );
			p2 = ZERO_3f + GLOBAL_AXIS[2] * (dist_factor + ds_factor * ((float*)&delta)[2] * div_factor );
			p3 = (p1 + p2) / 2.0f;


			if (selected_plane == XZ || selected_plane == XYZ)
			{
				diffuseColor = AXIS_COLOR[0];
				diffuseColor2 = AXIS_COLOR[2];
			}
			else
			{
				diffuseColor = DEF_AXIS_COLOR[0];
				diffuseColor2 = DEF_AXIS_COLOR[2];
			}
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Line3D((float*)&(p1), (float*)&(p3));
			draw::Line3D((float*)&(p1*dist_factor2), (float*)&(p3*dist_factor2));

			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor2.r, diffuseColor2.g, diffuseColor2.b);
			draw::Line3D((float*)&(p3), (float*)&(p2));
			draw::Line3D((float*)&(p3*dist_factor2), (float*)&(p2*dist_factor2));

			if (selected_plane == XZ)
			{
				diffuseColor = PLANE_HIGHLIGHT_COLOR;
				glDisable(GL_CULL_FACE);

				glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
				glBegin(GL_POLYGON);
				glVertex3f(p1.x, p1.y, p1.z);
				glVertex3f(p2.x, p2.y, p2.z);
				glVertex3f(p2.x * dist_factor2, p2.y * dist_factor2, p2.z * dist_factor2);
				glVertex3f(p1.x* dist_factor2, p1.y* dist_factor2, p1.z* dist_factor2);
				glEnd();
			}
		}

		// YZ - plane line indicators
		{
			p1 = ZERO_3f + GLOBAL_AXIS[1] * (dist_factor + ds_factor * ((float*)&delta)[1] * div_factor );
			p2 = ZERO_3f + GLOBAL_AXIS[2] * (dist_factor + ds_factor * ((float*)&delta)[2] * div_factor );
			p3 = (p1 + p2) / 2.0f;

			if (selected_plane == YZ || selected_plane == XYZ)
			{
				diffuseColor = AXIS_COLOR[1];
				diffuseColor2 = AXIS_COLOR[2];
			}
			else
			{
				diffuseColor = DEF_AXIS_COLOR[1];
				diffuseColor2 = DEF_AXIS_COLOR[2];
			}


			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Line3D((float*)&(p1), (float*)&(p3));
			draw::Line3D((float*)&(p1*dist_factor2), (float*)&(p3*dist_factor2));

			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor2.r, diffuseColor2.g, diffuseColor2.b);
			draw::Line3D((float*)&(p3), (float*)&(p2));
			draw::Line3D((float*)&(p3*dist_factor2), (float*)&(p2*dist_factor2));

			if (selected_plane == YZ)
			{
				diffuseColor = PLANE_HIGHLIGHT_COLOR;
				glDisable(GL_CULL_FACE);

				glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
				glBegin(GL_POLYGON);
				glVertex3f(p1.x, p1.y, p1.z);
				glVertex3f(p2.x, p2.y, p2.z);
				glVertex3f(p2.x * dist_factor2, p2.y * dist_factor2, p2.z * dist_factor2);
				glVertex3f(p1.x* dist_factor2, p1.y* dist_factor2, p1.z* dist_factor2);
				glEnd();
			}
		}

		// XYZ
		{
			p1 = ZERO_3f + GLOBAL_AXIS[0] * (dist_factor + ds_factor * ((float*)&delta)[0] * div_factor );
			p2 = ZERO_3f + GLOBAL_AXIS[1] * (dist_factor + ds_factor * ((float*)&delta)[1] * div_factor );
			p3 = ZERO_3f + GLOBAL_AXIS[2] * (dist_factor + ds_factor * ((float*)&delta)[2] * div_factor );

			if (selected_plane == XYZ)
			{
				diffuseColor = PLANE_HIGHLIGHT_COLOR;
				glDisable(GL_CULL_FACE);

				glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
				draw::Triangle_(p1, p2 - p1, p3 - p1);
			}
		}
	}

	//glUniform1i(m_utility_rendering_program["uniform_use_model_matrix"], 0);
}

void OpenGLRenderer::Render_Utility_Area_Selection(Utility_RenderList_Object utility_to_Render)
{
	

	// draw a dashed rectangle from point A -> to point B:
	const Io_Mouse& mouse_data = InterfaceManager::GetInstance().getMouseData();
	glm::vec2 mouse_start = glm::vec2(mouse_data.m0_down_x, mouse_data.m0_down_y);
	glm::vec2 mouse_end   = glm::vec2(mouse_data.x, mouse_data.y);

	draw::Rectangle2D(mouse_start, mouse_end, glm::vec3(1.0f), 4.0f, true);
}

void OpenGLRenderer::Render_Utility_Link(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{
	if (!vp.is_active)
		return;

	optix::float3 bbox_max    = utility_to_Render.obj_pack->raw_object->getGeomProperties().bbox_max;
	optix::float3 bbox_min    = utility_to_Render.obj_pack->raw_object->getGeomProperties().bbox_min;
	optix::float3 subject_pos = utility_to_Render.obj_pack->raw_object->getTranslationAttributes();
	optix::float3 object_pos;
	glm::vec3 line_color = glm::vec3(1.0f);
	float size = utility_to_Render.obj_pack->cam_inv_scale_factor;
	
	glm::mat4 model_matrix = glm::mat4(1.0f); //glm::scale(glm::mat4(1.0f), glm::vec3(0.6* utility_to_Render.obj_pack->cam_inv_scale_factor));

	
	// case : single selection is active
	if (!utility_to_Render.obj_pack->raw_object->isTemporary()
		&& utility_to_Render.obj_pack->raw_object->getType() != GROUP_PARENT_DUMMY)
	{
		float t = optix::length(subject_pos - utility_to_Render.camera_instance->getCameraPos());
		Ray mouse_ray = Geometry::SHAPES::createMouseRay();
		object_pos = mouse_ray.origin + mouse_ray.direction * t;

		Io_Mouse mouse_data = InterfaceManager::GetInstance().getMouseData();
		int x = mouse_data.m0_down_x;
		int y = mouse_data.m0_down_y;
		optix::float2 xy = Utilities::getMousePosRelativeToViewport(vp.id, optix::make_float2(x, y));
		

		Ray to_mouse_ray = Geometry::SHAPES::createMouseRay( glm::vec2(xy.x,xy.y));
		subject_pos = to_mouse_ray.origin + to_mouse_ray.direction * t;

		
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
		draw::stripped_Line3D(object_pos, subject_pos, size);
		//draw::Line3D((float*)&object_pos, (float*)&subject_pos);
	}

	else if (utility_to_Render.obj_pack->raw_object->getType() == GROUP_PARENT_DUMMY
			 && !utility_to_Render.obj_pack->raw_object->isTemporary())
	{
		
		SceneObject* subject = utility_to_Render.obj_pack->raw_object;

		float t = optix::length(subject_pos - utility_to_Render.camera_instance->getCameraPos());
		Ray mouse_ray = Geometry::SHAPES::createMouseRay();
		object_pos = mouse_ray.origin + mouse_ray.direction * t;

		Io_Mouse mouse_data = InterfaceManager::GetInstance().getMouseData();
		int x = mouse_data.m0_down_x;
		int y = mouse_data.m0_down_y;
		optix::float2 xy = Utilities::getMousePosRelativeToViewport(vp.id, optix::make_float2(x, y));


		Ray to_mouse_ray = Geometry::SHAPES::createMouseRay(glm::vec2(xy.x, xy.y));
		subject_pos = (subject->getBBoxMin() + subject->getBBoxMax()) * 0.5f; // subject->getTranslationAttributes();

		
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
		draw::stripped_Line3D(object_pos, subject_pos, size);

		Render_Object_BBOX(utility_to_Render.obj_pack->object_index, false);

	}

	// case : Area selection is active
	else
	{
		float t = optix::length(subject_pos - utility_to_Render.camera_instance->getCameraPos());
		Ray mouse_ray = Geometry::SHAPES::createMouseRay();
		object_pos = mouse_ray.origin + mouse_ray.direction * t;

		
		std::vector<SceneObject*> top_groups = InterfaceManager::GetInstance().get_Area_Selection_Data(utility_to_Render.obj_pack->raw_object->getId()).top_level_groups;
		std::vector<SceneObject*> top_objects = InterfaceManager::GetInstance().get_Area_Selection_Data(utility_to_Render.obj_pack->raw_object->getId()).solo_objects;
		for (SceneObject* obj : top_groups)
		{
			// object_pos  = mouse_pos
			// subject_pos = obj_pos
			subject_pos = (obj->getBBoxMin() + obj->getBBoxMax()) * 0.5f; //obj->getTranslationAttributes();


			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
			draw::stripped_Line3D(object_pos, subject_pos, size);
			Render_Object_BBOX(Mediator::requestObjectsArrayIndex(obj->getId()), false);
			//draw::Line3D((float*)&object_pos, (float*)&subject_pos);
		}
		for (SceneObject* obj : top_objects)
		{
			// object_pos  = mouse_pos
			// subject_pos = obj_pos
			subject_pos = obj->getTranslationAttributes() + (obj->getBBoxMin() + obj->getBBoxMax()) * 0.5f;


			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
			draw::stripped_Line3D(object_pos, subject_pos, size);
			//draw::Line3D((float*)&object_pos, (float*)&subject_pos);
		}

		/*
		std::vector<SceneObject*> selection_childs = utility_to_Render.obj_pack->raw_object->getGroupSelectionChilds();
		for (SceneObject* obj : selection_childs)
		{
			// object_pos  = mouse_pos
			// subject_pos = obj_pos
			subject_pos = obj->getTranslationAttributes() + (obj->getBBoxMin() + obj->getBBoxMax()) * 0.5f; 

			
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
			draw::stripped_Line3D(object_pos, subject_pos, size);
			//draw::Line3D((float*)&object_pos, (float*)&subject_pos);
		}
		*/
	}


	
}



// g_Attach
void OpenGLRenderer::Render_Utility_Attach_Sampler_Planar(Utility_RenderList_Object ut, const VIEWPORT& vp)
{
	
	bool use_bbox = interface_state->m_sampler_init_use_bbox;
	if (use_bbox)
	{
		Render_Utility_Attach_Planes(ut, vp);
		int isec_hit = interface_state->isec_p->hit_index;
		if (isec_hit == -1) return;
	}
	//


	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	SceneObject* object = ut.obj_pack->raw_object;
	
	optix::float3 local_axis[3];
	BASE_TRANSFORM::AXIS(object, LOCAL, local_axis);

	optix::float3 * bbox = object->getBBOX_Transformed();
	optix::float3 b_max = bbox[1];
	optix::float3 b_min = bbox[0];
	delete bbox;
	float bbox_size    = length((b_max - b_min)) * 0.15f;
	optix::float2 size = interface_state->m_sampler_params.dim_2f; //length(b_max - b_min) * 0.15f;

	optix::float3 m_axis[3] = { GLOBAL_AXIS[0],GLOBAL_AXIS[1],GLOBAL_AXIS[2] };
	optix::float3 color = optix::make_float3(0.1f, 0.9f, 0.2f);

	optix::float3 isec_p;
	optix::float3 po = b_min; 



	optix::float3 * mouse_data = Mediator::request_MouseHit_Buffer_Data();
	optix::float3 mouse_hit_p = interface_state->m_sampler_params.origin; //mouse_data[0];
	optix::float3 mouse_hit_n = mouse_data[1];
	
	SAMPLER_ALIGNEMENT SA = interface_state->m_sa;
	Sampler_Parameters s_params = interface_state->m_sampler_params;
	optix::float3 t_axis[3];
	t_axis[0] = s_params.basis[0];
	t_axis[1] = s_params.basis[1];
	t_axis[2] = s_params.basis[2];



    delete mouse_data;



	
	float offset    = bbox_size * 0.0f;
	optix::float3 p = mouse_hit_p;


	glDisable(GL_CULL_FACE);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.5f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
	draw::Plane_Centered(p, t_axis[0] * s_params.scale.x , t_axis[1] * s_params.scale.y, optix::make_float2(0.5f));

	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.0f, 0.0f, 0.0f);
	glLineWidth(2.0f);
	draw::Plane_Centered(p, t_axis[0] * s_params.scale.x, t_axis[1] * s_params.scale.y, optix::make_float2(0.5f), 1.0f, GL_LINE_LOOP); glLineWidth(1.0f);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
	glEnable(GL_CULL_FACE);

	glDisable(GL_DEPTH_TEST);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
	glLineWidth(1.0f);
	//draw::Line3D(object->getCentroid_Transformed(), p); glLineWidth(1.0f);
	draw::stripped_Line3D(object->getCentroid_Transformed(), p, 1.0f, false, 1.0f, 6);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
	glEnable(GL_DEPTH_TEST);



	// render axis //
	float s = 2.0f;
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR_X.x, DEF_AXIS_COLOR_X.y , DEF_AXIS_COLOR_X.z); draw::Line3D(p, p + t_axis[0] * s * ut.obj_pack->cam_inv_scale_factor);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR_Y.x, DEF_AXIS_COLOR_Y.y, DEF_AXIS_COLOR_Y.z);  draw::Line3D(p, p + t_axis[1] * s * ut.obj_pack->cam_inv_scale_factor);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR_Z.x, DEF_AXIS_COLOR_Z.y, DEF_AXIS_COLOR_Z.z);  draw::Line3D(p, p + t_axis[2] * s * ut.obj_pack->cam_inv_scale_factor);


}
void OpenGLRenderer::Render_Utility_Attach_Sampler_Volume(Utility_RenderList_Object ut, const VIEWPORT& vp)
{

	
	bool use_bbox = interface_state->m_sampler_init_use_bbox;
	if (use_bbox)
	{
		Render_Utility_Attach_Planes(ut, vp);
		int isec_hit = interface_state->isec_p->hit_index;
		if (isec_hit == -1) return;
	}
	//
	
	// mouse pos 
	// intersect xz plane with origin the object's centroid or object's bbox min point.\
	// Render a cube at that point
	//
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	SceneObject* object = ut.obj_pack->raw_object;

	optix::float3 local_axis[3];
	BASE_TRANSFORM::AXIS(object, LOCAL, local_axis);

	optix::float3 * bbox = object->getBBOX_Transformed();
	optix::float3 b_min = bbox[0];
	optix::float3 b_max = bbox[1];
	delete bbox;
	float bbox_size    = length(b_max - b_min) * 0.15f;
	optix::float3 size = interface_state->m_sampler_params.dim_3f;

	optix::float3 m_axis[3] = { GLOBAL_AXIS[0],GLOBAL_AXIS[1],GLOBAL_AXIS[2] };
	optix::float3 color = optix::make_float3(0.1f, 0.9f, 0.2f);

	optix::float3 isec_p;
	optix::float3 po = b_min; //object->getTranslationAttributes();


	optix::float3 * mouse_data = Mediator::request_MouseHit_Buffer_Data();
	optix::float3 mouse_hit_p = interface_state->m_sampler_params.origin;//mouse_data[0];
	optix::float3 mouse_hit_n = mouse_data[1];
	delete mouse_data;

	optix::float3 offset_3f = optix::make_float3(1.0f);
	float offset = 0.0f;
	optix::float3 offset_u = mouse_hit_n;

	optix::float3 t_axis[3];
	
	/*

	SAMPLER_ALIGNEMENT SA = interface_state->m_sa;
	OnB onb;
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

		float * v = (float *)&interface_state->m_sampler_params.dim_3f;
		offset = v[min_i] * 0.5f;
		offset_u = t_axis[min_i] * sign;

		BASE_TRANSFORM::AXIS(object, WORLD, t_axis);
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
		

		float * v = (float *)&interface_state->m_sampler_params.dim_3f;
		offset = v[min_i] * 0.5f;
		offset_u = t_axis[min_i] * sign;

		BASE_TRANSFORM::AXIS(object, LOCAL, t_axis);
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
		offset = interface_state->m_sampler_params.dim_3f.z * 0.5f;
	}


	t_axis[0] *= size.x;
	t_axis[1] *= size.y;
	t_axis[2] *= size.z;

	*/

    optix::float3 p = mouse_hit_p;// + offset_u * offset;
	Sampler_Parameters s_params = interface_state->m_sampler_params;
	t_axis[0] = s_params.basis[0] * s_params.scale.x;
	t_axis[1] = s_params.basis[1] * s_params.scale.y;
	t_axis[2] = s_params.basis[2] * s_params.scale.z;

	glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.5f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
	draw::Cube(p , 1.f, t_axis , true);

	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.0f, 0.0f, 0.0f);
	glLineWidth(1.0f);
	draw::Cube(p , 1.f, t_axis , false); glLineWidth(1.0f);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);

	glDisable(GL_DEPTH_TEST);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
	glLineWidth(1.0f);
	//draw::Line3D(object->getCentroid_Transformed(), p); glLineWidth(1.0f);
	draw::stripped_Line3D(object->getCentroid_Transformed(), p, 1.0f, false, 1.0f, 6);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
	glEnable(GL_DEPTH_TEST);


	glDisable(GL_DEPTH_TEST);
	for (int i = 0; i < 3; i++)t_axis[i] = normalize(t_axis[i]);
	float s = 2.0f;
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR_X.x, DEF_AXIS_COLOR_X.y, DEF_AXIS_COLOR_X.z);  draw::Line3D(p, p + t_axis[0] * s * ut.obj_pack->cam_inv_scale_factor);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR_Y.x, DEF_AXIS_COLOR_Y.y, DEF_AXIS_COLOR_Y.z);  draw::Line3D(p, p + t_axis[1] * s * ut.obj_pack->cam_inv_scale_factor);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR_Z.x, DEF_AXIS_COLOR_Z.y, DEF_AXIS_COLOR_Z.z);  draw::Line3D(p, p + t_axis[2] * s * ut.obj_pack->cam_inv_scale_factor);
	glEnable(GL_DEPTH_TEST);


}
void OpenGLRenderer::Render_Utility_Attach_Sampler_Directional(Utility_RenderList_Object ut, const VIEWPORT& vp)
{

	bool use_bbox = interface_state->m_sampler_init_use_bbox;
	if (use_bbox)
	{
		Render_Utility_Attach_Planes(ut, vp);
		int isec_hit = interface_state->isec_p->hit_index;
		if (isec_hit == -1) return;
	}
	//

	

	SceneObject* object = ut.obj_pack->raw_object;

	optix::float3 local_axis[3];
	BASE_TRANSFORM::AXIS(object, LOCAL, local_axis);

	optix::float3 * bbox = object->getBBOX_Transformed();
	optix::float3 b_min = bbox[0];
	optix::float3 b_max = bbox[1];
	delete bbox;
	float size = length(b_max - b_min) * 0.15f;

	optix::float3 m_axis[3] = { GLOBAL_AXIS[0],GLOBAL_AXIS[1],GLOBAL_AXIS[2] };
	optix::float3 color = optix::make_float3(0.1f, 0.9f, 0.2f);

	optix::float3 isec_p;
	optix::float3 po = b_min;


	Sampler_Parameters s_params = interface_state->m_sampler_params;
	optix::float3 * mouse_data = Mediator::request_MouseHit_Buffer_Data();
	optix::float3 mouse_hit_p = s_params.origin;//mouse_data[0];
	optix::float3 mouse_hit_n = mouse_data[1];


	SAMPLER_ALIGNEMENT SA = interface_state->m_sa;
	optix::float3 t_axis[3];
	t_axis[0] = s_params.basis[0];
	t_axis[1] = s_params.basis[1];
	t_axis[2] = s_params.basis[2];
	
	delete mouse_data;

	optix::Matrix4x4 mat = optix::Matrix4x4::fromBasis(t_axis[0], t_axis[2], t_axis[1], ZERO_3f);
	optix::float3 p = mouse_hit_p;



	PinholeCamera cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
	float scale_factor = cam.getDistanceFrom(object->getTranslationAttributes()) * cam.getInvScaleFactor() * LINKING_SCALE_FACTOR * FACE_VECTOR_SCALE_FACTOR * 0.045;

	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));
	glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p)) *  MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(mat) * scale_matrix;
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	optix::float2 cone_size = s_params.cone_size;
	draw::Frustum_Cone2(ZERO_3f, cone_size, optix::make_float2(0.0f, 20.0f), GLOBAL_AXIS[2], m_utility_rendering_program, 2.0f);

	model_matrix = glm::mat4(1.0f);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
	draw::stripped_Line3D(object->getCentroid_Transformed(), p, 1.0f, false, 1.0f, 6);

}


void OpenGLRenderer::Render_Utility_Attach_Planes(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{
	SceneObject* obj   = utility_to_Render.obj_pack->raw_object;
	float scale_factor = optix::length(obj->getScaleAttributes());
	optix::Matrix4x4 transformation     = MATRIX_TYPE_TRANSFORM::TO_OPTIX_MATRIX(utility_to_Render.obj_pack->translation_matrix_scaled);
	optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
	//transformation = def_transformation;

	// get centroid 
	optix::float3 centroid = obj->getCentroid();
	if (obj->getType() != GROUP_PARENT_DUMMY)
		centroid = optix::make_float3(def_transformation * optix::make_float4(centroid, 1.0f));

	// get all 8 bbox points 
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


	float inv_scale_factor = utility_to_Render.obj_pack->cam_inv_scale_factor;

	if (inv_scale_factor < 8.0f)
		inv_scale_factor = 8.0f;

	float dist = 2.5f * inv_scale_factor;
	float size = 1.4f * inv_scale_factor;
	dist = 0.0f;
	size = 0.0f;
	//const float offset = 40.0f + dist;
	const float offset = (obj->getType() != GROUP_PARENT_DUMMY)? (d_max + d_min) * 0.05f : 3.5f;


	optix::float2 scale = optix::make_float2(offset * 0.1f);
	optix::float3 * M_WORKING_AXIS = utility_to_Render.obj_pack->base_axis;

#define draw_planes
#ifdef draw_planes


	glDisable(GL_CULL_FACE);

	glm::vec3 diffuseColor;
	glm::mat4 model_matrix = glm::mat4(1.0f);
	float alpha = 0.8f;

	if (obj->getType() != GROUP_PARENT_DUMMY)
	{
		model_matrix = utility_to_Render.obj_pack->transformation_matrix;
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.7f, 0.7f, 0.7f);
		Render_GeometryNode(obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_matrix, m_utility_rendering_program, true);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
	}

	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.f, 1.f, 1.f);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	draw::BBox(bbox_min, bbox_max, ZERO_3f, 1.0f);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	Ray ray = Geometry::SHAPES::createMouseRay();
	optix::float3 isec_p[6];

	int hit_index = interface_state->isec_p->hit_index;
	for (int i = 0; i < 6; i++)
	{

#define SCALE_DIST_METHOD_A
#ifdef SCALE_DIST_METHOD_A

		optix::float3 p0 = planes[i].centroid + r_normals[i] * offset;
		optix::float3 du = normalize(planes[i].u) * scale.x;
		optix::float3 dv = normalize(planes[i].v) * scale.y;
		optix::float3 u = planes[i].u * 0.5f + du;
		optix::float3 v = planes[i].v * 0.5f + dv;

#endif

		//#define SCALE_DIST_METHOD_B
#ifdef SCALE_DIST_METHOD_B

		optix::float3 p0 = planes[i].centroid + r_normals[i] * 7.0f * inv_scale_factor;
		optix::float3 du = normalize(planes[i].u) * scale.x;
		optix::float3 dv = normalize(planes[i].v) * scale.y;
		optix::float3 u = planes[i].u * 0.07f * inv_scale_factor;
		optix::float3 v = planes[i].v * 0.07f * inv_scale_factor;

#endif	

		Plane p1 = Geometry::SHAPES::createPlane(p0, u, v);
		Plane p2 = Geometry::SHAPES::createPlane(p0, -u, -v);
		Plane p3 = Geometry::SHAPES::createPlane(p0, u, -v);
		Plane p4 = Geometry::SHAPES::createPlane(p0, -u, v);


		Plane render_plane = Geometry::SHAPES::createPlane_Centered_(p0,
																	 u,
																	 v);

		diffuseColor = hit_index == i ? HIGHLIGHT_COLOR * 0.8f : glm::vec3(0.7f);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.5f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		draw::Plane(render_plane, ZERO_3f);

		if (hit_index == i)
			glDisable(GL_DEPTH_TEST);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
		draw::Plane(render_plane, ZERO_3f, 1.0f, 2.0f, GL_LINE_LOOP);
		glEnable(GL_DEPTH_TEST);

		if (hit_index == i)
		{
			glDisable(GL_DEPTH_TEST);


			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			optix::float3 du = normalize(planes[i].u);
			optix::float3 dv = normalize(planes[i].v);
			//optix::float3 p0 = planes[i].p_normal;

			// Render u,v Lines over plane //
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			draw::stripped_Line3D(p0, p0 + u, 1.0f, false);
			draw::stripped_Line3D(p0, p0 + v, 1.0f, false);
			draw::stripped_Line3D(p0, p0 - u, 1.0f, false);
			draw::stripped_Line3D(p0, p0 - v, 1.0f, false);


			// Render cross on Normal_Point on hovered Plane //
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.8f, 0.8f, 0.8f);
			draw::Line3D(p0, p0 + du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 + dv * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - dv * 0.5f * inv_scale_factor, 3.0f);

			glEnable(GL_DEPTH_TEST);
		}

	}


	{

		if (hit_index != -1)
		{
			
		}

	}


#endif


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}
void OpenGLRenderer::Render_Utility_Attach_Face_Vector(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{
	Render_Utility_Attach_Face_vector4(utility_to_Render,vp);
	return;
	
	SceneObject* obj = utility_to_Render.obj_pack->raw_object;
	float scale_factor = optix::length(obj->getScaleAttributes());
	optix::Matrix4x4 transformation     = MATRIX_TYPE_TRANSFORM::TO_OPTIX_MATRIX( utility_to_Render.obj_pack->translation_matrix_scaled );
	optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
	//transformation = def_transformation;

	// get centroid 
	optix::float3 centroid = obj->getCentroid();
	centroid = optix::make_float3( def_transformation * optix::make_float4(centroid, 1.0f) );
	
	// get all 8 bbox points 
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

	for (int i = 0; i < 8; i++)
		p[i] = optix::make_float3( def_transformation * optix::make_float4(p[i], 1.0f));

	// xy plane //   
	optix::float3 x_u  = (p[0] + p[2]) * 0.5f;
	optix::float3 x_u_ = (p[4] + p[6]) * 0.5f;
	float          du  = length(x_u - centroid);
	float          du_ = length(x_u_ - centroid);
	optix::float3  u   = normalize(x_u - centroid);
	optix::float3  u_  = normalize(x_u_ - centroid);
	

	// xz plane //
	optix::float3 y_v  = (p[0] + p[7]) * 0.5f; 
	optix::float3 y_v_ = (p[1] + p[4]) * 0.5f;
	float          dv  = length(y_v - centroid);
	float          dv_ = length(y_v_ - centroid);
	optix::float3  v   = normalize(y_v - centroid);
	optix::float3  v_  = normalize(y_v_ - centroid);

	// yz plane //
	optix::float3 z_w  = (p[3] + p[4]) * 0.5f; 
	optix::float3 z_w_ = (p[0] + p[5]) * 0.5f;
	float          dw  = length(z_w - centroid);
	float          dw_ = length(z_w_ - centroid);
	optix::float3  w   = normalize(z_w - centroid);
	optix::float3  w_  = normalize(z_w_ - centroid);

	// OnB for every plane //
	OnB_uv onb_uv[3];
	onb_uv[0] = OnB_uv(v, w); // xy
	onb_uv[1] = OnB_uv(w, u); // xz
	onb_uv[2] = OnB_uv(u, v); // yz

	//
	float dist = 2.f;  // distance from actual bbox plane
	float size = 10.0f; // utility_to_Render.obj_pack->cam_inv_scale_factor; // size
	optix::float2 scale = optix::make_float2(size * scale_factor);

	optix::float3 _p[6];
	_p[0] = centroid + u  * du  * dist;
	_p[1] = centroid + u_ * du_ * dist;
	
	_p[2] = centroid + v  * dv  * dist;
	_p[3] = centroid + v_ * dv_ * dist;

	_p[4] = centroid + w  * dw  * dist;
	_p[5] = centroid + w_ * dw_ * dist;

	
#define draw_planes
#ifdef draw_planes

	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glm::vec3 diffuseColor;
	glm::mat4 model_matrix;
	float alpha = 0.8f;

	model_matrix = utility_to_Render.obj_pack->transformation_matrix;
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniform1f( m_utility_rendering_program["uniform_alpha"], 0.0f );
	glUniform3f( m_utility_rendering_program["uniform_diffuse"], 0.7f, 0.7f, 0.7f );
	Render_GeometryNode( obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_matrix, m_utility_rendering_program, true );
	glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha );
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	int k = 0;
	for (int i = 0; i < 6; i += 2)
	{

		bool hovered[2] = { false, false };
		optix::float3 isec_p[2] = { ZERO_3f, ZERO_3f };

		{
			//
			Ray ray     = Geometry::SHAPES::createMouseRay();
			Plane plane = Geometry::SHAPES::createPlane_Centered(_p[i], onb_uv[k].u*scale.x, onb_uv[k].v*scale.y, optix::Matrix4x4::identity(), false, true);
			if( Geometry::RAY::Intersect_Parallelogram(ray, plane, isec_p[0]) == 1)
			//if (Geometry::RAY::Intersect_Plane_Clipped(ray, plane, isec_p) == 1)
			{
				hovered[0] = true;
			}

			
			plane = Geometry::SHAPES::createPlane_Centered(_p[i+1], onb_uv[k].u*scale.x, onb_uv[k].v*scale.y, optix::Matrix4x4::identity(), false, true);
			if (Geometry::RAY::Intersect_Parallelogram(ray, plane, isec_p[1]) == 1)
			//if (Geometry::RAY::Intersect_Plane_Clipped(ray, plane, isec_p) == 1)
			{
				hovered[1] = true;
			}
			
		}

		diffuseColor = hovered[0] ? HIGHLIGHT_COLOR : glm::vec3(0.7f);
		glUniform1f( m_utility_rendering_program["uniform_alpha"], 0.5f );
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b );
		draw::Plane_Centered
		(_p[i],
		 onb_uv[k].u,
		 onb_uv[k].v,
		 scale);

		diffuseColor = hovered[1] ? HIGHLIGHT_COLOR : glm::vec3(0.7f);
		glUniform1f( m_utility_rendering_program["uniform_alpha"], 0.5f );
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		draw::Plane_Centered
		(_p[i+1],
		 onb_uv[k].u,
		 onb_uv[k].v,
		 scale);


		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f );
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
		draw::Plane_Centered
		(_p[i],
		 onb_uv[k].u,
		 onb_uv[k].v,
		 scale,
		 2.0f,
		 GL_LINE_LOOP);

		draw::Plane_Centered
		(_p[i + 1],
		 onb_uv[k].u,
		 onb_uv[k].v,
		 scale,
		 2.0f,
		 GL_LINE_LOOP);



		if (hovered[0])
		{
			glDisable(GL_DEPTH_TEST);
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
			draw::Point(isec_p[0], 5.0f);
			draw::Line3D(centroid, centroid + (isec_p[0] - centroid) * 10000.0f);
			glLineWidth(2.0f);
			draw::Line3D(centroid, isec_p[0]);
			draw::Arrow(centroid, isec_p[0], 3.0f * scale_factor, false);
			glLineWidth(1.0f);
			glEnable(GL_DEPTH_TEST);
		}
		if (hovered[1])
		{
			glDisable(GL_DEPTH_TEST);
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
			draw::Point(isec_p[1], 5.0f);
			draw::Line3D(centroid, centroid + (isec_p[1] - centroid) * 10000.0f);
			glLineWidth(2.0f);
			draw::Line3D(centroid, isec_p[1]);
			draw::Arrow(centroid, isec_p[1], 3.0f * scale_factor, false);
			glLineWidth(1.0f);
			glEnable(GL_DEPTH_TEST);
		}
		

		k++;
	}


	optix::float3 * M_WORKING_AXIS = utility_to_Render.obj_pack->base_axis;
	// draw axis //
	{
		glDisable(GL_DEPTH_TEST);
		for (int i = 0; i < 3; i++)
		{
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR[i].r, DEF_AXIS_COLOR[i].g, DEF_AXIS_COLOR[i].b);
			draw::Line3D( centroid,  centroid + M_WORKING_AXIS[i] * 4.0f * scale_factor * utility_to_Render.obj_pack->cam_inv_scale_factor );
			draw::Line3D( centroid,  centroid - M_WORKING_AXIS[i] * 4.0f * scale_factor * utility_to_Render.obj_pack->cam_inv_scale_factor );
		}
		glEnable(GL_DEPTH_TEST);
	}
	

#endif
	
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}
void OpenGLRenderer::Render_Utility_Attach_Face_vector2(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{
	Render_Utility_Attach_Face_vector3(utility_to_Render,vp);
	return;

	SceneObject* obj = utility_to_Render.obj_pack->raw_object;
	float scale_factor = optix::length(obj->getScaleAttributes());
	optix::Matrix4x4 transformation = MATRIX_TYPE_TRANSFORM::TO_OPTIX_MATRIX(utility_to_Render.obj_pack->translation_matrix_scaled);
	optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
	//transformation = def_transformation;

	// get centroid 
	optix::float3 centroid = obj->getCentroid();
	if(obj->getType() != GROUP_PARENT_DUMMY)
		centroid = optix::make_float3(def_transformation * optix::make_float4(centroid, 1.0f));

	// get all 8 bbox points 
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
	Plane planes[6];
	{
		// xy           // xy_
		xy.p0 = p[0];   xy_.p0 = p[6];
		xy.p1 = p[3];   xy_.p1 = p[7];
		xy.p2 = p[2];   xy_.p2 = p[4];
		xy.p3 = p[1];   xy_.p3 = p[5];
		
		xy.u       = (xy.p1 - xy.p0);
		xy.v       = (xy.p3 - xy.p0);
		xy.normal  = -cross(xy.u, xy.v);
		xy.normal  = ((xy.p0 + xy.p2) * 0.5f - centroid);
		planes[0] = xy;

		xy_.u      = (xy_.p1 - xy_.p0);
		xy_.v      = (xy_.p3 - xy_.p0);
		xy_.normal = -cross(xy_.u, xy_.v);
		xy_.normal = ((xy_.p0 + xy_.p2) * 0.5f - centroid);
		planes[1] = xy_;


		// yz           // yz_
		yz.p0 = p[3];   yz_.p0 = p[0];
		yz.p1 = p[7];   yz_.p1 = p[6];
		yz.p2 = p[4];   yz_.p2 = p[5];
		yz.p3 = p[2];   yz_.p3 = p[1];
		
		yz.u       = (yz.p1 - yz.p0);
		yz.v       = (yz.p3 - yz.p0);
		yz.normal  = -cross(yz.u, yz.v);
		yz.normal  = ((yz.p0 + yz.p2) * 0.5f - centroid);
		planes[2] = yz;

		yz_.u      = (yz_.p1 - yz_.p0);
		yz_.v      = (yz_.p3 - yz_.p0);
		yz_.normal = -cross(yz_.u, yz_.v);
		yz_.normal = ((yz_.p0 + yz_.p2) * 0.5f - centroid);
		planes[3] = yz_;



		// xz           // xz_
		xz.p0 = p[0];   xz_.p0 = p[1];
		xz.p1 = p[6];   xz_.p1 = p[5];
		xz.p2 = p[7];   xz_.p2 = p[4];
		xz.p3 = p[3];   xz_.p3 = p[2];

		xz.u       = (xz.p1 - xz.p0);
		xz.v       = (xz.p3 - xz.p0);
		xz.normal  = -cross(xz.u, xz.v);
		xz.normal  = ((xz.p0 + xz.p2) * 0.5f - centroid);
		planes[4] = xz;

		xz_.u      = (xz_.p1 - xz_.p0);
		xz_.v      = (xz_.p3 - xz_.p0);
		xz_.normal = -cross(xz_.u, xz_.v);
		xz_.normal = ((xz_.p0 + xz_.p2) * 0.5f - centroid);
		planes[5] = xz_;
	}


	
	float inv_scale_factor = utility_to_Render.obj_pack->cam_inv_scale_factor;
	float dist = 1.0f; //50.8f; //+ sqrtf(inv_scale_factor) * 0.5f; //0.3f * inv_scale_factor;  // distance from actual bbox plane
	float size = 15.0f;  //+ sqrtf(inv_scale_factor) * 0.5f; // utility_to_Render.obj_pack->cam_inv_scale_factor; // size
	optix::float2 scale = optix::make_float2( size * scale_factor );
	scale = optix::make_float2(1.0f);

	optix::float3 * M_WORKING_AXIS = utility_to_Render.obj_pack->base_axis;
	

#define draw_planes
#ifdef draw_planes

	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glm::vec3 diffuseColor;
	glm::mat4 model_matrix;
	float alpha = 0.8f;

	if (obj->getType() != GROUP_PARENT_DUMMY)
	{
		model_matrix = utility_to_Render.obj_pack->transformation_matrix;
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.7f, 0.7f, 0.7f);
		Render_GeometryNode(obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_matrix, m_utility_rendering_program, true);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
	}
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	Ray ray = Geometry::SHAPES::createMouseRay();
	
	optix::float3 isec_p[6];
	int hit_i = -1;
	int k = 0;
	for (int i = 0; i < 6; i ++)
	{
		
		Plane p1 = Geometry::SHAPES::createPlane(planes[i].p0 + planes[i].normal*dist, planes[i].u, planes[i].v);
		Plane p2 = Geometry::SHAPES::createPlane(planes[i + 1].p0 + planes[i + 1].normal*dist, planes[i + 1].u, planes[i + 1].v);
	
		hit_i = Geometry::RAY::Intersect_Parallelogram(ray, p1, isec_p[i]  ) == 1? i   : hit_i;
		hit_i = Geometry::RAY::Intersect_Parallelogram(ray, p2, isec_p[i+1]) == 1? i+1 : hit_i;
		
		diffuseColor = hit_i == i   ? HIGHLIGHT_COLOR * 0.8f : glm::vec3(0.7f);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.5f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		draw::Plane(planes[i], planes[i].normal * dist);

		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
		draw::Plane(planes[i], planes[i].normal * dist, 1.0f, 2.0f, GL_LINE_LOOP);
	
		if (hit_i == i)
		{
			glDisable(GL_DEPTH_TEST);

			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			optix::float3 du = normalize(planes[i].u);
			optix::float3 dv = normalize(planes[i].v);
			optix::float3 p0 = (planes[i].p0 + planes[i].p2) * 0.5f + planes[i].normal * dist;

			draw::stripped_Line3D(p0, p0 + planes[i].u * 0.5f, 1.0f, false);
			draw::stripped_Line3D(p0, p0 + planes[i].v * 0.5f, 1.0f, false);
			draw::stripped_Line3D(p0, p0 - planes[i].u * 0.5f, 1.0f, false);
			draw::stripped_Line3D(p0, p0 - planes[i].v * 0.5f, 1.0f, false);

			
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.9f, 0.9f, 0.9f);
			draw::Line3D(p0, p0 + du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 + dv * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - dv * 0.5f * inv_scale_factor, 3.0f);

			glEnable(GL_DEPTH_TEST);
		}
	}

	{

		// draw axis //
		{
			optix::float3 p0 = hit_i != -1 ? (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f : centroid;
			glDisable(GL_DEPTH_TEST);
			int k = 0;
			for (int i = 0; i < 6; i += 2)
			{
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR[i].r, DEF_AXIS_COLOR[i].g, DEF_AXIS_COLOR[i].b);
				//draw::Line3D(p0, centroid + planes[i].normal     * 4.0f  *  utility_to_Render.obj_pack->cam_inv_scale_factor);
				//draw::Line3D(p0, centroid + planes[i + 1].normal * 4.0f  *  utility_to_Render.obj_pack->cam_inv_scale_factor);

				draw::Line3D(p0, p0 + M_WORKING_AXIS[k] * 400.0f  *  utility_to_Render.obj_pack->cam_inv_scale_factor);
				draw::Line3D(p0, p0 - M_WORKING_AXIS[k] * 400.0f  *  utility_to_Render.obj_pack->cam_inv_scale_factor);
				k++;
			}
			glEnable(GL_DEPTH_TEST);
		}


		if (hit_i != -1)
		{
			glDisable(GL_DEPTH_TEST);

		
			optix::float3 p0 = (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f;
			optix::float3 u = normalize(isec_p[hit_i] - p0);
			
			

			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.3f, 0.05f, 0.88f);
			optix::float3 du = normalize(planes[hit_i].u);
			optix::float3 dv = normalize(planes[hit_i].v);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] + du * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] + dv * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] - du * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] - dv * 0.3f * inv_scale_factor, 2.0f);

			// test //
			
			{

				Plane n1 = Geometry::SHAPES::createPlane(p0, normalize(planes[hit_i].u));
				Plane n2 = Geometry::SHAPES::createPlane(p0, normalize(planes[hit_i].v));
				
				optix::float3 u1 = Geometry::VECTOR_SPACE::projectPointToPlane(n1, isec_p[hit_i]);
				optix::float3 u2 = Geometry::VECTOR_SPACE::projectPointToPlane(n2, isec_p[hit_i]);

				float d1 = dot ( normalize(u1 - p0), normalize(planes[hit_i].normal));
				float d2 = dot ( normalize(u2 - p0), normalize(planes[hit_i].normal));
				d1    = acosf(d1);
				d2    = acosf(d2);
				
				int sign1 = dot(planes[hit_i].v, u1 - p0) < 0.0f ? -1 : 1;
				int sign2 = dot(planes[hit_i].u, u2 - p0) < 0.0f ? -1 : 1;

				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);

				glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.65f, 0.50f, 0.50f);
				draw::Arc_arbitr_plane(p0, 20.f, 0.0f, d1 * sign1, normalize(planes[hit_i].normal), normalize(planes[hit_i].v), 30, 1, GL_POLYGON);
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.50f, 0.65f, 0.50f);
				draw::Arc_arbitr_plane(p0, 20.f, 0.0f, d2 * sign2, normalize(planes[hit_i].normal), normalize(planes[hit_i].u), 30, 1, GL_POLYGON);
				
				glDisable(GL_DEPTH_TEST);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
				draw::Arc_arbitr_plane(p0, 20.01f, 0.0f, d1 * sign1, normalize(planes[hit_i].normal), normalize(planes[hit_i].v), 30, 1, GL_LINE_LOOP);
				draw::Arc_arbitr_plane(p0, 20.01f, 0.0f, d2 * sign2, normalize(planes[hit_i].normal), normalize(planes[hit_i].u), 30, 1, GL_LINE_LOOP);
				
				// draw degreeds //
				glm::vec3 _p1 = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p0 + normalize(u1 - p0) * 20.0f + normalize(planes[hit_i].v) * 1.0f - normalize(planes[hit_i].u) * 2.0f );
				glm::vec3 _p2 = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p0 + normalize(u2 - p0) * 20.0f + normalize(planes[hit_i].u) * 1.0f - normalize(planes[hit_i].v) * 2.0f );
				std::string deg_d1 = std::to_string(Geometry::GENERAL::toDegrees(d1 * sign1)); if ( deg_d1.size() > 6 + (1 - sign1)/2 ) deg_d1.erase(deg_d1.begin() + 5 +(1-sign1)/2 , deg_d1.end());
				std::string deg_d2 = std::to_string(Geometry::GENERAL::toDegrees(d2 * sign2)); if ( deg_d2.size() > 6 + (1 - sign2)/2 ) deg_d2.erase(deg_d2.begin() + 5 +(1-sign2)/2 , deg_d2.end());
				
				//draw::Text( VECTOR_TYPE_TRANSFORM::TO_GLM_3f(u1), glm::vec3(0.0f), FONT5, (char *)deg_d1.c_str());
				//draw::Text( VECTOR_TYPE_TRANSFORM::TO_GLM_3f(u2), glm::vec3(0.0f), FONT5, (char *)deg_d2.c_str());
				draw::Text(_p1, glm::vec3(0.0f), FONT3, (char *)deg_d1.c_str());
				draw::Text(_p2, glm::vec3(0.0f), FONT3, (char *)deg_d2.c_str());


				// draw Face Vector //
				{
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
					draw::Line3D(isec_p[hit_i]    + u * 10000.0f, p0, 1.0f);
					draw::Line3D(isec_p[hit_i]    + u * 20.0f   , p0, 2.0f);
					draw::Arrow(p0, isec_p[hit_i] + u * 20.0f   , 1.f * inv_scale_factor, false);
				}

				// draw degree symbol //
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
				int off1 = deg_d1.size();
				int off2 = deg_d2.size();
				std::string o_1, o_2;
				for (int i = 0; i < off1; i++)o_1 += "  ";
				for (int i = 0; i < off2; i++)o_2 += "  ";
				o_1 += "o";
				o_2 += "o";
				draw::Text(_p1 + GLM_GLOBAL_AXIS_Y * 1.f - GLM_GLOBAL_AXIS_X * 0.3f , glm::vec3(0.0f), FONT5, (char*)o_1.c_str());
				draw::Text(_p2 + GLM_GLOBAL_AXIS_Y * 1.f - GLM_GLOBAL_AXIS_X * 0.3f , glm::vec3(0.0f), FONT5, (char*)o_2.c_str());

			}
			glEnable(GL_DEPTH_TEST);
		}
		
	}



	


#endif


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}
void OpenGLRenderer::Render_Utility_Attach_Face_vector3(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{

	Render_Utility_Attach_Face_vector4(utility_to_Render,vp);
	return;

	SceneObject* obj = utility_to_Render.obj_pack->raw_object;
	float scale_factor = optix::length(obj->getScaleAttributes());
	optix::Matrix4x4 transformation = MATRIX_TYPE_TRANSFORM::TO_OPTIX_MATRIX(utility_to_Render.obj_pack->translation_matrix_scaled);
	optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
	//transformation = def_transformation;

	// get centroid 
	optix::float3 centroid = obj->getCentroid();
	if (obj->getType() != GROUP_PARENT_DUMMY)
		centroid = optix::make_float3(def_transformation * optix::make_float4(centroid, 1.0f));

	// get all 8 bbox points 
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
		xy.normal = -cross(xy.u, xy.v);
		xy.normal = ((xy.p0 + xy.p2) * 0.5f - centroid);
		planes[0] = xy;
		r_normals[0] = normalize(cross(planes[0].u, planes[0].v));
		//r_dist[0] = (planes[0].p0 + planes[0].p2)*0.5f
		

		xy_.u = (xy_.p1 - xy_.p0);
		xy_.v = (xy_.p3 - xy_.p0);
		xy_.normal = -cross(xy_.u, xy_.v);
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
		yz.normal = -cross(yz.u, yz.v);
		yz.normal = ((yz.p0 + yz.p2) * 0.5f - centroid);
		planes[2] = yz;
		r_normals[2] = normalize(cross(planes[2].u, planes[2].v));

		yz_.u = (yz_.p1 - yz_.p0);
		yz_.v = (yz_.p3 - yz_.p0);
		yz_.normal = -cross(yz_.u, yz_.v);
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
		xz.normal = -cross(xz.u, xz.v);
		xz.normal = ((xz.p0 + xz.p2) * 0.5f - centroid);
		planes[4] = xz;
		r_normals[4] = normalize(cross(planes[4].u, planes[4].v));


		xz_.u = (xz_.p1 - xz_.p0);
		xz_.v = (xz_.p3 - xz_.p0);
		xz_.normal = -cross(xz_.u, xz_.v);
		xz_.normal = ((xz_.p0 + xz_.p2) * 0.5f - centroid);
		planes[5] = xz_;
		r_normals[5] = normalize(cross(planes[5].u, planes[5].v));
	}
	for (int i = 0; i < 6; i++)
	{
		planes[i].centroid = (planes[i].p0 + planes[i].p2) * 0.5f;
		
		Ray to_plane;
		to_plane.origin    = planes[i].centroid;
		to_plane.direction = r_normals[i];
		Geometry::RAY::Intersect_Plane
		(to_plane,
		 Geometry::SHAPES::createPlane(planes[i].p0 + planes[i].normal , r_normals[i]), planes[i].p_normal);
	}
	

	float inv_scale_factor = utility_to_Render.obj_pack->cam_inv_scale_factor;
	float dist = 1.0f; //50.8f; //+ sqrtf(inv_scale_factor) * 0.5f; //0.3f * inv_scale_factor;  // distance from actual bbox plane
	float size = 15.0f;  //+ sqrtf(inv_scale_factor) * 0.5f; // utility_to_Render.obj_pack->cam_inv_scale_factor; // size
	optix::float2 scale = optix::make_float2(size * scale_factor);
	scale = optix::make_float2(1.0f);

	optix::float3 * M_WORKING_AXIS = utility_to_Render.obj_pack->base_axis;


#define draw_planes
#ifdef draw_planes

	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glm::vec3 diffuseColor;
	glm::mat4 model_matrix;
	float alpha = 0.8f;

	if (obj->getType() != GROUP_PARENT_DUMMY)
	{
		model_matrix = utility_to_Render.obj_pack->transformation_matrix;
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.7f, 0.7f, 0.7f);
		Render_GeometryNode(obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_matrix, m_utility_rendering_program, true);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
	}
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	Ray ray = Geometry::SHAPES::createMouseRay();

	optix::float3 isec_p[6];
	int hit_i = -1;
	int k = 0;
	for (int i = 0; i < 6; i++)
	{
		
		Plane p1 = Geometry::SHAPES::createPlane( planes[i].p0 + planes[i].normal*dist, planes[i].u, planes[i].v );
		//Plane p2 = Geometry::SHAPES::createPlane( planes[i + 1].p0 + planes[i + 1].normal*dist, planes[i + 1].u, planes[i + 1].v );
		
		hit_i = Geometry::RAY::Intersect_Parallelogram(ray, p1 , isec_p[i]) == 1 ? i : hit_i;
		//hit_i = Geometry::RAY::Intersect_Parallelogram(ray, p2, isec_p[i + 1]) == 1 ? i + 1 : hit_i;
		
		diffuseColor = hit_i == i ? HIGHLIGHT_COLOR * 0.8f : glm::vec3(0.7f);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.5f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		draw::Plane(planes[i], planes[i].normal * dist);

		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
		draw::Plane(planes[i], planes[i].normal * dist, 1.0f, 2.0f, GL_LINE_LOOP);

		if (hit_i == i)
		{
			glDisable(GL_DEPTH_TEST);

			
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			optix::float3 du = normalize(planes[i].u);
			optix::float3 dv = normalize(planes[i].v);
			
			optix::float3 bbox_p0 = (planes[i].p0 + planes[i].p2) * 0.5f;
			optix::float3 plane_o = bbox_p0 + planes[i].normal * dist;
			Ray to_plane;
			to_plane.origin    = bbox_p0;
			to_plane.direction = normalize( r_normals[i] );
			
			optix::float3 p0;
			Geometry::RAY::Intersect_Plane
			( to_plane,
			 Geometry::SHAPES::createPlane( planes[i].p0 + planes[i].normal * dist, r_normals[i]), p0);

			float d_u, d_v;    // distances across {  u , v  }
			float d_u_, d_v_;  // distances across { -u ,-v  }
			optix::float3 dp;
			optix::float3 dp_proj_u, dp_proj_v;
			float p0_du, p0_dv;
			int s_u, s_v;
			{
				dp          = p0 - plane_o; // vector to Normal Point from plane's centroid
				dp_proj_u   = dp * normalize( planes[i].u ); // projection on plane's u
				dp_proj_v   = dp * normalize( planes[i].v ); // projection on plane's v
				p0_du = length( dp_proj_u ); // distance projected on plane's u
				p0_dv = length( dp_proj_v ); // distance projected on plane's v

				float half_plane_u = length( planes[i].u * 0.5f ); // dist of the half plane across u
				float half_plane_v = length( planes[i].v * 0.5f ); // dist of the half plane across v
				
				// define sign of dp relative to plane's { u , v }
				s_u = dot( dp_proj_u , planes[i].u ) >= 0.0f ? 1 : -1; // (+/-) sign across plane's u
				s_v = dot( dp_proj_v , planes[i].v ) >= 0.0f ? 1 : -1; // (+/-) sign across plane's v

				d_u  = s_u == 1 ? ( half_plane_u - p0_du ) : ( half_plane_u + p0_du );  // distance across  u
				d_v  = s_v == 1 ? ( half_plane_v - p0_dv ) : ( half_plane_v + p0_dv );  // distance across  v
				d_u_ = ( half_plane_u * 2.0f - d_u );                                   // distance across -u
				d_v_ = ( half_plane_v * 2.0f - d_v );                                   // distance across -v

			}
			

			// Render u,v Lines over plane //
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			draw::stripped_Line3D(p0, p0 + d_u  *   normalize(planes[i].u), 1.0f, false);
			draw::stripped_Line3D(p0, p0 + d_v  *   normalize(planes[i].v), 1.0f, false);
			draw::stripped_Line3D(p0, p0 + d_u_ * - normalize(planes[i].u), 1.0f, false);
			draw::stripped_Line3D(p0, p0 + d_v_ * - normalize(planes[i].v), 1.0f, false);
			

			// Render cross on hovered Plane //
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.9f, 0.9f, 0.9f);
			draw::Line3D(p0, p0 + du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 + dv * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - dv * 0.5f * inv_scale_factor, 3.0f);

			glEnable(GL_DEPTH_TEST);
		}
	}


	{
		// draw axis //
		{
			optix::float3 p0 = hit_i != -1 ? (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f : centroid;
			glDisable(GL_DEPTH_TEST);
			int k = 0;
			for (int i = 0; i < 6; i += 2)
			{
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR[i].r, DEF_AXIS_COLOR[i].g, DEF_AXIS_COLOR[i].b);
				draw::Line3D(p0, p0 + M_WORKING_AXIS[k] * 80.0f  *  utility_to_Render.obj_pack->cam_inv_scale_factor);
				draw::Line3D(p0, p0 - M_WORKING_AXIS[k] * 80.0f  *  utility_to_Render.obj_pack->cam_inv_scale_factor);
				k++;
			}
			glEnable(GL_DEPTH_TEST);
		}


		if (hit_i != -1)
		{
			glDisable(GL_DEPTH_TEST);

			optix::float3 p0 = (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f;
			optix::float3 u  = normalize(isec_p[hit_i] - p0);

			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.3f, 0.05f, 0.88f);
			optix::float3 du = normalize(planes[hit_i].u);
			optix::float3 dv = normalize(planes[hit_i].v);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] + du * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] + dv * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] - du * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] - dv * 0.3f * inv_scale_factor, 2.0f);

			{

				Plane n1 = Geometry::SHAPES::createPlane(p0, normalize(planes[hit_i].u));
				Plane n2 = Geometry::SHAPES::createPlane(p0, normalize(planes[hit_i].v));

				optix::float3 u1 = Geometry::VECTOR_SPACE::projectPointToPlane(n1, isec_p[hit_i]);
				optix::float3 u2 = Geometry::VECTOR_SPACE::projectPointToPlane(n2, isec_p[hit_i]);

				float d1 = dot(normalize(u1 - p0), r_normals[hit_i]);
				float d2 = dot(normalize(u2 - p0), r_normals[hit_i]);
				d1 = acosf(d1);
				d2 = acosf(d2);

				int sign1 = dot(planes[hit_i].v, u1 - p0) < 0.0f ? -1 : 1;
				int sign2 = dot(planes[hit_i].u, u2 - p0) < 0.0f ? -1 : 1;

				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);

				glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.65f, 0.50f, 0.50f);
				draw::Arc_arbitr_plane(p0, 20.f, 0.0f, d1 * sign1, normalize(r_normals[hit_i]), normalize(planes[hit_i].v), 30, 1, GL_POLYGON);
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.50f, 0.65f, 0.50f);
				draw::Arc_arbitr_plane(p0, 20.f, 0.0f, d2 * sign2, normalize(r_normals[hit_i]), normalize(planes[hit_i].u), 30, 1, GL_POLYGON);

				glDisable(GL_DEPTH_TEST);
				glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
				draw::Arc_arbitr_plane(p0, 20.01f, 0.0f, d1 * sign1, normalize(r_normals[hit_i]), normalize(planes[hit_i].v), 30, 1, GL_LINE_LOOP);
				draw::Arc_arbitr_plane(p0, 20.01f, 0.0f, d2 * sign2, normalize(r_normals[hit_i]), normalize(planes[hit_i].u), 30, 1, GL_LINE_LOOP);

				// draw degreeds //
				glm::vec3 _p1 = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p0 + normalize(u1 - p0) * 20.0f + normalize(planes[hit_i].v) * 1.0f - normalize(planes[hit_i].u) * 2.0f);
				glm::vec3 _p2 = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p0 + normalize(u2 - p0) * 20.0f + normalize(planes[hit_i].u) * 1.0f - normalize(planes[hit_i].v) * 2.0f);
				std::string deg_d1 = std::to_string(Geometry::GENERAL::toDegrees(d1 * sign1)); if (deg_d1.size() > 6 + (1 - sign1) / 2) deg_d1.erase(deg_d1.begin() + 5 + (1 - sign1) / 2, deg_d1.end());
				std::string deg_d2 = std::to_string(Geometry::GENERAL::toDegrees(d2 * sign2)); if (deg_d2.size() > 6 + (1 - sign2) / 2) deg_d2.erase(deg_d2.begin() + 5 + (1 - sign2) / 2, deg_d2.end());

				
				draw::Text(_p1, glm::vec3(0.0f), FONT3, (char *)deg_d1.c_str());
				draw::Text(_p2, glm::vec3(0.0f), FONT3, (char *)deg_d2.c_str());


				// draw Face Vector //
				{
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
					draw::Line3D(isec_p[hit_i] + u * 10000.0f, p0, 1.0f);
					draw::Line3D(isec_p[hit_i] + u * 20.0f, p0, 3.0f);
					draw::Arrow(p0, isec_p[hit_i] + u * 20.0f, 1.f * inv_scale_factor, false);
				}

				// draw degree symbol //
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
				int off1 = deg_d1.size();
				int off2 = deg_d2.size();
				std::string o_1, o_2;
				for (int i = 0; i < off1; i++)o_1 += "  ";
				for (int i = 0; i < off2; i++)o_2 += "  ";
				o_1 += "o";
				o_2 += "o";
				draw::Text(_p1 + GLM_GLOBAL_AXIS_Y * 1.f - GLM_GLOBAL_AXIS_X * 0.3f, glm::vec3(0.0f), FONT5, (char*)o_1.c_str());
				draw::Text(_p2 + GLM_GLOBAL_AXIS_Y * 1.f - GLM_GLOBAL_AXIS_X * 0.3f, glm::vec3(0.0f), FONT5, (char*)o_2.c_str());


			}
			glEnable(GL_DEPTH_TEST);
		}

	}






#endif


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void OpenGLRenderer::Render_Utility_Attach_Face_vector4(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{

	SceneObject* obj = utility_to_Render.obj_pack->raw_object;
	float scale_factor = optix::length(obj->getScaleAttributes());
	optix::Matrix4x4 transformation = MATRIX_TYPE_TRANSFORM::TO_OPTIX_MATRIX(utility_to_Render.obj_pack->translation_matrix_scaled);
	optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
	//transformation = def_transformation;

	// get centroid 
	optix::float3 centroid = obj->getCentroid();
	if (obj->getType() != GROUP_PARENT_DUMMY)
		centroid = optix::make_float3(def_transformation * optix::make_float4(centroid, 1.0f));

	// get all 8 bbox points 
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
		to_plane.origin    = planes[i].centroid;
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


	float inv_scale_factor = utility_to_Render.obj_pack->cam_inv_scale_factor;
	
	if (inv_scale_factor < 8.0f)
		inv_scale_factor = 8.0f;

	float dist = 2.5f * inv_scale_factor; 
	float size = 1.4f * inv_scale_factor;
	dist = 0.0f;
	size = 0.0f;
	//const float offset = 40.0f + dist;
	const float offset = (d_max + d_min) * 0.5f;
	

	optix::float2 scale = optix::make_float2( offset * 0.1f );
	optix::float3 * M_WORKING_AXIS = utility_to_Render.obj_pack->base_axis;
	
#define draw_planes
#ifdef draw_planes

	
	glDisable(GL_CULL_FACE);

	glm::vec3 diffuseColor;
	glm::mat4 model_matrix = glm::mat4(1.0f);
	float alpha = 0.8f;

	if (obj->getType() != GROUP_PARENT_DUMMY)
	{
		model_matrix = utility_to_Render.obj_pack->transformation_matrix;
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.7f, 0.7f, 0.7f);
		Render_GeometryNode(obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_matrix, m_utility_rendering_program, true);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
	}
		
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.f, 1.f, 1.f);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	draw::BBox(bbox_min, bbox_max, ZERO_3f,  1.0f);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	Ray ray = Geometry::SHAPES::createMouseRay();
	optix::float3 isec_p[6];
	
	int hit_index = interface_state->isec_p->hit_index;
	for (int i = 0; i < 6; i++)
	{

#define SCALE_DIST_METHOD_A
#ifdef SCALE_DIST_METHOD_A

		optix::float3 p0 = planes[i].centroid + r_normals[i] * offset;
		optix::float3 du = normalize(planes[i].u) * scale.x;
		optix::float3 dv = normalize(planes[i].v) * scale.y;
		optix::float3 u = planes[i].u * 0.5f + du;
		optix::float3 v = planes[i].v * 0.5f + dv;

#endif



		Plane p1 = Geometry::SHAPES::createPlane(p0, u, v);
		Plane p2 = Geometry::SHAPES::createPlane(p0, -u, -v);
		Plane p3 = Geometry::SHAPES::createPlane(p0, u, -v);
		Plane p4 = Geometry::SHAPES::createPlane(p0, -u, v);


		Plane render_plane = Geometry::SHAPES::createPlane_Centered_(p0,
																	 u,
																	 v);

		diffuseColor = hit_index == i ? HIGHLIGHT_COLOR * 0.8f : glm::vec3(0.7f);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.5f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		draw::Plane(render_plane, ZERO_3f);

		if (hit_index == i) glDisable(GL_DEPTH_TEST);

		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
		draw::Plane(render_plane, ZERO_3f, 1.0f, 2.0f, GL_LINE_LOOP);

		if (hit_index == i) glEnable(GL_DEPTH_TEST);

		if (hit_index == i)
		{
			glDisable(GL_DEPTH_TEST);

			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			optix::float3 du = normalize(planes[i].u);
			optix::float3 dv = normalize(planes[i].v);

			// Render u,v Lines over plane //
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			draw::stripped_Line3D(p0, p0 + u, 1.0f, false);
			draw::stripped_Line3D(p0, p0 + v, 1.0f, false);
			draw::stripped_Line3D(p0, p0 - u, 1.0f, false);
			draw::stripped_Line3D(p0, p0 - v, 1.0f, false);


			// Render cross on Normal_Point on hovered Plane //
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.8f, 0.8f, 0.8f);
			draw::Line3D(p0, p0 + du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 + dv * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - dv * 0.5f * inv_scale_factor, 3.0f);

			glEnable(GL_DEPTH_TEST);
		}


	}

	
	
		
	if (hit_index != -1)
	{
		int hit_i     = hit_index;
		isec_p[hit_i] = interface_state->isec_p->p;

		glDisable(GL_DEPTH_TEST);
		optix::float3 p0 = (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f;
		optix::float3 u  = normalize(isec_p[hit_i] - p0);

		Plane n1 = Geometry::SHAPES::createPlane(p0, normalize(planes[hit_i].u));
		Plane n2 = Geometry::SHAPES::createPlane(p0, normalize(planes[hit_i].v));

		optix::float3 u1 = Geometry::VECTOR_SPACE::projectPointToPlane(n1, isec_p[hit_i]);
		optix::float3 u2 = Geometry::VECTOR_SPACE::projectPointToPlane(n2, isec_p[hit_i]);

		float d1 = dot(normalize(u1 - p0), r_normals[hit_i]);
		float d2 = dot(normalize(u2 - p0), r_normals[hit_i]);
		d1 = acosf(d1);
		d2 = acosf(d2);

		int sign1 = dot(planes[hit_i].v, u1 - p0) < 0.0f ? -1 : 1;
		int sign2 = dot(planes[hit_i].u, u2 - p0) < 0.0f ? -1 : 1;

		//glClear(GL_DEPTH_BUFFER_BIT);

#ifdef draw_solid_angles_uv
		glEnable(GL_DEPTH_TEST);
		//float r = 20.0f;
		float r = length(planes[hit_i].centroid + r_normals[hit_i] * offset - p0) * 0.35f;

		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.65f, 0.50f, 0.50f);
		draw::Arc_arbitr_plane(p0, r, 0.0f, d1 * sign1, normalize(r_normals[hit_i]), normalize(planes[hit_i].v), 30, 1, GL_POLYGON);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.50f, 0.65f, 0.50f);
		draw::Arc_arbitr_plane(p0, r, 0.0f, d2 * sign2, normalize(r_normals[hit_i]), normalize(planes[hit_i].u), 30, 1, GL_POLYGON);

		glDisable(GL_DEPTH_TEST);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.05f, 0.05f, 0.05f);
		draw::Arc_arbitr_plane(p0, r + 0.01f, 0.0f, d1 * sign1, normalize(r_normals[hit_i]), normalize(planes[hit_i].v), 30, 1, GL_LINE_LOOP);
		draw::Arc_arbitr_plane(p0, r + 0.01f, 0.0f, d2 * sign2, normalize(r_normals[hit_i]), normalize(planes[hit_i].u), 30, 1, GL_LINE_LOOP);

		// draw degrees //
		glm::vec3 _p1 = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p0 + normalize(u1 - p0) * 20.0f + normalize(planes[hit_i].v) * 1.0f - normalize(planes[hit_i].u) * 2.0f);
		glm::vec3 _p2 = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p0 + normalize(u2 - p0) * 20.0f + normalize(planes[hit_i].u) * 1.0f - normalize(planes[hit_i].v) * 2.0f);
		std::string deg_d1 = std::to_string(Geometry::GENERAL::toDegrees(d1 * sign1)); if (deg_d1.size() > 6 + (1 - sign1) / 2) deg_d1.erase(deg_d1.begin() + 5 + (1 - sign1) / 2, deg_d1.end());
		std::string deg_d2 = std::to_string(Geometry::GENERAL::toDegrees(d2 * sign2)); if (deg_d2.size() > 6 + (1 - sign2) / 2) deg_d2.erase(deg_d2.begin() + 5 + (1 - sign2) / 2, deg_d2.end());
		draw::Text(_p1, glm::vec3(0.0f), FONT3, (char *)deg_d1.c_str());
		draw::Text(_p2, glm::vec3(0.0f), FONT3, (char *)deg_d2.c_str());
		glDisable(GL_DEPTH_TEST);
#endif

		// draw axis //
		{
			glDisable(GL_DEPTH_TEST);
			optix::float3 p0 = hit_i != -1 ? (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f : centroid;
			optix::float3 p1;
			float line_width = 1.0f;
			
			for (int i = 0; i < 3; i++)
			{
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR[i].r, DEF_AXIS_COLOR[i].g, DEF_AXIS_COLOR[i].b);

				if (hit_i == -1)
				{
					p1 = p0 + M_WORKING_AXIS[i] * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;
					line_width = 1.0f;
				}
				else
				{
					if (i == 0)
						p1 = p0 + normalize(planes[hit_i].u) * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;
					if (i == 1)
						p1 = p0 + normalize(planes[hit_i].v) * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;
					if (i == 2)
						p1 = p0 + r_normals[hit_i] * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;

					line_width = 2.0f;
				}
				draw::Line3D(p0, p1, line_width);

			}
			glEnable(GL_DEPTH_TEST);
		}




		// draw Face Vector //
		{
			glDisable(GL_DEPTH_TEST);
			PinholeCamera cam_instance = ViewportManager::GetInstance().getViewportCamera(vp.id);
			float obj_dist = cam_instance.getDistanceFrom(p0);
			float inv_fact = cam_instance.getInvScaleFactor();
			float scale_factor_2 = LINKING_SCALE_FACTOR * obj_dist * inv_fact;

			float _FAR_ = 99000;
			optix::float3 p1  = isec_p[hit_i] + u * scale_factor_2 * 3.0f;
			optix::float3 _p1 = isec_p[hit_i] + u * _FAR_;
			
			//glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
			//draw::Line3D(p0, isec_p[hit_i] + u * 10000.0f, 1.0f);
			//draw::Line3D(p0, isec_p[hit_i] + u * scale_factor_2 * 3.0f, 3.0f);
			//draw::Arrow(p0, isec_p[hit_i] + u * scale_factor_2 * 3.0f, scale_factor_2 * 1.3f, vp, false);
			interface_state->isec_p->p1 = p0;

			Shape line;
			line.shape_id = LINE;
			line.a = p0; line.b = _p1;
			line.thickness = 1.0f;
			line.color = glm::vec3(1.0f);
			line.draw_arrow = 0;
			line.test_depth = 0;
			m_Shapes2D_per_Viewport[vp.id].push_back(line);

			line.a = p0;
			line.b = p1;
			line.thickness = 3.0f;
			m_Shapes2D_per_Viewport[vp.id].push_back(line);

			line.arrow_scale_factor = scale_factor_2 * 1.3f;
			line.thickness  = 2.0f;
			line.draw_arrow = 1;
			line.only_arrow = 1;
			m_Shapes2D_per_Viewport[vp.id].push_back(line);
		}


		// draw degree symbol //
#ifdef draw_solid_angles_uv
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.05f, 0.05f, 0.05f);
		int off1 = deg_d1.size();
		int off2 = deg_d2.size();
		std::string o_1, o_2;
		for (int i = 0; i < off1; i++)o_1 += "  ";
		for (int i = 0; i < off2; i++)o_2 += "  ";
		o_1 += "o";
		o_2 += "o";
		draw::Text(_p1 + GLM_GLOBAL_AXIS_Y * 1.f - GLM_GLOBAL_AXIS_X * 0.3f, glm::vec3(0.0f), FONT5, (char*)o_1.c_str());
		draw::Text(_p2 + GLM_GLOBAL_AXIS_Y * 1.f - GLM_GLOBAL_AXIS_X * 0.3f, glm::vec3(0.0f), FONT5, (char*)o_2.c_str());
#endif

		{
			glDisable(GL_DEPTH_TEST);
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.3f, 0.05f, 0.88f);
			optix::float3 du = normalize(planes[hit_i].u);
			optix::float3 dv = normalize(planes[hit_i].v);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] + du * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] + dv * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] - du * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] - dv * 0.3f * inv_scale_factor, 2.0f);
		}


		glEnable(GL_DEPTH_TEST);
	}

	
	// Render object's FaceVectors //
	bool is_vp_draw_fv = vp.render_settings.draw_face_vectors_all || vp.render_settings.draw_face_vectors_only_selected;
	if (!is_vp_draw_fv)
		Render_Object_FaceVectors(utility_to_Render, vp);
	


#endif


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}
void OpenGLRenderer::Render_Utility_Attach_Face_Vector_Parallel(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{

	SceneObject* obj = utility_to_Render.obj_pack->raw_object;
	float scale_factor = optix::length(obj->getScaleAttributes());
	optix::Matrix4x4 transformation = MATRIX_TYPE_TRANSFORM::TO_OPTIX_MATRIX(utility_to_Render.obj_pack->translation_matrix_scaled);
	optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
	//transformation = def_transformation;

	// get centroid 
	optix::float3 centroid = obj->getCentroid();
	if (obj->getType() != GROUP_PARENT_DUMMY)
		centroid = optix::make_float3(def_transformation * optix::make_float4(centroid, 1.0f));

	// get all 8 bbox points 
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


	float inv_scale_factor = utility_to_Render.obj_pack->cam_inv_scale_factor;

	if (inv_scale_factor < 8.0f)
		inv_scale_factor = 8.0f;

	float dist = 2.5f * inv_scale_factor;
	float size = 1.4f * inv_scale_factor;
	dist = 0.0f;
	size = 0.0f;
	//const float offset = 40.0f + dist;
	const float offset = (d_max + d_min) * 0.5f;


	optix::float2 scale = optix::make_float2(offset * 0.1f);
	optix::float3 * M_WORKING_AXIS = utility_to_Render.obj_pack->base_axis;

#define draw_planes
#ifdef draw_planes


	glDisable(GL_CULL_FACE);

	glm::vec3 diffuseColor;
	glm::mat4 model_matrix = glm::mat4(1.0f);
	float alpha = 0.8f;

	if (obj->getType() != GROUP_PARENT_DUMMY)
	{
		model_matrix = utility_to_Render.obj_pack->transformation_matrix;
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.7f, 0.7f, 0.7f);
		Render_GeometryNode(obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_matrix, m_utility_rendering_program, true);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
	}

	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.f, 1.f, 1.f);
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
	draw::BBox(bbox_min, bbox_max, ZERO_3f, 1.0f);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

	Ray ray = Geometry::SHAPES::createMouseRay();
	optix::float3 isec_p[6];

	int hit_index = interface_state->isec_p->hit_index;
	for (int i = 0; i < 6; i++)
	{


#define SCALE_DIST_METHOD_A
#ifdef SCALE_DIST_METHOD_A

		optix::float3 p0 = planes[i].centroid + r_normals[i] * offset;
		optix::float3 du = normalize(planes[i].u) * scale.x;
		optix::float3 dv = normalize(planes[i].v) * scale.y;
		optix::float3 u = planes[i].u * 0.5f + du;
		optix::float3 v = planes[i].v * 0.5f + dv;

#endif

	

		Plane p1 = Geometry::SHAPES::createPlane(p0, u, v);
		Plane p2 = Geometry::SHAPES::createPlane(p0, -u, -v);
		Plane p3 = Geometry::SHAPES::createPlane(p0, u, -v);
		Plane p4 = Geometry::SHAPES::createPlane(p0, -u, v);


		Plane render_plane = Geometry::SHAPES::createPlane_Centered_(p0,
																	 u,
																	 v);

		diffuseColor = hit_index == i ? HIGHLIGHT_COLOR * 0.8f : glm::vec3(0.7f);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.5f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		draw::Plane(render_plane, ZERO_3f);

		if (hit_index == i) glDisable(GL_DEPTH_TEST);
		
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
		draw::Plane(render_plane, ZERO_3f, 1.0f, 2.0f, GL_LINE_LOOP);
		
		if (hit_index == i) glEnable(GL_DEPTH_TEST);

		if (hit_index == i)
		{
			glDisable(GL_DEPTH_TEST);

			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			optix::float3 du = normalize(planes[i].u);
			optix::float3 dv = normalize(planes[i].v);

			// Render u,v Lines over plane //
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			draw::stripped_Line3D(p0, p0 + u, 1.0f, false);
			draw::stripped_Line3D(p0, p0 + v, 1.0f, false);
			draw::stripped_Line3D(p0, p0 - u, 1.0f, false);
			draw::stripped_Line3D(p0, p0 - v, 1.0f, false);


			// Render cross on Normal_Point on hovered Plane //
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.8f, 0.8f, 0.8f);
			draw::Line3D(p0, p0 + du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 + dv * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - du * 0.5f * inv_scale_factor, 3.0f);
			draw::Line3D(p0, p0 - dv * 0.5f * inv_scale_factor, 3.0f);

			glEnable(GL_DEPTH_TEST);
		}

	}


	if (hit_index != -1)
	{
		int hit_i = hit_index;
		isec_p[hit_i] = interface_state->isec_p->p;

		glDisable(GL_DEPTH_TEST);
		optix::float3 p0 = (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f;
		optix::float3 u = normalize(isec_p[hit_i] - p0);


		// draw axis //
		{
			optix::float3 p0 = hit_i != -1 ? (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f : centroid;
			optix::float3 p1;
			float line_width = 1.0f;

			glDisable(GL_DEPTH_TEST);
			for (int i = 0; i < 3; i++)
			{
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR[i].r, DEF_AXIS_COLOR[i].g, DEF_AXIS_COLOR[i].b);

				if (hit_i == -1)
				{
					p1 = p0 + M_WORKING_AXIS[i] * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;
					line_width = 1.0f;
				}
				else
				{
					if (i == 0)
						p1 = p0 + normalize(planes[hit_i].u) * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;
					if (i == 1)
						p1 = p0 + normalize(planes[hit_i].v) * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;
					if (i == 2)
						p1 = p0 + r_normals[hit_i] * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;

					line_width = 2.0f;
				}
				draw::Line3D(p0, p1, line_width);

			}
			glEnable(GL_DEPTH_TEST);
		}

		// draw Face Vector //
		{
			glDisable(GL_DEPTH_TEST);

			Ray bbox_ray;
			bbox_ray.origin = isec_p[hit_i];
			bbox_ray.direction = -r_normals[hit_i];

			optix::float3 _p0;
			Geometry::RAY::Intersect_Plane(bbox_ray,
										   Geometry::SHAPES::createPlane(planes[hit_i].p0, r_normals[hit_i]),
										   _p0);
			PinholeCamera cam_instance = ViewportManager::GetInstance().getViewportCamera(vp.id);
			optix::float3 _p1    = isec_p[hit_i];
			float scale_factor   = 3.0f * cam_instance.getInvScaleFactor();
			float obj_dist       = cam_instance.getDistanceFrom(_p0);
			float scale_factor_2 = LINKING_SCALE_FACTOR * obj_dist * cam_instance.getInvScaleFactor();
			interface_state->isec_p->p1 = _p0;

			float _FAR_ = 99000;
			Shape line;
			line.shape_id = LINE;
			line.a = _p0; line.b = _p0 + r_normals[hit_i] * _FAR_;
			line.thickness = 1.0f;
			line.color = glm::vec3(1.0f);
			line.draw_arrow = 0;
			line.test_depth = 0;
			m_Shapes2D_per_Viewport[vp.id].push_back(line);

			line.a = _p0;
			line.b = _p1 + r_normals[hit_i] * scale_factor_2 * 3.0f;
			line.thickness = 3.0f;
			m_Shapes2D_per_Viewport[vp.id].push_back(line);

			line.arrow_scale_factor = scale_factor_2 * 1.3f;
			line.thickness = 2.0f;
			line.draw_arrow = 1;
			line.only_arrow = 1;
			m_Shapes2D_per_Viewport[vp.id].push_back(line);
			//

			glEnable(GL_DEPTH_TEST);
		}


		//
		{
			glDisable(GL_DEPTH_TEST);
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.3f, 0.05f, 0.88f);
			optix::float3 du = normalize(planes[hit_i].u);
			optix::float3 dv = normalize(planes[hit_i].v);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] + du * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] + dv * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] - du * 0.3f * inv_scale_factor, 2.0f);
			draw::Line3D(isec_p[hit_i], isec_p[hit_i] - dv * 0.3f * inv_scale_factor, 2.0f);
			glEnable(GL_DEPTH_TEST);
		}


	}


	// Render object's FaceVectors //
	bool is_vp_draw_fv = vp.render_settings.draw_face_vectors_all || vp.render_settings.draw_face_vectors_only_selected;
	if (!is_vp_draw_fv)
		Render_Object_FaceVectors(utility_to_Render, vp);


#endif

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}
void OpenGLRenderer::Render_Utility_Attach_Face_vector_Normal_Aligned(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{

	SceneObject* obj = utility_to_Render.obj_pack->raw_object;
	if (obj->getType() == GROUP_PARENT_DUMMY)
	{
		SceneObject* obj = utility_to_Render.obj_pack->raw_object;
		float scale_factor = optix::length(obj->getScaleAttributes());
		optix::Matrix4x4 transformation = MATRIX_TYPE_TRANSFORM::TO_OPTIX_MATRIX(utility_to_Render.obj_pack->translation_matrix_scaled);
		optix::Matrix4x4 def_transformation = obj->getTransformationMatrix();
		
		optix::float3 centroid = obj->getCentroid();
		if (obj->getType() != GROUP_PARENT_DUMMY)
			centroid = optix::make_float3(def_transformation * optix::make_float4(centroid, 1.0f));
		// get all 8 bbox points 
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
		//float const_dist = 100.0f;
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

		float inv_scale_factor = utility_to_Render.obj_pack->cam_inv_scale_factor;
		if (inv_scale_factor < 8.0f)
			inv_scale_factor = 8.0f;

		float dist = 2.5f * inv_scale_factor;
		float size = 1.4f * inv_scale_factor;
		dist = 0.0f;
		size = 0.0f;
		const float offset = 0.1f;//(d_max + d_min) * 0.5f;


		optix::float2 scale = optix::make_float2(offset * 0.1f);
		optix::float3 * M_WORKING_AXIS = utility_to_Render.obj_pack->base_axis;

#define draw_planes
#ifdef draw_planes

		glDisable(GL_CULL_FACE);

		glm::vec3 diffuseColor;
		glm::mat4 model_matrix = glm::mat4(1.0f);
		float alpha = 0.8f;

		if (obj->getType() != GROUP_PARENT_DUMMY)
		{
			model_matrix = utility_to_Render.obj_pack->transformation_matrix;
			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.0f);
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.7f, 0.7f, 0.7f);
			Render_GeometryNode(obj, Mesh_Pool::GetInstance().request_Mesh_for_OpenGL(obj), model_matrix, m_utility_rendering_program, true);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
		}

		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.f, 1.f, 1.f);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		draw::BBox(bbox_min, bbox_max, ZERO_3f, 1.0f);
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

		Ray ray = Geometry::SHAPES::createMouseRay();
		optix::float3 isec_p[6];

		int hit_index = interface_state->isec_p->hit_index;
		for (int i = 0; i < 6; i++)
		{

#define SCALE_DIST_METHOD_A
#ifdef SCALE_DIST_METHOD_A

			optix::float3 p0 = planes[i].centroid + r_normals[i] * offset;
			optix::float3 du = normalize(planes[i].u) * scale.x;
			optix::float3 dv = normalize(planes[i].v) * scale.y;
			optix::float3 u = planes[i].u * 0.5f + du;
			optix::float3 v = planes[i].v * 0.5f + dv;

#endif
			Plane p1 = Geometry::SHAPES::createPlane(p0, u, v);
			Plane p2 = Geometry::SHAPES::createPlane(p0, -u, -v);
			Plane p3 = Geometry::SHAPES::createPlane(p0, u, -v);
			Plane p4 = Geometry::SHAPES::createPlane(p0, -u, v);
			Plane render_plane = Geometry::SHAPES::createPlane_Centered_(p0,
																		 u,
																		 v);
			diffuseColor = hit_index == i ? HIGHLIGHT_COLOR * 0.8f : glm::vec3(0.7f);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.5f);
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			draw::Plane(render_plane, ZERO_3f);

			if (hit_index == i)
				glDisable(GL_DEPTH_TEST);
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
			draw::Plane(render_plane, ZERO_3f, 1.0f, 2.0f, GL_LINE_LOOP);
			glEnable(GL_DEPTH_TEST);

			if (hit_index == i)
			{
				glDisable(GL_DEPTH_TEST);


				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
				optix::float3 du = normalize(planes[i].u);
				optix::float3 dv = normalize(planes[i].v);
				//optix::float3 p0 = planes[i].p_normal;

				// Render u,v Lines over plane //
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
				draw::stripped_Line3D(p0, p0 + u, 1.0f, false);
				draw::stripped_Line3D(p0, p0 + v, 1.0f, false);
				draw::stripped_Line3D(p0, p0 - u, 1.0f, false);
				draw::stripped_Line3D(p0, p0 - v, 1.0f, false);


				// Render cross on Normal_Point on hovered Plane //
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.8f, 0.8f, 0.8f);
				draw::Line3D(p0, p0 + du * 0.5f * inv_scale_factor, 3.0f);
				draw::Line3D(p0, p0 + dv * 0.5f * inv_scale_factor, 3.0f);
				draw::Line3D(p0, p0 - du * 0.5f * inv_scale_factor, 3.0f);
				draw::Line3D(p0, p0 - dv * 0.5f * inv_scale_factor, 3.0f);

				glEnable(GL_DEPTH_TEST);
			}

		}

		{
			if (hit_index != -1)
			{
				int hit_i = hit_index;
				isec_p[hit_i] = interface_state->isec_p->p;

				glDisable(GL_DEPTH_TEST);
				optix::float3 p0 = (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f;
				optix::float3 u = normalize(isec_p[hit_i] - p0);

				{
					glClear(GL_DEPTH_BUFFER_BIT);
					glEnable(GL_DEPTH_TEST);
					// draw axis //
					{
						optix::float3 p0 = hit_i != -1 ? (planes[hit_i].p0 + planes[hit_i].p2) * 0.5f : centroid;
						optix::float3 p1;
						float line_width = 1.0f;
						glDisable(GL_DEPTH_TEST);
						for (int i = 0; i < 3; i++)
						{
							glUniform3f(m_utility_rendering_program["uniform_diffuse"], DEF_AXIS_COLOR[i].r, DEF_AXIS_COLOR[i].g, DEF_AXIS_COLOR[i].b);

							if (hit_i == -1)
							{
								p1 = p0 + M_WORKING_AXIS[i] * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;
								line_width = 1.0f;
							}
							else
							{
								if (i == 0)
									p1 = p0 + normalize(planes[hit_i].u) * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;
								if (i == 1)
									p1 = p0 + normalize(planes[hit_i].v) * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;
								if (i == 2)
									p1 = p0 + r_normals[hit_i] * 3.0f * utility_to_Render.obj_pack->cam_inv_scale_factor;

								line_width = 2.0f;
							}
							draw::Line3D(p0, p1, line_width);

						}
						glEnable(GL_DEPTH_TEST);
					}

					// draw Face Vector //
					{
						Ray bbox_ray;
						bbox_ray.origin = isec_p[hit_i];
						bbox_ray.direction = -r_normals[hit_i];

						optix::float3 _p0;
						Geometry::RAY::Intersect_Plane(bbox_ray,
													   Geometry::SHAPES::createPlane(planes[hit_i].p0, r_normals[hit_i]),
													   _p0);
						optix::float3 _p1 = isec_p[hit_i];

						PinholeCamera cam_instance = ViewportManager::GetInstance().getViewportCamera(vp.id);
						float scale_factor = 3.0f * inv_scale_factor;
						float obj_dist = cam_instance.getDistanceFrom(_p0);
						float scale_factor_2 = LINKING_SCALE_FACTOR * obj_dist * cam_instance.getInvScaleFactor();

						glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
						draw::Line3D(_p0, _p0 + r_normals[hit_i] * FAR, 1.0f);
						draw::Line3D(_p1 + r_normals[hit_i] * scale_factor, _p0, 3.0f);
						draw::Arrow(_p0, _p1 + r_normals[hit_i] * scale_factor, 1.3f * scale_factor_2 , vp, false);
						interface_state->isec_p->p1 = _p0;

					}

				}

				{
					glDisable(GL_DEPTH_TEST);
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], 0.3f, 0.05f, 0.88f);
					optix::float3 du = normalize(planes[hit_i].u);
					optix::float3 dv = normalize(planes[hit_i].v);
					draw::Line3D(isec_p[hit_i], isec_p[hit_i] + du * 0.3f * inv_scale_factor, 2.0f);
					draw::Line3D(isec_p[hit_i], isec_p[hit_i] + dv * 0.3f * inv_scale_factor, 2.0f);
					draw::Line3D(isec_p[hit_i], isec_p[hit_i] - du * 0.3f * inv_scale_factor, 2.0f);
					draw::Line3D(isec_p[hit_i], isec_p[hit_i] - dv * 0.3f * inv_scale_factor, 2.0f);
				}
				glEnable(GL_DEPTH_TEST);
			}

		}

#endif


		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

	}
	else
	{

		// 
		PinholeCamera cam_instance = ViewportManager::GetInstance().getViewportCamera(vp.id);

		optix::float3 * data = Mediator::request_MouseHit_Buffer_Data();
		optix::float3 pos = data[0];
		optix::float3 normal = data[1];
		float scale = utility_to_Render.obj_pack->cam_inv_scale_factor;
		optix::float3 p0, p1;
		p0 = pos;
		p1 = pos + normal * 4.5f * scale;

		float obj_dist = cam_instance.getDistanceFrom(p0);
		float scale_factor_2 = LINKING_SCALE_FACTOR * obj_dist * cam_instance.getInvScaleFactor();

		interface_state->isec_p->p = p1;
		interface_state->isec_p->p1 = p0;

		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
		draw::Line3D(p0, p0 + normal * FAR, 1.0f);
		draw::Line3D(p0, p1, 2.0f);
		draw::Arrow( p0, p1, scale_factor_2 * 1.3f, vp, false);
		delete data;

		/*
		
		PinholeCamera cam_instance = ViewportManager::GetInstance().getViewportCamera(vp.id);
					float obj_dist = cam_instance.getDistanceFrom(p0);
					float inv_fact = cam_instance.getInvScaleFactor();
					float scale_factor_2 = LINKING_SCALE_FACTOR * obj_dist * inv_fact;

					float scale_factor = 3.0f * cam_instance.getInvScaleFactor();
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
					draw::Line3D(p0, isec_p[hit_i] + u * 10000.0f, 1.0f);
					draw::Line3D(p0, isec_p[hit_i] + u * scale_factor, 3.0f);
					draw::Arrow( p0, isec_p[hit_i] + u * scale_factor, scale_factor_2 * 1.3f, vp, false);
					interface_state->isec_p->p1 = p0;

		*/
	}

	// Render object's FaceVectors //
	bool is_vp_draw_fv = vp.render_settings.draw_face_vectors_all || vp.render_settings.draw_face_vectors_only_selected;
	if( !is_vp_draw_fv ) 
		Render_Object_FaceVectors( utility_to_Render, vp);
}
void OpenGLRenderer::Render_Utility_Attach_Sampler(Utility_RenderList_Object utility_to_Render)
{

}
void OpenGLRenderer::Render_Utility_Create_Light(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{
	PinholeCamera cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	optix::float3 light_p0 = interface_state->m_light_params.position;//interface_state->isec_p->p;
	float obj_dist         = cam.getDistanceFrom(light_p0);


	bool is_BBOX = interface_state->m_sa == SA_BBOX;
	optix::float3 plane_p0 = interface_state->isec_p->p1;
	optix::float3 plane_n  = interface_state->isec_p->normal;
	//plane_n *= is_BBOX ? -1 : 1;
	float size_factor = 1000.0f;
	Light_Type l_type = interface_state->m_light_params.type;

	
	optix::float3 cam_w = normalize(cam.getW());
	int sign = -1;
	float scale_factor = utility_to_Render.obj_pack->cam_inv_scale_factor * obj_dist * 1.0f;

	bool is_hit    = interface_state->isec_p->hit_index != -1;
	bool flip_side = interface_state->flip_side;
	if (!is_hit && !is_BBOX) 
		return;
	else if (!is_hit && is_BBOX)
	{
		SceneObject* obj = utility_to_Render.obj_pack->raw_object;
		if (obj != 0)
		{
			Render_Utility_Attach_Planes(utility_to_Render, vp);
		}
		return;
	}

	SAMPLER_ALIGNEMENT sa = interface_state->m_sa;
	optix::float3 uu = interface_state->m_sampler_params.basis[0];
	optix::float3 vv = interface_state->m_sampler_params.basis[2];
	optix::float3 ww = interface_state->m_sampler_params.basis[1];

	// draw plane //
	glm::vec3 color = glm::vec3(0.1f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(m_utility_rendering_program["unfirom_alpha"], 1.0f);




#define draw_grid_layout
#ifdef draw_grid_layout
	
	//
	glUniform1i(m_utility_rendering_program["uniform_test_depth"], 1);
	glEnable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ViewportManager::GetInstance().getFbo_DepthTexture_2(vp.id));
	//


	if      (sa == SA_GLOBAL)
	{
		glm::vec3 l_color = glm::vec3(0.35f);
		color = l_color;
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
		glUniform1f(m_utility_rendering_program["unfirom_alpha"], 1.0f);

		float grd_dist = 350.0f;
		glUniform1i(m_utility_rendering_program["uniform_rendering_gradient_grid"], 1);
		glUniform3f(m_utility_rendering_program["uniform_gradient_grid_o"], light_p0.x, light_p0.y, light_p0.z);
		glUniform1f(m_utility_rendering_program["uniform_gradient_grid_dist"], grd_dist);

		int NUM_LINES = 20;
		float len     = 500;
		float FROM    = -len;
		float TO      = len;
		float offset  = 2 * len / NUM_LINES;

		glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(light_p0.x, light_p0.y, light_p0.z));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

		optix::float3 u = GLOBAL_AXIS[0];
		optix::float3 o = ZERO_3f;
		for (float z = FROM; z <= TO; z += offset)
		{
			float x0 = z==0? o.x - u.x * grd_dist : o.x - u.x * len;
			float y0 = o.y;
			float z0 = z;

			float x1 = z == 0 ? o.x + u.x * grd_dist : o.x + u.x * len;
			float y1 = o.y;
			float z1 = z;

			if (z == 0)
			{
				color = glm::vec3(0.05f);
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
				glUniform1i(m_utility_rendering_program["uniform_rendering_gradient_grid"], 0);
			}
			optix::float3 p0 = optix::make_float3(x0, y0, z0);
			optix::float3 p1 = optix::make_float3(x1, y1, z1);
			draw::Line3D(p0, p1, 1.0f);
			if (z == 0)
			{
				color = l_color;
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
				glUniform1i(m_utility_rendering_program["uniform_rendering_gradient_grid"], 1);
			}
		}

		u = GLOBAL_AXIS[2];
		for (float x = FROM; x <= TO; x += offset)
		{
			float x0 = x;
			float y0 = o.y;
			float z0 = x ==0 ? o.z - u.z * grd_dist : o.z - u.z * len;

			float x1 = x;
			float y1 = o.y;
			float z1 = x == 0 ? o.z + u.z * grd_dist : o.z + u.z * len;

			if (x == 0)
			{
				color = glm::vec3(0.05f);
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
				glUniform1i(m_utility_rendering_program["uniform_rendering_gradient_grid"], 0);
			}
			optix::float3 p0 = optix::make_float3(x0, y0, z0);
			optix::float3 p1 = optix::make_float3(x1, y1, z1);
			draw::Line3D(p0, p1, 1.0f);
			if (x == 0)
			{
				color = l_color;
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
				glUniform1i(m_utility_rendering_program["uniform_rendering_gradient_grid"], 1);
			}
		}

		glUniform1i(m_utility_rendering_program["uniform_rendering_gradient_grid"], 0);
		
	}
	else if (sa == SA_NORMAL)
	{

	}
	else if (sa == SA_BBOX  )
	{
		SceneObject* obj = utility_to_Render.obj_pack->raw_object;
		if (obj != 0)
		{
			Render_Utility_Attach_Planes(utility_to_Render, vp);
		}
	}
	

#endif

	glm::mat4 model_matrix = glm::mat4(1.0f);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

	
	float vp_factor = LINKING_SCALE_FACTOR * vp.scale_factor * obj_dist;
	float sign_ = sa == SA_GLOBAL ? 1 : -1;
	if      (l_type == QUAD_LIGHT)
	{
		float scale_factor = 15.0f;
		color = glm::vec3(1.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
		glUniform1f(m_utility_rendering_program["unfirom_alpha"], 1.0f);
		glDisable(GL_CULL_FACE);
		draw::Plane_Centered(light_p0, uu, vv, optix::make_float2(scale_factor), 1.0f, GL_POLYGON);
		glEnable(GL_CULL_FACE);

		color = glm::vec3(0.1f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
		glUniform1f(m_utility_rendering_program["unfirom_alpha"], 1.0f);
		draw::Plane_Centered(light_p0, uu, vv, optix::make_float2(scale_factor), 2.0f, GL_LINE_LOOP);

		// draw Light normal
		glDisable(GL_DEPTH_TEST);
		glUniform1i(m_utility_rendering_program["uniform_test_depth"], 0);
		color = glm::vec3(1.0f, 1.0f, 0.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
		optix::float3 p0 = light_p0;
		optix::float3 p1 = p0 - normalize(interface_state->m_sampler_params.basis[1])*sign_ * vp_factor * 4.0f;
		draw::Line3D(p0, p1, 2.0f);
		draw::Arrow(p0, p1, vp_factor * 1.3f, 0, 2.0f);
		glUniform1i(m_utility_rendering_program["uniform_test_depth"], 1);
		glEnable(GL_DEPTH_TEST);
		//
	}
	else if (l_type == SPOTLIGHT)
	{
		float scale_factor = 15.f; // SPOTLIGHT CHANGE // - prev(25)
		float dist = scale_factor * 1.f;
		sign = (sa == SA_NORMAL || sa == SA_BBOX) ? -1 : 1;
		
		//if (is_BBOX)
		//	if (flip_side) sign *= -1;

		draw::Spot_Light(m_utility_rendering_program,
						 light_p0,
						 uu,
						 vv,
						 sign,
						 optix::make_float2(scale_factor),
						 dist,
						 glm::vec3(1.0f),
						 glm::vec3(0.1f),
						 2.0f);

		// draw Light normal
		optix::Matrix4x4 basis = optix::Matrix4x4::fromBasis
		(
			interface_state->m_sampler_params.basis[0],
			interface_state->m_sampler_params.basis[1],
			interface_state->m_sampler_params.basis[2],
			ZERO_3f
		);


		// draw Light normal
		glDisable(GL_DEPTH_TEST);
		glUniform1i(m_utility_rendering_program["uniform_test_depth"], 0);


		color = glm::vec3(1.0f, 1.0f, 0.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
		optix::float3 p0 = light_p0;
		optix::float3 p1 = p0 - normalize(interface_state->m_sampler_params.basis[1])*sign_ * vp_factor * 4.0f;
		draw::Line3D(p0, p1, 2.0f);
		draw::Arrow(p0, p1, vp_factor * 1.3f, 0, 2.0f);
		glUniform1i(m_utility_rendering_program["uniform_test_depth"], 1);
		glEnable(GL_DEPTH_TEST);
		//
	}
	else if (l_type == SPHERICAL_LIGHT)
	{
		light_p0 += ww * 15.0f;
		glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(light_p0)) * glm::scale(glm::mat4(1.0f), glm::vec3(15.0f));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

		color = glm::vec3(1.0f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		glBindVertexArray(m_sphere_node->m_vao);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (int i = 0; i < m_sphere_node->parts.size(); i++)
			glDrawArrays(GL_TRIANGLES, m_sphere_node->parts[i].start_offset, m_sphere_node->parts[i].count);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	}
	else if (l_type == POINT_LIGHT)
	{
		light_p0 += ww * 5.0f;
		glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(light_p0)) * glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(model_matrix))));

		color = glm::vec3(1.f);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		glBindVertexArray(m_sphere_node->m_vao);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		for (int i = 0; i < m_sphere_node->parts.size(); i++)
			glDrawArrays(GL_TRIANGLES, m_sphere_node->parts[i].start_offset, m_sphere_node->parts[i].count);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	}

	// - - - - - - - - - - - - - - - - - - - - - //

	//
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_utility_rendering_program["uniform_test_depth"], 0);
	//

}

void OpenGLRenderer::Render_Utility_Attach_Restriction_Focus(Utility_RenderList_Object ut, const VIEWPORT& vp)
{
	return Render_Utility_Link(ut, vp);
}
void OpenGLRenderer::Render_Utility_Attach_Restriction_Conversation(Utility_RenderList_Object ut, const VIEWPORT& vp)
{
	return Render_Utility_Link(ut, vp);
}
void OpenGLRenderer::Render_Utility_Attach_Restriction_Distance(Utility_RenderList_Object ut, const VIEWPORT& vp)
{
	return Render_Utility_Link(ut, vp);
}

//
void OpenGLRenderer::Render_Utility_Object_Align(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{

	bool is_align_window_active = InterfaceManager::GetInstance().is_Object_Align_Active();

	if (!vp.is_active && !is_align_window_active)
		return;


	optix::float3 bbox_max = utility_to_Render.obj_pack->raw_object->getGeomProperties().bbox_max;
	optix::float3 bbox_min = utility_to_Render.obj_pack->raw_object->getGeomProperties().bbox_min;
	optix::float3 subject_pos = utility_to_Render.obj_pack->raw_object->getTranslationAttributes();
	optix::float3 object_pos;
	glm::vec3 line_color = glm::vec3(1.0f);
	float size = utility_to_Render.obj_pack->cam_inv_scale_factor;

	glm::mat4 model_matrix = glm::mat4(1.0f); //glm::scale(glm::mat4(1.0f), glm::vec3(0.6* utility_to_Render.obj_pack->cam_inv_scale_factor));

	if (!is_align_window_active)
	{

		// case : single selection is active
		if (!utility_to_Render.obj_pack->raw_object->isTemporary()
			&& utility_to_Render.obj_pack->raw_object->getType() != GROUP_PARENT_DUMMY)
		{
			float t = optix::length(subject_pos - utility_to_Render.camera_instance->getCameraPos());
			Ray mouse_ray = Geometry::SHAPES::createMouseRay();
			object_pos = mouse_ray.origin + mouse_ray.direction * t;

			Io_Mouse mouse_data = InterfaceManager::GetInstance().getMouseData();
			int x = mouse_data.m0_down_x;
			int y = mouse_data.m0_down_y;
			optix::float2 xy = Utilities::getMousePosRelativeToViewport(vp.id, optix::make_float2(x, y));


			Ray to_mouse_ray = Geometry::SHAPES::createMouseRay(glm::vec2(xy.x, xy.y));
			subject_pos = to_mouse_ray.origin + to_mouse_ray.direction * t;


			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
			draw::stripped_Line3D(object_pos, subject_pos, size, false);
		}

		else if (utility_to_Render.obj_pack->raw_object->getType() == GROUP_PARENT_DUMMY
				 && !utility_to_Render.obj_pack->raw_object->isTemporary())
		{

			SceneObject* subject = utility_to_Render.obj_pack->raw_object;

			float t = optix::length(subject_pos - utility_to_Render.camera_instance->getCameraPos());
			Ray mouse_ray = Geometry::SHAPES::createMouseRay();
			object_pos = mouse_ray.origin + mouse_ray.direction * t;

			Io_Mouse mouse_data = InterfaceManager::GetInstance().getMouseData();
			int x = mouse_data.m0_down_x;
			int y = mouse_data.m0_down_y;
			optix::float2 xy = Utilities::getMousePosRelativeToViewport(vp.id, optix::make_float2(x, y));


			Ray to_mouse_ray = Geometry::SHAPES::createMouseRay(glm::vec2(xy.x, xy.y));
			subject_pos = (subject->getBBoxMin() + subject->getBBoxMax()) * 0.5f; // subject->getTranslationAttributes();


			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
			draw::stripped_Line3D(object_pos, subject_pos, size, false);

			Render_Object_BBOX(utility_to_Render.obj_pack->object_index, false);

		}

		// case : Area selection is active
		else
		{
			float t = optix::length(subject_pos - utility_to_Render.camera_instance->getCameraPos());
			Ray mouse_ray = Geometry::SHAPES::createMouseRay();
			object_pos = mouse_ray.origin + mouse_ray.direction * t;


			std::vector<SceneObject*> top_groups = InterfaceManager::GetInstance().get_Area_Selection_Data(utility_to_Render.obj_pack->raw_object->getId()).top_level_groups;
			std::vector<SceneObject*> top_objects = InterfaceManager::GetInstance().get_Area_Selection_Data(utility_to_Render.obj_pack->raw_object->getId()).solo_objects;
			for (SceneObject* obj : top_groups)
			{

				subject_pos = (obj->getBBoxMin() + obj->getBBoxMax()) * 0.5f; //obj->getTranslationAttributes();

				glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
				draw::stripped_Line3D(object_pos, subject_pos, size);
				Render_Object_BBOX(Mediator::requestObjectsArrayIndex(obj->getId()), false);
			}
			for (SceneObject* obj : top_objects)
			{
				subject_pos = obj->getTranslationAttributes() + (obj->getBBoxMin() + obj->getBBoxMax()) * 0.5f;

				glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
				draw::stripped_Line3D(object_pos, subject_pos, size);
			}


		}

	}
	else
	{
		SceneObject* source_obj = InterfaceManager::GetInstance().get_Object_Align_SourceObj();
		SceneObject* target_obj = InterfaceManager::GetInstance().get_Object_Align_TargetObj();
		
		glm::mat4 source_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(source_obj->get_World_Translation_Matrix_value() * source_obj->getRotationMatrix_chain());
		glm::mat4 target_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(target_obj->get_World_Translation_Matrix_value() * target_obj->getRotationMatrix_chain());

		PinholeCamera cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
		float source_dist = cam.getDistanceFrom(source_obj->getTranslationAttributes());
		float target_dist = cam.getDistanceFrom(target_obj->getTranslationAttributes());
		float source_cam_scale_factor = source_dist * TRANSLATION_AXIS_SCALE_FACTOR * cam.getInvScaleFactor(vp) * 1.5f;
		float target_cam_scale_factor = target_dist * TRANSLATION_AXIS_SCALE_FACTOR * cam.getInvScaleFactor(vp) * 1.5f;

		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(source_matrix));
		for (int i = 0; i < 3; i++)
		{
			optix::float3 p0 = ZERO_3f;
			optix::float3 p1 = p0 + GLOBAL_AXIS[i] * source_cam_scale_factor;
			glm::vec3 color = glm::vec3(0.65f);//DEF_AXIS_COLOR[i] * 0.65f;
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y ,color.z);
			draw::Line3D(p0, p1, 1);

			//
			color = glm::vec3(0.1f);
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
			glUniform1i(m_utility_rendering_program["uniform_text_render"], 1);
			draw::Text(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p1 + GLOBAL_AXIS[i]), glm::vec3(1), FONT6, GLOBAL_AXIS_NAME[i]);
			glUniform1i(m_utility_rendering_program["uniform_text_render"], 0);

		}

		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(target_matrix));
		for (int i = 0; i < 3; i++)
		{
			optix::float3 p0 = ZERO_3f;
			optix::float3 p1 = p0 + GLOBAL_AXIS[i] * target_cam_scale_factor;
			glm::vec3 color  = glm::vec3(0.35f);//DEF_AXIS_COLOR[i] * 0.65f;
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
			draw::Line3D(p0, p1, 1);

			//
			color = glm::vec3(0.1f);
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.x, color.y, color.z);
			glUniform1i(m_utility_rendering_program["uniform_text_render"], 1);
			draw::Text(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p1 + GLOBAL_AXIS[i]), glm::vec3(1), FONT6, GLOBAL_AXIS_NAME[i]);
			glUniform1i(m_utility_rendering_program["uniform_text_render"], 0);
		}

	}
	// 
	


}
void OpenGLRenderer::Render_Utility_Camera_Align(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp)
{
	if (!vp.is_active)
		return;

	if (InterfaceManager::GetInstance().getMouseData().m0_down
		&& utility_to_Render.obj_pack->raw_object != 0
		&& utility_to_Render.ut.active)
	{
		optix::float3 bbox_max = utility_to_Render.obj_pack->raw_object->getGeomProperties().bbox_max;
		optix::float3 bbox_min = utility_to_Render.obj_pack->raw_object->getGeomProperties().bbox_min;
		optix::float3 subject_pos = utility_to_Render.obj_pack->raw_object->getTranslationAttributes();
		optix::float3 object_pos;
		glm::vec3 line_color = glm::vec3(1.0f);
		float size = utility_to_Render.obj_pack->cam_inv_scale_factor;

		glm::mat4 model_matrix = glm::mat4(1.0f); //glm::scale(glm::mat4(1.0f), glm::vec3(0.6* utility_to_Render.obj_pack->cam_inv_scale_factor));



			// case : single selection is active
		if (!utility_to_Render.obj_pack->raw_object->isTemporary()
			&& utility_to_Render.obj_pack->raw_object->getType() != GROUP_PARENT_DUMMY)
		{
			float t = optix::length(subject_pos - utility_to_Render.camera_instance->getCameraPos());
			Ray mouse_ray = Geometry::SHAPES::createMouseRay();
			object_pos = mouse_ray.origin + mouse_ray.direction * t;

			Io_Mouse mouse_data = InterfaceManager::GetInstance().getMouseData();
			int x = mouse_data.m0_down_x;
			int y = mouse_data.m0_down_y;
			optix::float2 xy = Utilities::getMousePosRelativeToViewport(vp.id, optix::make_float2(x, y));


			Ray to_mouse_ray = Geometry::SHAPES::createMouseRay(glm::vec2(xy.x, xy.y));
			subject_pos = to_mouse_ray.origin + to_mouse_ray.direction * t;


			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
			draw::stripped_Line3D(object_pos, subject_pos, size);
		}

		else if (utility_to_Render.obj_pack->raw_object->getType() == GROUP_PARENT_DUMMY
				 && !utility_to_Render.obj_pack->raw_object->isTemporary())
		{

			SceneObject* subject = utility_to_Render.obj_pack->raw_object;

			float t = optix::length(subject_pos - utility_to_Render.camera_instance->getCameraPos());
			Ray mouse_ray = Geometry::SHAPES::createMouseRay();
			object_pos = mouse_ray.origin + mouse_ray.direction * t;

			Io_Mouse mouse_data = InterfaceManager::GetInstance().getMouseData();
			int x = mouse_data.m0_down_x;
			int y = mouse_data.m0_down_y;
			optix::float2 xy = Utilities::getMousePosRelativeToViewport(vp.id, optix::make_float2(x, y));


			Ray to_mouse_ray = Geometry::SHAPES::createMouseRay(glm::vec2(xy.x, xy.y));
			subject_pos = (subject->getBBoxMin() + subject->getBBoxMax()) * 0.5f; // subject->getTranslationAttributes();


			glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
			glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
			draw::stripped_Line3D(object_pos, subject_pos, size);

			Render_Object_BBOX(utility_to_Render.obj_pack->object_index, false);

		}

		// case : Area selection is active
		else
		{
			float t = optix::length(subject_pos - utility_to_Render.camera_instance->getCameraPos());
			Ray mouse_ray = Geometry::SHAPES::createMouseRay();
			object_pos = mouse_ray.origin + mouse_ray.direction * t;


			std::vector<SceneObject*> top_groups = InterfaceManager::GetInstance().get_Area_Selection_Data(utility_to_Render.obj_pack->raw_object->getId()).top_level_groups;
			std::vector<SceneObject*> top_objects = InterfaceManager::GetInstance().get_Area_Selection_Data(utility_to_Render.obj_pack->raw_object->getId()).solo_objects;
			for (SceneObject* obj : top_groups)
			{

				subject_pos = (obj->getBBoxMin() + obj->getBBoxMax()) * 0.5f; //obj->getTranslationAttributes();

				glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
				draw::stripped_Line3D(object_pos, subject_pos, size);
				Render_Object_BBOX(Mediator::requestObjectsArrayIndex(obj->getId()), false);
			}
			for (SceneObject* obj : top_objects)
			{
				subject_pos = obj->getTranslationAttributes() + (obj->getBBoxMin() + obj->getBBoxMax()) * 0.5f;

				glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
				glUniform3f(m_utility_rendering_program["uniform_diffuse"], line_color.r, line_color.g, line_color.b);
				draw::stripped_Line3D(object_pos, subject_pos, size);
			}


		}

	}

	//
	SceneObject* focused_object  = InterfaceManager::GetInstance().Get_Focused_Object();
	SceneObject* selected_object = InterfaceManager::GetInstance().Get_Selected_Object();
	int f_id = focused_object != 0 ? focused_object->getId() : -1;
	int s_id = selected_object != 0 ? selected_object->getId() : -1;
	Type focused_object_type  = focused_object  != 0 ? focused_object->getType()  : DUMMY;
	Type selected_object_type = selected_object != 0 ? selected_object->getType() : DUMMY;
	bool cam_align_pickedObject = InterfaceManager::GetInstance().is_cam_align_PickedObject();
	bool cam_align_pickedTarget = InterfaceManager::GetInstance().is_cam_align_PickedTarget();
	NormalVector_Obj cam_align_object = InterfaceManager::GetInstance().get_cam_align_SourceObj();
	NormalVector_Obj cam_align_target = InterfaceManager::GetInstance().get_cam_align_TargetObj();

	// draw Normal Vector on focused object's Surface
	//std::cout << "\n - cam_align_pickedObject : " << cam_align_pickedObject << std::endl;
	//std::cout << " - focused_object : " << (focused_object != 0) << std::endl;


	// ForEach Active Group in Scene draw a Vector for every bbox side
	// Useless!
	if (!cam_align_pickedObject && false)
	{
		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		glm::vec3 color;
		float alpha = 1.0f;

		std::vector<SceneObject*> objects = Mediator::RequestSceneObjects();
		for (SceneObject* obj : objects)
		{
			Type obj_type = obj->getType();
			bool is_active = obj->isActive();
			bool is_not_area_selection = !obj->isTemporary();
			if (obj_type == GROUP_PARENT_DUMMY && is_active && is_not_area_selection)
			{
				SceneObject* group = obj;
				float obj_dist = ViewportManager::GetInstance().getViewportCamera(vp.id).getDistanceFrom(group->getTranslationAttributes());
				float scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp.scale_factor * 4.0f;

				optix::float3 axis[3]; BASE_TRANSFORM::AXIS(group, LOCAL, axis);

				optix::float3 * t_bbox = group->getBBOX_Transformed(true);
				optix::float3 bbox[2] = { t_bbox[0], t_bbox[1] };
				delete t_bbox;

				optix::float3 * b_pointst = group->getBBoxPoints(); // 8
				optix::float3 b_points[8]; for (int i = 0; i < 8; i++) b_points[i] = b_pointst[i];
				delete b_pointst;

				optix::float3 centroid = group->getCentroid_Transformed();
				optix::float3 du = (b_points[6] - b_points[7]); float lu = length(du);
				optix::float3 dv = (b_points[4] - b_points[7]); float lv = length(dv);
				optix::float3 dw = (b_points[3] - b_points[7]); float lw = length(dw);

				optix::float3 pu, pu_, pv, pv_, pw, pw_;
				pu = centroid + du * 0.5f; pu_ = centroid - du * 0.5f;
				pv = centroid + dv * 0.5f; pv_ = centroid - dv * 0.5f;
				pw = centroid + dw * 0.5f; pw_ = centroid - dw * 0.5f;
				 
				optix::float3 Ps[3] = { pu,pv,pw };
				optix::float3 Ds[3] = { du,dv,dw };

				// useless
				{
					glDisable(GL_CULL_FACE);


					float sl = 0.5f;
					float su = 0.9 * sl; // F - B
					float sv = 0.2 * sl; // U - D
					float sw = 0.4 * sl; // L - R
					float u_scales[3] = { su,sv,sw };

					optix::float2 scale_vw = optix::make_float2(sv,sw) * scale_factor;
					optix::float2 scale_uv = optix::make_float2(su,sv) * scale_factor;
					optix::float2 scale_uw = optix::make_float2(su,sw) * scale_factor;
					optix::float2 scales[3] = { scale_vw, scale_uw, scale_uv };

					for(int i = 0; i<3; i++)
					{
						optix::float3 plane_n = normalize(Ds[i]);
						optix::float3 plane_u = normalize(dv);
						optix::float3 plane_v = normalize(dw);
						float scale_u = sv;
						float scale_v = sw;

						if (i == 1) { plane_u = normalize(du); plane_v = normalize(dw); scale_u = su; scale_v = sw; }
						if (i == 2) { plane_u = normalize(du); plane_v = normalize(dv); scale_u = su; scale_v = sv; }

						// draw closest face (n)
						color = glm::vec3(0, 1, 0); glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
						alpha = 0.3f; glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
						
						//draw::Plane_(plane_o, pdu, pdv , scales[i] , 1.0f, GL_POLYGON); // | - 
						//draw::Plane_(plane_o + normalize(Ds[i]) * scale_factor, pdu, pdv, scales[i], 1.0f, GL_POLYGON); // | - |
						


						optix::float3 p0_FRONT = centroid + Ds[i] * 0.5f - plane_u * scale_u - plane_v * scale_v;
						optix::float3 p0_BACK  = p0_FRONT + normalize(Ds[i]) * su * scale_factor;

						optix::float3 p0_LEFT  = p0_FRONT;
						optix::float3 p0_RIGHT = p0_LEFT  + plane_u * sw * scale_factor;
						optix::float3 p0_UP    = p0_FRONT + plane_v * sv * scale_factor;
						optix::float3 p0_DOWN  = p0_FRONT;

						// front - back
						if (1)
						{
							color = glm::vec3(0, 1, 0); glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
							alpha = 0.3f; glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
							draw::Plane_(p0_FRONT, plane_u, plane_v, scales[0] * 2.0f, 1.0f, GL_POLYGON);
							draw::Plane_(p0_BACK, plane_u, plane_v, scales[0] * 2.0f, 1.0f, GL_POLYGON);
							color = glm::vec3(0.2); glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
							alpha = 1.f; glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
							draw::Plane_(p0_FRONT, plane_u, plane_v, scales[0] * 2.0f, 1.0f, GL_LINE_LOOP);
							draw::Plane_(p0_BACK, plane_u, plane_v, scales[0] * 2.0f, 1.0f, GL_LINE_LOOP);
						}

						// aristera - de3ia
						if (1)
						{
							color = glm::vec3(0, 1, 0); glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
							alpha = 0.3f; glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
							draw::Plane_(p0_LEFT,  plane_n, plane_v, scales[2], 1.0f, GL_POLYGON);
							draw::Plane_(p0_RIGHT, plane_n, plane_v, scales[2], 1.0f, GL_POLYGON);
							color = glm::vec3(0.2); glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
							alpha = 1.f; glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
							draw::Plane_(p0_LEFT,  plane_n, plane_v, scales[2], 1.0f, GL_LINE_LOOP);
							draw::Plane_(p0_RIGHT, plane_n, plane_v, scales[2], 1.0f, GL_LINE_LOOP);
						}

						// panw - katw
						if(0)
						{
							color = glm::vec3(0, 1, 0); glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
							alpha = 0.3f; glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
							draw::Plane_(p0_UP, plane_u, plane_n, scales[1], 1.0f, GL_POLYGON);
							draw::Plane_(p0_DOWN, plane_u, plane_n, scales[1], 1.0f, GL_POLYGON);
							color = glm::vec3(0.2); glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
							alpha = 1.f; glUniform1f(m_utility_rendering_program["uniform_alpha"], alpha);
							draw::Plane_(p0_UP, plane_u, plane_n, scales[1], 1.0f, GL_LINE_LOOP);
							draw::Plane_(p0_DOWN, plane_u, plane_n, scales[1], 1.0f, GL_LINE_LOOP);
						}

					}

					glEnable(GL_CULL_FACE);
				}

				color = DEF_AXIS_COLOR[0]; glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
				draw::Point3D(pu, 20.0f, glm::vec3(1.0f)); draw::Point3D(pu_, 20.0f, glm::vec3(1.0f));

				color = DEF_AXIS_COLOR[1]; glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
				draw::Point3D(pv, 20.0f, glm::vec3(1.0f)); draw::Point3D(pv_, 20.0f, glm::vec3(1.0f));

				color = DEF_AXIS_COLOR[2]; glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
				draw::Point3D(pw, 20.0f, glm::vec3(1.0f)); draw::Point3D(pw_, 20.0f, glm::vec3(1.0f));
			}
		}
	}

	if (!cam_align_pickedObject && focused_object != 0 && focused_object_type != FACE_VECTOR)
	{
		
		optix::float3 * buffer = Mediator::request_MouseHit_Buffer_Data();
		optix::float3 o = buffer[0];
		optix::float3 n = buffer[1];
		delete buffer;


		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::vec3 color = glm::vec3(0.05f, 1.0f, 0.25f);

		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

		float obj_dist = ViewportManager::GetInstance().getViewportCamera(vp.id).getDistanceFrom(focused_object->getTranslationAttributes());
		float scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp.scale_factor;
		optix::float3 p0 = o;
		optix::float3 p1 = p0 + normalize(n) * scale_factor * 3.5f;
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
		draw::Line3D(p0, p1, 1.0f);
		draw::Arrow(p0, p1, scale_factor * 1.0f, false, 1.0f);
		
	}
	if (cam_align_pickedObject && cam_align_object.is_object)
	{
		
		optix::float3 o = cam_align_object.origin;
		optix::float3 n = cam_align_object.dir;


		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::vec3 color = glm::vec3(0.05f, 1.0f, 0.25f);

		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));


		float obj_dist = ViewportManager::GetInstance().getViewportCamera(vp.id).getDistanceFrom(cam_align_object.obj_pos);
		float scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp.scale_factor;
		optix::float3 p0 = o;
		optix::float3 p1 = p0 + normalize(n) * scale_factor * 3.5f;
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
		draw::Line3D(p0, p1, 1.0f);
		draw::Arrow(p0, p1, scale_factor * 1.0f, false, 1.0f);
	}
	if (cam_align_pickedObject && focused_object != 0 && f_id != s_id && focused_object_type != FACE_VECTOR)
	{

		optix::float3 * buffer = Mediator::request_MouseHit_Buffer_Data();
		optix::float3 o = buffer[0];
		optix::float3 n = buffer[1];
		delete buffer;


		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::vec3 color = glm::vec3(0.05f, 1.0f, 0.25f);

		glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

		float obj_dist = ViewportManager::GetInstance().getViewportCamera(vp.id).getDistanceFrom(focused_object->getTranslationAttributes());
		float scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp.scale_factor;
		optix::float3 p0 = o;
		optix::float3 p1 = p0 + normalize(n) * scale_factor * 3.5f;
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
		draw::Line3D(p0, p1, 1.0f);
		draw::Arrow(p0, p1, scale_factor * 1.0f, false, 1.0f);

	
	}

}

void OpenGLRenderer::Render_Light_Object(Utility_RenderList_Object  ut)
{
	
}
void OpenGLRenderer::Render_Light_Object_Properties(Utility_RenderList_Object  ut, const VIEWPORT& vp)
{
	////std::cout << " - OpenGLRenderer::Render_Light_Object_Properties( ut, vp = "<< (int)vp.id<< " ):" << std::endl;
	optix::float3 M_WORKING_AXIS[3] = { GLOBAL_AXIS[0], GLOBAL_AXIS[1], GLOBAL_AXIS[2] };
	glm::mat4 model_matrix;
	glm::vec3 diffuse;
	SceneObject* light      = ut.obj_pack->raw_object;
	Light_Parameters params = light->get_LightParameters();
	Light_Type light_type   = params.type;

	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(ut.obj_pack->translation_matrix_scaled));
	
	PinholeCamera cam_instance = ViewportManager::GetInstance().getViewportCamera(vp.id);
	float obj_dist     = cam_instance.getDistanceFrom(light->getTranslationAttributes());
	float scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp.scale_factor;
	optix::float3 p0, p1;

	if (light_type == QUAD_LIGHT)
	{
		p0 = ZERO_3f + M_WORKING_AXIS[0] * light->getScaleAttributes().x * 0.5f + M_WORKING_AXIS[2] * light->getScaleAttributes().z * 0.5f;
		p1 = p0 + optix::make_float3(0, -1, 0) * scale_factor *  FACE_VECTOR_SCALE_FACTOR;
	}
	else
	{
		p0 = ZERO_3f;
		p1 = p0 + optix::make_float3(0, -1, 0) * scale_factor  * FACE_VECTOR_SCALE_FACTOR;
	}

	if      (light_type == QUAD_LIGHT)
	{
		draw::Frustum_Area_Light(p0, optix::make_float2(45.0f), optix::make_float2(0.0f,100.0f) * 0.01f * scale_factor * FACE_VECTOR_SCALE_FACTOR, m_utility_rendering_program);
	}
	else if (light_type == SPOTLIGHT)
	{
		draw::Frustum(ZERO_3f, params.cone_size, params.clipping_plane, m_utility_rendering_program);
		glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
		draw::Cube(p0 + optix::make_float3(0, -1, 0) * params.clipping_plane.y + optix::make_float3(0,1,0) * 20.0f , 40.0f, M_WORKING_AXIS, false);
	}
	else if (light_type == SPHERICAL_LIGHT || light_type == POINT_LIGHT)
	{

	}

	// draw Light's Normal Arrow
	if (light_type == Light_Type::SPOTLIGHT || light_type == Light_Type::QUAD_LIGHT)
	{
		//glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 0.0f);
		//draw::Line3D(p0, p1, 2.0f);
		//draw::Arrow( p0, p1, scale_factor * 1.3f , vp, false);
	}

	
}


#endif

#endif


#ifdef UTILITY_RENDERER_UTIL_FUNCTIONS


void OpenGLRenderer::gather_Scene_Data()
{
	////std::cout << "\n - GATHER_SCENE_DATA(): " << std::endl;

	// get Active Viewports
	std::vector<VIEWPORT> viewports = ViewportManager::GetInstance().getAllEnabledViewports();

	// traverse Scene //
	{


		bool USE_OPTIX_SCENE_DEPTH = false;
#ifdef USE_OptiXViewport_Scene_Depth
		//USE_OPTIX_SCENE_DEPTH = true;
#endif
		
		std::vector<SceneObject*> objects = Mediator::RequestSceneObjects();
		SceneObject* scene_grid = objects[0]; // SCENE_OBJECTS_NOT_UTILITY //
		
		SceneObject* selected_object = Mediator::Request_INTERFACE_Selected_Object(); //Mediator::RequestSelectedObject();
		int selected_id = selected_object == nullptr ? -1 : selected_object->getId();
		SceneObject* focused_object = Mediator::Request_INTERFACE_Focused_Object(); //Mediator::RequestFocusedObject();
		int focused_id    = focused_object == nullptr ? -1 : focused_object->getId();
		int ui_focused_id = Mediator::Request_Ui_Focused_Object();

		bool selected_obj_is_group = selected_object == nullptr ? false : selected_object->getType() == GROUP_PARENT_DUMMY;
		bool focused_obj_is_group  = focused_object  == nullptr ? false : focused_object->getType()  == GROUP_PARENT_DUMMY;

		FaceVector_Intersection_State FIS = InterfaceManager::GetInstance().get_FaceVector_Intersection_State();
		
		SceneObject* m_UI_focused_Relation_OBJECT = InterfaceManager::GetInstance().get_UI_Focused_Relation();
		int m_UI_focused_Relation_OBJECT_id       = m_UI_focused_Relation_OBJECT == nullptr ? -1 : m_UI_focused_Relation_OBJECT->getId();
		bool selected_object_FORCE_draw_Relation  = (InterfaceManager::GetInstance().get_Active_Tab() == RELATIONS_TAB && selected_id != -1);
		int m_UI_selected_Relation_OBJECT_id      = (selected_object_FORCE_draw_Relation) ? selected_id : -1;
		
		std::vector<Shape> samplers_shapes;
		std::vector<Shape> other_shapes;
		std::vector<Shape> facevector_shapes;


		// gather data for each Scene object
		for( SceneObject* obj : objects )
		{
			Type obj_type = obj->getType();
			bool is_Sampler       = obj_type == SAMPLER_OBJECT;
			bool is_Light         = obj_type == LIGHT_OBJECT;
			Light_Type light_type = obj->get_LightParameters().type;
			bool is_FaceVector    = obj_type == FACE_VECTOR;
			optix::float3 obj_pos = obj->getTranslationAttributes();
			bool obj_is_group     = (obj_type == GROUP_PARENT_DUMMY);
			bool is_app_grid      = (obj_type == APP_OBJECT);
			bool is_valid         = !(obj->isTemporary() 
									  || obj_type == FACE_VECTOR  
									  //|| obj_type == SAMPLER_OBJECT 
									  || is_app_grid 
									  || !obj->isActive());
			bool is_visible = obj->isVisible();

			if ( !is_valid )
				continue;

			
			bool is_ui_focused = (ui_focused_id == obj->getId());
			bool is_selected = (selected_id == obj->getId());
			bool is_focused  = (focused_id  == obj->getId()) || (is_ui_focused);

			SceneObject*  parent    = obj->getParent();
			bool has_parent         = parent == nullptr ? false : true;
			int  parent_id           = (!has_parent) ? -1 : parent->getId();
			bool is_parent_selected = (!has_parent) ? false : (parent_id == selected_id);
			bool parent_is_group    = (!has_parent) ? false : parent->getType() == GROUP_PARENT_DUMMY;
			bool parent_is_temp     = (!has_parent) ? false : parent->isTemporary();

			SceneObject* group_parent     = obj->getGroupParent();
			bool has_group_parent         = group_parent == nullptr ? false : true;
			int grp_parent_id             = has_group_parent ? group_parent->getId() : -1;
			bool is_temp_group            = has_group_parent ? group_parent->isTemporary() : false;
			bool is_grp_parent_selected   = (!has_group_parent) ? false : (grp_parent_id == selected_id);
			bool is_grp_parent_ui_focused = (!has_group_parent) ? false : (grp_parent_id == ui_focused_id);
			bool is_grp_parent_focused    = (!has_group_parent) ? false : ((grp_parent_id == focused_id) || (is_grp_parent_ui_focused));


			bool render_parent_relation = true;
			if (selected_obj_is_group && !is_selected && has_group_parent && !(is_ui_focused || is_grp_parent_ui_focused))
			{
				bool s = Utilities::is_object_in_List(obj, selected_object->getGroupSelectionChilds_and_Groups());
				is_selected |= s;
				//if (s) render_parent_relation = false;
			}
			if (focused_obj_is_group && !is_focused && has_group_parent && !(is_ui_focused || is_grp_parent_ui_focused))
			{
				bool s = Utilities::is_object_in_List(obj, focused_object->getGroupSelectionChilds_and_Groups());
				is_focused |= s;
				//if (s) render_parent_relation = false;
			}

			bool force_draw_Relation = false;
			force_draw_Relation     |= (obj->getId()  == m_UI_focused_Relation_OBJECT_id );
			force_draw_Relation     |= (obj->getId()  == m_UI_selected_Relation_OBJECT_id);
			force_draw_Relation     |= (has_parent) && (parent_id == m_UI_focused_Relation_OBJECT_id || parent_id == m_UI_selected_Relation_OBJECT_id);
			force_draw_Relation     |= (has_group_parent) && (grp_parent_id == m_UI_focused_Relation_OBJECT_id || grp_parent_id == m_UI_selected_Relation_OBJECT_id);
			if (force_draw_Relation && !is_selected)
			{
				render_parent_relation = false;
				is_selected = true;
			}

			
			glm::vec3 highlight_color = glm::vec3(1.0f);
			
			
			// for each viewport : gather data for current object // 
			for (VIEWPORT vp : viewports)
			{
				

				int vp_id          = vp.id;
				int r_relations    = 0;
				int r_face_vectors = 0;
				int r_restrictions = 0;
				int r_samplers     = 0;
				int r_bbox         = 0;
				
				bool draw_r_details  = vp.render_settings.draw_restriction_details;
				bool draw_r_cones    = vp.render_settings.draw_restriction_cones;
				bool draw_r_fv       = vp.render_settings.draw_restriction_fv;
				bool draw_r_fv_lines = vp.render_settings.draw_restriction_fv_lines;
				bool draw_dist_area  = vp.render_settings.draw_restriction_dist_area;
				bool draw_objects = vp.render_settings.draw_objects_all;
				bool draw_groups  = vp.render_settings.draw_groups_all;

				if (vp.render_settings.draw_bbox_only) r_bbox = 1;
				else if (vp.render_settings.draw_bbox_and_object) r_bbox = 2;

				if (vp.render_settings.draw_sampler_only_selected
					|| (is_selected && is_Sampler)
					|| (is_focused && is_Sampler)
					) r_samplers = 1;
				else if (vp.render_settings.draw_sampler_all) r_samplers = 2;

				if (vp.render_settings.draw_relations_only_selected || force_draw_Relation) r_relations = 1;
				else if (vp.render_settings.draw_relations_all)  r_relations = 2;

				if (FIS == FaceVector_Intersection_State::FIS_NONE)
				{
					if (vp.render_settings.draw_face_vectors_only_selected
						|| (is_selected && is_FaceVector)
						) r_face_vectors = 1;
					else if (vp.render_settings.draw_face_vectors_all) r_face_vectors = 2;
				}
				else
					r_face_vectors = 2;

				
				PinholeCamera vp_cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
				float obj_dist = vp_cam.getDistanceFrom(obj_pos);
				float vp_scale_factor = vp_cam.getInvScaleFactor(vp);



				if (is_Light && is_visible)
				{
					bool is_valid = true;
					Light_Type ltype = obj->get_LightParameters().type;
					if (ltype == QUAD_LIGHT && !(vp.render_settings.draw_lights_all || vp.render_settings.draw_lights_quad))
					{
						is_valid = false;
					}
					if (ltype == SPOTLIGHT && !(vp.render_settings.draw_lights_all || vp.render_settings.draw_lights_spot))
					{
						is_valid = false;
					}
					if (ltype == SPHERICAL_LIGHT && !(vp.render_settings.draw_lights_all || vp.render_settings.draw_lights_sphere))
					{
						is_valid = false;
					}
					if (ltype == POINT_LIGHT && !(vp.render_settings.draw_lights_all || vp.render_settings.draw_lights_point))
					{
						is_valid = false;
					}

					if (is_valid)
					{
						optix::float3 c = obj->getCentroid_Transformed();
						optix::float3 * b = obj->getBBOX_Transformed(true);
						optix::float3 bbox[2] = { b[0], b[1] };
						delete b;

						PinholeCamera cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
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

						Io_Mouse mouse_data = Mediator::Request_App_MouseData();
						bool is_hovered = obj->is_LightButtonHovered();
						bool is_clicked = is_hovered && mouse_data.m0_clicked;
						bool is_down = is_hovered && mouse_data.m0_down && !is_clicked;
						bool is_on = obj->get_LightParameters().is_on;

						float f = is_on ? 0.5f : 1.0f;
						plane.thickness = 1.0f;
						if (is_hovered) plane.color = glm::vec3(0.5, 0.5, 0.5);
						if (is_clicked || is_down) plane.color = glm::vec3(0.4, 0.4, 0.4);
						if (!is_hovered) plane.color = glm::vec3(0.7, 0.7, 0.7) *f;
						plane.alpha = 0.8f;
						other_shapes.push_back(plane);

						plane.color = is_on ? glm::vec3(0.8f,0.8f,0.5f) : glm::vec3(0.4f);
						plane.is_outline = 1;
						plane.alpha = 0.8f;
						plane.thickness = 2.0f;
						other_shapes.push_back(plane);

					}
				}



				// define Bbox's color //
				if ( r_bbox == 1  && !is_Sampler 
					&& vp.renderer == OPTIX_RENDERER
					//&& !USE_OPTIX_SCENE_DEPTH
					)
				{

					if (is_selected || is_grp_parent_selected && !(is_ui_focused || is_grp_parent_ui_focused))
						highlight_color = glm::vec3(1.0f);
					else if (is_focused && !is_grp_parent_focused && obj_type != GROUP_PARENT_DUMMY )
						highlight_color = VECTOR_TYPE_TRANSFORM::TO_GLM_3f( obj->get_Highlight_Color() );
					else if (is_grp_parent_focused && obj_type != GROUP_PARENT_DUMMY)
						highlight_color = HIGHLIGHT_COLOR;
					else if ( obj_type != GROUP_PARENT_DUMMY )
					{
						float * Kd = obj->getMaterialParams(0).Kd;
						highlight_color = glm::vec3(Kd[0], Kd[1], Kd[2]);
						
					}

				}

				// Render BoundingBoxes //
				// edw ginontai issue ta draw gia ton optix
				// ta bbox ginontai draw sto render scene apta opengl viewports
				if (  (r_bbox != 0 /* && !app_grid*/  
					|| ( interface_state->active_utility->group == g_RESTRICTIONS && obj_is_group ))
					&& !is_Sampler
					&& !is_Light
					&& vp.renderer == OPTIX_RENDERER
					&& is_visible
					&& (obj_type != GROUP_PARENT_DUMMY || draw_groups)
					//&& !USE_OPTIX_SCENE_DEPTH
					)
				{

					Shape BBOX;
					BBOX.shape_id = SHAPE_ID::BoundingBox;
					BBOX.color = highlight_color; //glm::vec3(1.0f, 1.0f, 1.0f);
					BBOX.a = obj->getBBoxMin();
					BBOX.b = obj->getBBoxMax();
					BBOX.model_matrix = obj_is_group ? glm::mat4(1.0f) : MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getTransformationMatrix());
					BBOX.bbox_dashed = ( /*!is_selected &&*/ obj_is_group ) ? true : false;
					BBOX.thickness   = ( (obj_is_group) && is_focused && !is_selected) ? 2.0f : 1.0f;
					//m_Shapes2D_per_Viewport[vp_id].push_back(BBOX);
					other_shapes.push_back(BBOX);
				}


				// Render Relations //
				if (r_relations != 0 && !is_Sampler)
				{

					// only selected 
					if ( (r_relations == 1 && (is_selected || is_parent_selected || is_grp_parent_selected)) 
						|| (r_relations == 2)
						)
					{
						// case : obj is Group -> Render Cube represantation //
						if (obj_is_group && (r_relations == 2))
						{
							Shape cube;
							cube.shape_id = SHAPE_ID::CUBE;
							cube.pos = obj_pos;
							cube.scale_factor = 20.0f; //UTILITY_SCALE_FACTOR * obj_dist * vp_scale_factor * 0.1f;
							cube.size  = 0.0f;
							cube.color = glm::vec3(0.0f, 0.0f, 1.0f);
							cube.is_outline = 1;
							//m_Shapes2D_per_Viewport[vp_id].push_back(cube);
							other_shapes.push_back(cube);
						}

						// case : obj has Parent //
						if (has_parent && !parent_is_temp 
							&& (is_selected || is_parent_selected || r_relations == 2)
							)
						{
							optix::float3 from = parent_is_group ? parent->getTranslationAttributes() : parent->getCentroid_Transformed(); // parent->getTranslationAttributes();
							optix::float3 to   = obj->getTranslationAttributes();   

							Shape arrow;
							arrow.shape_id = SHAPE_ID::LINE;
							arrow.scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp_scale_factor * 0.75f;
							arrow.arrow_scale_factor = 1.0f;
							arrow.a = from;
							arrow.b = to;
							arrow.draw_arrow = true;
							arrow.arrow_scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp_scale_factor * 0.75f; // 10.0f

							glm::vec3 color;
							if (parent_is_group)
								color = glm::vec3(0.0f, 0.8f, 1.0f);
							else
							{
								float * Kd = parent->getMaterialParams(0).Kd;
								color = glm::vec3(Kd[0], Kd[1], Kd[2]);
							}
							arrow.color = color;
							//m_Shapes2D_per_Viewport[vp_id].push_back(arrow);
							other_shapes.push_back(arrow);


							if ( has_group_parent && !is_temp_group && ( is_selected || is_grp_parent_selected || r_relations == 2 ) && render_parent_relation )
							{
								if ( group_parent != parent )
								{
									from = group_parent->getTranslationAttributes();
									to = obj->getCentroid_Transformed();

									Shape line;
									line.shape_id = SHAPE_ID::LINE;
									line.scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp_scale_factor * 0.75f;
									line.a = from;
									line.b = to;
									line.stripped = true;
									line.draw_arrow = false;
									line.color = glm::vec3(0.0f, 0.8f, 1.0f);
									//m_Shapes2D_per_Viewport[vp_id].push_back(line);
									other_shapes.push_back(line);
								}

								if ( r_relations == 1 )
								{
									Shape cube;
									cube.shape_id = SHAPE_ID::CUBE;
									cube.pos = from;
									cube.scale_factor = 20.0f; //UTILITY_SCALE_FACTOR * obj_dist * vp_scale_factor * 0.1f;
									cube.size = 0.0f;
									cube.color = glm::vec3(0.0f, 0.0f, 1.0f);
									cube.is_outline = 1;
									//m_Shapes2D_per_Viewport[vp_id].push_back(cube);
									other_shapes.push_back(cube);
								}
							}
						}

					}

				}

				// Render Face Vectors //
				if ((r_face_vectors != 0 || is_Light) 
					&& !is_Sampler) 
				{
					bool is_selected = (selected_id == obj->getId());
					if (
						( (r_face_vectors == 1 || is_Light) && is_selected )
						|| (r_face_vectors == 2)
						)
					{
						for (SceneObject* face_vector : obj->get_Face_Vectors())
						{
							bool is_valid   = face_vector->isActive();
							bool is_visible = face_vector->isVisible();
							if (!is_valid || !is_visible)
								continue;

							float obj_dist = vp_cam.getDistanceFrom(face_vector->getTranslationAttributes());
							float scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp_scale_factor; // 0.4f

							Shape arrow;
							arrow.shape_id = SHAPE_ID::LINE;
							arrow.scale_factor = 1.0f;
							
							arrow.thickness = FACE_VECTOR_LINE_WIDTH;
							arrow.gl_line_smooth = true;
							arrow.arrow_scale_factor = scale_factor * FACE_VECTOR_ARROW_SCALE_FACTOR;
							arrow.a = ZERO_3f;
							arrow.b = ZERO_3f + GLOBAL_AXIS[2] * FACE_VECTOR_SCALE_FACTOR * scale_factor;
							arrow.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(face_vector->getTransformationMatrix());
							arrow.use_trns = true;
							arrow.use_shader = true;
							arrow.draw_arrow = true;
							glm::vec3 color = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(face_vector->get_FaceVectorParameters().color);
							arrow.color = arrow.color = is_Light ? glm::vec3(1, 1, 0) : color;
							other_shapes.push_back(arrow);
						}
					}
				}

				// Render Samplers //
				if (r_samplers != 0)
				{
					bool is_sampler_selected = selected_id == obj->getId();
					bool is_sampler_focused  = focused_id  == obj->getId();
					bool is_directional_sampler = obj->get_SamplerParameters().type == s_DIRECTIONAL;

					float alpha_value = SAMPLER_FILL_ALPHA;
					float alpha_value2 = 0.5f;
					if (    r_samplers == 2
						|| (r_samplers == 1 && is_sampler_selected)
						|| (r_samplers == 1 && (is_ui_focused && is_Sampler))
						)
					{
						
						if (!is_Sampler
							//&& r_samplers == 2
							)
						{
							for (SceneObject* sampler : obj->get_Samplers())
							{
								bool is_valid = sampler->isActive() && sampler->isVisible();
								if (!is_valid) continue;

								bool is_selected = sampler->getId() == selected_id;
								bool is_focused = sampler->getId() == focused_id;

								glm::vec3 s_dir_color = glm::vec3(0.1f, 1.0f, 0.2f) * 0.65f;
								glm::vec3 s_dir_color_c = glm::vec3(0.1f, 1.0f, 0.2f);
								glm::vec3 s_dir_color_cf = glm::vec3(0.1f, 1.0f, 0.2f) * 0.75f;
								if (is_selected) s_dir_color_cf = s_dir_color_c = s_dir_color = SELECTED_COLOR;
								else if (is_focused) s_dir_color_cf = s_dir_color_c = s_dir_color = HIGHLIGHT_COLOR;


								optix::float3 pos = sampler->getTranslationAttributes();
								optix::float3 centroid = sampler->getCentroid_Transformed();
								optix::float3 scale = sampler->getScaleAttributes();
								Sampler_Parameters params = sampler->get_SamplerParameters();
								optix::Matrix4x4 TRS = sampler->getTransformationMatrix();

								float dist = vp_cam.getDistanceFrom(pos);
								float obj_scale_factor = LINKING_SCALE_FACTOR * dist  * vp_scale_factor;

								// line from to
								{
									Shape l;
									l.shape_id = SHAPE_ID::LINE;
									l.scale_factor = 1.0f;
									l.thickness = 1.0f;
									//l.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(TRS);
									l.use_trns = false;
									l.arrow_scale_factor = 1.0f;
									//l.a = sampler->getParent()->getTranslationAttributes();
									l.a = sampler->getParent()->getCentroid_Transformed();
									l.b = params.type == s_DIRECTIONAL ? pos : centroid;
									l.draw_arrow = false;
									l.arrow_scale_factor = l.scale_factor;
									l.color = glm::vec3(1.0f);
									l.alpha = 1.0f;
									l.is_first_pass = false;
									//m_Shapes2D_per_Viewport[vp.id].push_back(l);
									//samplers_shapes.push_back(l);
									other_shapes.push_back(l);
								}


								if      (params.type == s_PLANE)
								{
									Shape plane;
									plane.use_shader = 1;
									plane.use_trns = 0;
									plane.model_matrix = glm::mat4(1.0f);

									//plane.shape_id = SHAPE_ID::PLANE_CENTERED;
									plane.shape_id = SHAPE_ID::PLANE;
									plane.pos = pos;
									BASE_TRANSFORM::AXIS(sampler, LOCAL, plane.axis);
									plane.axis[0] *= scale.x * 10.0f;
									plane.axis[2] *= scale.z * 10.0f;

									plane.color = SAMPLER_COLOR_FILL;
									plane.alpha = 0.3f;
									//m_Shapes2D_per_Viewport[vp_id].push_back(plane);
									//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, plane);
									insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, plane);


									Shape plane_outline;
									plane_outline.use_shader = 1;
									plane_outline.use_trns = 0;
									plane_outline.model_matrix = glm::mat4(1.0f);

									plane_outline.shape_id = SHAPE_ID::PLANE;
									plane_outline.pos = pos;
									BASE_TRANSFORM::AXIS(sampler, LOCAL, plane_outline.axis);
									plane_outline.axis[0] *= scale.x * 10.0f;
									plane_outline.axis[2] *= scale.z * 10.0f;

									plane_outline.color = SAMPLER_OUTLINE_COLOR;
									plane_outline.alpha = 0.8f;
									plane_outline.is_outline = 1;
									plane_outline.thickness = SAMPLER_OUTLINE_THICKNESS;
									//m_Shapes2D_per_Viewport[vp_id].push_back(plane_outline);
									//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, plane_outline);
									insert_Sampler_with_Depth_Order(samplers_shapes,(VIEWPORT_ID)vp_id, plane_outline);
								}
								else if (params.type == s_VOLUME)
								{
									Shape plane;
									plane.use_shader = 1;
									plane.use_trns = 0;
									plane.model_matrix = glm::mat4(1.0f);

									plane.shape_id = SHAPE_ID::SAMPLER_CUBE;
									plane.pos = pos + (centroid - pos) * optix::make_float3(1, 1, 1);
									BASE_TRANSFORM::AXIS(sampler, LOCAL, plane.axis);
									plane.axis[0] *= scale.x  * 10.0f * 1.0f;;
									plane.axis[1] *= scale.y  * 10.0f * 1.0f;;
									plane.axis[2] *= scale.z  * 10.0f * 1.0f;;

									plane.color = SAMPLER_COLOR_FILL;
									plane.alpha = 0.3f; //alpha_value * 0.7f;
									//m_Shapes2D_per_Viewport[vp_id].push_back(plane);
									//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, plane);
									insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, plane);

									Shape plane_outline;
									plane_outline.use_shader = 1;
									plane_outline.use_trns = 0;
									plane_outline.model_matrix = glm::mat4(1.0f);

									plane_outline.shape_id = SHAPE_ID::SAMPLER_CUBE;
									plane_outline.pos = plane.pos;
									BASE_TRANSFORM::AXIS(sampler, LOCAL, plane_outline.axis);
									plane_outline.axis[0] *= scale.x  * 10.0f * 1.0f;;
									plane_outline.axis[1] *= scale.y  * 10.0f * 1.0f;;
									plane_outline.axis[2] *= scale.z  * 10.0f * 1.0f;

									plane_outline.color = SAMPLER_OUTLINE_COLOR;
									plane_outline.alpha = 0.8f;
									plane_outline.is_outline = 1;
									plane_outline.thickness = SAMPLER_OUTLINE_THICKNESS;
									//m_Shapes2D_per_Viewport[vp_id].push_back(plane_outline);
									//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, plane_outline);
									insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, plane_outline);
								}
								else if (params.type == s_DIRECTIONAL)
								{
									// cone
									Shape cone;
									cone.shape_id = CONE_FRUSTUM;
									cone.color = glm::vec3(0.1f);
									cone.thickness = 2.0f;
									cone.is_restriction = true;
									cone.arrow_scale_factor = obj_scale_factor * 1.3f;
									cone.length = FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 1.f;
									cone.a = ZERO_3f; // cone's Origin
									cone.b = ZERO_3f + GLOBAL_AXIS[2] * cone.length;
									cone.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(TRS);
									cone.fov = params.cone_size;
									cone.is_directional = 1;
									//m_Shapes2D_per_Viewport[vp.id].push_back(cone);
									//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, cone);
									insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, cone);

									// line
									Shape fva;
									fva.shape_id = SHAPE_ID::LINE;
									fva.scale_factor = obj_scale_factor * FACE_VECTOR_ARROW_SCALE_FACTOR;
									fva.thickness = FACE_VECTOR_LINE_WIDTH;
									fva.gl_line_smooth = true;
									fva.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(TRS);
									fva.use_trns = true;
									fva.arrow_scale_factor = 1.0f;
									fva.a = ZERO_3f;
									fva.b = ZERO_3f + GLOBAL_AXIS[2] * FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 1.f;  //2.f;
									fva.draw_arrow = false;
									fva.arrow_scale_factor = fva.scale_factor;
									fva.color = s_dir_color;
									fva.alpha = 1.0f;
									fva.is_first_pass = 1;
									fva.is_directional = 1;
									//m_Shapes2D_per_Viewport[vp.id].push_back(fva);
									//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, fva);
									insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, fva);


									// cube
									Shape cube;
									cube.shape_id = SHAPE_ID::SAMPLER_CUBE;
									cube.is_first_pass = 0;
									//cube.is_directional = true;
									cube.use_shader = 1;
									cube.use_trns = 1;
									cube.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(TRS);
									cube.pos = fva.b;
									cube.axis[0] *= FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 0.155f;
									cube.axis[1] *= FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 0.155f;
									cube.axis[2] *= FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 0.155f;
									cube.color = s_dir_color_c; //glm::vec3(0.1f, 1.0f, 0.2f);
									cube.alpha = 0.65f;
									//m_Shapes2D_per_Viewport[vp_id].push_back(cube);
									//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, cube);
									insert_Sampler_with_Depth_Order(samplers_shapes,(VIEWPORT_ID)vp_id, cube);

									cube.color = glm::vec3(0.0f);
									cube.alpha = 0.8f;
									cube.is_outline = 1;
									//m_Shapes2D_per_Viewport[vp_id].push_back(cube);
									//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, cube);
									insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, cube);
								}
							}
						}
						else if(      r_samplers == 1 && (is_sampler_selected || is_sampler_focused) && is_visible
						         //|| ( r_samplers == 1 && is_sampler_focused && is_directional_sampler )
								 )
						{

							SceneObject * sampler  = obj;
							optix::float3 pos      = sampler->getTranslationAttributes();
							optix::float3 centroid = sampler->getCentroid_Transformed();
							optix::float3 scale    = sampler->getScaleAttributes();
							Sampler_Parameters params = sampler->get_SamplerParameters();
							optix::Matrix4x4 TRS      = sampler->getTransformationMatrix();

							float dist = vp_cam.getDistanceFrom(pos);
							float obj_scale_factor = LINKING_SCALE_FACTOR * dist  * vp_scale_factor;

							glm::vec3 s_dir_color    = glm::vec3(0.1f, 1.0f, 0.2f) * 0.65f;
							glm::vec3 s_dir_color_c  = glm::vec3(0.1f, 1.0f, 0.2f);
							glm::vec3 s_dir_color_cf = glm::vec3(0.1f, 1.0f, 0.2f) * 0.75f;
							if (is_sampler_selected) s_dir_color_cf = s_dir_color_c = s_dir_color = SELECTED_COLOR;
							else if (is_sampler_focused) s_dir_color_cf = s_dir_color_c = s_dir_color = HIGHLIGHT_COLOR;


							// line from to
							{
								Shape l;
								l.shape_id = SHAPE_ID::LINE;
								l.scale_factor = 1.0f;
								l.thickness = 1.0f;
								//l.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(TRS);
								l.use_trns = false;
								l.arrow_scale_factor = 1.0f;
								//l.a = sampler->getParent()->getTranslationAttributes();
								l.a = sampler->getParent()->getCentroid_Transformed();
								l.b = params.type == s_DIRECTIONAL? pos : centroid;
								l.draw_arrow = false;
								l.arrow_scale_factor = l.scale_factor;
								l.color = glm::vec3(1.0f);
								l.alpha = 1.0f;
								l.is_first_pass = false;
								//m_Shapes2D_per_Viewport[vp.id].push_back(l);
								//samplers_shapes.push_back(l);
								other_shapes.push_back(l);
							}

							if (params.type == s_PLANE)
							{
								Shape plane;
								plane.use_shader = 1;
								plane.use_trns = 0;
								plane.model_matrix = glm::mat4(1.0f);

								//plane.shape_id = SHAPE_ID::PLANE_CENTERED;
								plane.shape_id = SHAPE_ID::PLANE;
								plane.pos = pos;
								BASE_TRANSFORM::AXIS(sampler, LOCAL, plane.axis);
								plane.axis[0] *= scale.x * 10.0f;
								plane.axis[2] *= scale.z * 10.0f;

								plane.color = SAMPLER_COLOR_FILL;
								plane.alpha = 0.3f;
								//m_Shapes2D_per_Viewport[vp_id].push_back(plane);
								//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, plane);
								insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, plane);

								Shape plane_outline;
								plane_outline.use_shader = 1;
								plane_outline.use_trns = 0;
								plane_outline.model_matrix = glm::mat4(1.0f);

								plane_outline.shape_id = SHAPE_ID::PLANE;
								plane_outline.pos = pos;
								BASE_TRANSFORM::AXIS(sampler, LOCAL, plane_outline.axis);
								plane_outline.axis[0] *= scale.x * 10.0f;
								plane_outline.axis[2] *= scale.z * 10.0f;

								plane_outline.color = SAMPLER_OUTLINE_COLOR;
								plane_outline.alpha = 0.8f;
								plane_outline.is_outline = 1;
								plane_outline.thickness = SAMPLER_OUTLINE_THICKNESS;
								//m_Shapes2D_per_Viewport[vp_id].push_back(plane_outline);
								//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, plane_outline);
								insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, plane_outline);
							}
							else if (params.type == s_VOLUME)
							{
								Shape plane;
								plane.use_shader = 1;
								plane.use_trns = 0;
								plane.model_matrix = glm::mat4(1.0f);

								plane.shape_id = SHAPE_ID::SAMPLER_CUBE;
								plane.pos = pos + (centroid - pos) * optix::make_float3(1, 1, 1);
								BASE_TRANSFORM::AXIS(sampler, LOCAL, plane.axis);
								plane.axis[0] *= scale.x  * 10.0f * 1.0f;;
								plane.axis[1] *= scale.y  * 10.0f * 1.0f;;
								plane.axis[2] *= scale.z  * 10.0f * 1.0f;;

								plane.color = SAMPLER_COLOR_FILL;
								plane.alpha = 0.3f; //alpha_value * 0.7f;
								//m_Shapes2D_per_Viewport[vp_id].push_back(plane);
								//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, plane);
								insert_Sampler_with_Depth_Order(samplers_shapes,(VIEWPORT_ID)vp_id, plane);

								Shape plane_outline;
								plane_outline.use_shader = 1;
								plane_outline.use_trns = 0;
								plane_outline.model_matrix = glm::mat4(1.0f);

								plane_outline.shape_id = SHAPE_ID::SAMPLER_CUBE;
								plane_outline.pos = plane.pos;
								BASE_TRANSFORM::AXIS(sampler, LOCAL, plane_outline.axis);
								plane_outline.axis[0] *= scale.x  * 10.0f * 1.0f;;
								plane_outline.axis[1] *= scale.y  * 10.0f * 1.0f;;
								plane_outline.axis[2] *= scale.z  * 10.0f * 1.0f;

								plane_outline.color = SAMPLER_OUTLINE_COLOR;
								plane_outline.alpha = 0.8f;
								plane_outline.is_outline = 1;
								plane_outline.thickness = SAMPLER_OUTLINE_THICKNESS;
								//m_Shapes2D_per_Viewport[vp_id].push_back(plane_outline);
								//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, plane_outline);
								insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, plane_outline);
							}
							else if (params.type == s_DIRECTIONAL)
							{
								
								// cone
								Shape cone;
								cone.shape_id = CONE_FRUSTUM;
								cone.color = glm::vec3(0.1f);
								cone.thickness = 2.0f;
								cone.is_restriction = true;
								cone.arrow_scale_factor = obj_scale_factor * 1.3f;
								cone.length = FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 1.f;
								cone.a = ZERO_3f; // cone's Origin
								cone.b = ZERO_3f + GLOBAL_AXIS[2] * cone.length;
								cone.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(TRS);
								cone.fov = params.cone_size;
								cone.is_directional = 1;
								//m_Shapes2D_per_Viewport[vp.id].push_back(cone);
								//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, cone);
								insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, cone);

								// line
								Shape fva;
								fva.shape_id = SHAPE_ID::LINE;
								fva.scale_factor = obj_scale_factor * FACE_VECTOR_ARROW_SCALE_FACTOR;
								fva.thickness = FACE_VECTOR_LINE_WIDTH;
								fva.gl_line_smooth = true;
								fva.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(TRS);
								fva.use_trns = true;
								fva.arrow_scale_factor = 1.0f;
								fva.a = ZERO_3f;
								fva.b = ZERO_3f + GLOBAL_AXIS[2] * FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 1.f;  //2.f;
								fva.draw_arrow = false;
								fva.arrow_scale_factor = fva.scale_factor;
								fva.color = s_dir_color;
								fva.alpha = 1.0f;
								fva.is_first_pass  = 1;
								fva.is_directional = 1;
								//m_Shapes2D_per_Viewport[vp.id].push_back(fva);
								//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, fva);
								insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, fva);


								// cube
								Shape cube;
								cube.shape_id = SHAPE_ID::SAMPLER_CUBE;
								cube.is_first_pass = 0;
								//cube.is_directional = true;
								cube.use_shader = 1;
								cube.use_trns = 1;
								cube.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(TRS);
								cube.pos = fva.b;
								cube.axis[0] *= FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 0.155f;
								cube.axis[1] *= FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 0.155f;
								cube.axis[2] *= FACE_VECTOR_SCALE_FACTOR * obj_scale_factor * 0.155f;
								cube.color = s_dir_color_c; //glm::vec3(0.1f, 1.0f, 0.2f);
								cube.alpha = 0.65f;
								//m_Shapes2D_per_Viewport[vp_id].push_back(cube);
								//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, cube);
								insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, cube);

								cube.color = glm::vec3(0.0f);
								cube.alpha = 0.8f;
								cube.is_outline = 1;
								//m_Shapes2D_per_Viewport[vp_id].push_back(cube);
								//insert_Sampler_with_Depth_Order((VIEWPORT_ID)vp_id, cube);
								insert_Sampler_with_Depth_Order(samplers_shapes, (VIEWPORT_ID)vp_id, cube);
							}

						}

					}

				}
				



				// ------- //
				for (Shape s : samplers_shapes)
					m_Shapes2D_per_Viewport[vp.id].push_back(s);
				for(Shape s : other_shapes)
					m_Shapes2D_per_Viewport[vp.id].push_back(s);
				samplers_shapes.clear();
				other_shapes.clear();
				facevector_shapes.clear();
				// ------- //
			}


		}


		// gather data for each Scene Restriction
#define gather_data_per_Restriction
#ifdef gather_data_per_Restriction

		Restriction* UI_selected_restriction = 0;
		UI_selected_restriction   = Mediator::get_UI_Selected_Restriction();
		int ui_selected_unique_id = UI_selected_restriction != 0 ? UI_selected_restriction->get_Unique_Id() : -1;
		std::vector<Restriction*> restrictions = Mediator::Request_Scene_Restrictions();

		for (VIEWPORT vp : viewports)
		{
			bool is_vp_Dirty = vp.isDirty();
			int vp_id = vp.id;

			int r_restrictions   = 0;
			bool draw_r_details  = vp.render_settings.draw_restriction_details;
			bool draw_r_cones    = vp.render_settings.draw_restriction_cones;
			bool draw_r_fv       = vp.render_settings.draw_restriction_fv;
			bool draw_r_fv_lines = vp.render_settings.draw_restriction_fv_lines;
			bool draw_dist_area  = vp.render_settings.draw_restriction_dist_area;


			if (vp.render_settings.draw_restrictions_only_selected)                  r_restrictions = 1; // only selected
			else if (vp.render_settings.draw_restrictions_all)                       r_restrictions = 2; // all
			else if (vp.render_settings.draw_restrictions_only_face_vector_selected) r_restrictions = 3; // only fv selected


			PinholeCamera vp_cam  = ViewportManager::GetInstance().getViewportCamera(vp.id);
			float vp_scale_factor = vp_cam.getInvScaleFactor(vp);

			// - for each Restriction - //
			if (r_restrictions == 0 && UI_selected_restriction == 0)
				continue;

			for ( Restriction* R : restrictions )
			{
				
				bool is_valid = R->is_Active() && R->isEnabled() && R->is_Renderable() && !R->isDrawn();
				if (!is_valid)
					continue;

				bool is_Restr_Valid   = R->is_Valid();
				Restriction_ID r_id   = R->getId();
				int r_unique_id       = R->get_Unique_Id();
				bool is_R_ui_selected = UI_selected_restriction != 0 ? r_unique_id == ui_selected_unique_id : false;
				bool is_R_dirty       = R->isDirty(false);

				if (!is_R_ui_selected && r_restrictions == 0) continue;
				//if (!is_R_dirty) continue;

				SceneObject* subject = R->getSubject(); // from //
				SceneObject* object  = R->getObject();  //  to  //
				SceneObject* oA = R->get_Owner(0);
				SceneObject* oB = R->get_Owner(1);

				// not all
				if (r_restrictions != 2 && !is_R_ui_selected)
				{
					SceneObject* owner_A = R->get_Owner(0);
					SceneObject* owner_B = R->get_Owner(1);

					bool is_selected = false;
					if (r_restrictions == 1)
						is_selected = (owner_A->getId() == selected_id || owner_B->getId() == selected_id) || (subject->getId() == selected_id || object->getId() == selected_id);
					else if (r_restrictions == 3)
					{
						if      (r_id == FOCUS_R)        is_selected = subject->getId() == selected_id;
						else if (r_id == CONVERSATION_R) is_selected = (subject->getId() == selected_id || object->getId() == selected_id);
						else if (r_id == DISTANCE_R)     is_selected = (subject->getId() == selected_id || object->getId() == selected_id);
					}

					if (!is_selected)
						continue;
				}

				float from_to_line_width = 1.0f;
				int r_stripple_factor    = 10;//16;

				glm::mat4 subj_TRS = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(subject->getTransformationMatrix());
				glm::mat4 obj_TRS  = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(object->getTransformationMatrix());

				optix::float3 subj_pos     = subject->getTranslationAttributes();
				optix::float3 obj_pos      = object->getTranslationAttributes();
				optix::float3 obj_centroid = object->getCentroid_Transformed();

				float subj_dist         = vp_cam.getDistanceFrom(subj_pos);
				float obj_dist          = vp_cam.getDistanceFrom(obj_pos);
				float subj_scale_factor = LINKING_SCALE_FACTOR * subj_dist * vp_scale_factor;
				float obj_scale_factor  = LINKING_SCALE_FACTOR * obj_dist  * vp_scale_factor;

				bool stripped = true;
				// FOCUS : { Cone Frustum && Source - Target Line }
				if      (r_id == FOCUS_R)
				{
					optix::float3 from = subj_pos;
					optix::float3 to = obj_centroid;

					// line along facevector //
					if (draw_r_fv_lines && draw_r_details)
					{
						optix::float3 ip;
						bool isHit = R->get_HitPoint_Subject(ip);

						Shape line;
						line.shape_id = SHAPE_ID::LINE;
						line.model_matrix = subj_TRS;
						line.use_trns  = isHit? false : true;
						line.thickness = 1.0f;
						line.a = isHit? subject->getTranslationAttributes() : ZERO_3f;
						line.b = isHit? ip : ZERO_3f + GLOBAL_AXIS[2] * 9999.0f;

						line.draw_arrow = false;
						line.stripped = false;
						line.color = glm::vec3(0.7f);
						other_shapes.push_back(line);

						if (isHit)
						{
							Shape point;
							point.shape_id = S_POINT;
							point.scale_factor = 4.0f;
							point.a = ip;
							point.pos = ip;
							point.color = glm::vec3(0.2f);
							other_shapes.push_back(point);
						}
					}
					//

					// line from -> to //
					Shape arrow;
					arrow.shape_id = SHAPE_ID::LINE;
					arrow.scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp_scale_factor * 0.75f;
					arrow.thickness = from_to_line_width;
					arrow.arrow_scale_factor = 1.0f;
					arrow.stripple_factor = r_stripple_factor;
					arrow.a = from;
					arrow.b = to;
					arrow.draw_arrow = true;
					arrow.arrow_scale_factor = arrow.scale_factor;
					arrow.color = is_Restr_Valid ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
					arrow.stripped = stripped;
					other_shapes.push_back(arrow);
					//

					// cone frustum of vector a //
					if (draw_r_cones && draw_r_details)
					{
						Shape shape;
						shape.shape_id = CONE_FRUSTUM;
						shape.arrow_scale_factor = subj_scale_factor * 1.3f;
						shape.color = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(subject->get_FaceVectorParameters().color);
						shape.thickness = 1.0f;
						shape.is_restriction = true;
						shape.arrow_scale_factor = subj_scale_factor * 1.3f;
						shape.a = ZERO_3f; // cone's Origin
						shape.b = ZERO_3f + GLOBAL_AXIS[2] * FACE_VECTOR_SCALE_FACTOR * subj_scale_factor; // cone's Origin
						shape.length = FACE_VECTOR_SCALE_FACTOR * subj_scale_factor; // cone's Length along its normal axis //
						shape.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(subject->getTransformationMatrix()); // Face Vector's Transformation //
						shape.color = RES_CONE_FRUSTUM_COLOR;
						shape.fov = R->get_DeltaLimit();
						other_shapes.push_back(shape);
					}

					// face vector A //
					if (draw_r_fv && draw_r_details)
					{
						Shape fva;
						fva.shape_id = SHAPE_ID::LINE;
						fva.model_matrix = subj_TRS;
						fva.use_trns = true;
						fva.scale_factor = subj_scale_factor * FACE_VECTOR_ARROW_SCALE_FACTOR;
						fva.thickness = FACE_VECTOR_LINE_WIDTH;
						fva.gl_line_smooth = true;
						fva.arrow_scale_factor = 1.0f;
						fva.a = ZERO_3f;
						fva.b = ZERO_3f + GLOBAL_AXIS[2] * FACE_VECTOR_SCALE_FACTOR * subj_scale_factor;
						fva.draw_arrow = true;
						fva.arrow_scale_factor = fva.scale_factor;
						fva.color = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(subject->get_FaceVectorParameters().color);
						other_shapes.push_back(fva);
					}
					//

				}
				if      (r_id == CONVERSATION_R)			{

					bool is_dist_valid = false;
					Restriction_Conversation * R_conv = (Restriction_Conversation*)R;
					is_dist_valid = R_conv->is_dist_Valid();
					optix::float3 oA_c = oA->getCentroid_Transformed();
					optix::float3 oB_c = oB->getCentroid_Transformed();
					float dist = length(oA_c - oB_c);

					optix::float3 from = subj_pos;
					optix::float3 to   = obj_pos;


					// line along facevector A //
					if (draw_r_fv_lines && draw_r_details)
					{
						optix::float3 ip;
						bool isHit = R->get_HitPoint_Subject(ip);


						Shape linea;
						linea.shape_id = SHAPE_ID::LINE;
						linea.model_matrix = subj_TRS;
						linea.use_trns = isHit ? false : true;
						linea.thickness = 1.0f;
						linea.a = isHit ? subject->getTranslationAttributes() : ZERO_3f;
						linea.b = isHit ? ip : ZERO_3f + GLOBAL_AXIS[2] * 9999.0f;
						linea.draw_arrow = false;
						linea.color = glm::vec3(0.7f);
						other_shapes.push_back(linea);
						if (isHit)
						{
							Shape point;
							point.shape_id = S_POINT;
							point.scale_factor = 4.0f;
							point.a     = ip;
							point.pos   = ip;
							point.color = glm::vec3(0.2f);
							other_shapes.push_back(point);
						}
						//

						// line along facevector B //
						ip;
						isHit = R->get_HitPoint_Object(ip);

						Shape lineb;
						lineb.shape_id = SHAPE_ID::LINE;
						lineb.model_matrix = obj_TRS;
						lineb.use_trns = isHit ? false : true;
						lineb.thickness = 1.0f;
						lineb.a = isHit ? object->getTranslationAttributes() : ZERO_3f;
						lineb.b = isHit ? ip : ZERO_3f + GLOBAL_AXIS[2] * 9999.0f;
						lineb.draw_arrow = false;
						lineb.color = glm::vec3(0.7f);
						other_shapes.push_back(lineb);

						if (isHit)
						{
							Shape point;
							point.shape_id = S_POINT;
							point.scale_factor = 4.0f;
							point.a = ip;
							point.pos = ip;
							point.color = glm::vec3(0.2f);
							other_shapes.push_back(point);
						}
					}
					//

					//
					if (draw_r_cones && draw_r_details)
					{
						Shape shapea;
						shapea.shape_id = CONE_FRUSTUM;
						shapea.arrow_scale_factor = subj_scale_factor * 1.3f;
						shapea.color = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(subject->get_FaceVectorParameters().color);
						shapea.thickness = 1.0f;
						shapea.is_restriction = true;
						shapea.arrow_scale_factor = subj_scale_factor * 1.3f;
						shapea.a = ZERO_3f; // cone's Origin
						shapea.b = ZERO_3f + GLOBAL_AXIS[2] * FACE_VECTOR_SCALE_FACTOR * subj_scale_factor; // cone's Origin
						shapea.length = FACE_VECTOR_SCALE_FACTOR * subj_scale_factor; // cone's Length along its normal axis //
						shapea.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(subject->getTransformationMatrix()); // Face Vector's Transformation //
						shapea.fov = R->get_DeltaLimit();
						shapea.color = RES_CONE_FRUSTUM_COLOR;
						//m_Shapes2D_per_Viewport[vp.id].push_back(shapea);
						other_shapes.push_back(shapea);
					}
					// face vector A
					if (draw_r_fv && draw_r_details)
					{
						Shape fva;
						fva.shape_id = SHAPE_ID::LINE;
						fva.scale_factor = subj_scale_factor * FACE_VECTOR_ARROW_SCALE_FACTOR;
						fva.thickness = FACE_VECTOR_LINE_WIDTH;
						fva.gl_line_smooth = true;
						fva.model_matrix = subj_TRS;
						fva.use_trns = true;
						fva.arrow_scale_factor = 1.0f;
						fva.a = ZERO_3f;
						fva.b = ZERO_3f + GLOBAL_AXIS[2] * FACE_VECTOR_SCALE_FACTOR * subj_scale_factor;
						fva.draw_arrow = true;
						fva.arrow_scale_factor = fva.scale_factor;
						fva.color = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(subject->get_FaceVectorParameters().color);
						//m_Shapes2D_per_Viewport[vp.id].push_back(fva);
						other_shapes.push_back(fva);
					}



					//
					if (draw_r_cones && draw_r_details)
					{
						Shape shapeb;
						shapeb.shape_id = CONE_FRUSTUM;
						shapeb.arrow_scale_factor = obj_scale_factor * 1.3f;
						shapeb.color = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(object->get_FaceVectorParameters().color);
						shapeb.thickness = 1.0f;
						shapeb.is_restriction = true;
						shapeb.arrow_scale_factor = obj_scale_factor * 1.3f;
						shapeb.a = ZERO_3f; // cone's Origin
						shapeb.b = ZERO_3f + GLOBAL_AXIS[2] * FACE_VECTOR_SCALE_FACTOR * obj_scale_factor; // cone's Origin
						shapeb.length = FACE_VECTOR_SCALE_FACTOR * obj_scale_factor; // cone's Length along its normal axis //
						shapeb.model_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(object->getTransformationMatrix()); // Face Vector's Transformation //
						shapeb.fov = R->get_DeltaLimit_2();
						shapeb.color = RES_CONE_FRUSTUM_COLOR;
						//m_Shapes2D_per_Viewport[vp.id].push_back(shapeb);
						other_shapes.push_back(shapeb);
					}
					// face vector B//
					if (draw_r_fv && draw_r_details)
					{
						Shape fvb;
						fvb.shape_id = SHAPE_ID::LINE;
						fvb.scale_factor = obj_scale_factor * FACE_VECTOR_ARROW_SCALE_FACTOR;
						fvb.model_matrix = obj_TRS;
						fvb.use_trns = true;
						fvb.thickness = FACE_VECTOR_LINE_WIDTH;
						fvb.gl_line_smooth = true;
						fvb.arrow_scale_factor = 1.0f;
						fvb.a = ZERO_3f;
						fvb.b = ZERO_3f + GLOBAL_AXIS[2] * FACE_VECTOR_SCALE_FACTOR * obj_scale_factor;
						fvb.draw_arrow = true;
						fvb.arrow_scale_factor = fvb.scale_factor;
						fvb.color = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(object->get_FaceVectorParameters().color);
						//m_Shapes2D_per_Viewport[vp.id].push_back(fvb);
						other_shapes.push_back(fvb);
					}


					// line from -> to //
					{
						Shape arrow;
						arrow.shape_id = SHAPE_ID::LINE;
						arrow.scale_factor = LINKING_SCALE_FACTOR * obj_dist * vp_scale_factor * 0.75f;
						arrow.thickness = from_to_line_width;
						arrow.arrow_scale_factor = 1.0f;
						arrow.a = from;
						arrow.b = to;
						arrow.stripple_factor = r_stripple_factor;
						arrow.draw_arrow = true;
						arrow.arrow_scale_factor = arrow.scale_factor;
						arrow.color = is_Restr_Valid? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
						arrow.stripped = stripped;
						//m_Shapes2D_per_Viewport[vp.id].push_back(arrow);
						other_shapes.push_back(arrow);

						arrow.a = to;
						arrow.b = from;
						arrow.only_arrow = true;
						//m_Shapes2D_per_Viewport[vp.id].push_back(arrow);
						other_shapes.push_back(arrow);

						
						

					}
					//

				

				}
				if      (r_id == DISTANCE_R 
						 || r_id == CONVERSATION_R
						 )
				{

					bool is_circle = false;
					SceneObject* other = nullptr;
					SceneObject* THIS  = nullptr;
					object  = r_id == DISTANCE_R ? object  : oB;
					subject = r_id == DISTANCE_R ? subject : oA;
					if      (selected_id == object->getId())
					{
						other = subject;
						THIS  = object;
						is_circle = true;
					}
					else if (selected_id == subject->getId())
					{
						other = object;
						THIS  = subject;
						is_circle = true;
					}
					optix::float2 dist_limit = R->get_DistLimit();

					if (!is_circle)
					{
						THIS  = subject;
						other = object;
					}


					optix::float3 a = THIS->getCentroid_Transformed();
					optix::float3 b = other->getCentroid_Transformed();
					
					{
						optix::float3  p = b;
						optix::float3  s = optix::make_float3(dist_limit.y);

#ifdef distance_R_visualization_new

						// arc
						if (length(b - a) > R->get_DistLimit().y)
						{
							// Min distance from selected object //
							optix::float3 u   = normalize(a - b);
							PinholeCamera cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
							optix::float3 min_pos_oc = b + u * R->get_DistLimit().y;
							float size_factor = 0.045f * cam.getInvScaleFactor() * cam.getDistanceFrom(min_pos_oc);
							optix::float3 min_pos = min_pos_oc - u * size_factor;
							optix::float3 cam_w   = normalize(min_pos - cam.getCameraPos());
							optix::float3 v  = cross(u, GLOBAL_AXIS[1]);
							optix::float3 up = cross(u, v);

							Shape line;
							line.shape_id = LINE;
							line.a = min_pos_oc - size_factor * v;
							line.b = min_pos_oc + size_factor * v;
							line.draw_arrow = false;
							line.stripped   = 0;
							line.stripple_factor = 16.0f;
							line.color     = glm::vec3(1.0f);//R->is_Valid() ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
							line.thickness = 2.0f;
							line.use_trns  = false;
							//m_Shapes2D_per_Viewport[vp.id].push_back(line);
							other_shapes.push_back(line);


							// text //
							{
								float fact = is_circle ? 6.0f : 6.0f; // 4 : 2
								optix::float3 c = a;
								optix::float3 u = p - c;
								float l = length(u);
								u = u / l;


								//if (length(a - b) > R->get_DistLimit().y)
								{
									float dist = length(a - b);
									Shape text;
									text.shape_id = S_TEXT;
									text.a = min_pos_oc + up * 40.0f;
									c = b + normalize(a - b) * R->get_DistLimit().y;
									l = ( dist > R->get_DistLimit().y) ? length(a - c) : 0;
									std::string er_l = l > 0 ? "[ " + std::to_string(l) + " ]" : "";
									std::string label = er_l;
									text.label = label;
									text.font = FONT5;
									float t = dist / ((R->get_DistLimit().y+0.00001f) * 3.0f);
									if (t > 1) t = 1.0f;
									text.color = glm::mix( glm::vec3(0.8, 0.7, 0.7), glm::vec3(1, 0.2, 0.2), t ); //glm::vec3(1, 0.2, 0.2);
									other_shapes.push_back(text);

								}
							}
						}

						// sphere
						if (draw_r_details
							&& draw_dist_area
							&& is_circle
							)
						{

							bool is_valid = R->get_DistLimit().y >= length(b - a);
							Shape shape;
							shape.a = b;
							shape.b = a;
							shape.length   = R->get_DistLimit().y;
							shape.vertical = is_valid ? 0 : 1;
							shape.color    = is_valid ? glm::vec3(0,1,0) : glm::vec3(1,0.2,0.1);
							m_Shapes_perViewport_depth[vp.id].push_back(shape);

						}

						// line
						bool draw_line = r_id == CONVERSATION_R ? false : true;
						if ( draw_line )
						{
							float dist = length(a - b);
							bool is_valid = R->is_Valid();

							optix::float3 u = normalize(a - b);
							Shape line;
							line.shape_id = LINE;
							line.a = b;
							line.b = is_valid? a : b + u * R->get_DistLimit().y;
							line.draw_arrow = false;
							line.stripped   = false;//true;
							line.stripple_factor = 16.0f;
							line.color = glm::vec3(0, 1, 0);
							line.thickness = 1.0f;
							line.use_trns  = false;
							other_shapes.push_back(line);

							if (!is_valid)
							{

								line.a = b + u * R->get_DistLimit().y;
								line.b = a;
								line.stripped = true;//false;//true;
								line.stripple_factor = r_stripple_factor;
								line.color = glm::vec3(0, 1, 0); //R->is_Valid() ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
								line.thickness = 1.0f;
								line.use_trns  = false;

								line.mix_color = true;
								line.mix_p0 = line.a;
								line.mix_p1 = line.b;
								line.m_color_a = glm::vec3(0.8, 0.7, 0.7);
								line.m_color_b = glm::vec3(1, 0.0, 0.0);
								other_shapes.push_back(line);
								//glm::vec3(0.8, 0.7, 0.7), glm::vec3(1, 0.2, 0.2)
							}
							
						}

						// point
						{
							Shape point;
							point.shape_id = S_POINT;
							point.size = 1.0f;
							point.scale_factor = 5.0f;
							point.a = b;
							point.color = glm::vec3(1.0f);
							point.use_trns = false;
							//m_Shapes2D_per_Viewport[vp.id].push_back(point);
							other_shapes.push_back(point);

							point.a = a;
							//m_Shapes2D_per_Viewport[vp.id].push_back(point);
							other_shapes.push_back(point);
						}

						// text //
						{
							float fact = is_circle ? 6.0f : 6.0f; // 4 : 2
							optix::float3 c = a;
							optix::float3 u = p - c;
							float l = length(u);
							u = u / l;

							optix::float3 w = optix::cross(u, GLOBAL_AXIS[1]);
							optix::float3 v = optix::cross(u, w);

							//if (length(a - b) > R->get_DistLimit().y)
							{
								Shape text;
								text.shape_id = S_TEXT;
								text.a = c + u * l / fact - v * 20.0f;
								c = b + normalize(a - b) * R->get_DistLimit().y;
								std::string label = "[ " + std::to_string(length(a - b)) +" ] ";
								text.label = label; //std::to_string((l));
								text.font = FONT5;
								other_shapes.push_back(text);


							}
						}

#endif
					}

				}


			}
			for ( Restriction* R : restrictions )
			{
				R->setDrawn(false);
			}

			// - - - - - - - - - - - - //


			// ------- //
			for (Shape s : other_shapes)
				m_Shapes2D_per_Viewport[vp.id].push_back(s);
			samplers_shapes.clear();
			other_shapes.clear();
			facevector_shapes.clear();
			// ------- //
		}
		
#endif
		


	}


}

void OpenGLRenderer::insert_Sampler_with_Depth_Order(VIEWPORT_ID vp_id, Shape& shape)
{
	optix::float3 cam_pos = ViewportManager::GetInstance().getViewportCamera(vp_id).getCameraPos();
	optix::float3 pos = shape.pos;
	float shape_dist_from_cam = length(pos - cam_pos);
	
	int i = -1;
	int j = -1;
	int k = 0;
	for (Shape sh : m_Shapes2D_per_Viewport[vp_id])
	{
		SHAPE_ID sh_id = sh.shape_id;
		if (sh_id != SAMPLER_CUBE && sh_id != PLANE){ k++; continue; }

		optix::float3 pos = sh.pos;
		float dist_from_camera = length(pos - cam_pos);

		// need to swap positions //
		if (dist_from_camera < shape_dist_from_cam)
		{
			i = k;
			break;
		}

		k++;
	}

	m_Shapes2D_per_Viewport[vp_id].push_back(shape);
	int last = m_Shapes2D_per_Viewport[vp_id].size() - 1;

	if (i != -1) std::swap(m_Shapes2D_per_Viewport[vp_id][i], m_Shapes2D_per_Viewport[vp_id][last]);

}
void OpenGLRenderer::insert_Sampler_with_Depth_Order(std::vector<Shape>& sampler_shapes, VIEWPORT_ID vp_id,Shape& shape)
{
	optix::float3 cam_pos = ViewportManager::GetInstance().getViewportCamera(vp_id).getCameraPos();
	optix::float3 pos = shape.pos;
	float shape_dist_from_cam = length(pos - cam_pos);

	int i = -1;
	int j = -1;
	int k = 0;
	for (Shape sh : sampler_shapes)
	{
		SHAPE_ID sh_id = sh.shape_id;
		if (sh_id != SAMPLER_CUBE && sh_id != PLANE) { k++; continue; }

		optix::float3 pos = sh.pos;
		float dist_from_camera = length(pos - cam_pos);

		// need to swap positions //
		if (dist_from_camera < shape_dist_from_cam)
		{
			i = k;
			break;
		}

		k++;
	}

	sampler_shapes.push_back(shape);
	int last = sampler_shapes.size() - 1;

	if (i != -1) std::swap(sampler_shapes[i], sampler_shapes[last]);

}
void OpenGLRenderer::depth_Order_Sampler_Shapes()
{
	
}

void OpenGLRenderer::PRE_STATE_APPLY( STATE state )
{

}
void OpenGLRenderer::POST_STATE_APPLY( STATE state )
{

}

void OpenGLRenderer::PRE_Viewport_Render_APPLY(const VIEWPORT& vp)
{
	//Utilities::scale_Light_Objects_Relative_To_Viewport( vp );
}
void OpenGLRenderer::POST_Viewport_Render_APPLY(const VIEWPORT& vp)
{
	//Utilities::reset_Light_Objects_Scale();
}

void OpenGLRenderer::Clear_Fbo(GLuint fbo, bool clear_color, glm::vec4 color)
{

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);


	if (clear_color)
		glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void OpenGLRenderer::reset_Viewport()
{
	glViewport(0, 0, 0, 0);
}



void OpenGLRenderer::add_Shape2D_To_RenderList(Shape shape, SHAPE_ID shape_id, struct UtilityProperties* utility)
{
	Shape_RenderList_Object obj;
	obj.shape    = shape;
	obj.shape_id = shape_id;
	if( utility != 0 )
		obj.ut_id    = utility->id;
	m_shapes2D_RenderList.push_back(obj);
}
void OpenGLRenderer::add_SceneObject_To_Object_RenderList(Object_Package* obj_pack, PinholeCamera* cam)
{
	SceneObject_RenderList_Object obj;
	obj.obj_pack        = obj_pack;
	obj.camera_instance = cam;
	m_SceneObject_RenderList.push_back(obj);
}
void OpenGLRenderer::add_Utility_To_RenderList( UtilityProperties * utility,Object_Package* subject_pack, Object_Package * obj_pack, PinholeCamera * camera_instance)
{

	Utility_RenderList_Object obj;
	obj.ut              = *utility;
	obj.obj_pack        = subject_pack;
	obj.obj2_pack       = obj_pack;
	obj.camera_instance = camera_instance;
	m_Utility_RenderList.push_back(obj);

}
void OpenGLRenderer::add_Utility_To_RenderList( Object_Package* obj_pack, UTIL_RENDER_TYPE render_type, PinholeCamera* camera_instance)
{
	Utility_RenderList_Object obj;
	UtilityProperties ut;
	ut.group = g_NONE;
	ut.id    = u_NONE;
	obj.ut = ut;
	obj.render_type = render_type;
	obj.obj_pack = obj_pack;
	obj.camera_instance = camera_instance;
	m_Utility_RenderList.push_back( obj );
}
void OpenGLRenderer::add_Object_To_HighlightList(Object_Package * obj_pack, PinholeCamera * camera_instance)
{

	Highlight_RenderList_Object obj;
	obj.obj_pack        = *obj_pack;
	obj.camera_instance = camera_instance;
	m_Highlight_RenderList.push_back( obj );

}
void OpenGLRenderer::add_Object_To_SceneRenderList(PinholeCamera camm, SceneObject* focus_obj, int width, int height, int type, int index)
{
	Scene_RenderList_Object scene_obj;// = Scene_RenderList_Object();

	PinholeCamera* cam = new PinholeCamera();
	cam->setViewport(m_screen_width / 2.0f, m_screen_height / 2.0f);
	
	optix::float3 cameraPosition;
	optix::float3 cameraU;
	optix::float3 cameraV;
	optix::float3 cameraW;
	cam->getFrustum(cameraPosition, cameraU, cameraV, cameraW);
	
	//scene_obj.camera_instance = camera;
	scene_obj.camera          = cam;
	scene_obj.focused_object  = focus_obj;
	scene_obj.scene_width     = width;
	scene_obj.scene_height    = height;
	scene_obj.render_type     = type;
	scene_obj.index           = index;

	if (m_scene_RenderList.size() > index)
	{
		m_scene_RenderList[index] = scene_obj;
	}
	else
	{
		m_scene_RenderList.push_back(scene_obj);
	}
}


void OpenGLRenderer::remove_Scene(int index)
{
	m_scene_RenderList.erase(m_scene_RenderList.begin() + index);
}
void OpenGLRenderer::remove_Scenes()
{
	m_scene_RenderList.clear();
}

void OpenGLRenderer::set_Cursor_To_Render(CURSOR_TYPE cursor_type )
{

	m_render_cursor = 0;
	m_cursor_to_render = cursor_type;
	if (cursor_type != CURSOR_TYPE::CURSOR_NONE)
		m_render_cursor = 1;
}

void OpenGLRenderer::clear_geometry_stored_objects()
{
	for (auto& it : m_geometry_stored_objects)
	{
		destroy_GeometryObject_OpenGL(it.second);
	}

	m_geometry_stored_objects.clear();
}
void OpenGLRenderer::destroy_GeometryObject_OpenGL(GeometryObject_OpenGL * obj)
{
	glDeleteVertexArrays(1, &obj->m_vao);
	glDeleteBuffers(1, &obj->m_vbo);
	glDeleteBuffers(1, &obj->m_ibo);
}

// transfer this function to Utilities::
void OpenGLRenderer::createGeometryObject_OpenGL_FromSceneObject(int scene_object_index)
{

	
}
void OpenGLRenderer::createGeometryObject_OpenGL_FromSceneObject(SceneObject* object)
{

}

void OpenGLRenderer::Render_GeometryNode_Arrays(class SceneObject* obj, class GeometryNode* node, glm::mat4& model_matrix, ShaderProgram& program, glm::vec3 diffuse, GLenum gl_type )
{
	glBindVertexArray(node->m_vao);
	glUniformMatrix4fv(program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniformMatrix4fv(program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(model_matrix))));

	{
		MaterialParameters mat_params;
		Light_Parameters light_params;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		bool has_tex = false;
		if (obj->getType() != LIGHT_OBJECT)
		{
			diffuseColor = diffuse;
			shininess = 0.1f;
		}
		else
		{
			diffuseColor = diffuse;
			specularColor = glm::vec3(1.0f);
			shininess = 0.01f;
			glUniform1i(program["uniform_rendering_utility"], 1);
		}

		glUniform3f(program["uniform_diffuse"], diffuseColor.x, diffuseColor.y, diffuseColor.z);
		glUniform3f(program["uniform_specular"], specularColor.x, specularColor.y, specularColor.z);
		glUniform1f(program["uniform_shininess"], shininess);
		glUniform1f(program["uniform_has_texture"], 0.0f);


	}

	for (int j = 0; j < node->parts.size(); j++)
	{
		glDrawArrays
		(
			gl_type,
			node->parts[j].start_offset,
			node->parts[j].count
		);

	}

	glUniform1i(program["uniform_rendering_utility"], 0);

	glBindVertexArray(0);
}
void OpenGLRenderer::Render_GeometryNode_Arrays(class SceneObject* obj, class GeometryNode* node, glm::mat4& model_matrix, ShaderProgram& program, GLenum gl_type)
{

	glBindVertexArray(node->m_vao);
	glUniformMatrix4fv(program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniformMatrix4fv(program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(model_matrix))));

	{
		MaterialParameters mat_params;
		Light_Parameters light_params;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		bool has_tex = false;
		if (obj->getType() != LIGHT_OBJECT)
		{
			mat_params = obj->getMaterialParams(0);
			diffuseColor = glm::vec3(mat_params.Kd[0], mat_params.Kd[1], mat_params.Kd[2]);
			specularColor = glm::vec3(mat_params.Ks[0], mat_params.Ks[1], mat_params.Ks[2]);
			shininess = 0.1f;

		}
		else
		{

			//light_params = obj->get_LightParameters();
			diffuseColor = glm::vec3(1.0f, 1.0f, 0.6f);
			specularColor = glm::vec3(1.0f);
			shininess = 0.01f;
			glUniform1i(program["uniform_rendering_utility"], 1);

		}


		glUniform3f(program["uniform_diffuse"], diffuseColor.x, diffuseColor.y, diffuseColor.z);
		glUniform3f(program["uniform_specular"], specularColor.x, specularColor.y, specularColor.z);
		glUniform1f(program["uniform_shininess"], shininess);
		glUniform1f(program["uniform_has_texture"], 0.0f);


	}

	for (int j = 0; j < node->parts.size(); j++)
	{
		glDrawArrays
		(
			gl_type,
			node->parts[j].start_offset,
			node->parts[j].count
		);

	}

	glUniform1i(program["uniform_rendering_utility"], 0);

	glBindVertexArray(0);
}
void OpenGLRenderer::Render_GeometryNode(SceneObject* obj, GeometryNode* node, glm::mat4& model_matrix, ShaderProgram& program, bool only_geometry)
{


	//Render_GeometryObject(obj, model_matrix, program, only_geometry);
	//return;

	

#define Indirect_Draw_Test
#ifdef Indirect_Draw_Test



#ifdef GET_RENDER_TIME_METRICS
	float t_start = g_timer.getTime();
#endif

	//glBindVertexArray( node->m_vao );
	

#ifdef GET_RENDER_TIME_METRICS

	global_state_change_counter++;
	float t_end = g_timer.getTime();
	float time_span = t_end - t_start;
	global_avg_State_Change_Time += time_span / global_state_change_counter;

#endif

	glUniformMatrix4fv(program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	
	if ( !only_geometry )
	{
		glUniformMatrix4fv(program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(model_matrix))));
		//glBindTexture(GL_TEXTURE_2D_ARRAY, node->m_materials[obj->getId()]);
		node->bind_Material(obj->getId());
	}
	

	node->bind_Vao();
	node->bind_MDIBuffer();
	
	node->Draw();
	
	node->unbind_Vao();
	node->unbind_MDIBuffer();
	node->unbind_Material();
	
	
	return;
#endif






	glBindVertexArray(node->m_vao);
	glUniformMatrix4fv(program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	if (!only_geometry)
		glUniformMatrix4fv(program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(model_matrix))));

	if (!only_geometry)
	{
		MaterialParameters mat_params;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		bool has_tex = false;
		{
			mat_params = obj->getMaterialParams(0);
			diffuseColor = glm::vec3(mat_params.Kd[0], mat_params.Kd[1], mat_params.Kd[2]);
			specularColor = glm::vec3(mat_params.Ks[0], mat_params.Ks[1], mat_params.Ks[2]);
			shininess = 0.01f;
		}


		glUniform3f(program["uniform_diffuse"], diffuseColor.x, diffuseColor.y, diffuseColor.z);
		glUniform3f(program["uniform_specular"], specularColor.x, specularColor.y, specularColor.z);
		glUniform1f(program["uniform_shininess"], shininess);
		glUniform1f(program["uniform_has_texture"], 0.0f );
		//glBindTexture(GL_TEXTURE_2D, node->parts[j].textureID);

	}

	for (int j = 0; j < node->parts.size(); j++)
	{
		glDrawArrays
		(
			GL_TRIANGLES,
			node->parts[j].start_offset,
			node->parts[j].count
		);



#ifdef GL_DRAW_ELEMENTS

		glBindVertexArray(node->parts[j].m_vao);

		glDrawElements(GL_TRIANGLES,
					   node->parts[j].count,
					   GL_UNSIGNED_INT,
					   0);

		glBindVertexArray(0);

#endif

	}
	
	glBindVertexArray(0);

}
void OpenGLRenderer::Render_GeometryObject(SceneObject* obj, glm::mat4& model_matrix, ShaderProgram& program, bool only_geometry)
{



	GeometryObject_OpenGL* mesh = Mesh_Pool::GetInstance().request_Mesh_for_OpenGL2(obj);
	glBindVertexArray(mesh->m_vao);

	glUniformMatrix4fv(program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	if (!only_geometry)
	{
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;

		MaterialParameters mat_params;
		{
			mat_params    = obj->getMaterialParams(0);
			diffuseColor  = glm::vec3(mat_params.Kd[0], mat_params.Kd[1], mat_params.Kd[2]);
			specularColor = glm::vec3(mat_params.Ks[0], mat_params.Ks[1], mat_params.Ks[2]);
			shininess = 0.01f;
		}
		

		glUniformMatrix4fv(program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(model_matrix))));
		glUniform3f(program["uniform_diffuse"], diffuseColor.x, diffuseColor.y, diffuseColor.z);
		glUniform3f(program["uniform_specular"], specularColor.x, specularColor.y, specularColor.z);
		glUniform1f(program["uniform_shininess"], shininess);

		glUniform1f(program["uniform_has_texture"], 0.0f );

	}

	glDrawElements(GL_TRIANGLES,
				   mesh->num_indices,
				   GL_UNSIGNED_INT,
				   0);

	glBindVertexArray(0);




}

#endif

#ifdef UTILITY_RENDERER_SET_FUNCTIONS

void OpenGLRenderer::setUp_RenderingViewport()
{
	VIEWPORT vp = ViewportManager::GetInstance().getActiveOptiXViewport();
	glViewport(0, 0, vp.renderSize.x, vp.renderSize.y);
}
void OpenGLRenderer::setUp_RenderingViewport(VIEWPORT vp)
{
	if( vp.is_dynamically_scaled )
		glViewport(0, 0, vp.renderSize_dynamic.x, vp.renderSize_dynamic.y);
	else
		glViewport(0, 0, vp.renderSize.x , vp.renderSize.y );
}
void OpenGLRenderer::setUp_OutputViewport()
{
	VIEWPORT vp = ViewportManager::GetInstance().getActiveOptiXViewport();
	glViewport(
		vp.dimensions.offset.x,
		vp.dimensions.offset.y,
		vp.dimensions.size.x, 
		vp.dimensions.size.y);

	// vp.renderSize.x,
	// vp.renderSize.y
}
void OpenGLRenderer::setUp_OutputViewport(VIEWPORT vp)
{
	
	glViewport(
		vp.dimensions.offset.x,
		vp.dimensions.offset.y,
		vp.dimensions.size.x,
		vp.dimensions.size.y);
	
}

void OpenGLRenderer::setRenderSize(int width, int height)
{
	m_renderSize.x = width;
	m_renderSize.y = height;
	ResizeBuffers(m_renderSize.x, m_renderSize.y);
}

#endif

#ifdef UTILITY_RENDERER_GET_FUNCTIONS


void OpenGLRenderer::Transform_World_To_Screen(optix::float3 p, int vp_id , float& screen_x, float& screen_y)
{

	VIEWPORT vp = ViewportManager::GetInstance().getViewport((VIEWPORT_ID)vp_id);
	float window_width  = Mediator::RequestWindowSize(0);
	float window_height = Mediator::RequestWindowSize(1);

	glm::vec2 vp_dim = glm::vec2(vp.renderSize.x, vp.renderSize.y);
	glm::vec2 vp_off = glm::vec2(vp.dimensions.offset.x, vp.dimensions.offset.y);

	glm::vec3 p_world = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p);

	PinholeCamera camera = ViewportManager::GetInstance().getViewportCamera(vp.id);
	optix::float3 cam_pos, camera_up, w;
	optix::float3 center = camera.getOrbitCenter();
	cam_pos   = camera.getCameraPos();
	camera_up = camera.getV();
	w         = camera.getW();
	glm::vec3 pos    = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
	glm::vec3 up     = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
	glm::vec3 target = pos + VECTOR_TYPE_TRANSFORM::TO_GLM_3f(normalize(w)) * 10.0f;
	
	// construct view matrix
	glm::mat4 view_matrix = glm::lookAt(pos, target, up);

	// construct projection matrix
	glm::mat4 proj_matrix = glm::perspective(glm::radians(M_FOV), vp.renderSize.x / vp.renderSize.y, NEAR, FAR);

	// projected point
	glm::vec4 proj_p = proj_matrix * (view_matrix * glm::vec4(p_world, 1.0f));

	// Normalized Device Coordinates : [ -1, 1 ]
	glm::vec3 ndc_p = glm::vec3(proj_p.x, proj_p.y, proj_p.z) * (1 / proj_p.w);

	// Window Coordinates
	glm::vec2 win_p = ((glm::vec2(ndc_p.x, ndc_p.y) + glm::vec2(1.0f)) / 2.0f) * vp_dim + vp_off;
	
	// Viewport Coordinates
	glm::vec2 vp_p = win_p;
	//vp_p.x = win_p.x * vp_dim.x + vp_off.x;
	//vp_p.y = win_p.y * vp_dim.y + vp_off.y
	
	//vp_p.x = win_p.x * window_width;
	//vp_p.y = win_p.y * window_height;
	
	//optix::float2 vpp = Utilities::getMousePosRelativeToViewport((VIEWPORT_ID)vp_id, optix::make_float2(vp_p.x, vp_p.y));


	
	screen_x = vp_p.x;
	screen_y = vp_p.y;

	//screen_x = vpp.x;
	//screen_y = vpp.y;

}

const optix::float2& OpenGLRenderer::getRenderSize()
{
	return m_renderSize;
}
ShaderProgram *      OpenGLRenderer::getShaderProgram()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_OpenGL_Renderer) 
	streamHandler::Report("OpenGL_Renderer", "getShaderProgram", "");
#endif

	return &m_utility_rendering_program;
}
GLuint               OpenGLRenderer::getSecondarySceneTex(int index)
{
	
	return 0;
	
}
bool                 OpenGLRenderer::checkIntegrity()
{
	bool is_ok = true;
	for (auto& it : m_geometry_stored_objects)
	{
		/*
		GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ibo;
	GLuint m_vbo_normals;
		*/

		bool vao_ok     = it.second->m_vao != 0;
		bool vbo_ok     = it.second->m_vbo != 0;
		bool ibo_ok     = it.second->m_ibo != 0;
		bool normals_ok = it.second->m_vbo_normals != 0;
		if (!vao_ok || !vbo_ok || !ibo_ok || !normals_ok)
		{
			//std::cout << " \nOpenGLRenderer::checkIntegrity():" << std::endl;
			//std::cout << "       - m_geometry_stored_object[" << it.first << "]:" << std::endl;
			//std::cout << "       - m_vao_ok     = "<< vao_ok << std::endl;
			//std::cout << "       - m_vbo_ok     = "<< vbo_ok << std::endl;
			//std::cout << "       - m_ibo_ok     = "<< ibo_ok << std::endl;
			//std::cout << "       - m_normals_ok = "<< normals_ok << std::endl;
			is_ok = false;
		}
	}

	return is_ok;
}

#endif




