#pragma once

#include "Defines.h"
#include "common_structs.h"
#include "Interface_Structs.h"
#include "Mediator.h"
#include "VertexAttributes.h"
#include "PPMLoader.h"
#include "HDRLoader.h"
#include "OptiXMesh.h"
#include "SceneObject.h"
#include "SceneGroup.h"
#include "ViewportManager.h"
#include "ViewportManager_structs.h"
#include "Action_Childs.h"
#include <fstream>

//#include "ActionManager_Structs.h"

typedef optix::float3 float3;

namespace Utilities 
{

	bool is_object_type_Muttable_To_Delete(SceneObject* object);
	bool is_object_type_Muttable_To_Transformation(SceneObject* object);
	bool is_object_type_SCALABLE(optix::float3 scale, AXIS_ORIENTATION axis_scope, SceneObject* object);
	bool is_object_type_SCALABLE(optix::float3 scale, optix::float3 * axis, AXIS_ORIENTATION axis_scope, SceneObject* object);
	
	std::string compress_string(std::string fi);
	std::string decompress_string(std::string fi);


	std::string decompress_file_toString(const char * filename);
	std::vector<std::string> decompress_file_toList(const char * filename);
	void decompress_file_toFile(const char* filename, const char* out_file);

    //std::string extract_FileName( std::string filepath );
	void export_1s(std::string id, std::string v, std::ofstream& fs);
	void export_1i(std::string id, int v, std::ofstream& fs);
	void export_1f(std::string id, float v, std::ofstream& fs);
	
	void export_2i(std::string id, optix::int2 v, std::ofstream& fs);
	void export_2f(std::string id, optix::float2 v, std::ofstream& fs);
	void export_3f(std::string id, optix::float3 v, std::ofstream& fs);

	void export_array_f(std::string id, float * d, int size, std::ofstream& fs);
	void export_array_i(std::string id, int * d, int size, std::ofstream& fs);

	void export_array_f(std::string id, std::vector<float> d, std::ofstream& fs);
	void export_array_i(std::string id, std::vector<int> d, std::ofstream& fs);

	static std::vector<Action_TRANSFORMATION> u_LOCAL_TEMP_USAGE_scale_actions_list;

	bool frustumCull(optix::float3 pos, const VIEWPORT& vp);

	std::vector<SceneObject*> get_Frustum_Objects(Rect2D rect, const VIEWPORT& viewport);
	std::vector<SceneObject*> get_Frustum_Objects(const VIEWPORT& viewport);
	bool                      get_Frustum_Objects_Mid_Distance(const VIEWPORT& viewport, float& mid_dist);
	
	struct GeometryObject_OpenGL* getOpenGL_Object_From_OptiXMesh(OptiXMesh* mesh);
    struct GeometryObject_OpenGL* getOpenGL_Object_From_OptiXMesh2(OptiXMesh* mesh);

	std::string getPtxPath(const char * filename);


	optix::Buffer createOutputBuffer(
		optix::Context context,
		RTformat format,
		unsigned width,
		unsigned height,
		bool use_pbo,
		RTbuffertype buffer_type);


	void displayBufferGL(optix::Buffer Buffer);

	// load texture JPEG function

	optix::TextureSampler loadTexture(optix::Context context, const std::string& filename, optix::float3 default_color);

	//
	void calculateCameraVariables(float3 eye, float3 lookat, float3 up,
								  float  fov, float  aspect_ratio,
								  float3& U, float3& V, float3& W, bool fov_is_vertical);


	optix::float3 toSphere(const optix::float2& v);

	//
	optix::Geometry createPlane(optix::Context& m_context ,  const int tessU, const int tessV, const int upAxis);
	optix::Geometry createSphere(const int tessU, const int tessV, const float radius, const float maxTheta);
	optix::Geometry createGeometry(optix::Context& m_context , std::vector<VertexAttributes> const& attributes, std::vector<unsigned int> const& indices);

	OptiXMesh LoadMesh(const char * filename, optix::Context& m_context, bool load_as_group);
	std::vector<OptiXMesh>& LoadMesh_As_Shapes(const char * filename, optix::Context& m_context);

	int getRand(int a, int b , int PI);
	
	Io_Mouse getMouseData();
	Io_Mouse getMouseData(Io_Mouse prev_mouse_data);
	void     getKeyData(Io_KeyBoard * io_keys, struct GLFWwindow * target_window);

	void transform_MousePos_Relative_To_Viewport(VIEWPORT viewport , int& x, int& y);
	optix::float2 getMousePosRelativeToViewport(VIEWPORT_ID id);
	optix::float2 getMousePosRelativeToViewport(VIEWPORT_ID id, const optix::float2& mouse);

