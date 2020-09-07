#pragma once

#include <optixu/optixu_math_namespace.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SceneObject.h"
#include "Mediator.h"
#include "common_structs.h"
#include "PinholeCamera.h"

enum SCOPE
{
	TO_WORLD,
	TO_SCREEN,
	TO_VIEW,
	TO_LOCAL,
	TO_PARENT,
	TO_PICK
};



namespace BASE_TRANSFORM
{
	void AXIS(SceneObject * object, AXIS_ORIENTATION scope, optix::float3 * m_axis);
	void Coords(SceneObject * object, optix::float3& coords);
	void Position_Relative_To(SceneObject* object, optix::float3 P, optix::float3& pos);
	void Data_Relative_To(SceneObject* child, SceneObject* parent);
	optix::Matrix4x4 ONB(optix::float3 * base_axis);
	glm::mat4 ONB_glm(optix::float3 * base_axis);
	
	void Create_ONB(optix::float3 n, optix::float3 * axis);

	optix::float3    Translation_Relative_To(SceneObject* obj, SceneObject* trg);
	optix::Matrix4x4 TranslationMatrix_Relative_To(SceneObject* obj, SceneObject* trg);
	optix::Matrix4x4 RotationMatrix_Relative_To(SceneObject* obj, SceneObject* trg);
	void TransformationData_Relative_To(SceneObject* obj, SceneObject* trg, optix::float3& t, optix::Matrix4x4 & Tmat, optix::Matrix4x4& Rmat, optix::Matrix4x4& Smat);
	void TransformationData_Flat(SceneObject* obj, optix::float3& t, optix::Matrix4x4 & Tmat, optix::Matrix4x4& Rmat, optix::Matrix4x4& Smat);
}

namespace MATRIX_TRANSFORM
{

	bool ScaleAttribsGood(optix::Matrix4x4 S);
	void TRANSLATE(SceneObject * object, optix::float3 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);
	void TRANSLATE(SceneObject * object, optix::Matrix4x4 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);
	void TRANSLATE_old(SceneObject * object, optix::float3 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);

	void ROTATE(   SceneObject * object, optix::float3 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);
	void ROTATE(   SceneObject * object, optix::Matrix4x4 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);
	void ROTATE_old(SceneObject * object, optix::float3 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);

	void SCALE(    SceneObject * object, optix::float3 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);
	void SCALE_2(SceneObject * object, optix::float3 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);
	void SCALE(    SceneObject * object, optix::Matrix4x4 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);
	void SCALE_old(SceneObject * object, optix::float3 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0, optix::float3 * base_axis = 0, bool ignore_flag = false, int child_index = -1);


	void SCALE_RELATIVE_TO(SceneObject* object, optix::float3 delta, AXIS_ORIENTATION scope, TRANSFORMATION_SETTING setting, SceneObject* relative_to = 0);


	optix::Matrix4x4 Euler_Angles_To_Rotation_Matrix(optix::float3 euler_angles);
	optix::Matrix4x4 Euler_Angles_To_Rotation_Matrix(optix::float3 euler_angles, int order_ind);
	optix::Matrix4x4 Align_axis_Transformation_Get(optix::float3 * src_axis, optix::float3 * trg_axis, int axis_index, bool* ortho=0, bool* partial_ortho=0);

	
}



namespace VECTOR_TYPE_TRANSFORM
{
	glm::vec3 TO_GLM_3f(optix::float3 v);
	optix::float3 TO_OPTIX_3f(glm::vec3 v);
}

namespace MATRIX_TYPE_TRANSFORM
{
	glm::mat4 TO_GLM_MATRIX(optix::Matrix4x4 mat);
	optix::Matrix4x4 TO_OPTIX_MATRIX(glm::mat4 mat);
}



