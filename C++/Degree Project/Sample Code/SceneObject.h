#pragma once

#include <optixu/optixu_matrix_namespace.h>
#include "OptiXMesh.h"
#include "common_structs.h"
#include "Grouping_Data.h"
#include "MaterialParameters.h"
#include "Defines.h"
#include <string.h>

#include <fstream>
#include <unordered_map>

#define SCENEOBJECT_CONSTRUCTORS
#define SCENEOBJECT_INIT_FUNCTIONS
#define SCENEOBJECT_UPDATE_FUNCTIONS
#define SCENEOBJECT_UTIL_FUNCTIONS
#define SCENEOBJECT_SET_FUNCTIONS
#define SCENEOBJECT_GET_FUNCTIONS
#define SCENEOBJECT_GROUP_FUNCTIONS

enum TRANSFORMATION_TYPE
{
	TRANSLATION,
	ROTATION,
	SCALE
};

enum TRANSFORMATION_SETTING
{
	TO_CHILDS,
	ONLY_TO_CHILDS,
	ONLY_THIS,
	TO_ALL
};

enum Type
{
	APP_OBJECT,
	IMPORTED_OBJECT,
	DUMMY,
	GROUP_PARENT_DUMMY,
	LIGHT_OBJECT,
	SAMPLER_OBJECT,
	UTILITY_OBJECT,
	FACE_VECTOR
};

struct TO_LINK_DATA
{
	int parent_id = -1;
	int grp_parent_id = -1;
	std::vector<int> child_ids;
	std::vector<int> grp_child_ids;
	std::vector<int> fv_ids;
	std::vector<int> smplr_ids;

	void clear()
	{
		parent_id = -1;
		grp_parent_id = -1;
		child_ids.clear();
		grp_child_ids.clear();
		fv_ids.clear();
		smplr_ids.clear();
	}
};

struct GeometryProperties
{
	std::string mesh_filename;
	int num_triangles;
	std::vector<std::string> mat_names;
	std::vector<int> mat_has_tex;
	optix::float3 bbox_min;
	optix::float3 bbox_max;
	float avg_geometry_spread;

	GeometryProperties()
		:  mesh_filename(""),num_triangles(0), bbox_min(ZERO_3f), bbox_max(ZERO_3f), avg_geometry_spread(0.0f)
	{

	}

};


class SceneObject
{
	
	static int objects_count;

#ifdef SCENEOBJECT_CONSTRUCTORS

public:
	SceneObject();
	SceneObject(SceneObject * source);
	SceneObject(int type);
	SceneObject(OptiXMesh * mesh);
	~SceneObject();
	void cleanUp();


	static void SceneObject::reset_Counter() { objects_count = 0; }

#endif

#ifdef SCENEOBJECT_INIT_FUNCTIONS
public:
	void                                                Init();
	void                                                Init(SceneObject * source);
	void                                                Init(OptiXMesh * mesh);
	void                                                Init_App_Material_Params();
private:

	void                                                Init_geometryProperties(SceneObject * source);
	void                                                Init_geometryProperties();
	void                                                InitMaterial(std::string& mat_name, optix::Material* mat);
	
	void                                                Init_Light_Parameters();

	void                                                Init_Bbox_Mesh();

#endif

#ifdef SCENEOBJECT_UPDATE_FUNCTIONS
private:
	void Update_world_position();
public:
	void                                                UpdateRelativePositionTo(SceneObject * object, optix::Matrix4x4& rot);

	void                                                UpdateTransformationMatrix(float * data = 0, bool to_childs = true);
	void                                                UpdateTransformationMatrix(AXIS_ORIENTATION axis_orientation, float * data = 0);
	void                                                Update_Childs_Transformation_Matrix();
	void                                                Update_Utility_Childs_Transformation_Matrix();
	void                                                UpdateTransformNode(float * data = 0);
	void                                                UpdateMaterial(MaterialParameters mat_params, int index);
	void                                                Transform_Light_Parameters();
#endif

#ifdef SCENEOBJECT_SET_FUNCTIONS
public:

	void                                                setImmutable(bool state);
	void                                                set_LightButtonHovered(bool state);

	void                                                keepUtilityChilds(bool state);
	
	void                                                setAttribute(optix::float3 a, int index);
	void                                                setRelativeTranslationAttributes(optix::float3 t);

	void                                                setTranslationMatrix(float * data, bool update = true);
	void                                                setRotationMatrix(float * data, bool update = true);
	void                                                setScaleMatrix(float * data, bool update = true);

