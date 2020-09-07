#include "GeometryFunctions.h"

#include <iostream>

#include "Mediator.h"
#include "Utility_Renderer_Defines.h"
#include "Utilities.h"

#define RAD    0.0174532925f//0.0174533f
#define DEGREE 57.295779513f//57.2958f

#define TO_RAD(x)     (x * RAD)
#define TO_DEGREES(x) (x * DEGREE)

using namespace Geometry;

// GEOM_GENERAL
float GENERAL::round(float val)
{
	if (val < 0) return ceil(val - 0.5);
	return floor(val + 0.5);
}
float GENERAL::toRadians(float degrees)
{
	return TO_RAD(degrees);
	//return degrees * RAD;
}
float GENERAL::toDegrees(float rad)
{
	return TO_DEGREES(rad);
	//return rad * DEGREE;
}
optix::float3 GENERAL::toRadians(const optix::float3 &ff)
{
	optix::float3 f = ff;
	f.x = GENERAL::toRadians(f.x);
	f.y = GENERAL::toRadians(f.y);
	f.z = GENERAL::toRadians(f.z);
	return f;
}
optix::float3  GENERAL::toDegrees(const optix::float3 &ff)
{
	optix::float3 f = ff;
	f.x = GENERAL::toDegrees(f.x);
	f.y = GENERAL::toDegrees(f.y);
	f.z = GENERAL::toDegrees(f.z);
	return f;
}
void GENERAL::clampTo360(optix::float3& v)
{
	float pi = 360.0f * 0.0174533f;
	for (int i = 0; i < 3; i++)
	{
		float val = ((float*)&v)[i];
		if (fabsf(val) > pi)
		{
			int m = fabsf(val) / pi;
			pi *= m;

			if (val < 0.0f)
				val += pi;
			else
				val -= pi;
		}

		((float*)&v)[i] = val;
	}
}

void GENERAL::EulerAnglesFromMatrix(const optix::Matrix4x4& s_rot, optix::float3& rot_attributes)
{
	optix::Matrix4x4 rot = optix::Matrix4x4(s_rot.getData());
	optix::float4 col[3] = { rot.getCol(0), rot.getCol(1), rot.getCol(2) };
	optix::float4 row[3] = { rot.getRow(0), rot.getRow(1), rot.getRow(2) };


	float * d = rot.getData();
	float r11, r12, r13;
	float r21, r22, r23;
	float r31, r32, r33;
	r11 = d[0]; r12 = d[1]; r13 = d[2];
	r21 = d[4]; r22 = d[5]; r23 = d[6];
	r31 = d[8]; r32 = d[9]; r33 = d[10];

	/*
	float r32 = col[1].z;
	float r33 = col[2].z;
	float r31 = col[0].z;
	float r11 = col[0].x;
	float r12 = row[0].y;
	float r13 = row[0].z;
	float r21 = col[0].y;
	*/

	float E1, E2, E3;
	float E11, E22, E33;
	float delta;
	if (r31 == 1 || r31 == -1)
	{
		E3 = 0;
		delta = atan2(r32, r33);
		if (r31 == -1)
		{
			E2 = M_PIf / 2.0f;
			E1 = E3 + atan2(r12, r13);
		}
		else
		{
			E2 = -M_PIf / 2.0f;
			E1 = -E3 + atan2(-r12, -r13);
		}
	}
	else
	{

		E1 = -asin(r31);
		E11 = M_PIf - E1;

		float c1 = cosf(E1);
		float c2 = cosf(E11);

		E2  = atan2((r32 / c1), (r33 / c1));
		E22 = atan2((r32 / c2), (r33 / c2));

		E3  = atan2((r21 / c1), (r11 / c1));
		E33 = atan2((r21 / c2), (r11 / c2));
	}

	float x, y, z;

	// testing method
	//x = atan2(r23, r33);
	//y = atan2(-r13, sqrtf(r11*r11 + r12*r12));
	//float s1 = sinf(x);
	//float c1 = cosf(x);
	//z = atan2(s1*r31 - c1 * r21, c1 * r22 - s1 * r32);

	// typical method
	//x = atan2(-r23, r33);
	//y = atan2(r13, sqrtf(r23*r23 + r33*r33));
	//z = atan2(-r12, r11);

	// typical method
	x = atan2(r32, r33);
	y = atan2(-r31, sqrtf(r32*r32 + r33*r33));
	z = atan2(r21, r11);

	rot_attributes.x = x;
	rot_attributes.y = y;
	rot_attributes.z = z;

}
void GENERAL::EulerAnglesFromMatrix2(optix::Matrix4x4& s_rot, optix::float3& rot_attributes)
{
	//optix::Matrix4x4 rot = s_rot.transpose();
	
	float * d = s_rot.getData();
	float r11, r12, r13;
	float r21, r22, r23;
	float r31, r32, r33;
	r11 = d[0]; r12 = d[1]; r13 = d[2];
	r21 = d[4]; r22 = d[5]; r23 = d[6];
	r31 = d[8]; r32 = d[9]; r33 = d[10];

	
	float x, y, z;
	if (r21 > 0.999)
	{
		y = atan2(r13, r33);
		z = M_PIf / 2.0f;
		x = 0;
	}
	else if (r21 < -0.999)
	{
		y = atan2(r13, r33);
		z = -M_PIf / 2.0f;
		x = 0;
	}
	else
	{
		y = atan2(-r31,  r11);
		z = atan2(-r23,  r22);
		x = asin(r21);
	}

	//float x = atan2(r32, r33);
	//float y = atan2(-r31, sqrtf(r32*r32 + r33 * r33));
	//float z = atan2(r21, r11);

	rot_attributes.x = z;
	rot_attributes.y = y;
	rot_attributes.z = x;
}

