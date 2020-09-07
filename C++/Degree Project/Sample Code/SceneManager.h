#pragma once

#define SCENE_MANAGER

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <vector>
#include "Timer.h"
#include "light_definition.h"
#include "HierarchyTree.h"
#include <stack>
#include "common_structs.h"
#include "Interface_Structs.h"




#define EXPORT_MESH_DATA_INSIDE_SCENE_FILE
#define EXPORT_TEXMAPS_TO_SCENE_FILE
#define USE_ZLIB_COMPRESSION

#define e_line_TEXMAP_BEGIN "TEXMAP_BEGIN_"
#define e_line_TEXMAP_END "TEXMAP_END_"



//
#define SCENEMANAGER_CONSTRUCTORS
#define SCENEMANAGER_INIT_FUNCTIONS
#define SCENEMANAGER_UPDATE_FUNCTIONS
#define SCENEMANAGER_SET_FUNCTIONS
#define SCENEMANAGER_GET_FUNCTIONS
#define SCENEMANAGER_UTIL_FUNCTIONS



#define SCENE_GRID_MESH_PATH "../Data/Assets/plane.obj"
#define QUAD_LIGHT_FILEPATH "../Data/Assets/quad_light.obj"
#define SPHERICAL_LIGHT_FILEPATH "../Data/Assets/spherical_light.obj"
#define POINT_LIGHT_FILEPATH "../Data/Assets/point_light.obj"
#define SPOTLIGHT_LIGHT_FILEPATH "../Data/Assets/spotlight_light.obj"
#define FACEVECTOR_FILEPATH "../Data/Assets/face_vector2.obj"

#define SAMPLER_PLANAR2_FILEPATH "../Data/Assets/sampler_planar2.obj"

#define SAMPLER_VOLUME_FILEPATH "../Data/Assets/sampler_volume.obj"
#define SAMPLER_VOLUME2_FILEPATH "../Data/Assets/sampler_volume2.obj"

#define SAMPLER_DIRECTIONAL_FILEPATH "../Data/Assets/sampler_directional.obj"
#define SAMPLER_DIRECTIONAL2_FILEPATH "../Data/Assets/sampler_directional2.obj"



struct LightsPack
{
	std::vector<ObjectStruct> all;
	std::vector<ObjectStruct> quads;
	std::vector<ObjectStruct> spotlights;
	std::vector<ObjectStruct> sphericals;
	std::vector<ObjectStruct> pointlights;
};
struct SamplersPack
{
	std::vector<ObjectStruct> all;
	std::vector<ObjectStruct> planars;
	std::vector<ObjectStruct> volumes;
	std::vector<ObjectStruct> directionals;
};
struct ConstraintPack
{
	ObjectStruct from;
	ObjectStruct to;
	
	optix::float2 angle_from;
	optix::float2 angle_to;
	optix::float2 distance;

	ConstraintPack(ObjectStruct from, ObjectStruct to, optix::float2 a_from, optix::float2 a_to, optix::float2 d)
	:
		from(from),
		to(to),
		angle_from(a_from),
		angle_to(a_to),
		distance(d)
	{

	}
};
struct SceneConstraints_Pack
{
	std::vector<ConstraintPack> all;
	std::vector<ConstraintPack> focus;
	std::vector<ConstraintPack> conversation;
	std::vector<ConstraintPack> distance;
};
struct SceneData_Pack
{
	std::vector<ObjectStruct> objects;
	std::vector<ObjectStruct> groups;
	LightsPack lights;
	std::vector<ObjectStruct> faceVectors;
	SamplersPack samplers;

	SceneConstraints_Pack constraints;
};



class SceneManager
{

public:

	 //


	static SceneManager& GetInstance()
	{
		static SceneManager scnManager;
		return scnManager;
	}


#ifdef SCENEMANAGER_CONSTRUCTORS
public:
	SceneManager();
	~SceneManager();
#endif

#ifdef SCENEMANAGER_INIT_FUNCTIONS
public:
	bool                                                                      Init();
private:
	void                                                                      Init_GeneralData();
	void                                                                      Init_UtilityNodes();
	void                                                                      Init_Scene();
	void                                                                      Init_Helpers();
	void                                                                      createScene();
	void                                                                      Init_Light_Buffer();
#endif

#ifdef SCENEMANAGER_UPDATE_FUNCTIONS
public:
	void                                                                      Update(float dt);
	void                                                                      Update_SceneRestrictions_State(class SceneObject* obj = 0);
	void                                                                      Notify_Restrictions(class SceneObject* obj);
private:

#endif

#ifdef SCENEMANAGER_GET_FUNCTIONS
public:


