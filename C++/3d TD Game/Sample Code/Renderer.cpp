#include "Renderer.h"
#include "Tools.h"
#include "TextureManager.h"
#include <algorithm>
#include "OBJLoader.h"
#include "Game.h"
#include "Game_objects.h"
#include "Defines.h"



#define M_PI 3.1415926535897932f

// RENDERER
Renderer::Renderer(class Game * game)
{

	this->game = game;

	m_linear_sampler = 0;
	m_nearest_sampler = 0;
	m_trilinear_sampler = 0;

	for (int i = 0; i < 3; i++)
		m_wrapping_samplers[i] = 0;

	m_vbo_fbo_vertices = 0;
	m_vao_fbo = 0;

	m_fbo = 0;
	m_shadowmap_blur_fbo = 0;
	m_fbo_texture = 0;

	m_shadowmap_blur_texture = 0;

	m_quad_vao = 0;
	m_quad_normals = 0;
	m_quad_texcoord = 0;
	m_quad_texture = 0;



	m_rendering_mode = RENDERING_MODE::TRIANGLES;
	m_continous_time = 0.0;


	m_shading_mode = SHADING_MODE::PCF;

	// initialize the camera parameters
	/*
	m_camera_position = glm::vec3(0.720552, 18.1377, -11.3135);
	m_camera_target_position = glm::vec3(4.005, 12.634, -5.66336);
	*/

	/* version 1*/
	m_camera_position = glm::vec3(16.4926, 27.3952, -13.6138);
	m_camera_target_position = glm::vec3(16.5256, 20.8648, -8.10423);
	m_camera_up_vector = glm::vec3(0, 1, 0);

	for (int i = 0; i < 4; i++)
		camera_moving_vars[i] = 0;
}

Renderer::~Renderer()
{

	//delete samplers
	glDeleteSamplers(1, &m_nearest_sampler);
	glDeleteSamplers(1, &m_linear_sampler);
	glDeleteSamplers(1, &m_trilinear_sampler);
	glDeleteSamplers(3, m_wrapping_samplers);


	// delete g_buffer
	glDeleteTextures(1, &m_fbo_texture);
	glDeleteTextures(1, &m_fbo_depth_texture);
	glDeleteTextures(1, &m_shadowmap_blur_texture);

	glDeleteFramebuffers(1, &m_fbo);
	glDeleteFramebuffers(1, &m_shadowmap_blur_fbo);

	// delete common data
	glDeleteVertexArrays(1, &m_vao_fbo);

	glDeleteBuffers(1, &m_vbo_fbo_vertices);


}

bool Renderer::Init(int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
	this->m_screen_width = SCREEN_WIDTH;
	this->m_screen_height = SCREEN_HEIGHT;


	// Initialize OpenGL functions

	//Set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//This enables depth and stencil testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	// glClearDepth sets the value the depth buffer is set at, when we clear it

	// Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);


	// open the viewport
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); //we set up our viewport

	bool techniques_initialization = InitRenderingTechniques();
	bool buffers_initialization = InitDeferredShaderBuffers();
	bool items_initialization = InitCommonItems();
	bool lights_sources_initialization = InitLightSources();
	bool meshes_initialization = InitGeometricMeshes();

	//If there was any errors
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		return false;
	}

	InitMatrices();

	//If everything initialized
	return techniques_initialization && items_initialization && buffers_initialization && meshes_initialization && lights_sources_initialization
		&& generateSamplers();
}

