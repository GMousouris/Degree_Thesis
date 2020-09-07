#ifndef BIM_ENGINE_RENDERER_H
#define BIM_ENGINE_RENDERER_H

#include "SDL2/SDL.h"
#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include <vector>
#include "ShaderProgram.h"
#include "SpotlightNode.h"
#include "GeometryNode.h"
#include "AnimationHandler.h"
#include "Defines.h"



class Renderer
{
public:
	enum RENDERING_MODE
	{
		TRIANGLES,
		LINES,
		POINTS
	};

	enum SHADING_MODE
	{
		NEAREST,
		PCF,
		PCSS_1,
		PCSS_2,
		VSM
    };

	

	//shadow test
	bool SHADOW_TEST = false;
	float test_dt = 0.0f;
	glm::mat4 cube_model_matrix1;
	glm::mat4 cube_normal_matrix1;

	glm::mat4 cube_model_matrix2;
	glm::mat4 cube_normal_matrix2;

	glm::mat4 sphere_model_matrix;
	glm::mat4 sphere_normal_matrix;

	glm::mat4 tower_model_matrix;
	glm::mat4 tower_normal_matrix;


	glm::mat4 pirate_model_matrix;
	glm::mat4 pirate_normal_matrix;
	//


protected:

	class Game *                                    game = 0;
	SDL_Window *                                    window = 0;
	int												m_screen_width, m_screen_height;
	glm::mat4										m_view_matrix, m_gui_view_matrix;
	glm::mat4										m_projection_matrix, m_gui_projection_matrix;
	glm::vec3										m_camera_position;
	glm::vec3										m_camera_target_position;
	glm::vec3										m_camera_up_vector;
	glm::vec2										m_camera_movement;
	glm::vec2										m_camera_look_angle_destination;

	bool                                            light_move = false;

	float                                           light_time = 0.0f;
	
	bool                                            testing_mode = false;

	bool                                            shadow_render_to_screen = false;

	bool                                            testObjects_move = false;

	bool                                            camera_moving = false;
	int                                             camera_moving_vars[4];

	glm::vec3                                       ray_origin;
	glm::vec3                                       ray_direction;
	float                                           intersection_distance;

	//common items


	//samplers
	float anisotropy = 1.0f;
	GLuint m_wrapping_samplers[3];
	GLuint m_nearest_sampler, m_linear_sampler, m_trilinear_sampler;

	//quads (base , gui elements)
	GLuint m_quad_gui_vao = 0;
	GLuint m_quad_gui_vertices = 0;
	GLuint m_quad_gui_texcoord = 0;
	GLuint m_quad_vao = 0;
	GLuint m_quad_texture = 0;
	GLuint m_quad_vertices = 0;
	GLuint m_quad_texcoord = 0;
	GLuint m_quad_normals = 0;

	GLuint m_cube_vao = 0;
	GLuint m_cube_vbo = 0;
	GLuint m_cube_normals = 0;
	GLuint m_cube_texcoords = 0;
	GLuint m_cube_texcoords2 = 0;

	//gui elements
	bool over_tower_icon = false; bool focused_tower = false; bool over_meteorite_icon = false; bool over_ufo_icon = false;
	
	
	glm::mat4 quad_wall_transformation_matrix;
	glm::mat4 quad_wall_normal_matrix;
	glm::mat4 bones_icon_model_matrix;
	glm::mat4 coins_icon_model_matrix;
	glm::mat4 bones_numbers_model_matrix[3];
	glm::mat4 tower_icon_model_matrix;
	glm::mat4 meteorite_icon_model_matrix;
	glm::mat4 ufo_icon_model_matrix;

	glm::mat4 coins_numbers_model_matrix[3];
	glm::mat4 delete_icon_model_matrix;
	glm::mat4 cube_model_matrix;
	glm::mat4 cube_normal_matrix;
	glm::mat4 info_model_matrix;

	// Geometry Rendering Intermediate Buffer
	GLuint m_fbo, m_shadowmap_blur_fbo;
	GLuint m_fbo_depth_texture;
	GLuint m_fbo_texture;
	GLuint m_shadowmap_blur_texture = 0;
	GLuint m_vao_fbo, m_vbo_fbo_vertices;

	float m_continous_time;

	// Rendering Mode
	RENDERING_MODE m_rendering_mode;

	// Shading mode
	SHADING_MODE m_shading_mode;

	// Lights
	SpotLightNode m_spotlight_node;

	// Protected Functions
	bool InitRenderingTechniques();
	bool InitDeferredShaderBuffers();
	bool InitCommonItems();
	bool InitLightSources();
	bool InitGeometricMeshes();
	void InitMatrices();

	void DrawGeometryNode(class GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix);
	void DrawGeometryNode(class GameObject * object);
	void DrawGeometryNodes(std::vector<class GameObject *> &objects);
	void DrawGeometryNodeToShadowMap(class GeometryNode* node, glm::mat4 model_matrix, glm::mat4 normal_matrix, ShaderProgram * program);
	void DrawGeometryNodesToShadowMap(std::vector<class GameObject *> &objects , ShaderProgram * program);
	void DrawSimpleGeometryNode(GameObject * object);

	ShaderProgram                               m_plane_rendering_program;
	ShaderProgram								m_geometry_rendering_program;
	ShaderProgram								m_postprocess_program;
	ShaderProgram								m_spot_light_shadow_map_program;
	ShaderProgram                               m_gui_rendering_program;
	ShaderProgram                               m_vsm_shading_program;
	ShaderProgram                               m_blur_program;

public:
	Renderer(Game * game);
	~Renderer();

	bool										Init(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	bool                                        generateSamplers();
	void										Update(float dt);
	bool										ResizeBuffers(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	bool										ReloadShaders();
	void										Render();

	// Passes
	void										RenderShadowMaps();
	void										RenderGeometry();
	void                                        RenderPlanes();
	void                                        RenderGui();
	void                                        ApplyFilters();
	void                                        BlurrShadowMap();
	void										RenderToOutFB();
	void                                        RenderShadowMapToScreen(GLint shadow_map_texture);
	void                                        ShadowRendering_toScreen();
	

	// Set functions
	void										SetRenderingMode(RENDERING_MODE mode);
	void                                        setShadingMode(SHADING_MODE mode);
	void                                        setTestingMode();
	void                                        setShadowTest();
	void                                        testObjectsMove();

	// Camera Function
	void                                        LightSourceMove();
	void										CameraMoveForward(bool enable);
	void										CameraMoveBackWard(bool enable);
	void										CameraMoveLeft(bool enable);
	void										CameraMoveRight(bool enable);
	void										CameraLook(glm::vec2 lookDir);
	void                                        cameraMove(bool enable);
	void                                        cameraMove_calc();

	//void                                        screenPosToWorldRay(int mouseX, int mouseY);
	void                                        screenPosToWorldRay(int mouseX, int mouseY, glm::mat4 & projection_matrix, glm::mat4 & view_matrix,bool state);
	bool                                        TestRayOBBIntersection(GameObject *object);
	bool                                        TestRayOBBIntersection(glm::mat4 & model_matrix , float scale_factor);
	void                                        CheckForSelection(int x, int y , bool state);
	bool                                        CheckForGuiInteraction();

	void                                        updateCamera(float dt);
	void                                        arcball_rotate(glm::vec2 vec);


};

#endif
