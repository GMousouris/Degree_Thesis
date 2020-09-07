#pragma once


#include "GL/glew.h"
#include "GL/glut.h"

#include <optixu/optixpp_namespace.h>

#include "glm/glm.hpp"
#include "ShaderProgram.h"
#include "common_structs.h"
#include "Utility_Renderer_Defines.h"
#include "Interface_Structs.h"
#include "PinholeCamera.h"

#include <vector>
#include <unordered_map>
#include <iostream>
#include "Utilities.h"
#include "ViewportManager.h"

#include "Timer.h"

//#include "ViewportManager_structs.h"

#define FONT1 GLUT_BITMAP_8_BY_13
#define FONT2 GLUT_BITMAP_9_BY_15 
#define FONT3 GLUT_BITMAP_TIMES_ROMAN_10 
#define FONT4 GLUT_BITMAP_TIMES_ROMAN_24
#define FONT5 GLUT_BITMAP_HELVETICA_10 
#define FONT6 GLUT_BITMAP_HELVETICA_12 
#define FONT7 GLUT_BITMAP_HELVETICA_18 




//#define DEFAULT_ARC_DRAWING_METHOD
#define ARBITRARY_PLANE_ARC_DRAWING_METHOD


#define UTILITY_RENDERER_CONSTRUCTORS
#define UTILITY_RENDERER_INIT_FUNCTIONS
#define UTILITY_RENDERER_UPDATE_FUNCTIONS
#define UTILITY_RENDERER_SHADER_FUNCTIONS
#define UTILITY_RENDERER_UTIL_FUNCTIONS
#define UTILITY_RENDERER_RENDER_FUNCTIONS
#define UTILITY_RENDERER_UTILITY_RENDER_FUNCTIONS
#define UTILITY_RENDERER_SET_FUNCTIONS
#define UTILITY_RENDERER_GET_FUNCTIONS

#define USE_OptiXViewport_Scene_Depth



enum UTIL_RENDER_TYPE
{
	RENDER_UTILITY,
	RENDER_LIGHT_OBJECT,
	RENDER_LIGHT_OBJECT_PROPERTIES
};

struct Shape_RenderList_Object
{
	SHAPE_ID shape_id;
	Shape shape;
	Utility ut_id;
	float scale_factor;

	Shape_RenderList_Object() {}
	~Shape_RenderList_Object() {}
};
struct SceneObject_RenderList_Object
{
	struct Object_Package * obj_pack;
	PinholeCamera * camera_instance;

	SceneObject_RenderList_Object()
		: obj_pack(0), camera_instance(0)
	{

	}


};
struct Utility_RenderList_Object
{
	UTIL_RENDER_TYPE render_type;

	glm::vec2 mouse_start;
	glm::vec2 mouse_current;

	struct UtilityProperties  ut;
	struct Object_Package * obj_pack;
	struct Object_Package * obj2_pack;
	PinholeCamera * camera_instance;
	bool set_mouse;

	VIEWPORT vp;

	Utility_RenderList_Object()
		:
		render_type(RENDER_UTILITY),
		//ut(nullptr), 
		obj_pack(nullptr),
		obj2_pack(nullptr),
		camera_instance(nullptr), 
		set_mouse(false)
	{}

	~Utility_RenderList_Object()
	{}

};
struct Highlight_RenderList_Object
{
	
	struct Object_Package obj_pack;
	PinholeCamera * camera_instance;

	Highlight_RenderList_Object()
		: obj_pack(), camera_instance(nullptr)
	{}

	~Highlight_RenderList_Object()
	{}
};
struct Scene_RenderList_Object
{
	int scene_width;
	int scene_height;
	int render_type;
	int index;

	PinholeCamera *camera;
	class SceneObject*  focused_object;

	Scene_RenderList_Object()
		:
		scene_width(0),scene_height(0), render_type(0), focused_object(0), index(-1)
	{

	}

	~Scene_RenderList_Object() 
	{
		//delete camera;
	}
};

