#pragma once



#define IMGUI_DEFINE_MATH_OPERATORS 1
#include "InterfaceManager_Defines.h"
#include "imgui.h"
#include "imgui_internal.h"
//#include "ImGuiStyleManager.h"

#include "Interface_Structs.h"
#include "common_structs.h"
#include "Utility_Renderer_Defines.h"
//#include "ViewportManager_structs.h"
#include <unordered_map>
#include <stack>
#include "HierarchyTree.h"
#include "SceneGroup.h"
#include <unordered_map>
#include "ViewportManager.h"

#include <thread>

#define INTERFACE_MAIN_FUNCTIONS
#define INTERFACE_INIT
#define INTERFACE_UI_CONSTRUCTORS
#define INTERFACE_UPDATE_FUNCTIONS
#define INTERFACE_RENDER_FUNCTIONS
#define INTERFACE_HANDLE_FUNCTIONS
#define INTERFACE_UTIL_FUNCTIONS
#define INTERFACE_SET_FUNCTIONS
#define INTERFACE_GET_FUNCTIONS

#define ButtonHandler
#define EventHandler
#define Intersection_Routine_Manager


#define pre_clone_Utility_Childs


enum SELECT_DIR_ACTION
{
	SDA_NONE,
	IMPORT_OBJECT,
	LOAD_SCENE,
	SAVE_SCENE,
	EXPORT_SCENE
};

enum TAB_ID
{
	GENERAL_INFO_TAB,
	TRANSFORMATIONS_TAB,
	MATERIALS_TAB,
	LIGHT_PARAMS_TAB,
	SAMPLER_PARAMS_TAB,
	RELATIONS_TAB,
	RESTRICTIONS_TAB,
	SCENE_SETTINGS_TAB,
	NULL_TAB
};




class InterfaceManager
{



//------------------------------------------------------------------------------
//
//  MEMBERS
//
//------------------------------------------------------------------------------

private:

	Scene_Mode m_scene_mode = Edit_Mode;
	SceneRenderSettings m_sceneRender_settings = SceneRenderSettings(optix::make_int2(5, 5),
																	 1,
																	 1,
																	 1,
																	 0);
	SceneRenderSettings m_sceneEdit_settings = SceneRenderSettings(optix::make_int2(0, 1),
																	 0,
																	 0,
																	 0,
																	 0);

	const char* m_tooltip_label[10] = {
		"General",
		"Transformations",
		"Materials",
		"Light Parameters",
		"Samplers Parameters",
		"Relations",
		"Restrictions",
		"Scene Settings",
		"null9",
		"null10"
	};

	// struct members keeping nescessairy data
	Interface_State   m_interface_state;
	Io_Mouse          mouse_data;
	Io_KeyBoard       key_data;

	std::vector<Object_General_Package> m_cloned_faceVectors;
	std::vector<Object_General_Package> m_cloned_Samplers;
	Object_Package    m_focused_object_pack;
	Object_Package    m_selected_object_pack;
	Object_Package    m_cloned_object_pack;
	Object_Package    m_prev_focused_object_pack;
	Object_Package    m_prev_selected_object_pack;
	Object_Package    m_prev_INTERFACE_selected_object_pack;
	Object_Package    m_prev_INTERFACE_focused_object_pack;
	std::vector<Object_Package> m_multiple_object_selection;
	std::vector<SceneObject*>   m_multiple_selected_objects;
	Object_Package    m_temporary_group_selection_dummy_parent;

	bool              m_utility_interaction;
	UtilityProperties m_selected_utility;
	std::unordered_map<Utility, UtilityProperties> m_utilities_state;
	IntersectionPoint m_utility_isec_p;
	float             m_renderer_camera_inv_scale_factor;
	Object_Transformations_Manipulation_Data m_object_transformations_manip_data;

	// key events ui-states
	KEY_GROUP_UI_STATE ui_key_grp_state;
	KEY_COMB_UI_STATE  ui_key_comb_state;


	// main_window_app vars
	float m_window_width, m_window_height;
	bool m_gui_focused;
	bool m_popup_open;

	// gui vars
	bool m_interfaceManager_init;
	bool m_fileChooser_open;
	bool m_fileChooser_export_open = false;
	bool m_fileChooser_select_dir  = false;
	SELECT_DIR_ACTION m_fileChooser_select_dir_action = SDA_NONE;
	bool m_edit_Style_open;
	//

