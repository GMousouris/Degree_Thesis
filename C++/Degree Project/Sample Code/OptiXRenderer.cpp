#include "Defines.h"
#include "streamHandler.h"
#include "OptiXRenderer.h"
#include <common.h>
#include "Utilities.h"
#include "Mediator.h"


#include "light_definition.h"
#include "ViewportManager.h"
#include "Restriction_Types.h"

#include "GeometryFunctions.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Transformations.h"

#define POST_PROCESS_SELECTED_OBJECT_OUTLINE_SIZE 1.0f
#define POST_PROCESS_SELECTED_OBJECT_OUTLINE_COLOR optix::make_float3(252 / 255.0f , 53 / 255.0f , 162 / 255.0f)

OptiXRenderer::OptiXRenderer() {}

OptiXRenderer::~OptiXRenderer()
{
	
	if (m_optix_init)
	{
		m_context->destroy();
	}

	glDeleteBuffers(1, &m_pboOutputBuffer);
	glDeleteTextures(1, &m_hdrTexture);

	glDeleteBuffers(1, &m_pboDepthBuffer);
	glDeleteTextures(1, &m_DepthTexture);
}


void OptiXRenderer::cleanUp()
{


}

//------------------------------------------------------------------------------
//
// INIT FUNCTIONS
//
//------------------------------------------------------------------------------

bool OptiXRenderer::Init()
{

	// init flags
	m_optix_init = true;
	m_openGL_init = true;
	m_renderer_init = true;
	m_programs_init = true;

	// init everything
	Init_GeneralData();
	Init_OpenGL();
	Init_OptiX();
	Init_Helpers();

	if (!getInitState())
	{
		printInitError();
		return false;
	}

	return true;

}

void OptiXRenderer::Init_GeneralData()
{
#ifdef DEBUGGING 
	streamHandler::Report("OptiXRenderer", "Init_GeneralData", "");
#endif

	//m_width = Mediator::RequestWindowSize()[0];//m_main_app->getWindowWidth();
	//m_height = Mediator::RequestWindowSize()[1];

	m_width  = Mediator::RequestWindowSize(0);
	m_height = Mediator::RequestWindowSize(1);

	
	//g_width  = Mediator::getRenderSize().x;
	//g_height = Mediator::getRenderSize().y;

	VIEWPORT vp = ViewportManager::GetInstance().getActiveOptiXViewport();
	g_width  = vp.renderSize.x;
	g_height = vp.renderSize.y;

	m_stackSize = Mediator::RequestOptiXstackSize();
	m_interop   = Mediator::RequestOpenGL_Interop();

	// Renderer set-up and gui Parameters
	m_focused_object       = -1;
	m_scene_epsilon_factor = 500;
	frame_number           = 1;
	
	// OpenGL shaders
	m_vertexShader   = 0;
	m_fragmentShader = 0;
	m_ShaderProgram  = 0;

	m_post_proc_parameters.m_gamma          = 2.2f;
	m_post_proc_parameters.m_colorBalance   = optix::make_float3(1.0f, 1.0f, 1.0f);
	m_post_proc_parameters.m_whitePoint     = 1.0f;
	m_post_proc_parameters.m_burnHighlights = 0.8f;
	m_post_proc_parameters.m_crushBlacks    = 0.2f;
	m_post_proc_parameters.m_saturation     = 1.2f;
	m_post_proc_parameters.m_brightness     = 0.8f;


	mousehit_data[0] = ZERO_3f;
	mousehit_data[1] = ZERO_3f;
	mousehit_data[2] = ZERO_3f;

	m_pinholeCamera.setViewport(g_width, g_height);


}

void OptiXRenderer::Init_OpenGL()
{

#ifdef DEBUGGING
	streamHandler::Report("OptiXRenderer", "Init_OpenGL", "");
#endif

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//CUSTOM_VIEWPORT viewport = Mediator::getMainViewport();
	
	//optix::float2 m_renderSize = Mediator::getRenderSize();
	VIEWPORT vp = ViewportManager::GetInstance().getActiveOptiXViewport();
	g_width  = vp.renderSize.x;
	g_height = vp.renderSize.y;
	
	//glViewport(viewport.dx, viewport.dy, g_width, g_height);
	glViewport(0, 0, g_width, g_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (m_interop)
	{
		// PBO for the fast OptiX sysOutputBuffer to texture transfer.
		glGenBuffers(1, &m_pboOutputBuffer);
		MY_ASSERT(m_pboOutputBuffer != 0);
		// Buffer size must be > 0 or OptiX can't create a buffer from it.
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboOutputBuffer);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, g_width * g_height * sizeof(float) * 4, (void*)0, GL_STREAM_READ); // RGBA32F from byte offset 0 in the pixel unpack buffer.
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	// glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // default, works for BGRA8, RGBA16F, and RGBA32F.
	glGenTextures(1, &m_hdrTexture);
	MY_ASSERT(m_hdrTexture != 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	// ImGui has been changed to push the GL_TEXTURE_BIT so that this works. 
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	//
	glGenBuffers(1, &m_pboDepthBuffer);
	MY_ASSERT(m_pboDepthBuffer != 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboDepthBuffer);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, g_width * g_height * sizeof(float) * 1, (void*)0, GL_STREAM_READ); // RGBA32F from byte offset 0 in the pixel unpack buffer.
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	//

	glGenTextures(1, &m_DepthTexture);
	MY_ASSERT(m_DepthTexture != 0);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	Init_GLSL();
}