	void                                                set_local_parent_rot_matrix(float * data, bool update = true , bool set = true);
	void                                                set_local_parent_rot_matrix_pos(float * data, bool update = true);
	void                                                set_local_parent_scale_matrix(float * data, bool update = true);

	void                                                setTransformNode(optix::Transform trns_node);


	void                                                setTransformation(optix::float3 d, 
																		  AXIS_ORIENTATION axis, 
																		  TRANSFORMATION_TYPE type, 
																		  TRANSFORMATION_SETTING setting = TO_ALL, 
																		  SceneObject* relative_to = 0, 
																		  optix::float3 * base_axis = 0, 
																		  bool update_topology = true,
																		  bool ignore_flag = false,
																		  int  child_index = -1);
	void                                                setTransformation(optix::Matrix4x4 mat,
																		  AXIS_ORIENTATION axis,
																		  TRANSFORMATION_TYPE type,
																		  TRANSFORMATION_SETTING setting = TO_ALL,
																		  SceneObject* relative_to  = 0,
																		  optix::float3 * base_axis = 0,
																		  bool update_topology = true,
																		  bool ignore_flag = false,
																		  int  child_index = -1);

	void                                                setTranslation(   optix::float3 d, 
																	      AXIS_ORIENTATION axis = WORLD,
																	      TRANSFORMATION_SETTING setting = TO_ALL,
																	      SceneObject* relative_to = 0,
																	      optix::float3 * base_axis = 0,
																	      bool ignore_flag = false,
																	      int  child_index = -1);


	void                                                setRotation(      optix::float3 d, 
																	      AXIS_ORIENTATION axis = WORLD,
																	      TRANSFORMATION_SETTING setting = TO_ALL,
																	      SceneObject* relative_to = 0,
																	      optix::float3 * base_axis = 0,
																	      bool ignore_flag = false,
																	      int  child_index = -1);


	void                                                setScale(         optix::float3 d, 
																          AXIS_ORIENTATION axis = WORLD,
																          TRANSFORMATION_SETTING setting = TO_ALL,
																          SceneObject* relative_to = 0,
																          optix::float3 * base_axis = 0,
																          bool ignore_flag = false,
																          int  child_index = -1);


	void                                                setTranslationAttributes(const optix::float3& v);

	void                                                setParentRotMatrix(float * data);
	void                                                setAttributesDelta(int index, optix::float3 dt);

	void                                                setName(std::string name);
	void                                                setNamePostfix(std::string postfix);
	void                                                setBbox(const optix::float3& min, const optix::float3& max);

	void                                                set_Group_Updating_Policy(bool state);

	void                                                setFocused(bool state);
	void                                                setSelected(bool state);
	void                                                setRenderingState(bool state, bool s = true);
	void                                                setActive(bool state, bool update_visibility = true);
	void                                                setVisible(bool state, bool update_visibility = true);
	void                                                setTemporary(bool state);
	void                                                markDirty();

	void                                                setSourceId(int id);
	void                                                set_MeshId(std::string mesh_id);
	void                                                set_as_Clone(bool state);
	void                                                set_as_Temp_Clone(bool state);
	void                                                push_Restriction(class Restriction* r);
	void                                                set_ui_TreeNode_Open(bool state);

	void                                                set_Object_Type_To(Type type_id);
	void                                                set_LightParameters(Light_Parameters params, bool transform = false);
	void                                                set_SamplerParameters(Sampler_Parameters params);
	void                                                set_FaceVectorParamaters(FaceVector_Parameters params);
	void                                                set_Notify_Scene_State(bool state);

	void                                                add_FaceVector(SceneObject* fv);
	void                                                add_Cloned_FaceVector(SceneObject* fv);
	void                                                attach_FaceVector(SceneObject* fv);
	SceneObject*                                        attach_FaceVector(optix::float3 pos, optix::float3 dir, bool save_ac);
	void                                                attach_FaceVector(optix::float3 pos, optix::float3 dir, int plane_id);
	void                                                remove_FaceVector(int index, bool by_id = false);
	void                                                remove_FaceVectors();
	
	void                                                add_Sampler(SceneObject* smplr);
	void                                                add_Cloned_Sampler(SceneObject* smplr);
	void                                                attach_Sampler(SceneObject* s);
	void                                                attach_Sampler(Sampler_Type s_type);
	void                                                attach_Sampler(Sampler_Parameters params);
	void                                                remove_Sampler(int index, bool by_id = false);
	void                                                remove_Samplers();
	void                                                remove_Utility_Childs();

