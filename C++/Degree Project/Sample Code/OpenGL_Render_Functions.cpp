#include "OpenGL_Render_Functions.h"
#include "OpenGL_Render_Functions.h"
#include "Utility_Renderer_Defines.h"
#include <GL/glut.h>
#include "ShaderProgram.h"
#include "AssetManager.h"
#include "GeometryFunctions.h"
#include "Transformations.h"
#include "Mediator.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ViewportManager.h"

void draw::Point(optix::float3 p, float size)
{
	glPointSize(size);

	glBegin(GL_POINTS);
	glVertex3f(p.x, p.y, p.z);
	glEnd();

	glPointSize(1.0f);
}
void draw::Points(optix::float3 * p, int arr_size, int offset, float size , GLenum type)
{
	glPointSize(size);

	glBegin( type );
	for (int i = offset; i < arr_size; i++)
		glVertex3f(p[i].x, p[i].y, p[i].z);
	glEnd();

	glPointSize(1.0f);
}

void draw::Triangle_(optix::float3 p0, optix::float3 u, optix::float3 v)
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
void draw::Triangle_(Triangle tri)
{
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLES);
	glVertex3f(tri.p0.x, tri.p0.y, tri.p0.z);
	glVertex3f(tri.p1.x, tri.p1.y, tri.p1.z);
	glVertex3f(tri.p2.x, tri.p2.y, tri.p2.z);
	glEnd();
	glEnable(GL_CULL_FACE);

}
void draw::Triangle_Outline(Triangle tri)
{
	glBegin(GL_LINE_LOOP);
	glVertex3f(tri.p0.x, tri.p0.y, tri.p0.z);
	glVertex3f(tri.p1.x, tri.p1.y, tri.p1.z);
	glVertex3f(tri.p2.x, tri.p2.y, tri.p2.z);
	glVertex3f(tri.p0.x, tri.p0.y, tri.p0.z);
	glEnd();

}
void draw::BBox(optix::float3 min, optix::float3 max, optix::float3 pos, float line_length)
{
	glLineWidth(line_length);

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

	glLineWidth(1.0f);
}
void draw::BBox_dashed(optix::float3 min, optix::float3 max, float dist_ratio, float line_length )
{
	glLineWidth(line_length);

	float xm, ym, zm, xM, yM, zM;
	xm = min.x;
	ym = min.y;
	zm = min.z;

	xM = max.x;
	yM = max.y;
	zM = max.z;
	
	optix::float3 p[8];
	p[0] = optix::make_float3(xm, ym, zm);
	p[1] = optix::make_float3(xM, ym, zm);
	p[2] = optix::make_float3(xM, yM, zm);
	p[3] = optix::make_float3(xm, yM, zm);

	p[4] = optix::make_float3(xM, yM, zM);
	p[5] = optix::make_float3(xM, ym, zM);
	p[6] = optix::make_float3(xm, ym, zM);
	p[7] = optix::make_float3(xm, yM, zM);


	float l;
	optix::float3 d, _d;
	int i, j;
	// front face
	
	// p0 - p1
	{
		i = 0;
		j = 1;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}

	// p0 - p3
	{
		i = 0;
		j = 3;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}

	// p0 - p6
	{
		i = 0;
		j = 6;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);
		
		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}


	// p1 - p5
	{
		i = 1;
		j = 5;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}


	// p1 - p2
	{
		i = 1;
		j = 2;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}

	// p2 - p4
	{
		i = 2;
		j = 4;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}


	// p3 - p2
	{
		i = 3;
		j = 2;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}


	// p3 - p7
	{
		i = 3;
		j = 7;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}


	// p7 - p6
	{
		i = 7;
		j = 6;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}

	// p7 - p4
	{
		i = 7;
		j = 4;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}


	// p6 - p5
	{
		i = 6;
		j = 5;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}


	// p5 - p4
	{
		i = 5;
		j = 4;
		d = p[j] - p[i];
		l = optix::length(d) * dist_ratio;
		_d = normalize(d);

		draw::Line3D(p[i], p[i] + l * _d, line_length);
		draw::Line3D(p[j], p[j] - l * _d, line_length);
	}


	glLineWidth(1.0f);
}
void draw::BBox_Planes(optix::float3 min, optix::float3 max, optix::float3 pos, float offset,  bool filled , float line_length )
{
	GLenum type = filled ? GL_POLYGON : GL_LINE_LOOP;

	
	glLineWidth(line_length);
	optix::float3 v0, v1, v2, v3;
	optix::float3 u;

	float x = min.x;
	float y = min.y;
	float z = min.z;
	float X = max.x;
	float Y = max.y;
	float Z = max.z;

	// front
	glBegin(type);
	{
		glVertex3f(x, y, z);
		glVertex3f(x, Y, z);
		glVertex3f(X, Y, z);
		glVertex3f(X, y, z);
	}
	glEnd();

	// back
	glBegin(type);
	{
		glVertex3f(x, y, Z);
		glVertex3f(x, Y, Z);
		glVertex3f(X, Y, Z);
		glVertex3f(X, y, Z);
	}
	glEnd();


	// left
	glBegin(type);
	{
		glVertex3f(x, y, z);
		glVertex3f(x, Y, z);
		glVertex3f(x, Y, Z);
		glVertex3f(x, y, Z);
	}
	glEnd();


	// right
	glBegin(type);
	{
		glVertex3f(X, y, z);
		glVertex3f(X, Y, z);
		glVertex3f(X, Y, Z);
		glVertex3f(X, y, Z);
	}
	glEnd();


	// bot
	glBegin(type);
	{
		glVertex3f(x, y, z);
		glVertex3f(x, y, Z);
		glVertex3f(X, y, Z);
		glVertex3f(X, y, z);
	}
	glEnd();

	// top
	glBegin(type);
	{
		glVertex3f(x, Y, z);
		glVertex3f(x, Y, Z);
		glVertex3f(X, Y, Z);
		glVertex3f(X, Y, z);
	}
	glEnd();



	glLineWidth(1.0f);
}