void OptiXRenderer::Init_GLSL()
{

#ifdef DEBUGGING
	streamHandler::Report("OptiXRenderer", "Init_GLSL", "");
#endif

	static const std::string vsSource =
		"#version 330\n"
		"layout(location = 0) in vec4 attrPosition;\n"
		"layout(location = 8) in vec2 attrTexCoord0;\n"
		"out vec2 varTexCoord0;\n"
		"void main()\n"
		"{\n"
		"  gl_Position  = attrPosition;\n"
		"  varTexCoord0 = attrTexCoord0;\n"
		"}\n";

	
	static const std::string fsSource =

		"#version 330\n"
		"uniform sampler2D samplerHDR;\n"
		"uniform vec3  colorBalance;\n"
		"uniform float invWhitePoint;\n"
		"uniform float burnHighlights;\n"
		"uniform float saturation;\n"
		"uniform float crushBlacks;\n"
		"uniform float invGamma;\n"
		"in vec2 varTexCoord0;\n"
		"uniform int selected;\n"
		"uniform int focused_index;\n"
		"uniform int  highlight_outline_kernel_size;\n"
		"uniform vec3 highlight_outline_color; \n"
		"layout(location = 0, index = 0) out vec4 outColor;\n"


		"vec3 clamp3f(vec3 vec){\n"
		"    for(int i = 0; i<3; i++)\n"
		"    {\n"
		"        vec[i] = clamp(vec[i] , 0.0f , 1.0f);\n"
		"    }\n"
		"    return vec;\n"
		"}\n"


		"void main()\n"
		"{\n"
		"  vec3 hdrColor = texture(samplerHDR, varTexCoord0).rgb;\n"
		"  float alpha   = texture(samplerHDR, varTexCoord0).a;\n"
		"  vec3 ldrColor = invWhitePoint * colorBalance * hdrColor;\n"
		"  ldrColor *= (ldrColor * burnHighlights + 1.0) / (ldrColor + 1.0);\n"
		"  float luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));\n"
		"  ldrColor = max(mix(vec3(luminance), ldrColor, saturation), 0.0);\n"
		"  luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));\n"
		"  if (luminance < 1.0)\n"
		"  {\n"
		"    ldrColor = max(mix(pow(ldrColor, vec3(crushBlacks)), ldrColor, sqrt(luminance)), 0.0);\n"
		"  }\n"
		"  ldrColor = pow(ldrColor, vec3(invGamma));\n"
		"\n"
		"  outColor = vec4(ldrColor, 1.0f);\n"

		"}\n";

	GLint vsCompiled = 0;
	GLint fsCompiled = 0;

	m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (m_vertexShader)
	{
		GLsizei len = (GLsizei)vsSource.size();
		const GLchar *vs = vsSource.c_str();
		glShaderSource(m_vertexShader, 1, &vs, &len);
		glCompileShader(m_vertexShader);
		checkInfoLog(vs, m_vertexShader);

		glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &vsCompiled);
		MY_ASSERT(vsCompiled);
	}

	m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (m_fragmentShader)
	{
		GLsizei len = (GLsizei)fsSource.size();
		const GLchar *fs = fsSource.c_str();
		glShaderSource(m_fragmentShader, 1, &fs, &len);
		glCompileShader(m_fragmentShader);
		checkInfoLog(fs, m_fragmentShader);

		glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &fsCompiled);
		MY_ASSERT(fsCompiled);
	}

	m_ShaderProgram = glCreateProgram();
	if (m_ShaderProgram)
	{
		GLint programLinked = 0;

		if (m_vertexShader && vsCompiled)
		{
			glAttachShader(m_ShaderProgram, m_vertexShader);
		}
		if (m_fragmentShader && fsCompiled)
		{
			glAttachShader(m_ShaderProgram, m_fragmentShader);
		}

		glLinkProgram(m_ShaderProgram);
		checkInfoLog("m_ShaderProgram", m_ShaderProgram);

		glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &programLinked);
		MY_ASSERT(programLinked);

		if (programLinked)
		{
			glUseProgram(m_ShaderProgram);

			glUniform1i(glGetUniformLocation(m_ShaderProgram, "samplerHDR"), 0);       // texture image unit 0
			glUniform1f(glGetUniformLocation(m_ShaderProgram, "invGamma"), 1.0f / m_post_proc_parameters.m_gamma);
			glUniform3f(glGetUniformLocation(m_ShaderProgram, "colorBalance"), m_post_proc_parameters.m_colorBalance.x, m_post_proc_parameters.m_colorBalance.y, m_post_proc_parameters.m_colorBalance.z);
			glUniform1f(glGetUniformLocation(m_ShaderProgram, "invWhitePoint"), m_post_proc_parameters.m_brightness / m_post_proc_parameters.m_whitePoint);
			glUniform1f(glGetUniformLocation(m_ShaderProgram, "burnHighlights"), m_post_proc_parameters.m_burnHighlights);
			glUniform1f(glGetUniformLocation(m_ShaderProgram, "crushBlacks"), m_post_proc_parameters.m_crushBlacks + m_post_proc_parameters.m_crushBlacks + 1.0f);
			glUniform1f(glGetUniformLocation(m_ShaderProgram, "saturation"), m_post_proc_parameters.m_saturation);
			glUniform1i(glGetUniformLocation(m_ShaderProgram, "highlight_outline_kernel_size"), POST_PROCESS_SELECTED_OBJECT_OUTLINE_SIZE);
			glUniform3f(glGetUniformLocation(m_ShaderProgram, "highlight_outline_color"), POST_PROCESS_SELECTED_OBJECT_OUTLINE_COLOR.x, POST_PROCESS_SELECTED_OBJECT_OUTLINE_COLOR.y, POST_PROCESS_SELECTED_OBJECT_OUTLINE_COLOR.z);
			glUseProgram(0);

		}
	}

}

void OptiXRenderer::Init_OptiX()
{
#ifdef DEBUGGING
	streamHandler::Report("OptiXRenderer", "Init_Optix", "");
#endif


	try
	{
		// create Optix::Context
		m_context = optix::Context::create();
		//m_context->setPrintEnabled(1);
		//m_context->setPrintBufferSize(4096);
		//m_context->setExceptionEnabled(RT_EXCEPTION_ALL, true);
		

		// Init Programs , Renderer , Scene 
		Init_OptiXShaders();
		Init_Programs();
		load_OptiXShader(m_active_shader, false);
		Init_LightSamplePrograms();
		Init_Renderer();

	}
	catch (optix::Exception& e)
	{
		std::cerr << e.getErrorString() << std::endl;
		m_optix_init = false;
	}

	

}

void OptiXRenderer::Init_OptiXShaders()
{
	
	optix_shaders["phong"] = OptiXShader("phong");
	optix_shaders["pathtrace_diffuse"] = OptiXShader("pathtrace_diffuse");
	optix_shaders["pathtrace_complete"] = OptiXShader("pathtrace_complete");
	
	//m_active_shader = "phong";
	//m_active_shader = "pathtrace_diffuse";
	m_active_shader = "pathtrace_complete";
}

void OptiXRenderer::Init_Programs()
{
#ifdef DEBUGGING
	streamHandler::Report("OptiXRenderer", "Init_Programs", "");
#endif

	try
	{

		//------------------------------------------------------------------------------
		//
		//                   Util : Programs
		//
		//------------------------------------------------------------------------------

		// mouse_ray_generation Program
		// Global mouse_ray_generation program for each OptiXShader
		
		m_ProgramsMap["mouse_ray_generation"] = m_context->createProgramFromPTXFile(Utilities::getPtxPath("mouse_ray_generation.cu"), "mouse_ray_generation");
		m_ProgramsMap["mouse_ray_generation"]->validate();


		// For each OptiXShader shader :
		// load shader.ray_gen Program
		// load shader.closest_hit Program
		// load shader.any_hit Program
		// load shader.miss Program
		std::string shader_id;
		for (auto& it : optix_shaders) 
		{
			OptiXShader shader = it.second;
			shader_id = shader.shader_id;

			// ray_generation Program
			m_ProgramsMap["ray_gen_" + shader_id] = m_context->createProgramFromPTXFile(Utilities::getPtxPath(std::string(shader.data.ray_gen_path.c_str() + std::string(".cu")).c_str()), shader.data.ray_gen_function.c_str());
			m_ProgramsMap["ray_gen_" + shader_id]->validate();

			// closest_hit Program
			m_ProgramsMap["closest_hit_" + shader_id ] = m_context->createProgramFromPTXFile(Utilities::getPtxPath(std::string(shader.data.closest_hit_path.c_str() + std::string(".cu")).c_str()), "closest_hit" );
			m_ProgramsMap["closest_hit_" + shader_id ]->validate();

			// closest_hit Program
			m_ProgramsMap["closest_hit_textured_" + shader_id ] = m_context->createProgramFromPTXFile(Utilities::getPtxPath(std::string(shader.data.closest_hit_path.c_str() + std::string(".cu")).c_str()), "closest_hit_textured");
			m_ProgramsMap["closest_hit_textured_" + shader_id ]->validate();

			// any_hit Program
			m_ProgramsMap["any_hit_" + shader_id ] = m_context->createProgramFromPTXFile(Utilities::getPtxPath(std::string(shader.data.any_hit_path.c_str() + std::string(".cu")).c_str()), "any_hit");
			m_ProgramsMap["any_hit_" + shader_id ]->validate();

			// miss Program
			m_ProgramsMap["miss_" + shader_id ] = m_context->createProgramFromPTXFile(Utilities::getPtxPath(std::string(shader.data.miss_path.c_str() + std::string(".cu")).c_str()), "miss");
			m_ProgramsMap["miss_" + shader_id ]->validate();

			// exception Program
			m_ProgramsMap["exception_" + shader_id ] = m_context->createProgramFromPTXFile(Utilities::getPtxPath(std::string(shader.data.exception_path.c_str() + std::string(".cu")).c_str()), "exception");
			m_ProgramsMap["exception_" + shader_id ]->validate();

		}
		

		
		

	}
	catch (optix::Exception& e)
	{

		std::cerr << e.getErrorString() << std::endl;
		m_programs_init = false;
	}


}