bool Renderer::generateSamplers()
{

	//GLenum wrap_mode = GL_REPEAT;
	//GLenum wrap_mode = GL_CLAMP_TO_BORDER;
	GLenum wrap_mode = GL_CLAMP_TO_EDGE;
	GLint max_anisotropy = 1;
	glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);

	// Generate Samplers
	glGenSamplers(1, &m_nearest_sampler);
	glSamplerParameteri(m_nearest_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(m_nearest_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(m_nearest_sampler, GL_TEXTURE_WRAP_S, wrap_mode);
	glSamplerParameteri(m_nearest_sampler, GL_TEXTURE_WRAP_T, wrap_mode);

	glGenSamplers(1, &m_linear_sampler);
	glSamplerParameteri(m_linear_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(m_linear_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_linear_sampler, GL_TEXTURE_WRAP_S, wrap_mode);
	glSamplerParameteri(m_linear_sampler, GL_TEXTURE_WRAP_T, wrap_mode);


	glGenSamplers(1, &m_trilinear_sampler);
	glSamplerParameteri(m_trilinear_sampler, GL_TEXTURE_WRAP_S, wrap_mode);
	glSamplerParameteri(m_trilinear_sampler, GL_TEXTURE_WRAP_T, wrap_mode);
	glSamplerParameteri(m_trilinear_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(m_trilinear_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_trilinear_sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);


	// Wrapping samplers
	glGenSamplers(3, m_wrapping_samplers);
	glSamplerParameteri(m_wrapping_samplers[0], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(m_wrapping_samplers[0], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(m_wrapping_samplers[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(m_wrapping_samplers[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_wrapping_samplers[0], GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);
	glSamplerParameteri(m_wrapping_samplers[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_wrapping_samplers[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_wrapping_samplers[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(m_wrapping_samplers[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_wrapping_samplers[1], GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);
	glSamplerParameteri(m_wrapping_samplers[2], GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(m_wrapping_samplers[2], GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(m_wrapping_samplers[2], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(m_wrapping_samplers[2], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_wrapping_samplers[2], GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);

	return true;
}

bool Renderer::InitCommonItems()
{
	GLfloat fbo_vertices[] = {
			-1, -1, 0,
			1, -1, 0,
			-1, 1, 0,
			1, 1, 0
	};

	/* fbo */
	{
		glGenVertexArrays(1, &m_vao_fbo);
		glBindVertexArray(m_vao_fbo);



		glGenBuffers(1, &m_vbo_fbo_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_fbo_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}


	/* terrain-wall quad */
	{
		glGenVertexArrays(1, &m_quad_vao);
		glBindVertexArray(m_quad_vao);


		glGenBuffers(1, &m_quad_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, m_quad_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		GLfloat normals[] =
		{
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0
		};


		glGenBuffers(1, &m_quad_normals);
		glBindBuffer(GL_ARRAY_BUFFER, m_quad_normals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);


		GLfloat quad_texcoord[] = {
			0, 0,
			8, 0,
			0, 8,
			8, 8
		};

		glGenBuffers(1, &m_quad_texcoord);
		glBindBuffer(GL_ARRAY_BUFFER, m_quad_texcoord);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_texcoord), quad_texcoord, GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}


	/* gui quad */
	{
		glGenVertexArrays(1, &m_quad_gui_vao);
		glBindVertexArray(m_quad_gui_vao);

		//glGenBuffers(1, &m_quad_gui_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, m_quad_vertices);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		GLfloat quad_texcoord2[] = {
			0, 0,
			1, 0,
			0, 1,
			1, 1
		};

		glGenBuffers(1, &m_quad_gui_texcoord);
		glBindBuffer(GL_ARRAY_BUFFER, m_quad_gui_texcoord);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_texcoord2), quad_texcoord2, GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}


	/* cube */
	{
		glm::vec3 v1(-1.00000, -1.00000, 1.00000);
		glm::vec3 v2(1.00000, -1.00000, 1.00000);
		glm::vec3 v3(-1.00000, 1.00000, 1.00000);
		glm::vec3 v4(1.00000, 1.00000, 1.00000);
		glm::vec3 v5(-1.00000, 1.00000, -1.00000);
		glm::vec3 v6(1.00000, 1.00000, -1.00000);
		glm::vec3 v7(-1.00000, -1.00000, -1.00000);
		glm::vec3 v8(1.00000, -1.00000, -1.00000);

		std::vector<glm::vec3> cube_vertices = {

			v1 , v2 , v3,
			v3 , v2 , v4,

			v3 , v4 , v5,
			v5 , v4 , v6,

			v5 , v6 , v7,
			v7 , v6 , v8,

			v7 , v8 , v1,
			v1 , v8 , v2,

			v2 , v8 , v4,
			v4 , v8 , v6,

			v7 , v1 , v5,
			v5 , v1 , v3

		};


		glGenVertexArrays(1, &m_cube_vao);
		glBindVertexArray(m_cube_vao);

		glGenBuffers(1, &m_cube_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*cube_vertices.size(), &cube_vertices[0][0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);

		glm::vec3 n1(0.000000, 0.000000, 1.000000);
		glm::vec3 n2(0.000000, 1.000000, 0.000000);
		glm::vec3 n3(0.000000, 0.000000, -1.000000);
		glm::vec3 n4(0.000000, -1.000000, 0.000000);
		glm::vec3 n5(1.000000, 0.000000, 0.000000);
		glm::vec3 n6(-1.000000, 0.000000, 0.000000);

		std::vector<glm::vec3> normals =
		{
			n1 , n1 , n1,
			n1 , n1 , n1,

			n2 , n2 , n2,
			n2 , n2 , n2,

			n3 , n3 , n3,
			n3 , n3 , n3,

			n4 , n4 , n4,
			n4 , n4 , n4,

			n5 , n5 , n5,
			n5 , n5 , n5,

			n6 , n6 , n6,
			n6 , n6 , n6

		};

		glGenBuffers(1, &m_cube_normals);
		glBindBuffer(GL_ARRAY_BUFFER, m_cube_normals);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*normals.size(), &normals[0][0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);

		float k = 1.0f;
		glm::vec2 t1(0.000000, 0.000000);
		glm::vec2 t2(k * 1.000000, 0.000000);
		glm::vec2 t3(0.000000, k * 1.000000);
		glm::vec2 t4(k * 1.000000, k * 1.000000);
		std::vector<glm::vec2> tex_coords = {

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4

		};


		glGenBuffers(1, &m_cube_texcoords);
		glBindBuffer(GL_ARRAY_BUFFER, m_cube_texcoords);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*tex_coords.size(), &tex_coords[0][0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);

		//
		k = 8.0f;
		std::vector<glm::vec2> tex_coords2 = {

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4,

			t1,t2,t3,
			t3,t2,t4

		};
		for (int i = 0; i < tex_coords2.size(); i++)
		{
			tex_coords2[i] *= k;
		}

		glGenBuffers(1, &m_cube_texcoords2);
		glBindBuffer(GL_ARRAY_BUFFER, m_cube_texcoords2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*tex_coords2.size(), &tex_coords2[0][0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(
			3,
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			0
		);

	}


	glBindVertexArray(0);
	return true;
}

bool Renderer::InitRenderingTechniques()
{
	bool initialized = true;

	std::string vertex_shader_path;
	std::string fragment_shader_path;

	// Geometry Rendering Program
	{
		vertex_shader_path = "../Data/Shaders/basic_rendering.vert";
		fragment_shader_path = "../Data/Shaders/basic_rendering.frag";
		m_geometry_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
		m_geometry_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
		initialized = m_geometry_rendering_program.CreateProgram();
		m_geometry_rendering_program.LoadUniform("uniform_projection_matrix");
		m_geometry_rendering_program.LoadUniform("uniform_view_matrix");
		m_geometry_rendering_program.LoadUniform("uniform_model_matrix");
		m_geometry_rendering_program.LoadUniform("uniform_normal_matrix");
		m_geometry_rendering_program.LoadUniform("uniform_diffuse");
		m_geometry_rendering_program.LoadUniform("uniform_specular");
		m_geometry_rendering_program.LoadUniform("uniform_shininess");
		m_geometry_rendering_program.LoadUniform("uniform_has_texture");
		m_geometry_rendering_program.LoadUniform("uniform_damage_texture");
		m_geometry_rendering_program.LoadUniform("uniform_life_time");
		m_geometry_rendering_program.LoadUniform("uniform_dizzy_state");
		m_geometry_rendering_program.LoadUniform("uniform_coin_collection");
		m_geometry_rendering_program.LoadUniform("diffuse_texture");
		m_geometry_rendering_program.LoadUniform("uniform_camera_position");
		m_geometry_rendering_program.LoadUniform("uniform_focused");
		m_geometry_rendering_program.LoadUniform("uniform_wall");
		m_geometry_rendering_program.LoadUniform("uniform_shading_mode");
		m_geometry_rendering_program.LoadUniform("uniform_testing_mode");
		m_geometry_rendering_program.LoadUniform("uniform_shadow_proj_matrix");
		m_geometry_rendering_program.LoadUniform("uniform_shadow_view_matrix");
		m_geometry_rendering_program.LoadUniform("uniform_cannon_ball ");
		m_geometry_rendering_program.LoadUniform("uniform_damage_value");
		m_geometry_rendering_program.LoadUniform("uniform_meteorite");

	}

	// Light Source Uniforms
	{
		m_geometry_rendering_program.LoadUniform("uniform_light_projection_matrix");
		m_geometry_rendering_program.LoadUniform("uniform_light_view_matrix");
		m_geometry_rendering_program.LoadUniform("uniform_light_position");
		m_geometry_rendering_program.LoadUniform("uniform_light_direction");
		m_geometry_rendering_program.LoadUniform("uniform_light_color");
		m_geometry_rendering_program.LoadUniform("uniform_light_umbra");
		m_geometry_rendering_program.LoadUniform("uniform_light_penumbra");
		m_geometry_rendering_program.LoadUniform("uniform_cast_shadows");
		m_geometry_rendering_program.LoadUniform("shadowmap_texture");
		m_geometry_rendering_program.LoadUniform("shadowmap_color_texture");
		m_geometry_rendering_program.LoadUniform("uniform_penumbra_size");
		m_geometry_rendering_program.LoadUniform("uniform_blocker_distance");
	}

	//plane rendering (green , red )
	{
		vertex_shader_path = "../Data/Shaders/plane_rendering.vert";
		fragment_shader_path = "../Data/Shaders/plane_rendering.frag";
		m_plane_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
		m_plane_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
		initialized = initialized && m_plane_rendering_program.CreateProgram();

		m_plane_rendering_program.LoadUniform("uniform_projection_matrix");
		m_plane_rendering_program.LoadUniform("uniform_view_matrix");
		m_plane_rendering_program.LoadUniform("uniform_model_matrix");
		m_plane_rendering_program.LoadUniform("uniform_normal_matrix");
		m_plane_rendering_program.LoadUniform("uniform_has_texture");
		m_plane_rendering_program.LoadUniform("diffuse_texture");
		m_plane_rendering_program.LoadUniform("has_uniform_diffuse");
		m_plane_rendering_program.LoadUniform("uniform_diffuse");
		// Light Source Uniforms
		m_plane_rendering_program.LoadUniform("uniform_light_projection_matrix");
		m_plane_rendering_program.LoadUniform("uniform_light_view_matrix");
		m_plane_rendering_program.LoadUniform("uniform_cast_shadows");
		m_plane_rendering_program.LoadUniform("shadowmap_texture");
		m_plane_rendering_program.LoadUniform("uniform_focused");
	}
	//

	// gui rendering program
	{
		vertex_shader_path = "../Data/Shaders/simple_rendering.vert";
		fragment_shader_path = "../Data/Shaders/simple_rendering.frag";
		m_gui_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
		m_gui_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
		initialized = initialized && m_gui_rendering_program.CreateProgram();

		m_gui_rendering_program.LoadUniform("uniform_model_matrix");
		m_gui_rendering_program.LoadUniform("uniform_view_matrix");
		m_gui_rendering_program.LoadUniform("uniform_projection_matrix");
		m_gui_rendering_program.LoadUniform("uniform_texture");
	}

	// Post Processing Program
	{
		vertex_shader_path = "../Data/Shaders/postproc.vert";
		fragment_shader_path = "../Data/Shaders/postproc.frag";
		m_postprocess_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
		m_postprocess_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
		initialized = initialized && m_postprocess_program.CreateProgram();
		m_postprocess_program.LoadUniform("uniform_texture");
		m_postprocess_program.LoadUniform("uniform_time");
		m_postprocess_program.LoadUniform("uniform_depth");
		m_postprocess_program.LoadUniform("uniform_projection_inverse_matrix");
		m_postprocess_program.LoadUniform("uniform_apply");
		m_postprocess_program.LoadUniform("uniform_blur_enabled");
		m_postprocess_program.LoadUniform("uniform_camera_move");
		m_postprocess_program.LoadUniform("uniform_scaleU");
		m_postprocess_program.LoadUniform("background");
	}

	// Shadow mapping Program
	{
		vertex_shader_path = "../Data/Shaders/shadow_map_rendering.vert";
		fragment_shader_path = "../Data/Shaders/shadow_map_rendering.frag";
		m_spot_light_shadow_map_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
		m_spot_light_shadow_map_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
		initialized = initialized && m_spot_light_shadow_map_program.CreateProgram();
		m_spot_light_shadow_map_program.LoadUniform("uniform_projection_matrix");
		m_spot_light_shadow_map_program.LoadUniform("uniform_view_matrix");
		m_spot_light_shadow_map_program.LoadUniform("uniform_model_matrix");
	}


	// vsm shader
	{
		vertex_shader_path = "../Data/Shaders/vsm_shadow_shader.VERT";
		fragment_shader_path = "../Data/Shaders/vsm_shadow_shader.FRAG";
		m_vsm_shading_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
		m_vsm_shading_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
		initialized = initialized && m_vsm_shading_program.CreateProgram();
		m_vsm_shading_program.LoadUniform("uniform_projection_matrix");
		m_vsm_shading_program.LoadUniform("uniform_view_matrix");
		m_vsm_shading_program.LoadUniform("uniform_model_matrix");

	}

	// blur shader
	{
		vertex_shader_path = "../Data/Shaders/blur_shader.vert";
		fragment_shader_path = "../Data/Shaders/blur_shader.frag";
		m_blur_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
		m_blur_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
		initialized = initialized && m_blur_program.CreateProgram();
		m_blur_program.LoadUniform("uniform_texture");
		m_blur_program.LoadUniform("uniform_scale_u");


	}

	return initialized;
}

bool Renderer::ReloadShaders()
{
	bool reloaded = true;

	reloaded = reloaded && m_geometry_rendering_program.ReloadProgram();
	reloaded = reloaded && m_plane_rendering_program.ReloadProgram();
	reloaded = reloaded && m_postprocess_program.ReloadProgram();
	reloaded = reloaded && m_spot_light_shadow_map_program.ReloadProgram();
	reloaded = reloaded && m_gui_rendering_program.ReloadProgram();
	reloaded = reloaded && m_vsm_shading_program.ReloadProgram();
	reloaded = reloaded && m_blur_program.ReloadProgram();
	reloaded = reloaded && ParticleSystem::reloadShaders();

	return reloaded;
}

bool Renderer::InitDeferredShaderBuffers()
{
	// generate texture handles 
	glGenTextures(1, &m_fbo_texture);
	glGenTextures(1, &m_fbo_depth_texture);

	glGenTextures(1, &m_shadowmap_blur_texture);

	// framebuffer to link everything together
	glGenFramebuffers(1, &m_fbo);
	glGenFramebuffers(1, &m_shadowmap_blur_fbo);

	return ResizeBuffers(m_screen_width, m_screen_height);
}

// resize post processing textures and save the screen size
bool Renderer::ResizeBuffers(int width, int height)
{
	m_screen_width = width;
	m_screen_height = height;

	// texture
	glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_screen_width, m_screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);



	// depth texture
	glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_screen_width, m_screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	// framebuffer to link to everything together
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fbo_depth_texture, 0);

	//
	glBindTexture(GL_TEXTURE_2D, m_shadowmap_blur_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 1024, 1024, 0, GL_RG, GL_FLOAT, NULL);


	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowmap_blur_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_shadowmap_blur_texture, 0);

	GLenum status = Tools::CheckFramebufferStatus(m_fbo);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	status = Tools::CheckFramebufferStatus(m_shadowmap_blur_fbo);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_projection_matrix = glm::perspective(glm::radians(60.f), width / (float)height, 0.1f, 1500.0f);
	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

	return true;
}


void Renderer::SetRenderingMode(RENDERING_MODE mode)
{
	m_rendering_mode = mode;
}

void Renderer::setShadingMode(SHADING_MODE mode)
{
	m_shading_mode = mode;
}

void Renderer::setTestingMode()
{
	testing_mode = !testing_mode;
}

void Renderer::setShadowTest()
{
	SHADOW_TEST = !SHADOW_TEST;
}

void Renderer::testObjectsMove()
{
	testObjects_move = !testObjects_move;
}

void Renderer::ShadowRendering_toScreen()
{
	shadow_render_to_screen = !shadow_render_to_screen;
}

// Load Geometric Meshes
bool Renderer::InitGeometricMeshes()
{
	bool initialized = true;

	return initialized;
}

void Renderer::InitMatrices()
{
	// geometry elements


	// gui elements
	{
		// gui view matrix && projection matrix
		m_gui_view_matrix = glm::mat4(1.f);
		m_gui_projection_matrix = glm::ortho(-2.f, 2.f, -2.f, 2.f);

		// bones icon model matrix
		bones_icon_model_matrix =
			glm::translate(glm::mat4(1.0f), glm::vec3(-1.7, -1.8, 0))
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.25, 0.15, 0)*0.55f);

		// coins icon model matrix
		coins_icon_model_matrix =
			glm::translate(glm::mat4(1.0f), glm::vec3(-1.71, -1.65, 0))
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.25, 0.15, 0)*0.6f);

		// bones numbers model matrices
		for (int i = 0; i < 3; i++)
		{
			bones_numbers_model_matrix[i] =
				glm::translate(glm::mat4(1.0f), glm::vec3(-1.785 + 0.25 + i * 0.087, -1.8, 0))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.15, 0.25, 0)*0.2f);
		}

		// coins numbers model matrices
		for (int i = 0; i < 3; i++)
		{
			coins_numbers_model_matrix[i] =
				glm::translate(glm::mat4(1.0f), glm::vec3(-1.785 + 0.25 + i * 0.087, -1.65, 0))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.15, 0.25, 0)*0.2f);
		}


		// tower icon model matrix
		tower_icon_model_matrix =
			glm::translate(glm::mat4(1.0f), glm::vec3(-1.7 + 1, -1.8, 0))
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.15, 0.25, 0.25)*0.65f);

		// meteorite icon model matrix
		meteorite_icon_model_matrix =
			glm::translate(glm::mat4(1.0f), glm::vec3(-1.7 + 1.3, -1.8, 0))
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.15, 0.25, 0.25)*0.55f);

		// ufo icon model matrix
		ufo_icon_model_matrix =
			glm::translate(glm::mat4(1.0f), glm::vec3(-1.7 + 1.5, -1.8, 0))
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.15, 0.25, 0.25)*0.65f);

		// info icon model matrix
		info_model_matrix =
			glm::translate(glm::mat4(1.0f), glm::vec3(1.75, 1.72, 0))
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.25));
	}


}