	Utility convert_Utility_id_To_Utility(std::string utility_id);
	Utility_Group get_UtilityGroup(Utility ut);
	std::string Utility_Tooltip(Utility ut_id);
	std::string convert_Utility_To_Utility_id(Utility ut_id);
	CURSOR_TYPE Utility_to_Cursor_Type(UtilityProperties ut);
	
	
	void load_Default_Key_Preset(Io_KeyBoard * io_keys);


	void clone_utility_childs_Facevectors(SceneObject* from, SceneObject* to, bool adjust_pos = false , Object_Package* cloned_object_pack = 0);
	void clone_utility_childs_Samplers(SceneObject* from, SceneObject* to, bool adjust_pos = false, Object_Package* cloned_object_pack = 0);
	SceneObject * getInstanceOfObject(SceneObject * source_obj);
	void clone_Object(SceneObject * source);
	SceneObject* get_clone_Object(SceneObject * source, bool clone_facevectors = true, bool clone_samplers = true );
	SceneObject* get_clone_Object_By_Type(SceneObject * source);
	SceneObject* get_clone_group(SceneObject* group_parent, Multiple_Selection_Data& group_data);
	SceneObject* get_clone_group( SceneObject* group_parent );
	SceneObject* get_clone_group(std::vector<SceneObject*> group_childs);
	OptiXMesh getMeshInstance(SceneObject* source);
	OptiXMesh getMeshInstance2(SceneObject* source);
	OptiXMesh getMeshInstance_from_Object(SceneObject* source);
	OptiXMesh getMeshInstance_from_Mesh( OptiXMesh   mesh,  SceneObject* source = 0 );
	OptiXMesh getMeshInstance_from_Mesh( OptiXMesh * mesh , SceneObject* source = 0 );

	SceneObject* create_SceneGroup(std::vector<SceneObject*> childs);
	SceneObject* create_SceneGroupFrom(std::vector<SceneObject*> objects);
	void         ReEvaluate_SceneGroupFrom(SceneObject* group, std::vector<SceneObject*> objects);
	void         flat_Link_Group_With(SceneObject* parent, std::vector<SceneObject*>& group);

	optix::float3 * getGroupBBox(std::vector<SceneObject*> group);
	optix::float3 * getGroupTransformedBBox(std::vector<SceneObject*> group, bool first_time = false);
	optix::float3 * getGroupTransformedBBoxAndSummedOrigin(std::vector<SceneObject*> group, bool first_time = false);

	optix::float3 * getTreeBBOX(SceneObject * parent, bool transformed = false);

	int is_multiple_Linking_valid(Multiple_Selection_Data& data, SceneObject* parent);
	int is_Linking_valid(SceneObject* child, SceneObject* parent);
	bool is_Unlinking_valid(SceneObject* child);
	void object_force_Link(SceneObject* child, SceneObject* parent);
	void object_Link_Only_HTree(SceneObject* child, SceneObject* parent);
	void object_Link(SceneObject* child, SceneObject* parent);
	void object_Link(SceneObject* child, SceneObject* parent, Parent_Data parent_data, bool as_group = false);
	void object_Link_With_Top_Level_Hierarchy(SceneObject* parent, std::vector<SceneObject*>& group);
	bool object_unLink(SceneObject* object);
	bool object_unLink_children(SceneObject * object);

	void Update_Group_Origin_RelativeToGroupChilds(SceneObject* object);
	void Update_Selections_Group_Topology(Multiple_Selection_Data& data , bool only_BBOX = false);
	void Update_Upper_Groups_State(SceneObject* group, bool activate = false);
	void Update_Groups_State(SceneObject* group, bool activate = false);
	void Update_Upper_Groups_Hierarchy_remove( SceneObject* new_object, SceneObject* group, bool only_this = false, bool change_state = false );
	void Update_Upper_Groups_Hierarchy_insert( SceneObject* new_object, SceneObject* group, bool only_this = false, bool change_state = false );
	void Update_Upper_Groups_Hierarchy_remove(std::vector<SceneObject*> objects, SceneObject* group, bool only_this = false, bool change_state = false);
	void Update_Upper_Groups_Hierarchy_insert(std::vector<SceneObject*> objects, SceneObject* group, bool only_this = false, bool change_state = false);

	SceneObject* get_Closest_Upper_GroupParent(SceneObject* object);
	bool is_group_Covered(SceneObject* group, std::vector<SceneObject*>& selection);

	SceneObject* fetch_Clone_with_SourceId(int id, std::vector<SceneObject*> clones );

	void clone_Attributes(SceneObject* clone, SceneObject* source);