void OptiXRenderer::Init_LightSamplePrograms()
{
	try
	{
		//m_bufferSampleLight_programs = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_PROGRAM_ID, 2);
		//int* sampleLight = (int*)m_bufferSampleLight_programs->map(0, RT_BUFFER_MAP_WRITE_DISCARD);

		//sampleLight[LIGHT_ENVIRONMENT] = RT_PROGRAM_ID_NULL;
		//sampleLight[LIGHT_PARALLELOGRAM] = RT_PROGRAM_ID_NULL;
		//optix::Program prg;
		//RTprogram prg_rt;
		////std::cout << Utilities::getPtxPath("asd");
		//prg = m_context->createProgramFromPTXFile(Utilities::getPtxPath("light_sample.cu"), "sample_light_parallelogram");
		//std::string path = Utilities::get
		//rtProgramCreateFromPTXFile(
		//	m_context->get(), path.c_str(), "sample_light_parallelogram", &prg_rt);
		//rtProgramValidate(prg_rt);

		//optix::Program(prg_rt);
		//sampleLight[LIGHT_ENVIRONMENT] = prg->getId();

		//m_context->createProgramFromPTXFile(Utilities::getPtxPath("light_sample.cu"), "sample_light_parallelogram");
		//m_ProgramsMap["sample_light_parallelogram"] = prg;
		//sampleLight[LIGHT_PARALLELOGRAM] = prg->getId();

		//m_bufferSampleLight_programs->unmap();
		//m_context["sys_SampleLight"]->setBuffer(m_bufferSampleLight_programs);
	}
	catch (optix::Exception& e)
	{
		std::cerr << e.getErrorString() << std::endl;
		m_optix_init = false;
	}
}

void OptiXRenderer::Init_Renderer()
{

#ifdef DEBUGGING
	streamHandler::Report("OptiXRenderer", "Init_Renderer", "");
#endif
	try
	{
		m_context->setRayTypeCount(2);
		m_context->setEntryPointCount(4);
		m_context->setStackSize(m_stackSize);


		m_OutputBuffer = (m_interop) ? m_context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, m_pboOutputBuffer)
			: m_context->createBuffer(RT_BUFFER_OUTPUT);
		m_OutputBuffer->setFormat(RT_FORMAT_FLOAT4); // RGBA32F
		m_OutputBuffer->setSize(g_width, g_height);
		m_context["sys_OutputBuffer"]->set(m_OutputBuffer);

		m_Optix_DepthBuffer = m_context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, m_pboDepthBuffer);
		m_Optix_DepthBuffer->setFormat(RT_FORMAT_FLOAT);
		m_Optix_DepthBuffer->setSize(g_width, g_height);
		m_context["sys_DepthBuffer"]->set(m_Optix_DepthBuffer);

		m_mouse_hit_buffer = m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT3);
		m_mouse_hit_buffer->setSize(3);
		m_context["sys_mouseHit_buffer_Output"]->set( m_mouse_hit_buffer );
		

		m_cast_dir_position = m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT3);
		m_cast_dir_position->setSize(1);
		m_context["sys_custom_dir_Output"]->set( m_cast_dir_position );
		
		m_SelectedObjectBuffer = m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT);
		m_SelectedObjectBuffer->setFormat(RT_FORMAT_INT);
		m_SelectedObjectBuffer->setSize(1);
		m_context["sys_SelectedObjectBuffer"]->set(m_SelectedObjectBuffer);

		float color_u = 8.0f / 255.0f;
		float color_v = 50.0f / 255.0f;
		m_context["bg_color"]->setFloat( color_u, color_u, color_u);
		m_context["bg_color2"]->setFloat(color_v, color_v, color_v);
		m_context["bad_color"]->setFloat(1.0f, 0.0f, 1.0f);
		

		m_context["cast_shadow_ray"]->setInt(0);
		m_context["sys_cast_reflections"]->setInt(0);
		m_context["sys_sample_light_refl"]->setUint(0);
		m_context["sys_bsdf_light_refl"]->setUint(0);
		m_context["sys_use_fresnel_equations"]->setInt(0);
		m_context["sys_sample_glossy_index"]->setUint(0);
		m_context["sys_const_light_model"]->setInt(0);
		m_context["sys_IterationIndex"]->setUint(0);
		m_context["sys_utility_ray"]->setInt(0);

		m_context["sys_only_write_depth"]->setInt(0);
		m_context["sys_write_depth"]->setInt(1);


		//------------------------------------------------------------------------------
		//
		//                       BIND PROGRAMS TO CONTEXT
		//
		//------------------------------------------------------------------------------

		// Load Mouse_Ray Generation Program
		std::map<std::string, optix::Program>::const_iterator it = m_ProgramsMap.find("mouse_ray_generation");
		MY_ASSERT(it != m_ProgramsMap.end());
		m_context->setRayGenerationProgram(0, it->second); // entrypoint

		// load Ray Generation Programs for each OptiXShader
		std::string shader_id;
		int index;
		for (auto& shader_index : optix_shaders)
		{
			shader_id = shader_index.second.shader_id;
			index = shader_index.second.index;
			it = m_ProgramsMap.find("ray_gen_" + shader_id);
			MY_ASSERT(it != m_ProgramsMap.end());
			m_context->setRayGenerationProgram(index + 1, it->second);
		}

		load_Programs(m_active_shader);


		// Camera Initialization
		optix::float3 cameraPosition;
		optix::float3 cameraU;
		optix::float3 cameraV;
		optix::float3 cameraW;
		m_pinholeCamera.getFrustum(cameraPosition, cameraU, cameraV, cameraW);
		m_context["sys_CameraPosition"]->setFloat(cameraPosition);
		m_context["sys_CameraU"]->setFloat(cameraU);
		m_context["sys_CameraV"]->setFloat(cameraV);
		m_context["sys_CameraW"]->setFloat(cameraW);

	}
	catch (optix::Exception& e)
	{
		std::cerr << e.getErrorString() << std::endl;
		m_renderer_init = false;
	}


}

void OptiXRenderer::Init_Helpers()
{

}

void OptiXRenderer::load_OptiXShader(std::string shader_id, bool set_materials)
{
	

	m_active_shader = shader_id;
	OptiXShader shader = optix_shaders[shader_id];
	
	//m_context["sys_abort"]->setUint(1);
	m_context["shader_index"]->setUint(shader.index);
	m_context["min_depth"]->setUint(shader.data.min_depth);
	m_context["max_depth"]->setUint(shader.data.max_depth);
	m_context["sys_SceneEpsilon"]->setFloat(shader.data.scene_epsilon);
	m_context["sqrt_num_samples"]->setUint(shader.data.sqrt_num_samples);
	m_context["rr_begin_depth"]->setUint(shader.data.rr_begin_depth);
	//m_context->setMaxTraceDepth(shader.data.max_depth);

	if (set_materials)
	{
		load_Programs(shader_id);
		std::vector<SceneObject *>& scene_objects = Mediator::RequestSceneObjects();
		for (int i = 0; i < scene_objects.size(); i++)
		{
			int mat_count = scene_objects[i]->getMaterialCount();
			for (int mat_ind = 0; mat_ind < mat_count; mat_ind++)
			{
				bool has_tex = scene_objects[i]->MaterialHasTex(mat_ind);
				if (has_tex)
					scene_objects[i]->getMaterial(mat_ind)->setClosestHitProgram(0, getActiveClosestHitTexturedProgram());
				else
					scene_objects[i]->getMaterial(mat_ind)->setClosestHitProgram(0, getActiveClosestHitProgram());
				scene_objects[i]->getMaterial(mat_ind)->setAnyHitProgram(1, getActiveAnyHitProgram());
			}
		}
	}

}

