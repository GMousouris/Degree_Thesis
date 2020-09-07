#include "Defines.h"
#include "streamHandler.h"
#include "MyAssert.h"


#include "OptiXSceneEditor.h"

#include "OptiXRenderer.h"
#include "OpenGL_Renderer.h"
#include "SceneManager.h"
#include "HierarchyTree.h"
#include "Mediator.h"


#ifdef GUI_MANAGER
#include "guiManager.h"
#endif
#ifdef INTERFACE_MANAGER
#include "InterfaceManager.h"
#endif

#include "ActionManager.h"

#include "AssetManager.h"

#include <GLFW/glfw3.h>



#include <iostream>
#include <string>
#include <map>

#include "Mediator.h"
#include "Mesh_Pool.h"

#include <fstream>

#include <direct.h>


OptiXSceneEditor::OptiXSceneEditor(GLFWwindow * app_window, const int window_width, const int window_height,
								   const unsigned int stackSize, const bool openGL_interop)

	: m_window(app_window), m_window_width(window_width), m_window_height(window_height),
	m_OptiXstackSize(stackSize), m_openGL_interop(openGL_interop)
{
	
}

OptiXSceneEditor::~OptiXSceneEditor()
{
#ifdef GUI_MANAGER
	guiManager::GetInstance().cleanUp();
#endif
	
	//clear();
	

	//_rmdir( TEMP_DATA_IMPORT_MESH_FOLDER_PATH  );
	_rmdir( TEMP_DATA_MESH_FOLDER_TEX_MAP_PATH );
	_rmdir( TEMP_DATA_MESH_FOLDER_PATH );
	_rmdir( TEMP_DATA_FOLDER_PATH      );

}


void OptiXSceneEditor::cleanUp()
{
	
}

//------------------------------------------------------------------------------
//
// INIT FUNCTIONS
//
//------------------------------------------------------------------------------

bool OptiXSceneEditor::Init()
{


#ifdef DEBUGGING
	//std::cout << "\n";
	streamHandler::Report("OptiXSceneEditor", "Init", "Initializing . . . ");
#endif



	_mkdir(TEMP_FOLDER_PATH);
	_mkdir(TEMP_DATA_SAVE_FOLDER_PATH);
	_mkdir(TEMP_DATA_FOLDER_PATH);
	_mkdir(TEMP_DATA_MESH_FOLDER_PATH);
	_mkdir(TEMP_DATA_MESH_FOLDER_TEX_MAP_PATH);


	m_OptiXRenderer_init  = true;
	m_OpenGLRenderer_init = true;
	m_sceneManager_init   = true;
	m_guiManager_init     = true;
	m_assetManager_init   = true;
	m_ActionManager_init  = true;

	Init_GeneralData();
	Init_Helpers();
	

#ifdef  INTERFACE_MANAGER
	InterfaceManager::GetInstance().Init(m_window);
#endif

	Init_OptiXRenderer();
	Init_AssetManager();
	Init_SceneManager();

	
	if (!getInitState())
	{
		printInitError();
		return false;
	}

	return true;
}

bool OptiXSceneEditor::Init(GLFWwindow * app_window, const int window_width, const int window_height,
								   const unsigned int stackSize, const bool openGL_interop)

{
	m_window = app_window;
	m_window_width = window_width;
	m_window_height = window_height;
	m_OptiXstackSize = stackSize;
	m_openGL_interop = openGL_interop;
	return Init();
}

void OptiXSceneEditor::Init_GeneralData()
{
	m_window_width  = 0;
	m_window_height = 0;
	m_windowSize_changed = true;
	//glfwSetWindowSizeCallback(m_window, window_size_callback);
}

void OptiXSceneEditor::Init_OptiXRenderer()
{
#ifdef DEBUGGING
	//std::cout << "\n";
	streamHandler::Report("OptiXSceneEditor", "Init_OptiXRenderer", "Initializing . . . ");
#endif

	m_OptiXRenderer_init = OptiXRenderer::GetInstace().Init();
}

void OptiXSceneEditor::Init_OpenGLRenderer()
{
#ifdef DEBUGGING
	//std::cout << "\n";
	streamHandler::Report("OptiXSceneEditor", "Init_OpenGLRenderer", "Initializing . . . ");
#endif

#ifdef OPENGL_RENDERER_OLD
	m_OpenGLRenderer_init = OpenGLRenderer::GetInstace().Init();
#endif

}

void OptiXSceneEditor::Init_SceneManager()
{
#ifdef DEBUGGING
	//std::cout << "\n";
	streamHandler::Report("OptiXSceneEditor", "Init_SceneManager", "Initializing . . . ");
	
#endif
	m_sceneManager_init = SceneManager::GetInstance().Init();
}

void OptiXSceneEditor::Init_guiManager()
{
#ifdef DEBUGGING
	//std::cout << "\n";
	streamHandler::Report("OptiXSceneEditor", "Init_guiManager", "Initializing . . . ");
	
#endif

#ifdef GUI_MANAGER
	m_guiManager_init = guiManager::GetInstance().Init(m_window);
#endif
}

