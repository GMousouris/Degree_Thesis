#include "OpenGLRenderer.h"

//#include <optix.h>

#include "GeometryNode.h"
#include "Tools.h"
#include <algorithm>
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OBJLoader.h"

#include "PinholeCamera.h"
#include "SceneObject.h"
#include "guiManager.h"

#include "Mediator.h"
#include "GeometryFunctions.h"
#include "Transformations.h"
#include "Utilities.h"

#include <iostream>

#define NEAR 0.1f  // 0.1
#define FAR 999999.f // 1500
#define M_FOV 45.0f

#define UTILITY_SCALE_FACTOR 0.78f
#define ROTATION_AXIS_SCALE_FACTOR 0.076823f // 0.078823f
#define TRANSLATION_AXIS_SCALE_FACTOR 0.03333f

#define CIRCLE_SEGMENTS 35
#define DEF_AXIS_COLOR_X glm::vec3(0.75,0,0)
#define DEF_AXIS_COLOR_Y glm::vec3(0,0.8,0)
#define DEF_AXIS_COLOR_Z glm::vec3(0,0,0.8)
#define HIGHLIGHT_COLOR glm::vec3(1.0f,1.0f,0.0f)

//#define UTILITY_INTERSECTION_DEBUGGING




OpenGLRenderer::OpenGLRenderer()
{

	m_screen_width = 0;
	m_screen_height = 0;

	for (int i = 0; i < 3; i++)
	{
		m_base_axis_node[i] = nullptr;
	}


	DEF_AXIS_COLOR[0] = DEF_AXIS_COLOR_X;
	DEF_AXIS_COLOR[1] = DEF_AXIS_COLOR_Y;
	DEF_AXIS_COLOR[2] = DEF_AXIS_COLOR_Z;


	AXIS_COLOR[0] = DEF_AXIS_COLOR_X;
	AXIS_COLOR[1] = DEF_AXIS_COLOR_Y;
	AXIS_COLOR[2] = DEF_AXIS_COLOR_Z;

	M_WORKING_AXIS = new optix::float3[3];
	M_WORKING_AXIS[0] = GLOBAL_AXIS[0];
	M_WORKING_AXIS[1] = GLOBAL_AXIS[1];
	M_WORKING_AXIS[2] = GLOBAL_AXIS[2];
	m_axis_orientation = AXIS_ORIENTATION::WORLD;

	m_render_utility = 0;
	m_render_cam_center_indicator = 0;
	m_active_utility = 0;
	

	m_selected_object = nullptr;
	m_selected_object_translation_attributes = optix::make_float3(0.0f);
	m_utility_translation_attributes = optix::make_float3(0.0f);
	m_camera_inv_scale_factor = 1.0f;
	m_focused_utility = -1;
	m_active_arc = -1;

	m_selected_plane = UtilityPlane::NONE;
	m_rendering_mode = RENDERING_MODE::TRIANGLES;
}

OpenGLRenderer::~OpenGLRenderer()
{
	for (int i = 0; i < 3; i++)
	{
		delete m_base_axis_node[i];
	}

	delete M_WORKING_AXIS;
}

bool OpenGLRenderer::Init()
{

	


	bool techniques_initialization = InitRenderingTechniques();
	bool buffers_initialization = InitDeferredShaderBuffers();
	bool meshes_initialization = InitGeometricMeshes();

	//If there was any errors
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		return false;
	}

	

	char * * argv = nullptr;
	int argc = 0;
	glutInit(&argc, argv);

	
	return techniques_initialization && buffers_initialization && meshes_initialization;
}

bool OpenGLRenderer::InitRenderingTechniques()
{
	bool initialized = true;

	std::string vertex_shader_path = "../Data/Shaders/utility_rendering.vert";
	std::string fragment_shader_path = "../Data/Shaders/utility_rendering.frag";

	m_utility_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_utility_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	m_utility_rendering_program.CreateProgram();
	m_utility_rendering_program.LoadUniform("uniform_projection_matrix");
	m_utility_rendering_program.LoadUniform("uniform_view_matrix");
	m_utility_rendering_program.LoadUniform("uniform_model_matrix");
	m_utility_rendering_program.LoadUniform("uniform_parent_matrix");
	m_utility_rendering_program.LoadUniform("uniform_parent_matrix_scaled");
	m_utility_rendering_program.LoadUniform("uniform_parent_matrix_rotated_scaled");

	m_utility_rendering_program.LoadUniform("uniform_use_model_matrix");
	m_utility_rendering_program.LoadUniform("uniform_use_parent_matrix");
	m_utility_rendering_program.LoadUniform("uniform_use_parent_matrix_scaled");
	m_utility_rendering_program.LoadUniform("uniform_use_parent_matrix_rotated_scaled");

	m_utility_rendering_program.LoadUniform("uniform_object_center");
	m_utility_rendering_program.LoadUniform("uniform_is_arc");
	m_utility_rendering_program.LoadUniform("uniform_diffuse");
	m_utility_rendering_program.LoadUniform("uniform_alpha");

	m_utility_rendering_program.LoadUniform("uniform_stable_circle");
	m_utility_rendering_program.LoadUniform("uniform_camera_pos");
	m_utility_rendering_program.LoadUniform("uniform_text_render");
	m_utility_rendering_program.LoadUniform("uniform_camera_scale_factor");
	m_utility_rendering_program.LoadUniform("uniform_selected_circle");
	m_utility_rendering_program.LoadUniform("uniform_circle");
	m_utility_rendering_program.LoadUniform("uniform_camera_w");
	m_utility_rendering_program.LoadUniform("uniform_utility_pos");

	m_utility_rendering_program.LoadUniform("outline_1st_pass");
	m_utility_rendering_program.LoadUniform("outline_2nd_pass");

	return initialized;
}

bool OpenGLRenderer::InitDeferredShaderBuffers()
{
	ResizeBuffers();
	return true;
}