void OptiXRenderer::load_Programs(std::string shader_id)
{
	// currently only loading to memory OptiXShader's miss program && OptiXShader's Exception Program

	// Miss Program
	m_context->setMissProgram(0, m_ProgramsMap["miss_" + shader_id]); // raytype

	// Exception Program
	m_context->setExceptionProgram(0, m_ProgramsMap["exception_" + shader_id]); // raytype

}



//------------------------------------------------------------------------------
//
// UPDATE - RENDER FUNCTIONS
//
//------------------------------------------------------------------------------

void OptiXRenderer::Update(float dt)
{

#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::Update()" << std::endl;
#endif

	if (Mediator::is_AppWindowSizeChanged() || true)
	{
		////std::cout << "asas" << std::endl;
		ResizeBuffers();
	}
	
	
	//launch_Utility_Rays();
	resetFocusedObject();
	launchMouseRay();

	//
	//post_process_MouseRay_Data();

}

void OptiXRenderer::ResizeBuffers()
{

#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::Reshape()" << std::endl;
#endif

	//int  window_size[] = { Mediator::RequestWindowSize(0) , Mediator::RequestWindowSize(1) };

	//optix::float2 m_renderSize = Mediator::getRenderSize();
	VIEWPORT vp = ViewportManager::GetInstance().getActiveOptiXViewport();
	
	int width  = vp.renderSize.x;
	int height = vp.renderSize.y;
	
	if ((width != 0 && height != 0) && // 
		(g_width != width || g_height != height) )
	{ 
		////std::cout << "\n\n new size : ( " << width << " , " << height << " ) " << std::endl;
		////std::cout << " old size : ( " << g_width << " , " << g_height << " ) " << std::endl;
		g_width  = vp.renderSize.x;
		g_height = vp.renderSize.y;


		//g_width  = m_width;
		//g_height = m_height;

		
		//g_width = Mediator::getMainViewport();
		////std::cout << "\n" << std::endl;
		////std::cout << " g_width : " << g_width << std::endl;
		////std::cout << " g_height : " << g_height << std::endl;
		
		glViewport(0, 0, g_width , g_height);
		try
		{
			m_OutputBuffer->setSize(g_width, g_height); // RGBA32F buffer.
			m_OutputBuffer->unregisterGLBuffer(); // Must unregister or CUDA won't notice the size change and crash.
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_OutputBuffer->getGLBOId());
			glBufferData(GL_PIXEL_UNPACK_BUFFER, m_OutputBuffer->getElementSize() * g_width * g_height, nullptr, GL_STREAM_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			m_OutputBuffer->registerGLBuffer();

			m_Optix_DepthBuffer->setSize(g_width, g_height); // RGBA32F buffer.
			m_Optix_DepthBuffer->unregisterGLBuffer(); // Must unregister or CUDA won't notice the size change and crash.
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_Optix_DepthBuffer->getGLBOId());
			glBufferData(GL_PIXEL_UNPACK_BUFFER, m_Optix_DepthBuffer->getElementSize() * g_width * g_height, nullptr, GL_STREAM_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			m_Optix_DepthBuffer->registerGLBuffer();

		}
		catch (optix::Exception& e)
		{
			std::cerr << e.getErrorString() << std::endl;
		}


		


		m_pinholeCamera.setViewport(g_width, g_height);
		restartAccumulation();
	}

	//delete window_size;
}

void OptiXRenderer::ResizeBuffers(int width, int height)
{

#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::Reshape()" << std::endl;
#endif

	
	if ((width != 0 && height != 0) && // 
		(m_width != width || m_height != height))
	{
		
		m_width  = width;
		m_height = height;

		glViewport(0, 0, m_width, m_height);
		try
		{
			m_OutputBuffer->setSize(m_width, m_height); // RGBA32F buffer.
			m_OutputBuffer->unregisterGLBuffer(); // Must unregister or CUDA won't notice the size change and crash.
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_OutputBuffer->getGLBOId());
			glBufferData(GL_PIXEL_UNPACK_BUFFER, m_OutputBuffer->getElementSize() * m_width * m_height, nullptr, GL_STREAM_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			m_OutputBuffer->registerGLBuffer();

			m_Optix_DepthBuffer->setSize(g_width, g_height); // RGBA32F buffer.
			m_Optix_DepthBuffer->unregisterGLBuffer(); // Must unregister or CUDA won't notice the size change and crash.
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_Optix_DepthBuffer->getGLBOId());
			glBufferData(GL_PIXEL_UNPACK_BUFFER, m_Optix_DepthBuffer->getElementSize() * g_width * g_height, nullptr, GL_STREAM_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			m_Optix_DepthBuffer->registerGLBuffer();

		
		}
		catch (optix::Exception& e)
		{
			std::cerr << e.getErrorString() << std::endl;
		}

		m_pinholeCamera.setViewport(m_width, m_height);
		restartAccumulation();
	}

	//delete window_size;
}


bool OptiXRenderer::Render()
{
#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::Render()" << std::endl;
#endif

	
	bool repaint = false;

	try
	{


		optix::float3 cameraPosition;
		optix::float3 cameraU;
		optix::float3 cameraV;
		optix::float3 cameraW;

		VIEWPORT optixViewport = ViewportManager::GetInstance().getActiveOptiXViewport();
		PinholeCamera camera   = ViewportManager::GetInstance().getViewportCamera( optixViewport.id );

		Render_Settings r_settings = optixViewport.render_settings;
		int r_bbox = 0;
		if (r_settings.draw_bbox_only) r_bbox = 1;
		else if (r_settings.draw_bbox_and_object) r_bbox = 2;

		//bool cameraChanged = m_pinholeCamera.getFrustum(cameraPosition, cameraU, cameraV, cameraW);
		bool changed = camera.isChanged( true );
		if ( changed )
		{
			//camera.getFrustum(cameraPosition, cameraU, cameraV, cameraW);
			cameraPosition = camera.getCameraPos();
			cameraU = camera.getU();
			cameraV = camera.getV();
			cameraW = camera.getW();
			m_context["sys_CameraPosition"]->setFloat(cameraPosition);
			m_context["sys_CameraU"]->setFloat(cameraU);
			m_context["sys_CameraV"]->setFloat(cameraV);
			m_context["sys_CameraW"]->setFloat(cameraW);
			camera.reset();

			restartAccumulation();
		}

		m_context["frame_number"]->setUint( frame_number );
		frame_number++;

		
		//bool m_scene_isDirty = Mediator::isSceneDirty();
		int max_depth        = (int) m_context["max_depth"]->getUint();
		bool needs_repaint   = frame_number > 50 && max_depth <= 1 ? false : true;
		
		if (needs_repaint)
		{
			// write only Depth ? //
			//if (r_bbox == 1) 
			//	m_context["sys_only_write_depth"]->setInt(1);
			
			
			if (r_bbox == 1) Mediator::toggle_Bbox_Only_Scene_Render(true);

			// pass the proj - view matrix to Shader //
			{
				glm::vec3 pos, up, w, target;
				pos = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(camera.getCameraPos());
				up = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(camera.getV());
				w = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(camera.getW());
				target = pos + normalize(w) * 10.0f;

				glm::mat4 scene_view_matrix = glm::lookAt(pos, target, up);
				glm::mat4 projection_matrix = glm::perspective(glm::radians(M_FOV), optixViewport.renderSize.x / optixViewport.renderSize.y, NEAR, FAR);
				glm::mat4 proj_view_g = projection_matrix * scene_view_matrix;

				optix::Matrix4x4 proj_view_mat = MATRIX_TYPE_TRANSFORM::TO_OPTIX_MATRIX(proj_view_g);
				m_context["sys_vp_mat"]->setMatrix4x4fv(false, proj_view_mat.getData());
			}

			// Ray Trace Scene //
			m_context->launch(optix_shaders[m_active_shader].index + 1, g_width, g_height);

			// Re - Trace this time only Scene Grid
			if (r_bbox == 1)
			{

				//m_context["sys_only_write_depth"]->setInt(0);
				//m_context["sys_write_depth"]->setInt(0);
				//Mediator::toggle_Bbox_Only_Scene_Render(true);

				// Ray Trace Scene //
				//m_context->launch(optix_shaders[m_active_shader].index + 1, g_width, g_height);
			    

				//m_context["sys_write_depth"]->setInt(1);
				Mediator::toggle_Bbox_Only_Scene_Render(false);
			}

		}

		{

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_hdrTexture); 
			{
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_OutputBuffer->getGLBOId());
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei)g_width, (GLsizei)g_height, 0, GL_RGBA, GL_FLOAT, (void*)0); // RGBA32F from byte offset 0 in the pixel unpack buffer.
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			}

			glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
			{
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_Optix_DepthBuffer->getGLBOId());
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24 , (GLsizei)g_width, (GLsizei)g_height, 0, GL_DEPTH_COMPONENT , GL_FLOAT, (void*)0); // RGBA32F from byte offset 0 in the pixel unpack buffer.
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			}
			

			repaint = true; 
		}


	}
	catch (optix::Exception& e)
	{
		std::cerr << e.getErrorString() << std::endl;
	}

	

	return repaint;
}