	void                                                set_Highlight_Color(optix::float3 color);
	void                                                set_to_link_Data(TO_LINK_DATA data);

#endif

#ifdef SCENEOBJECT_GET_FUNCTIONS
public:

	bool                                                isImmutable();
	bool                                                is_LightButtonHovered();

	bool                                                iskeepUtilityChilds();
	int                                                 getMaterialCount();
	optix::Material                                     getMaterial(int index);
	MaterialParameters                                  getMaterialParams(int index);
	MaterialParameters                                  getMaterialParams(std::string mat_name);
	GeometryProperties                                  getGeomProperties();

	optix::float3                                       getTranslationAttributes(const char * caller = 0);
	optix::float3                                       getRelativeTranslationAttributes();
	optix::float3                                       getScaleAttributes();
	optix::float3                                       getRotationAttributes();
	optix::float3                                       getAttributesDelta(int index);
	optix::float3                                       getAttributes(int index);

	optix::Matrix4x4&                                   getTranslationMatrix();
	optix::Matrix4x4                                    getTranslationMatrix_value();
	optix::Matrix4x4&                                   get_World_Translation_Matrix();
	optix::Matrix4x4                                    get_World_Translation_Matrix_value();
	optix::Matrix4x4&                                   getScaleMatrix();
	optix::Matrix4x4                                    getScaleMatrix_value();
	optix::Matrix4x4                                    getAbsoluteScaleMatrix();
	optix::Matrix4x4&                                   getRotationMatrix();
	optix::Matrix4x4                                    getRotationMatrix_value();
	optix::Matrix4x4                                    getRotationMatrix_chain();
	optix::Matrix4x4                                    getTransformationMatrix();
	optix::Matrix4x4                                    getTransformationMatrix_Updated();
	optix::float3                                       get_World_Position_Updated();

	optix::float3                                       get_absolute_position();
	void                                                fill_Attributes_Relative_To(SceneObject* object);
	optix::float3                                       get_position_relative_To(SceneObject * object);
	optix::float3                                       get_position_relative_To(optix::float3& world_pos, SceneObject * object);
	optix::float3                                       get_Updated_World_Position(optix::Matrix4x4& parent_local_rot_pos);

	optix::Matrix4x4&                                   get_local_parent_translation_matrix();
	optix::Matrix4x4&                                   get_local_parent_rot_matrix();
	optix::Matrix4x4                                    get_local_parent_rot_matrix_value();
	optix::Matrix4x4&                                   get_local_parent_rot_matrix_pos();
	optix::Matrix4x4                                    get_local_parent_rot_matrix_pos_value();
	optix::Matrix4x4                                    get_parent_rot_matrix_as_value();
	optix::Matrix4x4                                    get_local_parent_rot_matrix_chain();
	optix::Matrix4x4                                    get_local_parent_rot_mat_offset();
	optix::Matrix4x4                                    get_local_parent_rot_mat_offset_inv();
	optix::Matrix4x4&                                   get_local_parent_scale_matrix();
	optix::Matrix4x4                                    get_local_parent_scale_matrix_value();
	optix::Matrix4x4                                    get_local_parent_scale_matrix_chain(bool is_parent = true,bool print_info = false);

	optix::Matrix4x4                                    get_centroid_offset_mat( bool transformed = false );
	optix::Matrix4x4                                    get_centroid_offset_mat_relativeTo( SceneObject* obj , bool transformed = false );
	optix::Matrix4x4                                    get_centroid_offset_mat_relativeTo( optix::float3 origin , bool transformed = false);

	optix::Transform                                    getTransformNode();
	optix::GeometryInstance                             getGeometryInstance(); // prepei na fugei to pointer....
	OptiXMesh*                                          getOptiXMesh();
	int                                                 getId();
	std::string                                         getId_toString();
	int                                                 getSourceId();
	Type                                                getType();

	static int                                          getObjectsCount();

	float                                               getDistanceFromCamera();

	bool                                                isTempClone();
	bool                                                isFocused();
	bool                                                isSelected();
	bool                                                isGuiOpen();
	bool                                                isRotationChanged();
	bool                                                isActive();
	bool                                                isVisible();
	bool                                                isTemporary();
	bool                                                getRenderingState();
	bool                                                isDirty(bool reset = true);
	bool                                                is_ui_TreeNode_Open();

