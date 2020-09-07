#pragma once
#include <optixu/optixpp_namespace.h>
#include "common_structs.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <unordered_map>
#include "SceneObject.h"
#include <unordered_map>
#include "Timer.h"



enum SAMPLER_ALIGNEMENT
{
	SA_GLOBAL,
	SA_LOCAL,
	SA_NORMAL,
	SA_BBOX
};

struct Transformation_Package
{
	int object_id = -1;
	std::vector<optix::float3> m_stacked_rotations;
	optix::float3 delta;
	AXIS_ORIENTATION axis_orientation;
	TRANSFORMATION_SETTING t_setting;
	TRANSFORMATION_TYPE t_type;

	void clear() { m_stacked_rotations.clear(); object_id = -1; }
};

enum FaceVector_View_State
{
	FVS_SELECTED_ONLY,
	FVS_ALL,
	FVS_NONE
};

enum FaceVector_Intersection_State
{
	FIS_SELECTED_ONLY,
	FIS_ALL,
	FIS_NONE
};

struct Object_General_Package
{
	class SceneObject* obj    = 0;
	class SceneObject* parent = 0;
	class SceneObject* group  = 0;
	std::vector<SceneObject*> childs;

	void reset()
	{
		obj = 0;
		parent = 0;
		group = 0;
		childs.clear();
	}

	void destroy()
	{

	}
};
struct Object_Package
{
	Transformation_Package trs_pack;

	float mouse_start_x;
	float mouse_start_y;

	class SceneObject * raw_object;
	int object_index;
	int selected;
	int focused;
	int highlight;
	// geometric properties
	optix::float3 base_axis[3];
	glm::mat4 def_transformation_matrix;
	glm::mat4 transformation_matrix;
	glm::mat4 translation_matrix_scaled;
	glm::mat4 screen_trs_matrix;
	float cam_inv_scale_factor;
	std::unordered_map<int, float> obj_dists;

	std::vector<class SceneObject*> childs;
	std::vector<class SceneObject*> GroupSelectionChilds;
	std::vector<class SceneObject*> GroupSelectionChilds_and_Groups;

	Object_Package()
		:mouse_start_x(0.0f),mouse_start_y(0.0f),raw_object(nullptr), object_index(-1), selected(0), focused(0), highlight(0), cam_inv_scale_factor(0.0f)
	{

	}

	~Object_Package() 
	{

	}
	

	void reset()
	{
		mouse_start_x = 0.0f;
		mouse_start_y = 0.0f;
		raw_object = nullptr;
		object_index = -1;
		selected = false;
		focused = false;
		highlight = false;

		trs_pack.clear();
		childs.clear();
		GroupSelectionChilds.clear();
		GroupSelectionChilds_and_Groups.clear();
	}
	// other data
	//optix::float3 start_trns;
	//optix::float3 start_rot;
	//optix::float3 start_scale;



};
enum Utility
{
	u_NONE,
	u_UNDO, u_REDO,
	u_LINK, u_UNLINK,
	u_SELECTION, u_SELECTION_GROUP, u_SELECTION_AREA,
	u_TRANSLATION, u_ROTATION, u_SCALE,
	u_CALLIBRATION_ANALOGUE,
	u_CALLIBRATION_FIXED,


	u_CREATE_LIGHT,
	u_CREATE_CAMERA,


	u_ATTACH_FACE_VECTOR,
	u_ATTACH_FACE_VECTOR_BBOX_ALIGNED,
	u_ATTACH_FACE_VECTOR_BBOX_ALIGNED_PARALLEL,
	u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED,
	u_ATTACH_SAMPLER,

	u_ATTACH_SAMPLER_PLANAR,
	u_ATTACH_SAMPLER_VOLUME,
	u_ATTACH_SAMPLER_DIRECTIONAL,


	u_ALIGN,
	u_ALIGN_CAMERA,

	u_ATTACH_RESTRICTION_FOCUS,
	u_ATTACH_RESTRICTION_CONVERSATION,
	u_ATTACH_RESTRICTION_DISTANCE,