class OpenGLRenderer
{
	struct STATE_STRUCT
	{
		VIEWPORT vp;
	};

	enum STATE
	{
		RENDER_VIEWPORT,
		DISPLAY_VIEWPORT
	};

	enum RENDERING_MODE
	{
		TRIANGLES,
		LINES,
		POINTS
	};


public:

	static OpenGLRenderer& GetInstace()
	{
		static OpenGLRenderer renderer;
		return renderer;
	}

	OpenGLRenderer();
	~OpenGLRenderer();

private:

	//SceneGrid_Paramaters SceneGrid_obj;

	bool m_scene_first_pass            = false;
	
	RENDERING_MODE m_rendering_mode;

	int m_screen_width;
	int m_screen_height;
	glm::mat4 m_projection_matrix;
	glm::mat4 m_view_matrix;

	glm::vec3 DEF_AXIS_COLOR[3];
	glm::vec3 AXIS_COLOR[3];
	class GeometryNode * m_base_axis_node[3];
	class GeometryNode * m_sphere_node;
	class GeometryNode * m_cube_node;
	class GeometryNode * m_disk_node;
	class GeometryNode * m_disk_nswe_node;
	class GeometryNode * m_cube_text_node;
	glm::mat4 m_sphere_transformation_matrix, m_sphere_normal_matrix;
	glm::mat4 m_object_transformation_matrix, m_object_transformation_matrix_scaled, m_object_transformation_matrix_rotated_scaled;
	glm::mat4 m_object_rotation_matrix;

	int m_render_utility;
	int m_render_cam_center_indicator;
	int m_active_utility;
	int m_active_arc;
	int m_render_cursor;
	CURSOR_TYPE m_cursor_to_render;

	ShaderProgram m_utility_rendering_program;
	ShaderProgram m_postprocess_program;
	ShaderProgram m_postproc_fxaa_program;
	ShaderProgram m_cursor_rendering_program;
	ShaderProgram m_scene_rendering_program;
	ShaderProgram m_background_rendering_program;

	std::unordered_map< int, std::vector<Shape> >   m_Shapes2D_per_Viewport;
	std::vector<Shape_RenderList_Object>            m_shapes2D_RenderList;
	std::vector<Highlight_RenderList_Object>        m_Highlight_RenderList;
	std::vector<Utility_RenderList_Object>          m_Utility_RenderList;
	std::vector<SceneObject_RenderList_Object>      m_SceneObject_RenderList;
	std::vector<Scene_RenderList_Object>            m_scene_RenderList;
	std::unordered_map<int, std::vector<Shape>> m_Shapes_perViewport_depth;
	
	std::unordered_map<int, GeometryObject_OpenGL*> m_geometry_stored_objects;
	struct Interface_State * interface_state;

	bool m_focused_object_changed;
	int  m_current_focused_object_index;
	int  m_prev_focused_object_index;

	bool m_selected_object_changed;
	int  m_current_selected_object_index;
	int  m_prev_selected_object_index;

	GLuint m_fbo, m_fbo_utility;
	GLuint m_fbo_texture, m_fbo_texture_utility;
	GLuint m_fbo_depth_texture;
	PostProcessParameters m_post_proc_parameters;

	
	float timer;
	optix::float2 m_renderSize;


	double global_avg_Render_time       = 0.0f;
	double global_avg_State_Change_Time = 0.0f;
	int    global_state_change_counter  = 0;
	int    global_render_count          = 0;
	double global_time_counter          = 0.0f;

	double global_avg_render_scene_time = 0.0f;
	int    global_render_scene_counter  = 0;

	Timer g_timer;


	STATE_STRUCT m_State_Data;


	GLuint cursor_fbo, cursor_fbo_texture;
	GLuint view_axis_fbo, view_axis_texture, view_axis_depth_texture;
	optix::float2 m_cursor_size = optix::make_float2(400.0f);
	optix::float2 m_view_axis_size = optix::make_float2(400.0f);

