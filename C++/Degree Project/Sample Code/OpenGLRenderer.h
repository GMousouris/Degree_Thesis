#pragma once


#include "GL/glew.h";
#include "GL/glut.h"

#include <optixu/optixpp_namespace.h>

#include "glm/glm.hpp"
#include "ShaderProgram.h"
#include "common_structs.h"

#include <vector>
#include <unordered_map>


#define FONT1 GLUT_BITMAP_8_BY_13
#define FONT2 GLUT_BITMAP_9_BY_15 
#define FONT3 GLUT_BITMAP_TIMES_ROMAN_10 
#define FONT4 GLUT_BITMAP_TIMES_ROMAN_24
#define FONT5 GLUT_BITMAP_HELVETICA_10 
#define FONT6 GLUT_BITMAP_HELVETICA_12 
#define FONT7 GLUT_BITMAP_HELVETICA_18 


//#define DEFAULT_ARC_DRAWING_METHOD
#define ARBITRARY_PLANE_ARC_DRAWING_METHOD




class OpenGLRenderer
{

	

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

	RENDERING_MODE m_rendering_mode;
	UtilityPlane m_selected_plane;

	int m_screen_width;
	int m_screen_height;
	glm::mat4 m_projection_matrix;
	glm::mat4 m_view_matrix;

	glm::vec3 DEF_AXIS_COLOR[3];
	glm::vec3 AXIS_COLOR[3];
	class GeometryNode * m_base_axis_node[3];
	glm::mat4 m_object_transformation_matrix, m_object_transformation_matrix_scaled, m_object_transformation_matrix_rotated_scaled;
	glm::mat4 m_object_rotation_matrix;

	int m_render_utility;
	int m_active_utility;
	int m_active_arc;

	AXIS_ORIENTATION m_axis_orientation;
	optix::float3 * M_WORKING_AXIS;
	class SceneObject * m_selected_object;
	optix::float3 m_selected_object_translation_attributes;
	optix::float3 m_utility_translation_attributes;
	float m_camera_inv_scale_factor;
	std::vector<IntersectionPoint> potentialIntersectionPoints;
	IntersectionPoint m_arc_isec_point, m_sphere_isec_point;
	bool m_focused_arc_point, m_focused_sphere_point;

	ShaderProgram m_utility_rendering_program;
	int m_focused_utility;

	// testing OptiXMesh -> GeometryNode //
	std::unordered_map<int, GeometryObject_OpenGL*> m_geometry_stored_objects;
	//GeometryObject_OpenGL * m_highlighted_object = nullptr;
	


public:

	bool Init();
	void Update(float dt);
	
	void Render();
	void Render_Highlighted_objects();

	int getFocusedUtility();
	IntersectionPoint getIsecPoint();
	float getInvCameraScaleFactor();
	optix::float3 * getWorkingAxis();

private:

	bool InitRenderingTechniques();
	bool InitDeferredShaderBuffers();
	bool InitGeometricMeshes();

	void createGeometryObject_OpenGL_FromSceneObject(int scene_object_index);
	void draw_Object_Outline(int scene_object_index, bool selected);



private:

	void ResizeBuffers();
	int  checkForUtilitySelection();

	int  testPlaneIntersection(Ray ray, Plane plane);
	void drawTriangle(optix::float3 p0, optix::float3 u, optix::float3 v);
	void drawTriangle(Triangle tri);
	void drawBBox(optix::float3 min , optix::float3 max, optix::float3 pos);
	void drawTangent(IntersectionPoint p , float rot_d);
	void drawArcSegment(IntersectionPoint p, float r, float start_angle, float d_angle, int segments);
	void drawGrid(optix::float3 o);
	void drawCircle(float _x, float _y, float _z, float _r , int segments , int vertical);
	void drawCircle2D(float cx, float cy, float r, int num_segments);
	void drawCircleAligned(optix::float3 c, float r, UtilityPlane plane, int num_segments);
	void draw_Arc_arbitr_plane(optix::float3 c, float r, float s_angle, float e_angle, optix::float3 &u, optix::float3& v, int Nsegments, int is_arc, GLenum type);
	void drawArc(float _x, float _y, float _z,  float _r, float start_angle, float d_angle , int segments , int vertical);
	void drawArc2(float _x, float _y, float _r, float depth, float start_angle, float d_angle, int segments);
	void drawLine3D(float * p1, float * p2);
	void drawLine2D(float * p1, float * p2);
	void drawPlane(optix::float3 p , optix::float3 u, optix::float3 v);
	void draw(float * p);
	void RenderText(glm::vec3 p, glm::vec3 color, void * font, char * msg);

};