	SceneParameters get_SceneParameters();
	void get_Scene_HierarchyTree_Depth_Flattened(Hierarchy_Tree * tree, unsigned int MAX_DEPTH = 2 , bool link_to_upper = true );
	std::vector<ObjectStruct> get_SceneHierarchyTree_Depth_Flattened_asList(unsigned int MAX_DEPTH = 2, bool link_to_upper = true);
	Hierarchy_Tree * Get_Scene_Hierarchy_Prunned(unsigned int MAX_DEPTH = 2);
	std::vector<ObjectStruct> Get_Scene_Hierarchy_Prunned_asList(unsigned int MAX_DEPTH = 2);
	SceneData_Pack Get_Scene_Hierarchy_Prunned_asPack(unsigned int MAX_DEPTH = 2);

	std::vector<class SceneObject *>&                                         getSceneObjects(bool utility_objects = true);
	std::vector<class SceneObject *>&                                         getSceneGroups();

	optix::Group&                                                             get_Utility_Samplers_node();
	optix::Group&                                                             get_Utility_FaceVectors_node();
	optix::Group&                                                             get_UtilityRootNode();
	optix::Group&                                                             getRootNode();
	optix::Group&                                                             getRootShadowNode();
	TreeNode*                                                                 getHierarchyTreeRoot();
	Hierarchy_Tree*                                                           getHierarchyTreeHandle();

	int                                                                       getFocusedObjectIndex();
	class SceneObject*                                                        getSelectedObject();
	int                                                                       getSelectedObjectIndex();
	int                                                                       getObjectsCount();

	class SceneObject*                                                        getFocusedGroup();
	int                                                                       getFocusedGroupIndex();
	class SceneObject*                                                        getSelectedGroup();
	int                                                                       getSelectedGroupIndex();
	bool                                                                      isSceneDirty();

	std::vector<class Restriction*>                                           get_Scene_Objects_Restrictions();
	class Restriction*                                                        get_Restriction(int id, bool by_id = false);

	bool                                                                      checkIntegrity();


	void get_Scene_Import_State(int& state, std::string& msg);

#endif

#ifdef SCENEMANAGER_SET_FUNCTIONS
public:

	void                                                                      setSceneParameters(SceneParameters params);
	void                                                                      setSelectedObject(int index , bool by_id = false);
	void                                                                      setFocusedObject(int index);

	void                                                                      setSelectedGroup(int index);
	void                                                                      setFocusedGroup(int index);

	void                                                                      setSceneDirty(bool state);


#endif

#ifdef SCENEMANAGER_UTIL_FUNCTIONS
private:
	void                                                                      cleanUp();

public:
	void clear(bool state = true);

	void                                                                      Export_Scene(const char * path, const char * filename);
	void                                                                      Export_Scene_as_JSON(const char* path, const char* filename);
	void                                                                      Export_Scene_as_JSON2(const char* path, const char* filename);
	void                                                                      Import_Scene(const char* file);

	void                                                                      Reset_Restrictions_UI_states(bool only_focus = false);
	void                                                                      add_Restriction(class Restriction * R);
	void                                                                      Remove_Restriction(int id , bool by_id = false);
	
	void                                                                      exp_addSceneObject(const char * filepath1);
	void                                                                      addSceneObject(const char * filepath);
	void                                                                      addSceneObject( SceneObject* object , const char * filepath );
	void                                                                      addSceneObject(const char * filepath, optix::float3 translation, optix::float3 scale, Type type = IMPORTED_OBJECT);
	void                                                                      addSceneObject(const char * filepath, optix::float3 translation, optix::float3 scale, class SceneObject* handle_obj, Type type = IMPORTED_OBJECT);
	void                                                                      addSceneObject(class SceneObject * object);
	void                                                                      addSceneObject(const char* filepath, optix::float3 translation, optix::float3 rotation, optix::float3 scale,
																							 struct MaterialParameters mat_params, bool app_obj = false);

	void                                                                      addSceneLight(Light_Type type, optix::float3 p, optix::float3 * basis, bool link = false, SAMPLER_ALIGNEMENT sa = SA_GLOBAL,  bool save_ac = true);
	void                                                                      addSceneLight( Light_Type type , bool save_ac = true );
	void                                                                      attachSamplerTo( SceneObject* object, Sampler_Type type);
	void                                                                      addSceneLight_To_Buffer( Light_Parameters light );
	void                                                                      removeSceneLight_From_Buffer( int index );
	void                                                                      Update_SceneLight(Light_Parameters light, int index);