	// Main Menu bar - tools vars
	std::unordered_map<std::string, Button*> tool_buttons;
	AXIS_ORIENTATION m_selected_axis_orientation;
	bool m_selected_axis_orientation_changed;
	CURSOR_TYPE m_active_cursor;


	// Interface Panels(windows) vars
	int m_interface_active_object;
	float timer;

	// state vars
	bool m_ui_restrictions_panel_focused = false;
	int  m_m0_down_on_LightButton = -1;
	bool m_m0_down_on_focused_object;
	bool m_m0_down_on_utility_interaction;
	bool m_m0_down_on_gui;
	bool m_m0_down_on_camera_control_handle = false;
	int  m_mouse_movement_while_m0_down = 0;
	bool m_m2_down_on_gui;
	bool m_transforming_active;
	bool m_cloning_active;
	int  m_cloning_active_type = -1;
	bool m_cloning_group;
	bool m_cloning_finished;
	bool m_retargeting_camera;
	bool m_skip_frame = false;
	bool m_reselecting;
	bool m_wait_to_release;
	bool m_wait_to_release_m2;
	bool m_force_input_update;
	bool m_object_tree_focused;
	

	Utility m_prev_utility;
	UtilityProperties m_prev_selected_utility;
	

	// temp
	bool m_vp_frame_buttons_focused = false;
	bool m_ui_focused;
	int  m_ui_focused_object_id = -1;
	bool m_popup_object_cloning_approval;
	bool m_popup_group_creating_approval;
	bool m_popup_create_light;
	bool m_popup_create_sampler;
	bool m_popup_import_file;
	bool m_popup_import_file_toggle = false;
	bool m_popup_crt_dir = false;
	bool m_popup_save_file;
	bool m_popup_load_file;
	bool m_popup_viewport_settings = false;
	bool m_retargeting_viewport = false;
	bool m_focusing_menu_bar_items = false;

	bool m_skip_user_input;
	bool m_skip_user_input_finished;
	bool m_finished_area_selection;
	bool m_multiple_selection_active;
	int  m_multiple_selection_active_id;
	std::vector<SceneObject*>              m_prev_multiple_selection;
	std::unordered_map< int, SceneObject*> m_parents_bucket;
	std::unordered_map< int, Parent_Data>  m_parent_data;
	int                                    m_temp_group_parent_id;
	std::vector<Multiple_Selection_Data>   m_area_selections_data;

	// popUp object cloning approval vars
	bool as_instance;
	bool as_copy;
	bool cloning_options_clone_fvs      = true;
	bool cloning_options_clone_samplers = true;
	bool approve_grouping;
	char group_name[101]   = "";
	char object_name[101] = "";
	char cloning_object_name[101] = "";
	bool m_cloning_asSingleGroup = false;

	bool m_changing_name = false;
	bool m_changing_name_is_active = false;


	bool is_m0_down;
	optix::float2 m0_down;
	
	// window structs
	std::unordered_map<ImGui_Window_ID, ImGui_Window_Data> imgui_windows;

	// viewports Handle
	//VIEWPORT_ID m_active_viewport = VIEWPORT_ID::MAIN_VIEWPORT;
	optix::float2 m_renderer_renderSize;
	bool m_viewport_camera_active;

	//
	SELECTION_SETTING m_selection_setting;
	int object_tree_max_depth;
	float object_treeNode_width;
	float object_treeNode_indent;

	//
	
	bool m_analogue_callibration;

	TRANSFORMATION_SETTING m_hierarchy_setting;


	// ui
	int selected_material_index = -1;
	SceneObject* gui_selected_child = 0;
	SceneObject* gui_selected_grp_child = 0;
	int gui_selected_child_index = -1;
	int gui_selected_grp_child_index = -1;


	bool print_debug_info = false;

	
	Object_Transformations_Manipulation_Data m_transformation_input_data;
	bool m_dragFloat_state = false;
	bool m_plus_minus_button_state = false;
	bool m_plus_minus_button_state_2 = false;
	bool m_transformation_input_state_activating = false;
	bool m_transformation_input_state      = false;
	bool m_transformation_input_state_2    = false;
	int  m_transformation_input_i = -1;
	int  m_transformation_input_attr_index = -1;
	bool m_transformation_input_m0_down    = false;
	bool m_transformation_input_m0_release = false;
	bool m_transformation_input[3] = { false, false, false };
	bool m_transformation_input_changed[3] = { false, false, false };
	int m_transformation_input_j[3] = { 0,0,0 };
	int m_gui_transforming[3] = { 0,0,0 };
	float m_euler_prev_value  = 0.0f;
	float m_euler_value_delta = 0.0f;
	optix::float3 m_euler_starting_values = ZERO_3f;