bool OpenGLRenderer::InitGeometricMeshes()
{
	bool initialized = true;
	OBJLoader loader;

	auto mesh = loader.load("../Data/Assets/splittedAxis/xAxis3.obj");
	if (mesh != nullptr)
	{
		m_base_axis_node[0] = new GeometryNode();
		m_base_axis_node[0]->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Assets/splittedAxis/yAxis3.obj");
	if (mesh != nullptr)
	{
		m_base_axis_node[1] = new GeometryNode();
		m_base_axis_node[1]->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	mesh = loader.load("../Data/Assets/splittedAxis/zAxis3.obj");
	if (mesh != nullptr)
	{
		m_base_axis_node[2] = new GeometryNode();
		m_base_axis_node[2]->Init(mesh);
	}
	else
		initialized = false;
	delete mesh;

	return initialized;
}

void OpenGLRenderer::ResizeBuffers()
{
	int width = Mediator::RequestWindowSize(0);
	int height = Mediator::RequestWindowSize(1);

	if (m_screen_width != width || m_screen_height != height)
	{
		m_screen_width = width;
		m_screen_height = height;
		m_projection_matrix = glm::perspective(glm::radians(M_FOV), width / (float)height, NEAR, FAR);

		// pass updated projection matrix to shaders
		m_utility_rendering_program.Bind();
		glUniformMatrix4fv(m_utility_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
		m_utility_rendering_program.Unbind();
	}
	
}

int OpenGLRenderer::checkForUtilitySelection()
{

	if (!m_render_utility || guiManager::GetInstance().isObjectManipulationActive())
		return -1;


	int return_value = -1;
	m_focused_arc_point = m_focused_sphere_point = false;

	optix::float3 object_position = m_selected_object_translation_attributes;
	optix::float3 c = m_utility_translation_attributes;
	optix::float3 W = Mediator::RequestCameraInstance().getW();
	Ray ray = Geometry::SHAPES::createMouseRay();

	float radius;
	if (m_active_utility == GuiState::Utility::TRANSLATION)
		radius = UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor * 3.3f;
	else
		radius = UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor;

	// check if mouse position is in utility area before going in intersection tests
	{
		float dist = optix::length(ray.origin - c);
		optix::float3 mouse_pos = ray.origin + ray.direction * dist;
		float eq = (mouse_pos.x - c.x) * (mouse_pos.x - c.x) + (mouse_pos.y - c.y) * (mouse_pos.y - c.y) + (mouse_pos.z - c.z) * (mouse_pos.z - c.z);
		if (eq > radius * radius + 0.5f * m_camera_inv_scale_factor * m_camera_inv_scale_factor)
		{
#ifndef UTILITY_INTERSECTION_DEBUGGING
			m_selected_plane = UtilityPlane::NONE;
			return -1;
#endif
		}
		
	}

	optix::float3 plane_n;
	optix::float3 projected_point;
	float x, y, z;
	
	if (m_active_utility      == GuiState::Utility::TRANSLATION) // Translation 
	{
		float d_u = optix::dot( W, M_WORKING_AXIS[0] );
		float d_y = optix::dot( W, M_WORKING_AXIS[1] );
		float d_z = optix::dot( W, M_WORKING_AXIS[2] );

		float e_dist = 2.7f * m_camera_inv_scale_factor;
		float e = 0.1f * m_camera_inv_scale_factor;
		float rayDotLine_e = 0.003f;

		/* XYZ - plane */
		{
			
			plane_n = Geometry::VECTOR_SPACE::getPerpendicularBasePlane(ray.direction);
			Geometry::RAY::Intersect_Plane(ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);
			x = projected_point.x;
			y = projected_point.y;
			z = projected_point.z;

			float ee = 0.2f * m_camera_inv_scale_factor;

			if (plane_n.x == 1)
			{
				if (fabs(z - c.z) < ee && fabs(y - c.y) < ee)
				{
					m_selected_plane = UtilityPlane::XYZ;
					return 123;
				}
			}
			else if (plane_n.y == 1)
			{
				if (fabs(x - c.x) < ee && fabs(z - c.z) < ee)
				{
					m_selected_plane = UtilityPlane::XYZ;
					return 123;
				}
			}
			else if (plane_n.z == 1)
			{
				
				if (fabs(x - c.x) < ee && fabs(y - c.y) < ee)
				{
					m_selected_plane = UtilityPlane::XYZ;
					return 123;
				}
			}
		
			
		}

		/* X - axis */
		{
			if (fabs(d_y) < fabs(d_z))
				plane_n = M_WORKING_AXIS[2];
			else
				plane_n = M_WORKING_AXIS[1];

			Geometry::RAY::Intersect_Plane(ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);
#define LINE_INTERSECTION_TEST
#ifdef LINE_INTERSECTION_TEST
			float length = optix::length(projected_point - c);
			optix::float3 _dir = optix::normalize(projected_point - c);
			float _dot = optix::dot(_dir, M_WORKING_AXIS[0]);
			
			if (1.0f - _dot < rayDotLine_e && length < e_dist)
			{
				m_selected_plane = UtilityPlane::X;
				return 0;
			}
			
#endif

#ifdef LINE_INTERSECTION
			x = projected_point.x;
			y = projected_point.y;
			z = projected_point.z;
			
			float eq1 = y - c.y;
			float eq2 = z - c.z;
			float eq3 = x - c.x;
	
			if (fabs(eq1) < e && fabs(eq2) < e && eq3 < e_dist && eq3 > 0.0f)
			{
				//std::cout << "intersection X - axis!" << std::endl;
				m_selected_plane = UtilityPlane::X;
				return 0;
			}
#endif
			
		}

		/* Y - axis */
		{
			if (fabs(d_u) < fabs(d_z))
				plane_n = M_WORKING_AXIS[2];
			else
				plane_n = M_WORKING_AXIS[0];

			Geometry::RAY::Intersect_Plane(ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);
#define LINE_INTERSECTION_TEST
#ifdef LINE_INTERSECTION_TEST
			float length = optix::length(projected_point - c);
			optix::float3 _dir = optix::normalize(projected_point - c);
			float _dot = optix::dot(_dir, M_WORKING_AXIS[1]);
			//std::cout << "\n length : " << length << std::endl;
			//std::cout << " dot : " << _dot << std::endl;
			if (1.0f - _dot < rayDotLine_e && length < e_dist)
			{
				m_selected_plane = UtilityPlane::Y;
				return 1;
			}
			
#endif

#ifdef LINE_INTERSECTION
			x = projected_point.x;
			y = projected_point.y;
			z = projected_point.z;

			float eq3 = y - c.y;
			float eq2 = z - c.z;
			float eq1 = x - c.x;

			if (fabs(eq1) < e && fabs(eq2) < e && eq3 < e_dist && eq3 > 0.0f)
			{
				//std::cout << "intersection Y - axis!" << std::endl;
				m_selected_plane = UtilityPlane::Y;
				return 1;
			}
#endif
		}
		
		/* Z - axis */
		{
			if (fabs(d_u) < fabs(d_y))
				plane_n = M_WORKING_AXIS[1];
			else
				plane_n = M_WORKING_AXIS[0];

			Geometry::RAY::Intersect_Plane(ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);
#define LINE_INTERSECTION_TEST
#ifdef LINE_INTERSECTION_TEST
			float length = optix::length(projected_point - c);
			optix::float3 _dir = optix::normalize(projected_point - c);
			float _dot = optix::dot(_dir, M_WORKING_AXIS[2]);
			//std::cout << "\n length : " << length << std::endl;
			//std::cout << " dot : " << _dot << std::endl;
			if (1.0f - _dot < rayDotLine_e && length < e_dist)
			{
				m_selected_plane = UtilityPlane::Z;
				return 2;
			}
#endif

#ifdef LINE_INTERSECTION
			x = projected_point.x;
			y = projected_point.y;
			z = projected_point.z;

			float eq1 = y - c.y;
			float eq3 = z - c.z;
			float eq2 = x - c.x;

			if (fabs(eq1) < e && fabs(eq2) < e && fabs(eq3) < e_dist && eq3 > 0.0f)
			{
				//std::cout << "intersection Z - axis!" << std::endl;
				m_selected_plane = UtilityPlane::Z;
				return 2;
			}
#endif
			
		}


#define PLANE_INTERSECTION_TEST_TRIANGLE_METHOD
#ifdef PLANE_INTERSECTION_TEST_TRIANGLE_METHOD
		
		float scale = UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor;//UTILITY_SCALE_FACTOR * m_camera_inv_scale_factor;
		optix::float3 isec_point;

		Plane plane_xy = Geometry::SHAPES::createPlane(c, M_WORKING_AXIS[0] * scale, M_WORKING_AXIS[1] * scale);
		Plane plane_xz = Geometry::SHAPES::createPlane(c, M_WORKING_AXIS[0] * scale, M_WORKING_AXIS[2] * scale);
		Plane plane_yz = Geometry::SHAPES::createPlane(c, M_WORKING_AXIS[1] * scale, M_WORKING_AXIS[2] * scale);

		
		if (Geometry::RAY::Intersect_Parallelogram(ray, plane_xz, isec_point) == 1)
		{
			//std::cout << "XZ" << std::endl;
			m_selected_plane = UtilityPlane::XZ;
			return 20;
		}
		if (Geometry::RAY::Intersect_Parallelogram(ray, plane_xy, isec_point) == 1)
		{
			//std::cout << "XY" << std::endl;
			IntersectionPoint iP;
			iP.p = isec_point;
			iP.plane_id = UtilityPlane::XY;
			potentialIntersectionPoints.push_back(iP);
		}
		if (Geometry::RAY::Intersect_Parallelogram(ray, plane_yz, isec_point) == 1)
		{
			//std::cout << "YZ" << std::endl;
			IntersectionPoint iP;
			iP.p = isec_point;
			iP.plane_id = UtilityPlane::YZ;
			potentialIntersectionPoints.push_back(iP);
		}
		

#endif


#ifdef PLANE_INTERSECTION_DEGENERATE_METHOD
		/* XZ - plane */
		{
			plane_n = M_WORKING_AXIS[1];

			Geometry::RAY::Intersect_Plane(ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);
			x = projected_point.x;
			y = projected_point.y;
			z = projected_point.z;

			float e = 0.8f * m_camera_inv_scale_factor;

			if (x - c.x < e && z - c.z < e && x > c.x && z > c.z)
			{
				//std::cout << "XZ plane intersection!" << std::endl;
				m_selected_plane = UtilityPlane::XZ;
				return 20;
			}
		}

		/* XY - plane */
		{
			plane_n = M_WORKING_AXIS[2];

			Geometry::RAY::Intersect_Plane(ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);
			x = projected_point.x;
			y = projected_point.y;
			z = projected_point.z;

			float e = 0.8f * m_camera_inv_scale_factor;

			if (x - c.x < e && y - c.y < e && x > c.x && y > c.y)
			{
				//std::cout << "XY plane intersection!" << std::endl;
				IntersectionPoint ip;
				ip.p = projected_point;
				ip.plane_id = UtilityPlane::XY;
				potentialIntersectionPoints.push_back(ip);
				//return 20;
			}
		}

		/* YZ - plane */
		{

			plane_n = M_WORKING_AXIS[0];

			Geometry::RAY::Intersect_Plane(ray, Geometry::SHAPES::createPlane(c, plane_n), projected_point);
			x = projected_point.x;
			y = projected_point.y;
			z = projected_point.z;

			float e = 0.8f * m_camera_inv_scale_factor;

			if (y - c.y < e && z - c.z< e && y > c.y && z > c.z)
			{
				//std::cout << "YZ plane intersection!" << std::endl;
				IntersectionPoint ip;
				ip.p = projected_point;
				ip.plane_id = UtilityPlane::YZ;
				potentialIntersectionPoints.push_back(ip);
				//return 12;
			}
		}
#endif
		
	}
	else if (m_active_utility == GuiState::Utility::ROTATION)
	{

#define SPHERE_INTERSECTION
#ifdef SPHERE_INTERSECTION
	    //testRaySphereIntersection(ray_origin, ray_dir, c, 1.1f * 0.6 * m_camera_inv_scale_factor * m_camera_inv_scale_factor);
		float e_r = 1.1 * 0.6 * m_camera_inv_scale_factor * m_camera_inv_scale_factor;
		float real_r = 1.0f * 0.6 * m_camera_inv_scale_factor;
		Geometry::RAY::Intersect_SphereArcs(ray, c, e_r, real_r, &M_WORKING_AXIS[0], m_axis_orientation, potentialIntersectionPoints);
#endif

//#define ARC_INTERSECTION
#ifdef ARC_INTERSECTION
	else if (m_active_utility == GuiState::Utility::ROTATION && false) // Rotation
	{

		float r = 1.0f * 0.6f * m_camera_inv_scale_factor * m_camera_inv_scale_factor; //1.3f * 0.6f * m_camera_inv_scale_factor;
		float eq1;
		float eq2 = r * r;
		float e = 0.25f * m_camera_inv_scale_factor * m_camera_inv_scale_factor; // 0.4 * m_cam * m_cam
		float dot_e = 0.05f; // 0.1
		float normal_e = 0.05f; // 0.05

		/* X - axis Circle  (YZ - plane)*/
		{

			// check if circle is parallel to ray direction
			// test intersection ray to line instead of circle
			// If intersection just set Tanget as the def tang perp to current plane (eg. if plane : (1,0,0) -> tang :  (0,1,0) , etc.
			if (fabs(optix::dot(ray_dir, GLOBAL_AXIS[0])) < dot_e)
			{
				plane_n = Geometry::VECTOR_SPACE::getPerpendicularBasePlane(ray_dir);
				Geometry::RAY::Intersect_Plane(ray_origin, ray_dir, c, plane_n, projected_point);

				// check if isec_point's normal is positive and reject
				optix::float3 circle_point_normal = projected_point - c;
				if (optix::dot(circle_point_normal, W) + normal_e < 0.0f || true)
				{


					x = projected_point.x;
					y = projected_point.y;
					z = projected_point.z;

					float e_dist = 1.3f * m_camera_inv_scale_factor;
					float ee = 0.05f * m_camera_inv_scale_factor;

					float eq3 = y - c.y;
					float eq1 = z - c.z;
					float eq2 = x - c.x;

					float _z;
					{
						float a = 1.0f;
						float b = -2 * c.z;
						float g = powf(x - c.x, 2) + powf(y - c.y, 2) + c.z * c.z - r * r;

						float d = b * b - 4 * a*g;
						float z1 = (-b + sqrtf(d)) / (2.0f*a);
						float z2 = (-b - sqrtf(d)) / (2.0f*a);

						optix::float3 cam_pos = Mediator::RequestCameraInstance().getCameraPos();
						optix::float3 dist_z1 = optix::make_float3(x, y, z1) - cam_pos;
						optix::float3 dist_z2 = optix::make_float3(x, y, z2) - cam_pos;
						if (optix::length(dist_z1) < optix::length(dist_z2))
							_z = z1;
						else
							_z = z2;
					}

					if (fabs(eq2) < e &&  fabs(eq3) < e_dist)
					{

						plane_n = optix::make_float3(1, 0, 0);
						Geometry::RAY::Intersect_Plane(ray_origin, ray_dir, c, plane_n, projected_point);

						IntersectionPoint ip;
						ip.p = projected_point;
						ip.p.z = _z;
						ip.plane_id = UtilityPlane::X;
						potentialIntersectionPoints.push_back(ip);


					}

				}

			}
			else
			{
				plane_n = optix::make_float3(1, 0, 0);
				Geometry::RAY::Intersect_Plane(ray_origin, ray_dir, c, plane_n, projected_point);

				// check if isec_point's normal is positive and reject
				optix::float3 circle_point_normal = projected_point - c;
				if (optix::dot(circle_point_normal, W) <= 0.0f)
				{
					x = projected_point.x;
					y = projected_point.y;
					z = projected_point.z;

					eq1 = (y - c.y)*(y - c.y) + (z - c.z)*(z - c.z);

					if (fabs(eq1 - eq2) < e)
					{
						//std::cout << "intersection on X-axis Circle!" << std::endl;
						//m_selected_plane = UtilityPlane::X;
						//return 0;
						IntersectionPoint ip;
						ip.p = projected_point;
						ip.plane_id = UtilityPlane::X;
						potentialIntersectionPoints.push_back(ip);
					}
				}

			}

		}

		/* Y - axis Circle  (XZ - plane)*/
		{

			if (fabs(optix::dot(ray_dir, GLOBAL_AXIS[1])) < dot_e)
			{
				plane_n = Geometry::VECTOR_SPACE::getPerpendicularBasePlane(ray_dir);
				Geometry::RAY::Intersect_Plane(ray_origin, ray_dir, c, plane_n, projected_point);

				// check if isec_point's normal is positive and reject
				optix::float3 circle_point_normal = projected_point - c;
				if (optix::dot(circle_point_normal, W) + normal_e <= 0.0f)
				{

					x = projected_point.x;
					y = projected_point.y;
					z = projected_point.z;

					float e_dist = 5.f * m_camera_inv_scale_factor;
					float e = 0.05f * m_camera_inv_scale_factor;

					float eq3 = y - c.y;
					float eq2 = z - c.z;
					float eq1 = x - c.x;

					if (fabs(eq3) < e  && fabs(eq1) < e_dist)
					{
						//std::cout << "intersection on X-axis Circle!" << std::endl;
						//m_selected_plane = UtilityPlane::Y;
						//return 1;
						IntersectionPoint ip;
						ip.p = projected_point;
						ip.plane_id = UtilityPlane::Y;
						potentialIntersectionPoints.push_back(ip);
					}
				}
			}
			else
			{
				plane_n = optix::make_float3(0, 1, 0);
				Geometry::RAY::Intersect_Plane(ray_origin, ray_dir, c, plane_n, projected_point);

				// check if isec_point's normal is positive and reject
				optix::float3 circle_point_normal = projected_point - c;
				if (optix::dot(circle_point_normal, W) <= 0.0f)
				{
					x = projected_point.x;
					y = projected_point.y;
					z = projected_point.z;

					eq1 = (x - c.x)*(x - c.x) + 0 + (z - c.z)*(z - c.z);

					if (fabs(eq1 - eq2) < e)
					{
						//std::cout << "intersection on Y-axis Circle!" << std::endl;
						//m_selected_plane = UtilityPlane::Y;
						//return 1;
						IntersectionPoint ip;
						ip.p = projected_point;
						ip.plane_id = UtilityPlane::Y;
						potentialIntersectionPoints.push_back(ip);
					}

				}
			}

		}

		/* Z - axis Circle  (YZ - plane)*/
		{

			if (fabs(optix::dot(ray_dir, GLOBAL_AXIS[2])) < dot_e)
			{
				plane_n = Geometry::VECTOR_SPACE::getPerpendicularBasePlane(ray_dir);
				Geometry::RAY::Intersect_Plane(ray_origin, ray_dir, c, plane_n, projected_point);

				// check if isec_point's normal is positive and reject
				optix::float3 circle_point_normal = projected_point - c;
				if (optix::dot(circle_point_normal, W) + normal_e <= 0.0f)
				{
					x = projected_point.x;
					y = projected_point.y;
					z = projected_point.z;

					float e_dist = 5.f * m_camera_inv_scale_factor;
					float e = 0.05f * m_camera_inv_scale_factor;

					float eq3 = y - c.y;
					float eq2 = z - c.z;
					float eq1 = x - c.x;

					if (fabs(eq2) < e  && fabs(eq1) < e_dist)
					{
						//std::cout << "intersection on X-axis Circle!" << std::endl;
						//m_selected_plane = UtilityPlane::Z;
						//return 2;
						IntersectionPoint ip;
						ip.p = projected_point;
						ip.plane_id = UtilityPlane::Z;
						potentialIntersectionPoints.push_back(ip);
					}

				}
			}
			else
			{
				plane_n = optix::make_float3(0, 0, 1);
				Geometry::RAY::Intersect_Plane(ray_origin, ray_dir, c, plane_n, projected_point);

				// check if isec_point's normal is positive and reject
				optix::float3 circle_point_normal = projected_point - c;
				if (optix::dot(circle_point_normal, W) <= 0.0f)
				{

					x = projected_point.x;
					y = projected_point.y;
					z = projected_point.z;
					eq1 = (x - c.x)*(x - c.x) + (y - c.y)*(y - c.y) + 0;

					if (fabs(eq1 - eq2) < e)
					{
						//std::cout << "intersection on Z-axis Circle!" << std::endl;
						//m_selected_plane = UtilityPlane::Z;
						//return 2;
						IntersectionPoint ip;
						ip.p = projected_point;
						ip.plane_id = UtilityPlane::Z;
						potentialIntersectionPoints.push_back(ip);
					}
				}
			}

		}

	}
#endif
	}
	else if (m_active_utility == GuiState::Utility::SCALE)
	{
		
	}
	
	

	// get the nearest intersetion point if any
	if (potentialIntersectionPoints.size() != 0)
	{
		optix::float3 cam_pos = Mediator::RequestCameraInstance().getCameraPos();

		float z_min = optix::length(potentialIntersectionPoints[0].p - cam_pos);
		int index = 0;
		for (int i = 1; i < potentialIntersectionPoints.size(); i++)
		{
			float z = optix::length(potentialIntersectionPoints[i].p - cam_pos);
			if (z < z_min)
			{
				z_min = z;
				index = i;
			}
		}

		int plane_id = potentialIntersectionPoints[index].plane_id;
		if (m_active_utility == GuiState::Utility::ROTATION)
		{
			m_arc_isec_point.p = potentialIntersectionPoints[index].p;
			m_arc_isec_point.normal = potentialIntersectionPoints[index].normal;
			m_arc_isec_point.plane_id = potentialIntersectionPoints[index].plane_id;
			m_arc_isec_point.tangent = Geometry::VECTOR_SPACE::getCircleTangentOnPlane(m_arc_isec_point, &M_WORKING_AXIS[0], m_arc_isec_point.plane_id);
			m_arc_isec_point.out_normal = optix::cross(m_arc_isec_point.normal, m_arc_isec_point.tangent);
			m_arc_isec_point.center = m_utility_translation_attributes;
			m_focused_arc_point = true;

		}
		
		potentialIntersectionPoints.clear();
		switch (plane_id)
		{
		case  X: m_selected_plane = X;  return 0;
		case  Y: m_selected_plane = Y;  return 1;
		case  Z: m_selected_plane = Z;  return 2;
		case XZ: m_selected_plane = XZ; return 20;
		case XY: m_selected_plane = XY; return 10;
		case YZ: m_selected_plane = YZ; return 12;
		}
	}

	
	m_selected_plane = UtilityPlane::NONE;
	return -2;
}

void OpenGLRenderer::Update(float dt)
{
	ResizeBuffers();


	// set Highlighted axis
	m_active_utility = guiManager::GetInstance().getGuiState();

	
	
	if (m_active_utility == GuiState::Utility::TRANSLATION || m_active_utility == GuiState::Utility::ROTATION || m_active_utility == GuiState::Utility::SCALE && !guiManager::GetInstance().isObjectManipulationActive())
	{
		
		//std::cout << "A" << std::endl;
		AXIS_COLOR[0] = DEF_AXIS_COLOR[0];
		AXIS_COLOR[1] = DEF_AXIS_COLOR[1];
		AXIS_COLOR[2] = DEF_AXIS_COLOR[2];

		switch (m_selected_plane)
		{
		case X:  AXIS_COLOR[0] = HIGHLIGHT_COLOR;
			break;
		case Y:  AXIS_COLOR[1] = HIGHLIGHT_COLOR;
			break;
		case Z:  AXIS_COLOR[2] = HIGHLIGHT_COLOR;
			break;
		case XY: AXIS_COLOR[0] = HIGHLIGHT_COLOR;
			     AXIS_COLOR[1] = HIGHLIGHT_COLOR;
			break;

		case XZ: AXIS_COLOR[0] = HIGHLIGHT_COLOR;
			     AXIS_COLOR[2] = HIGHLIGHT_COLOR;
			break;

		case YZ: AXIS_COLOR[1] = HIGHLIGHT_COLOR;
			     AXIS_COLOR[2] = HIGHLIGHT_COLOR;

			break;
		case XYZ: /*
			      AXIS_COLOR[0] = HIGHLIGHT_COLOR;
			      AXIS_COLOR[1] = HIGHLIGHT_COLOR;
			      AXIS_COLOR[2] = HIGHLIGHT_COLOR;
				  */
			break;
		}
	}
	
	// fill in necessary data if any object is selected
	if( ( m_selected_object = Mediator::RequestSelectedObject()) != NULL && (m_active_utility == GuiState::Utility::TRANSLATION || m_active_utility == GuiState::Utility::ROTATION || m_active_utility == GuiState::Utility::SCALE) )
	{
		bool selected_obj_changed = guiManager::GetInstance().isSelectedObjectChanged();

		// get working coord system
		if (guiManager::GetInstance().getAxisOrientation(m_axis_orientation) || (m_selected_object->isRotationChanged() && m_axis_orientation == AXIS_ORIENTATION::LOCAL) || selected_obj_changed)
		{
			switch (m_axis_orientation)
			{
			case WORLD:
				M_WORKING_AXIS = BASE_TRANSFORM::AXIS(m_selected_object, TO_WORLD);
				m_object_rotation_matrix = glm::mat4(1.0f);
				break;
			case LOCAL:
				M_WORKING_AXIS = BASE_TRANSFORM::AXIS(m_selected_object, TO_LOCAL);
				m_object_rotation_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(m_selected_object->getRotationMatrix());
				break;
			}
		}

		// get world position
		m_selected_object_translation_attributes = m_selected_object->getTranslationAttributes();
		m_utility_translation_attributes = m_selected_object_translation_attributes;
		glm::vec3 world_pos = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(m_utility_translation_attributes);

		// calculate camera inverse scale factor
		float obj_dist = m_selected_object->getDistanceFromCamera();
		optix::float3 delta = m_selected_object->getAttributesDelta(0);
		if (m_active_utility == GuiState::Utility::TRANSLATION && (delta.x == 0 && delta.y == 0 && delta.z == 0))
			m_camera_inv_scale_factor = obj_dist * TRANSLATION_AXIS_SCALE_FACTOR;
		else if (m_active_utility == GuiState::Utility::ROTATION)
			m_camera_inv_scale_factor = obj_dist * ROTATION_AXIS_SCALE_FACTOR;

		
		// calculate object_transformation_matrix
		m_object_transformation_matrix =
			  glm::translate(glm::mat4(1.0f), world_pos)
			* m_object_rotation_matrix
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.6f * m_camera_inv_scale_factor));
		
		m_object_transformation_matrix_scaled =
			  glm::translate(glm::mat4(1.0f), world_pos)
			* glm::scale(glm::mat4(1.0f), glm::vec3(1.3f * 0.6f * m_camera_inv_scale_factor));
		
		m_object_transformation_matrix_rotated_scaled =
			m_object_transformation_matrix * glm::scale(glm::mat4(1.0f), glm::vec3(1.3f));

		/*
		// set up camera look at
		bool camera_changed = false;
		optix::float3 cam_pos, center, camera_up;
		if (camera_changed = Mediator::RequestCameraInstance().isChanged())
		{
			optix::float3 cam_pos = Mediator::RequestCameraInstance().getCameraPos();
			optix::float3 center = Mediator::RequestCameraInstance().getCenter();
			optix::float3 camera_up = Mediator::RequestCameraInstance().getV();

			glm::vec3 pos = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
			glm::vec3 up = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
			glm::vec3 target = glm::vec3(center.x, center.y, center.z);
			m_view_matrix = glm::lookAt(pos, target, up);
			Mediator::RequestCameraInstance().reset();
		}

		// update shaders
		m_utility_rendering_program.Bind();
		//glUniform1i(m_utility_rendering_program["uniform_use_model_matrix"], 0);
		//glUniform1i(m_utility_rendering_program["uniform_use_parent_matrix"], 0);
		//glUniform1i(m_utility_rendering_program["uniform_use_parent_matrix_scaled"], 0);
		//glUniform1i(m_utility_rendering_program["uniform_use_parent_matrix_rotated_scaled"], 0);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		glUniformMatrix4fv(m_utility_rendering_program["uniform_parent_matrix"], 1, GL_FALSE, glm::value_ptr(m_object_transformation_matrix));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_parent_matrix_rotated_scaled"], 1, GL_FALSE, glm::value_ptr(m_object_transformation_matrix_rotated_scaled));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_parent_matrix_scaled"], 1, GL_FALSE, glm::value_ptr(m_object_transformation_matrix_scaled));
		glUniform1f(m_utility_rendering_program["uniform_camera_scale_factor"], m_camera_inv_scale_factor);
		glUniform3f(m_utility_rendering_program["uniform_object_center"], world_pos.x , world_pos.y , world_pos.z);
		glUniform3f(m_utility_rendering_program["uniform_utility_pos"], m_utility_translation_attributes.x, m_utility_translation_attributes.y, m_utility_translation_attributes.z);
		glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
		glUniform1i(m_utility_rendering_program["uniform_circle"], -2);
		glUniform1i(m_utility_rendering_program["uniform_selected_circle"], -4);
		if (camera_changed)
		{
			optix::float3 m_camera_w = Mediator::RequestCameraInstance().getW();
			glUniformMatrix4fv(m_utility_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
			glUniform3f(m_utility_rendering_program["uniform_camera_pos"], cam_pos.x, cam_pos.y, cam_pos.z);
			glUniform3f(m_utility_rendering_program["uniform_camera_w"], m_camera_w.x, m_camera_w.y, m_camera_w.z);
		}
		m_utility_rendering_program.Unbind();
		*/

		m_render_utility = 1;
	}
	
	// Update Camera and other variables
	if (true)
	{
		bool camera_changed = false;
		optix::float3 cam_pos, center, camera_up;
		if (camera_changed = Mediator::RequestCameraInstance().isChanged())
		{
			m_render_cam_center_indicator = 1;
			optix::float3 cam_pos = Mediator::RequestCameraInstance().getCameraPos();
			optix::float3 center = Mediator::RequestCameraInstance().getCenter();
			optix::float3 camera_up = Mediator::RequestCameraInstance().getV();

			glm::vec3 pos = glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z);
			glm::vec3 up = glm::vec3(camera_up.x, camera_up.y, camera_up.z);
			glm::vec3 target = glm::vec3(center.x, center.y, center.z);
			m_view_matrix = glm::lookAt(pos, target, up);
			Mediator::RequestCameraInstance().reset();
			
		}
		else
			m_render_cam_center_indicator = 0;

		// update shaders
		m_utility_rendering_program.Bind();
		//glUniform1i(m_utility_rendering_program["uniform_use_model_matrix"], 0);
		//glUniform1i(m_utility_rendering_program["uniform_use_parent_matrix"], 0);
		//glUniform1i(m_utility_rendering_program["uniform_use_parent_matrix_scaled"], 0);
		//glUniform1i(m_utility_rendering_program["uniform_use_parent_matrix_rotated_scaled"], 0);
		glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		glUniformMatrix4fv(m_utility_rendering_program["uniform_parent_matrix"], 1, GL_FALSE, glm::value_ptr(m_object_transformation_matrix));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_parent_matrix_rotated_scaled"], 1, GL_FALSE, glm::value_ptr(m_object_transformation_matrix_rotated_scaled));
		glUniformMatrix4fv(m_utility_rendering_program["uniform_parent_matrix_scaled"], 1, GL_FALSE, glm::value_ptr(m_object_transformation_matrix_scaled));
		glUniform1f(m_utility_rendering_program["uniform_camera_scale_factor"], m_camera_inv_scale_factor);
		//glUniform3f(m_utility_rendering_program["uniform_object_center"], world_pos.x, world_pos.y, world_pos.z);
		glUniform3f(m_utility_rendering_program["uniform_utility_pos"], m_utility_translation_attributes.x, m_utility_translation_attributes.y, m_utility_translation_attributes.z);
		glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
		glUniform1i(m_utility_rendering_program["uniform_circle"], -2);
		glUniform1i(m_utility_rendering_program["uniform_selected_circle"], -4);
		if (camera_changed)
		{
			optix::float3 m_camera_w = Mediator::RequestCameraInstance().getW();
			glUniformMatrix4fv(m_utility_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
			glUniform3f(m_utility_rendering_program["uniform_camera_pos"], cam_pos.x, cam_pos.y, cam_pos.z);
			glUniform3f(m_utility_rendering_program["uniform_camera_w"], m_camera_w.x, m_camera_w.y, m_camera_w.z);
		}
		m_utility_rendering_program.Unbind();
	}

	// get focused utility ( utility objects intersection tests )
	m_focused_utility = checkForUtilitySelection();

}


void OpenGLRenderer::Render()
{
	

	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// openGL parameters
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glClear(GL_DEPTH_BUFFER_BIT);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// render only points, lines, triangles
			switch (m_rendering_mode)
			{
			case RENDERING_MODE::TRIANGLES:
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				break;
			case RENDERING_MODE::LINES:
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				break;
			case RENDERING_MODE::POINTS:
				glPointSize(2);
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				break;
			};

			glDisable(GL_LINE_SMOOTH);
			glLineWidth(1.45f);
		}
		

		// Render utility...
		m_utility_rendering_program.Bind();




		// Render Highlighted Objects if any
		Render_Highlighted_objects();
		
		if (m_render_cam_center_indicator)
			drawCameraCenterIndicator(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(Mediator::RequestCameraInstance().getCenter()), 4.0f, glm::vec3(0, 1, 0));

		//if (!m_render_utility)
		//{
		//	m_utility_rendering_program.Unbind();
		//	return;
		//}

		glm::mat4 model_matrix;
		glm::vec3 diffuseColor;

		// Render Utility gui if object is selected..
		if (m_selected_object != NULL)
		{
			if (m_active_utility == GuiState::Utility::TRANSLATION)
			{

				glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_object_transformation_matrix));
				//glUniform1i(m_utility_rendering_program["uniform_use_model_matrix"], 1);

				// Render Translation Axis
				{
					// pass the model matrix to glsl , need it when object is on local space
					for (int i = 0; i < 3; i++)
					{
						glBindVertexArray(m_base_axis_node[i]->m_vao);
						for (int j = 0; j < m_base_axis_node[i]->parts.size(); j++)
						{
							optix::float3 p1 = optix::make_float3(0.0f) + GLOBAL_AXIS[i] * 0.5f;
							optix::float3 p2 = GLOBAL_AXIS[i] * 4.0f;
							optix::float3 p3 = p2 + GLOBAL_AXIS[1] * 0.5f;

							// render translation axis line segment
							{
								diffuseColor = AXIS_COLOR[i] * 0.9f + glm::vec3(0.1f) * GLM_GLOBAL_INV_AXIS[i];
								glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
								drawLine3D((float*)&p1, (float*)&p2);
							}

							// render translation axis cone parts
							{
								// cone's back darker color
								diffuseColor = DEF_AXIS_COLOR[i];
								if (j == 0)
									diffuseColor *= 0.5f;

								glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
								glDrawArrays(GL_TRIANGLES, m_base_axis_node[i]->parts[j].start_offset, m_base_axis_node[i]->parts[j].count);
							}

							// render translation axis logo
							{
								if (AXIS_COLOR[i] != HIGHLIGHT_COLOR)
									diffuseColor = glm::vec3(0.65f);
								else
									diffuseColor = AXIS_COLOR[i];

								glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
								glUniform1i(m_utility_rendering_program["uniform_text_render"], 1);
								RenderText(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p3), glm::vec3(1), FONT6, GLOBAL_AXIS_NAME[i]);
								glUniform1i(m_utility_rendering_program["uniform_text_render"], 0);
							}

						}
					}
				}

				// Render XYZ plane
				{
					glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 1);
					diffuseColor = glm::vec3(0.4f);
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);

					glBegin(GL_LINE_LOOP);
					glVertex3f(0.4, -0.4, 0.0f);
					glVertex3f(0.4, 0.4, 0.0f);
					glVertex3f(-0.4, 0.4, 0.0f);
					glVertex3f(-0.4, -0.4, 0.0f);
					glEnd();

					if (m_selected_plane == XYZ)
					{
						diffuseColor = HIGHLIGHT_COLOR;
						glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
						glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);

						glDisable(GL_CULL_FACE);
						glBegin(GL_POLYGON);
						glVertex3f(0.4, -0.4, 0.0f);
						glVertex3f(0.4, 0.4, 0.0f);
						glVertex3f(-0.4, 0.4, 0.0f);
						glVertex3f(-0.4, -0.4, 0.0f);
						glEnd();
						glEnable(GL_CULL_FACE);
					}
					//drawPlane(optix::make_float3(0.5f,0.5f,0.0f), 1.0f * GLOBAL_AXIS[0], 1.0f * GLOBAL_AXIS[1]);
					glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);

				}

				// Render XY , XZ , YZ Planes
				{
					optix::float3 p1, p2;
					float dist_factor = 1.2f;

					// XY - plane line indicators
					{
						p1 = optix::make_float3(0.0f) + GLOBAL_AXIS[0] * dist_factor;
						p2 = p1 + GLOBAL_AXIS[1] * dist_factor;
						diffuseColor = DEF_AXIS_COLOR[0] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[1];
						glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
						drawLine3D((float*)&p1, (float*)&(p2));

						p1 = optix::make_float3(0.0f) + GLOBAL_AXIS[1] * dist_factor;
						p2 = p1 + GLOBAL_AXIS[0] * dist_factor;
						diffuseColor = DEF_AXIS_COLOR[1] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[1];
						glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
						drawLine3D((float*)&p1, (float*)&(p2));

						if (m_selected_plane == XY)
						{
							diffuseColor = HIGHLIGHT_COLOR;
							glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
							glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
							drawPlane(optix::make_float3(0.0f), GLOBAL_AXIS[0] * dist_factor, GLOBAL_AXIS[1] * dist_factor);
							glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);

						}

					}

					// XZ - plane line indicators
					{
						p1 = optix::make_float3(0.0f) + GLOBAL_AXIS[0] * dist_factor;
						p2 = p1 + GLOBAL_AXIS[2] * dist_factor;
						diffuseColor = DEF_AXIS_COLOR[0] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[0];
						glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
						drawLine3D((float*)&p1, (float*)&(p2));

						p1 = optix::make_float3(0.0f) + GLOBAL_AXIS[2] * dist_factor;
						p2 = p1 + GLOBAL_AXIS[0] * dist_factor;
						diffuseColor = DEF_AXIS_COLOR[2] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
						glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
						drawLine3D((float*)&p1, (float*)&(p2));

						if (m_selected_plane == XZ)
						{
							diffuseColor = HIGHLIGHT_COLOR;
							glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
							glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
							drawPlane(optix::make_float3(0.0f), GLOBAL_AXIS[0] * dist_factor, GLOBAL_AXIS[2] * dist_factor);
							glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
						}
					}

					// YZ - plane line indicators
					{
						p1 = optix::make_float3(0.0f) + GLOBAL_AXIS[1] * dist_factor;
						p2 = p1 + GLOBAL_AXIS[2] * dist_factor;
						diffuseColor = DEF_AXIS_COLOR[1] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[1];
						glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
						drawLine3D((float*)&p1, (float*)&(p2));


						p1 = optix::make_float3(0.0f) + GLOBAL_AXIS[2] * dist_factor;
						p2 = p1 + GLOBAL_AXIS[1] * dist_factor;
						diffuseColor = DEF_AXIS_COLOR[2] * 0.8f + glm::vec3(0.2f) * GLM_GLOBAL_INV_AXIS[2];
						glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
						drawLine3D((float*)&p1, (float*)&(p2));

						if (m_selected_plane == YZ)
						{
							diffuseColor = HIGHLIGHT_COLOR;
							glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
							glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
							drawPlane(optix::make_float3(0.0f), GLOBAL_AXIS[1] * dist_factor, GLOBAL_AXIS[2] * dist_factor);
							glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
						}
					}
				}

				//glUniform1i(m_utility_rendering_program["uniform_use_model_matrix"], 0);
			}
			if (m_active_utility == GuiState::Utility::ROTATION)
			{

				glDisable(GL_CULL_FACE);
				glm::vec3 pos = glm::vec3(m_utility_translation_attributes.x, m_utility_translation_attributes.y, m_utility_translation_attributes.z);
				glm::vec3 scale = glm::vec3(m_camera_inv_scale_factor * 0.6);
				optix::float3 rot_dt = Mediator::RequestSelectedObject()->getAttributesDelta(1);

				// outer stable circles
				{
					int a = 34;
					int b = 35;
					int PI = 1;
					if (m_focused_utility >= 0)
					{
						a = 25;
						b = 35;
						PI = 4;
					}

					glm::mat4 outer_matrix_1 = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), scale * 1.3f);
					glm::mat4 outer_matrix_2 = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), scale * 1.45f);
					glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 1);

					diffuseColor = glm::vec3(0.65f);
					glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(outer_matrix_1));
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					drawCircle(0.0, 0.0, 0.0, 1.0f, Utilities::getRand(a, b, PI), 1);


					diffuseColor = glm::vec3(0.5f);
					glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(outer_matrix_2));
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
					drawCircle(0.0, 0.0, 0.0, 1.0f, Utilities::getRand(a - 5, b, PI), 1);

					glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);

				}

				//model_matrix = m_object_transformation_matrix_rotated_scaled;
				model_matrix = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), scale * 1.3f);
				glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

				// draw Rotation Arcs
				for (uint arc_i = 0; arc_i < 3; arc_i++)
				{
					float rot_dti = ((float*)&rot_dt)[arc_i];

					if (rot_dti != 0)
					{

						glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
						glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);
						glUniform1i(m_utility_rendering_program["uniform_selected_circle"], arc_i);

						diffuseColor = DEF_AXIS_COLOR[arc_i];
						glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);

						optix::float3 c = ZERO_3f;
						optix::float3 p0 = m_arc_isec_point.p;
						optix::float3 plane_n = M_WORKING_AXIS[arc_i];
						optix::float3 u = m_arc_isec_point.normal; //normalize(p0 - c);
						optix::float3 v = -cross(u, plane_n);
						draw_Arc_arbitr_plane(c, 1.0f, 0.0f, rot_dti, u, v, 50, 1, GL_POLYGON);

						// draw Tangent movement - arrows
						drawTangent(m_arc_isec_point, rot_dti);

					}

					optix::float3 u, v;
					if (arc_i == 0)
					{
						u = M_WORKING_AXIS[1];
						v = M_WORKING_AXIS[2];
					}
					else if (arc_i == 1)
					{
						u = M_WORKING_AXIS[0];
						v = M_WORKING_AXIS[2];
					}
					else
					{
						u = M_WORKING_AXIS[0];
						v = M_WORKING_AXIS[1];
					}

					glUniform1i(m_utility_rendering_program["uniform_is_arc"], 1);
					glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
					glUniform1i(m_utility_rendering_program["uniform_circle"], arc_i);
					diffuseColor = AXIS_COLOR[arc_i];
					glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
					draw_Arc_arbitr_plane(ZERO_3f, 1.0f, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, CIRCLE_SEGMENTS, 0, GL_LINE_LOOP);

				}

				glUniform1i(m_utility_rendering_program["uniform_is_arc"], 0);
				glEnable(GL_CULL_FACE);


				// draw XYZ - axis inside roation sphere
				{
					glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(m_object_transformation_matrix_scaled));
					glUniform1i(m_utility_rendering_program["uniform_use_parent_matrix_scaled"], 1);

					optix::float3 p1 = ZERO_3f;
					optix::float3 p2, p3;
					float size = 0.5f;
					glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.7f);

					for (int i = 0; i < 3; i++)
					{

						p2 = M_WORKING_AXIS[i] * size;
						if (AXIS_COLOR[i] != HIGHLIGHT_COLOR)
							diffuseColor = glm::vec3(0.5f);
						else
							diffuseColor = DEF_AXIS_COLOR[i];

						glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.x, diffuseColor.y, diffuseColor.z);
						glBegin(GL_LINE_LOOP);
						glVertex3f(p1.x, p1.y, p1.z);
						glVertex3f(p2.x, p2.y, p2.z);
						glEnd();

						p3 = p2 + M_WORKING_AXIS[i] * 0.07f;
						glUniform1i(m_utility_rendering_program["uniform_text_render"], 1);
						RenderText(glm::vec3(p3.x, p3.y, p3.z), glm::vec3(1), FONT5, GLOBAL_AXIS_NAME[i]);
						glUniform1i(m_utility_rendering_program["uniform_text_render"], 0);

					}

				}


			}
			if (m_active_utility == GuiState::Utility::SCALE)
			{

			}

		}

		
		
		
		

		glBindVertexArray(0);
		m_utility_rendering_program.Unbind();
		if (m_rendering_mode != RENDERING_MODE::TRIANGLES)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDisable(GL_DEPTH_TEST);
		glPointSize(1.0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// just for debugging reasons
		RenderText(glm::vec3(0.5) , glm::vec3(1.0f), FONT2, "");
	}

	GLenum error = Tools::CheckGLError();
	if (error != GL_NO_ERROR)
	{
		printf("Renderer:Draw GL Error\n");
		system("pause");
	}

	m_render_utility = 0;
}

