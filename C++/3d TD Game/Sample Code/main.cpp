#include "SDL2/SDL.h"
#include <iostream>
#include <chrono>
#include "GLEW\glew.h"
#include "Renderer.h"
#include "Game.h"
#include <thread>
#include "Defines.h"

using namespace std;

//Screen attributes
SDL_Window * window;

//OpenGL context 
SDL_GLContext gContext;
const int SCREEN_WIDTH = 800;	//800;	//640;
const int SCREEN_HEIGHT = 600;	//600;	//480;

//Event handler
SDL_Event event;

Game * game = nullptr;

void func()
{
	system("pause");
}

// initialize SDL and OpenGL
bool init()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		return false;
	}

	// use Double Buffering
	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0)
		cout << "Error: No double buffering" << endl;

	// set OpenGL Version (3.3)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Create Window
	window = SDL_CreateWindow("OpenGL Lab 6", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		printf("Could not create window: %s\n", SDL_GetError());
		return false;
	}

	//Create OpenGL context
	gContext = SDL_GL_CreateContext(window);
	if (gContext == NULL)
	{
		printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// Disable Vsync
	if (SDL_GL_SetSwapInterval(0) == -1)
		printf("Warning: Unable to disable VSync! SDL Error: %s\n", SDL_GetError());

	// Initialize GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error loading GLEW\n");
		return false;
	}
	// some versions of glew may cause an opengl error in initialization
	glGetError();

	game = new Game();
	bool engine_initialized = game->Init(SCREEN_WIDTH, SCREEN_HEIGHT);

	//atexit(func);
	
	return engine_initialized;
}