void GENERAL::ScaleAttributesFromMatrix(const optix::Matrix4x4& s_scale, optix::float3& scale_attributes)
{
	optix::Matrix4x4 scale = s_scale.transpose();
	float x = scale.getCol(0).x;
	float y = scale.getCol(1).y;
	float z = scale.getCol(2).z;

	scale_attributes.x = x;
	scale_attributes.y = x;
	scale_attributes.z = x;
}

// GEOM_MATH
bool MATH::solveQuadratic(const float& a, const float& b, const float& c, float &x0, float& x1)
{

	float discr = b * b - 4 * a * c;
	if (discr < 0) return false;
	else if (discr == 0) x0 = x1 = -0.5 * b / a;
	else {
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1) std::swap(x0, x1);

	return true;

}


// Geometry::VECTOR_SPACE
optix::float3 VECTOR_SPACE::getPerpendicularBasePlane(optix::float3 dir)
{
	optix::float3 x = optix::make_float3(1, 0, 0);
	optix::float3 y = optix::make_float3(0, 1, 0);
	optix::float3 z = optix::make_float3(0, 0, 1);

	float dx = fabs(optix::dot(dir, x));
	float dy = fabs(optix::dot(dir, y));
	float dz = fabs(optix::dot(dir, z));

	optix::float3 max_plane = x;
	float max = dx;

	if (dy > max)
	{
		max = dy;
		max_plane = y;
	}

	if (dz > max)
	{
		max = dz;
		max_plane = z;
	}

	return max_plane;
}
optix::float3 VECTOR_SPACE::getPerpendicularArbitraryPlane(optix::float3 dir, optix::float3 * base_axis)
{
	optix::float3 x = base_axis[0];
	optix::float3 y = base_axis[1];
	optix::float3 z = base_axis[2];

	float dx = fabs(optix::dot(dir, x));
	float dy = fabs(optix::dot(dir, y));
	float dz = fabs(optix::dot(dir, z));

	optix::float3 max_plane = x;
	float max = dx;

	if (dy > max)
	{
		max = dy;
		max_plane = y;
	}

	if (dz > max)
	{
		max = dz;
		max_plane = z;
	}

	return max_plane;
}
optix::float3 VECTOR_SPACE::getTangentOfCircle(int plane, optix::float3 normal)
{
	optix::Matrix4x4 rot;

	float rad = TO_RAD(90.0f);
	if (plane == 1) // XY - plane
	{
		rot = optix::Matrix4x4::rotate(rad, optix::make_float3(1, 0, 0));
	}
	else if (plane == 2) // XZ - plane
	{
		rot = optix::Matrix4x4::rotate(rad, optix::make_float3(0, 1, 0));
	}
	else if (plane == 3) // YZ - plane
	{
		rot = optix::Matrix4x4::rotate(rad, optix::make_float3(0, 0, 1));
	}

	optix::float4 tang = rot * optix::make_float4(normal, 0.0f);
	return optix::normalize(optix::make_float3(tang));
}
optix::float3 VECTOR_SPACE::getCircleTangentOnPlane(IntersectionPoint point_on_circle, optix::float3 * base_axis, int plane_id)
{
	int axis_id;
	if (plane_id == 1)
		axis_id = 0;
	else if (plane_id == 2)
		axis_id = 1;
	else if (plane_id == 3)
		axis_id = 2;
	
	optix::float3 point = point_on_circle.p;
	optix::float3 normal = point_on_circle.normal;

	optix::float3 tangent = -optix::cross(normal, base_axis[plane_id - 1]);
	return optix::normalize(tangent);
}
float         VECTOR_SPACE::getVectorAngle(optix::float3 a, optix::float3 b)
{
	float _a_ = optix::length(a);
	float _b_ = optix::length(b);
	float dot = optix::dot(a, b);

	return acosf(dot / (_a_*_b_)); // x RADS

}
optix::float3 VECTOR_SPACE::projectPointToPlane(Plane plane, optix::float3& p)
{
	optix::float3 v = p - plane.p0;
	float dist = optix::dot(v, plane.normal);
	optix::float3 proj_point = p - dist * plane.normal;
	return proj_point;
}
optix::float3 * VECTOR_SPACE::OnB(optix::float3 u, optix::float3 v)
{
	u = normalize(u);
	v = normalize(v);

	optix::float3 _w = cross(u, v);
	optix::float3 _v = cross(u, _w);
	optix::float3 _u = cross(_v, _w);

	optix::float3 * onb = new optix::float3[3];
	onb[0] = _u;
	onb[1] = _v;
	onb[2] = _w;
	return onb;
}