	bool                                                MaterialHasTex(int mat_ind);
	bool                                                is_Material_Valid(std::string key);
	float                                               getGeometryAverageSpread();
	const optix::float3&                                getBBoxMin();
	const optix::float3&                                getBBoxMax();
	optix::float3*                                      getBBoxPoints();
	optix::float3*                                      getBBOX(bool transformed = false);
	optix::float3*                                      getBBOX_Transformed( bool transformed_minMax = true );
	optix::float3                                       getCentroid();
	optix::float3                                       getCentroid_Transformed( bool orbiting_centroid = false );                                                                                     
	std::string                                         getName( bool with_postfix = true );
	std::string                                         getNameWithID();
	std::string                                         getNamePostfix();

	bool                                                checkIntegrity();

	class Restriction*                                  getRestriction(int index);
	std::vector<class Restriction*>                     getRestrictions();
	std::string                                         get_MeshId();
	bool                                                is_Clone();

	Light_Parameters                                    get_LightParameters();
	Light_Parameters&                                   get_LightParameters_Raw();
	FaceVector_Parameters                               get_FaceVectorParameters();
	FaceVector_Parameters&                              get_FaceVectorParameters_Raw();
	Sampler_Parameters                                  get_SamplerParameters();
	Sampler_Parameters&                                 get_SamplerParameters_Raw();

	SceneObject*                                        get_Face_Vector(int index, bool by_id = false);
	std::vector<SceneObject*>&                          get_Face_Vectors();
	std::vector<SceneObject*>                           get_Face_Vectors_Active();

	SceneObject*                                        get_Sampler(int index, bool by_id = false);
	std::vector<SceneObject*>&                          get_Samplers();

	optix::float3                                       get_Highlight_Color();

	TO_LINK_DATA                                        get_to_link_Data();
	TO_LINK_DATA&                                       get_to_link_Data_Raw();

	std::vector<SceneObject*> get_Cloned_Utility_Childs();
	std::vector<SceneObject*>& get_Cloned_FaceVectors();
	std::vector<SceneObject*>& get_Cloned_Samplers();

	

#endif

#ifdef SCENEOBJECT_UTIL_FUNCTIONS
public:

	void                                                clear_to_Link_Data();
	void                                                process_to_Link_Data();
	void                                                Export( std::ofstream& of );
	void                                                Export_JSON(std::ofstream& of, struct Hierarchy_Tree* export_tree);

	void                                                reset_Data();
	void                                                guiSwitch();
	void                                                clearGuiFlags();

	void                                                remove_Restriction( int index , bool del = false , bool by_id = false );

	void                                                reset_Attribute(int index);
	void                                                reset_Attributes();
	void                                                reset_Matrices();
	void                                                reset_Matrix(int index);

	void                                                clamp_Rotation_Attributes();
	
	void clone_Utility_Childs();
	void clone_FaceVectors();
	void clone_Samplers();
	void clear_Cloned_Utility_Childs(bool rm = false);
	void clear_Cloned_FaceVectors(bool rm = false);
	void clear_Cloned_Samplers(bool rm = false);

	void destroy();
	void clear();

#endif

#ifdef SCENEOBJECT_GROUP_FUNCTIONS

public:
	void setParent_Test_(SceneObject* obj, bool do_update);
	void setParent_Test(SceneObject* obj);
	void setParent_Test(SceneObject* obj, Parent_Data parent_data, bool as_group = false);
	void setParent(SceneObject * obj);
	void setParent_(SceneObject * obj, bool do_update);
	void setParent(SceneObject* obj, Parent_Data parent_data, bool as_group = false);
	void addChild(SceneObject * obj);
	void addGroupChild(SceneObject* obj);
	void addGroupSelectionChild(SceneObject* obj);
	void removeChild(int index);
	void removeGroupChild(int index);
	void removeGroupSelectionChild(int index);
	void removeParent();
	SceneObject* getParent();
	bool         hasParent();
	std::vector<SceneObject*>& getChilds();
	std::vector<SceneObject*>  getUtilityChilds(bool only_active = false);
	std::vector<SceneObject*>  getChilds_EveryType();
	std::vector<SceneObject*>& getGroupChilds();
	std::vector<SceneObject*> getGroupSelectionChilds();
	std::vector<SceneObject*>& getGroupSelectionChilds_and_Groups();
	std::vector<SceneObject*>& getGroupChilds_Recursively();
	SceneObject* getChild(int index);
	SceneObject* getGroupChild(int index);