// Initialize the light sources
bool Renderer::InitLightSources()
{
	// Initialize the spot light
	m_spotlight_node.SetPosition(glm::vec3(30, 28, 38)); //glm::vec3(30, 25, 40)
	m_spotlight_node.SetTarget(glm::vec3(16.4, 0, 15));
	m_spotlight_node.SetColor(glm::vec3(215 - 60, 215 - 80, 215 - 60));                   //glm::vec3(140,140,140);
	m_spotlight_node.SetConeSize(83, 90); //73 , 80
	m_spotlight_node.CastShadow(true);
	m_spotlight_node.StoreColor(true);

	return true;
}






void Renderer::arcball_rotate(glm::vec2 vec)
{
	m_camera_look_angle_destination += glm::vec2(0.2, -0.2) * vec;
}


void Renderer::updateCamera(float dt)
{
	float movement_speed = 10.0f;

#define camera1
#ifdef camera1

	
	glm::vec3 direction = glm::normalize(m_camera_target_position - m_camera_position);

	m_camera_position += m_camera_movement.x *  movement_speed * direction * dt;
	m_camera_target_position += m_camera_movement.x * movement_speed * direction * dt;

	glm::vec3 right = glm::normalize(glm::cross(direction, m_camera_up_vector));
	m_camera_position += m_camera_movement.y *  movement_speed * right * dt;
	m_camera_target_position += m_camera_movement.y * movement_speed * right * dt;

	//std::cout << "\n\nm_camera_angle_dest.x = "<< m_camera_look_angle_destination.x<< std::endl;
	//std::cout << "m_camera_angle_dest.y = " << m_camera_look_angle_destination.y << std::endl;

	//std::cout << "right v : (" << right.x << " , " << right.y << " , " << right.z << " ) " << std::endl;
	//std::cout << "up v    : (" << m_camera_up_vector.x << " , " << m_camera_up_vector.y << " , " << m_camera_up_vector.z << " ) " << std::endl;

	m_camera_look_angle_destination *= glm::pi<float>();// / 90;
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), m_camera_look_angle_destination.y * dt, right) * glm::rotate(glm::mat4(1.0), -m_camera_look_angle_destination.x * dt, m_camera_up_vector);
	direction = rotation * glm::vec4(direction, 0);
	float dist = glm::distance(m_camera_position, m_camera_target_position);
	m_camera_target_position = m_camera_position + direction * dist;
	m_camera_look_angle_destination = glm::vec2(0);

	m_camera_look_angle_destination -= m_camera_look_angle_destination * dt;
	m_camera_look_angle_destination.x = (abs(m_camera_look_angle_destination.x) < 0.01) ? 0.0 : m_camera_look_angle_destination.x;
	m_camera_look_angle_destination.y = (abs(m_camera_look_angle_destination.y) < 0.01) ? 0.0 : m_camera_look_angle_destination.y;

	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

#endif

#define camera2
#ifdef camera2
	


#endif

}