void OptiXSceneEditor::Init_AssetManager()
{
#ifdef DEBUGGING
	//std::cout << "\n";
	streamHandler::Report("OptiXSceneEditor", "Init_AssetManager", "Initializing . . . ");
	
#endif
	m_assetManager_init = AssetManager::GetInstance().Init(OptiXRenderer::GetInstace().getContext());
}

void OptiXSceneEditor::Init_ActionManager()
{
#ifdef DEBUGGING
	//std::cout << "\n";
	streamHandler::Report("OptiXSceneEditor", "Init_ActionManager", "Initializing . . . ");
#endif

	//ActionManager::GetInstance().Init();
	m_ActionManager_init = true;
}

void OptiXSceneEditor::Init_Helpers()
{
#ifdef DEBUGGING
	//std::cout << "\n";
	streamHandler::Report("OptiXSceneEditor", "Init_Helpers", "Initializing . . . ");
	
#endif

	//Mediator::Init(this);
}

//------------------------------------------------------------------------------
//
// UPDATE - RENDER FUNCTIONS
//
//------------------------------------------------------------------------------

void OptiXSceneEditor::Update(float dt)
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_SCENE_EDITOR) || defined(DEBUGGING) 
	//std::cout << "\n\n  --OptixSceneEditor::Update()" << std::endl;
#endif

	// update window events && frame buffer size
	glfwPollEvents();
	int width, height;
	glfwGetWindowSize(m_window, &width, &height);
	//glfwGetFramebufferSize(m_window, &width, &height);
	if (m_window_width != width || m_window_height != height)
	{
		m_windowSize_changed = true;

		m_window_width  = width;
		m_window_height = height;
    }
	
	

	// update Scene , gui && interface managers
#ifdef OPENGL_RENDERER_OLD
	OpenGLRenderer::GetInstace().Update(dt);
#endif



	
	SceneManager::GetInstance().Update(dt);

#ifdef INTERFACE_MANAGER
	InterfaceManager::GetInstance().Update(dt);
#endif

	OptiXRenderer::GetInstace().Update(dt);

	m_windowSize_changed = false;

#if defined(CANCER_DEBUG) && defined(INTEGRITY_CHECKS)
	
	bool tree_ok = true;
	Hierarchy_Tree* tree = Mediator::request_HierarchyTreeHandle();
	if (tree->isDirty())
	{
		
		tree_ok = tree->checkIntegrity();
		/*
		if (!is_ok)
		{
			bool sceneManager_ok     = SceneManager::GetInstance().checkIntegrity();
			bool openglRenderer_ok   = OpenGLRenderer::GetInstace().checkIntegrity();
			bool interfaceManager_ok = InterfaceManager::GetInstance().checkIntegrity();

			//std::cout << "\n OptiXSceneEditor::checkIntegrity():" << std::endl;
			//std::cout << "        -  HierarchyTree_ok    : " << is_ok << std::endl;
			//std::cout << "        -  SceneManager_ok     : " << sceneManager_ok << std::endl;
			//std::cout << "        -  OpenGLRenderer_ok   : " << openglRenderer_ok << std::endl;
			//std::cout << "        -  InterfaceManager_ok : " << interfaceManager_ok << std::endl;
			system("pause");
		}
		*/
	}

	bool sceneManager_ok = SceneManager::GetInstance().checkIntegrity();
	bool openglRenderer_ok = OpenGLRenderer::GetInstace().checkIntegrity();
	bool interfaceManager_ok = true;//InterfaceManager::GetInstance().checkIntegrity();

	if (
		   !sceneManager_ok 
		|| !openglRenderer_ok 
		//|| !interfaceManager_ok 
		|| !tree_ok)
	{
		//std::cout << "\n OptiXSceneEditor::checkIntegrity():" << std::endl;
		//std::cout << "        -  HierarchyTree_ok    : " << tree_ok << std::endl;
		//std::cout << "        -  SceneManager_ok     : " << sceneManager_ok << std::endl;
		//std::cout << "        -  OpenGLRenderer_ok   : " << openglRenderer_ok << std::endl;
		//std::cout << "        -  InterfaceManager_ok : " << interfaceManager_ok << std::endl;
		system("pause");
	}

#endif

}

void OptiXSceneEditor::Render()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_SCENE_EDITOR) 
	//std::cout << "  --OptixSceneEditor::Render()" << std::endl;
#endif
	OptiXRenderer::GetInstace().Render();

}

void OptiXSceneEditor::Display()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_SCENE_EDITOR) 
	//std::cout << "  --OptixSceneEditor::Display()" << std::endl;
#endif

	OptiXRenderer::GetInstace().Display();

#ifdef OPENGL_RENDERER_OLD
	OpenGLRenderer::GetInstace().Render();
#endif

}