	// temp data //
	//std::vector<class Action_TRANSFORMATION> m_light_scale_actions;

#ifdef UTILITY_RENDERER_INIT_FUNCTIONS

public:
	bool Init();

private:

	bool Init_Viewports();
	bool Init_Shaders();
	bool Init_Shader_Uniforms();
	bool Init_postproc_effects();

	bool InitDeferredShaderBuffers();
	bool Init_Utility_FrameBuffers();
	bool InitGeometricMeshes();


#endif

#ifdef UTILITY_RENDERER_UPDATE_FUNCTIONS

public:

	void Update(float dt);

private:

	void Update_Viewports();

	void Update_Shader_Utility_To_Render(Utility_RenderList_Object* utility_to_render);
	void Update_Shader_Utility_To_Render(Utility_RenderList_Object* utility_to_render, const VIEWPORT& vp);

	void Update_OBJECT_Matrices(Utility_RenderList_Object& OBJECT, PinholeCamera& camera, const VIEWPORT& vp);
	void Update_Shader_Uniforms(Utility_RenderList_Object& OBJECT);
	void Update_Shader_Uniforms(Highlight_RenderList_Object& OBJECT);

	void Update_Shader_Highlight_To_Render(Highlight_RenderList_Object* highlight_to_render);
	void Update_Shader_Highlight_To_Render(Highlight_RenderList_Object* highlight_to_render, const VIEWPORT& vp);
	
	void Update_ColorVars();
	void Update_Camera(PinholeCamera& camera_instance, bool force = false);

#endif

#ifdef UTILITY_RENDERER_RENDER_FUNCTIONS

public:

	void Render();
	void RenderSceneTofbo();
	void postProcessScene( GLuint m_fbo, GLuint m_fbo_texture );
	void apply_FXAA(GLuint m_fbo, GLuint tex);
	void RenderToOutFB(GLuint m_fbo_texture, bool use_blend = false );
	void RenderToOutFB(GLuint m_fbo_to , 
					   GLuint m_fbo_texture_from , 
					   bool post_proc = false , 
					   bool use_blend = false , 
					   bool discard_alpha = false , 
					   float scale = 1.0f, 
					   bool visualize_depth = 0, 
					   bool write_depth = 0, 
					   GLuint depth_texture = 0,
					   bool invert_pixels = 0 );
	
	void Render_Background(const VIEWPORT& vp, GLuint m_fbo_texture);
	void Render_Viewports();
	void Render_Viewport( const VIEWPORT& vp, bool is_dirty, bool is_active , bool is_skipFrame, bool scene_is_dirty , bool force_render = 0 );
	void Render_Viewport_Scene(const VIEWPORT& vp);
	void Render_Scene_Light_Objects(const VIEWPORT& vp); //, bool init = false, bool bind = false, bool bind_fbo = false );

	void Display();
	void Display_Viewports();
	
	void Clear_Fbo(GLuint fbo, bool clear_color = false , glm::vec4 color = glm::vec4(0.0f));

	void Render_Viewports_Overlay();

private:

	void Render_Shapes2D(const VIEWPORT& vp);
	void Render_Viewport_Shapes( const VIEWPORT& vp , bool first_phase = false , bool only_trans_spheres = false , bool draw_trans_spheres = false );
	
	
	void Render_Utilities(const VIEWPORT& vp);
	void Render_SceneObject_RenderList(const VIEWPORT& vp);
	void Render_Utility_RenderList(const VIEWPORT& vp);
	void Render_Highlight_RenderList(const VIEWPORT& vp);
	void Render_Camera_Orbit_Center(const VIEWPORT& vp);
	