void OpenGLRenderer::Render_Highlighted_objects()
{
	
	int focused_obj_index  = Mediator::RequestFocusedObject();
	int selected_obj_index = Mediator::RequestSelectedObjectIndex();

	//SceneObject * focused_obj;
	//SceneObject * selected_obj;

	if (focused_obj_index >= 0 && focused_obj_index != selected_obj_index)
	{
		createGeometryObject_OpenGL_FromSceneObject(focused_obj_index);
		draw_Object_Outline(focused_obj_index, false);
	}

	if (selected_obj_index >= 0)
	{
		createGeometryObject_OpenGL_FromSceneObject(selected_obj_index);
		draw_Object_Outline(selected_obj_index, true);
	}

}


//
void OpenGLRenderer::createGeometryObject_OpenGL_FromSceneObject(int scene_object_index)
{
	SceneObject * object = nullptr;

	// key already exists -> no need to reload data
	if (m_geometry_stored_objects.find(scene_object_index) != m_geometry_stored_objects.end())
		return;
	else
		object = Mediator::RequestSceneObjects()[scene_object_index];
	

	GLuint m_vao, m_vbo, m_ibo;
	unsigned int num_vertices, num_indices;

	optix::GeometryInstance g_inst = object->getGeometryInstance();
	
	// number of vertices
	int num_triangles = g_inst->getGeometryTriangles()->getPrimitiveCount();
	num_vertices = num_triangles * 3;

	// vbo positions buffer data
	RTsize vbo_width , vbo_height;
	g_inst["vertex_buffer"]->getBuffer()->getSize(vbo_width, vbo_height);
	const void * vbo_pos_data = g_inst["vertex_buffer"]->getBuffer()->map();
	g_inst["vertex_buffer"]->getBuffer()->unmap();

	// vbo indices buffer data
	RTsize vbo_ind_width, vbo_ind_height;
	g_inst["index_buffer"]->getBuffer()->getSize(vbo_ind_width, vbo_ind_height);
	const void * vbo_ind_data = g_inst["index_buffer"]->getBuffer()->map();
	g_inst["index_buffer"]->getBuffer()->unmap();


	// normals buffer data
	RTsize normal_buffer_width, normal_buffer_height;
	g_inst["normal_buffer"]->getBuffer()->getSize(normal_buffer_width, normal_buffer_height);
	const void * vbo_normal_data = g_inst["normal_buffer"]->getBuffer()->map();
	g_inst["normal_buffer"]->getBuffer()->unmap();
	
	
	// init geometryNode
	optix::float3 * vbo_buffer  = (optix::float3 *)vbo_pos_data;
	optix::int3   * vbo_indices = (optix::int3 *)vbo_ind_data;
	optix::float3 * vbo_normals = (optix::float3 *)vbo_normal_data;


	std::vector<glm::vec3> vbo_buf;
	for (int i = 0; i < vbo_width * vbo_height; i++)
		vbo_buf.push_back(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(vbo_buffer[i]));



	int ind_size = vbo_ind_width * vbo_ind_height;
	num_indices = ind_size * 3;
	std::vector<unsigned int> indices;
	
	for (int i = 0; i < ind_size; i++)
	{
		indices.push_back((unsigned int)vbo_indices[i].x);
		indices.push_back((unsigned int)vbo_indices[i].y);
		indices.push_back((unsigned int)vbo_indices[i].z);
	}

	
	// creating m_vao
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, vbo_buf.size() * sizeof(glm::vec3), &vbo_buf[0], GL_STATIC_DRAW);
	

	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);

	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_geometry_stored_objects[scene_object_index] = new GeometryObject_OpenGL(m_vao, m_vbo, m_ibo, num_vertices, num_indices);
	
}