void OptiXSceneEditor::RenderGui()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_SCENE_EDITOR) 
	//std::cout << "  --OptixSceneEditor::RenderGui()" << std::endl;
#endif

#ifdef GUI_MANAGER
	guiManager::GetInstance().NewFrame();
	guiManager::GetInstance().createGUI();
	guiManager::GetInstance().Render();
#endif


#ifdef INTERFACE_MANAGER
	InterfaceManager::GetInstance().Render_Interface();
#endif

}

//------------------------------------------------------------------------------
//
// HANDLE_EVENTS FUNCTIONS
//
//------------------------------------------------------------------------------

void OptiXSceneEditor::HandleEvents()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_SCENE_EDITOR) 
	//std::cout << "  --OptixSceneEditor::HandleEvents()" << std::endl;
#endif

#ifdef GUI_MANAGER
	guiManager::GetInstance().HandleEvents();
#endif // GUI_MANAGER


	InterfaceManager::GetInstance().HandleEvents();
}

void OptiXSceneEditor::window_size_callback(GLFWwindow* window, int width, int height)
{
	//setWindowSizeChanged(true);
    //m_windowSize_changed = true;
}

//------------------------------------------------------------------------------
//
// GET FUNCTIONS
//
//------------------------------------------------------------------------------

GLFWwindow * OptiXSceneEditor::getAppWindow()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_SCENE_EDITOR) 
	//std::cout << "  --OptixSceneEditor::getAppWindow()" << std::endl;
#endif

	return m_window;
}

int * OptiXSceneEditor::getWindowSize()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_SCENE_EDITOR) 
	//std::cout << "  --OptixSceneEditor::getWindowSize()" << std::endl;
#endif
	int * size = new int[2];
	size[0] = m_window_width;
	size[1] = m_window_height;

	return size;
}

unsigned int OptiXSceneEditor::getOptiXstackSize()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_SCENE_EDITOR) 
	//std::cout << "  --OptixSceneEditor::getOptiXstackSize()" << std::endl;
#endif
	return m_OptiXstackSize;
}

bool OptiXSceneEditor::getOpenGL_interop()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_SCENE_EDITOR) 
	//std::cout << "  --OptixSceneEditor::getOpenGL_interop()" << std::endl;
#endif
	return m_openGL_interop;
}

bool OptiXSceneEditor::isWindowSizeChanged()
{
	return m_windowSize_changed;
}

//------------------------------------------------------------------------------
//
// GET RENDERER / SCENE INSTANCE
//
//------------------------------------------------------------------------------

class OptiXRenderer&  OptiXSceneEditor::GetRendererInstance()
{
	return OptiXRenderer::GetInstace();
}

class SceneManager& OptiXSceneEditor::GetSceneManagerInstance()
{
	return SceneManager::GetInstance();
}

//------------------------------------------------------------------------------
//
// HELPER FUNCTIONS
//
//------------------------------------------------------------------------------

void OptiXSceneEditor::clear( bool state )
{
	//std::cout << "\n - OptiXSceneEditor::clear( state = " << state << " ) " << std::endl;
	ActionManager::GetInstance().clear();
	Mediator::force_Execute_Scene_Remove_Calls();

	SceneManager::GetInstance().clear( state );
	
	_rmdir(TEMP_DATA_FOLDER_PATH);

	//std::cout << " - OptiXSceneEditor::clear()_END_:" << std::endl;

}

bool OptiXSceneEditor::getInitState()
{
	return m_OptiXRenderer_init && m_sceneManager_init && m_guiManager_init && m_assetManager_init;
}

void OptiXSceneEditor::printInitError()
{

	if (!m_OptiXRenderer_init)
	{
		streamHandler::Error("OptiXSceneEditor", "Init_OptiXRenderer", "Failed to Initialize!");
	}

	if (!m_OpenGLRenderer_init)
	{
		streamHandler::Error("OptiXSceneEditor", "Init_OpenGLRenderer", "Failed to Initialize!");
	}

	if (!m_sceneManager_init)
	{
		streamHandler::Error("OptiXSceneEditor", "Init_SceneManager", "Failed to Initialize!");
	}

	if (!m_guiManager_init)
	{
		streamHandler::Error("OptiXSceneEditor", "Init_guiManager", "Failed to Initialize!");
	}

	if (!m_assetManager_init)
	{
		streamHandler::Error("OptiXSceneEditor", "Init_AssetManager", "Failed to Initialize!");
	}

}

void OptiXSceneEditor::set_Global_TimeDelta(float dt)
{
	m_global_dt = dt;
}
float OptiXSceneEditor::get_Global_TimeDelta()
{
	return m_global_dt;
}
void OptiXSceneEditor::SAVE_TEMP_DATA(std::string name)
{
	if (name == "") name = "curr_scene_temp_save";
	std::string path = TEMP_DATA_SAVE_FOLDER_PATH + name + ".r_scene";
	Mediator::Save_Scene(path.c_str(), path.c_str());
}