void OptiXRenderer::Display()
{
#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::Display()" << std::endl;
#endif

	/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_hdrTexture);

	glUseProgram(m_ShaderProgram);

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

	glUseProgram(0);
	*/

}

void OptiXRenderer::launchMouseRay()
{
#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::launchMouseRay()" << std::endl;
#endif

	////std::cout << "\n - launchMouseRay():" << std::endl;

	int focused_viewport_id  = ViewportManager::GetInstance().getFocusedViewport();
	VIEWPORT active_viewport = ViewportManager::GetInstance().getActiveViewport();
	Render_Settings r_settings = active_viewport.render_settings;

	if (focused_viewport_id == active_viewport.id)// && m_focused_object < 0 )
	{

		optix::float2 mouse_pos = Utilities::getMousePosRelativeToViewport(active_viewport.id);
		optix::float3 cameraPosition;
		optix::float3 cameraU;
		optix::float3 cameraV;
		optix::float3 cameraW;

		PinholeCamera camera = ViewportManager::GetInstance().getViewportCamera(active_viewport.id);
		cameraPosition = camera.getCameraPos();
		cameraU = camera.getU();
		cameraV = camera.getV();
		cameraW = camera.getW();

		m_context["sys_m_CameraPosition"]->setFloat(cameraPosition);
		m_context["sys_m_CameraU"]->setFloat(cameraU);
		m_context["sys_m_CameraV"]->setFloat(cameraV);
		m_context["sys_m_CameraW"]->setFloat(cameraW);

		m_context["sys_mousePos"]->set2fv((float*)&mouse_pos);
		m_context["sys_m_width"]->setUint(active_viewport.renderSize.x);
		m_context["sys_m_height"]->setUint(active_viewport.renderSize.y);


		bool is_selected_obj  = Mediator::RequestSelectedObject() == nullptr ? false : true;
		bool is_smplr         = is_selected_obj ? Mediator::RequestSelectedObject()->getType() == SAMPLER_OBJECT : false;
		bool is_FaceVector    = is_selected_obj ? Mediator::RequestSelectedObject()->getType() == FACE_VECTOR : false;
		bool has_any_samplers = is_selected_obj ? Mediator::RequestSelectedObject()->get_Samplers().size() > 0 || is_smplr : false;
		int r_q    = 0;
		int r_spot = 0;
		int r_s    = 0;
		int r_p    = 0;
		int r_lights = 0;
		int r_samplers = 0;

		if (r_settings.draw_lights_all) 
		{
			r_lights = 1;
			r_q      = 1;
			r_spot   = 1;
			r_s      = 1;
			r_p      = 1;
		}
		if (r_settings.draw_lights_quad)   r_lights = r_q = 1;
		if (r_settings.draw_lights_spot)   r_lights = r_spot = 1;
		if (r_settings.draw_lights_sphere) r_lights = r_s = 1;
		if (r_settings.draw_lights_point)  r_lights = r_p = 1;

		if (r_settings.draw_sampler_all) r_samplers = 2;
		else if (   r_settings.draw_sampler_only_selected
				 || is_smplr
				 )  r_samplers = 1;

		bool is_render_fv = r_settings.draw_face_vectors_all 
			|| (r_settings.draw_face_vectors_only_selected && is_FaceVector)
			|| (r_settings.draw_face_vectors_only_selected && is_selected_obj);

		int  r_restrictions = 0;
		bool r_restrictions_draw_fv = r_settings.draw_restriction_fv;
		Restriction* UI_selected_Restriction = Mediator::get_UI_Selected_Restriction();
		if (r_restrictions_draw_fv)
		{
			if (r_settings.draw_restrictions_all)
				r_restrictions = 2;
			else if (r_settings.draw_restrictions_only_selected == 1)
				r_restrictions = 1;
			else if (r_settings.draw_restrictions_only_face_vector_selected)
				r_restrictions = 3;

			// kanw mouse pick ola ta face vectors ths skhnhs //
			if (r_restrictions == 2)
			{

			}
			// kanw mouse pick ola ta face vector poy summetexoun sta restrictions toy selected object //
			if (r_restrictions == 1)
			{

			}
		}

		FaceVector_Intersection_State FVI_state = Mediator::Request_FaceVector_Intersection_State();
		bool is_FVI_state = (FVI_state != FaceVector_Intersection_State::FIS_NONE) || is_render_fv || r_restrictions != 0 || (UI_selected_Restriction != nullptr && r_restrictions_draw_fv);
		bool is_samplers  = (r_samplers == 1 && has_any_samplers) || (r_samplers == 2);
		bool is_lights    = (r_lights != 0);

		if (is_FVI_state 
			|| is_samplers 
			//|| is_lights
			)
		{
			
			Mediator::toggle_Utility_Root_Node_State(true);
			if ( is_FVI_state ) Mediator::toggle_FaceVector_State( true );
			if ( is_samplers )  Mediator::toggle_Samplers_State( true, r_samplers);
			//if ( is_lights )    Mediator::toggle_LightObjects( true, r_q, r_spot, r_s, r_p );
			m_context->launch(0, 1);
			if ( is_samplers )  Mediator::toggle_Samplers_State( false, r_samplers);
			if ( is_FVI_state ) Mediator::toggle_FaceVector_State( false );
			//if ( is_lights )    Mediator::toggle_LightObjects( false , r_q, r_spot, r_s, r_p);
			Mediator::toggle_Utility_Root_Node_State( false );
			retrieveFocusedObject();
			int prev_m_focused_object = m_focused_object;
			SceneObject* prev_selected_obj = Mediator::RequestSelectedObject();
			
			// 
			std::vector<SceneObject*> valid_facevectors;
			std::vector<SceneObject*> valid_samplers;
			std::vector<SceneObject*> valid_lights;
			if (
				((r_settings.draw_face_vectors_only_selected || r_restrictions != 0 || (UI_selected_Restriction != nullptr && r_restrictions_draw_fv))
				&& FVI_state == FIS_NONE)
				|| is_samplers
				//|| is_lights
				)
			{
				int focused_index = Mediator::requestObjectsArrayIndex(m_focused_object);
				SceneObject* focused_faceVector = Mediator::RequestSceneObjects()[focused_index];
				SceneObject* selected_object    = Mediator::RequestSelectedObject();
				bool is_valid_fv      = false;
				bool is_valid_sampler = false;
				bool is_valid_light   = false;
				bool is_selected   = selected_object != nullptr;
				bool is_faceVector = is_selected ? selected_object->getType() == FACE_VECTOR : false;
				bool is_Sampler    = is_selected ? selected_object->getType() == SAMPLER_OBJECT : false;
				bool is_Light      = is_selected ? selected_object->getType() == LIGHT_OBJECT : false;
				int selected_id    = is_selected ? selected_object->getId() : -1;
				int faceVector_id  = is_faceVector ? selected_id : -1;
				int sampler_id     = is_Sampler    ? selected_id : -1;
				int light_id       = is_Light      ? selected_id : -1;

				if (is_faceVector || is_Sampler)
					selected_id = selected_object->getParent()->getId();

				//                              //
				// ------- FACE VECTORS ------- //
				//                              //
				if      (r_settings.draw_face_vectors_only_selected && is_selected)
				{
					SceneObject* selected_obj = is_faceVector ? selected_object->getParent() : selected_object;
					for (SceneObject* fv : selected_obj->get_Face_Vectors())
					{

						int prev_selected_id = Mediator::Request_INTERFACE_prev_Selected_Object_Id();
						SceneObject* prev_selected_object = Mediator::Request_INTERFACE_prev_Selected_Object();
						bool is_parent_selected = selected_id == fv->getParent()->getId();
						bool was_parent_selected = prev_selected_id == fv->getParent()->getId();


						if (is_faceVector && (fv->getId() == selected_object->getId()) || !is_faceVector)
						{

							if (!is_faceVector)
							{
								if ((was_parent_selected) && (is_parent_selected))
									valid_facevectors.push_back(fv);
							}
							else
								valid_facevectors.push_back(fv);

							//valid_facevectors.push_back(fv);
						}
					}
				}
				else if (r_settings.draw_face_vectors_all)
				{
					for (SceneObject* obj : Mediator::RequestSceneObjects())
					{
						bool is_valid = obj->isActive();
						if (!is_valid)
							continue;

						for (SceneObject* fv : obj->get_Face_Vectors())
							valid_facevectors.push_back(fv);
					}
				}
				if (r_restrictions != 0)
				{
					std::vector<Restriction*> restrictions = Mediator::Request_Scene_Restrictions();
					for (Restriction* R : restrictions)
					{
						bool r_is_valid = R->is_Active() && R->isEnabled() && R->is_Renderable();
						if (!r_is_valid)
							continue;

						SceneObject* obj = R->getObject();
						SceneObject* sub = R->getSubject();
						SceneObject* A = R->get_Owner(0);
						SceneObject* B = R->get_Owner(1);
						Restriction_ID r_id = R->getId();

						bool is_owner_selected = (A->getId() == selected_id) || (B->getId() == selected_id);
						bool is_selected_Fv_in_R = (obj->getId() == faceVector_id) || (sub->getId() == faceVector_id);
						if (r_id == FOCUS_R)
						{
							if (
								((r_restrictions == 1 && is_owner_selected) || r_restrictions == 2)
								|| (r_restrictions == 3 && is_selected_Fv_in_R)
								)
								valid_facevectors.push_back(sub);

						}
						else if (r_id == CONVERSATION_R)
						{
							if (
								((r_restrictions == 1 && is_owner_selected) || r_restrictions == 2)
								|| (r_restrictions == 3 && is_selected_Fv_in_R)
								)
							{
								valid_facevectors.push_back(obj);
								valid_facevectors.push_back(sub);
							}
						}
						else if (r_id == DISTANCE_R)
						{

						}

					}

				}
				if (UI_selected_Restriction != nullptr && r_restrictions_draw_fv)
				{
					Restriction* R = UI_selected_Restriction;
					bool r_is_valid = R->is_Active() && R->isEnabled() && R->is_Renderable();
					if (r_is_valid)
					{
						SceneObject* obj = R->getObject();
						SceneObject* sub = R->getSubject();
						Restriction_ID r_id = R->getId();

						if (r_id == FOCUS_R)
						{
							valid_facevectors.push_back(sub);
						}
						else if (r_id == CONVERSATION_R)
						{
							valid_facevectors.push_back(obj);
							valid_facevectors.push_back(sub);
						}
						else if (r_id == DISTANCE_R)
						{

						}
					}
				}
				for (SceneObject* fv : valid_facevectors)
				{
					int fv_id = fv->getId();
					if (fv_id == m_focused_object)
					{
						is_valid_fv = true;
						break;
					}
				}

				//                          //
				// ------- SAMPLERS ------- //
				//                          //
				if      (r_samplers == 1 && is_selected)
				{
					SceneObject* selected_obj = is_Sampler ? selected_object->getParent() : selected_object;

					for (SceneObject* smplr : selected_obj->get_Samplers())
					{
						bool is_valid = smplr->isActive();
						if (!is_valid)
							continue;

						int prev_selected_id = Mediator::Request_INTERFACE_prev_Selected_Object_Id();
						SceneObject* prev_selected_object = Mediator::Request_INTERFACE_prev_Selected_Object();
						bool is_parent_selected = selected_id == smplr->getParent()->getId();
						bool was_parent_selected = prev_selected_id == smplr->getParent()->getId();

						////std::cout << "\n" << std::endl;
						////std::cout << "  - sampler : " << smplr->getName() << std::endl;
						////std::cout << "  - parent : " << smplr->getParent()->getId() << std::endl;
						////std::cout << "  - selected_id : " << selected_id << std::endl;
						////std::cout << "  - prev_selected_id : " << prev_selected_id << std::endl;


						if ((is_Sampler && (smplr->getId() == selected_object->getId()))
							|| !is_Sampler
							)
						{
							if (!is_Sampler)
							{
								if ((was_parent_selected) && (is_parent_selected))
								{
									////std::cout << "   - pushing_back[a] : [ "<< smplr->getParent()->getName()<<" ]::" << smplr->getName() << std::endl;
									valid_samplers.push_back(smplr);
								}
							}
							else
							{
								////std::cout << "   - pushing_back[b] : [ " << smplr->getParent()->getName() << " ]::" << smplr->getName() << std::endl;
								valid_samplers.push_back(smplr);
							}
						}
					}

				}
				else if (r_samplers == 2)
				{
					for (SceneObject* obj : Mediator::RequestSceneObjects())
					{
						Type obj_type = obj->getType();
						bool is_valid = obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT && obj->isActive();
						if (!is_valid)
							continue;

						for (SceneObject* sampler : obj->get_Samplers())
						{
							valid_samplers.push_back(sampler);
							////std::cout << "   - pushing_back : [ " << sampler->getParent()->getName() << " ]::" << sampler->getName() << std::endl;
						}
					}
				}
				if      (r_samplers != 0)
				{

					////std::cout << "   - focused_object : " << m_focused_object << std::endl;
					for ( SceneObject* s : valid_samplers )
					{
						////std::cout << "    - candidate_sampler : [ "<< s->getParent()->getName() << " ]::" << s->getName() << std::endl;
						////std::cout << "    - id : " << s->getId() << std::endl;
						if ( s->getId() == m_focused_object )
						{
							////std::cout << "    - is_valid!" << std::endl;
							is_valid_sampler = true;
							break;
						}
					}
				}

				//                        //
				// ------- LIGHTS ------- //
				//                        //
				for (SceneObject* obj : Mediator::RequestSceneObjects())
				{
					
					bool is_valid = obj->getType() == LIGHT_OBJECT;
					is_valid     &= obj->isActive();
					if (!is_valid) continue;

					Light_Type type = obj->get_LightParameters().type;
					if (   (type == QUAD_LIGHT      && r_q)
						|| (type == SPOTLIGHT       && r_spot)
						|| (type == SPHERICAL_LIGHT && r_s)
						|| (type == POINT_LIGHT     && r_p)
						|| (obj->getId() == light_id)
						)
					{
						valid_lights.push_back( obj );
					}
				}
				for (SceneObject* light : valid_lights)
				{
					if (light->getId() == m_focused_object)
					{
						is_valid_light = true;
						break;
					}
				}
				//

				//
				bool b1 = !is_valid_fv;
				bool b2 = !is_valid_sampler;
				bool b3 = !is_valid_light;
				bool b4 = (FVI_state == FIS_NONE 
						   || !is_samplers 
						   //|| !is_lights
						   );
				if ( b1 
					&& b2 
					//&& b3 
					&& b4 
					)
				{
					////std::cout << "   - resetting focused object!" << std::endl;
					resetFocusedObject();
				}
			}


			// if none face vectors is picked, re - cast mouse ray for objects //
			if (m_focused_object == -1)
			{
				resetFocusedObject();
				if (is_lights) Mediator::toggle_LightObjects(1, r_q, r_spot, r_s, r_p);
				m_context->launch(0, 1);
				if (is_lights) Mediator::toggle_LightObjects(0, r_q, r_spot, r_s, r_p);
			}
		}
		else
		{
			if (is_lights) Mediator::toggle_LightObjects(1, r_q, r_spot, r_s, r_p);
			m_context->launch(0, 1);
			if (is_lights) Mediator::toggle_LightObjects(0, r_q, r_spot, r_s, r_p);
		}
	}
	retrieveFocusedObject();


}