void Renderer::Update(float dt)
{

	updateCamera(dt);

	// update light source

	if (light_move)
	{
		m_spotlight_node.SetPosition
		(
			glm::vec4(glm::vec3(30, 25 , 35), 1.0f) // 30 , 28 , 38
			* glm::rotate(glm::mat4(1.0f), light_time * 0.25f, glm::vec3(0.0, 1 , -0.03)) //0.0 1 -0.03
			* glm::translate(glm::mat4(1.0f), glm::vec3(2.5, 0, 0))
		);
		//m_spotlight_node.SetColor(glm::vec3(215 - 60, 215 - 80, 215 - 60));                   //glm::vec3(140,140,140);
		m_spotlight_node.SetColor(glm::vec3(215 - 60 + 10 * cos(light_time *0.07f) , 215 - 80 + 10 * sin(light_time * 0.07f)  ,
								  215 - 60 + 10*cos(light_time * 0.07f)) );

		//m_spotlight_node.SetColor(glm::vec3(215 - 70, 215 - 80, 215 - 90));                   

		light_time += dt;
	}




	if (SHADOW_TEST)
	{
		if (testObjects_move)
			test_dt += dt;
		
		// cube 1
		cube_model_matrix1 =
			glm::translate(glm::mat4(1.0f), game->getTerrain()->getWorldPosition() + glm::vec3(10, 5.1 , 10))
			* glm::rotate(glm::mat4(1.0f), test_dt * 0.1f, glm::vec3(1, 1, 1))
			* glm::scale(glm::mat4(1.0f), glm::vec3(3.25 , 0.1 , 3.25f));

		cube_normal_matrix1 = glm::mat4(glm::transpose(glm::inverse(glm::mat3(cube_model_matrix1))));

		// cube 2
		cube_model_matrix2 =
			glm::translate(glm::mat4(1.0f), game->getTerrain()->getWorldPosition() + glm::vec3(-10, 5, 8))
			* glm::rotate(glm::mat4(1.0f), test_dt * 0.1f, glm::vec3(1, 1, 1))
			* glm::scale(glm::mat4(1.0f), glm::vec3(2.f, 10.0f, 2.f));

		cube_normal_matrix2 = glm::mat4(glm::transpose(glm::inverse(glm::mat3(cube_model_matrix2))));

		// sphere
		sphere_model_matrix =
			glm::translate(glm::mat4(1.0f), glm::vec3(game->getTerrain()->getWorldPosition()) + glm::vec3(0,10,0) )
			* glm::rotate(glm::mat4(1.0f) , test_dt * 0.1f ,  glm::vec3(0,1,0))
			*glm::scale(glm::mat4(1.0f), glm::vec3(4.0f));

		sphere_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(sphere_model_matrix))));

		// tower
		tower_model_matrix =
			glm::translate(glm::mat4(1.0f), glm::vec3(15, 0.1, 15))
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.4));

		tower_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(tower_model_matrix))));


		// pirate
		

	}
	

	// check for input
	{
		int x; int y;
		if (game->Selecting()) // selection is true , check for focused tiles
		{

			SDL_GetMouseState(&x, &y);
			CheckForSelection(x, y, true);

		}
		else // selection is false, check for focused objects like towers or gui buttons
		{
			SDL_GetMouseState(&x, &y);
			CheckForSelection(x, y, false);
		}

	}


	// update animations && particles
	{
		if (!AnimationHandler::isEmpty())
		{
			AnimationHandler::Update(dt);
		}

		if (!ParticleSystem::isEmpty())
		{
			ParticleSystem::Update(dt);
		}
	}

	m_continous_time += dt;
}


/* draw GeometryNode functions */
void Renderer::DrawGeometryNode(GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix)
{

	glBindVertexArray(node->m_vao);
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(normal_matrix));



	for (int j = 0; j < node->parts.size(); j++)
	{

		glm::vec3 diffuseColor = node->parts[j].diffuseColor;
		glm::vec3 specularColor = node->parts[j].specularColor;
		float shininess = node->parts[j].shininess;
		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (node->parts[j].textureID > 0) ? 1.0f : 0.0f);
		glUniform1i(m_geometry_rendering_program["uniform_damage_texture"], 0);
		glUniform1i(m_geometry_rendering_program["uniform_dizzy_state"], 0);
		glUniform1i(m_geometry_rendering_program["uniform_cannon_ball"], 0);
		glUniform1i(m_geometry_rendering_program["uniform_meteorite"], 0);

		glUniform1i(m_geometry_rendering_program["uniform_wall"], 0);

		glBindTexture(GL_TEXTURE_2D, node->parts[j].textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

		glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);
	}
}

void Renderer::DrawGeometryNode(GameObject * object)
{

	if (object->getObjectID()!=GameObject::CASSINI_PROJECTILE)
	{
		GeometryNode * node = object->getNode();

		glm::mat4 model_matrix = object->getTransformationMatrix();
		glm::mat4 normal_matrix = object->getNormalMatrix();

		glBindVertexArray(node->m_vao);
		glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(normal_matrix));

		for (int j = 0; j < node->parts.size(); j++)
		{

			glm::vec3 diffuseColor = node->parts[j].diffuseColor;
			glm::vec3 specularColor = node->parts[j].specularColor;
			float shininess = node->parts[j].shininess;
			glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
			glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (node->parts[j].textureID > 0) ? 1.0f : 0.0f);
			glUniform1i(m_geometry_rendering_program["uniform_wall"], 0);
			glUniform1i(m_geometry_rendering_program["uniform_dizzy_state"], 0);

			glUniform1i(m_geometry_rendering_program["uniform_cannon_ball"], 0);
			glUniform1i(m_geometry_rendering_program["uniform_meteorite"], 0);
			glUniform1i(m_geometry_rendering_program["uniform_damage_texture"], 0);


			if (object->getObjectID() == GameObject::TOWER && object->isFocused())
			{
				glUniform1i(m_geometry_rendering_program["uniform_focused"], 1);
			}
			else
				glUniform1i(m_geometry_rendering_program["uniform_focused"], 0);

			glBindTexture(GL_TEXTURE_2D, node->parts[j].textureID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

			glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);

		}
	}
	else
	{
		glBindVertexArray(m_cube_vao);

		glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(object->getTransformationMatrix()));
		glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(object->getNormalMatrix()));

		glm::vec3 diffuseColor = glm::vec3(1.0);
		glm::vec3 specularColor = glm::vec3(0.4);
		float shininess = 0.001; //0.001
		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], 0.0f);
		glUniform1i(m_geometry_rendering_program["uniform_wall"], 0);
		glUniform1i(m_geometry_rendering_program["uniform_damage_texture"], 0);
		glUniform1i(m_geometry_rendering_program["uniform_dizzy_state"], 0);
		glUniform1i(m_geometry_rendering_program["uniform_cannon_ball"], 0);
		glUniform1i(m_geometry_rendering_program["uniform_meteorite"], 0);

		
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
	}
}

void Renderer::DrawGeometryNodes(std::vector<GameObject *> &objects)
{
	for (int i = 0; i < objects.size(); i++)
	{
		if (objects[i]->isDrawable())
			DrawGeometryNode(objects[i]);

		for (int j = 0; j < objects[i]->getPartsSize(); j++)
		{
			if (objects[i]->getIndex(j)->isDrawable())
			{
				DrawGeometryNode(objects[i]->getIndex(j));
			}
		}
	}

}

void Renderer::DrawSimpleGeometryNode(GameObject * object)
{
	bool tower = false;
	bool occupied = false;

	GeometryNode * node = nullptr;
	if (object->getObjectID() != GameObject::TOWER)
	{
		if (object->isOccupied() && !game->isActiveSuperPower())
			node = AssetManager::GetIndex("red_plane");
		else
			node = AssetManager::GetIndex("green_plane");
	}
	else
	{
		node = object->getNode();
		tower = true;
	}


	glBindVertexArray(node->m_vao);
	glUniformMatrix4fv(m_plane_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(object->getTransformationMatrix()));
	glUniformMatrix4fv(m_plane_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(object->getNormalMatrix()));


	for (int j = 0; j < node->parts.size(); j++)
	{
		glm::vec3 diffuseColor = node->parts[j].diffuseColor;
		glm::vec3 specularColor = node->parts[j].specularColor;
		float shininess = node->parts[j].shininess;

		if (tower)
		{
			if (game->Selecting())
			{
				if (game->getFocusedObject()->isOccupied())
					diffuseColor = glm::vec3(0.8, 0, 0);
				else
					diffuseColor = glm::vec3(0, 0.8, 0);
				glUniform1i(m_plane_rendering_program["uniform_focused"], 0);
			}
			else
			{
				diffuseColor = glm::vec3(1, 1, 1);
				glUniform1i(m_plane_rendering_program["uniform_focused"], 1);
			}

			glUniform1i(m_plane_rendering_program["has_uniform_diffuse"], 1);
			glUniform3f(m_plane_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			//glUniform3f(m_plane_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			//glUniform1f(m_plane_rendering_program["uniform_shininess"], shininess);
		}
		else
		{
			glUniform1i(m_plane_rendering_program["has_uniform_diffuse"], 0);
		}


		glUniform1f(m_plane_rendering_program["uniform_has_texture"], (node->parts[j].textureID > 0) ? 1.0f : 0.0f);
		glBindTexture(GL_TEXTURE_2D, node->parts[j].textureID);

		glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);
	}
}

void Renderer::DrawGeometryNodeToShadowMap(GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix, ShaderProgram * program)
{
	
	glBindVertexArray(node->m_vao);
	glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

	for (int j = 0; j < node->parts.size(); j++)
	{
		glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);
	}
}

void Renderer::DrawGeometryNodesToShadowMap(std::vector<GameObject *> &objects, ShaderProgram * program)
{
	for (int i = 0; i < objects.size(); i++)
	{

		if (objects[i]->isDrawable())
			DrawGeometryNodeToShadowMap(objects[i]->getNode(), objects[i]->getTransformationMatrix(), objects[i]->getNormalMatrix(), program);

		if (objects[i]->getObjectID() == GameObject::CASSINI)
			continue;

		for (int j = 0; j < objects[i]->getPartsSize(); j++)
		{
			if (objects[i]->getIndex(j)->isDrawable())
				DrawGeometryNodeToShadowMap(objects[i]->getIndex(j)->getNode(), objects[i]->getIndex(j)->getTransformationMatrix(), objects[i]->getIndex(j)->getNormalMatrix(), program);
		}

	}
}
/* */