void OpenGLRenderer::draw_Object_Outline(int scene_object_index, bool selected )
{
	//std::cout << " draw_Object_Outline(" << scene_object_index << " , " << selected << " ) " << std::endl;
	
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);

//#define BBOX_TEST
#ifdef BBOX_TEST
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], 1.0f, 1.0f, 1.0f);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr( MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(m_selected_object->getTransformationMatrix())));
	optix::float3 bmin = m_selected_object->getGeomProperties().bbox_min;
	optix::float3 bmax = m_selected_object->getGeomProperties().bbox_max;
	drawBBox(bmin, bmax, ZERO_3f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	return;
#endif

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);

	SceneObject * obj = Mediator::RequestSceneObjects()[scene_object_index];
	optix::float3 obj_pos = obj->getTranslationAttributes();
	glm::vec3 obj_cam_offset = glm::vec3(0.0f);

#ifdef OBJECT_OUTLINE_CAM_OFFSET
	optix::float3 cam_center = Mediator::RequestCameraInstance().getCenter();
	obj_cam_offset = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj_pos - cam_center);
	float obj_dist_center = length(obj_cam_offset);
	obj_cam_offset /= obj_dist_center;
	float F_squared = 179.0f * 179.0f;
	float a = 1.3f;
	float b = 0.7f;
	float l = a * obj_dist_center + (179.0f - obj_dist_center) * b;
	l *= obj_dist_center;
	l /= F_squared; 
	obj_cam_offset *= l;