	u_SCENE_EDIT_MODE,
	u_SCENE_RENDER_MODE,
	u_TOGGLE_LIGHTS,


	u_OTHER
};
static const std::string Utility_ids[29] = 
{ 

"undo", 
"redo", 

"link",
"unlink", 

"selection", 
"selection_group",
"selection_area", 

"translation", 
"rotation", 
"scale", 

"callibration_analogue",
"callibration_fixed",

"create_sampler",
"create_light",
"create_camera",

"sampler_planar",
"sampler_volume",
"sampler_directional",

"face_vector_bbox_aligned",
"face_vector_bbox_aligned_parallel",
"face_vector_normal_aligned",

"object_align",
"object_camera_align",


"scene_edit_mode",
"scene_render_mode",
"scene_toggle_lights",

"restriction_focus",
"restriction_conversation",
"restriction_distance"

};

enum SELECTION_SETTING
{
	S_OBJECT,
	S_GROUP,
	S_TOP_LEVEL_GROUP
};

enum Utility_Group
{
	g_NONE,
	g_ACTION,
	g_LINKING,
	g_SELECTIONS,
	g_TRANSFORMATIONS,
	g_CALLIBRATION,

	g_CREATE,
	g_ATTACH_FACE_VECTOR,

	g_ALIGN,
	g_SCENE_MODE,
	g_LIGHTS_TOGGLE,

	g_ATTACH,
	g_RESTRICTIONS,
	g_OTHER
};
struct UtilityProperties
{
	Utility id;
	Utility_Group group;
	bool active;

	UtilityProperties()
		:id(Utility::u_NONE), group(Utility_Group::g_NONE), active(false)
	{}

	void reset()
	{
		id     = u_NONE;
		group  = g_NONE;
		active = false;
	}
};
enum CURSOR_TYPE
{
	CURSOR_NONE,
	CURSOR_TRANSLATION,
	CURSOR_ROTATION,
	CURSOR_ROTATION_ACTIVE,
	CURSOR_SCALE,
	CURSOR_SCALE_ACTIVE,
	CURSOR_CAMERA_ORBIT,
	CURSOR_CAMERA_FOCUS,
	CURSOR_CAMERA_PAN,
	CURSOR_CAMERA_PAN_DRAGGING,
	CURSOR_SELECTION,
	CURSOR_LINK,
	CURSOR_LINK_ACTIVE
};
static const std::string cursor_types[12] =
{
	"cursor_translation",
	"cursor_rotation",
	"cursor_rotation_active",
	"cursor_scale",
	"cursor_scale_active",
	"cursor_camera_orbit",
	"cursor_camera_focus",
	"cursor_camera_pan",
	"cursor_camera_pan_dragging",
	"cursor_selection",
	"cursor_link",
	"cursor_link_active"
};
struct Interface_State
{
	IntersectionPoint* isec_p;
	UtilityProperties* active_utility;
	UtilityPlane selected_utility_plane;
	Object_Package* selected_object_pack;
	AXIS_ORIENTATION active_axis_orientation;
	bool axis_orientation_changed;
	bool utility_manipulation_active;
	bool selected_object_changed;
	float m_camera_inv_scale_factor;
	bool inner_radius_hit;
	bool m_gui_transforming;
	SAMPLER_ALIGNEMENT m_sa;
	bool m_sampler_init_use_bbox = 0;
	Sampler_Parameters m_sampler_params;

	Light_Parameters m_light_params;
	int m_light_isec_plane = 1;
	bool link_to_object    = 0;
	bool flip_side = 0;
	bool attach_bbox = 0;

	Interface_State()

		:
		isec_p(nullptr),
		active_utility(nullptr),
		selected_utility_plane(UtilityPlane::NONE),
		selected_object_pack(nullptr),
		active_axis_orientation(AXIS_ORIENTATION::WORLD),
		axis_orientation_changed(false),
		utility_manipulation_active(false),
		selected_object_changed(false),
		m_camera_inv_scale_factor(0.0f),
		inner_radius_hit(false),
		m_gui_transforming(false)
	{
		m_light_params.type = QUAD_LIGHT;
	}
	~Interface_State() {}