/* Rendering functions */
void Renderer::Render()
{

	// Draw the geometry to the shadow maps
	RenderShadowMaps();

	// Draw the geometry
	RenderGeometry();

	// apply filters
	ApplyFilters();

	// Render Planes
	RenderPlanes();

	// draw gui elements
	RenderGui();

	if (shadow_render_to_screen)
		RenderShadowMapToScreen(0);
	else
		RenderToOutFB();



	GLenum error = Tools::CheckGLError();
	if (error != GL_NO_ERROR)
	{
		printf("Reanderer:Draw GL Error\n");
		system("pause");
	}
}

void Renderer::RenderShadowMaps()
{
	

	// if the light source casts shadows
	if (m_spotlight_node.GetCastShadowsStatus())
	{
		ShaderProgram * program = 0;

		{
			int m_depth_texture_resolution = m_spotlight_node.GetShadowMapResolution();

			// bind the framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, m_spotlight_node.GetShadowMapFBO());
			glViewport(0, 0, m_depth_texture_resolution, m_depth_texture_resolution);
			GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, drawbuffers);

			// clear depth buffer
			glClear( GL_DEPTH_BUFFER_BIT  | GL_COLOR_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			//glDisable(GL_BLEND);



			if (m_shading_mode == VSM)
				program = &m_vsm_shading_program;
			else
				program = &m_spot_light_shadow_map_program;

			// Bind the shadow mapping program
			program->Bind();




			// pass the projection and view matrix to the uniforms
			glUniformMatrix4fv((*program)["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetProjectionMatrix()));
			glUniformMatrix4fv((*program)["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetViewMatrix()));

		}

		if (!SHADOW_TEST)
		{
			/* - - - - - - - -  - - - - - - - - - -  Drawing Game Objects - - - - - - - -  - - - - - - - - - -  */

		    // draw Game Terrain
		    // DrawGeometryNodeToShadowMap(game->getTerrain()->getNode(), game->getTerrain()->getTransformationMatrix(), game->getTerrain()->getNormalMatrix());

		    // draw Game Tiles
			

			for (int i = 0; i < game->getRoadTiles().size(); i++)
			{
				//DrawGeometryNodeToShadowMap(game->getRoadTiles()[i]->getNode(), game->getRoadTiles()[i]->getTransformationMatrix(), game->getRoadTiles()[i]->getNormalMatrix());

				glBindVertexArray(m_cube_vao);
				glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(game->getRoadTiles()[i]->getCubeTransformationMatrix()));
				glDrawArrays(GL_TRIANGLES, 0, 36);

			}
			
			// draw Game objects ( pirates , towers , chests , projectiles , etc . . )


			DrawGeometryNodesToShadowMap(game->getTowers(), program);
			DrawGeometryNodesToShadowMap(game->getPirates(), program); // update towers shadow only if changed position
			DrawGeometryNodesToShadowMap(game->getChests(), program);

			// draw the selected object if any
			/*
			if (game->Selecting() && game->getFocusedObject() != nullptr)
			{
				GameObject * tower = new Tower(game->getMesh("tower"), nullptr);
				tower->setWorldPosition(game->getFocusedObject()->getWorldPosition());
				tower->Update(0.0f);
				DrawGeometryNodeToShadowMap(tower->getNode() , tower->getTransformationMatrix() , tower->getNormalMatrix());
			}
			*/

			// draw the cube
			{

				glBindVertexArray(m_cube_vao);
				glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(game->getTerrain()->getCubeTransformationMatrix()));
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

			// draw the asteroids
			DrawGeometryNodesToShadowMap(game->getAsteroids(), program);

		}

		else
		{
			

			// draw test cubes
			{

				glBindVertexArray(m_cube_vao);

				glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(cube_model_matrix1));
				glDrawArrays(GL_TRIANGLES, 0, 36);

				glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(cube_model_matrix2));
				glDrawArrays(GL_TRIANGLES, 0, 36);

				glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(game->getTerrain()->getCubeTransformationMatrix()));
				glDrawArrays(GL_TRIANGLES, 0, 36);

			}

			// draw spehres
			{
				DrawGeometryNodeToShadowMap(AssetManager::GetIndex("sphere"), sphere_model_matrix, sphere_normal_matrix , program);
			}

			// draw towers
			{
				DrawGeometryNodeToShadowMap(AssetManager::GetIndex("tower"), tower_model_matrix, tower_normal_matrix, program);
			}

			// draw pirates
			{
				//DrawGeometryNodeToShadowMap(game->getMesh("pirate"))
			}
		}

		/* - - - - - - - -  - - - - - - - - - -  - - - - - - - - - - - - - - - - - -  - - - - - - - - - -  */


		glBindVertexArray(0);
		// Unbind shadow mapping program
		program->Unbind();

		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (m_shading_mode == VSM)
	{
		//BlurrShadowMap();
	}

	
}

void Renderer::RenderGeometry()
{
	// bind geometry rendering program and pass parameters
	{
		// Bind the Intermediate framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glViewport(0, 0, m_screen_width, m_screen_height);
		GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawbuffers);

		// clear color and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClearColor(0.2, 0.1 , 0, 1);


		glEnable(GL_DEPTH_TEST);

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

		// Bind the geometry rendering program
		m_geometry_rendering_program.Bind();

		switch (m_shading_mode)
		{
		case SHADING_MODE::NEAREST:
			glUniform1i(m_geometry_rendering_program["uniform_shading_mode"], 0);
			break;
		case SHADING_MODE::PCF:
			glUniform1i(m_geometry_rendering_program["uniform_shading_mode"], 1);
			break;
		case SHADING_MODE::PCSS_1:
			glUniform1i(m_geometry_rendering_program["uniform_shading_mode"], 2);
			break;
		case SHADING_MODE::PCSS_2:
			glUniform1i(m_geometry_rendering_program["uniform_shading_mode"], 3);
			break;
		case SHADING_MODE::VSM:
			glUniform1i(m_geometry_rendering_program["uniform_shading_mode"], 4);
			break;
		};

		if (testing_mode)
			glUniform1i(m_geometry_rendering_program["uniform_testing_mode"], 1);
		else
			glUniform1i(m_geometry_rendering_program["uniform_testing_mode"], 0);

		// pass camera parameters to uniforms
		glUniformMatrix4fv(m_geometry_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
		glUniformMatrix4fv(m_geometry_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
		glUniform3f(m_geometry_rendering_program["uniform_camera_position"], m_camera_position.x, m_camera_position.y, m_camera_position.z);

		// pass the light source parameters to uniforms
		glm::vec3 light_position = m_spotlight_node.GetPosition();
		glm::vec3 light_direction = m_spotlight_node.GetDirection();
		glm::vec3 light_color = m_spotlight_node.GetColor();
		glUniformMatrix4fv(m_geometry_rendering_program["uniform_light_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetProjectionMatrix()));
		glUniformMatrix4fv(m_geometry_rendering_program["uniform_light_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetViewMatrix()));
		glUniform3f(m_geometry_rendering_program["uniform_light_position"], light_position.x, light_position.y, light_position.z);
		glUniform3f(m_geometry_rendering_program["uniform_light_direction"], light_direction.x, light_direction.y, light_direction.z);
		glUniform3f(m_geometry_rendering_program["uniform_light_color"], light_color.x, light_color.y, light_color.z);
		glUniform1f(m_geometry_rendering_program["uniform_light_umbra"], m_spotlight_node.GetUmbra());
		glUniform1f(m_geometry_rendering_program["uniform_light_penumbra"], m_spotlight_node.GetPenumbra());
		glUniform1i(m_geometry_rendering_program["uniform_cast_shadows"], (m_spotlight_node.GetCastShadowsStatus()) ? 1 : 0);
		// Set the sampler2D uniform to use texture unit 1
		glUniform1i(m_geometry_rendering_program["shadowmap_texture"], 1);
		// Bind the shadow map texture to texture unit 1
		glActiveTexture(GL_TEXTURE1);
		if (m_shading_mode == VSM)
		{
			glBindTexture(GL_TEXTURE_2D, m_spotlight_node.GetShadowMapDepthTexture());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, (m_spotlight_node.GetCastShadowsStatus()) ? m_spotlight_node.GetShadowMapColorTexture() : 0);
			glUniform1i(m_geometry_rendering_program["shadowmap_color_texture"], 2);

		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, (m_spotlight_node.GetCastShadowsStatus()) ? m_spotlight_node.GetShadowMapDepthTexture() : 0);

		}

		// Enable Texture Unit 0
		glUniform1i(m_geometry_rendering_program["uniform_diffuse_texture"], 0);
		glActiveTexture(GL_TEXTURE0);



	}


	/* - - - - - - - -  - - - - - - - - - -  Drawing Game Objects - - - - - - - -  - - - - - - - - - -  */

	if (!SHADOW_TEST)
	{

		// draw Terrain
		DrawGeometryNode(game->getTerrain());
		// draw cube
		{
			glBindVertexArray(m_cube_vao);


			glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(game->getTerrain()->getCubeTransformationMatrix()));
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(game->getTerrain()->getNormalMatrix()));

			glm::vec3 diffuseColor = glm::vec3(0.6);
			glm::vec3 specularColor = glm::vec3(0.4);
			float shininess = 0.001; //0.001
			glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
			glUniform1f(m_geometry_rendering_program["uniform_has_texture"], 1.0f);
			glUniform1i(m_geometry_rendering_program["uniform_wall"], 1);
			glUniform1i(m_geometry_rendering_program["uniform_damage_texture"], 0);
			glUniform1i(m_geometry_rendering_program["uniform_dizzy_state"], 0);
			glUniform1i(m_geometry_rendering_program["uniform_cannon_ball"], 0);
			glUniform1i(m_geometry_rendering_program["uniform_meteorite"], 0);



			glBindTexture(GL_TEXTURE_2D, *AssetManager::getTexture("floor02"));

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
			glDrawArrays(GL_TRIANGLES, 0, 36);

		}


		// draw Tiles
		for (int i = 0; i < game->getRoadTiles().size(); i++)
		{
			DrawGeometryNode(game->getRoadTiles()[i]);

			{

				glBindVertexArray(m_cube_vao);
				glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(game->getRoadTiles()[i]->getCubeTransformationMatrix()));
				glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(game->getRoadTiles()[i]->getCubeNormalMatrix()));

				glm::vec3 diffuseColor = glm::vec3(0.6);
				glm::vec3 specularColor = glm::vec3(0.4);
				float shininess = 0.001; //0.001
				glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
				glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
				glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
				glUniform1f(m_geometry_rendering_program["uniform_has_texture"], 1.0f);
				glUniform1i(m_geometry_rendering_program["uniform_wall"], 0);
				glUniform1i(m_geometry_rendering_program["uniform_damage_texture"], 0);
				glUniform1i(m_geometry_rendering_program["uniform_dizzy_state"], 0);
				glUniform1i(m_geometry_rendering_program["uniform_cannon_ball"], 0);
				glUniform1i(m_geometry_rendering_program["uniform_meteorite"], 0);


				glBindTexture(GL_TEXTURE_2D, *AssetManager::getTexture("floor02"));

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glGenerateMipmap(GL_TEXTURE_2D);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

		// draw Towers
		DrawGeometryNodes(game->getTowers());

		// draw Pirates
		DrawGeometryNodes(game->getPirates());

		// draw chests
		DrawGeometryNodes(game->getChests());

		// draw animations
		{
			if (!AnimationHandler::isEmpty())
			{
				AnimationHandler::setProgram(&m_geometry_rendering_program);
				AnimationHandler::Render();
			}
		}


		// ASTEROIDS
		{
			//glDepthMask(GL_FALSE);
			
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			DrawGeometryNodes(game->getAsteroids());
			//glDepthMask(GL_TRUE);
			
			//glDisable(GL_BLEND);
		}


	}

	else
	{
		// draw test cubes
		{
			glBindVertexArray(m_cube_vao);



			glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(cube_model_matrix1));
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(cube_normal_matrix1));

			glm::vec3 diffuseColor = glm::vec3(0.6);
			glm::vec3 specularColor = glm::vec3(0.4);
			float shininess = 0.001; //0.001
			glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
			glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
			glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
			glUniform1f(m_geometry_rendering_program["uniform_has_texture"], 1.0f);
			glUniform1i(m_geometry_rendering_program["uniform_wall"], 1);
			glUniform1i(m_geometry_rendering_program["uniform_damage_texture"], 0);
			glUniform1i(m_geometry_rendering_program["uniform_dizzy_state"], 0);
			glUniform1i(m_geometry_rendering_program["uniform_cannon_ball"], 0);
			glUniform1i(m_geometry_rendering_program["uniform_meteorite"], 0);


			glBindTexture(GL_TEXTURE_2D, m_quad_texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
			// cube 1
			glDrawArrays(GL_TRIANGLES, 0, 36);

			// cube 2
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(cube_model_matrix2));
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(cube_normal_matrix2));
			glDrawArrays(GL_TRIANGLES, 0, 36);

			// terrain cube
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(game->getTerrain()->getCubeTransformationMatrix()));
			glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(game->getTerrain()->getNormalMatrix()));
			glDrawArrays(GL_TRIANGLES, 0, 36);

		}

		// draw test spheres
		{
			DrawGeometryNode(AssetManager::GetIndex("sphere"), sphere_model_matrix, sphere_normal_matrix);
		}

		// draw test towers
		{
			DrawGeometryNode(AssetManager::GetIndex("tower"), tower_model_matrix, tower_normal_matrix);
		}

		// draw test pirate
		{

		}

	}

	m_geometry_rendering_program.Unbind();


	/*
	{

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo2);
		glViewport(0, 0, m_screen_width, m_screen_height);
		GLenum drawbuffers2[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawbuffers2);

		// clear color and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2, 0.1, 0, 1);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
	*/

	
	// Render Particles
	if (!SHADOW_TEST)
	{

		if (!ParticleSystem::isEmpty())
		{
			ParticleSystem::setViewMatrix(m_view_matrix);
			ParticleSystem::setProjectionMatrix(m_projection_matrix);
			ParticleSystem::Render();
		}

	}

	/* - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - - - - - - -  */
	glBindVertexArray(0);

	glDisable(GL_DEPTH_TEST);
	if (m_rendering_mode != RENDERING_MODE::TRIANGLES)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPointSize(1.0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderPlanes()
{
	// drawing focused object if any
	if (game->getFocusedObject() != nullptr && game->Selecting())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glViewport(0, 0, m_screen_width, m_screen_height);
		GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawbuffers);
		glEnable(GL_DEPTH_TEST);


		m_plane_rendering_program.Bind();


		// pass camera parameters to uniforms
		glUniformMatrix4fv(m_plane_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
		glUniformMatrix4fv(m_plane_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));

		glUniformMatrix4fv(m_plane_rendering_program["uniform_light_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetProjectionMatrix()));
		glUniformMatrix4fv(m_plane_rendering_program["uniform_light_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetViewMatrix()));
		glUniform1i(m_plane_rendering_program["uniform_cast_shadows"], (m_spotlight_node.GetCastShadowsStatus()) ? 1 : 0);
		glUniform1i(m_plane_rendering_program["shadowmap_texture"], 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, (m_spotlight_node.GetCastShadowsStatus()) ? m_spotlight_node.GetShadowMapDepthTexture() : 0);

		glUniform1i(m_plane_rendering_program["uniform_diffuse_texture"], 0);
		glActiveTexture(GL_TEXTURE0);

		//
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		DrawSimpleGeometryNode(game->getFocusedObject());
		if (!game->isActiveSuperPower())
		{
			GameObject * tower = new Tower(AssetManager::GetIndex("tower"), nullptr);
			tower->setWorldPosition(game->getFocusedObject()->getWorldPosition());
			tower->Update(0.0f);
			DrawSimpleGeometryNode(tower);
			delete tower;
		}


		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		m_plane_rendering_program.Unbind();


		glBindVertexArray(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
	}
}

void Renderer::RenderGui()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_screen_width, m_screen_height);
	GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawbuffers);

	m_gui_rendering_program.Bind();

	// pass camera parameters to uniforms
	glUniformMatrix4fv(m_gui_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_gui_projection_matrix));
	glUniformMatrix4fv(m_gui_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr((m_gui_view_matrix)));

	glUniform1i(m_gui_rendering_program["uniform_texture"], 0);
	glActiveTexture(GL_TEXTURE0);


	glBindVertexArray(m_quad_gui_vao);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// bones icon
	{

		glUniformMatrix4fv(m_gui_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(bones_icon_model_matrix));
		glBindTexture(GL_TEXTURE_2D, *AssetManager::getTexture("bones"));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}


	// coins icon
	{

		glUniformMatrix4fv(m_gui_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(coins_icon_model_matrix));
		glBindTexture(GL_TEXTURE_2D, *AssetManager::getTexture("coins"));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		


		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}


	// Bone numbers
	{
		int bones = game->getBones();

		if (bones > 999)
			bones = 999;


		int a = bones / 100;

		bones = bones % 100;

		int b = bones / 10;

		int c = bones % 10;

		int numb[] = { a,b,c };

		for (int i = 0; i < 3; i++)
		{

			glUniformMatrix4fv(m_gui_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(bones_numbers_model_matrix[i]));
			std::string  no = "numb_" + std::to_string(numb[i]);
			glBindTexture(GL_TEXTURE_2D, *AssetManager::getTexture(no));

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}


	}


	// coins numbers
	{
		int coins = game->getCoins();

		if (coins < 0)
			coins = 0;

		int a = coins / 100;

		coins = coins % 100;

		int b = coins / 10;

		int c = coins % 10;

		int numb[] = { a,b,c };

		for (int i = 0; i < 3; i++)
		{

			glUniformMatrix4fv(m_gui_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(coins_numbers_model_matrix[i]));
			std::string  no = "numb_" + std::to_string(numb[i]);
			glBindTexture(GL_TEXTURE_2D, *AssetManager::getTexture(no));

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}


	}



	// Tower icon
	{
		GLuint text = 0;
		if (over_tower_icon)
			text = *AssetManager::getTexture("Tower2");//tower_texture[1];
		else
			text = *AssetManager::getTexture("Tower1");

		glUniformMatrix4fv(m_gui_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(tower_icon_model_matrix));
		glBindTexture(GL_TEXTURE_2D, text);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}


	// meteorite icon
	{
		GLuint text = 0;
		if (over_meteorite_icon)
			text = *AssetManager::getTexture("sp_meteorite2");
		else
			text = *AssetManager::getTexture("sp_meteorite1");

		glUniformMatrix4fv(m_gui_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(meteorite_icon_model_matrix));
		glBindTexture(GL_TEXTURE_2D, text);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}


	// Info icon
	{
		glUniformMatrix4fv(m_gui_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(info_model_matrix));
		glBindTexture(GL_TEXTURE_2D, *AssetManager::getTexture("info"));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	//glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);



	m_gui_rendering_program.Unbind();
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::ApplyFilters()
{

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_screen_width, m_screen_height);
	GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawbuffers);

	// disable depth testing and blending
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// bind the post processing program
	m_postprocess_program.Bind();

	glBindVertexArray(m_vao_fbo);
	// Bind the intermediate color image to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
	glUniform1i(m_postprocess_program["uniform_texture"], 0);

	glUniform1i(m_postprocess_program["uniform_apply"], 1);
	glUniform1i(m_postprocess_program["uniform_blur_enabled"], 0);
	glUniform1i(m_postprocess_program["uniform_camera_move"], camera_moving ? 1 : 0);

	glUniform1i(m_postprocess_program["background"], 0);

	// Bind the intermediate depth buffer to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
	glUniform1i(m_postprocess_program["uniform_depth"], 1);

	glUniform1f(m_postprocess_program["uniform_time"], m_continous_time);
	glm::mat4 projection_inverse_matrix = glm::inverse(m_projection_matrix);
	glUniformMatrix4fv(m_postprocess_program["uniform_projection_inverse_matrix"], 1, GL_FALSE, glm::value_ptr(projection_inverse_matrix));


	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);

	// Unbind the post processing program
	m_postprocess_program.Unbind();

	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::BlurrShadowMap()
{



	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowmap_blur_fbo);
	glViewport(0, 0, m_spotlight_node.GetShadowMapResolution(), m_spotlight_node.GetShadowMapResolution());
	//glViewport(0, 0, m_screen_width, m_screen_height);

	m_blur_program.Bind();
	// clear the color and depth buffer

	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(m_quad_gui_vao);
	// Bind the intermediate color image to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_spotlight_node.GetShadowMapColorTexture());
	glUniform1i(m_blur_program["uniform_texture"], 0);

	// horizontal
	glUniform2f(m_blur_program["uniform_scale_u"], 1.0f / m_spotlight_node.GetShadowMapResolution()*2.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// vertical
	glBindFramebuffer(GL_FRAMEBUFFER, m_spotlight_node.GetShadowMapFBO());
	glBindTexture(GL_TEXTURE_2D, m_shadowmap_blur_texture);
	glUniform2f(m_blur_program["uniform_scale_u"], 0.0f, 1.0f / m_spotlight_node.GetShadowMapResolution() * 2.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	glBindVertexArray(0);

	// Unbind the post processing program
	m_blur_program.Unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void Renderer::RenderToOutFB()
{
	// Bind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_screen_width, m_screen_height);


	// clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2, 0.1, 0, 1);

	// disable depth testing and blending
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// bind the post processing program
	m_postprocess_program.Bind();

	glBindVertexArray(m_vao_fbo);
	// Bind the intermediate color image to texture unit 0
	glActiveTexture(GL_TEXTURE0);

	glm::mat4 projection_inverse_matrix = glm::inverse(m_projection_matrix);
	glUniformMatrix4fv(m_postprocess_program["uniform_projection_inverse_matrix"], 1, GL_FALSE, glm::value_ptr(projection_inverse_matrix));

	glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
	glUniform1i(m_postprocess_program["uniform_texture"], 0);

	//glUniform1i(m_postprocess_program["background"], 1);
	glUniform1i(m_postprocess_program["uniform_apply"], 0);
	glUniform1i(m_postprocess_program["uniform_blur_enabled"], 0);
	glUniform1i(m_postprocess_program["uniform_camera_move"], 0);


	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//Bind the intermediate depth buffer to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
	glUniform1i(m_postprocess_program["uniform_depth"], 1);
	glUniform1f(m_postprocess_program["uniform_time"], m_continous_time);



	glBindVertexArray(0);

	// Unbind the post processing program
	m_postprocess_program.Unbind();
}

void Renderer::RenderShadowMapToScreen(GLint shadow_map_texture)
{

	// Bind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_screen_width, m_screen_height);


	// clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClearColor(1 , 1 , 1, 1);

	// disable depth testing and blending
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// bind the post processing program
	m_postprocess_program.Bind();

	glBindVertexArray(m_quad_vao);
	// Bind the intermediate color image to texture unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, m_spotlight_node.GetShadowMapColorTexture());
	//glBindTexture(GL_TEXTURE_2D, m_spotlight_node.GetShadowMapColorTexture());
	glUniform1i(m_postprocess_program["uniform_texture"], 0);

	//glUniform1i(m_postprocess_program["background"], 1);
	glUniform1i(m_postprocess_program["uniform_apply"], 0);
	glUniform1i(m_postprocess_program["uniform_blur_enabled"], 0);


	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);



	glBindVertexArray(0);

	// Unbind the post processing program
	m_postprocess_program.Unbind();
}

/* */

void Renderer::LightSourceMove()
{
	light_move = !light_move;
}
void Renderer::CameraMoveForward(bool enable)
{
	m_camera_movement.x = (enable) ? 1 : 0;
	camera_moving_vars[0] = (enable) ? 1 : 0;

	cameraMove_calc();
}
void Renderer::CameraMoveBackWard(bool enable)
{
	m_camera_movement.x = (enable) ? -1 : 0;
	camera_moving_vars[1] = (enable) ? 1 : 0;

	cameraMove_calc();
}
void Renderer::CameraMoveLeft(bool enable)
{
	m_camera_movement.y = (enable) ? -1 : 0;
	camera_moving_vars[2] = (enable) ? 1 : 0;

	cameraMove_calc();
}
void Renderer::CameraMoveRight(bool enable)
{
	m_camera_movement.y = (enable) ? 1 : 0;
	camera_moving_vars[3] = (enable) ? 1 : 0;

	cameraMove_calc();
}
void Renderer::cameraMove(bool enable)
{
	cameraMove_calc();
	camera_moving += enable;

}
void Renderer::CameraLook(glm::vec2 lookDir)
{
	m_camera_look_angle_destination += glm::vec2(0.2, -0.2) * lookDir;
	//cameraMove(enable);
}



void Renderer::cameraMove_calc()
{
	camera_moving = camera_moving_vars[0] + camera_moving_vars[1] + camera_moving_vars[2] + camera_moving_vars[3];

}

void Renderer::CheckForSelection(int x, int y, bool state)
{
	if (state)
	{
		screenPosToWorldRay(x, y, m_projection_matrix, m_view_matrix, state);

		std::vector<GameObject * >objects = game->getTiles();
		int z_min = 1000;
		int _min = -1;

		for (int i = 0; i < objects.size(); i++)
		{
			//if (objects[i]->getObjectID() == GameObject::FREE_TILE)
			      //objects[i]->Update(0.0f);
		

			if (TestRayOBBIntersection(objects[i]))
			{
				if (objects[i]->getWorldPosition().z < z_min)
				{
					z_min = objects[i]->getWorldPosition().z;
					_min = i;
					
				}

			}
			else
				objects[i]->isFocused() = false;


		}
		if (_min != -1)
			game->setFocusedObject(objects[_min]);

	}
	else
	{

		screenPosToWorldRay(x, y, m_gui_projection_matrix, m_gui_view_matrix, state);
		if (TestRayOBBIntersection(tower_icon_model_matrix, 1.0f))
		{

			over_tower_icon = true;
			return;
		}
		else
			over_tower_icon = false;

		if (TestRayOBBIntersection(meteorite_icon_model_matrix, 1.f))
		{
			
			over_meteorite_icon = true;
			game->activateSuperPower(Game::METEORITE);
			return;
			
		}
		else
		{
			
			over_meteorite_icon = false;
			game->activateSuperPower(-1);
			
		}

		

		screenPosToWorldRay(x, y, m_projection_matrix, m_view_matrix, true);
		std::vector<GameObject*>objects = game->getTowers();
		bool focused = false;
		int z_min = 100;
		int _min;
		for (int i = 0; i < objects.size(); i++)
		{


			if (objects[i]->getObjectID() == GameObject::TOWER)
			{

				if (TestRayOBBIntersection(objects[i]))
				{
					if (objects[i]->getWorldPosition().z < z_min)
					{
						z_min = objects[i]->getWorldPosition().z;
						_min = i;
						focused = true;
					}
					
				}
				else
					objects[i]->isFocused() = false;
			}


		}

		if (!focused)
			game->setFocusedObject(nullptr);
		else
		{
			game->setFocusedObject(objects[_min]);
			objects[_min]->isFocused() = true;
		}

	}

}

void Renderer::screenPosToWorldRay(int mouseX, int mouseY, glm::mat4 & projection_matrix, glm::mat4 & view_matrix, bool state)
{
	float mouseXX = 2.0f * (float)mouseX / m_screen_width - 1.0f;
	float mouseYY = 1 - 2.0f * (float)mouseY / m_screen_height;

	glm::mat4 invVP = glm::inverse(projection_matrix * view_matrix);
	glm::vec4 screenPos = glm::vec4(mouseXX, mouseYY, 1.0f, 1.0f);
	glm::vec4 worldPos = invVP * screenPos;


	ray_direction = glm::normalize(glm::vec3(worldPos));
	if (state)
		ray_origin = m_camera_position;
	else
		ray_origin = glm::vec3(0);
}

bool Renderer::TestRayOBBIntersection(GameObject * object)
{
	glm::vec3 aabb_min;
	glm::vec3 aabb_max;

	int id = object->getObjectID();
	if (id == GameObject::ROAD_TILE || id == GameObject::FREE_TILE)
	{

		aabb_min = glm::vec3(-1, -1, -1); //glm::vec3(-1.0, -1.2, -1.0);
		aabb_max = glm::vec3(1, 1, 1); //glm::vec3(1.0, 1.2, 1.0);
	}
	else if (id == GameObject::TOWER)
	{
		aabb_min = glm::vec3(-0.7, 1, -2.6373);
		aabb_max = glm::vec3(0.7, 4.5, 2.4856);
	}
	else if (id == GameObject::PIRATE)
	{

	}



	aabb_min *= object->getScaleFactor();
	aabb_max *= object->getScaleFactor();

	glm::mat4 ModelMatrix = object->getTransformationMatrix();

#define INTERSECTION_1
#ifdef INTERSECTION_1
	float tMin = 0.01f;
	float tMax = 15000.0f;
	float tresh = 0.0000001f; //

	glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);
	glm::vec3 delta = OBBposition_worldspace - ray_origin;


	{
		glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
		//std::cout << xaxis.x << " , "<<xaxis.y <<" , "<<xaxis.z << std::endl;
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray_direction, xaxis);

		if (fabs(f) > tresh)
		{ // Standard case

			float t1 = (e + aabb_min.x) / f; 
			float t2 = (e + aabb_max.x) / f; 

			if (t1 > t2) {
				float w = t1;
				t1 = t2;
				t2 = w; 
			}

			if (t2 < tMax)
				tMax = t2;

			if (t1 > tMin)
				tMin = t1;

			if (tMax < tMin)
				return false;

		}
		else
		{
			if (-e + aabb_min.x > 0.0f || -e + aabb_max.x < 0.0f)
				return false;
		}
	}


	{
		glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray_direction, yaxis);

		if (fabs(f) > tresh) {

			float t1 = (e + aabb_min.y) / f;
			float t2 = (e + aabb_max.y) / f;

			if (t1 > t2) { float w = t1;t1 = t2;t2 = w; }

			if (t2 < tMax)
				tMax = t2;
			if (t1 > tMin)
				tMin = t1;
			if (tMin > tMax)
				return false;

		}
		else {
			if (-e + aabb_min.y > 0.0f || -e + aabb_max.y < 0.0f)
				return false;
		}
	}


	{
		glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
		float e = glm::dot(zaxis, delta);
		float f = glm::dot(ray_direction, zaxis);

		if (fabs(f) > tresh) {

			float t1 = (e + aabb_min.z) / f;
			float t2 = (e + aabb_max.z) / f;

			if (t1 > t2) { float w = t1;t1 = t2;t2 = w; }

			if (t2 < tMax)
				tMax = t2;
			if (t1 > tMin)
				tMin = t1;
			if (tMin > tMax)
				return false;

		}
		else {
			if (-e + aabb_min.z > 0.0f || -e + aabb_max.z < 0.0f)
				return false;
		}
	}

	intersection_distance = tMin;
	return true;