	std::string tooltip_label = "";
	ImVec2 tooltip_pos;
	bool m_popup_tooltip = false;

	

	TAB_ID m_active_tab = GENERAL_INFO_TAB;
	bool m_force_tab_open_active = false;
	bool m_force_tab_open[10]    = { false,false,false,false,false,false,false,false,false,false };
	class Restriction* m_UI_selected_Restriction = 0;
	int  m_UI_selected_Restriction_id            = -1;
	int  m_UI_hovered_Restriction_id             = -1;

	class SceneObject* m_UI_focused_Relation = nullptr;
	bool is_Relations_Tab_Active = false;
	
	FaceVector_View_State m_FaceVector_view_State = FVS_NONE;
	FaceVector_Intersection_State m_FaceVector_Intersection_State = FIS_NONE;

	int  m_sampler_crt_def_type          = 1;
	bool m_sampler_init_isec_object_bbox = 0;
	bool m_should_peek_ac_flag = true;

	int m_light_crt_def_type    = 1;
	int m_light_init_isec_plane = 1; 


	Timer tooltip_timer;
	float m_item_hovered_timer = 0.0f;


	bool m_object_tree_view_obj = 1;
	bool m_object_tree_view_grp = 1;
	bool m_object_tree_view_fv     = 0;
	bool m_object_tree_view_smplr  = 0;
	bool m_object_tree_view_lights = 1;
	std::vector<TreeNode*> m_exclusive_nodes;

	//
	bool m_Scene_Import_STATE = 0;
	std::thread m_scene_Import_THR;

	bool m_hovering_over_camera_orbit = false;
	bool m_repositioning_cursor = false;

	std::vector<class Action*> local_action_stack;
	class Action* m_align_pos_action = 0;
	class Action* m_align_rot_action = 0;
	optix::Matrix4x4 m_align_rot_mat;
	bool m_align_rot_changed = false;
	bool m_popup_object_align_window = false;
	Object_Transformations_Manipulation_Data m_align_object_data;
	bool m_popup_object_align_toggled = false;
	class SceneObject* m_align_object = 0;
	class SceneObject* m_align_target = 0;
	bool m_pos_x = true;
	bool m_pos_y = true;
	bool m_pos_z = true;
	bool m_axis_x = false;
	bool m_axis_y = false;
	bool m_axis_z = false;
	bool m_scale_x = false;
	bool m_scale_y = false;
	bool m_scale_z = false;
	bool m_pos_align_enabled = true;
	bool m_rot_align_enabled = true;
	int A_pos_align = 2; // pivot
	int B_pos_align = 2; // pivot
	TRANSFORMATION_SETTING m_align_hierarchy_setting = TO_ALL;

	bool m_popup_camera_align_window = false;

	std::string button_hovered_id  = "";
	std::string buttona_hovered_id = "";
	std::string buttonb_hovered_id = "";

	bool is_button_hovered  = false;
	bool is_buttona_hovered = false;
	bool is_buttonb_hovered = false;

	// Camera Align Vars //
	bool m_camera_align_picking_Object = false;
	bool m_camera_align_picking_Target = false;
	bool m_camera_align_picked_Object = false;
	bool m_camera_align_picked_Target = false;
	//bool is_cam_align_SourceObj_NormalVector = false;
	//bool is_cam_align_TargetObj_NormalVector = false;
	NormalVector_Obj m_camera_align_object;
	NormalVector_Obj m_camera_align_target;


	bool m_scene_lights_on = false;
	bool m_draw_scene_dimensions = false;

	std::vector<class SceneObject*> m_traversed_objects;
	std::vector<class SceneObject*> m_traversed_objects_for_selection;
	bool create_area_selection = false;
	class SceneObject* m_tree_from = 0;
	class SceneObject* m_tree_to = 0;
	//class SceneObject* m_tree_to_select;
	
	
	//

//------------------------------------------------------------------------------
//
//  FUNCTIONS
//
//------------------------------------------------------------------------------

public:

	static InterfaceManager& GetInstance()
	{
		static InterfaceManager interfaceManager;
		return interfaceManager;
	}

	InterfaceManager();
	~InterfaceManager();


	// Interface Main Functions
#ifdef INTERFACE_MAIN_FUNCTIONS

#ifdef INTERFACE_INIT

public:

	bool                                             Init(struct GLFWwindow * main_app_window);

private:

	bool                                             Init_ViewportManager();
	bool                                             Init_Renderer();
	bool                                             Init_ImGui(struct GLFWwindow * main_app_window);
	bool                                             Init_tool_Buttons();
	bool                                             Init_Renderer_RenderSize();
	bool                                             Init_Renderer_Viewport();
	bool                                             Init_ImGui_Windows();

#endif
	
#ifdef INTERFACE_UPDATE_FUNCTIONS
	
public:
	void                                             Update(float dt);
	void                                             Update_group_button_state(Utility_Group group, Utility selected_utility);

private:
	void                                             Update_WindowSize();
	void                                             Update_User_Input();
	void                                             Update_Mouse_Wheel_Input();
	void                                             Update_current_Interface_State();

	void                                             Catch_User_Input_General_Callback_Func();

	void                                             Update_ViewportManager();
	void                                             Update_Renderer_Data();
	void                                             Update_Renderer_RenderSize();
	void                                             Update_Renderer_Viewport();
	void                                             Update_Renderer_Highlight_RenderList();
	void                                             Update_Renderer_Utility_RenderList();
	void                                             Update_Renderer_Utility_Cursor();

#endif

#ifdef INTERFACE_UI_CONSTRUCTORS

private:

	void                                             construct_Menu_Items();
	void                                             construct_Tools();
	void                                             construct_Tools_Helpers();

	void                                             construct_Interface();
	void                                             construct_MainMenuBar_2();
	void                                             construct_MainMenuBar();
	void                                             construct_MainMenuBar_Helpers();
	void                                             construct_LeftPanel();
	void                                             construct_SceneObjectTree(TreeNode* node, int depth , int counter);
	void                                             construct_SceneObjectTree(SceneObject* obj, int depth, int counter);

	void                                             construct_RightPanel();
	void                                             construct_RightPanel_Test();

	void                                             construct_Object_General_Settings_Window();
	void                                             construct_Object_Relations_Window();
	void                                             construct_Object_Restrictions_Window();
	void                                             construct_Transformations_child_Window();
	void                                             construct_Materials_child_Window();
	
	void                                             construct_Light_Parameters_child_Window();
	void                                             construct_Sampler_Parameters_Child_Window();
	void                                             construct_Scene_Render_Settings_Window();
	void                                             construct_Scene_Options_Window();

	void                                             construct_Viewports_Frame_Settings();
	void                                             construct_BottomPanel();
	void                                             construct_BottomPanel_Test();

	void                                             construct_FileBrowser_Test();
	void                                             construct_FileBrowser();
	void                                             construct_FileBrowser_select_Dir();
	void                                             construct_window_render_test();
	
	// popUps
	bool                                             pop_Object_cloning_Approval_window();
	bool                                             pop_Group_creating_Approval_window();
	bool                                             pop_Create_Light_Window();
	bool                                             pop_Create_Sampler_Window();
	bool                                             pop_Object_Align_Window();
	bool                                             pop_Camera_Align_Window();

	void                                             construct_Scene_Import_Reporting_Window();

	// helpers
	void  construct_Child_Window_3f_var( std::string title, ImVec2 size, std::string& var_title,  optix::float3& var , bool &is_changed, bool is_vertical = true, bool is_colorEdit = false );
	void  construct_Child_Window_1f_var( std::string title, ImVec2 size, std::string& var_title,  float& var         , bool &is_changed, bool is_colorEdit = false );

#endif

#ifdef INTERFACE_RENDER_FUNCTIONS
			
public:
	void                                             Render_Interface(bool render_utility = true);

private:
	void                                             Render_Utility();


#endif
	
#ifdef INTERFACE_HANDLE_FUNCTIONS

public:
	void                                             HandleEvents();
	void                                             handle_Utility_Interaction();

#endif

#ifdef INTERFACE_UTIL_FUNCTIONS

private:

	void                                             process_TAB_SELECTION_STATE_change(TAB_ID to_tab);
	void                                             process_SELECTION_STATE_change(SceneObject* to_obj);
	void                                             process_SELECTION_STATE_change_post_order(SceneObject* to_obj = 0);
	void                                             reset_Tabs_State(TAB_ID force_tab_id);
	void                                             reset_Force_Tabs_Open_Flags();
	int                                              getFocusedViewport();

	void                                             calculate_Object_Pack_Data(struct Object_Package * obj_pack, UtilityProperties utility);
	void                                             calculate_new_utility_isec_point_relative_on_selected_utility_plane();
	void                                             fill_pre_object_manipulation_Data();
	bool                                             is_finally_object_cloning_aproved(Object_Transformations_Manipulation_Data& data);


	void                                             edit_style();

public:

	class Action*                                    get_Temp_Group_Disband_Action(int index, int index2);
	void                                             push_Temp_Group_Disbad_Action(int index, int index2);
	void                                             recover_selections_state( int index , bool reset_state = false );
	void                                             recover_selections_state_flat(int index);
	void                                             remove_Multiple_Selection_Data( int index );

	bool                                             get_Grouping_Information(SceneObject* object);
	void                                             get_Grouping_Information(Multiple_Selection_Data& group_data);
	void                                             create_SceneGroup(std::vector< SceneObject*>& group, std::vector< SceneObject*>& top_level_groups, std::vector<SceneObject*>& area_selection, std::string name = "");
	void                                             create_SceneGroup(SceneObject* object, std::string name = "");


	void                                             process_FocusedObjectPack( Object_Package& obj_pack );
	void                                             process_AreaSelection(Multiple_Selection_Data& data , bool as_temp_group = false);
	void                                             process_pre_AreaSelection(std::vector<SceneObject*>& selection);

	void print_Object_TRS_info();
	void print_Debug_Info(bool ut_childs);
	void print_Debug_Info_(SceneObject* obj);
	void print_SceneObjects_Info();

	void show_Tooltip(std::string label);
	void show_Tooltip();
	void popup_Tooltip(ImVec2 pos, std::string label);

	void check_viewports_SELECT_FOCUS_state( bool force_focus = false, bool force_selec = false );
	
	
	void test_thread_func();
	void IMPORT_SCENE_THREADED(const char * filename);

#endif

#ifdef INTERFACE_SET_FUNCTIONS

private:
	void                                             resetSelectedObject(bool save_action = true, bool dummy_action = false);
	
public:


	void                                             set_UI_Selected_Restriction(int id);
	void                                             set_UI_Selected_Restriction(class Restriction* R);

	void                                             setSelectedGroup(SceneGroup* , bool reset = true);
	void                                             setSelectedObject(int object_index, bool reset = true, bool by_id = false);
	void                                             setSelectedObject(struct Object_Package object_pack, bool save_action = true, bool reset = true);
	void                                             setSelectedObject(SceneObject* object, bool reset = true);
	void                                             set_active_Axis_Orientation(AXIS_ORIENTATION axis);
	void                                             set_active_Utility(Utility utility);

	void                                             set_active_multiple_selection(int index);

	void                                             gui_set_Style(std::string style_id);
	void                                             gui_set_Font(int id, std::string font_attr, int size);
	void                                             set_FaceVector_States(UtilityProperties ut);
	void                                             reset_FaceVector_States();

#endif

#ifdef INTERFACE_GET_FUNCTIONS

public:

	bool                                             is_utility_toggle_valid(Utility ut_id);

	bool                                             is_Hovering_Over_Camera_Orbit();
	bool                                             is_Camera_Orbit_Active();
	TAB_ID                                           get_Active_Tab();
	class SceneObject*                               get_UI_Focused_Relation();
	
	class Restriction*                               get_UI_Selected_Restriction();
	int                                              get_UI_Selected_Restriction_Id();
	Interface_State *                                get_current_Interface_State();
	AXIS_ORIENTATION                                 get_active_Axis_Orientation();
	bool                                             is_selected_utility_active();
	bool                                             isGuiFocused();
	int                                              get_Ui_Focused_Object();
	Io_Mouse                                         getMouseData();

	optix::float2                                    get_MousePos_RelativeTo(int id); // VIEWPORT_ID
	
	Dimension_Constraints                            getViewportSpaceConstraints();
	