void clean_up()
{
	delete game;

	SDL_GL_DeleteContext(gContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	//Initialize
	if (init() == false)
	{
		system("pause");
		return EXIT_FAILURE;
	}

	//Quit flag
	bool quit = false;
	bool mouse_button_pressed = false;
	glm::vec2 prev_mouse_position(0);

	auto simulation_start = chrono::steady_clock::now();

	// Wait for user exit
	while (quit == false)
	{
		// While there are events to handle
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				// Key down events
				if (event.key.keysym.sym == SDLK_ESCAPE) quit = true;
				else if (event.key.keysym.sym == SDLK_r) game->getRenderer()->ReloadShaders();
				else if (event.key.keysym.sym == SDLK_t) game->getRenderer()->SetRenderingMode(Renderer::RENDERING_MODE::TRIANGLES);
				else if (event.key.keysym.sym == SDLK_l) game->getRenderer()->SetRenderingMode(Renderer::RENDERING_MODE::LINES);
				else if (event.key.keysym.sym == SDLK_p) game->getRenderer()->SetRenderingMode(Renderer::RENDERING_MODE::POINTS);
				else if (event.key.keysym.sym == SDLK_w)
				{
					game->getRenderer()->CameraMoveForward(true);
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					game->getRenderer()->CameraMoveBackWard(true);
				}
				else if (event.key.keysym.sym == SDLK_a)
				{
					game->getRenderer()->CameraMoveLeft(true);
				}
				else if (event.key.keysym.sym == SDLK_d)
				{
					game->getRenderer()->CameraMoveRight(true);
				}
				else if (event.key.keysym.sym == SDLK_c)
				{
					game->Pause();
				}
				else if (event.key.keysym.sym == SDLK_h)
				{
					game->showInfo();
				}
				else if (event.key.keysym.sym == SDLK_RCTRL)
					game->spawnPirate(1);
				else  if (event.key.keysym.sym == SDLK_LCTRL)
					game->DeletePirates();
				else if (event.key.keysym.sym == SDLK_v)
				{
					if (game->towerCanBuy())
					{
						int x; int y;
						SDL_GetMouseState(&x, &y);
						game->setSelection(!game->Selecting());
					}
					
				}
				else if (event.key.keysym.sym == SDLK_m)
					game->getRenderer()->LightSourceMove();
				else if (event.key.keysym.sym == SDLK_o)
					game->getRenderer()->testObjectsMove();

				else if (event.key.keysym.sym == SDLK_8 || event.key.keysym.sym == SDLK_KP_8)
				{
					game->getRenderer()->ShadowRendering_toScreen();
				}

				else if (event.key.keysym.sym == SDLK_0 || event.key.keysym.sym == SDLK_KP_0)
					game->getRenderer()->setShadingMode(Renderer::NEAREST); // nearest
				else if (event.key.keysym.sym == SDLK_1 || event.key.keysym.sym == SDLK_KP_1)
					game->getRenderer()->setShadingMode(Renderer::PCF); // pcf
				else if (event.key.keysym.sym == SDLK_2 || event.key.keysym.sym == SDLK_KP_2)
					game->getRenderer()->setShadingMode(Renderer::PCSS_1); // pcss v1
				else if (event.key.keysym.sym == SDLK_3 || event.key.keysym.sym == SDLK_KP_3)
					game->getRenderer()->setShadingMode(Renderer::PCSS_2); // pcss v2
				else if (event.key.keysym.sym == SDLK_4 || event.key.keysym.sym == SDLK_KP_4)
					game->getRenderer()->setShadingMode(Renderer::VSM); // pcss v2

				else if (event.key.keysym.sym == SDLK_5 || event.key.keysym.sym == SDLK_KP_5)
					game->getRenderer()->setShadowTest();
				else if (event.key.keysym.sym == SDLK_9 || event.key.keysym.sym == SDLK_KP_9)
					game->getRenderer()->setTestingMode();
				
				
					
#ifdef COLLISION_TEST
				else if (event.key.keysym.sym == SDLK_UP)
				{
					game->getRenderer()->sphere_move_forward(true);
				}
				else if (event.key.keysym.sym == SDLK_DOWN)
				{
					game->getRenderer()->sphere_move_backward(true);
				}
				else if (event.key.keysym.sym == SDLK_LEFT)
				{
					game->getRenderer()->sphere_move_left(true);
				}
				else if (event.key.keysym.sym == SDLK_RIGHT)
				{
					game->getRenderer()->sphere_move_right(true);
				}
#endif

#ifdef PIRATE_TEST

				else if (event.key.keysym.sym == SDLK_LEFT)
				{
					game->getRenderer()->PirateRotateLeft(true);
				}
				else if (event.key.keysym.sym == SDLK_RIGHT)
				{
					game->getRenderer()->PirateRotateRight(true);
				}
#endif

			}
			else if (event.type == SDL_KEYUP)
			{
				if (event.key.keysym.sym == SDLK_w)
				{
					game->getRenderer()->CameraMoveForward(false);
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					game->getRenderer()->CameraMoveBackWard(false);
				}
				else if (event.key.keysym.sym == SDLK_a)
				{
					game->getRenderer()->CameraMoveLeft(false);
				}
				else if (event.key.keysym.sym == SDLK_d)
				{
					game->getRenderer()->CameraMoveRight(false);
				}
				
#ifdef COLLISION_TEST
				else if (event.key.keysym.sym == SDLK_UP)
				{
					game->getRenderer()->sphere_move_forward(false);
				}
				else if (event.key.keysym.sym == SDLK_DOWN)
				{
					game->getRenderer()->sphere_move_backward(false);
				}
				else if (event.key.keysym.sym == SDLK_LEFT)
				{
					game->getRenderer()->sphere_move_left(false);
				}
				else if (event.key.keysym.sym == SDLK_RIGHT)
				{
					game->getRenderer()->sphere_move_right(false);
				}
#endif

#ifdef PIRATE_TEST

				else if (event.key.keysym.sym == SDLK_LEFT)
				{
					game->getRenderer()->PirateRotateLeft(false);
				}
				else if (event.key.keysym.sym == SDLK_RIGHT)
				{
					game->getRenderer()->PirateRotateRight(false);
				}
#endif

			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				int x = event.motion.x;
				int y = event.motion.y;

				if (mouse_button_pressed)
				{
					game->getRenderer()->cameraMove(true);
					game->getRenderer()->CameraLook(glm::vec2(x, y) - prev_mouse_position);
					game->getRenderer()->arcball_rotate(glm::vec2(x, y) - prev_mouse_position);
					prev_mouse_position = glm::vec2(x, y);
				}
			}
			
			else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
			{
				if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONUP)
				{
					
					if (game->getRenderer()->CheckForGuiInteraction())
					{
						game->setSelection(!game->Selecting());
					}
					

				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					if (event.type == SDL_MOUSEBUTTONUP)
					{
						game->getRenderer()->cameraMove(false);
						if ( game->Selecting() )
							game->cancelSelection();
						
						   
					}
						
					int x = event.button.x;
					int y = event.button.y;
					mouse_button_pressed = (event.type == SDL_MOUSEBUTTONDOWN);
					prev_mouse_position = glm::vec2(x, y);
					
				}
			}
			else if (event.type == SDL_MOUSEWHEEL)
			{
				int x = event.wheel.x;
				int y = event.wheel.y;
			}
			else if (event.type == SDL_WINDOWEVENT)
			{
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					game->getRenderer()->ResizeBuffers(event.window.data1, event.window.data2);
				}
			}
		}

		// Compute the ellapsed time
		auto simulation_end = chrono::steady_clock::now();
		float dt = chrono::duration <float>(simulation_end - simulation_start).count(); // in seconds
		simulation_start = chrono::steady_clock::now();


		// Update
		game->Update(dt);


		// Draw
		game->getRenderer()->Render();
		

		// Update screen (swap buffer for double buffering)
		SDL_GL_SwapWindow(window);

		
	}

	//Clean up
	clean_up();

	return 0;
}