void draw::Tangent(IntersectionPoint p, float rot_d)
{
	
	return draw::Tangent2(p, rot_d);

	float arrow_size = 0.12f;
	float tangent_size = 0.5f;

	ShaderProgram * m_program = Mediator::RequestShaderProgram();
	//glm::mat4 model_matrix = m_object_transformation_matrix * glm::scale(glm::mat4(1.0f), glm::vec3(1.3f));
	
	glm::vec3 diffuseColor = glm::vec3(1.0f, 1.0f, 0.0f); //HIGHLIGHT_COLOR;
	glm::vec3 def_color    = glm::vec3(1.0f, 1.0f, 0.2f) * 0.5f;
	glUniform3f( (*m_program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
	

	//glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	optix::float3 p0  = p.normal;
	optix::float3 p0_ = p.normal * 2.0f;
	optix::float3 p1  = p0 - optix::normalize(p.tangent) * tangent_size; // tangent down
	optix::float3 p2  = p0 + optix::normalize(p.tangent) * tangent_size; // tangent up

	
	
	optix::float3 from_p_to_cam = normalize( p0_ - Mediator::RequestCameraInstance().getCameraPos() );
	float dist_p1 = optix::length(p1 - Mediator::RequestCameraInstance().getCameraPos());
	float dist_p2 = optix::length(p2 - Mediator::RequestCameraInstance().getCameraPos());
	if (dist_p1 > dist_p2)
	{
		struct Plane plane_ = Geometry::SHAPES::createPlane(p2, -from_p_to_cam); //-optix::normalize(Mediator::RequestCameraInstance().getW()));
		p1 = Geometry::VECTOR_SPACE::projectPointToPlane(plane_, p1);
	}
	else
	{
		struct Plane plane_ = Geometry::SHAPES::createPlane(p1, -from_p_to_cam);//-optix::normalize(Mediator::RequestCameraInstance().getW()));
		p2 = Geometry::VECTOR_SPACE::projectPointToPlane(plane_, p2);
	}
	

	optix::float3 axis = optix::normalize( p2 - p1 );
	optix::float3 tang = optix::normalize(optix::cross(p2 - p1, from_p_to_cam)); //Mediator::RequestCameraInstance().getW()) );

	optix::float3 l_ar, r_ar, ar1, ar2;
	// draw up tangent
	{
		if (rot_d > 0)
			glUniform3f((*m_program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		else
			glUniform3f((*m_program)["uniform_diffuse"], def_color.r, def_color.g, def_color.b);

		glBegin(GL_LINE_LOOP);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glEnd();

		// draw up arrow
		l_ar = optix::normalize(p2 - p0_);
		r_ar = optix::normalize(p2 - p0_);
		ar1 = p2 - tang * arrow_size/2 - axis * arrow_size;
		ar2 = p2 + tang * arrow_size/2 - axis * arrow_size;

		
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
		

		if (rot_d > 0)
			glUniform3f((*m_program)["uniform_diffuse"], def_color.r, def_color.g, def_color.b);
		else
			glUniform3f((*m_program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);

		glBegin(GL_LINE_LOOP);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p1.x, p1.y, p1.z);
		glEnd();

		// draw down arrow
		l_ar = optix::normalize(p1 - 0);
		r_ar = optix::normalize(p1 - p0_);
		ar1 = p1 - tang * arrow_size/2 + axis * arrow_size;
		ar2 = p1 + tang * arrow_size/2 + axis * arrow_size;

		glBegin(GL_LINE_LOOP);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(ar1.x, ar1.y, ar1.z);
		glEnd();
		glBegin(GL_LINE_LOOP);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(ar2.x, ar2.y, ar2.z);
		glEnd();
	}

	glUniform1f((*m_program)["uniform_alpha"], 0.3f);
}
void draw::Tangent2(IntersectionPoint p, float rot_d)
{

	float arrow_size   = 0.12f;
	float tangent_size = 0.5f;

	ShaderProgram * m_program = Mediator::RequestShaderProgram();
	//glm::mat4 model_matrix = m_object_transformation_matrix * glm::scale(glm::mat4(1.0f), glm::vec3(1.3f));

	glm::vec3 diffuseColor = glm::vec3(1.0f, 1.0f, 0.0f); //HIGHLIGHT_COLOR;
	glm::vec3 def_color    = glm::vec3(1.0f, 1.0f, 0.2f) * 0.5f;
	glUniform3f((*m_program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);


	//glUniformMatrix4fv(m_utility_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	optix::float3 p0  = p.normal;
	optix::float3 p0_ = p.normal * 2.0f;
	optix::float3 p1 = p0 - optix::normalize(p.tangent) * tangent_size; // tangent down
	optix::float3 p2 = p0 + optix::normalize(p.tangent) * tangent_size; // tangent up



	optix::float3 from_p_to_cam = normalize(p0_ - Mediator::RequestCameraInstance().getCameraPos());
	float dist_p1 = optix::length(p1 - Mediator::RequestCameraInstance().getCameraPos());
	float dist_p2 = optix::length(p2 - Mediator::RequestCameraInstance().getCameraPos());
	if (dist_p1 > dist_p2)
	{
		//struct Plane plane_ = Geometry::SHAPES::createPlane(p2, -from_p_to_cam); //-optix::normalize(Mediator::RequestCameraInstance().getW()));
		//p1 = Geometry::VECTOR_SPACE::projectPointToPlane(plane_, p1);
	}
	else
	{
		//struct Plane plane_ = Geometry::SHAPES::createPlane(p1, -from_p_to_cam);//-optix::normalize(Mediator::RequestCameraInstance().getW()));
		//p2 = Geometry::VECTOR_SPACE::projectPointToPlane(plane_, p2);
	}


	
	optix::float3 axis = optix::normalize(p2 - p1);
	optix::float3 tang = optix::normalize(optix::cross(p2 - p1, from_p_to_cam)); //Mediator::RequestCameraInstance().getW()) );

	optix::float3 l_ar, r_ar, ar1, ar2;
	// draw up tangent
	{
		if (rot_d > 0)
			glUniform3f((*m_program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		else
			glUniform3f((*m_program)["uniform_diffuse"], def_color.r, def_color.g, def_color.b);

		glBegin(GL_LINE_LOOP);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glEnd();

		// draw up arrow
		l_ar = optix::normalize(p2 - p0_);
		r_ar = optix::normalize(p2 - p0_);
		ar1 = p2 - tang * arrow_size / 2 - axis * arrow_size;
		ar2 = p2 + tang * arrow_size / 2 - axis * arrow_size;


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
		if (rot_d > 0)
			glUniform3f((*m_program)["uniform_diffuse"], def_color.r, def_color.g, def_color.b);
		else
			glUniform3f((*m_program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);

		glBegin(GL_LINE_LOOP);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p1.x, p1.y, p1.z);
		glEnd();

		// draw down arrow
		l_ar = optix::normalize(p1 - 0);
		r_ar = optix::normalize(p1 - p0_);
		ar1 = p1 - tang * arrow_size / 2 + axis * arrow_size;
		ar2 = p1 + tang * arrow_size / 2 + axis * arrow_size;

		glBegin(GL_LINE_LOOP);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(ar1.x, ar1.y, ar1.z);
		glEnd();
		glBegin(GL_LINE_LOOP);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(ar2.x, ar2.y, ar2.z);
		glEnd();
	}

	glUniform1f((*m_program)["uniform_alpha"], 0.3f);
}

void draw::ArcSegment(IntersectionPoint p, float r, float start_angle, float d_angle, int segments)
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
void draw::Grid(optix::float3 center, int num_segments, float seg_length, float seg_width, glm::vec3 color)
{
	ShaderProgram* program = Mediator::RequestShaderProgram();
	optix::float3 p1, p2;
	float dist = 20.0f;
	
	// start : - num_segments/2.0f * 20.0f;
	float x_start = -num_segments / 2.0f * dist;

	glLineWidth(seg_width);
	glUniform3f((*program)["uniform_diffuse"], color.x, color.y, color.z);

	for (int i = 0; i < num_segments; i++)
	{
		p1 = optix::make_float3(x_start + dist*i, 0.0f, -seg_length * 0.5f);
		p2 = optix::make_float3(x_start + dist*i, 0.0f,  seg_length * 0.5f);

		glBegin(GL_LINES);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glEnd();
	}

	glLineWidth(UTILITY_LINE_WIDTH);
}
void draw::Circle(float _x, float _y, float _z, float _r, int segments, int vertical, GLenum type)
{
	float theta = 2 * M_PIf / float(segments);
	float tangetial_factor = tanf(theta);
	float radial_factor = 1 - cosf(theta);

	float x = _x + _r;
	float y = _y;

	glBegin(type);
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
void draw::Circle(optix::float3 p, float _r, int segments, int vertical, float line_thickness , bool stripped , GLenum type )
{

	glLineWidth(line_thickness);

	if (stripped)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glLineStipple(16.0f, 0xAAAA); // 16 gia ta face vectors //
		glEnable(GL_LINE_STIPPLE);
	}

	float _x = p.x;
	float _y = p.y;
	float _z = p.z;

	float theta = 2 * M_PIf / float(segments);
	float tangetial_factor = tanf(theta);
	float radial_factor = 1 - cosf(theta);

	float x = _x + _r;
	float y = _y;

	glBegin(type);
	for (int ii = 0; ii < segments; ii++)
	{

		if (vertical)
			glVertex3f(x, y, _z);
		else
			glVertex3f(x, _z, y);

		
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

	if(stripped)
		glPopAttrib();

	glLineWidth(1.0f);
}
void draw::Circle2D(float cx, float cy, float r, int num_segments)
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
void draw::CircleAligned(optix::float3 c, float r, UtilityPlane plane, int num_segments)
{
	float d_theta = 2 * M_PIf / float(num_segments);
	float theta = 0.0f;

	float x = c.x;
	float y = c.y;
	float z = c.z;

	float _x;
	float _y;

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
void draw::Arc_arbitr_plane(optix::float3 c, float r, float s_angle, float e_angle, optix::float3 &u, optix::float3& v, int Nsegments, int is_arc, GLenum draw_type)

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
	if (is_arc)
		glVertex3f(c.x, c.y, c.z);
	glEnd();
}
void draw::Arc_arbitr_plane(optix::float3 c, float r, float s_angle, float e_angle, optix::float3 &u, optix::float3& v, int Nsegments, int is_arc, bool stripped, int stripple_factor, float thickness, GLenum draw_type)
{
	//s_angle = Geometry::GENERAL::toRadians(s_angle);
	//e_angle = Geometry::GENERAL::toRadians(e_angle);

	if (stripped)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glLineStipple(stripple_factor, 0xAAAA); // 16 gia ta face vectors //
		glEnable(GL_LINE_STIPPLE);
	}

	float theta = s_angle;
	float theta_dt = (e_angle - s_angle) / ((float)Nsegments - 1.0f);
	

	glLineWidth(thickness);

	glBegin(draw_type);
	if (is_arc)
		glVertex3f(c.x, c.y, c.z);
	for (int iter = 0; iter < Nsegments; iter++)
	{
		optix::float3 pi = (r*cosf(theta))*u + (r*sinf(theta))*v + c;
		glVertex3f(pi.x, pi.y, pi.z);

		theta += theta_dt;
	}
	if (is_arc)
		glVertex3f(c.x, c.y, c.z);
	glEnd();


	
	if (stripped)
	{
		glDisable(GL_LINE_STIPPLE);
		glPopAttrib();
	}

	glLineWidth(1.0f);

}
void draw::Arc_arbitr_plane_to_Origin(optix::float3 o, optix::float3 c, float r, float s_angle, float e_angle, optix::float3 &u, optix::float3& v, int Nsegments, int is_arc, GLenum draw_type, int jump_index )
{
	float theta = s_angle;
	float theta_dt = (e_angle - s_angle) / ((float)Nsegments - 1.0f);

	int pivot = 0;
	int iter  = 0;
	int k     = jump_index;
	////std::cout << "\n - Nsegments : " << Nsegments << std::endl;
	////std::cout << " - LOOP:" << std::endl;

	if (k > 0)
	{
		glBegin(draw_type);
		for (int iter = 0; iter < Nsegments; iter++)
		{
			optix::float3 pi = (r*cosf(theta))*u + (r*sinf(theta))*v + c;
			glVertex3f(pi.x, pi.y, pi.z);

			if ( k >= jump_index )
			{
				glVertex3f(o.x,o.y,o.z);
				glVertex3f(pi.x, pi.y, pi.z);
				k = 0;
			}

			theta += theta_dt;
			k++;
		}
		glEnd();
	}
	else
	{
		while (iter <= Nsegments)
		{
			////std::cout << "\n" << std::endl;
			////std::cout << " - iter : " << iter << std::endl;
			////std::cout << " - pivot : " << pivot << std::endl;

			optix::float3 pi = (r*cosf(theta))*u + (r*sinf(theta))*v + c;

			if (pivot == 0)
			{
				////std::cout << " ~Begin():" << std::endl;
				glBegin(draw_type);
				glVertex3f(o.x, o.y, o.z);
			}
			else if (pivot < 3)
			{
				glVertex3f(pi.x, pi.y, pi.z);
				if (pivot == 1)
				{
					iter++;
					theta += theta_dt;
				}
			}
			else if (pivot == 3)
			{
				glVertex3f(o.x, o.y, o.z);
			}


			if (pivot == 3 || iter == Nsegments)
			{
				////std::cout << " ~End():" << std::endl;
				glEnd();
				pivot = 0;

				if (iter == Nsegments)
					break;
			}
			else
				pivot++;
		}

	}

	


}
void draw::Arc_on_plane( optix::float3 c, float r, float s_angle, float e_angle, optix::float3 u, optix::float3 v, int Nsegments , GLenum type )
{
	float theta    = s_angle;
	float theta_dt = (e_angle - s_angle) / ((float)Nsegments - 1.0f);

	glBegin(type);
	
	glVertex3f(c.x, c.y, c.z);
	for (int iter = 0; iter < Nsegments; iter++)
	{
		optix::float3 pi = c + ((cosf(theta))*u + (sinf(theta))*v) * r;
		glVertex3f(pi.x, pi.y, pi.z);
		theta += theta_dt;
	}
	glVertex3f(c.x, c.y, c.z);
	glEnd();
}
void draw::Arc(float _x, float _y, float _z, float _r, float start_angle, float d_angle, int segments, int vertical)
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
void draw::Arc2(float _x, float _y, float _r, float depth, float start_angle, float d_angle, int segments)
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
void draw::Line3D(float * p1, float * p2)
{
	glBegin(GL_LINE_LOOP);
	glVertex3f(p1[0], p1[1], p1[2]);
	glVertex3f(p2[0], p2[1], p2[2]);
	glEnd();
}
void draw::Line3D(optix::float3& p1, optix::float3& p2, float line_width)
{
	glLineWidth(line_width);
	glBegin(GL_LINE_LOOP);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glEnd();
	glLineWidth(1.0f);
}

void draw::Line2D(float * p1, float * p2) 
{
	glBegin(GL_LINE_LOOP);
	glVertex2f(p1[0], p1[1]);
	glVertex2f(p2[0], p2[1]);
	glEnd();
}
void draw::Arrow(const optix::float3& p1, const optix::float3& p2, float size, bool stripped, float line_width )
{
	optix::float3 axis = optix::normalize(p2 - p1);

	optix::float3 plane_o = Mediator::RequestCameraInstance().getCameraPos();
	optix::float3 plane_n = -Mediator::RequestCameraInstance().getW();

	optix::float3 tangent = optix::cross(axis, -plane_n);

	optix::float3 p11 = p2 + 4.0f*size * tangent - size * axis;
	optix::float3 p22 = p2 - 4.0f*size * tangent - size * axis;

	glLineWidth(line_width);

	glBegin(GL_LINES);
	glVertex3f(p11.x, p11.y,p11.z);
	glVertex3f(p2.x, p2.y,p2.z);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(p22.x, p22.y,p22.z);
	glVertex3f(p2.x, p2.y,p2.z);
	glEnd();

	glLineWidth(UTILITY_LINE_WIDTH);


}
void draw::Arrow(optix::float3& p1, optix::float3& p2, float size, const VIEWPORT& vp, bool stripped, float line_width )
{

	PinholeCamera cam = ViewportManager::GetInstance().getViewportCamera(vp.id);
	//
	optix::float3 axis = optix::normalize(p2 - p1);
	optix::float3 plane_o = cam.getCameraPos();
	optix::float3 plane_n = -cam.getW();
	optix::float3 tangent = optix::cross(axis, -plane_n);
	optix::float3 p11 = p2 + 4.0f*size * tangent - size * axis;
	optix::float3 p22 = p2 - 4.0f*size * tangent - size * axis;

	glLineWidth(line_width);

	glBegin(GL_LINES);
	glVertex3f(p11.x, p11.y, p11.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(p22.x, p22.y, p22.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glEnd();

	glLineWidth(UTILITY_LINE_WIDTH);


}
void draw::stripped_Line3D(optix::float3 &p1, optix::float3 &p2, float frag_length , bool arrow, float line_width , int stripple_factor )
{
	
	glPushAttrib(GL_ENABLE_BIT);
	glLineWidth( line_width );
	glLineStipple( stripple_factor , 0xAAAA ); // 16 gia ta face vectors //
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glEnd();
	glPopAttrib();

	glLineWidth(UTILITY_LINE_WIDTH);

	if( arrow )
		draw::Arrow(p1, p2, frag_length, false );

}
void draw::Plane(optix::float3 p, optix::float3 u, optix::float3 v)
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
void draw::Plane_(optix::float3 c, optix::float3 u, optix::float3 v, optix::float2 scale, float line_length, GLenum type )
{
	glLineWidth(line_length);

	optix::float3 p1 = c + u * scale.x;
	optix::float3 p2 = c + v * scale.y;
	optix::float3 p3 = c + ( u * scale.x + v * scale.y );

	glBegin( type );
	glVertex3f(c.x, c.y, c.z);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glEnd();

	glLineWidth(1.0f);
}
void draw::Plane_Centered(optix::float3 c, optix::float3 u, optix::float3 v, optix::float2 scale, float line_length , GLenum type )
{

	glLineWidth(line_length);

	optix::float3 p0 = c + u * scale.x + v * scale.y;
	optix::float3 p1 = c - u * scale.x + v * scale.y;
	optix::float3 p2 = c - u * scale.x - v * scale.y;
	optix::float3 p3 = c + u * scale.x - v * scale.y;


	glBegin(type);

	glVertex3f(p0.x, p0.y, p0.z);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);

	glEnd();

	glLineWidth(1.0f);

}
void draw::Plane_Centered(optix::float3 c, optix::float3 u, optix::float3 v, optix::float2 scale, optix::float3 * points, float line_length, GLenum type)
{

	glLineWidth(line_length);

	optix::float3 p0 = c + u * scale.x + v * scale.y;
	optix::float3 p1 = c - u * scale.x + v * scale.y;
	optix::float3 p2 = c - u * scale.x - v * scale.y;
	optix::float3 p3 = c + u * scale.x - v * scale.y;
	points[0] = p0;
	points[1] = p1;
	points[2] = p2;
	points[3] = p3;

	glBegin(type);

	glVertex3f(p0.x, p0.y, p0.z);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);

	glEnd();

	glLineWidth(1.0f);

}
void draw::Plane( struct Plane plane, optix::float3 offset, float scale, float line_witdth, GLenum type )
{
	glLineWidth(line_witdth);

	optix::float3 p0 = offset + plane.p0 * scale;
	optix::float3 p1 = offset + plane.p1 * scale;
	optix::float3 p2 = offset + plane.p2 * scale;
	optix::float3 p3 = offset + plane.p3 * scale;

	glBegin(type);
	glVertex3f(p0.x, p0.y, p0.z);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glEnd();

	glLineWidth(1.0f);
}
void draw::Cube(optix::float3 c, float size, optix::float3 * M_WORKING_AXIS, bool fill)
{
	GLenum type;
	if (fill)
		type = GL_POLYGON;
	else
		type = GL_LINE_LOOP;

	glDisable(GL_CULL_FACE);
	
	optix::float3 u = M_WORKING_AXIS[0];
	optix::float3 v = M_WORKING_AXIS[1];
	optix::float3 w = M_WORKING_AXIS[2];
	optix::float3 p, p1, p2, p3, p4;


	//1st face
	p = c - u * size / 2;
	p1 = p - w * size / 2 + v * size/2;
	p2 = p1 + w * size;
	p3 = p2 - v * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//2nd face
	p = c + u * size / 2;
	p1 = p - w * size / 2 + v * size / 2;
	p2 = p1 + w * size;
	p3 = p2 - v * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//3d face
	p = c - v * size / 2;
	p1 = p - w * size / 2 + u * size / 2;
	p2 = p1 + w * size;
	p3 = p2 - u * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//4th face
	p = c + v * size / 2;
	p1 = p - w * size / 2 + u * size / 2;
	p2 = p1 + w * size;
	p3 = p2 - u * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//5th face
	p = c - w * size / 2;
	p1 = p - v * size / 2 + u * size / 2;
	p2 = p1 + v * size;
	p3 = p2 - u * size;
	p4 = p3 - v * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//6th face
	p = c + w * size / 2;
	p1 = p - v * size / 2 + u * size / 2;
	p2 = p1 + v * size;
	p3 = p2 - u * size;
	p4 = p3 - v * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	

}
void draw::Cube_Not_Centered(optix::float3 c, float size, optix::float3 * M_WORKING_AXIS, bool fill)
{
	GLenum type;
	if (fill)
		type = GL_POLYGON;
	else
		type = GL_LINE_LOOP;

	glDisable(GL_CULL_FACE);

	optix::float3 u = M_WORKING_AXIS[0];
	optix::float3 v = M_WORKING_AXIS[1];
	optix::float3 w = M_WORKING_AXIS[2];
	optix::float3 p, p1, p2, p3, p4;


	//1st face
	p  = c - u  * size;
	p1 = p - w  * size + v * size;
	p2 = p1 + w * size;
	p3 = p2 - v * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//2nd face
	p = c + u * size;
	p1 = p - w * size + v * size;
	p2 = p1 + w * size;
	p3 = p2 - v * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//3d face
	p = c - v * size;
	p1 = p - w * size + u * size;
	p2 = p1 + w * size;
	p3 = p2 - u * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//4th face
	p = c + v * size;
	p1 = p - w * size + u * size;
	p2 = p1 + w * size;
	p3 = p2 - u * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//5th face
	p = c - w * size;
	p1 = p - v * size  + u * size;
	p2 = p1 + v * size;
	p3 = p2 - u * size;
	p4 = p3 - v * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//6th face
	p = c + w * size;
	p1 = p - v * size + u * size;
	p2 = p1 + v * size;
	p3 = p2 - u * size;
	p4 = p3 - v * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();



}

void draw::Cube(optix::float3 c, optix::float3 size, optix::float3 * M_WORKING_AXIS, bool fill)
{
	GLenum type;
	if (fill)
		type = GL_POLYGON;
	else
		type = GL_LINE_LOOP;

	glDisable(GL_CULL_FACE);

	optix::float3 u = M_WORKING_AXIS[0];
	optix::float3 v = M_WORKING_AXIS[1];
	optix::float3 w = M_WORKING_AXIS[2];
	optix::float3 p, p1, p2, p3, p4;


	//1st face
	p  = c  - u * size / 2;
	p1 = p  - w * size / 2 + v * size / 2;
	p2 = p1 + w * size;
	p3 = p2 - v * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//2nd face
	p = c + u * size / 2;
	p1 = p - w * size / 2 + v * size / 2;
	p2 = p1 + w * size;
	p3 = p2 - v * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//3d face
	p = c - v * size / 2;
	p1 = p - w * size / 2 + u * size / 2;
	p2 = p1 + w * size;
	p3 = p2 - u * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//4th face
	p = c + v * size / 2;
	p1 = p - w * size / 2 + u * size / 2;
	p2 = p1 + w * size;
	p3 = p2 - u * size;
	p4 = p3 - w * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//5th face
	p = c - w * size / 2;
	p1 = p - v * size / 2 + u * size / 2;
	p2 = p1 + v * size;
	p3 = p2 - u * size;
	p4 = p3 - v * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

	//6th face
	p = c + w * size / 2;
	p1 = p - v * size / 2 + u * size / 2;
	p2 = p1 + v * size;
	p3 = p2 - u * size;
	p4 = p3 - v * size;
	glBegin(type);
	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();



}

void draw::Point3D(optix::float3 c, float size, glm::vec3 color)
{
	ShaderProgram * program = Mediator::RequestShaderProgram();
	glUniform3f((*program)["uniform_diffuse"], color.x, color.y, color.z);
	glPointSize(size);
	glBegin(GL_POINTS);
	glVertex3f(c.x, c.y, c.z);
	glEnd();
	glPointSize(1.0f);
}
void draw::Text(glm::vec3 p, glm::vec3 color, void * font, char * msg)
{
	glColor3f(color.r, color.g, color.b);
	glRasterPos3f(p.x, p.y, p.z);
	int len, i;
	len = (int)strlen(msg);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(font, msg[i]);
	}
}
void draw::Text_onScreen(glm::vec2 p, glm::vec3 color, void * font, char * msg)
{
	int w = Mediator::RequestWindowSize(0);
	int h = Mediator::RequestWindowSize(1);

	//
	//float screen_y = p.y; float screen_x = p.x;
	float screen_x = 2.0f*((p.x / w)) - 1.0f; float screen_y = 2.0f*(1 - (p.y / h)) - 1.0f;
	
	////std::cout << " - screen : [ " << screen_x << " , " << screen_y << " ] " << std::endl;
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluOrtho2D(0, w, 0, h);
	//glOrtho(-1, 1, -1, 1, 0, 1);

	glColor3f(color.r, color.g, color.b);
	glRasterPos2f(screen_x, screen_y);
	int len, i;

	char * msg2 = "asdasdsa";
	len = (int)strlen(msg2);
	
	for (i = 0; i < len; i++) 
	{
		glutBitmapCharacter(font, msg2[i]);
	}
}

void draw::Cursor(float * mouse_pos, CURSOR_TYPE cursor_type , ShaderProgram * shader)
{

	
	float x = mouse_pos[0];
	float y = mouse_pos[1];

	int w = Mediator::RequestWindowSize(0);
	int h = Mediator::RequestWindowSize(1);
	float a = w / h;

	float screen_x = 2.0f*((x / w)) - 1.0f;
	float screen_y = 2.0f*(1 - (y / h)) - 1.0f;

	float size_factor_x = 550.0f / w; //850
	float size_factor_y = 550.0f / h;

	//glm::vec2 d = glm::vec2(0.02f , 0.02f + 0.02f * a);
	glm::vec2 d = glm::vec2(0.04f * size_factor_x, 0.04f * a * size_factor_y);

	shader->Bind();
	GLuint tex = *AssetManager::GetInstance().GetCursorTexture(cursor_type);


	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i((*shader)["uniform_texture"], 0);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(screen_x - d.x, screen_y - d.y);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(screen_x + d.x, screen_y - d.y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(screen_x + d.x, screen_y + d.y);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(screen_x - d.x, screen_y + d.y);

	glEnd();

	/*
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(screen_x - 0.1, screen_y - 0.1);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(screen_x + 0.1, screen_y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(screen_x + 0.1, screen_y - 0.1);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(screen_x, screen_y - 0.1);
	glEnd();
	*/

	shader->Unbind();


	//delete mouse_pos;
	


}
void draw::Cursor(const optix::float2& mousePos, const optix::float2& window_size, CURSOR_TYPE cursor_type, ShaderProgram * shader)
{
	int w = window_size.x;
	int h = window_size.y;
	//float a = w / (float)h;
	float a = 1.0f;
	


	
	float screen_x = 2.0f*((mousePos.x / w)) - 1.0f;
	float screen_y = 2.0f*(1 - (mousePos.y / h)) - 1.0f;

	float size_factor_x = 550.0f / w; //850
	float size_factor_y = 550.0f / h;

	glm::vec2 d = glm::vec2(0.04f * size_factor_x, 0.04f * a * size_factor_y);
	
	shader->Bind();
	GLuint tex = *AssetManager::GetInstance().GetCursorTexture(cursor_type);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i((*shader)["uniform_texture"], 0);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(screen_x - d.x, screen_y - d.y);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(screen_x + d.x, screen_y - d.y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(screen_x + d.x, screen_y + d.y);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(screen_x - d.x, screen_y + d.y);

	glEnd();

	shader->Unbind();

}
void draw::Cursor_Test(const optix::float2& mousePos, const optix::float2& window_size, CURSOR_TYPE cursor_type, ShaderProgram& program)
{

	/*
	int w = window_size.x;
	int h = window_size.y;
	float a = w / h;
	//a = 1.0f;

	float screen_x = 2.0f*((mousePos.x / w)) - 1.0f;
	float screen_y = 2.0f*(1 - (mousePos.y / h)) - 1.0f;

	float size_factor_x = 550.0f / w; //850
	float size_factor_y = 550.0f / h;

	glm::vec2 d = glm::vec2(0.04f * size_factor_x, 0.04f * a * size_factor_y);

	shader->Bind();
	GLuint tex = *AssetManager::GetInstance().GetCursorTexture(cursor_type);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i((*shader)["uniform_texture"], 0);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(screen_x - d.x, screen_y - d.y);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(screen_x + d.x, screen_y - d.y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(screen_x + d.x, screen_y + d.y);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(screen_x - d.x, screen_y + d.y);

	glEnd();

	shader->Unbind();

	*/

	float width  = window_size.x;
	float height = window_size.y;
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 proj_matrix = glm::perspective(glm::radians(M_FOV), width / height, NEAR, FAR);


}
void draw::Cursor(const optix::float2& mouse_pos, CURSOR_TYPE cursor_type, const VIEWPORT& vp, ShaderProgram * shader)
{

	
	float x = mouse_pos.x;
	float y = mouse_pos.y;

	int w = vp.renderSize.x;
	int h = vp.renderSize.y;
	float a = w / h;

	float screen_x = 2.0f*((x / w))     - 1.0f;
	float screen_y = 2.0f*(1 - (y / h)) - 1.0f;

	float size_factor_x = 550.0f / w; //850
	float size_factor_y = 550.0f / h;

	//glm::vec2 d = glm::vec2(0.02f , 0.02f + 0.02f * a);
	glm::vec2 d = glm::vec2(0.04f * size_factor_x, 0.04f * a * size_factor_y);

	shader->Bind();
	GLuint tex = *AssetManager::GetInstance().GetCursorTexture(cursor_type);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i((*shader)["uniform_texture"], 0);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(screen_x - d.x, screen_y - d.y);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(screen_x + d.x, screen_y - d.y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(screen_x + d.x, screen_y + d.y);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(screen_x - d.x, screen_y + d.y);

	glEnd();

	/*
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(screen_x - 0.1, screen_y - 0.1);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(screen_x + 0.1, screen_y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(screen_x + 0.1, screen_y - 0.1);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(screen_x, screen_y - 0.1);
	glEnd();
	*/

	shader->Unbind();

}
void draw::CameraFocus(PinholeCamera camera_instance, float size, glm::vec3 color)
{
	ShaderProgram * program = Mediator::RequestShaderProgram();
	optix::float3 p = camera_instance.getOrbitCenter();
	optix::float3 pos = camera_instance.getCameraPos();
	float dist = optix::length(pos - camera_instance.getOrbitCenter());
	float scale_factor = size * dist;
	
	
	//glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glm::vec3 diffuseColor;
	glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f) * scale_factor);
	glUniform1f((*program)["uniform_alpha"], 1.f);
	glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	
	diffuseColor = glm::vec3(0.05f, 0.7f, 0.05f);
	glUniform3f((*program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
	glUniform1i((*program)["uniform_stable_circle"], 1);
	glUniform1f((*program)["uniform_alpha"], 1.f);
	glUniform1f((*program)["uniform_camera_scale_factor"], scale_factor * 1.4f);
	draw::Circle(0, 0, 0, 1.f, 10, 1, GL_POLYGON);
	glUniform1i((*program)["uniform_stable_circle"], 0);
	
	
	diffuseColor = color;
	glUniform3f((*program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
	for (unsigned int arc_i = 0; arc_i < 3; arc_i++)
	{

		glLineWidth(1.0f);
		optix::float3 u, v;
		if (arc_i == 0)
		{
			u = GLOBAL_AXIS[1];
			v = GLOBAL_AXIS[2];
		}
		else if (arc_i == 1)
		{
			u = GLOBAL_AXIS[0];
			v = GLOBAL_AXIS[2];
		}
		else
		{
			u = GLOBAL_AXIS[0];
			v = GLOBAL_AXIS[1];
		}


		draw::Arc_arbitr_plane(ZERO_3f, 1.0f, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, 10, 0, GL_LINE_LOOP);
		glLineWidth(UTILITY_LINE_WIDTH);
	}

	

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void draw::CameraFocus(PinholeCamera camera_instance, const VIEWPORT& vp, float size, glm::vec3 color)
{
	ShaderProgram * program = Mediator::RequestShaderProgram();
	optix::float3 p    = camera_instance.getOrbitCenter();
	optix::float3 pos  = camera_instance.getCameraPos();
	float dist         = optix::length(pos - camera_instance.getOrbitCenter());
	float scale_factor1 = (size - 0.0005f) * dist * camera_instance.getInvScaleFactor(vp);
	float scale_factor2 = size * dist * camera_instance.getInvScaleFactor(vp);

	//glDisable(GL_CULL_FACE);
	//glDisable(GL_DEPTH_TEST);

	glm::vec3 diffuseColor;
	glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f) * scale_factor1);
	glUniform1f((*program)["uniform_alpha"], 1.f);
	glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));

	diffuseColor = glm::vec3(0.05f, 0.7f, 0.1f);
	glUniform3f((*program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
	glUniform1i((*program)["uniform_stable_circle"], 1);
	glUniform1f((*program)["uniform_camera_scale_factor"], scale_factor1 * 1.4f);
	draw::Circle(0, 0, 0, 1.f, 10, 1, GL_POLYGON);
	glUniform1i((*program)["uniform_stable_circle"], 0);


	diffuseColor = color;
	model_matrix = glm::translate(glm::mat4(1.0f), VECTOR_TYPE_TRANSFORM::TO_GLM_3f(p)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f) * scale_factor2);
	glUniformMatrix4fv((*program)["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniform3f((*program)["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
	glUniform1f((*program)["uniform_camera_scale_factor"], scale_factor2 * 1.4f);
	//glUniform1f((*program)["uniform_alpha"], 1.0f);
	for (unsigned int arc_i = 0; arc_i < 3; arc_i++)
	{

		glLineWidth(1.0f);
		optix::float3 u, v;
		if (arc_i == 0)
		{
			u = GLOBAL_AXIS[1];
			v = GLOBAL_AXIS[2];
		}
		else if (arc_i == 1)
		{
			u = GLOBAL_AXIS[0];
			v = GLOBAL_AXIS[2];
		}
		else
		{
			u = GLOBAL_AXIS[0];
			v = GLOBAL_AXIS[1];
		}


		draw::Arc_arbitr_plane(ZERO_3f, 1.0f, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, 10, 0, GL_LINE_LOOP);
		glLineWidth(UTILITY_LINE_WIDTH);
	}



	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void draw::Rectangle2D(glm::vec2 a, glm::vec2 b,glm::vec3 color, float line_width, bool dashed , int * render_sides )
{
	int w = Mediator::RequestWindowSize(0);
	int h = Mediator::RequestWindowSize(1);
	
	a.x = 2.0f*((a.x / w)) - 1.0f;
	a.y = 2.0f*(1 - (a.y / h)) - 1.0f;

	b.x = 2.0f*((b.x / w)) - 1.0f;
	b.y = 2.0f*(1 - (b.y / h)) - 1.0f;

	glLineWidth(line_width);

	if (dashed)
	{
		glLineStipple(6, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
	}

	glColor3f(color.x, color.y, color.z);

	int down, up, left, right;
	if (render_sides != 0)
	{
		left  = render_sides[0];
		right = render_sides[1];
		down  = render_sides[2];
		up    = render_sides[3];
	}

	if (render_sides == 0)
	{
		glBegin(GL_LINE_LOOP);
		glVertex2f(a.x, a.y);
		glVertex2f(b.x, a.y);
		glVertex2f(b.x, b.y);
		glVertex2f(a.x, b.y);
		glEnd();
	}
	else
	{
		glLineWidth(left);
		{
			glBegin(GL_LINES);
			glVertex2f(a.x, a.y);
			glVertex2f(a.x, b.y);
			glEnd();
		}
		glLineWidth(right);
		{
			glBegin(GL_LINES);
			glVertex2f(b.x, a.y);
			glVertex2f(b.x, b.y);
			glEnd();
		}
		glLineWidth(down);
		{
			glBegin(GL_LINES);
			glVertex2f(a.x, b.y);
			glVertex2f(b.x, b.y);
			glEnd();
		}
		glLineWidth(up);
		{
			glBegin(GL_LINES);
			glVertex2f(a.x, a.y);
			glVertex2f(b.x, a.y);
			glEnd();
		}
	}

	if (dashed)
	{
		glDisable(GL_LINE_STIPPLE);
	}

	glLineWidth(UTILITY_LINE_WIDTH);
}
void draw::Rectangle2D(const optix::float3& aa, const optix::float3& bb, glm::vec3 color, float line_width, ShaderProgram* shader, const VIEWPORT& vp, bool dashed)
{
	//shader->Bind();

	int w = vp.renderSize.x;
	int h = vp.renderSize.y;

	optix::float2 a = (optix::make_float2(aa));
	optix::float2 b = (optix::make_float2(bb));

	a.x = 2.0f*((a.x / w)) - 1.0f;
	a.y = 2.0f*(1 - (a.y / h)) - 1.0f;

	b.x = 2.0f*((b.x / w)) - 1.0f;
	b.y = 2.0f*(1 - (b.y / h)) - 1.0f;


	glLineWidth(line_width);

	if (dashed)
	{
		glLineStipple(6, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
	}

	glColor3f(color.x, color.y, color.z);

	glBegin(GL_LINE_LOOP);
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, a.y);
	glVertex2f(b.x, b.y);
	glVertex2f(a.x, b.y);
	glEnd();

	if (dashed)
	{
		glDisable(GL_LINE_STIPPLE);
	}

	glLineWidth(UTILITY_LINE_WIDTH);
}
void draw::Rectangle2D(const optix::float3& aa, const optix::float3& bb, glm::vec3 color, float line_width, ShaderProgram* shader, bool dashed)
{
	//shader->Bind();

	int w  = Mediator::RequestWindowSize(0);
	int h = Mediator::RequestWindowSize(1);

	optix::float2 a = (optix::make_float2(aa));
	optix::float2 b = (optix::make_float2(bb));

	a.x = 2.0f*((a.x / w)) - 1.0f;
	a.y = 2.0f*(1 - (a.y / h)) - 1.0f;

	b.x = 2.0f*((b.x / w)) - 1.0f;
	b.y = 2.0f*(1 - (b.y / h)) - 1.0f;


	glLineWidth(line_width);

	if (dashed)
	{
		glLineStipple(6, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
	}

	glColor3f(color.x, color.y, color.z);

	glBegin(GL_LINE_LOOP);
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, a.y);
	glVertex2f(b.x, b.y);
	glVertex2f(a.x, b.y);
	glEnd();

	if (dashed)
	{
		glDisable(GL_LINE_STIPPLE);
	}

	glLineWidth(UTILITY_LINE_WIDTH);
}
void draw::SolidAngle(float r, float length)
{
	float _x = 0.0f;
	float _y = 0.0f;
	float _z = 10.0f;

	bool vertical = true;
	int segments = 40;
	float theta = 2 * M_PIf / float(segments);
	float tangetial_factor = tanf(theta);
	float radial_factor    = 1 - cosf(theta);

	float x = _x + r;
	float y = _y;

	glBegin(GL_LINE_LOOP);
	for (int ii = 0; ii < segments; ii++)
	{

		if (vertical)
		{
			glVertex3f(x, y, _z);
			glVertex3f(0, 0, 0);
			glVertex3f(x, y, _z);
		}
		else
		{
			glVertex3f(x, _z, y);
			glVertex3f(0, 0, 0);
			glVertex3f(x, _z, y);
		}

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
void draw::Frustum(optix::float3 pos, optix::float2 fov, optix::float2 clip_planes, ShaderProgram& program)
{
	
	float _far  = clip_planes.y;
	float fov_0 = fov.x;
	float fov_1 = fov.y;

	// light's default normal
	optix::float3 u = GLOBAL_AXIS[0];
	optix::float3 v = GLOBAL_AXIS[2];
	optix::float3 n = optix::make_float3(0, -1, 0);
	optix::float3 p0 = pos + n * _far;
	auto far_plane = Geometry::SHAPES::createPlane(p0, -n);
	Ray ray0, ray1, ray00, ray11;
	ray0.origin = pos;
	ray1.origin = pos;
	ray00.origin = pos;
	ray11.origin = pos;
	
	optix::float3 d0  = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_0), u) *  optix::make_float4(n, 0.0f));
	optix::float3 d1  = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_1), u) *  optix::make_float4(n, 0.0f));
	optix::float3 d00 = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_0), v) *  optix::make_float4(n, 0.0f));
	optix::float3 d11 = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_1), v) *  optix::make_float4(n, 0.0f));

	d0 = normalize(d0); d1 = normalize(d1); d00 = normalize(d00); d11 = normalize(d11);
	
	ray0.direction = d0;
	ray1.direction = d1;
	ray00.direction = d00;
	ray11.direction = d11;

	optix::float3 i1, i2, i11, i22;
	Geometry::RAY::Intersect_Plane(ray0, far_plane, i1);
	Geometry::RAY::Intersect_Plane(ray1, far_plane, i2);
	Geometry::RAY::Intersect_Plane(ray00, far_plane, i11);
	Geometry::RAY::Intersect_Plane(ray11, far_plane, i22);

	optix::float3 u1 = i1 - p0;
	optix::float3 u2 = i2 - p0;
	optix::float3 u11 = i11 - p0;
	optix::float3 u22 = i22 - p0;
	float r1 = length(u1);
	float r2 = length(u2);
	u1 /= r1;
	u2 /= r2;
	
	float r11 = length(u11);
	float r22 = length(u22);
	u11 /= r11;
	u22 /= r22;


	glUniform3f(program["uniform_diffuse"], 1.0f, 1.0f, 0.5f);
	glUniform1f(program["uniform_alpha"], 0.8f);

	draw::Line3D(pos, p0, 1.0f);
	
	draw::Line3D(pos, p0 + u1  * r1, 1.0f);
	draw::Line3D(pos, p0 - u1  * r1, 1.0f);
	draw::Line3D(pos, p0 + u11 * r11, 1.0f);
	draw::Line3D(pos, p0 - u11 * r11, 1.0f);
	
	draw::Line3D(pos, p0 + u2 * r2, 1.0f);
	draw::Line3D(pos, p0 - u2 * r2, 1.0f);
	draw::Line3D(pos, p0 + u22 * r22, 1.0f);
	draw::Line3D(pos, p0 - u22 * r22, 1.0f);

	glUniform3f(program["uniform_diffuse"], 1.0f, 1.0f, 0.5f);
	glUniform1f(program["uniform_alpha"], 0.8f);
	draw::Arc_arbitr_plane(p0, r1, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, 30, 0, GL_LINE_LOOP);

	
	draw::Arc_arbitr_plane(p0, r2, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, 30, 0, GL_LINE_LOOP);
	


}
void draw::Frustum_Area_Light(optix::float3 pos, optix::float2 fov, optix::float2 clip_planes, ShaderProgram& program)
{

	float _far = clip_planes.y;
	float fov_0 = fov.x;
	float fov_1 = fov.y;

	// light's default normal
	optix::float3 u = GLOBAL_AXIS[0];
	optix::float3 v = GLOBAL_AXIS[2];
	optix::float3 n = optix::make_float3(0, -1, 0);
	optix::float3 p0 = pos + n * _far;
	auto far_plane = Geometry::SHAPES::createPlane(p0, -n);
	Ray ray0, ray1, ray00, ray11;
	ray0.origin = pos;
	ray1.origin = pos;
	ray00.origin = pos;
	ray11.origin = pos;

	optix::float3 d0 = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_0), u) *  optix::make_float4(n, 0.0f));
	optix::float3 d1 = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_1), u) *  optix::make_float4(n, 0.0f));
	optix::float3 d00 = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_0), v) *  optix::make_float4(n, 0.0f));
	optix::float3 d11 = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_1), v) *  optix::make_float4(n, 0.0f));

	d0 = normalize(d0); d1 = normalize(d1); d00 = normalize(d00); d11 = normalize(d11);

	ray0.direction = d0;
	ray1.direction = d1;
	ray00.direction = d00;
	ray11.direction = d11;

	optix::float3 i1, i2, i11, i22;
	Geometry::RAY::Intersect_Plane(ray0, far_plane, i1);
	Geometry::RAY::Intersect_Plane(ray1, far_plane, i2);
	Geometry::RAY::Intersect_Plane(ray00, far_plane, i11);
	Geometry::RAY::Intersect_Plane(ray11, far_plane, i22);

	optix::float3 u1 = i1 - p0;
	optix::float3 u2 = i2 - p0;
	optix::float3 u11 = i11 - p0;
	optix::float3 u22 = i22 - p0;
	float r1 = length(u1);
	float r2 = length(u2);
	u1 /= r1;
	u2 /= r2;

	float r11 = length(u11);
	float r22 = length(u22);
	u11 /= r11;
	u22 /= r22;


	glUniform3f(program["uniform_diffuse"], 1.0f, 1.0f, 0.5f);
	glUniform1f(program["uniform_alpha"], 0.8f);

	//draw::Line3D(pos, p0, 1.0f);

	draw::Line3D(pos, p0 + u1 * r1, 1.0f);
	draw::Line3D(pos, p0 - u1 * r1, 1.0f);
	draw::Line3D(pos, p0 + u11 * r11, 1.0f);
	draw::Line3D(pos, p0 - u11 * r11, 1.0f);

	//draw::Line3D(pos, p0 + u2 * r2, 1.0f);
	//draw::Line3D(pos, p0 - u2 * r2, 1.0f);
	//draw::Line3D(pos, p0 + u22 * r22, 1.0f);
	//draw::Line3D(pos, p0 - u22 * r22, 1.0f);

	glUniform3f(program["uniform_diffuse"], 1.0f, 1.0f, 0.5f);
	glUniform1f(program["uniform_alpha"], 0.8f);
	draw::Arc_arbitr_plane(p0, r1, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, 30, 0, GL_LINE_LOOP);
	//draw::Arc_arbitr_plane(p0, r2, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, 30, 0, GL_LINE_LOOP);



}
void draw::Frustum_Cone(optix::float3 pos, optix::float2 fov, optix::float2 clip_planes, optix::float3 n_, ShaderProgram& program, float line_width , glm::vec3 color, bool is_directional)
{

	float _far  = clip_planes.y;
	float fov_0 = fov.x;
	float fov_1 = fov.y;

	// light's default normal
	optix::float3 u = GLOBAL_AXIS[0];
	optix::float3 v = GLOBAL_AXIS[1];
	optix::float3 n = GLOBAL_AXIS[2];
	
	optix::float3 p0 = pos + n * _far;
	auto far_plane = Geometry::SHAPES::createPlane(p0, -n);
	Ray ray0, ray1, ray00, ray11;
	ray0.origin  = pos;
	ray1.origin  = pos;
	ray00.origin = pos;
	ray11.origin = pos;

	optix::float3 d0  = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_0), u) *  optix::make_float4(n, 0.0f));
	optix::float3 d00 = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_0), v) *  optix::make_float4(n, 0.0f));

	d0  = normalize(d0); 
	d00 = normalize(d00); 

	ray0.direction  = d0;
	ray00.direction = d00;

	optix::float3 i1, i2, i11, i22;
	Geometry::RAY::Intersect_Plane(ray0,  far_plane, i1);
	Geometry::RAY::Intersect_Plane(ray00, far_plane, i11);

	optix::float3 u1  = i1 - p0;
	optix::float3 u11 = i11 - p0;
	float r1 = length(u1);
	u1 /= r1;

	float r11 = length(u11);
	u11 /= r11;

	optix::float4 t  = optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(45.0f), n) * optix::make_float4(u1,  0.0f);
	optix::float4 t2 = optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(45.0f), n) * optix::make_float4(u11, 0.0f);
	optix::float3 u2 = normalize(optix::make_float3(t.x, t.y, t.z));
	optix::float3 u3 = normalize(optix::make_float3(t2.x, t2.y, t2.z));

	
	int segs   = is_directional ? 10 : 32;
	line_width = is_directional ? 2 : 2;
	
	if (!is_directional
		//&& false
		)
	{
		glUniform3f(program["uniform_diffuse"], color.r* 0.8f, color.g* 0.8f, color.b* 0.8f);
		glUniform1f(program["uniform_alpha"], 0.8f);

		draw::Line3D(pos, p0 + u1 * r1, line_width);
		draw::Line3D(pos, p0 - u1 * r1, line_width);
		draw::Line3D(pos, p0 + u11 * r11, line_width);
		draw::Line3D(pos, p0 - u11 * r11, line_width);
	}

	glLineWidth(line_width);
	if (is_directional)
	{
		//glUniform3f(program["uniform_diffuse"], color.r, color.g, color.b);
		//glUniform1f(program["uniform_alpha"], 0.8f);
		//draw::Arc_arbitr_plane_to_Origin(pos, p0, r1, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, segs, 0, GL_POLYGON);
		
		//glUniform3f(program["uniform_diffuse"], 0.0f,0.0f,0.0f);
		glUniform3f(program["uniform_diffuse"], color.r, color.g, color.b);
		glUniform1f(program["uniform_alpha"], 0.8f);
		draw::Arc_arbitr_plane_to_Origin(pos, p0, r1, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, segs, 0, GL_LINE_LOOP);
	}
	else
	{
		glUniform3f(program["uniform_diffuse"], color.r* 0.8f, color.g* 0.8f, color.b* 0.8f);
		glUniform1f(program["uniform_alpha"], 0.9f);
		draw::Arc_arbitr_plane(p0, r1, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, segs, 0, GL_LINE_LOOP);

		//glUniform3f(program["uniform_diffuse"], color.r * 0.8f, color.g * 0.8f, color.b * 0.8f);
		//glUniform3f(program["uniform_diffuse"], 0.2f, 0.2f, 0.2f);
		//glUniform1f(program["uniform_alpha"], 0.8f);
		//draw::Arc_arbitr_plane_to_Origin(pos, p0, r1, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, segs, 0, GL_LINE_LOOP, 6);
	}



	glLineWidth(1.0f);
}
void draw::Frustum_Cone2(optix::float3 pos, optix::float2 fov, optix::float2 clip_planes, optix::float3 n_, ShaderProgram& program, float line_width)
{

	float _far = clip_planes.y;
	float fov_0 = fov.x;
	float fov_1 = fov.y;

	// light's default normal
	optix::float3 u = GLOBAL_AXIS[0];
	optix::float3 v = GLOBAL_AXIS[1];
	optix::float3 n = GLOBAL_AXIS[2];

	optix::float3 p0 = pos + n * _far;
	auto far_plane = Geometry::SHAPES::createPlane(p0, -n);
	Ray ray0, ray1, ray00, ray11;
	ray0.origin = pos;
	ray1.origin = pos;
	ray00.origin = pos;
	ray11.origin = pos;

	optix::float3 d0 = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_0), u) *  optix::make_float4(n, 0.0f));
	optix::float3 d00 = optix::make_float3(optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(fov_0), v) *  optix::make_float4(n, 0.0f));

	d0 = normalize(d0);
	d00 = normalize(d00);

	ray0.direction = d0;
	ray00.direction = d00;

	optix::float3 i1, i2, i11, i22;
	Geometry::RAY::Intersect_Plane(ray0, far_plane, i1);
	Geometry::RAY::Intersect_Plane(ray00, far_plane, i11);

	optix::float3 u1 = i1 - p0;
	optix::float3 u11 = i11 - p0;
	float r1 = length(u1);
	u1 /= r1;

	float r11 = length(u11);
	u11 /= r11;

	optix::float4 t = optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(45.0f), n) * optix::make_float4(u1, 0.0f);
	optix::float4 t2 = optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(45.0f), n) * optix::make_float4(u11, 0.0f);
	optix::float3 u2 = normalize(optix::make_float3(t.x, t.y, t.z));
	optix::float3 u3 = normalize(optix::make_float3(t2.x, t2.y, t2.z));

	glUniform3f(program["uniform_diffuse"], 0.05f, 0.05f, 0.05f);
	glUniform1f(program["uniform_alpha"], 1.0f);

	optix::float3 p1 = p0 + u1 * r1;
	optix::float3 p2 = p0 - u1 * r1;
	optix::float3 p3 = p0 + u11 * r11;
	optix::float3 p4 = p0 - u11 * r11;

	draw::Line3D(pos, p0 + u1 * r1, line_width);
	draw::Line3D(pos, p0 - u1 * r1, line_width);
	draw::Line3D(pos, p0 + u11 * r11, line_width);
	draw::Line3D(pos, p0 - u11 * r11, line_width);
	

	glLineWidth(line_width);
	glUniform3f(program["uniform_diffuse"], 0.1f, 0.8f, 0.1f);
	glUniform1f(program["uniform_alpha"], 1.f);
	draw::Arc_arbitr_plane(p0, r1, 0.0f, Geometry::GENERAL::toRadians(360.0f), u, v, 30, 0, GL_LINE_LOOP);
	glLineWidth(1.0f);

}