void OptiXRenderer::post_process_MouseRay_Data()
{
	VIEWPORT active_vp   = ViewportManager::GetInstance().getActiveViewport();
	PinholeCamera camera = ViewportManager::GetInstance().getViewportCamera(active_vp.id);
	optix::float3 * mouseHitBuffer_Data = getMouseHitBuffer_Data();

	optix::float3 raw_normal = mouseHitBuffer_Data[1];
	float view_dot = dot(raw_normal, camera.getW());
	
	mousehit_data[0] = mouseHitBuffer_Data[0];
	mousehit_data[1] = view_dot > 0 ? -raw_normal : raw_normal;
	mousehit_data[2] = mouseHitBuffer_Data[2];

	delete mouseHitBuffer_Data;
}

void OptiXRenderer::launch_Utility_Rays()
{
	
	optix::float3* buffer_data = getMouseHitBuffer_Data();
	optix::float3 prev_data[3] = { buffer_data[0], buffer_data[1], buffer_data[2] };
	delete buffer_data;
	
	optix::float3 cameraPosition;
	optix::float3 cameraU;
	optix::float3 cameraV;
	optix::float3 cameraW;
	optix::float3 origin;
	optix::float3 dir;

	VIEWPORT vp = ViewportManager::GetInstance().getActiveViewport();

	m_context["sys_m_width"]->setUint(vp.renderSize.x);
	m_context["sys_m_height"]->setUint(vp.renderSize.y);
	m_context["sys_utility_ray"]->setInt(1);

	Restriction* uiR    = Mediator::get_UI_Selected_Restriction();
	SceneObject* object = Mediator::Request_INTERFACE_Selected_Object();
	int uiR_id = uiR != 0 ? uiR->getId() : -2;
	for (Restriction* R : Mediator::Request_Scene_Restrictions())
	{
		R->setHit_subj(false);
		R->setHit_obj(false);

		if (!R->is_Active() || !R->is_Renderable() || !R->isEnabled()) continue;
		
		Restriction_ID rID = R->getId();

		if      (rID == DISTANCE_R) continue;
		if      (rID == FOCUS_R)
		{
			SceneObject* parent = R->get_Owner(0);
			bool is_visible = parent->isVisible();
			if(is_visible) parent->setVisible(false, false);

			SceneObject* fv = R->getSubject();
			dir    = optix::make_float3(fv->getRotationMatrix_chain() * optix::make_float4(GLOBAL_AXIS[2], 0.0f));
			origin = fv->getTranslationAttributes();

			// trace utility ray
			{
			
				optix::float2 mouse_pos = optix::make_float2(vp.dimensions.size.x*0.5f,vp.dimensions.size.y*0.5);

				m_context["sys_utility_ray_origin"]->setFloat(origin);
				m_context["sys_utility_ray_direction"]->setFloat(dir);
				m_context->launch(0, 1);

				const void * mouse_buffer_data = m_context["sys_mouseHit_buffer_Output"]->getBuffer()->map();
				m_context["sys_mouseHit_buffer_Output"]->getBuffer()->unmap();
				optix::float3 * data = (optix::float3 *)mouse_buffer_data;
				if (data[2].x == 1)
				{
					optix::float3 p = data[0];
					R->setHit_subj(true);
					R->set_HitPoint_Subject(p);
				}
				
			}
			
			if(is_visible)
				parent->setVisible(is_visible, false);
		}
		else if (rID == CONVERSATION_R)
		{

			SceneObject* parent0 = R->get_Owner(0);
			SceneObject* parent1 = R->get_Owner(1);

			SceneObject* subj = R->getSubject();
			SceneObject* obj  = R->getObject();
 			
			// trace utility ray A
			{
				bool is_visible = parent0->isVisible();
				if(is_visible) parent0->setVisible(false, false);

				dir    = optix::make_float3(subj->getRotationMatrix_chain() * optix::make_float4(GLOBAL_AXIS[2], 0.0f));
				origin = subj->getTranslationAttributes();

				float W = Mediator::RequestWindowSize(0);
				float H = Mediator::RequestWindowSize(1);
				optix::float2 mouse_pos = optix::make_float2(vp.dimensions.size.x*0.5f, vp.dimensions.size.y*0.5);

				m_context["sys_utility_ray_origin"]->setFloat(origin);
				m_context["sys_utility_ray_direction"]->setFloat(dir);
				m_context->launch(0, 1);

				const void * mouse_buffer_data = m_context["sys_mouseHit_buffer_Output"]->getBuffer()->map();
				m_context["sys_mouseHit_buffer_Output"]->getBuffer()->unmap();
				optix::float3 * data = (optix::float3 *)mouse_buffer_data;
				if (data[2].x == 1)
				{
					optix::float3 p = data[0];
					R->setHit_subj(true);
					R->set_HitPoint_Subject(p);
				}

				if(is_visible)
					parent0->setVisible(is_visible, false);
			}

			// trace utility ray B
			{
				bool is_visible = parent1->isVisible();
				if (is_visible) parent1->setVisible(false, false);

				dir    = optix::make_float3(obj->getRotationMatrix_chain() * optix::make_float4(GLOBAL_AXIS[2], 0.0f));
				origin = obj->getTranslationAttributes();

				float W = Mediator::RequestWindowSize(0);
				float H = Mediator::RequestWindowSize(1);
				optix::float2 mouse_pos = optix::make_float2(vp.dimensions.size.x*0.5f, vp.dimensions.size.y*0.5);

				m_context["sys_utility_ray_origin"]->setFloat(origin);
				m_context["sys_utility_ray_direction"]->setFloat(dir);
				m_context->launch(0, 1);
				
				const void * mouse_buffer_data = m_context["sys_mouseHit_buffer_Output"]->getBuffer()->map();
				m_context["sys_mouseHit_buffer_Output"]->getBuffer()->unmap();
				optix::float3 * data = (optix::float3 *)mouse_buffer_data;
				if (data[2].x == 1)
				{
					optix::float3 p = data[0];
					R->setHit_obj(true);
					R->set_HitPoint_Object(p);
				}

				if (is_visible)
					parent1->setVisible(is_visible, false);
			}

			
		}

		
	}

	m_context["sys_utility_ray"]->setInt(0);

	mousehit_data[0] = prev_data[0];
	mousehit_data[1] = prev_data[1];
	mousehit_data[2] = prev_data[2];
}