#endif



	// 1st render pass //
	glm::mat4 model = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getTranslationAttributes())	 - (obj_cam_offset)) 
		* MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getRotationMatrix());

	glm::mat4 model_1 = model * glm::scale(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getScaleAttributes())); //MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getScaleMatrix());
	glm::vec3 diffuse = glm::vec3(1.0f);
	if (selected)
		diffuse = glm::vec3(0, 255, 195) / 255.0f;
	else
		diffuse = glm::vec3(255, 0, 251) / 255.0f;

	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_1));
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuse.r, diffuse.g, diffuse.b);

	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 1);
	glBindVertexArray(m_geometry_stored_objects[scene_object_index]->m_vao);
	glDrawElements(
		GL_TRIANGLES,
		m_geometry_stored_objects[scene_object_index]->num_indices,
		GL_UNSIGNED_INT,
		0
	);
	glUniform1i(m_utility_rendering_program["outline_1st_pass"], 0);
	glBindVertexArray(0);


	// 2nd render pass //
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00); // disable writing to the stencil buffer
	glDisable(GL_DEPTH_TEST);


	// object's geometric properties
	optix::float3 min = obj->getGeomProperties().bbox_min;
	optix::float3 max = obj->getGeomProperties().bbox_max;
	float distance_factor = obj->getDistanceFromCamera() * 0.07f;
	glm::vec3 scale_old = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getScaleAttributes());

	// calculate new scale
	float outline_thickness = 0.034f; // 0.04f;
	optix::float3 div_factor = max;
	if (optix::length(min) > optix::length(max) || true)
	{
		glm::vec3 t = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(min);
		t = glm::abs(t);
		div_factor = VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(t) + max;
	}
	

	
	glm::vec3 scale_factor = glm::vec3(outline_thickness) * distance_factor  / glm::abs(VECTOR_TYPE_TRANSFORM::TO_GLM_3f(div_factor));
	glm::vec3 scale_new = scale_old + scale_factor;

	// calculate object's old and new center
	optix::float3 c     = (max + min) * obj->getScaleAttributes() / 2.0f;
	optix::float3 c_new = (max + min) * VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(scale_new) / 2.0f;
	glm::vec3 Dc = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(c_new - c); // centers' distance offset in case object is not locally centered

	//
	optix::float3 min_old = min * VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(scale_old);
	optix::float3 max_old = max * VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(scale_old);
	optix::float3 min_new = min * VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(scale_new);
	optix::float3 max_new = max * VECTOR_TYPE_TRANSFORM::TO_OPTIX_3f(scale_new); 

	glm::mat4 model_2 = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(obj->getTranslationAttributes())- (obj_cam_offset) - Dc )
		* MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(obj->getRotationMatrix()) * glm::scale(glm::mat4(1.0f), scale_new);

	
	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_2));
	
	glBindVertexArray(m_geometry_stored_objects[scene_object_index]->m_vao);
	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 1);
	glDrawElements(
		GL_TRIANGLES,
		m_geometry_stored_objects[scene_object_index]->num_indices,
		GL_UNSIGNED_INT,
		0
	);

	glUniform1i(m_utility_rendering_program["outline_2nd_pass"], 0);
	glBindVertexArray(0);
	glStencilMask(0xFF);
	glDisable(GL_STENCIL_TEST);
	//

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

}

