#pragma once



#if defined(_WIN32)
#include <windows.h>
#endif

#include <string>




class OptiXSceneEditor
{
	

public:

	OptiXSceneEditor() {}
	OptiXSceneEditor(struct GLFWwindow * app_window, const int window_width, const int window_height, 
					 const unsigned int OptiXstackSize , const bool OpenGL_interop);
	~OptiXSceneEditor();

	static OptiXSceneEditor& GetInstance()
	{
		static OptiXSceneEditor MAIN;
		return MAIN;
	}

	bool                                                                                      Init();
	bool                                                                                      Init(struct GLFWwindow * app_window, const int window_width, const int window_height,
																								   const unsigned int OptiXstackSize, const bool OpenGL_interop);
	void                                                                                      cleanUp();

private:

	void                                                                                      Init_GeneralData();
	void                                                                                      Init_OptiXRenderer();
	void                                                                                      Init_OpenGLRenderer();
	void                                                                                      Init_SceneManager();
	void                                                                                      Init_guiManager();
	void                                                                                      Init_AssetManager();
	void                                                                                      Init_ActionManager();
	void                                                                                      Init_Helpers();

	bool                                                                                      getInitState();
	void                                                                                      printInitError();

	static void                                                                               window_size_callback(GLFWwindow* window, int width, int height);

public:

	void                                                                                      Update(float dt);
	void                                                                                      Render();
	void                                                                                      Display();
	void                                                                                      RenderGui();
	void                                                                                      HandleEvents();

	struct GLFWwindow*                                                                        getAppWindow();
	int *                                                                                     getWindowSize();
	bool                                                                                      isWindowSizeChanged();
	unsigned int                                                                              getOptiXstackSize();
	bool                                                                                      getOpenGL_interop();
	//void                                                                                      setWindowSizeChanged(bool state);

	static class OptiXRenderer&                                                               GetRendererInstance();
	static class SceneManager&                                                                GetSceneManagerInstance();

	void set_Global_TimeDelta(float dt);
	float get_Global_TimeDelta();

	void                                                                                      clear( bool state = true );
	void                                                                                      SAVE_TEMP_DATA(std::string name = "");

private:



	GLFWwindow *                m_window;
	
	int                         m_window_width;
	int                         m_window_height;
	unsigned int                m_OptiXstackSize;
	bool                        m_openGL_interop;

	bool                        m_OptiXRenderer_init;
	bool                        m_OpenGLRenderer_init;
	bool                        m_sceneManager_init;
	bool                        m_guiManager_init;
	bool                        m_assetManager_init;
	bool                        m_ActionManager_init;
	bool                        m_windowSize_changed;

	float                       m_global_dt = 0.0f;

};