	bool                                             checkIntegrity();
	bool                                             is_multiple_selection_Active();

	Multiple_Selection_Data&                         get_Area_Selection_Data(int parent_id);
	const Multiple_Selection_Data&                   get_Active_Area_Selection_Data();

	std::string                                      get_LabelIcon_by_Type(Type type);

	
	FaceVector_View_State                            get_FaceVector_View_State();
	FaceVector_Intersection_State                    get_FaceVector_Intersection_State();


	SceneObject*                                     Get_Focused_Object();
	SceneObject*                                     Get_Selected_Object();
	SceneObject*                                     Get_prev_Selected_Object();
	SceneObject*                                     Get_prev_Focused_Object();
	int                                              Get_prev_Selectecd_Object_Id();
	int                                              Get_prev_Focused_Object_id();
	Object_Package                                   Get_Focused_Object_Pack();
	Object_Package                                   Get_Selected_Object_Pack();

	

	bool                                             is_Object_Align_Active();
	class SceneObject*                               get_Object_Align_SourceObj();
	class SceneObject*                               get_Object_Align_TargetObj();

	bool is_cam_align_PickingObject();
	bool is_cam_align_PickingTarget();
	bool is_cam_align_PickedObject();
	bool is_cam_align_PickedTarget();
	NormalVector_Obj get_cam_align_SourceObj();
	NormalVector_Obj get_cam_align_TargetObj();


	int                                              get_Area_Selection_Active_Id();
	std::string                                      get_IconLabel(Type obj_type);

	UtilityProperties                                get_active_Utility();

private:
	bool                                             get_Utility_Interaction();
	int                                              getArea_Selection_DataIndex_FromParent(int id);
	bool                                             is_Object_Valid_For_Camera_Align( class SceneObject* object );



#endif

#endif
	
	// ButtonHandler Functions
#ifdef ButtonHandler
#define MainMenuBar_Events
#define Tools_Events
#define Tools_Helpers_Events
#define LeftPanel_Events
#define RightPanel_Events

	//------------------------------------------------------------------------------
	//
	// MainMenuBar Events
	//
	//------------------------------------------------------------------------------


	//------------------------------------------------------------------------------
	//
	// Tools Events
	//
	//------------------------------------------------------------------------------

public:

	void toggle(Utility ut_id, bool push_action = true , bool toggle_button = false, bool * is_valid = 0);

private:

	

	void toggle_UNDO();
	void toggle_REDO();

	void toggle_LINK();
	void toggle_UNLINK();

	void toggle_SELECTION();
	void toggle_SELECTION_GROUP();
	void toggle_SELECTION_AREA();

	void toggle_TRANSLATION();
	void toggle_ROTATION();
	void toggle_SCALE();
	void toggle_AXIS_SELECTION(bool push_action = true);


	void toggle_Callibration_Analogue();
	void toggle_Callibration_Fixed();

	void toggle_Create_Light();

	void toggle_Attach_Sampler_Planar();
	void toggle_Attach_Sampler_Volume();
	void toggle_Attach_Sampler_Directional();

	void toggle_Attach_Face_Vector_BBOX_Aligned();
	void toggle_Attach_Face_Vector_BBOX_Aligned_Parallel();
	void toggle_Attach_Face_Vector_Normal_Aligned();

	void toggle_Attach_Restriction_Focus();
	void toggle_Attach_Restriction_Conversation();
	void toggle_Attach_Restriction_Distance();

	void toggle_Object_Align();
	void toggle_Camera_Align();

	void toggle_Scene_Edit_Mode();
	void toggle_Scene_Render_Mode();
	void toggle_Scene_Lights_Toggle();

	//------------------------------------------------------------------------------
	//
	// Tools_Helpers Events
	//
	//------------------------------------------------------------------------------

	void toggle_Linking_Settings();
	void toggle_Hierarchy_Settings();

	//------------------------------------------------------------------------------
	//
	// LeftPanel Events
	//
	//------------------------------------------------------------------------------

	//void Handle_CollapsingHeader_SceneObjects();

	//------------------------------------------------------------------------------
	//
	// RightPanel Events
	//
	//------------------------------------------------------------------------------
#endif

	// EventHandler Functions
#ifdef EventHandler

	void handle_m0_CLICKED();
	void handle_m0_DOWN();
	void handle_m0_RELEASED();
	void handle_m1_DOWN();
	void handle_m1_RELEASED();