	// grouping info //

	void set_Grouping_Data(std::vector<SceneObject*> top_level_objects,
						   std::vector<SceneObject*> top_level_groups,
						   std::vector<SceneObject*> group_selection);

	Grouping_Data& get_Grouping_Data();

	
	//

	SceneObject* getRealGroupParent();
	SceneObject* getGroupParent();
	void         setRealGroupParent(SceneObject* obj);
	void         setGroupParent(SceneObject* obj , bool set = true);

	void         Update_BBOX_relativeToGroupChilds();
	void         Update_Position_relativeToGroupChilds();
	void         Update_BBOX_and_Position_relativeToGroupChilds(bool only_bbox = false, bool force = false);
	void         Update_Average_Geometry_Spread_RelativeToGroupChilds();
	void         Update_Average_Geometry_Spread_RelativeTo(std::vector<SceneObject*> objects);
	void         Update_Group_Topology(bool only_bbox = false, bool force = false);
	void         Update_Group_State();
#endif

													
private:

	TO_LINK_DATA                                        m_to_link_data;

	int                                                 id;
	int                                                 source_id = -1;
	Type                                                type;
	std::string                                         name;
	std::string                                         name_postfix = "";

	struct OptiXMesh *                                  g_mesh;
	optix::GeometryInstance                             g_inst;
	GeometryProperties                                  g_inst_properties;

	optix::float3                                       absolute_translation_attributes;
	optix::float3                                       translation_attributes;
	optix::float3                                       relative_translation_attributes;
	optix::float3                                       scale_attributes;
	optix::float3                                       rotation_attributes;
	optix::Matrix4x4                                    rot_mat[3];

	optix::float3                                       attributes_delta[3];

	optix::Matrix4x4                                    translation_matrix;
	optix::Matrix4x4                                    scale_matrix;
	optix::Matrix4x4                                    rotation_matrix;

	optix::Matrix4x4                                    parent_translation_matrix;
	optix::Matrix4x4                                    parent_rot_matrix;
	optix::Matrix4x4                                    parent_rot_matrix_pos;
	optix::Matrix4x4                                    parent_rot_matrix_chain;
	optix::Matrix4x4                                    parent_scale_matrix;
	optix::Matrix4x4                                    parent_scale_matrix_chain;

	optix::Matrix4x4                                    parent_rot_mat_offset;
	optix::Matrix4x4                                    parent_rot_mat_offset_inv;

	bool parent_rot_matrix_is_dirty;
	bool parent_scale_matrix_is_dirty;

	optix::Matrix4x4                                    transformation_matrix;
	optix::Transform                                    transform_node;

	
	bool                                                gui_open;
	bool                                                is_focused;
	bool                                                is_selected;
	bool                                                m_rotation_changed;
	bool                                                m_is_active;
	bool                                                m_is_visible = true;
	bool                                                m_rendering_state;
	bool                                                has_parent;
	bool                                                m_is_temporary;

	bool                                                m_is_dirty;
	bool                                                is_clone;
	bool                                                is_temp_clone = false;

	bool                                                m_group_updating_policy = true;
	bool                                                m_notify_scene = true;


	bool                                                m_is_immutable = false;
	// mesh  info //
	std::string                                         mesh_id = "";

private:

	optix::Matrix4x4 relative_translation_matrix;
	
	SceneObject* group_parent;
	SceneObject* real_group_parent;
	SceneObject * parent;
	std::vector<SceneObject*> childs;
	std::vector<SceneObject*> group_childs;
	std::vector<SceneObject*> group_selection_childs;

	// Grouping Information //
	Grouping_Data m_Grouping_Data;

	std::vector<optix::float3> ff_vectors;
	std::vector<class Restriction*> m_restrictions;
	std::vector<SceneObject*> m_face_vectors;

	std::vector<SceneObject*> m_Samplers;

private:

	bool is_lightButton_hovered = false;

	Light_Parameters   m_light_parameters;
	FaceVector_Parameters m_facevector_parameters;
	Sampler_Parameters m_sampler_parameters;
	optix::float3 m_gui_highlight_color;


	// unit_cube ( bbox ) mesh //
	optix::Transform m_Box_node;


	std::vector<SceneObject*> m_temp_cloned_Facevectors;
	std::vector<SceneObject*> m_temp_cloned_Samplers;
	

	bool keep_utility_childs = false;


};