#endif

}

bool Renderer::TestRayOBBIntersection(glm::mat4 & model_matrix, float scale_factor2)
{

	glm::vec3 aabb_min;
	glm::vec3 aabb_max;

	aabb_min = glm::vec3(0.06, 0, -0.9);
	aabb_max = glm::vec3(0.07, 0.44, 0.9);

	//scale_factor2 = 0.1625f;
	//aabb_min = glm::vec3(0.08, 0, -2); 
	//aabb_max = glm::vec3(0.15, 0.5, 2);


	glm::vec3 scale_factor = glm::vec3((0.15, 0.25, 0.25)*0.75f);
	aabb_min *= scale_factor;
	aabb_max *= scale_factor;

	glm::mat4 ModelMatrix = model_matrix;

#define INTERSECTION_1
#ifdef INTERSECTION_1
	float tMin = 0.1f;
	float tMax = 1500.0f;

	glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);
	glm::vec3 delta = OBBposition_worldspace - ray_origin;


	{
		glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray_direction, xaxis);

		if (fabs(f) > 0.001f) { // Standard case

			float t1 = (e + aabb_min.x) / f; 
			float t2 = (e + aabb_max.x) / f; 

			if (t1 > t2) {
				float w = t1;t1 = t2;t2 = w; // swap t1 and t2
			}

			if (t2 < tMax)
				tMax = t2;
			if (t1 > tMin)
				tMin = t1;

			if (tMax < tMin)
				return false;

		}
		else { 
			if (-e + aabb_min.x > 0.0f || -e + aabb_max.x < 0.0f)
				return false;
		}
	}


	{
		glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray_direction, yaxis);

		if (fabs(f) > 0.001f) {

			float t1 = (e + aabb_min.y) / f;
			float t2 = (e + aabb_max.y) / f;

			if (t1 > t2) { float w = t1;t1 = t2;t2 = w; }

			if (t2 < tMax)
				tMax = t2;
			if (t1 > tMin)
				tMin = t1;
			if (tMin > tMax)
				return false;

		}
		else {
			if (-e + aabb_min.y > 0.0f || -e + aabb_max.y < 0.0f)
				return false;
		}
	}


	{
		glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
		float e = glm::dot(zaxis, delta);
		float f = glm::dot(ray_direction, zaxis);

		if (fabs(f) > 0.001f) {

			float t1 = (e + aabb_min.z) / f;
			float t2 = (e + aabb_max.z) / f;

			if (t1 > t2) { float w = t1;t1 = t2;t2 = w; }

			if (t2 < tMax)
				tMax = t2;
			if (t1 > tMin)
				tMin = t1;
			if (tMin > tMax)
				return false;

		}
		else {
			if (-e + aabb_min.z > 0.0f || -e + aabb_max.z < 0.0f)
				return false;
		}
	}

	intersection_distance = tMin;
	return true;
