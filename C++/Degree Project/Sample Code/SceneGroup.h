#pragma once
#include <optixu/optixu_matrix_namespace.h>
#include "common_structs.h"
#include "SceneObject.h"
#include <vector>

#define SCENE_GROUP_CONSTRUCTOR_FUNCTIONS
#define SCENE_GROUP_INIT_FUNCTIONS
#define SCENE_GROUP_UPDATE_FUNCTIONS
#define SCENE_GROUP_UTIL_FUNCTIONS
#define SCENE_GROUP_SET_FUNCTIONS
#define SCENE_GROUP_GET_FUNCTIONS

class SceneGroup
{

private:

	std::vector<SceneObject*> m_childs;
	std::vector<SceneObject*> top_level_childs;
	SceneObject*         dummy_parent;

	optix::float3 bbox_min, bbox_max;
	optix::float3 origin;
	
#ifdef SCENE_GROUP_CONSTRUCTOR_FUNCTIONS
public:

	SceneGroup();
	SceneGroup(std::vector< SceneObject * > childs);
	~SceneGroup();

#endif

#ifdef SCENE_GROUP_INIT_FUNCTIONS

	void Init();

#endif

#ifdef SCENE_GROUP_UPDATE_FUNCTIONS

public:

	void Update();

#endif

#ifdef SCENE_GROUP_SET_FUNCTIONS

public:
	
#endif

#ifdef SCENE_GROUP_GET_FUNCTIONS

public:

	SceneObject* getDummyParent();
	

#endif

#ifdef SCENE_GROUP_UTIL_FUNCTIONS
public:
	
#endif
	



	




};