	SceneObject*                                                              create_FaceVector( optix::float3 pos, optix::float3 scale, bool save_action = true, class SceneObject* parent = 0 );
	SceneObject*                                                              create_Sampler(optix::float3 trns, optix::float3 scale, Sampler_Type s_type, bool save_action = true, SceneObject* parent = 0);


private:
	optix::Acceleration                                                       create_Acceleration(std::string type);
public:
	void                                                                      create_SceneGroup(std::vector< class SceneObject * > group_childs);
	void                                                                      create_SceneGroup(std::vector< int > group_childs);
	void                                                                      remove_SceneGroup(int group_index);
	void                                                                      removeObject(class SceneObject* object);
	void                                                                      removeObject(int object_index);
private:
	void                                                                      execute_remove_calls();
	void                                                                      resetFocusedObject();

public:

	void                                                                      force_execute_remove_calls();
	bool                                                                      getInitState();
	void                                                                      printInitError();

public:
	void                                                                      clearSceneObjectGuiFlags();
	void                                                                      toggle_Only_Bbox_Scene_Render(bool state);
	void                                                                      toggleSceneRenderPass(bool state);
	void                                                                      toggleUtilityNodes(bool state);
	void                                                                      toggleScene_grid_RenderPass(bool state);
	void                                                                      toggle_Light_Objects(bool state, int r_q, int r_spot, int r_s, int r_p);

	void                                                                      toggle_Utility_Root_Node_State(bool state);
	void                                                                      toggle_FaceVector_State(bool state , bool only_face_vector = false , bool scale = false);
	void                                                                      toggle_Samplers_State(bool state, int is_samplers,  bool only_face_vector = false, bool scale = false);
	void                                                                      pre_process_toggle_Samplers_State(int is_samplers);
	void                                                                      post_process_toggle_Samplers_State(int is_samplers);
	void                                                                      pre_process_toggle_FaceVectors_State(int is_fv);
	void                                                                      post_process_toggle_FaceVectors_State(int is_fv);

	void                                                                      scale_Light(SceneObject* light, int vp_id, bool state);
	void                                                                      scale_Lights(bool state);
	void                                                                      scale_FaceVectors(bool state);
	void                                                                      scale_Directional_Samplers(bool state);

	bool                                                                      isDirty();
	bool                                                                      isDirty(bool *prev_state, bool reset = true );
	bool                                                                      isDirty(bool reset);


	void                                                                      markDirty();

#endif
	
	

	
private:

	SceneParameters m_scene_params;

	// init flags
	bool                                                                      m_utility_init;
	bool                                                                      m_scene_init;
	// Optix Variables
	optix::Context                                                            m_context;
	
	bool                                                                      use_tri_api;
	bool                                                                      ignore_mats;

	//
	optix::Group                                                              m_root_node;
	optix::Group                                                              m_root_top_Shadower;
	optix::Group                                                              m_scene_grid_root_node;
	optix::Group                                                              m_root_Utility_node;
	optix::Group                                                              m_root_Utility_Samplers_node;
	optix::Group                                                              m_root_Utility_FaceVectors_node;
	
	bool                                                                      m_scene_isDirty;

	Hierarchy_Tree*                                                           m_scene_Hierarchy_Tree;
	std::vector<class SceneObject*>                                           m_scene_objects;
	std::vector<class Restriction*>                                           m_object_restrictions;
	std::vector<class SceneObject*>                                           m_scene_groups;
	std::vector<class SceneObject*>                                           m_scene_objects_remove_stack;
	std::vector<LightDefinition>                                              m_lightDefinitions;
	optix::Buffer                                                             m_bufferLightDefinitions;

	class SceneObject*                                                        m_scene_grid;

	optix::Transform                                                          m_translation_node;
	optix::Transform                                                          m_rotation_node;
	optix::Transform                                                          m_scale_node;
	optix::Transform                                                          m_bounding_box_node;

	
	std::vector<SceneObject*> m_temp_added_Samplers;
	std::vector<SceneObject*> m_temp_added_FaceVectors;
	std::vector<SceneObject*> m_temp_added_Lights;
	

	int                                                                       m_focused_object_index;
	int                                                                       m_selected_object_index;

	int                                                                       m_focused_group_index;
	int                                                                       m_selected_group_index;

	Timer                                                                     m_timer;


	bool m_is_dirty;
	bool m_was_dirty;


	int m_Scene_Import_STATE       = -1;
	std::string m_Scene_Import_MSG = "";
};