#endif
	return false;
}

bool Renderer::CheckForGuiInteraction()
{
	/*
	if (!game->Selecting())
	{
		if (over_tower_icon)
			game->setSelection(true);
		else
			game->setSelection(false);
	}
	else
	{
		game->setSelection(false);
	}
	*/

	if (over_tower_icon && game->towerCanBuy())
		return true;
	else if (game->getFocusedObject() != nullptr)
	{


		if (!game->Selecting())
		{
			if (!game->towerCanMove())
				return false;

			game->setMovingObject(game->getFocusedObject());
			game->removeObject(game->getFocusedObject());
		}

		return true;
	}
	else if (over_meteorite_icon)
		if (game->superPowerCanUse(Game::METEORITE))
			return true;
	
	return false;
		
}

#ifdef COLLISION_TEST
void Renderer::sphere_move_forward(bool enable)
{
	if (enable)
		sphere_direction.z++;
}
void Renderer::sphere_move_backward(bool enable)
{
	if (enable)
		sphere_direction.z--;
}
void Renderer::sphere_move_left(bool enable)
{
	if (enable)
		sphere_direction.x++;
}
void Renderer::sphere_move_right(bool enable)
{
	if (enable)
		sphere_direction.x--;;
}
#endif

#ifdef PIRATE_TEST
void Renderer::PirateRotateLeft(bool enable)
{
	if (enable)
		pirate_rotation += 2.0f;

}

void Renderer::PirateRotateRight(bool enable)
{
	if (enable)
		pirate_rotation -= 2.0f;
}
#endif