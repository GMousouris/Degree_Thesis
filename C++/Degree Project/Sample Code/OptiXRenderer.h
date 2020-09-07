#pragma once
#define OptiX_Renderer


#include "MyAssert.h"

#include <GL/glew.h>

#include "common_structs.h"
#include "PinholeCamera.h"
#include "Timer.h"
#include "OptiXRenderer_structs.h"
#include <string>

#include <map>



class OptiXRenderer
{



	struct STATE_STRUCT
	{

	};

	enum STATE
	{
		TRACE_MOUSE_RAY,
		TRACE_SCENE
	};


public:

	//OptiXRenderer(GLFWwindow * window, const int widht, const int height, const unsigned int stackSize, const bool interop);
	static OptiXRenderer& GetInstace()
	{
		static OptiXRenderer renderer;
		return renderer;
	}
	~OptiXRenderer();

	bool                                                                      Init();
	void                                                                      cleanUp();
	void                                                                      Update(float dt);

	void                                                                      ResizeBuffers();
	void                                                                      ResizeBuffers(int width, int height);

	bool                                                                      Render();
	void                                                                      Display();


private:

	void                                                                      Init_GeneralData();
	void                                                                      Init_OptiX();
	void                                                                      Init_OptiXShaders();
	void                                                                      Init_OpenGL();
	void                                                                      Init_Helpers();
	void                                                                      Init_GLSL();
	void                                                                      Init_Renderer();
	void                                                                      Init_Programs();
	void                                                                      Init_LightSamplePrograms();

	void                                                                      PRE_STATE_APPLY(STATE state);
	void                                                                      POST_STATE_APPLY(STATE state);

public:
	void                                                                      load_OptiXShader( std::string shader_id, bool set_materials);
	void                                                                      load_Programs(std::string shader_id);
	OptiXShader&                                                              getActiveOptiXShader();
	void                                                                      setActiveOptixShader(std::string shader_id, bool set_materials = false);

public:

	GLuint get_Optix_DepthBuffer();
	optix::float3 *                                                           getMouseHitBuffer_Data();
	GLuint                                                                    getRenderedSceneMissTexture();
	GLuint                                                                    getRenderedSceneTexture();
	PinholeCamera&                                                            getCameraInstance();
	optix::Context&                                                           getContext();
	GLuint&                                                                   getOpenGLShaderProgram();
	PostProcessParameters&                                                    getPostProcessParamateres();
	GLuint                                                                    getFrameBuffer() const;
	void                                                                      printInitError();


	void                                                                      resetFocusedObject();
	void                                                                      launchMouseRay();
	void                                                                      post_process_MouseRay_Data();
	void                                                                      launch_Utility_Rays();
	//void                                                                      setFocusedObject(int index);
	void                                                                      retrieveFocusedObject();

private:

	bool                                                                      getInitState();
	void                                                                      checkInfoLog(const char *msg, GLuint object);

public:
	void                                                                      restartAccumulation();
	optix::Program                                                            getActiveClosestHitProgram();
	optix::Program                                                            getActiveClosestHitTexturedProgram();
	optix::Program                                                            getActiveAnyHitProgram();
private:

	// init flags
	//bool                                                                      m_app_init;
	bool                                                                      m_optix_init;
	bool                                                                      m_openGL_init;
	bool                                                                      m_renderer_init;
	bool                                                                      m_programs_init;

	// window properties
	int                                                                       m_width;
	int                                                                       m_height;
	int                                                                       g_width;
	int                                                                       g_height;

	unsigned int                                                              m_stackSize;
	bool                                                                      m_interop;

	float                                                                     m_scene_epsilon_factor;

	// OpenGL variables
	GLuint                                                                    m_pboOutputBuffer;
	GLuint                                                                    m_pboDepthBuffer;
	GLuint                                                                    m_pboMissOutputBuffer;
	GLuint                                                                    m_hdrTexture;
	GLuint                                                                    m_DepthTexture;
	GLuint                                                                    m_missTexture;

	// GLSL shaders objects and Program
	GLuint                                                                    m_fragmentShader;
	GLuint                                                                    m_vertexShader;
	GLuint                                                                    m_ShaderProgram;

	//
	PostProcessParameters                                                     m_post_proc_parameters;


	// Optix Variables
	optix::Context                                                            m_context;
	optix::Buffer                                                             m_OutputBuffer;
	optix::Buffer                                                             m_MissOutputBuffer;
	optix::Buffer                                                             m_Optix_DepthBuffer;
	optix::Buffer                                                             m_SelectedObjectBuffer;
	std::map<std::string, optix::Program>                                     m_ProgramsMap;

	float                                                                     m_mouseSpeedRatio;
	Timer                                                                     m_timer;
	PinholeCamera                                                             m_pinholeCamera;

	int                                                                       m_focused_object;

	int                                                                       frame_number;
	int                                                                       m_IterationIndex; // init


	// OptiXShaders Data
	std::unordered_map<std::string, OptiXShader> optix_shaders;
	std::string m_active_shader;
	optix::Buffer                m_bufferSampleLight_programs;
	optix::Buffer                m_mouse_hit_buffer;
	optix::Buffer                m_cast_dir_position;
	


	STATE_STRUCT m_State_data;
	// temp data //
	// std::vector<class Action_TRANSFORMATION> m_light_scale_actions;


	optix::float3 mousehit_data[3];
	//


protected:
	OptiXRenderer();

};




