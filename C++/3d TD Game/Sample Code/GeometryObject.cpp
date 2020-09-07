#include "GeometryObject.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

int GeometryObject::TERRAIN = 0;
int GeometryObject::TOWER = 1;

GeometryObject::GeometryObject(GeometryNode * node , int id)
{
	this->node = node;
	this->id = id;
}


GeometryObject::~GeometryObject()
{
	delete this->node;
}

GeometryNode * GeometryObject::getNode()
{
	return this->node;
}

glm::mat4  GeometryObject::TransformationMatrix()
{
	return this->transformation_matrix;
}

glm::mat4  GeometryObject::NormalMatrix()
{
	return this->normal_matrix;
}

void GeometryObject::setTransformationMatrix(glm::mat4 & transformation_matrix)
{
	this->transformation_matrix = transformation_matrix;
}

void GeometryObject::setNormalMatrix(glm::mat4 & normal_matrix)
{
	this->normal_matrix = normal_matrix;
}

int GeometryObject::getID()
{
	return this->id;
}


void GeometryObject::update()
{

	if (id == GeometryObject::TERRAIN)
	{

		transformation_matrix =
			glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0))
			* glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0, 0, 0))
			* glm::scale(glm::mat4(1.0f), glm::vec3(20));
		normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformation_matrix))));

	}
	else if (id == GeometryObject::TOWER)
	{

	}
}