//RAY_INTERSECTION
int RAY::Intersect_Plane(Ray ray, Plane plane, optix::float3& intersection_point)
{
	optix::float3 u = ray.direction;
	optix::float3 w = ray.origin - plane.p0;

	float d = optix::dot(u, plane.normal);
	float n = -optix::dot(w, plane.normal);
	if (fabs(d) < 0.000001)
	{
		if (n == 0)
			return 2;
		else
			return 0;
	}

	float d1 = optix::dot(plane.normal, plane.p0);
	float d2 = optix::dot(plane.normal, ray.origin);

	float t = (d1 - d2) / d;
	intersection_point = ray.origin + u * t;
	return 1;

}
int RAY::Intersect_Plane_Clipped(Ray ray, Plane plane, optix::float3& intersection_point)
{
	optix::float3 u = ray.direction;
	optix::float3 w = ray.origin - plane.p0;

	float d = optix::dot(u, plane.normal);
	float n = -optix::dot(w, plane.normal);
	if (fabs(d) < 0.000001)
	{
		if (n == 0)
			return 2;
		else
			return 0;
	}

	float d1 = optix::dot(plane.normal, plane.p0);
	float d2 = optix::dot(plane.normal, ray.origin);

	float t = (d1 - d2) / d;
	intersection_point = ray.origin + u * t;

	// clip //
	float x = intersection_point.x;
	float y = intersection_point.y;
	float z = intersection_point.z;

	optix::float3 min = plane.min;
	optix::float3 max = plane.max;

	
	////std::cout << "\n";
	////std::cout << "\n - isec_p : [ " << x << " , " << y << " , " << z << " ] " << std::endl;
	////std::cout << " - z : [ " << min.z << " , " << max.z << " ] " << std::endl;
	////std::cout << "\n z : " << z << std::endl;
	////std::cout << "    -  [ " << min.z << " , " << max.z << " ] " << std::endl;

	if (x >= min.x && x <= max.x)
	{
		////std::cout << " x " << std::endl;
		if (y >= min.y && y <= max.y)
		{
			////std::cout << " y " << std::endl;
			if (z >= min.z && z <= max.z )
			{
				////std::cout << " z " << std::endl;
				return 1;
			}
		}
	}

	

	return 0;
}
int RAY::Intersect_Triangle(Ray ray, Triangle triangle, optix::float3& intersection_point)
{
	float NdotRay = optix::dot(triangle.normal, ray.direction);
	if (fabs(NdotRay) < 0.00001f)
	{
		////std::cout << "ray parallel to triangle's normal!" << std::endl;
		return 0;
	}

	float d = optix::dot(triangle.normal, triangle.p0);
	float t = -(optix::dot(triangle.normal, ray.origin) + d) / NdotRay;

	if (t < 0)
	{
		////std::cout << "triangle is behind!" << std::endl;
		return 0;
	}

	optix::float3 P = ray.origin + ray.direction * t;
	optix::float3 C;  // vector perpendicular to triangle's plane

	optix::float3 edge0 = triangle.p1 - triangle.p0;
	optix::float3 vp0 = P - triangle.p0;
	C = optix::cross(edge0, vp0);
	if (optix::dot(triangle.normal, C) < 0)
	{
		////std::cout << "P is on the right side(1)!" << std::endl;
		return 0;
	}
	

	optix::float3 edge1 = triangle.p2 - triangle.p1;
	optix::float3 vp1 = P - triangle.p1;
	C = optix::cross(edge1, vp1);
	if (optix::dot(triangle.normal, C) < 0)
	{
		////std::cout << "P is on the right side(2)!" << std::endl;
		return 0;
	}
	
	optix::float3 edge2 = triangle.p0 - triangle.p2;
	optix::float3 vp2 = P - triangle.p2;
	C = optix::cross(edge2, vp2);
	if (optix::dot(triangle.normal, C) < 0)
	{
		////std::cout << "P is on the right side(3)!" << std::endl;
		return 0;
	}
	
	intersection_point = P;
	return 1;
}
int RAY::Intersect_Triangle_(Ray ray, Triangle triangle, optix::float3& intersection_point)
{
	const float e = 0.0000001;
	optix::float3 v0 = triangle.p0;
	optix::float3 v1 = triangle.p1;
	optix::float3 v2 = triangle.p2;
	optix::float3 edge1, edge2, h, s, q;
	float a, f, u, v;

	edge1 = v1 - v0;
	edge2 = v2 - v0;

	h = optix::cross(ray.direction, edge2);
	a = optix::dot(edge1, h);
	if (a > -e && a < e)
		return 0;

	f = 1.0 / a;
	s = ray.origin - v0;
	u = f * optix::dot(s, h);
	if (u < 0.0 || u > 1.0)
		return 0;

	q = optix::cross(s, edge1);
	v = f * optix::dot(ray.direction, q);
	if (v < 0.0 || u + v > 1.0)
		return 0;

	float t = f * optix::dot(edge2, q);
	if (t > e)
	{
		intersection_point = ray.origin + ray.direction * t;
		return 1;
	}
	
	return 0;
}
int RAY::Intersect_Parallelogram(Ray ray, Plane plane, optix::float3& intersection_point)
{
	Triangle tri_1 = SHAPES::createTriangle_(plane.p0, plane.p1, plane.p2);
	Triangle tri_2 = SHAPES::createTriangle_(plane.p1, plane.p3, plane.p2);

	int tri_isec;
	tri_isec = RAY::Intersect_Triangle_(ray, tri_1, intersection_point);
	if(tri_isec == 0)
		return RAY::Intersect_Triangle_(ray, tri_2, intersection_point);
	return tri_isec;
}
int RAY::Intersect_SphereArcs(Ray ray, optix::float3 center, float r, float real_r, optix::float3 * arc_axis, AXIS_ORIENTATION orientation, std::vector<IntersectionPoint>& isec_points)
{
	float t0, t1; // solutions for t if the ray intersects 

	//#define GEOMETRIC_SOLUTION
#ifdef GEOMETRIC_SOLUTION

	// geometric solution
	optix::float3 L = center - ray_o;
	float tca = optix::dot(L, ray_d);
	// if (tca < 0) return false;
	float d2 = optix::dot(L, L) - tca * tca;
	if (d2 > r) return false;
	float thc = sqrtf(r - d2);
	t0 = tca - thc;
	t1 = tca + thc;

#endif

#define ANALYTIC_SOLUTION
#ifdef ANALYTIC_SOLUTION
	// analytic solution
	optix::float3 L = ray.origin - center;
	float a = optix::dot(ray.direction, ray.direction);
	float b = 2 * optix::dot(ray.direction, L);
	float c = optix::dot(L, L) - r;
	if (!Geometry::MATH::solveQuadratic(a, b, c, t0, t1))
	{
		return false;
	}

#endif 

	if (t0 > t1) std::swap(t0, t1);

	if (t0 < 0)
	{
		t0 = t1; // if t0 is negative, let's use t1 instead 
		if (t0 < 0)
		{
			return false; // both t0 and t1 are negative 
		}
	}
	float t = t0;
	//


	// point of sphere in X arc //
	optix::float3 hit_point = ray.origin + ray.direction * t;
	optix::float3 normal = optix::normalize(hit_point - center);
	float dot_axis_x = optix::dot(normal, arc_axis[0]);
	float dot_axis_y = optix::dot(normal, arc_axis[1]);
	float dot_axis_z = optix::dot(normal, arc_axis[2]);


	optix::float3 W = normalize(Mediator::RequestCameraInstance().getW());
	float dot_plane_x = optix::dot(W, arc_axis[0]);
	float dot_plane_y = optix::dot(W, arc_axis[1]);
	float dot_plane_z = optix::dot(W, arc_axis[2]);


	float  e = 0.15;
	float _e = e * 3.2f;
	float plane_perpendicular_treshold_e = 0.9f;
	float e_x, e_y, e_z;
	int axis = -1;
	optix::float3 projected_point;
	
	IntersectionPoint isec_point;

	// camera W perpendicular to plane's normal
	if (fabs(dot_plane_x) > plane_perpendicular_treshold_e)
		e_x = _e;
	else
		e_x = e;
	if (fabs(dot_plane_y) > plane_perpendicular_treshold_e)
		e_y = _e;
	else
		e_y = e;
	if (fabs(dot_plane_z) > plane_perpendicular_treshold_e)
		e_z = _e;
	else
		e_z = e;


	bool is_screen = orientation == AXIS_ORIENTATION::SCREEN;

	if (fabsf(1.0f - fabs(dot_plane_x)) < 1e-4) e_x *= 4.0f;
	if (fabsf(1.0f - fabs(dot_plane_y)) < 1e-4) e_y *= 4.0f;
	if (fabsf(1.0f - fabs(dot_plane_z)) < 1e-4) e_z *= 4.0f;
	
	

	
	if (is_screen)
	{
		e_x *= 0.5f;
		e_y *= 0.5f;
		e_z *= 0.3f; // 4.0f 
	}

	if (fabs(dot_axis_x) < e_x)
	{
		axis = 1;
		if (orientation == AXIS_ORIENTATION::WORLD)
		{
			// project point on current (global) plane
			projected_point = GLOBAL_AXIS[axis - 1] * center + GLOBAL_INV_AXIS[axis - 1] * hit_point;
			normal = optix::normalize(projected_point - center);
			projected_point = center + normal * real_r;
			
		}
		// AXIS_ORIENTATION : LOCAL_AXIS
		if (orientation == AXIS_ORIENTATION::LOCAL)
		{
			// project point on current (local) plane
			optix::float3 plane_o = center;
			optix::float3 plane_n = optix::normalize(arc_axis[axis - 1]);
			Plane plane = SHAPES::createPlane(plane_o, plane_n);

			optix::float3 ray_o = hit_point;
			optix::float3 ray_d = optix::normalize(arc_axis[axis - 1]);
			Ray ray = SHAPES::createRay(ray_o, ray_d);

			Geometry::RAY::Intersect_Plane(ray, plane, projected_point);
			normal = optix::normalize(projected_point - center);
			projected_point = center + normal * real_r;
		}

		if (orientation == AXIS_ORIENTATION::SCREEN)
		{
			// project point on current (local) plane
			optix::float3 plane_o = center;
			optix::float3 plane_n = optix::normalize(arc_axis[axis - 1]);
			Plane plane = SHAPES::createPlane(plane_o, plane_n);

			optix::float3 ray_o = hit_point;
			optix::float3 ray_d = optix::normalize(arc_axis[axis - 1]);
			Ray ray = SHAPES::createRay(ray_o, ray_d);

			Geometry::RAY::Intersect_Plane(ray, plane, projected_point);
			normal = optix::normalize(projected_point - center);
			projected_point = center + normal * real_r;
		}

		isec_point.p = projected_point;
		isec_point.normal = normal;
		isec_point.plane_id = UtilityPlane::X;
		isec_points.push_back(isec_point);
	}
	if (fabs(dot_axis_y) < e_y)
	{
		axis = 2;
		if (orientation == AXIS_ORIENTATION::WORLD)
		{
			// project point on current (global) plane
			projected_point = GLOBAL_AXIS[axis - 1] * center + GLOBAL_INV_AXIS[axis - 1] * hit_point;
			normal = optix::normalize(projected_point - center);
			projected_point = center + normal * real_r;

		}
		// AXIS_ORIENTATION : LOCAL_AXIS
		if (orientation == AXIS_ORIENTATION::LOCAL)
		{
			// project point on current (local) plane
			optix::float3 plane_o = center;
			optix::float3 plane_n = optix::normalize(arc_axis[axis - 1]);
			Plane plane = SHAPES::createPlane(plane_o, plane_n);

			optix::float3 ray_o = hit_point;
			optix::float3 ray_d = optix::normalize(arc_axis[axis - 1]);
			Ray ray = SHAPES::createRay(ray_o, ray_d);

			Geometry::RAY::Intersect_Plane(ray, plane, projected_point);
			normal = optix::normalize(projected_point - center);
			projected_point = center + normal * real_r;
		}

		if (orientation == AXIS_ORIENTATION::SCREEN)
		{
			// project point on current (local) plane
			optix::float3 plane_o = center;
			optix::float3 plane_n = optix::normalize(arc_axis[axis - 1]);
			Plane plane = SHAPES::createPlane(plane_o, plane_n);

			optix::float3 ray_o = hit_point;
			optix::float3 ray_d = optix::normalize(arc_axis[axis - 1]);
			Ray ray = SHAPES::createRay(ray_o, ray_d);

			Geometry::RAY::Intersect_Plane(ray, plane, projected_point);
			normal = optix::normalize(projected_point - center);
			projected_point = center + normal * real_r;
		}
		isec_point.p = projected_point;
		isec_point.normal = normal;
		isec_point.plane_id = UtilityPlane::Y;
		isec_points.push_back(isec_point);
	}
	if (fabs(dot_axis_z) < e_z)
	{
		axis = 3;
		if (orientation == AXIS_ORIENTATION::WORLD)
		{
			// project point on current (global) plane
			projected_point = GLOBAL_AXIS[axis - 1] * center + GLOBAL_INV_AXIS[axis - 1] * hit_point;
			normal = optix::normalize(projected_point - center);
			projected_point = center + normal * real_r;

		}
		// AXIS_ORIENTATION : LOCAL_AXIS
		if (orientation == AXIS_ORIENTATION::LOCAL)
		{
			// project point on current (local) plane
			optix::float3 plane_o = center;
			optix::float3 plane_n = optix::normalize(arc_axis[axis - 1]);
			Plane plane = SHAPES::createPlane(plane_o, plane_n);

			optix::float3 ray_o = hit_point;
			optix::float3 ray_d = optix::normalize(arc_axis[axis - 1]);
			Ray ray = SHAPES::createRay(ray_o, ray_d);

			Geometry::RAY::Intersect_Plane(ray, plane, projected_point);
			normal = optix::normalize(projected_point - center);
			projected_point = center + normal * real_r;
		}

		if (orientation == AXIS_ORIENTATION::SCREEN)
		{
			// project point on current (local) plane
			optix::float3 plane_o = center;
			optix::float3 plane_n = optix::normalize(arc_axis[axis - 1]);
			Plane plane = SHAPES::createPlane(plane_o, plane_n);

			optix::float3 ray_o = hit_point;
			optix::float3 ray_d = optix::normalize(arc_axis[axis - 1]);
			Ray ray = SHAPES::createRay(ray_o, ray_d);

			Geometry::RAY::Intersect_Plane(ray, plane, projected_point);
			normal = optix::normalize(projected_point - center);
			projected_point = center + normal * real_r;
		}

		isec_point.p = projected_point;
		isec_point.normal = normal;
		isec_point.plane_id = UtilityPlane::Z;
		isec_points.push_back(isec_point);
	}

	return 1;
}
int RAY::Intersect_Sphere(Ray ray, optix::float3 center, float r, optix::float3& isec_p)
{
	float t0, t1; // solutions for t if the ray intersects 

//#define GEOMETRIC_SOLUTION
#ifdef GEOMETRIC_SOLUTION

	// geometric solution
	optix::float3 L = center - ray_o;
	float tca = optix::dot(L, ray_d);
	// if (tca < 0) return false;
	float d2 = optix::dot(L, L) - tca * tca;
	if (d2 > r) return false;
	float thc = sqrtf(r - d2);
	t0 = tca - thc;
	t1 = tca + thc;

#endif

#define ANALYTIC_SOLUTION
#ifdef ANALYTIC_SOLUTION
	// analytic solution
	
	optix::float3 L = ray.origin - center;
	float a         = optix::dot(ray.direction, ray.direction);
	float b         = 2 * optix::dot(ray.direction, L);
	float c         = optix::dot(L, L) - r;
	if (!Geometry::MATH::solveQuadratic(a, b, c, t0, t1))
	{
		////std::cout << "\n";
		////std::cout << " false:" << std::endl;
		////std::cout << "    - t0 : " << t0 <<std::endl;
		////std::cout << "    - t1 : " << t1 << std::endl;

		////std::cout << " 0 " << std::endl;
		return 0;
	}
	

#endif 

	if (t0 > t1) std::swap(t0, t1);

	if (t0 < 0)
	{
		t0 = t1; // if t0 is negative, let's use t1 instead 
		if (t0 < 0)
		{
			return -1; // both t0 and t1 are negative 
		}
	}

	float t = t0;
	isec_p = ray.origin + t * ray.direction;
	return 1;
}
int RAY::Intersect_Sphere2(Ray ray, optix::float3 center, float r, float& t)
{
	optix::float3 oc = ray.origin - center;
	float a = dot(ray.direction, ray.direction);
	float b = 2.0f * dot(oc, ray.direction);
	float c = dot(oc, oc) - r * r;
	float disc = b * b - 4 * a*c;
	t = (-b - sqrtf(disc)) / (2.0f*a);

	return disc >= 0;
}