	void reset()
	{

		isec_p = nullptr;
		active_utility = nullptr;
		selected_utility_plane = UtilityPlane::NONE;
		selected_object_pack = nullptr;
		active_axis_orientation = AXIS_ORIENTATION::WORLD;
		axis_orientation_changed = false;
		utility_manipulation_active = false;
		selected_object_changed = false;
		m_camera_inv_scale_factor = 0.0f;
		inner_radius_hit = false;
		m_gui_transforming = false;
		m_sampler_init_use_bbox = 0;
		link_to_object = 0;
		flip_side = 0;
		attach_bbox = 0;
	}

};
enum KEY_GROUP_UI_STATE
{
	UI_KEY_GRP_NONE,
	UI_KEY_GRP_EDIT,
	UI_KEY_GRP_CLONE
};
enum KEY_COMB_UI_STATE
{
	UI_KEY_COMB_NONE,
	UI_KEY_COMB_UNDO,
	UI_KEY_COMB_REDO,

	UI_KEY_COMB_COPY,
	UI_KEY_COMB_PASTE,
	UI_KEY_COMB_CUT,
	UI_KEY_COMB_DELETE,
	UI_KEY_COMB_ACCEPT,
	UI_KEY_COMB_CANCEL
};
struct Object_Transformations_Manipulation_Data
{
	IntersectionPoint isec_p;

	UtilityPlane plane_id;
	UtilityPlane plane_id1, plane_id2;

	Plane utility_plane;

	optix::float3 plane_n;
	optix::float3 starting_point;
	optix::float3 starting_dir;

	float starting_delta;
	float starting_delta1, starting_delta2, starting_delta3;
	optix::float3 starting_plane_delta;

	optix::float3 start_translation;
	optix::float3 start_rotation;
	optix::float3 start_scale;
	float start_acc_rot = 0.0f;
	float acc_rot = 0.0f;
	float start_rot_theta;
	float * mat_data;
	std::vector< float * > child_mat_data;
	std::vector<optix::float3> childs_relative_positions;
	std::vector<class SceneObject*> child_index;
	std::unordered_map<class SceneObject*, optix::float3> child_attri;
	optix::float3 delta;
	optix::float3 overall_delta;
	bool got_delta;
	float delta_length;
	bool init_matrices = false;
	std::vector<optix::float3> stack_rotations;
	optix::Matrix4x4 acc_mat;
	optix::Matrix4x4 start_rot_mat;
	optix::Matrix4x4 start_scale_mat;
	optix::float3 start_bbox[2];
	optix::float3 start_centroid;
	
	Object_Transformations_Manipulation_Data()
	{
		plane_id = plane_id1 = plane_id2 = NONE;
		
		plane_n = optix::make_float3(0.0f);
		starting_point = optix::make_float3(0.0f);
		starting_delta = starting_delta1 = starting_delta2 = 0.0f;
		starting_plane_delta = optix::make_float3(0.0f);
		start_translation = optix::make_float3(0.0f);
		start_rotation = optix::make_float3(0.0f);
		start_scale = optix::make_float3(0.0f);
		start_rot_theta = 0.0f;
		//mat_data = new float[16];
		delta = ZERO_3f;
		overall_delta = ZERO_3f;

		delta_length = 0.0f;
		acc_mat = optix::Matrix4x4::identity();
		//base_axis = 0;
	}
	~Object_Transformations_Manipulation_Data()
	{
		if (init_matrices)
		{
			delete mat_data;
			for (int i = 0; i < child_mat_data.size(); i++)
				delete child_mat_data[i];
		}
		

		//delete base_axis;
	}