int OpenGLRenderer::getFocusedUtility()
{
	return m_focused_utility;
}

float OpenGLRenderer::getInvCameraScaleFactor()
{
	return m_camera_inv_scale_factor;
}

IntersectionPoint OpenGLRenderer::getIsecPoint()
{
	return m_arc_isec_point;
}

optix::float3 * OpenGLRenderer::getWorkingAxis()
{
	return &M_WORKING_AXIS[0];
}

int OpenGLRenderer::testPlaneIntersection(Ray ray, Plane plane)
{
	
	return 0;
}


#define DRAW_FUNCTIONS
#ifdef DRAW_FUNCTIONS

void OpenGLRenderer::drawCameraCenterIndicator(glm::vec3 p, float size, glm::vec3 color)
{
	
	glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.f);
	glUniform1i(m_utility_rendering_program["uniform_stable_circle"], 0);

	//glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glPointSize(size);
	glm::mat4 model_matrix = glm::mat4(1.0f);
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], color.r, color.g, color.b);
	glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	
	glBegin(GL_POINTS);
	glVertex3f(p.x, p.y, p.z);
	glEnd();

	glPointSize(1.0f);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

void OpenGLRenderer::draw_Arc_arbitr_plane(optix::float3 c, float r, float s_angle, float e_angle, optix::float3 &u, optix::float3& v, int Nsegments, int is_arc, GLenum draw_type)
{
	//s_angle = Geometry::GENERAL::toRadians(s_angle);
	//e_angle = Geometry::GENERAL::toRadians(e_angle);

	float theta = s_angle;
	float theta_dt = (e_angle - s_angle) / ((float)Nsegments - 1.0f);
	
	glBegin(draw_type);
	if (is_arc)
		glVertex3f(c.x, c.y, c.z);
	for (int iter = 0; iter < Nsegments; iter++)
	{
		optix::float3 pi = (r*cosf(theta))*u + (r*sinf(theta))*v + c;
		glVertex3f(pi.x, pi.y, pi.z);

		theta += theta_dt;
	}
	if(is_arc)
		glVertex3f(c.x, c.y, c.z);
	glEnd();
}