	void Render_Object_Outline(int scene_object_index, bool selected, const VIEWPORT& vp);
	void Render_Sphere_Outline(glm::mat4 model_matrix , glm::vec3 color, bool selected, const VIEWPORT& vp);
	void Render_Group_Outline(int scene_object_index, bool selected,  const VIEWPORT& vp , bool use_bbox = false);
	void Render_Object_Outline_as_Group(int scene_object_index, bool selected, const VIEWPORT& vp,bool use_bbox = false);
	void Render_Object_Outline_as_Group(std::vector<class SceneObject*> objects, bool selected, const VIEWPORT& vp,bool use_bbox = false);
	void Render_Object_Outline_as_Group(SceneObject* parent, std::vector<class SceneObject*> objects, bool selected, const VIEWPORT& vp, bool use_bbox = false);
	void Render_Object_BBOX(int scene_object_index, bool selected);
	void Render_Object_BBOX(SceneObject* object);
	void Render_Object_FaceVectors(SceneObject* object);
	void Render_Object_FaceVectors(Utility_RenderList_Object utility_to_Render , bool is_creating_face_vector = false);
	void Render_Object_FaceVectors(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp);

	void Render_FaceVector_Outline(Highlight_RenderList_Object ut, int scene_object_index, bool selected, VIEWPORT vp);
	void Render_Sampler_Outline(Highlight_RenderList_Object ut, int scene_object_index, bool selected, VIEWPORT vp);

	void Render_Scene(const VIEWPORT& vp, glm::mat4 view_matrix = glm::mat4(1.0f), glm::mat4 proj_matrix = glm::mat4(1.0f));

	void Render_Cursor_toScreen();
	void Render_Cursor_toScreen_();
	void Render_View_Axis(const VIEWPORT& vp);
	void Render_Viewport_Camera_Orbit_Handle(const VIEWPORT& vp);

	void Render_GeometryNode_Arrays(class SceneObject* obj, class GeometryNode* node, glm::mat4& model_matrix, ShaderProgram& program, glm::vec3 difuse, GLenum gl_type = GL_TRIANGLES);
	void Render_GeometryNode_Arrays( class SceneObject* obj, class GeometryNode* node, glm::mat4& model_matrix, ShaderProgram& program, GLenum gl_type = GL_TRIANGLES );
	void Render_GeometryNode ( class SceneObject* obj, class GeometryNode* node , glm::mat4& model_matrix, ShaderProgram& program , bool only_geometry = false );
	void Render_GeometryObject(class SceneObject* obj, glm::mat4& model_matrix, ShaderProgram& program, bool only_geometry = false);

#endif

#ifdef UTILITY_RENDERER_UTILITY_RENDER_FUNCTIONS

private:

	// g_TRANSFORMATIONS
	void Render_Utility_Translation(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp);
	void Render_Utility_Rotation(Utility_RenderList_Object utility_to_Render   , const VIEWPORT& vp);
	void Render_Utility_Scale(Utility_RenderList_Object utility_to_Render      , const VIEWPORT& vp);

	// g_AREA_SELECTION
	void Render_Utility_Area_Selection(Utility_RenderList_Object utility_to_Render);

