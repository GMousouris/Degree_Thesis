#include "Defines.h"

#ifndef TEST_MAIN




#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "OptixSceneEditor.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include "Timer.h"
#include <vector>


#ifdef WIN32
  #include <Windows.h>
#endif


static void error_callback(int error, const char* description)
{
	std::cerr << "Error: " << error << ": " << description << std::endl;
}

static int Init_glfw_glew(GLFWwindow * window, int windowWidth, int windowHeight)
{
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		error_callback(1, "GLFW failed to initialize!");
		return 1;
	}

	window = glfwCreateWindow(
		windowWidth,
		windowHeight,
		"OptiXSceneEditor",
		NULL,
		NULL

	);

	if (!window)
	{
		error_callback(2, "glfwCreateWindow() failed.");
		glfwTerminate();
		return 2;
	}
	glfwMakeContextCurrent(window);

	// Init glew
	if (glewInit() != GL_NO_ERROR)
	{
		error_callback(3, "GLEW failed to initialize.");
		glfwTerminate();
		return 3;
	}

	return 0;
}


static OptiXSceneEditor * g_app = nullptr;
static bool displayGUI = true;

int main(int argc, char * argv[])
{

#ifdef WIN32
	
	HWND wnd;
	AllocConsole();
	wnd = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(wnd, 1);

#endif

	GLFWwindow * window = nullptr;
	
	int windowWidth = 1024 + 200;
	int windowHeight = 768;
	int devices = 3210;  // Decimal digits encode OptiX device ordinals. Default 3210 means to use all four first installed devices, when available.
	bool interop = true;  // Use OpenGL interop Pixel-Bufferobject to display the resulting image. Disable this when running on multi-GPU or TCC driver mode.
	int stackSize = 4096;  // Command line parameter just to be able to find the smallest working size.
	// bool light = false; // Add a geometric are light. Best used with miss 0 and 1.
	// int  miss = 1;     // Select the environment light (0 = black, no light; 1 = constant white environment; 3 = spherical environment texture.

	bool hasGui = true;


	// init GLFW && glew
	glfwSetErrorCallback(error_callback);


	if (!glfwInit())
	{
		error_callback(1, "GLFW failed to initialize.");
		return 1;
	}


	window = glfwCreateWindow(windowWidth, windowHeight, "XSceneEditor", NULL, NULL);
	if (!window)
	{
		error_callback(2, "glfwCreateWindow() failed.");
		glfwTerminate();
		return 2;
	}


	glfwMakeContextCurrent(window);

	if (glewInit() != GL_NO_ERROR)
	{
		error_callback(3, "GLEW failed to initialize.");
		glfwTerminate();
		return 3;
	}


	// Init Application
	//g_app = new OptiXSceneEditor(window, windowWidth, windowHeight, stackSize, interop);
	if(!OptiXSceneEditor::GetInstance().Init(window, windowWidth, windowHeight, stackSize, interop))
	//if (!OptiXSceneEditor::GetInstance().Init())
	{
		error_callback(4, "Application initialization failed.");
		glfwTerminate();
		return 4;
	}

	
	glfwWindowHint(GLFW_SAMPLES, 4);

	
	// Main loop

#define APP_GET_TIME
#ifdef APP_GET_TIME
	Timer timer;
	timer.start();

	int ticks_count          = 0;
	float update_time        = 0.0f;
	float handle_events_time = 0.0f;
	float display_time   = 0.0f;
	float render_time    = 0.0f;
	float gl_render_time = 0.0f;

	float t_start;
	float t_end;
	float dt = 0.0f;

	float t_start_global;
	float t_end_global;
	float dt_global;
	float dt_overall = 0.0f;

	std::vector<float> update_dt;
	std::vector<float> handle_events_dt;
	std::vector<float> display_dt;
	std::vector<float> render_dt;
	std::vector<float> gl_render_dt;

	float update_min = 999999999.f; float update_max = -9999999999.0f;
	float handle_min = 999999999.f; float handle_max = -9999999999.0f;
	float disp_min = 999999999.f; float disp_max = -9999999999.0f;
	float render_min = 999999999.f; float render_max = -9999999999.0f;
	float gl_render_min = 999999999.f;float gl_render_max = -9999999999.0f;

#endif

	while (!glfwWindowShouldClose(window))
	{
		
#ifdef APP_GET_TIME
		//t_start = timer.getTime();
		t_start_global = timer.getTime();
#endif
		OptiXSceneEditor::GetInstance().Update(0.0f);
#ifdef APP_GET_TIME
		//t_end = timer.getTime();
		//dt = t_end - t_start;
		//update_time += dt;
#endif

		if (hasGui)
		{
			


#ifdef APP_GET_TIME
			//t_start = timer.getTime();
#endif

			OptiXSceneEditor::GetInstance().HandleEvents();

#ifdef APP_GET_TIME
			//t_end = timer.getTime();
			//dt = t_end - t_start;
			//handle_events_time += dt;
#endif





#ifdef APP_GET_TIME
			//t_start = timer.getTime();
#endif

			OptiXSceneEditor::GetInstance().Render();

#ifdef APP_GET_TIME
			//t_end = timer.getTime();
			//dt = t_end - t_start;
			//render_time += dt;
#endif

#ifdef APP_GET_TIME
			//t_start = timer.getTime();
#endif

			OptiXSceneEditor::GetInstance().Display();

#ifdef APP_GET_TIME
			//t_end = timer.getTime();
			//dt = t_end - t_start;
			//display_time += dt;
#endif

	
#ifdef APP_GET_TIME
			//t_start = timer.getTime();
#endif

			OptiXSceneEditor::GetInstance().RenderGui();

#ifdef APP_GET_TIME
			//t_end = timer.getTime();
			//dt = t_end - t_start;
			//gl_render_time += dt;
#endif

			glfwSwapBuffers( window );
		}
		else
		{
			OptiXSceneEditor::GetInstance().Render();
			glfwSetWindowShouldClose( window, 1 );
		}



#ifdef APP_GET_TIME

		t_end_global = timer.getTime();
		dt_global    = t_end_global - t_start_global;
		//dt_overall  += dt_global;
		OptiXSceneEditor::GetInstance().set_Global_TimeDelta(dt_global);

		//std::cout << " - dt : " << dt_global << std::endl;

#ifdef APP_TEMP_AUTO_SAVE
		if (dt_overall >= DT_SAVE_TEMP_DATA)
		{
			OptiXSceneEditor::GetInstance().SAVE_TEMP_DATA();
			dt_overall = 0.0f;
		}
#endif

		timer.reset();
		timer.stop();
		timer.start();

#endif
	}

	
	
	// clean up
	//delete g_app;
	glfwTerminate();
	OptiXSceneEditor::GetInstance().clear();

	return 0;

}


#endif