void OpenGLRenderer::drawTriangle(Triangle tri)
{
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLES);
	glVertex3f(tri.p0.x, tri.p0.y, tri.p0.z);
	glVertex3f(tri.p1.x, tri.p1.y, tri.p1.z);
	glVertex3f(tri.p2.x, tri.p2.y, tri.p2.z);
	glEnd();
	glEnable(GL_CULL_FACE);
}

void OpenGLRenderer::drawTriangle(optix::float3 p0, optix::float3 u, optix::float3 v)
{
	optix::float3 p1 = p0 + u;
	optix::float3 p2 = p0 + v;
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLES);
	glVertex3f(p0.x, p0.y, p0.z);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glEnd();
	glEnable(GL_CULL_FACE);
}

void OpenGLRenderer::drawBBox(optix::float3 min, optix::float3 max, optix::float3 pos)
{
	optix::float3 v_x, v_y, v_z;
	optix::float3 v0, v1, v2, v3;

	// right face - left face
	{
		// right face
		v0 = optix::make_float3(min.x, min.y, min.z);
		v1 = optix::make_float3(min.x, max.y, min.z);
		v2 = optix::make_float3(min.x, max.y, max.z);
		v3 = optix::make_float3(min.x, min.y, max.z);
		glBegin(GL_LINE_LOOP);
		glVertex3f(v0.x, v0.y, v0.z);
		glVertex3f(v1.x, v1.y, v1.z);
		glVertex3f(v2.x, v2.y, v2.z);
		glVertex3f(v3.x, v3.y, v3.z);
		glVertex3f(v0.x, v0.y, v0.z);
		glEnd();

		// left face
		v0 = optix::make_float3(max.x, min.y, min.z);
		v1 = optix::make_float3(max.x, max.y, min.z);
		v2 = optix::make_float3(max.x, max.y, max.z);
		v3 = optix::make_float3(max.x, min.y, max.z);
		glBegin(GL_LINE_LOOP);
		glVertex3f(v0.x, v0.y, v0.z);
		glVertex3f(v1.x, v1.y, v1.z);
		glVertex3f(v2.x, v2.y, v2.z);
		glVertex3f(v3.x, v3.y, v3.z);
		glVertex3f(v0.x, v0.y, v0.z);
		glEnd();
	}

	// front - back face
	{
		// front face
		v0 = optix::make_float3(min.x, min.y, min.z);
		v1 = optix::make_float3(max.x, min.y, min.z);
		v2 = optix::make_float3(min.x, max.y, min.z);
		v3 = optix::make_float3(max.x, max.y, min.z);
		glBegin(GL_LINE_LOOP);
		glVertex3f(v0.x, v0.y, v0.z);
		glVertex3f(v1.x, v1.y, v1.z);
		glEnd();
		glBegin(GL_LINE_LOOP);
		glVertex3f(v2.x, v2.y, v2.z);
		glVertex3f(v3.x, v3.y, v3.z);
		glEnd();

		// back face
		v0 = optix::make_float3(min.x, min.y, max.z);
		v1 = optix::make_float3(max.x, min.y, max.z);
		v2 = optix::make_float3(min.x, max.y, max.z);
		v3 = optix::make_float3(max.x, max.y, max.z);
		glBegin(GL_LINE_LOOP);
		glVertex3f(v0.x, v0.y, v0.z);
		glVertex3f(v1.x, v1.y, v1.z);
		glEnd();
		glBegin(GL_LINE_LOOP);
		glVertex3f(v2.x, v2.y, v2.z);
		glVertex3f(v3.x, v3.y, v3.z);
		glEnd();
	}


}