int RAY::Intersect_Vector(Ray ray, optix::float3 p0, optix::float3 p1, optix::float3& isec_p)
{
	optix::float3 P0 = p0;
	optix::float3 v0 = p1 - p0;
	float a = length(v0);
	v0 = v0 * 1 / a;
	

	optix::float3 P1 = ray.origin;
	optix::float3 v1 = ray.direction;

	return 0;
}

// SHAPES
Sphere   SHAPES::createSphere(optix::float3 center, float r)
{
	Sphere sph;
	sph.center = center;
	sph.radius = r;
	return sph;
}
Circle   SHAPES::createCircle(optix::float3 center, float r, optix::float3 normal)
{
	Circle circ;
	circ.center = center;
	circ.radius = r;
	circ.normal = normal;
	return circ;
}
Plane    SHAPES::createPlane(optix::float3 p0, optix::float3 normal)
{
	Plane plane;
	plane.p0 = p0;
	plane.normal = normal;
	return plane;
}
Plane    SHAPES::createPlane(optix::float3 p0, optix::float3 u, optix::float3 v)
{
	Plane plane;
	plane.p0 = p0;
	plane.p1 = p0 + u;
	plane.p2 = p0 + v;
	plane.p3 = p0 + u + v;
	plane.u = u;
	plane.v = v;
	plane.normal = -optix::cross(u,v);

	return plane;
}
Plane    SHAPES::createPlane_Centered(optix::float3 p0, optix::float3 u, optix::float3 v, optix::Matrix4x4 transformation , bool transform ,  bool clipped )
{
	Plane plane;
	plane.p0 = p0 + u + v;
	plane.p1 = p0 - u + v;
	plane.p2 = p0 - u - v;
	plane.p3 = p0 + u - v;

	if (transform)
		plane.Transform(transformation);

	plane.u = normalize(plane.p1 - plane.p0);
	plane.v = normalize(plane.p2 - plane.p1);
	plane.normal = -optix::cross(plane.u, plane.v);

	optix::float3 min = plane[0];
	optix::float3 max = plane[0];
	if ( clipped )
	{
		for (int i = 1; i < 4; i++)
		{
			optix::float3 p = plane[i];
			float x = p.x;
			float y = p.y;
			float z = p.z;

			if (x < min.x)
				min.x = x;
			if (y < min.y)
				min.y = y;
			if (z < min.z)
				min.z = z;
			if (x > max.x)
				max.x = x;
			if (y > max.y)
				max.y = y;
			if (z > max.z)
				max.z = z;
		}

		plane.min = min;
		plane.max = max;
	}

	return plane;
}
Plane    SHAPES::createPlane_Centered_(optix::float3 p0, optix::float3 u, optix::float3 v)
{
	Plane plane;
	plane.p0 = p0 - u + v;
	plane.p1 = p0 + u + v;
	plane.p2 = p0 + u - v;
	plane.p3 = p0 - u - v;

	
	plane.u = normalize(u);
	plane.v = normalize(v);
	plane.normal = optix::cross(plane.u, plane.v);

	return plane;
}