	// g_LINKING
	void Render_Utility_Link(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp);
	void Render_Utility_Attach_Restriction_Focus(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Attach_Restriction_Conversation(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Attach_Restriction_Distance(Utility_RenderList_Object ut, const VIEWPORT& vp);

	// g_ALIGN
	void Render_Utility_Object_Align(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Camera_Align(Utility_RenderList_Object ut, const VIEWPORT& vp);

	// g_ATTACH
	void Render_Utility_Attach_Sampler_Planar(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Attach_Sampler_Volume(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Attach_Sampler_Directional(Utility_RenderList_Object ut, const VIEWPORT& vp);

	void Render_Utility_Attach_Planes(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp);
	void Render_Utility_Attach_Face_Vector(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp);
	void Render_Utility_Attach_Face_vector2(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Attach_Face_vector3(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Attach_Face_vector4(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Attach_Face_Vector_Parallel(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Attach_Face_vector_Normal_Aligned(Utility_RenderList_Object ut, const VIEWPORT& vp);
	void Render_Utility_Attach_Sampler(Utility_RenderList_Object utility_to_Render);

	void Render_Utility_Create_Light(Utility_RenderList_Object utility_to_Render, const VIEWPORT& vp);

	void Render_Light_Object( Utility_RenderList_Object ut );
	void Render_Light_Object_Properties(Utility_RenderList_Object ut, const VIEWPORT& vp);

#endif

#ifdef UTILITY_RENDERER_UTIL_FUNCTIONS

public:

	void PRE_STATE_APPLY(STATE state);
	void POST_STATE_APPLY(STATE state);
	void PRE_Viewport_Render_APPLY(  const VIEWPORT& vp );
	void POST_Viewport_Render_APPLY( const VIEWPORT& vp );

	//void add_Utility_To_RenderList(struct UtilityProperties * utility, struct Object_Package * obj_pack, PinholeCamera * camera_instance);
	void add_SceneObject_To_Object_RenderList(struct Object_Package * obj_pack , PinholeCamera* camera_instance);
	void add_Shape2D_To_RenderList(Shape shape, SHAPE_ID shape_id, struct UtilityProperties* utility = 0);
	void add_Utility_To_RenderList( struct UtilityProperties * utility, struct Object_Package * subject_pack, struct Object_Package * obj_pack, PinholeCamera* camera_instance);
	void add_Utility_To_RenderList( struct Object_Package* obj_pack, UTIL_RENDER_TYPE render_type, PinholeCamera* camera_instance);
	void add_Object_To_HighlightList(struct Object_Package * obj_pack, PinholeCamera * camera_instance);
	void add_Object_To_SceneRenderList(PinholeCamera camera, class SceneObject* focus_obj, int width, int height, int type, int index );
	void remove_Scenes();
	void remove_Scene(int index);

	void set_Cursor_To_Render(CURSOR_TYPE cursor_type);

public:
	void ResizeBuffers();
	void ResizeBuffers(int width, int height);
private:

	void createGeometryObject_OpenGL_FromSceneObject(int scene_object_index);
	void clear_geometry_stored_objects();
	void destroy_GeometryObject_OpenGL(GeometryObject_OpenGL * obj);

	void gather_Scene_Data();
	void insert_Sampler_with_Depth_Order(VIEWPORT_ID vp_id, Shape& shape);
	void insert_Sampler_with_Depth_Order(std::vector<Shape>& sampler_shapes, VIEWPORT_ID vp_id, Shape& shape);
	void depth_Order_Sampler_Shapes();

public:
	void createGeometryObject_OpenGL_FromSceneObject(class SceneObject* object);

#endif

#ifdef UTILITY_RENDERER_SHADER_FUNCTIONS

private:

	
	void Shader_start(const VIEWPORT& vp, bool enable_blend = false );
	void Shader_start(const VIEWPORT& vp, GLuint fbo, bool enable_blend );
	void Shader_end();
	
#endif

#ifdef UTILITY_RENDERER_SET_FUNCTIONS

	//glViewport()
public:

	void setRenderSize(int width, int height);


private:

	void setUp_RenderingViewport();
	void setUp_RenderingViewport(VIEWPORT viewport);
	void setUp_OutputViewport();
	void setUp_OutputViewport(VIEWPORT viewport);
	void reset_Viewport();

#endif

#ifdef UTILITY_RENDERER_GET_FUNCTIONS

public:
	

	//const optix::float2&   getMainViewPort_RenderSize();
	const optix::float2&   getRenderSize();
	//CUSTOM_VIEWPORT        getMainViewport();
	
	ShaderProgram*         getShaderProgram();
	GLuint                 getSecondarySceneTex(int index);
	
	void update_Scene_Camera(int index, Io_Mouse mouse_data)
	{
		if (m_scene_RenderList.size() > index)
		{
			m_scene_RenderList[index].camera->Update(Utilities::getMouseData());
		}
	}
	void freeze_Scene_Camera(int index)
	{
		if (m_scene_RenderList.size() > index)
		{
			m_scene_RenderList[index].camera->setState(CAM_NONE);
		}
	}


	bool checkIntegrity();


	void Transform_World_To_Screen(optix::float3 p_world, int vp_id , float& screen_x, float& screen_y);


#endif


	

	




};