void draw::Light_Object(optix::float3 pos, Light_Parameters params, float scale , float line_width )
{
	
	float size = 1.0f;
	Light_Type type = params.type;

	switch (type)
	{

	
	// parallelogram
	case  QUAD_LIGHT:
	{

	}
	break;


	// Sphere
	case SPHERICAL_LIGHT:
	{

	}
	break;

	// A cylinded like object
	case SPOTLIGHT:
	{

	}
	break;

	
	case POINT_LIGHT:
	{

	}
	break;

	}

}
void draw::Spot_Light(ShaderProgram& program,optix::float3 c, optix::float3 u, optix::float3 v, int sign, optix::float2 scale, float dist, glm::vec3 surf_color, glm::vec3 outline_col, float line_length)
{
	optix::float3 points1[4] = { ZERO_3f, ZERO_3f, ZERO_3f, ZERO_3f };
	optix::float3 points2[4] = { ZERO_3f, ZERO_3f, ZERO_3f, ZERO_3f };
	float scale_factor = scale.x;
	glm::vec3 color;

	optix::float3 _v = -1.0f * optix::cross(normalize(u), normalize(v));



	optix::float3 l0 = sign > 0 ? c + _v * dist : c;
	color = surf_color;
	glUniform3f(program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(program["unfirom_alpha"], 1.0f);
	glDisable(GL_CULL_FACE);
	draw::Plane_Centered(l0, u, v, optix::make_float2(scale_factor) * 0.5f, points1, 1, GL_POLYGON);
	glEnable(GL_CULL_FACE);

	//
	color = outline_col;
	glUniform3f(program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(program["unfirom_alpha"], 1.0f);
	draw::Plane_Centered(l0, u, v, optix::make_float2(scale_factor) * 0.5f, line_length, GL_LINE_LOOP);
	
	
	// -------------------------------------------------------------------------------------------------------------


	l0 = sign > 0 ? c : c + _v * dist;
	color = surf_color;
	glUniform3f(program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(program["unfirom_alpha"], 1.0f);
	glDisable(GL_CULL_FACE);
	draw::Plane_Centered(l0, u, v, optix::make_float2(scale_factor) * 1.f, points2, 1, GL_POLYGON);
	glEnable(GL_CULL_FACE);

	color = outline_col;
	glUniform3f(program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(program["unfirom_alpha"], 1.0f);
	draw::Plane_Centered(l0, u, v, optix::make_float2(scale_factor) * 1.f, line_length, GL_LINE_LOOP);


	// -------------------------------------------------------------------------------------------------------------
	
	color = outline_col;
	glUniform3f(program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(program["unfirom_alpha"], 1.0f);
	for (int i = 0; i < 4; i++)
	{
		draw::Line3D(points1[i], points2[i], line_length);
	}


	optix::float3 _pp0[4] = { points1[0], points1[1], points2[1], points2[0] };
	optix::float3 _pp1[4] = { points1[2], points1[3], points2[3], points2[2] };
	optix::float3 _pp2[4] = { points1[0], points1[3], points2[3], points2[0] };
	optix::float3 _pp3[4] = { points1[1], points1[2], points2[2], points2[1] };

	glDisable(GL_CULL_FACE);
	color = surf_color * 0.5f;
	glUniform3f(program["uniform_diffuse"], color.r, color.g, color.b);
	glUniform1f(program["unfirom_alpha"], 1.0f);
	//
	draw::Points( 
		_pp0,
		4,
		0,
		1.0f,
		GL_POLYGON
	);
	
	//
	draw::Points(
		_pp1,
		4,
		0,
		1.0f,
		GL_POLYGON
	);

	draw::Points(
		_pp2,
		4,
		0,
		1.0f,
		GL_POLYGON
	);

	//
	draw::Points(
		_pp3,
		4,
		0,
		1.0f,
		GL_POLYGON
	);
	


}


void draw::view_axis(optix::float3 o,optix::float3 * axis)
{

}