	void reset()
	{
		start_rot_mat = optix::Matrix4x4::identity();
		start_scale_mat = optix::Matrix4x4::identity();

		optix::float3 start_bbox[2] = { ZERO_3f, ZERO_3f };
		optix::float3 start_centroid = ZERO_3f;
		////std::cout << " - Object_Transformations_Manipulation_Data::reset():" << std::endl;
		plane_id = plane_id1 = plane_id2 = NONE;

		plane_n = optix::make_float3(0.0f);
		starting_point = optix::make_float3(0.0f);
		starting_delta = starting_delta1 = starting_delta2 = 0.0f;
		starting_plane_delta = optix::make_float3(0.0f);
		start_translation = optix::make_float3(0.0f);
		start_rotation = optix::make_float3(0.0f);
		start_scale = optix::make_float3(0.0f);
		start_rot_theta = 0.0f;
		acc_rot = 0.0f;

		if (init_matrices)
		{
			delete mat_data;
			for (int i = 0; i < child_mat_data.size(); i++)
				delete child_mat_data[i];
		}

		child_mat_data.clear();
		child_index.clear();
		childs_relative_positions.clear();
		child_attri.clear();
		
		delta = ZERO_3f;
		overall_delta = ZERO_3f;
		got_delta = 0;
		delta_length = 0.0f;
		init_matrices = false;
		stack_rotations.clear();
		acc_mat = optix::Matrix4x4::identity();
	
	}

};

enum ImGui_Window_ID
{
	MAIN_MENU_BAR_,
	MAIN_TOOLS,
	MAIN_TOOLS_HELPERS,
	LEFT_PANEL,
	BOTTOM_PANEL,
	RIGHT_PANEL
};
struct ImGui_Window_Data
{
	ImGui_Window_ID id;
	ImVec2 pos;
	ImVec2 pos_delta;
	ImVec2 size_delta;
	ImVec2 size;
	bool resizable;
	bool movable;
	
};


#define UI_BUTTON
#define BUTTON_SIZE ImVec2(33,33) // 36, 36
#define BUTTON_RATIO ImVec2(0.970f,0.970f)
#define BUTTON_COLOR_BG ImVec4(0.0f,0.0f,0.0f,0.0f)
#define BUTTON_PADDING -1.0f
struct Button
{
	Timer timer;

	float focused_timer = 0.0f;
	bool focused;
	bool selected;
	bool pressed;

	std::string label;
	std::string tooltip;
	Utility utility;
	Utility_Group ut_group;
	GLuint * icon;
	std::string icon_id;

	
	ImVec2 size;
	ImVec2 ratio;
	float padding;
	ImVec4 bg_color;


	Button();
	Button(std::string label, Utility ut, struct ImVec2 size, struct ImVec2 ratio, float padding, struct ImVec4 bg_color);
	Button(std::string label, Utility ut, GLuint * icon, struct ImVec2 size, struct ImVec2 ratio,float padding, struct ImVec4 bg_color);

	void construct();
	void toggle(bool * is_valid = 0);
	void setFocused(bool state);
	void setSelected(bool state);
	void setPressed(bool state);
	void updateIcon();
	
};



struct Multiple_Selection_Data
{
	int parent_id;
	std::vector<SceneObject*> selection_objects;

	std::unordered_map<int, SceneObject*> group_parents_bucket;
	std::unordered_map<int, SceneObject*> parents_bucket;
	std::unordered_map<int, Parent_Data>  parents_data;

	std::vector<SceneObject*> top_level_subgroups;
	std::vector<SceneObject*> top_level_entities;
	std::vector<SceneObject*> top_level_groups;
	std::vector<SceneObject*> solo_objects;

	bool can_group;
};



struct NormalVector_Obj
{
	optix::float3 origin = ZERO_3f;
	optix::float3 dir    = ZERO_3f;
	
	class SceneObject* object = 0;
	optix::float3 obj_pos = ZERO_3f;

	bool is_object = false;

	void reset()
	{
		object    = 0;
		is_object = false;
		obj_pos = ZERO_3f;
		origin  = ZERO_3f;
		dir     = ZERO_3f;
	}
};