void OpenGLRenderer::drawTangent(IntersectionPoint p, float rot_d)
{
	float arrow_size = 0.1f;
	float tangent_size = 0.70f;

	//glm::mat4 model_matrix = m_object_transformation_matrix * glm::scale(glm::mat4(1.0f), glm::vec3(1.3f));
	glm::vec3 diffuseColor = HIGHLIGHT_COLOR;
	glUniform3f(m_utility_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
	//glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

	optix::float3 p0 = p.normal;
	optix::float3 p0_ = p.normal * 2.0f;
	optix::float3 p1 = p0 - optix::normalize(p.tangent) * tangent_size; // tangent down
	optix::float3 p2 = p0 + optix::normalize(p.tangent) * tangent_size; // tangent up

	optix::float3 l_ar, r_ar, ar1, ar2;
	// draw up tangent
	{
		if (rot_d > 0)
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		else
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.15f);

		glBegin(GL_LINE_LOOP);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glEnd();

		// draw up arrow
		l_ar = optix::normalize(p2 - 0);
		r_ar = optix::normalize(p2 - p0_);
		ar1 = p2 - l_ar * arrow_size;
		ar2 = p2 - r_ar * arrow_size;

		glBegin(GL_LINE_LOOP);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(ar1.x, ar1.y, ar1.z);
		glEnd();
		glBegin(GL_LINE_LOOP);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(ar2.x, ar2.y, ar2.z);
		glEnd();
	}

	// draw down tangent
	{
		if (rot_d < 0)
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 1.0f);
		else
			glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.15f);
		glBegin(GL_LINE_LOOP);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p1.x, p1.y, p1.z);
		glEnd();

		// draw down arrow
		l_ar = optix::normalize(p1 - 0);
		r_ar = optix::normalize(p1 - p0_);
		ar1 = p1 - l_ar * arrow_size;
		ar2 = p1 - r_ar * arrow_size;

		glBegin(GL_LINE_LOOP);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(ar1.x, ar1.y, ar1.z);
		glEnd();
		glBegin(GL_LINE_LOOP);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(ar2.x, ar2.y, ar2.z);
		glEnd();
	}

	glUniform1f(m_utility_rendering_program["uniform_alpha"], 0.3f);

}

void OpenGLRenderer::drawArcSegment(IntersectionPoint p, float r, float start_angle, float d_angle, int segments)
{

	float d_theta = fabs(d_angle - start_angle) / (float)segments;
	float sign = (d_angle - start_angle) > 0 ? 1 : -1;
	if (p.plane_id == Y)
		sign *= -1;

	float theta = start_angle;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	float _x = x;
	float _y = y;
	float _z = z;

	glBegin(GL_POLYGON);
	for (int i = 0; i <= segments + 1; i++)
	{
		glVertex3f(_x, _y, _z);

		if (p.plane_id == X)
		{
			_y = y + r * cosf(theta);
			_z = z + r * sinf(theta);
		}
		else if (p.plane_id == Y)
		{
			_x = x + r * cosf(theta);
			_z = z + r * sinf(theta);
		}
		else if (p.plane_id == Z)
		{
			_x = y + r * cosf(theta);
			_y = y + r * sinf(theta);
		}

		theta += d_theta * sign;
	}
	glVertex3f(x, y, z);
	glEnd();

}

void OpenGLRenderer::drawPlane(optix::float3 p, optix::float3 u, optix::float3 v)
{
	optix::float3 p1 = p + u;
	optix::float3 p2 = p + v;
	optix::float3 p3 = p1 + p2;

	glDisable(GL_CULL_FACE);
	glBegin(GL_POLYGON);
	glVertex3f(p.x, p.y, p.z);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glEnd();
	glEnable(GL_CULL_FACE);

}

void OpenGLRenderer::drawGrid(optix::float3 o)
{

	for (int i = 0; i < 15; i++)
	{
		// do some stuff
	}

}

void OpenGLRenderer::draw(float * p)
{

	optix::float3 pp = optix::make_float3(p[0], p[1], p[2]);

	float theta = M_PIf / 4.0f;
	float x = 0.0f;
	float y = 0.0f;
	float r = 0.0f;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 1000; i++)
	{
		//r = cosf(theta) / sinf(theta);

		r = glm::length(glm::vec3(x, y, pp.z));

		x = r * cosf(theta);
		y = r * sinf(theta);

		x = x + pp.x;
		y = y + pp.y;




		glVertex3f(x, y, pp.z);

		//theta += 0.1f;
	}
	glEnd();
}

void OpenGLRenderer::drawLine3D(float * p1, float * p2)
{
	glBegin(GL_LINE_LOOP);
	glVertex3f(p1[0], p1[1], p1[2]);
	glVertex3f(p2[0], p2[1], p2[2]);
	glEnd();
}

void OpenGLRenderer::drawLine2D(float * p1, float * p2)
{
	glBegin(GL_LINE_LOOP);
	glVertex2f(p1[0], p1[1]);
	glVertex2f(p2[0], p2[1]);
	glEnd();
}

void OpenGLRenderer::drawCircle(float _x, float _y, float _z, float _r, int segments, int vertical)
{

	float theta = 2 * M_PIf / float(segments);
	float tangetial_factor = tanf(theta);
	float radial_factor = 1 - cosf(theta);

	float x = _x + _r;
	float y = _y;

	glBegin(GL_LINE_LOOP);
	for (int ii = 0; ii < segments; ii++)
	{

		if (vertical)
			glVertex3f(x, y, _z);
		else
			glVertex3f(x, _z, y);

		//glVertex3f(x  , y  , depth);

		float tx = -(y - _y);
		float ty = x - _x;

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		float rx = _x - x;
		float ry = _y - y;

		x += rx * radial_factor;
		y += ry * radial_factor;
	}
	glEnd();

}

void OpenGLRenderer::drawCircleAligned(optix::float3 c, float r, UtilityPlane plane, int num_segments)
{
	float d_theta = 2 * M_PIf / float(num_segments);
	float theta = 0.0f;

	float x = c.x;
	float y = c.y;
	float z = c.z;

	float _x;
	float _y;
	float _z;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < num_segments; i++)
	{
		_x = r * cosf(theta);
		_y = r * sinf(theta);

		if (plane == UtilityPlane::X)
		{
			glVertex3f(x, _x, _y);
		}
		else if (plane == UtilityPlane::Y)
		{
			glVertex3f(_x, y, _y);
		}
		else if (plane == UtilityPlane::Z)
		{
			glVertex3f(_x, _y, z);
		}

		theta += d_theta;
	}
	glEnd();
}

void OpenGLRenderer::drawCircle2D(float cx, float cy, float r, int num_segments)
{

	glBegin(GL_LINE_LOOP);
	for (int ii = 0; ii < num_segments; ii++)
	{
		float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle

		float x = r * cosf(theta);//calculate the x component
		float y = r * sinf(theta);//calculate the y component

		glVertex2f(x + cx, y + cy);//output vertex

	}
	glEnd();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);


}

void OpenGLRenderer::drawArc(float _x, float _y, float _z, float _r, float start_angle, float d_angle, int segments, int vertical)
{



	int real_segments = int(fabsf(d_angle) / (2 * M_PIf) * (float)segments) + 1;

	float theta = d_angle / float(real_segments);
	float tangetial_factor = tanf(theta);
	float radial_factor = 1 - cosf(theta);

	float x = _x + _r * cosf(start_angle);
	float y = _y + _r * sinf(start_angle);

	glBegin(GL_POLYGON);
	glVertex3f(_x, _z, _y);
	for (int i = 0; i < real_segments + 1; i++)
	{
		glVertex3f(x, y, _z);

		float tx = -(y - _y);
		float ty = x - _x;

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		float rx = _x - x;
		float ry = _y - y;

		x += rx * radial_factor;
		y += ry * radial_factor;
	}
	glVertex3f(_x, _y, _z);
	glEnd();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void OpenGLRenderer::drawArc2(float _x, float _y, float _r, float depth, float start_angle, float d_angle, int segments)
{


	int real_segments = int(fabsf(d_angle) / (2 * M_PIf) * (float)segments) + 1;

	float theta = d_angle / float(real_segments);
	float tangetial_factor = tanf(theta);
	float radial_factor = 1 - cosf(theta);

	float x = _x + _r * cosf(start_angle);
	float y = _y + _r * sinf(start_angle);

	glBegin(GL_POLYGON);
	glVertex3f(_x, depth, _y);
	for (int ii = 0; ii < real_segments + 1; ii++)
	{

		glVertex3f(x, depth, y);

		float tx = -(y - _y);
		float ty = x - _x;

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		float rx = _x - x;
		float ry = _y - y;

		x += rx * radial_factor;
		y += ry * radial_factor;
	}
	glVertex3f(_x, depth, _y);
	glEnd();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void OpenGLRenderer::RenderText(glm::vec3 p, glm::vec3 color, void * font, char *msg)
{

	glColor3f(color.r, color.g, color.b);
	glRasterPos3f(p.x, p.y, p.z);
	int len, i;
	len = (int)strlen(msg);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(font, msg[i]);
	}
}

#endif