	void link_clones_based_on_sources(std::vector<SceneObject*>& clones, std::vector<SceneObject*>& sources);
	void link_clones_based_on_sources(Object_Package* cloned_object_pack, std::vector<SceneObject*>& clones, Multiple_Selection_Data& data , std::vector<SceneObject*>& groups_created, bool copy_fv = true, bool copy_samplers = true );
	void link_clones_based_on_sources2(Object_Package* cloned_object_pack, std::vector<SceneObject*>& clones, Multiple_Selection_Data& data, std::vector<SceneObject*>& groups_created,bool inc_upper = true, bool copy_fv = true, bool copy_samplers = true);

	void fill_Parent_Data(SceneObject* object, Parent_Data& parent_data);

	void Link_as_Group(SceneObject* group_parent, std::vector<SceneObject*>& group);
	std::vector<SceneObject*> get_top_level_objects(SceneObject* group_parent, std::vector<SceneObject*>& group, std::vector<SceneObject*>& top_level_grps, std::vector<SceneObject*>& top_subgroups);

	SceneObject* getSceneObject_Based_On_SourceId(int id);
	std::vector<SceneObject*> getGroupObjects(SceneObject* group_parent);
	std::vector<SceneObject*> getGroupObjects_Recursively(SceneObject* group_parent);
	std::vector<SceneObject*> getSubGroupObjects_Recursively(SceneObject* group_parent);
	
	std::vector<SceneObject*> get_LowerHierarchy(SceneObject* object, int max_level = 0, int type_inc = 0);
	std::vector<SceneObject*> get_UpperHierarchy(SceneObject* object, int max_level = 0, int type_inc = 0);
	
	bool isSelectionConvex(std::vector<SceneObject*>& group, std::vector<SceneObject*>& top_level_group_parents, std::vector<SceneObject*>& top_subgroups );
	bool is_object_in_List(SceneObject* obj, std::vector<SceneObject*> list);
	bool is_object_in_List(SceneObject* obj, std::vector<int> list);

	std::vector<SceneObject*> remove_Duplicates(std::vector<SceneObject*> objs);
	void EraseElement(std::vector<SceneObject*>& objs, SceneObject* obj);
	void EraseElement(std::vector<SceneObject*>& objs, int id);


	std::string getViewportNameFromID(VIEWPORT_ID id);

	void printVec(glm::vec3 * v, std::string n);
	void printVec(optix::float3 * v, std::string n);
	void printMatrix(float * data, int size, std::string n);
	void printMatrix_asMatrix(optix::Matrix4x4 m, std::string n);

	bool is_equal(optix::float3 a, optix::float3 b, float e = 0.0f);
	bool is_equal(optix::float4 a, optix::float4 b, float e = 0.0f);
	bool is_equal(optix::Matrix4x4 a, optix::Matrix4x4 b, float e = 0.0f);
	
	std::string erase_char_from_String(std::string str, std::vector<char> chars);
	std::string erase_char_from_String(std::string str, char c);
	std::vector<std::string> tokenize_String(std::string exp, std::string delim);
	
	float * tokenize_String_To_f(std::string exp, int *size = 0, std::string delim = ",");
	int   * tokenize_String_To_i(std::string exp, int *size = 0,std::string delim = ",");

	optix::float3 tokenize_String_To_3f(std::string exp, std::string delim =",", int size = 3);
	optix::int3 tokenize_String_To_3i(std::string exp, std::string delim = ",", int size = 3);

	optix::float2 tokenize_String_To_2f(std::string exp, std::string delim = ",", int size = 2);
	optix::int2 tokenize_String_To_2i(std::string exp, std::string delim = ",", int size = 2);

	std::string getFilePath(std::string fp);
	std::string getFileName(std::string filepath, bool ext = false );
	std::string getFileExt(std::string fp);
	std::string getTRS_setting_toString(TRANSFORMATION_SETTING setting);
	void setImportedObjectName(class SceneObject* obj, std::string filename);
	void setObjectName(class SceneObject* obj, std::string name, bool validate = false);
	void setObjectNameByObjectName(class SceneObject* obj, std::string name);
	void setObjectNameByObjectName(class SceneObject* obj, int source_id);
	int getMeshUsageCount(std::string mesh_id);

	void scale_Light_Objects_Relative_To_Viewport( const VIEWPORT& vp );
	void scale_Light_Objects_Relative_To_Viewport( const VIEWPORT& vp , std::vector<Action_TRANSFORMATION>& actions );
	void reset_Light_Objects_Scale();
	void reset_Light_Objects_Scale( std::vector<Action_TRANSFORMATION>& actions );


	std::string get_Available_Object_Name(class SceneObject* obj);
	bool is_Object_Name_Valid(std::string name);


	
	
};
