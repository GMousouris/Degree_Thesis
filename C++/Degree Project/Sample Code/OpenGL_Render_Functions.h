#pragma once

#include "Utility_Renderer_Defines.h"
#include "Interface_Structs.h"
#include "common_structs.h"
#include "ShaderProgram.h"
#include "PinholeCamera.h"
#include "ViewportManager_structs.h"

namespace draw
{


	void Point(optix::float3 p, float size);
	void Points(optix::float3 * p, int arr_size, int offset = 0, float size = 1.0f , GLenum type = GL_POINTS );

	void Triangle_(optix::float3 p0, optix::float3 u, optix::float3 v);
	void Triangle_(Triangle tri);
	void Triangle_Outline(Triangle tri);
	void BBox(optix::float3 min, optix::float3 max, optix::float3 pos , float line_lenght = 1);
	void BBox_Planes(optix::float3 min, optix::float3 max, optix::float3 pos, float offset = 0.7f ,  bool filled = false, float line_lenght = 1);
	void BBox_dashed(optix::float3 min, optix::float3 max, float dist_ratio, float line_length = 1);
	void Tangent(IntersectionPoint p, float rot_d);
	void Tangent2(IntersectionPoint p, float rot_d);

	void ArcSegment(IntersectionPoint p, float r, float start_angle, float d_angle, int segments);
	void Grid(optix::float3 center, int num_segments, float seg_length, float seg_width, glm::vec3 color);
	void Circle(float _x, float _y, float _z, float _r, int segments, int vertical, GLenum type = GL_LINE_LOOP);
	void Circle(optix::float3 p, float _r, int segments, int vertical,float line_thickness = 1.0f, bool stripped = false,  GLenum type = GL_LINE_LOOP);
	void Circle2D(float cx, float cy, float r, int num_segments);
	void CircleAligned(optix::float3 c, float r, UtilityPlane plane, int num_segments);
	void Arc_arbitr_plane(optix::float3 c, float r, float s_angle, float e_angle, optix::float3 &u, optix::float3& v, int Nsegments, int is_arc, bool stripped, int stripple_factor, float thickness, GLenum type);
	void Arc_arbitr_plane(optix::float3 c, float r, float s_angle, float e_angle, optix::float3 &u, optix::float3& v, int Nsegments, int is_arc, GLenum type);
	void Arc_arbitr_plane_to_Origin(optix::float3 o,optix::float3 c, float r, float s_angle, float e_angle, optix::float3 &u, optix::float3& v, int Nsegments, int is_arc, GLenum type, int jump_index = 0);
	void Arc_on_plane(optix::float3 c, float r, float s_angle, float e_angle, optix::float3 u, optix::float3 v, int Nsegments = 30 , GLenum type = GL_POLYGON);
	void Arc(float _x, float _y, float _z, float _r, float start_angle, float d_angle, int segments, int vertical);
	void Arc2(float _x, float _y, float _r, float depth, float start_angle, float d_angle, int segments);
	void Line3D(float * p1, float * p2);
	void Line3D(optix::float3& p1, optix::float3& p2, float line_width = 1.0f);
	void Line2D(float * p1, float * p2);
	void Arrow(const optix::float3& p1, const optix::float3& p2,float size, bool stripped, float line_width = 2.0f);
	void Arrow(optix::float3& p1, optix::float3& p2, float size, const VIEWPORT& vp , bool stripped, float line_width = 1.0f);
	void stripped_Line3D(optix::float3 &p1, optix::float3 &p2, float frag_length , bool arrow = true , float line_width = 1.0f, int stripple_factor = 6 );
	void Plane(optix::float3 p, optix::float3 u, optix::float3 v);
	void Plane_(optix::float3 c, optix::float3 u, optix::float3 v, optix::float2 scale , float line_length = 1.0f , GLenum type = GL_POLYGON );
	void Plane_Centered(optix::float3 c, optix::float3 u, optix::float3 v, optix::float2 scale, float line_length = 1.0f, GLenum type = GL_POLYGON);
	void Plane_Centered(optix::float3 c, optix::float3 u, optix::float3 v, optix::float2 scale, optix::float3 * points, float line_length = 1.0f, GLenum type = GL_POLYGON);
	void Spot_Light(ShaderProgram& program ,optix::float3 c, optix::float3 u, optix::float3 v, int sign, optix::float2 scale, float dist, glm::vec3 surf_color = glm::vec3(1.0f), glm::vec3 outline_col = glm::vec3(0.1f), float line_length = 1.0f);
	void Plane( struct Plane plane, optix::float3 offset = ZERO_3f, float scale = 1.0f, float line_width = 1.0f,  GLenum type = GL_POLYGON);
	void Point3D(optix::float3 c, float size, glm::vec3 color);
	void Cube(optix::float3 c, float size, optix::float3 * M_WORKING_AXIS, bool fill = true);
	void Cube_Not_Centered(optix::float3 c, float size, optix::float3 * M_WORKING_AXIS, bool fill = true);
	void Cube(optix::float3 c, optix::float3 size, optix::float3 * M_WORKING_AXIS, bool fill = true);
	void Text(glm::vec3 p, glm::vec3 color, void * font, char * msg);
	void Text_onScreen(glm::vec2 p, glm::vec3 color, void * font, char * msg);
	void Cursor(float * mouse_pos, CURSOR_TYPE cursor_type , ShaderProgram * shader);
	void Cursor_Test(const optix::float2& mousePos, const optix::float2& window_size, CURSOR_TYPE cursor_type, ShaderProgram& program);
	void Cursor(const optix::float2& mousePos, const optix::float2& window_size, CURSOR_TYPE cursor_type, ShaderProgram * shader);
	void Cursor(const optix::float2& mouse_pos, CURSOR_TYPE cursor_type, const VIEWPORT& vp, ShaderProgram * shader);
	void CameraFocus(PinholeCamera camera_instance, float size, glm::vec3 color);
	void CameraFocus(PinholeCamera camera_instance, const VIEWPORT& vp, float size, glm::vec3 color);
	void Rectangle2D(glm::vec2 a, glm::vec2 b, glm::vec3 color,float line_width, bool dashed = false, int * render_sides = 0);
	void Rectangle2D(const optix::float3& a,const optix::float3& b, glm::vec3 color, float line_width, ShaderProgram* program,bool dashed = false);
	void Rectangle2D(const optix::float3& a, const optix::float3& b, glm::vec3 color, float line_width, ShaderProgram* program, const VIEWPORT& vp, bool dashed = false);
	void SolidAngle(float r, float length);
	void Frustum(optix::float3 pos, optix::float2 fov, optix::float2 clip_planes , ShaderProgram& program );
	void Frustum_Area_Light(optix::float3 pos, optix::float2 fov, optix::float2 clip_planes, ShaderProgram& program);
	void Frustum_Cone(optix::float3 pos, optix::float2 fov, optix::float2 clip_planes, optix::float3 n, ShaderProgram& program, float line_width = 1.0f , glm::vec3 color = glm::vec3(1.0f, 1.0f, 0.5f), bool is_directional = false );
	void Frustum_Cone2(optix::float3 pos, optix::float2 fov, optix::float2 clip_planes, optix::float3 n_, ShaderProgram& program, float line_width );
	void Light_Object(optix::float3 pos, Light_Parameters params, float scale = 1.0f, float line_width = 1.0f );
	void view_axis(optix::float3 o, optix::float3 * axis);
	//void FrontFraceVector(optix::float3 origin, optix::float3 dir);

};