//------------------------------------------------------------------------------
//
// GET FUNCTIONS
//
//------------------------------------------------------------------------------

GLuint OptiXRenderer::getRenderedSceneTexture()
{
	return m_hdrTexture;
}

GLuint OptiXRenderer::getRenderedSceneMissTexture()
{
	return m_missTexture;
}

PinholeCamera& OptiXRenderer::getCameraInstance()
{
#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::getCameraInstance()" << std::endl;
#endif
	return m_pinholeCamera;
}

optix::Context& OptiXRenderer::getContext()
{
#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::getContext()" << std::endl;
#endif
	return m_context;
}

GLuint& OptiXRenderer::getOpenGLShaderProgram()
{
#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::getOpenGLShaderProgram()" << std::endl;
#endif
	return m_ShaderProgram;
}

PostProcessParameters& OptiXRenderer::getPostProcessParamateres()
{
#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::getPostProcessParameteres()" << std::endl;
#endif
	return m_post_proc_parameters;
}

GLuint OptiXRenderer::getFrameBuffer() const
{
	return m_OutputBuffer->getGLBOId();
}
GLuint OptiXRenderer::get_Optix_DepthBuffer()
{
	return m_DepthTexture;
}

optix::Program OptiXRenderer::getActiveClosestHitProgram()
{
	return m_ProgramsMap["closest_hit_" + m_active_shader];
}
optix::Program OptiXRenderer::getActiveClosestHitTexturedProgram()
{
	return m_ProgramsMap["closest_hit_textured_" + m_active_shader];
}
optix::Program OptiXRenderer::getActiveAnyHitProgram()
{
	return m_ProgramsMap["any_hit_" + m_active_shader];
}