	void handle_Utility_m0_CLICKED( UtilityProperties ut_prop  );
	void handle_Utility_m0_DOWN( UtilityProperties ut_prop     );
	void handle_Utility_m0_RELEASED( UtilityProperties ut_prop );
	void handle_Utility_m1_DOWN( UtilityProperties ut_prop     );
	void handle_Utility_m1_RELEASED( UtilityProperties ut_prop );

	void handle_preCLONE();
	void handle_preCLONE_SINGLE_SELECTION();
	void handle_preCLONE_AREA_SELECTION();
	void handle_preCLONE_GROUP_SELECTION();
	void handle_preCLONE_GROUP_SELECTION_fromSingle();
	void handle_CLONE(bool approve, int cloning_id, Object_Package& cloned_object_pack, Object_Package& prev_object_pack, std::string name);
	void handle_CLONE_Approve(int cloning_id, Object_Package& cloned_object_pack, Object_Package& prev_object_pack, std::string name);
	void handle_CLONE_Cancel(int cloning_id, Object_Package& cloned_object_pack, Object_Package& prev_object_pack, std::string name);
	void handle_DELETE();

	void handle_UNDO();
	void handle_REDO();

	void handle_LINK();
	void handle_UNLINK();

	void handle_SELECTION();
	void handle_MULTIPLE_SELECTION();
	class Action *  get_MULTIPLE_SELECTION();
	void handle_AREA_SELECTION();
	class Action * get_AREA_SELECTION();

	void handle_TRANSFORMATION();
	void handle_TRANSLATION();
	void handle_ROTATION();
	void handle_SCALE();
	void handle_AXIS_SELECTION();
	void handle_Callibration();
	
	void handle_Create_Light();

	void handle_GROUP( std::string name = " " );
	void handle_UNGROUP();


	void handle_Attach_Sampler_Planar();
	void handle_Attach_Sampler_Volume();
	void handle_Attach_Sampler_Directional();

	class Restriction* handle_Attach_Restriction_Focus();
	class Restriction* handle_Attach_Restriction_Conversation();
	class Restriction* handle_Attach_Restriction_Distance();
	class Restriction* handle_Attach_Restriction_Focus_ObjToObj();
	class Restriction* handle_Attach_Restriction_Conversation_ObjToObj();
	class Restriction* handle_Attach_Restriction_Distance_ObjToObj();

	void handle_Object_Align();
	void handle_Camera_Align();

#endif

	// Intersection Routines - Functions
#ifdef Intersection_Routine_Manager

	
	// Utilities graphical environment properties
	bool get_active_utility_intersectionPoint(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility);
	

	// Intersect g_TRANSFORMATION utilities
	bool get_Utility_Interaction_g_TRANSFORMATIONS(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility);
	bool get_translation_utility_intersectionPoint(struct Ray * ray,Object_Package * obj_prop, optix::float3 &cam_w, float m_camera_inv_scale_factor, std::vector<IntersectionPoint>& potentialIntersectionPoints);
	bool get_rotation_utility_intersectionPoint(struct Ray * ray, Object_Package * obj_prop, optix::float3 &cam_w, float m_camera_inv_scale_factor, std::vector<IntersectionPoint>& potentialIntersectionPoints);
	bool get_scale_utility_intersectionPoint(struct Ray * ray, Object_Package * obj_prop, optix::float3 &cam_w, float m_camera_inv_scale_factor, std::vector<IntersectionPoint>& potentialIntersectionPoints);

	// Intersect g_ATTACH Utilities
	bool get_Utility_Interaction_g_ATTACH(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility);
	bool get_Utility_Interaction_g_ATTACH_FACE_VECTOR(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility);
	bool get_Utility_Interaction_g_ATTACH_SAMPLER(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility);

	bool get_Utility_Interaction_g_CREATE(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility);
	bool get_Utility_Interaction_g_CREATE_LIGHT(struct GLFWwindow * target_window, class PinholeCamera* target_camera, Object_Package * target_obj_properties, IntersectionPoint * isec_p, UtilityProperties interaction_utility);
	
	bool get_Light_toggleButton_Interaction(class PinholeCamera* target_camera, optix::float3& ip, SceneObject* light_object, int& obj_id);

#endif


};


void THR_callFunc_Load_Scene(const char * filename);