Triangle SHAPES::createTriangle(optix::float3 p0, optix::float3 u, optix::float3 v)
{
	Triangle tri;
	tri.p0 = p0;
	tri.p1 = p0 + u;
	tri.p2 = p0 + v;
	tri.normal = optix::cross(u,v);
	return tri;
}
Triangle SHAPES::createTriangle_(optix::float3 p0, optix::float3 p1, optix::float3 p2)
{
	Triangle tri;
	tri.p0 = p0;
	tri.p1 = p1;
	tri.p2 = p2;
	tri.normal = optix::cross(p1 - p0, p2 - p0);
	return tri;
}
Ray      SHAPES::createRay(optix::float3 origin, optix::float3 direction)
{
	Ray ray;
	ray.origin = origin;
	ray.direction = direction;
	return ray;
}
Ray      SHAPES::createMouseRay()
{
	//CUSTOM_VIEWPORT vp = Mediator::getMainViewport();
	return SHAPES::createMouseRay(ViewportManager::GetInstance().getActiveViewport());

	optix::float2 size = Mediator::getRenderSize();

	int w = size.x; 
	int h = size.y;

	//optix::float2 mouse_pos = Mediator::Request_MousePosRelativeTo(VIEWPORT_ID::MAIN_VIEWPORT);
	optix::float2 mouse_pos = optix::make_float2(Mediator::RequestMousePos(0), Mediator::RequestMousePos(1));


	optix::float2 pixel    = optix::make_float2(mouse_pos.x, mouse_pos.y); //Mediator::RequestMousePos(0), Mediator::RequestMousePos(1));
	optix::float2 fragment = pixel + optix::make_float2(0.5f);
	optix::float2 screen   = optix::make_float2(  w, h );
	optix::float2 ndc      = (fragment / screen)*2.0f - 1.0f;
	optix::float3 dir      = optix::normalize(ndc.x * Mediator::RequestCameraInstance().getU() - ndc.y * Mediator::RequestCameraInstance().getV() + Mediator::RequestCameraInstance().getW());

	Ray ray = SHAPES::createRay(Mediator::RequestCameraInstance().getCameraPos(), dir);
	return ray;
}
Ray      SHAPES::createMouseRay(PinholeCamera camera_instance, const VIEWPORT& vp)
{
	optix::float2 size    = vp.renderSize;
	PinholeCamera camera = camera_instance;

	int w = size.x;
	int h = size.y;

	//optix::float2 mouse_pos = Mediator::Request_MousePosRelativeTo(VIEWPORT_ID::MAIN_VIEWPORT);
	optix::float2 mouse_pos = Utilities::getMousePosRelativeToViewport(vp.id);

	optix::float2 pixel    = optix::make_float2(mouse_pos.x, mouse_pos.y); //Mediator::RequestMousePos(0), Mediator::RequestMousePos(1));
	optix::float2 fragment = pixel + optix::make_float2(0.5f);
	optix::float2 screen   = optix::make_float2(w, h);
	optix::float2 ndc      = (fragment / screen)*2.0f - 1.0f;
	optix::float3 dir      = optix::normalize(ndc.x * camera.getU() - ndc.y * camera.getV() + camera.getW());

	Ray ray = SHAPES::createRay(camera.getCameraPos(), dir);
	return ray;
	
}
Ray      SHAPES::createMouseRay(float mouseX, float mouseY)
{
	optix::float2 pixel = optix::make_float2(mouseX, mouseY);
	optix::float2 fragment = pixel + optix::make_float2(0.5f);
	optix::float2 screen = optix::make_float2(Mediator::RequestWindowSize(0), Mediator::RequestWindowSize(1));
	optix::float2 ndc = (fragment / screen)*2.0f - 1.0f;
	optix::float3 dir = optix::normalize(ndc.x * Mediator::RequestCameraInstance().getU() - ndc.y * Mediator::RequestCameraInstance().getV() + Mediator::RequestCameraInstance().getW());

	Ray ray = SHAPES::createRay(Mediator::RequestCameraInstance().getCameraPos(), dir);
	return ray;
}
Ray      SHAPES::createMouseRay(glm::vec2 mouse)
{

	VIEWPORT vp = ViewportManager::GetInstance().getActiveViewport();
	PinholeCamera camera = ViewportManager::GetInstance().getViewportCamera(vp.id);
	optix::float2 pixel = optix::make_float2(mouse.x, mouse.y);
	optix::float2 fragment = pixel + optix::make_float2(0.5f);
	optix::float2 screen = optix::make_float2(vp.renderSize.x, vp.renderSize.y);
	optix::float2 ndc = (fragment / screen)*2.0f - 1.0f;
	optix::float3 dir = optix::normalize(ndc.x * camera.getU() - ndc.y * camera.getV() + camera.getW());

	Ray ray = SHAPES::createRay(camera.getCameraPos(), dir);
	return ray;
}
Ray      SHAPES::createMouseRay(glm::vec2 mouse, const VIEWPORT& vp)
{
	PinholeCamera camera   = ViewportManager::GetInstance().getViewportCamera(vp.id);
	optix::float2 pixel    = optix::make_float2(mouse.x, mouse.y);
	optix::float2 fragment = pixel + optix::make_float2(0.5f);
	optix::float2 screen   = optix::make_float2(vp.renderSize.x, vp.renderSize.y);
	optix::float2 ndc      = (fragment / screen)*2.0f - 1.0f;
	optix::float3 dir      = optix::normalize(ndc.x * camera.getU() - ndc.y * camera.getV() + camera.getW());

	Ray ray = SHAPES::createRay(camera.getCameraPos(), dir);
	return ray;
}
Ray      SHAPES::createMouseRay(const VIEWPORT& vp)
{
	optix::float2 size = vp.renderSize;
	PinholeCamera& camera = ViewportManager::GetInstance().getViewportCamera(vp.id);

	int w = size.x;
	int h = size.y;

	//optix::float2 mouse_pos = Mediator::Request_MousePosRelativeTo(VIEWPORT_ID::MAIN_VIEWPORT);
	optix::float2 mouse_pos = Utilities::getMousePosRelativeToViewport(vp.id);

	optix::float2 pixel = optix::make_float2(mouse_pos.x, mouse_pos.y); //Mediator::RequestMousePos(0), Mediator::RequestMousePos(1));
	optix::float2 fragment = pixel + optix::make_float2(0.5f);
	optix::float2 screen = optix::make_float2(w, h);
	optix::float2 ndc = (fragment / screen)*2.0f - 1.0f;
	optix::float3 dir = optix::normalize(ndc.x * camera.getU() - ndc.y * camera.getV() + camera.getW());

	Ray ray = SHAPES::createRay(camera.getCameraPos(), dir);
	return ray;
}

// 2D SHAPES
Rect3D SHAPES::createRect3D(optix::float3 a, optix::float3 b)
{
	Rect3D rect;
	rect.a = a;
	rect.b = b;
	return rect;
}
Rect2D SHAPES::createRect2D(optix::float2 a, optix::float2 b)
{
	Rect2D rect;
	rect.a = optix::make_float3(a.x,a.y,0.0f);
	rect.b = optix::make_float3(b.x, b.y, 0.0f);
	return rect;
}