void OptiXRenderer::setActiveOptixShader(std::string shader, bool set_mat)
{
	load_OptiXShader(shader, set_mat);
}
OptiXShader& OptiXRenderer::getActiveOptiXShader()
{
	return optix_shaders[m_active_shader];
}

//------------------------------------------------------------------------------
//
// HELPER FUNCTIONS
//
//------------------------------------------------------------------------------

void OptiXRenderer::PRE_STATE_APPLY(STATE state)
{
	
}
void OptiXRenderer::POST_STATE_APPLY(STATE state)
{

}

void OptiXRenderer::resetFocusedObject()
{
#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::resetFocusedObject()" << std::endl;
#endif

	m_focused_object = -1;
	int ui_focused_id = Mediator::Request_Ui_Focused_Object();

	if ( ui_focused_id >= 0 )
		m_focused_object = ui_focused_id;


	const void * data = m_SelectedObjectBuffer->map();
	int * id = (int*)data;
	*id = m_focused_object;
	m_SelectedObjectBuffer->unmap();
	
	m_context["sys_focusedObject"]->setInt( m_focused_object );
}

void OptiXRenderer::retrieveFocusedObject()
{
#if defined (FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_RENDERER)
	//std::cout << "  --OptixRenderer::retrieveFocusedObject()" << std::endl;
#endif

#ifdef GUI_MANAGER

	m_focused_object = Mediator::requestGuiFocusedObject();
	if (m_focused_object >= 0)
	{
		m_context["sys_focusedObject"]->setInt(m_focused_object);
		Mediator::setFocusedObject(m_focused_object);
		return;
	}

#endif

	const void * data = m_SelectedObjectBuffer->map();
	int * id = (int*)data;
	m_SelectedObjectBuffer->unmap();

	
	if (m_focused_object < 0)
		m_focused_object = *id;

	
	int id_to_scene_index = Mediator::requestObjectsArrayIndex( m_focused_object );
	Mediator::setFocusedObject( id_to_scene_index );
	m_context["sys_focusedObject"]->setInt( m_focused_object );


	if (*id >= 0)
	{

		const void * mouse_buffer_data = m_context["sys_mouseHit_buffer_Output"]->getBuffer()->map();
		m_context["sys_mouseHit_buffer_Output"]->getBuffer()->unmap();

		optix::float3 * buffer_data = (optix::float3 *)mouse_buffer_data;

		mousehit_data[0] = buffer_data[0]; // pos
		mousehit_data[1] = buffer_data[1]; // normal
		mousehit_data[2] = buffer_data[2]; // hit

		optix::float3 w = normalize(Mediator::RequestCameraInstance().getW());
		float dot = optix::dot(w, mousehit_data[1]);
		if (dot > 0.0f)
		{
			//std::cout << " - flipping normal!" << std::endl;
			//mousehit_data[1] *= -1.0f;
		}
	}
}

void OptiXRenderer::restartAccumulation()
{
	////std::cout << " OptiXRenderer::restartAccumulation():" << std::endl;
	frame_number = 1;
}

void OptiXRenderer::checkInfoLog(const char * msg, GLuint object)
{
	GLint maxLength;
	GLint length;
	GLchar *infoLog;

	if (glIsProgram(object))
	{
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &maxLength);
	}
	else
	{
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &maxLength);
	}
	if (maxLength > 1)
	{
		infoLog = (GLchar *)malloc(maxLength);
		if (infoLog != NULL)
		{
			if (glIsShader(object))
			{
				glGetShaderInfoLog(object, maxLength, &length, infoLog);
			}
			else
			{
				glGetProgramInfoLog(object, maxLength, &length, infoLog);
			}
			//fprintf(fileLog, "-- tried to compile (len=%d): %s\n", (unsigned int)strlen(msg), msg);
			//fprintf(fileLog, "--- info log contents (len=%d) ---\n", (int) maxLength);
			//fprintf(fileLog, "%s", infoLog);
			//fprintf(fileLog, "--- end ---\n");
			//std::cout << infoLog << std::endl;
			// Look at the info log string here...
			free(infoLog);
		}
	}
}

bool OptiXRenderer::getInitState()
{
	return m_optix_init && m_openGL_init && m_renderer_init && m_programs_init;
}

void OptiXRenderer::printInitError()
{
	if (!m_optix_init)
	{
		streamHandler::Error("OptiXRenderer", "Init_Optix", "Failed to Initialize!");
	}

	if (!m_openGL_init)
	{
		streamHandler::Error("OptiXRenderer", "Init_OpenGL", "Failed to Initialize!");

	}

	if (!m_renderer_init)
	{
		streamHandler::Error("OptiXRenderer", "Init_Renderer", "Failed to Initialize!");

	}

	if (!m_programs_init)
	{
		streamHandler::Error("OptiXRenderer", "Init_Programs", "Failed to Initialize!");

	}

}

optix::float3 * OptiXRenderer::getMouseHitBuffer_Data()
{
	optix::float3 * data = new optix::float3[3];
	data[0] = mousehit_data[0];
	data[1] = mousehit_data[1];
	data[2] = mousehit_data[2];
	return data;

	/*
	const void * mouse_buffer_data = m_context["sys_mouseHit_buffer_Output"]->getBuffer()->map();
	m_context["sys_mouseHit_buffer_Output"]->getBuffer()->unmap();
	optix::float3 * buffer_data = (optix::float3 *)mouse_buffer_data;
	optix::float3 * data = new optix::float3[3];
	data[0] = buffer_data[0];
	data[1] = buffer_data[1];
	data[2] = buffer_data[2];
	return data;
	*/

}