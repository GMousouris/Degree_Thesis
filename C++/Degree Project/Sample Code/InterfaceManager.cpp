#include "Defines.h"
#include "streamHandler.h"

#include "InterfaceManager.h"
#include "InterfaceManager_HelperFunctions.h"
#include "imgui_impl_glfw.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ImGuiStyleManager.h"
//#include "imguifilesystem.h"
#include "ImGuiFileDialog.h"


#include "SceneObject.h"
#include "PinholeCamera.h"

#include "AssetManager.h"
#include "Mediator.h"
#include "Utilities.h"
#include "GeometryFunctions.h"
#include "TextureManager.h"

#include "OpenGL_Renderer.h"
#include "Transformations.h"
#include "ActionManager.h"
#include "Action.h"
#include "Action_Childs.h"
#include "OptiXRenderer_structs.h"
#include "Restriction_Types.h"

#include "ImGuiFileBrowser.h"
#include <stack>

#include "imfilebrowser.h"
#include <fstream>

#include <chrono>

#include "Data_Get_Interface.h"


void THR_callFunc_Load_Scene(const char * filename)
{
	
	//std::cout << " - THR_callfunc_Load_Scene():" << std::endl;
	InterfaceManager::GetInstance().IMPORT_SCENE_THREADED( filename );
	// thread join ?
}



static bool show_filedialog = false;
static imgui_addons::ImGuiFileBrowser file_dialog;
static imgui_addons::ImGuiFileBrowser file_dialog_select_dir;
static ImGui::FileBrowser file_dialog_select_file(ImGuiFileBrowserFlags_CloseOnEsc | ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_EnterNewFilename);


InterfaceManager::InterfaceManager() {}
InterfaceManager::~InterfaceManager()
{
	guiStyleManager::cleanUp();
}


#ifdef INTERFACE_INIT

bool InterfaceManager::Init(GLFWwindow * main_app_window)
{

#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER) || defined(INIT_STATE)
	streamHandler::Report("InterfaceManager", "Init", "");
#endif

	m_selected_utility.id     = u_NONE;
	m_selected_utility.group  = g_NONE;
	m_selected_utility.active = false;

	m_utility_interaction = false;


	m_interfaceManager_init = false;
	m_fileChooser_open      = false;
	m_edit_Style_open       = false;

	// Main menu bar - tools vars
	m_selected_axis_orientation         = AXIS_ORIENTATION::WORLD;
	m_selected_axis_orientation_changed = false;
	m_active_cursor                     = CURSOR_TYPE::CURSOR_NONE;

	// Interface Panels(windows) vars
	m_interface_active_object = -1;


	// key events ui-states
	ui_key_grp_state   = UI_KEY_GRP_NONE;
	ui_key_comb_state  = UI_KEY_COMB_NONE;

	// main_app window
	m_window_width  = Mediator::RequestWindowSize(0);
	m_window_height = Mediator::RequestWindowSize(1);
	m_gui_focused   = false;

	Init_ImGui_Windows();
	Init_ViewportManager();
	Init_Renderer();
	Init_Renderer_RenderSize();
	
	Init_ImGui(main_app_window);
	Init_Renderer_Viewport();
	Init_tool_Buttons();
	Utilities::load_Default_Key_Preset(&key_data);
	
	m_m0_down_on_focused_object      = false;
	m_m0_down_on_utility_interaction = false;
	m_m0_down_on_gui                 = false;
	m_m2_down_on_gui                = false;
	m_transforming_active           = false;
	m_ui_focused                    = false;
	m_cloning_active                = false;
	m_cloning_group                 = false;
	m_cloning_finished              = false;
	m_retargeting_camera            = false;
	m_reselecting                   = false;
	m_popup_object_cloning_approval = false;
	m_popup_group_creating_approval = false;
	m_popup_create_light            = false;
	m_popup_create_sampler          = false;
	m_popup_import_file = false;
	m_popup_load_file = false;
	m_popup_save_file = false;
	m_skip_user_input               = false;
	m_skip_user_input_finished      = false;
	m_finished_area_selection       = false;
	m_multiple_selection_active     = false;
	m_wait_to_release               = false;
	m_wait_to_release_m2            = false;
	m_object_tree_focused = false;
	m_popup_open = false;
	m_force_input_update = false;

	m_analogue_callibration = true;

	is_m0_down                      = false;
	m0_down                         = optix::make_float2(-1, -1);

	as_copy          = true;
	as_instance      = false;
	approve_grouping = false;
	
	m_viewport_camera_active = false;

	timer = 0.0f;


	m_selection_setting    = SELECTION_SETTING::S_OBJECT;
	m_hierarchy_setting    = TRANSFORMATION_SETTING::TO_ALL;
	object_tree_max_depth  = 1;
	object_treeNode_width  = 220.f;
	object_treeNode_indent = 23.0f;
	

	

	return true;
}
bool InterfaceManager::Init_ViewportManager()
{
	return ViewportManager::GetInstance().Init();
}
bool InterfaceManager::Init_Renderer()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER) || defined(INIT_STATE)
	streamHandler::Report("InterfaceManager", "Init_Renderer", "");
#endif

	bool init_renderer = false;
	init_renderer = OpenGLRenderer::GetInstace().Init();
	return init_renderer;
}
bool InterfaceManager::Init_ImGui(GLFWwindow * main_app_window)
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER) || defined(INIT_STATE)
	streamHandler::Report("InterfaceManager", "Init_ImGui", "");
#endif

	ImGui::CreateContext();
	
	ImGui_ImplGlfwGL2_Init(main_app_window, true);

	guiStyleManager::LoadFonts();
	guiStyleManager::LoadStyles();

	return true;
}
bool InterfaceManager::Init_tool_Buttons()
{

	for (std::string utility_id : Utility_ids)
	{
		
		std::string label      = utility_id;
		Utility utility        = Utilities::convert_Utility_id_To_Utility( utility_id );
		std::string tooltip    = Utilities::Utility_Tooltip( utility );
		ImVec2 button_size     = BUTTON_SIZE;
		ImVec2 button_ratio    = BUTTON_RATIO;
		float button_padding   = BUTTON_PADDING;
		ImVec4 button_color_bg = BUTTON_COLOR_BG;
		
		Button * button = new Button(label, utility, button_size, button_ratio, button_padding, button_color_bg);
		button->tooltip = tooltip;
		tool_buttons[utility_id.c_str()] = button;
		
		if (utility == u_SELECTION || utility == u_CALLIBRATION_ANALOGUE || utility == u_SCENE_EDIT_MODE)
			tool_buttons[utility_id.c_str()]->selected = true;
		
	}

	return true;
}
bool InterfaceManager::Init_ImGui_Windows()
{

	ImGui_Window_Data main_tools;
	main_tools.pos       = ImVec2(0, 0);
	main_tools.size.y    = 57;
	main_tools.size.x    = 0;
	main_tools.resizable = 0;
	main_tools.movable   = 0;
	imgui_windows[MAIN_TOOLS] = main_tools;

	ImGui_Window_Data main_tools_helpers;
#ifdef TOOLS_HELPERS_BAR
	main_tools_helpers.pos       = ImVec2(0, 39);
	main_tools_helpers.size.y    = 10;
#else
	main_tools_helpers.pos       = ImVec2(0, 0);
	main_tools_helpers.size.y    = 0;
#endif
	main_tools_helpers.size.x    = 0;
	main_tools_helpers.resizable = 0;
	main_tools_helpers.movable   = 0;
	imgui_windows[MAIN_TOOLS_HELPERS] = main_tools_helpers;

	ImGui_Window_Data left_panel;
	left_panel.pos            = ImVec2( 0, main_tools.size.y + main_tools_helpers.size.y - 2.0f );
	left_panel.size           = ImVec2(270, 0);
	left_panel.size_delta     = ImVec2(0, -left_panel.pos.y);
	left_panel.resizable      = 0;
	left_panel.movable        = 0;
	imgui_windows[LEFT_PANEL] = left_panel;

	ImGui_Window_Data right_panel;
	right_panel.pos           = ImVec2(0, left_panel.pos.y + 2.0f);
	right_panel.size          = ImVec2(280, 0);
	right_panel.size_delta    = ImVec2(0, -right_panel.pos.y);
	right_panel.pos_delta     = ImVec2(-right_panel.size.x, -right_panel.size.y);
	right_panel.resizable     = 0;
	right_panel.movable       = 0;
	imgui_windows[RIGHT_PANEL] = right_panel;

	ImGui_Window_Data bottom_panel;
	bottom_panel.pos = ImVec2(0, 0);
	bottom_panel.size = ImVec2(0, 65);
	bottom_panel.size_delta = ImVec2(0, 0);
	bottom_panel.pos_delta = ImVec2(0, 0);
	bottom_panel.resizable = 0;
	bottom_panel.movable = 0;
	imgui_windows[BOTTOM_PANEL] = bottom_panel;

	return true;
}
bool InterfaceManager::Init_Renderer_RenderSize()
{
	m_renderer_renderSize.x = m_window_width;
	m_renderer_renderSize.y = m_window_height;
	OpenGLRenderer::GetInstace().setRenderSize( m_renderer_renderSize.x, m_renderer_renderSize.y );
	return true;
}
bool InterfaceManager::Init_Renderer_Viewport()
{
	
	int screen_width  = m_window_width;
	int screen_height = m_window_height;

	int width  = screen_width  - (imgui_windows[LEFT_PANEL].size.x + imgui_windows[RIGHT_PANEL].size.x);
	int height = screen_height - (imgui_windows[MAIN_TOOLS].size.y + imgui_windows[MAIN_TOOLS_HELPERS].size.y);

	int offset_x = imgui_windows[LEFT_PANEL].size.x;
	int offset_y = imgui_windows[MAIN_TOOLS].size.y + imgui_windows[MAIN_TOOLS_HELPERS].size.y;

	//OpenGLRenderer::GetInstace().setActiveViewPort(0);
	//OpenGLRenderer::GetInstace().setViewport(0, 0, 0, m_renderer_renderSize.x, m_renderer_renderSize.y);

	return true;
}

#endif

#ifdef INTERFACE_UI_CONSTRUCTORS

void InterfaceManager::construct_Menu_Items()
{
	std::string label;
	std::string ut_label = "";
	label += " File";
	guiStyleManager::setIconFont(MDI, 16);
	bool is_focused = false;

	// FILE //
	if (ImGui::BeginMenu("  File  "))
	{
		is_focused |= ImGui::IsItemHovered();

		ut_label = "Import Object      ";
		label = std::string("  ") + ICON_MDI_FILE_UPLOAD_OUTLINE; label += std::string("  ") + ut_label + std::string("        ");
		ImGui::Dummy(ImVec2(20.0f, 2.0f));
		if (ImGui::MenuItem(label.c_str(), NULL))
		{
			m_fileChooser_open = true;
			m_popup_import_file = true;
			m_popup_import_file_toggle = true;
			m_fileChooser_select_dir_action = IMPORT_OBJECT;
			file_dialog_select_file.ac_id = m_fileChooser_select_dir_action;
		}
		is_focused |= ImGui::IsItemHovered();

		ImGui::Separator();

		ut_label = "Load Scene      ";
		label = std::string("  ") + ICON_MDI_FOLDER_UPLOAD; label += std::string("  ") + ut_label + std::string("        ");
		if (ImGui::MenuItem(label.c_str(), NULL))
		{
			m_fileChooser_open = true;
			m_popup_import_file = true;
			m_popup_import_file_toggle = true;
			m_fileChooser_select_dir_action = LOAD_SCENE;
			file_dialog_select_file.ac_id = m_fileChooser_select_dir_action;
		}
		is_focused |= ImGui::IsItemHovered();


		ut_label = "Save Scene      ";
		label = std::string("  ") + ICON_MDI_CONTENT_SAVE; label += std::string("  ") + ut_label + std::string("        ");
		if (ImGui::MenuItem(label.c_str(), NULL))
		{
			m_fileChooser_open = true;
			m_popup_import_file = true;
			m_popup_import_file_toggle = true;
			m_fileChooser_select_dir_action = SAVE_SCENE;
			file_dialog_select_file.ac_id = m_fileChooser_select_dir_action;
		}
		is_focused |= ImGui::IsItemHovered();



		ImGui::Separator();

		ut_label = "Export Scene     ";
		label = std::string("  ") + ICON_MDI_APPLICATION_EXPORT; label += std::string("  ") + ut_label + std::string("        ");
		if (ImGui::MenuItem(label.c_str(), NULL))
		{
			m_fileChooser_open = true;
			m_popup_import_file = true;
			m_popup_import_file_toggle = true;
			m_fileChooser_select_dir_action = EXPORT_SCENE;
			file_dialog_select_file.ac_id = m_fileChooser_select_dir_action;
		}
		is_focused |= ImGui::IsItemHovered();

		ImGui::Dummy(ImVec2(20.0f, 4.0f));
		ImGui::EndMenu();
	}
	is_focused |= ImGui::IsAnyItemHovered();

	// EDIT //
	if (ImGui::BeginMenu("  Edit  "))
	{
		is_focused |= ImGui::IsItemHovered();

		ImGui::Dummy(ImVec2(20.0f, 2.0f));
		
		ut_label = "Undo";
		label = std::string("  ") + ICON_MDI_UNDO; label += std::string("  ") + ut_label + std::string("        ");
		if (ImGui::MenuItem(label.c_str(), "CTRL+Z")) 
		{ 
			handle_UNDO();
		}
		is_focused |= ImGui::IsItemHovered();

		ut_label = "Redo";
		label = std::string("  ") + ICON_MDI_REDO; label += std::string("  ") + ut_label + std::string("        ");
		if (ImGui::MenuItem(label.c_str(), "CTRL+Y", false, false)) 
		{
			handle_REDO();
		}  
		is_focused |= ImGui::IsItemHovered();

		ImGui::Separator();
		
		if (ImGui::MenuItem("   Edit Style    "))
		{
			m_edit_Style_open = true;
		}
		is_focused |= ImGui::IsItemHovered();

		ImGui::Dummy(ImVec2(20.0f, 4.0f));
		ImGui::EndMenu();
	}
	is_focused |= ImGui::IsAnyItemHovered();

	// GROUP //
	if (ImGui::BeginMenu("  Group  "))
	{
		is_focused |= ImGui::IsItemHovered();

		ImGui::Dummy(ImVec2(20.0f, 2.0f));
		bool can_group        = false;
		bool single_selection = false;
		// case : Area selection is active
		if (m_multiple_selection_active)
		{
			can_group = m_area_selections_data[m_multiple_selection_active_id].can_group;
		}
		// case : single selection is active
		else
		{
			can_group        = get_Grouping_Information( m_selected_object_pack.raw_object );
			single_selection = true;
		}

		ImVec4 disabled_color = ImVec4(0.6, 0.6, 0.6, 1.0f);
		if (can_group)
			ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		else
			ImGui::GetStyle().Colors[ImGuiCol_Text] = disabled_color;
		
		
		// handle_GROUPING :
		ut_label = "Group";
		label = std::string("  ") + ICON_MDI_GROUP; label += std::string("  ") + ut_label + std::string("        ");
		if (ImGui::MenuItem(label.c_str()) && can_group)
		{
			m_popup_group_creating_approval = true;
			//handle_GROUP();
		}
		is_focused |= ImGui::IsItemHovered();

		ImGui::Separator();

		if (m_selected_object_pack.raw_object != nullptr)
		{
			if (m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY && !m_selected_object_pack.raw_object->isTemporary())
				ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			else
				ImGui::GetStyle().Colors[ImGuiCol_Text] = disabled_color;
		}


		// handle_UNGROUPING :
		ut_label = "Ungroup";
		label = std::string("  ") + ICON_MDI_UNGROUP; label += std::string("  ") + ut_label + std::string("        ");
		if (ImGui::MenuItem(label.c_str()))
		{
			handle_UNGROUP();
		}
		is_focused |= ImGui::IsItemHovered();

		//ImGui::Separator();
		ImGui::Dummy(ImVec2(20.0f, 4.0f));

		ImGui::EndMenu();
		ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	is_focused |= ImGui::IsItemHovered(); //|| ImGui::IsAnyItemFocused();

	// CREATE //
	if (ImGui::BeginMenu("  Create  "))
	{
		is_focused |= ImGui::IsItemHovered();

		ImGui::Dummy(ImVec2(20.0f, 4.0f));
		
		
		ut_label = "Light";
		label = std::string("  ") + ICON_MDI_SPOTLIGHT_BEAM; label += std::string("  ") + ut_label + std::string("        ") + std::string("           ");
		if (ImGui::BeginMenu(label.c_str()))
		{
			ImGui::Dummy(ImVec2(0.0f, 4.0f));

			Light_Type light_type;
			bool chose = false;

			
			ut_label = "Quad Light";
			label = std::string("  ") + ICON_MDI_WALL_SCONCE_FLAT; label += std::string("  ") + ut_label + std::string("        ") + std::string("           ");
			if (ImGui::MenuItem(label.c_str()))
			{
				light_type = QUAD_LIGHT;
				chose = true;
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();

			ut_label = "Spotlight";
			label = std::string("  ") + ICON_MDI_TRACK_LIGHT; label += std::string("  ") + ut_label + std::string("        ") + std::string("           ");
			if (ImGui::MenuItem(label.c_str()))
			{
				light_type = SPOTLIGHT;
				chose = true;
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();

			
			//ICON_MDI_GLOBE_MODEL
			ut_label = "Spherical Light";
			label = std::string("  ") + ICON_MDI_CIRCLE_SLICE_8; label += std::string("  ") + ut_label + std::string("        ") + std::string("           ");
			if (ImGui::MenuItem(label.c_str()))
			{
				light_type = SPHERICAL_LIGHT;
				chose = true;
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();


			ut_label = "Point Light";
			//ICON_MDI_CIRCLE_MEDIUM
			label = std::string("  ") + ICON_MDI_CIRCLE_MEDIUM; label += std::string("  ") + ut_label + std::string("        ") + std::string("           ");
			if (ImGui::MenuItem(label.c_str()))
			{
				light_type = POINT_LIGHT;
				chose = true;
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();



			//if( chose )
			//	Mediator::addSceneLight(light_type);
			if (chose)
			{
				m_light_crt_def_type = light_type;
				m_interface_state.m_light_params.type = light_type;
				toggle(u_CREATE_LIGHT, false, true);
			}
			
			ImGui::Dummy(ImVec2(0.0f, 4.0f));

			ImGui::EndMenu();
		}
		is_focused |= ImGui::IsItemHovered();


		ImGui::Dummy(ImVec2(20.0f, 4.0f));
		ImGui::EndMenu();
	}
	is_focused |= ImGui::IsAnyItemHovered();

	// ATTACH //
	if (ImGui::BeginMenu("  Attach  "))
	{

		is_focused |= ImGui::IsItemHovered();

		ImGui::Dummy(ImVec2(20.0f, 4.0f));
		

		ut_label = "Attach Face Vector";
		label = std::string("  ") + ICON_MDI_ARROW_EXPAND_RIGHT; label += std::string("  ") + ut_label + std::string("        ") + std::string("##");
		if (ImGui::BeginMenu(label.c_str()))
		{
			ImGui::Dummy(ImVec2(20.0f, 2.0f));

			ut_label = "Attach Face Vector ( Bbox - Aligned )";
			label = std::string("  ") + ICON_MDI_ARROW_SPLIT_VERTICAL; label += std::string("  ") + ut_label + std::string("        ") + std::string("##");
			if (ImGui::MenuItem(label.c_str()))
			{
				toggle(u_ATTACH_FACE_VECTOR_BBOX_ALIGNED_PARALLEL, false, true);
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();

			ut_label = "Attach Face Vector ( Bbox - Variable )";
			label = std::string("  ") + ICON_MDI_ANGLE_ACUTE; label += std::string("  ") + ut_label + std::string("        ") + std::string("##");
			if (ImGui::MenuItem(label.c_str()))
			{
				toggle(u_ATTACH_FACE_VECTOR_BBOX_ALIGNED, false, true);
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();

			ut_label = "Attach Face Vector ( Normal Aligned )";
			label = std::string("  ") + ICON_MDI_ANGLE_RIGHT; label += std::string("  ") + ut_label + std::string("        ") + std::string("##");
			if (ImGui::MenuItem(label.c_str()))
			{
				toggle(u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED, false, true);
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();

			ImGui::Dummy(ImVec2(20.0f, 4.0f));
			ImGui::EndMenu();
		}
		is_focused |= ImGui::IsItemHovered();


		ImGui::Separator();


		ut_label = "Attach Sampler";
		label = std::string("  ") + ICON_MDI_SHAPE_PLUS; label += std::string("  ") + ut_label + std::string("        ") + std::string("           ");
		if (ImGui::BeginMenu(label.c_str()))
		{
			ImGui::Dummy(ImVec2(20.0f, 4.0f));
			
			ut_label = "Planar Sampler";
			label = std::string("  ") + ICON_MDI_SQUARE; label += std::string("  ") + ut_label + std::string("        ") + std::string("           ");
			if (ImGui::MenuItem(label.c_str()))
			{
				toggle(u_ATTACH_SAMPLER_PLANAR, false, true);
				//toggle_Attach_Sampler_Planar();
				m_sampler_crt_def_type = 1;
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();

			ut_label = "Volume Sampler";
			label = std::string("  ") + ICON_MDI_CUBE_OUTLINE; label += std::string("  ") + ut_label + std::string("        ") + std::string("           ");
			if (ImGui::MenuItem(label.c_str()))
			{
				toggle(u_ATTACH_SAMPLER_PLANAR, false, true);
				//toggle_Attach_Sampler_Volume();
				m_sampler_crt_def_type = 2;
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();

			ut_label = "Directional Sampler";
			label = std::string("  ") + ICON_MDI_TRIANGLE_OUTLINE; label += std::string("  ") + ut_label + std::string("        ") + std::string("           ");
			if (ImGui::MenuItem(label.c_str()))
			{
				toggle(u_ATTACH_SAMPLER_PLANAR, false, true);
				//toggle_Attach_Sampler_Directional();
				m_sampler_crt_def_type = 3;
			}
			is_focused |= ImGui::IsItemHovered() || ImGui::IsItemFocused();

			ImGui::Dummy(ImVec2(20.0f, 4.0f));
			ImGui::EndMenu();
		}
		is_focused |= ImGui::IsItemHovered();
		
		
		ImGui::Dummy(ImVec2(20.0f, 4.0f));
		ImGui::EndMenu();
	}
	is_focused |= ImGui::IsAnyItemHovered();
	
	ImGui::PopFont();

	m_focusing_menu_bar_items |= is_focused;
}
void InterfaceManager::construct_Tools()
{

	guiStyleManager::setStyle("tools_style");
	float but_width_offset = 37.8f;
	float col_offset  = 0.08f;
	float dummy_width = 0.5f;

	ImGui::Spacing();

	ImGui::BeginColumns("", 24);
	ImGui::SetColumnWidth(0, but_width_offset * 2.f);    // : undo - redo
	ImGui::SetColumnWidth(1, but_width_offset * dummy_width);
	ImGui::SetColumnWidth(2, but_width_offset * 2.f);    // : link - unlink
	ImGui::SetColumnWidth(3, but_width_offset * dummy_width);
	ImGui::SetColumnWidth(4, but_width_offset * 2.95f);    // : selections
	ImGui::SetColumnWidth(5, but_width_offset * dummy_width);   // : dummy ( spacing )
	ImGui::SetColumnWidth(6, but_width_offset * 2.95f);  // : transformations


	ImGui::SetColumnWidth(7,  but_width_offset * 4.0f);   // : axis_selection //3.5
	ImGui::SetColumnWidth(8,  but_width_offset * 1.95f);    // : callibration
	ImGui::SetColumnWidth(9,  but_width_offset * dummy_width);  
	ImGui::SetColumnWidth(10, but_width_offset * 1.0f);  // lights
	ImGui::SetColumnWidth(11, but_width_offset * dummy_width);
	ImGui::SetColumnWidth(12, but_width_offset * 1.0f); // samplers
	ImGui::SetColumnWidth(13, but_width_offset * dummy_width);
	ImGui::SetColumnWidth(14, but_width_offset * 2.95f); // face vectors
	ImGui::SetColumnWidth(15, but_width_offset * dummy_width);   
	ImGui::SetColumnWidth(16, but_width_offset * 2.95f);  // restrictions

	ImGui::SetColumnWidth(17, but_width_offset * dummy_width);
	ImGui::SetColumnWidth(18, but_width_offset * 1.95f); // Align

	ImGui::SetColumnWidth(19, but_width_offset * dummy_width * 24.75f); // (20.0,  24.75 , 34.5) 
	ImGui::SetColumnWidth(20, but_width_offset * 1 ); // dummy offset column

	ImGui::SetColumnWidth(21, but_width_offset * dummy_width);
	ImGui::SetColumnWidth(22, but_width_offset * 2.95f); // Scene Modes

	ImGui::SetColumnWidth(23, but_width_offset * dummy_width );
	//



	//0. UNDO - REDO
	{
		// undo
		tool_buttons["undo"]->construct();

		// redo
		ImGui::SameLine();
		tool_buttons["redo"]->construct();
	}

	//1. DUMMY - SPACING
	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));

	// LINKING
	{
		ImGui::NextColumn();

		// link
		tool_buttons["link"]->construct();

		//ImVec2 pos = ImGui::GetCur


		// unlink
		ImGui::SameLine();
		tool_buttons["unlink"]->construct();
	}


	//2. DUMMY - SPACING
	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));

	// SELECTIONS
	{
		ImGui::NextColumn();

		// selection
		tool_buttons["selection"]->construct();

		// selection_group
		ImGui::SameLine();
		tool_buttons["selection_group"]->construct();

		// selection_area
		ImGui::SameLine();
		tool_buttons["selection_area"]->construct();
	}



	//3. DUMMY - SPACING
	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(8.0f, 7.0f), 1.0f);


	// TRANSFORMATIONS	
	{
		ImGui::NextColumn();

		// Translation
		tool_buttons["translation"]->construct();


		// Rotation
		ImGui::SameLine();
		tool_buttons["rotation"]->construct();


		// scale
		ImGui::SameLine();
		tool_buttons["scale"]->construct();
	}


	// AXIS - SELECTION
	{
		ImGui::NextColumn();
		ImGui::Dummy(ImVec2(0.0f, 2.8f));
		ImGui::Dummy(ImVec2(0.2f, 0.0f));
		ImGui::SameLine();

		// axis selection
		{
			guiStyleManager::setStyle("tools_style2");
			guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 16);
			if (ImGui::BeginCombo("Axis##", (" " + AxisOrientationToString(m_selected_axis_orientation)).c_str()))
			{
				toggle_AXIS_SELECTION();
				ImGui::EndCombo();
			}
			//ImGui::PopFont();
			guiStyleManager::setStyle("tools_style");
		}
	}

	
	// CALLIBRATION
	{
		ImGui::NextColumn();

		tool_buttons["callibration_analogue"]->construct();

		ImGui::SameLine();
		tool_buttons["callibration_fixed"]->construct();
	}


	//9. DUMMY - SPACING
	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));

	//ImGui::NextColumn();
	//ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));

	// 10. CREATE
	{

		ImGui::NextColumn();
		tool_buttons["create_light"]->construct();
		//ImGui::SameLine();
		//tool_buttons["create_camera"]->construct();
		
		//ImGui::SameLine();
		//tool_buttons["create_sampler"]->construct();
	}


	// 11.
	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));


	// 12. Samplers
	{
		ImGui::NextColumn();
		tool_buttons["sampler_planar"]->construct();
		
		//ImGui::SameLine();
		//tool_buttons["sampler_volume"]->construct();
		
		//ImGui::SameLine();
		//tool_buttons["sampler_directional"]->construct();
	}

	// 13.
	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));

	// 14. Face Vector
	{
		ImGui::NextColumn();
		tool_buttons["face_vector_bbox_aligned_parallel"]->construct();
		ImGui::SameLine();
		tool_buttons["face_vector_bbox_aligned"]->construct();
		ImGui::SameLine();
		tool_buttons["face_vector_normal_aligned"]->construct();
		
	}

	
	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));

	// 15. Restrictions
	{
		ImGui::NextColumn();
		tool_buttons["restriction_focus"]->construct();
		ImGui::SameLine();
		tool_buttons["restriction_conversation"]->construct();
		ImGui::SameLine();
		tool_buttons["restriction_distance"]->construct();

	}

	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));

	// 16. Align
	{
		ImGui::NextColumn();
		tool_buttons["object_align"]->construct();
		
		ImGui::SameLine();
		tool_buttons["object_camera_align"]->construct();
		
		//ImGui::SameLine();
		//tool_buttons["restriction_distance"]->construct();
	}


	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));
	

	// 17. (dummy Column)
	{
		ImGui::NextColumn();
		//ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));
	}

	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));


	// 18. Scene Modes
	{
		ImGui::NextColumn();
		tool_buttons["scene_edit_mode"]->construct();

		ImGui::SameLine();
		tool_buttons["scene_render_mode"]->construct();

		ImGui::SameLine();
		tool_buttons["scene_toggle_lights"]->construct();
	}


	ImGui::NextColumn();
	ImGui::Custom_Separator(ImVec2(7.0f, 7.0f));


	ImGui::EndColumns();
	//ImGui::PopStyleColor();

}
void InterfaceManager::construct_Tools_Helpers()
{
	ImVec2 button_size(36, 10);
	ImVec2 ratio(1.0f, 1.0f);
	float padding = -1.0f;
	ImVec4 bg_color(0.0f, 0.0f, 0.0f, 0.0f);

	ImGui::NextColumn();
	ImGui::NextColumn();

	ImGui::NextColumn();
	ImGui::NextColumn();

	// SELECTION_SETTINGS
	{
		guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
		{
			std::string title = "";
			switch (m_selection_setting)
			{
			case S_OBJECT: title = "Object"; break;
			case S_GROUP:  title = "Group";  break;
			case S_TOP_LEVEL_GROUP:  title = "Top Level Group";  break;
			}


			ImGui::PushItemWidth(75.0f);
			if (ImGui::BeginCombo("##", title.c_str()))
			{
				guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 16);
				if (ImGui::Selectable("Object"))
					m_selection_setting = SELECTION_SETTING::S_OBJECT;
				if (ImGui::Selectable("Group"))
					m_selection_setting = SELECTION_SETTING::S_GROUP;
				if (ImGui::Selectable("Top Level Group"))
					m_selection_setting = SELECTION_SETTING::S_TOP_LEVEL_GROUP;

				ImGui::PopFont();
				ImGui::EndCombo();
			}
		}
		ImGui::PopFont();
	}



	ImGui::NextColumn();
	ImGui::NextColumn();
	//ImGui::Dummy(ImVec2(0.0f, 2.8f));
	//ImGui::Dummy(ImVec2(0.2f, 0.0f));
	//ImGui::SameLine();


	//ImGuiComboFlags comb_f = ImGuiComboFlags_HeightLargest;
	ImGui::PushItemWidth(110.0f);
	
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
	toggle_Hierarchy_Settings();
	
	ImGui::PopFont();
	ImGui::PopItemWidth();


	ImGui::NextColumn();
	ImGui::NextColumn();
	ImGui::NextColumn();
	ImGui::NextColumn();


	ImGui::NextColumn();
}

void InterfaceManager::construct_Interface()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "construct_Interface", "");
#endif


	// reset vars every frame
	m_ui_restrictions_panel_focused = false;
	m_gui_focused =
		ImGui::IsAnyItemFocused()
		|| ImGui::IsAnyWindowHovered()
		|| ImGui::IsMouseHoveringAnyWindow()
		|| ImGui::IsAnyItemActive()
		//|| ImGui::IsAnyItemHovered()
		//|| ImGui::IsPosHoveringAnyWindow(ImGui::GetCursorPos())
		//|| ImGui::IsAnyItemFocused() 
		//|| ImGui::IsAnyWindowFocused() 
		//|| m_focusing_menu_bar_items
		|| ImGui::IsRootWindowOrAnyChildFocused();

	construct_FileBrowser();
	construct_MainMenuBar_2();
	construct_MainMenuBar();
	construct_MainMenuBar_Helpers();
	construct_LeftPanel();
	construct_RightPanel();
	construct_BottomPanel();
	construct_Viewports_Frame_Settings();
	
	m_popup_open = m_popup_create_light || m_popup_create_sampler || m_popup_group_creating_approval || m_popup_group_creating_approval 
		|| m_popup_object_cloning_approval || m_popup_import_file || m_popup_crt_dir || m_popup_viewport_settings
		|| m_popup_object_align_window || m_popup_camera_align_window;

	if (m_popup_object_cloning_approval)
		pop_Object_cloning_Approval_window();
	if (m_popup_group_creating_approval)
		pop_Group_creating_Approval_window();
	if (m_popup_create_light)
		pop_Create_Light_Window();
	if (m_popup_create_sampler)
		pop_Create_Sampler_Window();
	if (m_popup_object_align_window)
		pop_Object_Align_Window();
	if (m_popup_camera_align_window)
		pop_Camera_Align_Window();
	if (m_popup_tooltip)
		show_Tooltip();

	if (m_edit_Style_open)
		edit_style();

}

void InterfaceManager::construct_MainMenuBar()
{

#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "construct_MainMenuBar", "");
#endif

	guiStyleManager::setStyle("menu_bar_style");
	std::string label;
	// Main Menu bar && Tools panel
	{
		float off = 18.0f;
		ImGui_Window_Data window = imgui_windows[MAIN_TOOLS];
		ImGui::SetNextWindowPos(window.pos + ImVec2(0.0f,18.0f));
		ImGui::SetNextWindowSize(ImVec2(m_window_width, window.size.y - off));
		ImGui::SetNextWindowCollapsed(false);
		ImGui::SetNextWindowSizeConstraints(ImVec2(m_window_width, window.size.y - off), ImVec2(m_window_width, window.size.y - off));
		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.1, 0.1, 0.0f));

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		//window_flags |= ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
		//window_flags |= ImGuiWindowFlags_NoDecoration;
		//window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		

		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameBorderSize = 1.0f;
		ImGui::Begin("##", NULL, window_flags);
		
//#define CONSTRUCT_MAIN_MENU_BAR_INSIDE_TOP_WINDOW
#ifdef CONSTRUCT_MAIN_MENU_BAR_INSIDE_TOP_WINDOW

	
#else
		//ImGui::Dummy(ImVec2(0.0f, 12.0f));
#endif

		ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(76, 76, 76, 1.0f);
		//style.FrameBorderSize = 0.0f;

		// Tools panel
		{
			construct_Tools();
			guiStyleManager::setStyle("menu_bar_style");
		}

		//style.FrameBorderSize = 1.0f;
		ImGui::End();
		//style.FrameBorderSize = 0.0f;
	}


	//m_hovering_over_menu_bar_item = ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
	//ImGui::PopFont();
	//
}
void InterfaceManager::construct_MainMenuBar_2()
{

#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "construct_MainMenuBar", "");
#endif

	guiStyleManager::setStyle("main_menu_bar_style");
	std::string label;
	// Main Menu bar && Tools panel
	{
		ImGui_Window_Data window = imgui_windows[MAIN_TOOLS];
		ImGui::SetNextWindowPos(window.pos - ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(m_window_width, 0));
		ImGui::SetNextWindowCollapsed(false);
		ImGui::SetNextWindowSizeConstraints(ImVec2(m_window_width, 0), ImVec2(m_window_width, 0));

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		window_flags |= ImGuiWindowFlags_MenuBar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		window_flags |= ImGuiWindowFlags_NoScrollWithMouse;


		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameBorderSize = 1.0f;
		ImGui::Begin("asdasdd##", NULL, window_flags);

		m_focusing_menu_bar_items = false;
		m_focusing_menu_bar_items |= ImGui::IsWindowHovered();
		m_focusing_menu_bar_items |= ImGui::IsAnyWindowHovered();

		if (ImGui::BeginMenuBar())
		{
			m_focusing_menu_bar_items |= ImGui::IsItemHovered();
			construct_Menu_Items();

			ImGui::EndMenuBar();
			
		}
		//m_focusing_menu_bar_items |= ImGui::IsAnyItemActive();

		ImGui::End();
	}


	//m_hovering_over_menu_bar_item = ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
	//ImGui::PopFont();
	//
}

void InterfaceManager::construct_MainMenuBar_Helpers()
{

#ifndef TOOLS_HELPERS_BAR
	return;
#endif

#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "construct_MainMenuBar_Helpers", "");
#endif

	std::string label;
	// Tools panel
	{

		ImGui_Window_Data window = imgui_windows[MAIN_TOOLS_HELPERS];
		guiStyleManager::setStyle("tools_helpers");

		
		ImGui::SetNextWindowPos(ImVec2(window.pos.x, window.pos.y + 2)); // -16
		ImGui::SetNextWindowSize(ImVec2(m_window_width, window.size.y));
		ImGui::SetNextWindowCollapsed(false);
		//ImGui::SetNextWindowSizeConstraints(ImVec2(m_window_width, window.size.y), ImVec2(m_window_width, window.size.y));
		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.1, 0.1, 0.0f));

		ImGuiWindowFlags window_flags = 0;

		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;


		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameBorderSize = 1.0f;


		ImGui::Begin("##2", NULL, window_flags);
		style.FrameBorderSize = 0.0f;

		ImGui::Dummy(ImVec2(0.0f, 12.0f));
		// Tools panel
		{
			/*
			float but_width_offset = 37.8f;
			float col_offset = 0.08f;
			ImGui::Spacing();
			ImGui::BeginColumns("", 8);
			ImGui::SetColumnWidth(0, but_width_offset * 2.f); //
			ImGui::SetColumnWidth(1, but_width_offset * 2.f); //ImGui::SetColumnOffset(0, -20.0f);
			ImGui::SetColumnWidth(2, but_width_offset * 2.f);
			ImGui::SetColumnWidth(3, but_width_offset * 2.95f);
			ImGui::SetColumnWidth(4, but_width_offset * 3.5f);
			ImGui::SetColumnWidth(5, but_width_offset * 3.f);
			ImGui::SetColumnWidth(6, but_width_offset * 2.f); ////////
			ImGui::SetColumnWidth(7, but_width_offset * 2.f); ////////
			*/

			float but_width_offset = 37.8f;
			float col_offset = 0.08f;
			float dummy_width = 0.5f;
			ImGui::Spacing();
			ImGui::BeginColumns("", 12);
			ImGui::SetColumnWidth(0, but_width_offset * 2.f);    // : undo - redo
			ImGui::SetColumnWidth(1, but_width_offset * dummy_width);
			ImGui::SetColumnWidth(2, but_width_offset * 2.f);    // : link - unlink
			ImGui::SetColumnWidth(3, but_width_offset * dummy_width);
			ImGui::SetColumnWidth(4, but_width_offset * 2.95f);    // : selections
			ImGui::SetColumnWidth(5, but_width_offset * dummy_width);   // : dummy ( spacing )
			ImGui::SetColumnWidth(6, but_width_offset * 2.95f);  // : transformations

			ImGui::SetColumnWidth(7, but_width_offset * 3.5f);   // : axis_selection
			ImGui::SetColumnWidth(8, but_width_offset * 2.f);    // : callibration
			ImGui::SetColumnWidth(9, but_width_offset * dummy_width);   // : dummy ( spacing )
			ImGui::SetColumnWidth(10, but_width_offset * 2.95f);  // 
			ImGui::SetColumnWidth(11, but_width_offset * 2.f);    //

			construct_Tools_Helpers();
			
			ImGui::EndColumns();
			
			
		}
		//ImGui::PopStyleColor();

		style.FrameBorderSize = 1.0f;
		ImGui::End();
		style.FrameBorderSize = 0.0f;

		//ImGui::PopFont();
	}

	//
}
void InterfaceManager::construct_LeftPanel()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "construct_LeftPanel", "");
#endif

#ifdef PRINT_DEBUG_INFO_CLONE_DELETE
	//std::cout << "\n\n - - - - - - - construct_LeftPanel() - - - - - - - -" << std::endl;
#endif

	
	guiStyleManager::setStyle("main_style_leftPanel");
	ImGui_Window_Data window = imgui_windows[LEFT_PANEL];
	ImGui::SetNextWindowPos(window.pos);
	ImGui::SetNextWindowSize(ImVec2(window.size.x - 2, m_window_height - (window.pos.y + imgui_windows[BOTTOM_PANEL].size.y + 0.0f )));


	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	
	ImGui::Begin("Optix Project", NULL, window_flags);
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

	ImGui::Dummy(ImVec2(0.0f, 3.0f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 2.0f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	// LEFT CHILD ( COLUMN )
	guiStyleManager::setStyle("child_style");
	ImGui::BeginChild("child##",ImVec2(24.0f, ImGui::GetWindowHeight() - 20.0f), true, 0);
	{
		ImVec4 but_bg        = ImGui::GetStyle().Colors[ImGuiCol_Button];
		ImVec4 but_bg_active = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] * 0.8f; but_bg_active.w = 1.0f;

		m_object_tree_view_obj ? ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg_active : ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg;
		if (ImGui::Button(ICON_FA_CUBE, ImVec2(24, 24)))
		{
			m_object_tree_view_obj = !m_object_tree_view_obj;

		}ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg; ImGui::Dummy(ImVec2(0.0f, 2.0f));

		
		m_object_tree_view_grp ? ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg_active : ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg;
		if (ImGui::Button(ICON_FK_OBJECT_GROUP, ImVec2(24, 24)))
		{
			m_object_tree_view_grp = !m_object_tree_view_grp;

		}ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg; ImGui::Dummy(ImVec2(0.0f, 2.0f));

		
		guiStyleManager::setIconFont(MDI, 16);
		
		m_object_tree_view_lights ? ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg_active : ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg;
		if (ImGui::Button(ICON_MDI_SPOTLIGHT_BEAM, ImVec2(24, 24)))
		{
			m_object_tree_view_lights = !m_object_tree_view_lights;

		}ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg; ImGui::Dummy(ImVec2(0.0f, 2.0f));


		m_object_tree_view_fv ? ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg_active : ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg;
		if (ImGui::Button(ICON_MDI_ARROW_EXPAND_RIGHT, ImVec2(24, 24)))
		{
			m_object_tree_view_fv = !m_object_tree_view_fv;

		}ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg; ImGui::Dummy(ImVec2(0.0f, 2.0f));
		
		
		m_object_tree_view_smplr ? ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg_active : ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg;
		if (ImGui::Button(ICON_MDI_SHAPE, ImVec2(24, 24)))
		{
			m_object_tree_view_smplr = !m_object_tree_view_smplr;

		}ImGui::GetStyle().Colors[ImGuiCol_Button] = but_bg;

		ImGui::PopFont();



		// viewport layout button
		GLuint * button_icon = 0;
		ImVec2 butt_size = ImVec2(53.33, 40);
		ImVec2 butt_ratio = ImVec2(1, 1);
		float butt_padding = 0.0f;
		ImVec4 butt_bg_col = ImVec4(0, 0, 0, 0);
		ImVec2 spacing   = ImVec2(4.0f, 1.0f);
		ImVec2 h_spacing = ImVec2(4.0f, 0.0f);
		ImVec2 v_spacing = ImVec2(0.0f, 3.0f);
		int index = -1;
		guiStyleManager::setIconFont(Icon_Font::MDI, 20);

		ImGui::Dummy(ImVec2(0, ImGui::GetWindowHeight() - 5 * 24.0f - 36.0f));
		guiStyleManager::setStyle("viewports_frame_style2");
		if(ImGui::Button(ICON_MDI_MENU_RIGHT_OUTLINE, ImVec2(24, 24)))
		{
			ImGui::OpenPopup("viewport_style_popup");
		}
		if (ImGui::BeginPopup("viewport_style_popup"))
		{
			ImGui::Dummy(ImVec2(10.0f, 8.0f));

			ImGuiStyle &style = ImGui::GetStyle();
			ImVec4 col = style.Colors[ImGuiCol_Button];
			ImVec4 hcol = ImVec4(0.49, 0.41, 0.17,1.0f)*1.1f;
			
			// 1st row
			ImGui::Dummy(h_spacing); ImGui::SameLine();
			
			if (ViewportManager::GetInstance().isActiveViewportPreset(0)) style.Colors[ImGuiCol_Button] = hcol;
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c0_0");
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 0; } ImGui::SameLine(); ImGui::Dummy(spacing); ImGui::SameLine(); //1_vp_0
			style.Colors[ImGuiCol_Button] = col;

			if (ViewportManager::GetInstance().isActiveViewportPreset(4)) style.Colors[ImGuiCol_Button] = hcol;
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c1_0");
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 4; } ImGui::SameLine(); ImGui::Dummy(spacing); ImGui::SameLine(); //2_vp_0
			style.Colors[ImGuiCol_Button] = col;

			if (ViewportManager::GetInstance().isActiveViewportPreset(8)) style.Colors[ImGuiCol_Button] = hcol;
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c2_0");
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 8; }
			style.Colors[ImGuiCol_Button] = col;

			ImGui::SameLine(); ImGui::Dummy(h_spacing + ImVec2(2, 0));
			//

			ImGui::Dummy(v_spacing);


			// 2nd row
			ImGui::Dummy(h_spacing); ImGui::SameLine();


			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c0_1");
			style.Colors[ImGuiCol_Button] = col;

			if (ViewportManager::GetInstance().isActiveViewportPreset(1)) style.Colors[ImGuiCol_Button] = hcol;
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 1; } ImGui::SameLine(); ImGui::Dummy(spacing); ImGui::SameLine(); //1_vp_0
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c1_1");
			style.Colors[ImGuiCol_Button] = col;

			if (ViewportManager::GetInstance().isActiveViewportPreset(5)) style.Colors[ImGuiCol_Button] = hcol;
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 5; } ImGui::SameLine(); ImGui::Dummy(spacing); ImGui::SameLine(); //2_vp_0
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c2_1");
			style.Colors[ImGuiCol_Button] = col;

			if (ViewportManager::GetInstance().isActiveViewportPreset(9)) style.Colors[ImGuiCol_Button] = hcol;
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 9; }
			style.Colors[ImGuiCol_Button] = col;

			ImGui::SameLine(); ImGui::Dummy(h_spacing + ImVec2(2, 0));
			//

			ImGui::Dummy(v_spacing);


			// 3nd row
			ImGui::Dummy(h_spacing); ImGui::SameLine();
			
			if (ViewportManager::GetInstance().isActiveViewportPreset(2)) style.Colors[ImGuiCol_Button] = hcol;
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c0_2");
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 2; } ImGui::SameLine(); ImGui::Dummy(spacing); ImGui::SameLine(); //1_vp_0
			style.Colors[ImGuiCol_Button] = col;

			if (ViewportManager::GetInstance().isActiveViewportPreset(6)) style.Colors[ImGuiCol_Button] = hcol;
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c1_2");
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 6; } ImGui::SameLine(); ImGui::Dummy(spacing); ImGui::SameLine(); //2_vp_0
			style.Colors[ImGuiCol_Button] = col;

			if (ViewportManager::GetInstance().isActiveViewportPreset(10)) style.Colors[ImGuiCol_Button] = hcol;
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c2_2");
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 10; }
			style.Colors[ImGuiCol_Button] = col;

			ImGui::SameLine(); ImGui::Dummy(h_spacing + ImVec2(2, 0));
			//

			ImGui::Dummy(v_spacing);

			// 4nd row
			ImGui::Dummy(h_spacing); ImGui::SameLine();
			
			if (ViewportManager::GetInstance().isActiveViewportPreset(3)) style.Colors[ImGuiCol_Button] = hcol;
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c0_3");
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 3; } ImGui::SameLine(); ImGui::Dummy(spacing); ImGui::SameLine(); //1_vp_0
			style.Colors[ImGuiCol_Button] = col;

			if (ViewportManager::GetInstance().isActiveViewportPreset(7)) style.Colors[ImGuiCol_Button] = hcol;
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c1_3");
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 7; } ImGui::SameLine(); ImGui::Dummy(spacing); ImGui::SameLine(); //2_vp_0
			style.Colors[ImGuiCol_Button] = col;

			if (ViewportManager::GetInstance().isActiveViewportPreset(11)) style.Colors[ImGuiCol_Button] = hcol;
			button_icon = AssetManager::GetInstance().GetTexture("vp_layout_c2_3");
			if (ImGui::ImageButton((void*)*button_icon, butt_size, ImVec2(0.0f, 0.0f), butt_ratio, butt_padding, butt_bg_col)) { index = 11; }
			style.Colors[ImGuiCol_Button] = col;

			ImGui::SameLine(); ImGui::Dummy(h_spacing + ImVec2(2, 0));
			//

		    if (index != -1)
			{
				ViewportManager::GetInstance().setActiveViewportPreset_byIndex(index);
			}

			ImGui::Dummy(ImVec2(10.0f, 8.0f));
			ImGui::EndPopup();
		}
		guiStyleManager::setStyle("child_style");
		ImGui::PopFont();



		ImGui::EndChild();
	}
	ImGui::SameLine();

	
	float region_width    = object_treeNode_width + object_tree_max_depth * object_treeNode_indent;
	float region_height   = ImGui::GetWindowHeight() - 20.0f;
	object_tree_max_depth = 1;
	m_ui_focused = false;
	m_ui_focused_object_id = -1;
	guiStyleManager::setStyle("child_style2");

	
	//ImGui::SetNextWindowContentSize( ImVec2 ( region_width, region_height ) );
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
	
	ImGui::SetNextWindowContentWidth( region_width );
	ImGui::BeginChild("child223##", ImVec2( ImGui::GetWindowWidth() - 25.0f , ImGui::GetWindowHeight() - 20.0f), true, window_flags);
	{
		ImGui::Indent(10.0f);

		guiStyleManager::setStyle("main_style2");
		guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

		bool is_child_hovered = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
		m_focusing_menu_bar_items = is_child_hovered ? false : m_focusing_menu_bar_items;
		
		//
		m_traversed_objects.clear();
		m_traversed_objects_for_selection.clear();
		create_area_selection = false;
		m_tree_from = 0;
		m_tree_to   = 0;

		Mediator::request_HierarchyTreeHandle()->insert_Utility_Childs(m_object_tree_view_fv, m_object_tree_view_smplr, m_object_tree_view_lights);
		construct_SceneObjectTree(Mediator::request_HierarchyTree_Root(), 0, 0);
		Mediator::request_HierarchyTreeHandle()->clear_Utility_Childs();

		if (create_area_selection)
		{
			
			int from_i = -1;
			int to_i   = -1;

			int i = 0;
			//std::cout << "\n > Traversed:" << std::endl;
			for (SceneObject* obj : m_traversed_objects)
			{
				if      (obj->getId() == m_tree_from->getId()) from_i = i;
				else if (obj->getId() == m_tree_to->getId()  ) to_i   = i;

				//std::cout << "    - obj : " << obj->getName() << std::endl;
				i++;
			}

			//std::cout << "\n - from : " << m_tree_from->getName() << std::endl;
			//std::cout <<   " - to : "   << m_tree_to->getName()   << std::endl;
			bool from_is_area = m_tree_from->getType() == GROUP_PARENT_DUMMY && m_tree_from->isTemporary();

			if (!from_is_area)
			{


				int a = 0;
				int b = 0;
				if (from_i < to_i)
				{
					a = from_i;
					b = to_i;
				}
				else
				{
					a = to_i;
					b = from_i;
				}



				std::vector<SceneObject*> list;
				for (int i = a; i <= b; i++)
				{
					SceneObject* obj = m_traversed_objects[i];
					//std::cout << "\n   - obj : " << (obj != nullptr) << std::endl;
					//std::cout << "   - is_area : " << (obj->isTemporary()) << std::endl;
					//std::cout << "   - id : " << obj->getId() << std::endl;
					//std::cout << "   - name : " << obj->getName() << std::endl;
					if (obj->getType() != SAMPLER_OBJECT && obj->getType() != FACE_VECTOR)
						list.push_back(obj);
				}


				bool same = false;
				if (m_multiple_selection_active)
				{
					same = true;
					for (SceneObject* child : m_focused_object_pack.raw_object->getGroupSelectionChilds())
					{
						if (!Utilities::is_object_in_List(child, list))
						{
							same = false;
							break;
						}
					}
				}
				if (!same)
				{
					m_multiple_selected_objects.clear();
					for (SceneObject* obj : list)
						m_multiple_selected_objects.push_back(obj);

					process_pre_AreaSelection(m_multiple_selected_objects);
					handle_MULTIPLE_SELECTION();
					m_reselecting = true;
				}

			}
			
		}

		ImGui::PopFont();
		guiStyleManager::setStyle("child_style2");
		ImGui::EndChild();
	}

	ImGui::PopFont();
	ImGui::End();

}
void InterfaceManager::construct_SceneObjectTree(TreeNode* node,int depth, int counter)
{
	
	int o_id = node->is_root ? -1 : node->object->getId();
	////std::cout << "    - Node : " << o_id << std::endl;

	if (depth >= object_tree_max_depth)
		object_tree_max_depth = depth;

	std::vector<TreeNode*> childs = node->childs_by_depth_order(0, 0);//node->childs_depth_ordered; //
	
	std::string label;
	std::string obj_name;
	int         obj_id;
	int selected_obj_id = m_selected_object_pack.raw_object == nullptr ? -1 : m_selected_object_pack.raw_object->getId();
	
	int index = counter;
	for (TreeNode* child : childs)
	{
		
		bool is_even = index % 2 == 0 ? true : false;

		if (
			   !child->object->isActive() 
			||  child->object->getType()  == Type::APP_OBJECT 
			||  child->object->getId()    == -100
			||  child->object->isTemporary()
			)
			continue;

		m_traversed_objects.push_back(child->object);


		Type obj_type = child->object->getType();
		obj_id   = child->object->getId();
		obj_name = child->object->getName();
		
		if (   (obj_type == LIGHT_OBJECT       && !m_object_tree_view_lights)
			|| (obj_type == IMPORTED_OBJECT    && !m_object_tree_view_obj)
			|| (obj_type == GROUP_PARENT_DUMMY && !m_object_tree_view_grp)
			)
		{
			// m_exclusive_nodes.push_back( child );
			continue;
		}

		std::string icon_l = "";
		if (child->object->getType() == IMPORTED_OBJECT)
			label = std::string("  ") + std::string("  ") + obj_name + std::string("##"); // + std::to_string(child->object->getId());
		else if (child->object->getType() == LIGHT_OBJECT)
		{
			icon_l = ICON_MDI_SPOTLIGHT_BEAM;
			label = std::string("  ") + icon_l + std::string("  ") + obj_name + std::string("##");
		}
		else if (child->object->getType() == SAMPLER_OBJECT)
		{
			icon_l = ICON_MDI_SHAPE;
			label = std::string("  ") + icon_l + std::string("  ") + obj_name + std::string("##");
		}
		else if (child->object->getType() == FACE_VECTOR)
		{
			icon_l = ICON_MDI_RAY_START_ARROW; //ICON_MDI_AXIS_Y_ARROW;
			label = std::string("  ") + icon_l + std::string("  ") + obj_name + std::string("##");
		}
		else if (child->object->getType() == GROUP_PARENT_DUMMY)
		{
			icon_l = ICON_MDI_GROUP;
			label = std::string("  ") + icon_l + std::string("  ") + obj_name + std::string("##");
		}


		

		ImGuiTreeNodeFlags flags = 0;
		flags |= ImGuiTreeNodeFlags_FramePadding;
		flags |= ImGuiTreeNodeFlags_Selected;
		if (   child->getActiveChilds_size( m_object_tree_view_obj, m_object_tree_view_grp, m_object_tree_view_lights ) == 0)
		{
			flags |= ImGuiTreeNodeFlags_Bullet;
		}
		else
		{
			flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		}

		if (depth >= 1)
		{
			ImVec4 tex_col = ImGui::GetStyle().Colors[ImGuiCol_Text];
			ImGui::GetStyle().Colors[ImGuiCol_Text] = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] * 0.45f;
			ImGui::GetStyle().Colors[ImGuiCol_Text] = tex_col * 0.5f;

			ImVec2 a = ImGui::GetCursorScreenPos()  + ImVec2( 10.0f, 5.0f );
			ImVec2 a1 = a - ImVec2(20.0f, 0.0f);

			ImVec2 b = a1 - ImVec2(0.0f, 14.0f);
			ImGui::GetWindowDrawList()->AddLine(a, a1, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
			ImGui::GetWindowDrawList()->AddLine(a1, b, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);

			ImGui::GetStyle().Colors[ImGuiCol_Text] = tex_col;
		}

		
		float indent_space   = 23.0f;
		ImVec2 frame_padding = ImGui::GetStyle().FramePadding + ImVec2(2.0f,2.0f);
		ImVec2 pos           = ImGui::GetCursorScreenPos();
		ImVec4 hov_color     = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
		ImVec4 sel_color     = ImGui::GetStyle().Colors[ImGuiCol_HeaderActive ];
		ImVec4 def_color     = ImGui::GetStyle().Colors[ImGuiCol_Header       ];
		ImVec4 hov_color2    = ImGui::GetStyle().Colors[ImGuiCol_HeaderActive ] * 0.55f;
		
		
		bool open     = child->object->isSelected();
		bool selected = obj_id == selected_obj_id ? true : false;
		bool is_hoveringRect = ImGui::IsMouseHoveringRect(
			pos + ImVec2(58.0f, 0.0f), ImVec2(pos.x + ImGui::GetContentRegionMax().x + frame_padding.x, pos.y + ImGui::GetTextLineHeight() + frame_padding.y) + ImVec2(depth*indent_space, 0.0f));

		if      (open && !selected)
		{
			ImGui::GetStyle().Colors[ImGuiCol_Header] = hov_color;
		}
		else if ( selected )
		{
			ImGui::GetStyle().Colors[ImGuiCol_Header] = sel_color;
		}
		else
		{
			hov_color = is_even ? def_color : def_color * 0.65f;
			ImGui::GetStyle().Colors[ImGuiCol_Header] = hov_color;
		}

		
		if (is_hoveringRect 
			&& mouse_data.m0_down
			&& 
			(
			   m_selected_utility.id    == u_LINK
			|| m_selected_utility.group == g_RESTRICTIONS
			)
			&& m_selected_utility.active)
		{
			ImGui::GetStyle().Colors[ImGuiCol_Header] = hov_color2;
		}


		bool is_open          = child->object->is_ui_TreeNode_Open();
		bool child_has_childs = child->object->getChilds().size() > 0 ? true : false;
		int  array_index      = Mediator::requestObjectsArrayIndex(child->object->getId());
		
		if ( !m_popup_open
			&& ImGui::IsMouseHoveringRect(
			pos + ImVec2(58.0f, 0.0f), //48
			ImVec2(pos.x + ImGui::GetContentRegionMax().x + frame_padding.x, pos.y + ImGui::GetTextLineHeight() + frame_padding.y) + ImVec2(depth*indent_space, 0.0f)) 
			&& !m_skip_frame
			&& !m_reselecting
			&& !m_focusing_menu_bar_items)
		{
			m_ui_focused = true;
			m_ui_focused_object_id = child->object->getId();
			Mediator::setFocusedObject( array_index );
			if (ImGui::IsMouseClicked(0))
			{
				if (key_data[GLFW_KEY_LEFT_SHIFT].pressed)
				{
					m_tree_from = m_selected_object_pack.raw_object;
					int from_id = m_tree_from != 0 ? m_tree_from->getId() : -2;
					int to_id = m_ui_focused_object_id;
					if (from_id != to_id)
					{
						m_tree_to = child->object;
						create_area_selection = true;
					}
				}

				if (!create_area_selection && !key_data[GLFW_KEY_LEFT_SHIFT].pressed)
				{
					Mediator::setSelectedObject(array_index);
				}
				
			}
		}
		if (   !m_popup_open
			&& ImGui::IsMouseHoveringRect(
			pos + ImVec2(32.0f, 0.0f),
			ImVec2(pos.x + 25.0f + frame_padding.x, pos.y + ImGui::GetTextLineHeight() + frame_padding.y) + ImVec2(0.0f, 0.0f))
			&& !m_skip_frame
			&& !m_reselecting
			&& !m_focusing_menu_bar_items
			)
		{
			if(ImGui::IsMouseClicked(0))
				child->object->setVisible(!child->object->isVisible());
			//std::cout << child->object->getName() << std::endl;
		}
		

		bool hov_bullet = false;

		//ImGui::RenderFrame( pos - ImVec2(-12.0f, -3.0f), pos + ImVec2(12.0f, 15.0f) , ImGui::GetColorU32( ImGui::GetStyle().Colors[ImGuiCol_Button]) );
		if ( !m_popup_open && ImGui::IsMouseHoveringRect(pos - ImVec2(-12.0f, -3.0f), pos + ImVec2(12.0f, 15.0f)) )
		{
			hov_bullet = true;
			if (ImGui::IsMouseClicked(0) && (child_has_childs || true))
			{
				is_open = !is_open;
			}
		}

		//ICON_MDI_EYE
		child->object->set_ui_TreeNode_Open(is_open);
		ImGui::SetNextTreeNodeOpen( is_open );
		
		guiStyleManager::setIconFont(MDI, 15);
		if ( ImGui::TreeNodeEx(label.c_str(), child->object->isVisible(), flags) )
		{
			ImGui::GetStyle().Colors[ImGuiCol_Header] = def_color;
			construct_SceneObjectTree( child, depth + 1 , index + 1 );
			ImGui::TreePop();
		}
		ImGui::PopFont();

		ImGui::GetStyle().Colors[ImGuiCol_Header] = def_color;
		index++;
	}

	
	
}
void InterfaceManager::construct_SceneObjectTree(SceneObject* object, int depth, int counter)
{

	int o_id = object->getId();
	////std::cout << "    - Node : " << o_id << std::endl;

	if (depth >= object_tree_max_depth)
		object_tree_max_depth = depth;

	//std::vector<TreeNode*> childs = node->childs_by_depth_order();

	std::string label;
	std::string obj_name;
	int         obj_id;
	int selected_obj_id = m_selected_object_pack.raw_object == nullptr ? -1 : m_selected_object_pack.raw_object->getId();

	int index = counter;
	
	{
		bool is_even = index % 2 == 0 ? true : false;

		if (
			!object->isActive()
			|| object->getType() == Type::APP_OBJECT
			|| object->getId() == -100
			|| object->isTemporary()
			)
			return;

		obj_id   = object->getId();
		obj_name = object->getName();

		if (object->getType() != GROUP_PARENT_DUMMY)
			label = std::string("  ") + std::string("  ") + obj_name + std::string("##"); // + std::to_string(child->object->getId());
		else
		{
			label = std::string("  ") + ICON_FK_OBJECT_UNGROUP + std::string("  ") + obj_name + std::string("##");
		}



		ImGuiTreeNodeFlags flags = 0;
		flags |= ImGuiTreeNodeFlags_FramePadding;
		flags |= ImGuiTreeNodeFlags_Selected;
		flags |= ImGuiTreeNodeFlags_Bullet;
		
		if (depth >= 1)
		{
			ImVec4 tex_col = ImGui::GetStyle().Colors[ImGuiCol_Text];
			ImGui::GetStyle().Colors[ImGuiCol_Text] = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] * 0.45f;
			ImGui::GetStyle().Colors[ImGuiCol_Text] = tex_col;

			ImVec2 a = ImGui::GetCursorScreenPos() + ImVec2(10.0f, 5.0f);
			ImVec2 a1 = a - ImVec2(20.0f, 0.0f);

			ImVec2 b = a1 - ImVec2(0.0f, 14.0f);
			ImGui::GetWindowDrawList()->AddLine(a, a1, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
			ImGui::GetWindowDrawList()->AddLine(a1, b, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
		}


		float indent_space = 23.0f;
		ImVec2 frame_padding = ImGui::GetStyle().FramePadding + ImVec2(2.0f, 2.0f);
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec4 hov_color = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
		ImVec4 sel_color = ImGui::GetStyle().Colors[ImGuiCol_HeaderActive];
		ImVec4 def_color = ImGui::GetStyle().Colors[ImGuiCol_Header];


		bool open = object->isSelected();
		bool selected = obj_id == selected_obj_id ? true : false;
		if (open && !selected)
		{
			ImGui::GetStyle().Colors[ImGuiCol_Header] = hov_color;
		}
		else if (selected)
		{
			ImGui::GetStyle().Colors[ImGuiCol_Header] = sel_color;
		}
		else
		{
			hov_color = is_even ? def_color : def_color * 0.75f;
			ImGui::GetStyle().Colors[ImGuiCol_Header] = hov_color;
		}


		bool is_open          = object->is_ui_TreeNode_Open();
		bool child_has_childs = object->getChilds().size() > 0 ? true : false;
		int  array_index = Mediator::requestObjectsArrayIndex(object->getId());

		if (!m_popup_open
			&& ImGui::IsMouseHoveringRect(
			pos + ImVec2(48.0f, 0.0f),
			ImVec2(pos.x + ImGui::GetContentRegionMax().x + frame_padding.x, pos.y + ImGui::GetTextLineHeight() + frame_padding.y) + ImVec2(depth*indent_space, 0.0f))
			&& !m_skip_frame
			&& !m_reselecting
			&& !m_focusing_menu_bar_items)
		{
			m_ui_focused = true;
			m_ui_focused_object_id = object->getId();
			Mediator::setFocusedObject(array_index);
			if (ImGui::IsMouseClicked(0))
				Mediator::setSelectedObject(array_index);
		}

		bool hov_bullet = false;

		//ImGui::RenderFrame( pos - ImVec2(-12.0f, -3.0f), pos + ImVec2(12.0f, 15.0f) , ImGui::GetColorU32( ImGui::GetStyle().Colors[ImGuiCol_Button]) );
		if (!m_popup_open && ImGui::IsMouseHoveringRect(pos - ImVec2(-12.0f, -3.0f), pos + ImVec2(12.0f, 15.0f)))
		{
			hov_bullet = true;
			if (ImGui::IsMouseClicked(0) && child_has_childs)
			{
				is_open = !is_open;
			}
		}


		object->set_ui_TreeNode_Open(is_open);
		ImGui::SetNextTreeNodeOpen(is_open);

		//guiStyleManager::setIconFont(MDI, 15);
		guiStyleManager::setIconFont(MDI, 15);

		if (ImGui::TreeNodeEx(label.c_str(), flags))
		{
			ImGui::GetStyle().Colors[ImGuiCol_Header] = def_color;
			ImGui::TreePop();
		}
		ImGui::PopFont();

		ImGui::GetStyle().Colors[ImGuiCol_Header] = def_color;
		index++;
	}


}
void InterfaceManager::construct_Viewports_Frame_Settings()
{
	m_popup_viewport_settings    = false;
	m_vp_frame_buttons_focused   = false;
	m_hovering_over_camera_orbit = false;

	std::vector<VIEWPORT> viewports = ViewportManager::GetInstance().getAllEnabledViewports();
	for (VIEWPORT vp : viewports)
	{
		glm::vec2 a        = glm::vec2(vp.dimensions.offset.x, m_window_height - (vp.dimensions.offset.y + vp.renderSize.y));
		ImVec2 window_pos  = ImVec2(a.x, a.y - 12.0f);//ImVec2(a.x, a.y - 4);
		ImVec2 window_size = ImVec2(60.0f,0.0f);
		ImVec2 end_pos     = window_pos + ImVec2(vp.dimensions.size.x , 0.0f);

		ImGuiWindowFlags flags = 0;
		flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		flags |= ImGuiWindowFlags_NoCollapse;
		flags |= ImGuiWindowFlags_NoBackground;
		flags |= ImGuiWindowFlags_NoTitleBar;
		flags |= ImGuiWindowFlags_NoDecoration;
		flags |= ImGuiWindowFlags_NoNavInputs;
		flags |= ImGuiWindowFlags_NoNavFocus;

		std::string popup_label;
		std::string label = "viewport_Frame" + std::string("##") + std::to_string(vp.id);
		ImGui::SetNextWindowSize(window_size);
		ImGui::SetNextWindowPos(window_pos);
		guiStyleManager::setStyle("viewports_frame_style");
		ImGui::Begin(label.c_str(), 0, flags);
		{

			std::string outer_offset  = std::string("        ");
			std::string outer_offset2 = std::string("                 ");
			std::string inner_offset  = std::string("        ");
			std::string inner_offset2 = std::string("            ");
			std::string label  = "";
			std::string label2 = "";
			std::string post   = "";


			std::string button_label;
			std::string id = std::to_string(vp.id);

			ImGui::Dummy(ImVec2(0.0f, 14.0f));
			
			// Viewport Render Settings popup //
			{
				guiStyleManager::setIconFont(MDI, 14);
				//button_label = ICON_FA_EDIT + std::string("##") + std::to_string(vp.id);
				button_label = ICON_MDI_DRAWING + std::string("##") + std::to_string(vp.id);
				popup_label = "Viewport_Render_Settings" + std::string("##") + std::to_string(vp.id);
				if (ImGui::Button(button_label.c_str(), ImVec2(25, 20)))
					ImGui::OpenPopup(popup_label.c_str());
				ImGui::PopFont();
				if (ImGui::IsItemHovered()) m_vp_frame_buttons_focused = true;
				guiStyleManager::setStyle("viewports_popup_style");
				if (ImGui::BeginPopup(popup_label.c_str()))
				{
					m_popup_viewport_settings = true;
					bool is_changed = false;

					ImGui::Dummy(ImVec2(0.0f, 1.0f));

					Render_Settings settings = vp.render_settings;
					Shading_Mode mode = vp.render_settings.shading_mode;
					bool enabled[2] = { false, false };
					bool is_wireframe = vp.render_settings.wireframe_mode;
					if (mode == DEFAULT_SHADING)
						enabled[0] = true;
					else if (mode == GRAY_SHADING)
						enabled[1] = true;
					

					label = outer_offset + std::string("  Default Shading") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);
					ImGui::Checkbox(label.c_str(), &enabled[0]);
					if (ImGui::IsItemActivated())
					{
						is_changed = true;
						settings.shading_mode = DEFAULT_SHADING;
					}
					label = outer_offset + std::string("  Gray Shading") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);;
					ImGui::Checkbox(label.c_str(), &enabled[1]);
					if (ImGui::IsItemActivated())
					{
						is_changed = true;
						settings.shading_mode = GRAY_SHADING;
					}
					ImGui::Separator();

					label = outer_offset + std::string("  Wireframe Mode") + outer_offset2 + post + std::string("##");
					ImGui::Checkbox(label.c_str(), &is_wireframe);
					if (ImGui::IsItemActivated())
					{
						is_changed = true;
						settings.wireframe_mode = !is_wireframe;
					}
					ImGui::Dummy(ImVec2(0, 1.0f));

					ViewportManager::GetInstance().setViewports_Render_Settings( settings, vp.id );
					if (is_changed && vp.renderer != OPTIX_RENDERER)
						ViewportManager::GetInstance().getViewport(vp.id).markDirty();
					ImGui::EndPopup();
				}
				guiStyleManager::setStyle("viewports_frame_style");
			}

			ImGui::SameLine();
			ImGui::Dummy(ImVec2(5.0f, 0.0f));
			ImGui::SameLine();

			
			{

				guiStyleManager::setIconFont(MDI, 14);
				//button_label = ICON_FA_EYE + std::string("##") + std::to_string(vp.id);
				button_label = ICON_MDI_EYE_PLUS + std::string("##") + std::to_string(vp.id);
				popup_label = "Viewport_View_Settings" + std::string("##") + std::to_string(vp.id);
				if (ImGui::Button(button_label.c_str(), ImVec2(25, 20)))
					ImGui::OpenPopup(popup_label.c_str());
				ImGui::PopFont();
				if (ImGui::IsItemHovered()) m_vp_frame_buttons_focused = true;
				guiStyleManager::setStyle("viewports_popup_style");
				if (ImGui::BeginPopup(popup_label.c_str()))
				{
					m_popup_viewport_settings = true;

					Render_Settings settings = vp.render_settings;
					Camera_Look_At camera_settings = vp.camera_settings;
					bool need_restart_acc = false;
					bool changed = false;

					bool enabled_bbox[2] = { false, false };
					bool enabled_smplr[2] = { false,false };
					bool enabled_rel[3] = { false, false, false };
					bool enabled_fv[2] = { false, false };
					bool enabled_rs[8] = { false, false , false, false, false, false, false, false };
					bool enabled_ls[5] = { false,false,false,false,false };
					bool enabled_obj = settings.draw_objects_all;
					bool enabled_grp = settings.draw_groups_all;

					bool draw_lights_fill_pol = !settings.draw_lights_fill_polygon;
					bool draw_lights_fill_poly[4] = { false,false,false,false };
					//bool draw_lights_line_pol = settings.draw_lights_line_polygon;

					enabled_bbox[0] = settings.draw_bbox_only;
					enabled_bbox[1] = settings.draw_bbox_and_object;

					enabled_smplr[0] = settings.draw_sampler_all;
					enabled_smplr[1] = settings.draw_sampler_only_selected;

					enabled_rel[0] = settings.draw_relations_only_selected;
					enabled_rel[1] = settings.draw_relations_only_selected_tree;
					enabled_rel[2] = settings.draw_relations_all;

					enabled_fv[0] = settings.draw_face_vectors_only_selected;
					enabled_fv[1] = settings.draw_face_vectors_all;

					enabled_rs[0] = settings.draw_restrictions_only_selected;
					enabled_rs[1] = settings.draw_restrictions_all;
					enabled_rs[2] = settings.draw_restrictions_only_face_vector_selected;


					enabled_rs[3] = settings.draw_restriction_cones;
					enabled_rs[4] = settings.draw_restriction_fv;
					enabled_rs[5] = settings.draw_restriction_fv_lines;
					enabled_rs[6] = settings.draw_restriction_dist_area;
					enabled_rs[7] = settings.draw_restriction_details;

					enabled_ls[0] = settings.draw_lights_all;
					enabled_ls[1] = settings.draw_lights_quad;
					enabled_ls[2] = settings.draw_lights_spot;
					enabled_ls[3] = settings.draw_lights_sphere;
					enabled_ls[4] = settings.draw_lights_point;

					draw_lights_fill_poly[0] = settings.draw_lights_fill_poly[0];
					draw_lights_fill_poly[1] = settings.draw_lights_fill_poly[1];
					draw_lights_fill_poly[2] = settings.draw_lights_fill_poly[2];
					draw_lights_fill_poly[3] = settings.draw_lights_fill_poly[3];


					ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Text];
					ImVec4 td_col = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
					ImVec4 c_col = ImGui::GetStyle().Colors[ImGuiCol_CheckMark];


					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					guiStyleManager::setIconFont(MDI, 16);

					// bbox 
					post = "";
					post = settings.draw_bbox_only ? std::string(" ") + ICON_MDI_CUBE_OUTLINE : post;
					post = settings.draw_bbox_and_object ? std::string(" ") + ICON_MDI_CUBE : post;
					label = outer_offset + ICON_MDI_CUBE_OUTLINE + std::string("  Bounding Box") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);;
					if (ImGui::BeginMenu(label.c_str()))
					{
						ImGui::Dummy(ImVec2(0.0f, 1.0f));

						label2 = inner_offset + ICON_MDI_CUBE_OUTLINE + std::string("  Bounding Box Only") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_bbox[0]))
						{
							settings.draw_bbox_only = !settings.draw_bbox_only;
							settings.draw_bbox_and_object = false;
							changed = true;
							need_restart_acc = true;
						}

						label2 = inner_offset + ICON_MDI_CUBE + std::string("  Bounding Box + Mesh") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_bbox[1]))
						{
							if (settings.draw_bbox_only)
								need_restart_acc = true;

							settings.draw_bbox_only = false;
							settings.draw_bbox_and_object = !settings.draw_bbox_and_object;
							changed = true;
						}

						ImGui::Dummy(ImVec2(0.0f, 1.0f));
						ImGui::EndMenu();
					}

					ImGui::Separator();
					
#ifdef vp_render_settings_objects_groups
					// Objects - Groups
					{
						// Objects
						post = "";
						post = settings.draw_objects_all ? std::string("            ") + ICON_MDI_CROP_FREE : post;
						label = outer_offset + ICON_MDI_CUBE + std::string("  Objects") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);;
						if (ImGui::BeginMenu(label.c_str()))
						{
							ImGui::Dummy(ImVec2(0.0f, 1.0f));

							label2 = inner_offset + ICON_MDI_CROP_FREE + std::string("  All objects") + inner_offset2 + std::string("##") + std::to_string(vp.id);
							if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_obj))
							{
								settings.draw_objects_all = !settings.draw_objects_all;
								changed = true;
							}

							//ImGui::Separator();
							
							ImGui::Dummy(ImVec2(0.0f, 1.0f));
							ImGui::EndMenu();
						}


						// Groups
						post = "";
						post = settings.draw_groups_all ? std::string("             ") + ICON_MDI_CROP_FREE : post;
						label = outer_offset + get_IconLabel(GROUP_PARENT_DUMMY) + std::string("  Groups") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);;
						if (ImGui::BeginMenu(label.c_str()))
						{
							ImGui::Dummy(ImVec2(0.0f, 1.0f));

							label2 = inner_offset + ICON_MDI_CROP_FREE + std::string("  All Groups") + inner_offset2 + std::string("##") + std::to_string(vp.id);
							if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_grp))
							{
								settings.draw_groups_all = !settings.draw_groups_all;
								changed = true;
							}

							//ImGui::Separator();

							ImGui::Dummy(ImVec2(0.0f, 1.0f));
							ImGui::EndMenu();
						}
					}
					ImGui::Separator();

#endif

					// lights
					post = "";
					post = settings.draw_lights_all ? std::string("              ") + ICON_MDI_CROP_FREE : post;
					label = outer_offset + ICON_MDI_SPOTLIGHT_BEAM + std::string("  Lights") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);;
					if (ImGui::BeginMenu(label.c_str()))
					{
						ImGui::Dummy(ImVec2(0.0f, 1.0f));

						label2 = inner_offset + ICON_MDI_CROP_FREE + std::string("  All objects") + inner_offset2 + std::string("##") + std::to_string(vp.id);
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_ls[0]))
						{
							settings.draw_lights_all = !settings.draw_lights_all;
							changed = true;
						}

						ImGui::Separator();

						if (settings.draw_lights_all)
						{
							ImGui::GetStyle().Colors[ImGuiCol_Text] = td_col;
							ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = td_col;
						}

						// quad 
						label2 = inner_offset + ICON_MDI_WALL_SCONCE_FLAT + std::string("  Quad Lights") + inner_offset2 + std::string("##") + std::to_string(vp.id);
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_ls[1]))
						{

							settings.draw_lights_quad = !settings.draw_lights_quad;
							changed = true;
						}

						// spotlights 
						label2 = inner_offset + ICON_MDI_TRACK_LIGHT + std::string("  SpotLights") + inner_offset2 + std::string("##") + std::to_string(vp.id);
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_ls[2]))
						{

							settings.draw_lights_spot = !settings.draw_lights_spot;
							changed = true;
						}

						// spherical
						label2 = inner_offset + ICON_MDI_CIRCLE_SLICE_8 + std::string("  Spherical Lights") + inner_offset2 + std::string("##") + std::to_string(vp.id);
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_ls[3]))
						{

							settings.draw_lights_sphere = !settings.draw_lights_sphere;
							changed = true;
						}


						// spherical
						label2 = inner_offset + ICON_MDI_CIRCLE_MEDIUM + std::string("  Point Lights") + inner_offset2 + std::string("##") + std::to_string(vp.id);
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_ls[4]))
						{

							settings.draw_lights_point = !settings.draw_lights_point;
							changed = true;
						}

						ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;
						ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = c_col;


						ImGui::Separator();


						label2 = inner_offset + " " + std::string("  Render in wireframe mode") + inner_offset2 + std::string("##") + std::to_string(vp.id);
						if (ImGui::Custom_Checkbox(label2.c_str(), &draw_lights_fill_pol))
						{
							settings.draw_lights_fill_polygon = !settings.draw_lights_fill_polygon;
							changed = true;
						}


						ImGui::Dummy(ImVec2(0.0f, 1.0f));
						ImGui::EndMenu();
					}

					
					// samplers 
					post = "";
					post = settings.draw_sampler_all ? std::string("         ") + ICON_MDI_CROP_FREE : post;
					post = settings.draw_sampler_only_selected ? std::string("         ") + ICON_MDI_CUBE_SCAN : post;
					label = outer_offset + ICON_MDI_SHAPE_OUTLINE + std::string("  Samplers") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);;
					if (ImGui::BeginMenu(label.c_str()))
					{
						ImGui::Dummy(ImVec2(0.0f, 1.0f));

						
						label2 = inner_offset + ICON_MDI_CUBE_SCAN + std::string("  Selected Object Only") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_smplr[1]))
						{
							settings.draw_sampler_only_selected = !settings.draw_sampler_only_selected;
							settings.draw_sampler_all = false;
							changed = true;
						}

						label2 = inner_offset + ICON_MDI_CROP_FREE + std::string("  All objects") + inner_offset2 + std::string("##") + std::to_string(vp.id);
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_smplr[0]))
						{
							settings.draw_sampler_all = !settings.draw_sampler_all;
							settings.draw_sampler_only_selected = false;
							changed = true;
							//need_restart_acc = true;
						}


						ImGui::Dummy(ImVec2(0.0f, 1.0f));
						ImGui::EndMenu();
					}



					// face vectors
					post = "";
					post = settings.draw_face_vectors_only_selected ? std::string("  ") + ICON_MDI_CUBE_SCAN : post;
					post = settings.draw_face_vectors_all ? std::string("  ") + ICON_MDI_CROP_FREE : post;
					label = outer_offset + ICON_MDI_ARROW_EXPAND_RIGHT + std::string("  Face Vectors") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);
					if (ImGui::BeginMenu(label.c_str()))
					{
						ImGui::Dummy(ImVec2(0.0f, 1.0f));

						label2 = inner_offset + ICON_MDI_CUBE_SCAN + std::string("  Selected Object Only") + inner_offset2 + std::string("##") + std::to_string(vp.id);
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_fv[0]))
						{
							settings.draw_face_vectors_only_selected = !settings.draw_face_vectors_only_selected;
							settings.draw_face_vectors_all = false;
							changed = true;
						}

						label2 = inner_offset + ICON_MDI_CROP_FREE + std::string("  All objects") + inner_offset2 + std::string("##") + std::to_string(vp.id);
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_fv[1]))
						{
							settings.draw_face_vectors_only_selected = false;
							settings.draw_face_vectors_all = !settings.draw_face_vectors_all;
							changed = true;
						}

						ImGui::Dummy(ImVec2(0.0f, 1.0f));
						ImGui::EndMenu();
					}


					
					ImGui::Separator();

					
					// restrictions
					post = "";
					post = settings.draw_restrictions_only_selected ? std::string("    ") + ICON_MDI_CUBE_SCAN : post;
					post = settings.draw_restrictions_only_face_vector_selected ? std::string("    ") + ICON_MDI_RAY_START_ARROW : post;
					post = settings.draw_restrictions_all ? std::string("    ") + ICON_MDI_CROP_FREE : post;
					label = outer_offset + ICON_MDI_VECTOR_LINE + std::string("  Restrictions") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);;
					if (ImGui::BeginMenu(label.c_str()))
					{
						ImGui::Dummy(ImVec2(0.0f, 1.0f));

						label2 = inner_offset + ICON_MDI_CUBE_SCAN + std::string("  Selected Object Only") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rs[0]))
						{
							settings.draw_restrictions_only_selected = !settings.draw_restrictions_only_selected;
							settings.draw_restrictions_only_face_vector_selected = false;
							settings.draw_restrictions_all = false;
							changed = true;
						}

						//label2 = inner_offset + ICON_MDI_CONTAIN + std::string("  All objects") + inner_offset2 + std::string("##");
						label2 = inner_offset + ICON_MDI_RAY_START_ARROW + std::string("  Selected Face Vector Only") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rs[2]))
						{

							settings.draw_restrictions_only_face_vector_selected = !settings.draw_restrictions_only_face_vector_selected;
							settings.draw_restrictions_only_selected = false;
							settings.draw_restrictions_all = false;
							changed = true;
						}

						//label2 = inner_offset + ICON_MDI_CONTAIN + std::string("  All objects") + inner_offset2 + std::string("##");
						label2 = inner_offset + ICON_MDI_CROP_FREE + std::string("  All objects") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rs[1]))
						{

							settings.draw_restrictions_all = !settings.draw_restrictions_all;
							settings.draw_restrictions_only_selected = false;
							settings.draw_restrictions_only_face_vector_selected = false;
							changed = true;
						}

						ImGui::Separator();


						if (!settings.draw_restriction_details)
						{
							ImGui::GetStyle().Colors[ImGuiCol_Text] = td_col;
							ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = td_col;
						}

						// cones //
						label2 = inner_offset + ICON_MDI_EYE_PLUS + std::string("  View: Solid Angle") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rs[3]))
						{
							settings.draw_restriction_cones = !settings.draw_restriction_cones;
							changed = true;
						}

						// face vectors //
						label2 = inner_offset + ICON_MDI_EYE_PLUS + std::string("  View: Face Vectors") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rs[4]))
						{
							settings.draw_restriction_fv = !settings.draw_restriction_fv;
							changed = true;
						}

						// fv - lines //
						label2 = inner_offset + ICON_MDI_EYE_PLUS + std::string("  View: Face Vector Directions") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rs[5]))
						{
							settings.draw_restriction_fv_lines = !settings.draw_restriction_fv_lines;
						}


						// dist area //
						label2 = inner_offset + ICON_MDI_EYE_PLUS + std::string("  View: Distance Area") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rs[6]))
						{
							settings.draw_restriction_dist_area = !settings.draw_restriction_dist_area;
						}


						ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;
						ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = c_col;

						// overall - details //
						label2 = inner_offset + ICON_MDI_EYE_SETTINGS + std::string("  View: Details") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rs[7]))
						{
							settings.draw_restriction_details = !settings.draw_restriction_details;
							//if (!settings.draw_restriction_details)
							//{
								//settings.draw_restriction_cones = false;
								//settings.draw_restriction_fv = false;
								//settings.draw_restriction_fv_lines = false;
							//}
							changed = true;
						}


						ImGui::Dummy(ImVec2(0.0f, 1.0f));
						ImGui::EndMenu();
					}



					// relations
					post = "";
					post = settings.draw_relations_only_selected ? std::string("         ") + ICON_MDI_CUBE_SCAN : post;
					post = settings.draw_relations_all ? std::string("         ") + ICON_MDI_CROP_FREE : post;
					post = settings.draw_relations_only_selected_tree ? std::string("         ") + ICON_MDI_ARRANGE_BRING_TO_FRONT : post;
					label = outer_offset + ICON_MDI_LINK_VARIANT + std::string("  Relations") + outer_offset2 + post + std::string("##") + std::to_string(vp.id);
					if (ImGui::BeginMenu(label.c_str()))
					{
						ImGui::Dummy(ImVec2(0.0f, 1.0f));

						label2 = inner_offset + ICON_MDI_CUBE_SCAN + std::string("  Selected Object Only") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rel[0]))
						{

							settings.draw_relations_all = false;
							settings.draw_relations_only_selected = !settings.draw_relations_only_selected;
							settings.draw_relations_only_selected_tree = false;
							changed = true;
						}


						label2 = inner_offset + ICON_MDI_ARRANGE_BRING_TO_FRONT + std::string("  Selected Object Tree") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rel[1]))
						{
							settings.draw_relations_only_selected_tree = !settings.draw_relations_only_selected_tree;
							settings.draw_relations_all = false;
							settings.draw_relations_only_selected = false;
							changed = true;
						}

						//label2 = inner_offset + ICON_MDI_CONTAIN + std::string("  All objects") + inner_offset2 + std::string("##");
						label2 = inner_offset + ICON_MDI_CROP_FREE + std::string("  All objects") + inner_offset2 + std::string("##") + std::to_string(vp.id);;
						if (ImGui::Custom_Checkbox(label2.c_str(), &enabled_rel[2]))
						{
							settings.draw_relations_all = !settings.draw_relations_all;
							settings.draw_relations_only_selected = false;
							settings.draw_relations_only_selected_tree = false;
							changed = true;
						}

						ImGui::Dummy(ImVec2(0.0f, 1.0f));
						ImGui::EndMenu();
					}



					
					//


					ImGui::Dummy(ImVec2(0, 2.0f));

					// if changed -> setSceneDirty() && viewports dirty()
					if (need_restart_acc && vp.renderer == OPTIX_RENDERER)
						Mediator::restartAccumulation();
					if (changed)
						ViewportManager::GetInstance().getViewport(vp.id).markDirty();


					ViewportManager::GetInstance().setViewports_Render_Settings(settings, vp.id);

					ImGui::PopFont();
					ImGui::EndPopup();
				}
				guiStyleManager::setStyle("viewports_frame_style");
			}

			//
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(5.0f, 0.0f));
			ImGui::SameLine();
			
			guiStyleManager::setStyle("viewports_frame_style");

			//
			
		}
		ImGui::End();


		float BUT_SIZE = 55.0f;
		label = "viewport_Frame_2" + std::string("##") + std::to_string(vp.id);
		ImGui::SetNextWindowSize(ImVec2(window_size.x / 2.0f + BUT_SIZE , 0.0f));
		ImGui::SetNextWindowPos(window_pos + ImVec2(vp.renderSize.x - BUT_SIZE - 5.0f , 1.0f)); // -35
		guiStyleManager::setStyle("viewports_frame_style");
		ImGui::Begin(label.c_str(), 0, flags);
		{
			ImGui::Dummy(ImVec2(0.0f, 13.0f));

			ImVec2 but_size = ImVec2(100.0f, 25.0f);
			guiStyleManager::setIconFont(MDI, 20);
			
			ImVec4 bh_col = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
			ImVec4 ba_col = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
			ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0, 0, 0, 0);
			ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0, 0, 0, 0);
			GLuint button_icon = ViewportManager::GetInstance().getFboTexture_Camera_Orbit(vp.id);
			bool activated = ImGui::ImageButton((void*)button_icon, ImVec2(BUT_SIZE,BUT_SIZE), ImVec2(0.0f, 0.0f), ImVec2(1.f,1.f) , 1.0f, ImVec4(0,0,0,0));
			ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = bh_col;
			ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ba_col;
			if (ImGui::IsItemHovered())
			{
				if (vp.is_active)
					m_hovering_over_camera_orbit = true;

				if (ImGui::IsMouseDown(0) || ImGui::IsMouseDown(1))
				{
					ViewportManager::GetInstance().setFocusedViewport(vp.id);
					ViewportManager::GetInstance().ActivateFocusedViewport();
					
					if(ImGui::IsMouseDown(0)) 
						m_m0_down_on_camera_control_handle = true;

					if (ImGui::IsMouseClicked(0))
					{

					}
				}

			}
			if     (activated && m_mouse_movement_while_m0_down == 0
				|| (m_hovering_over_camera_orbit && ImGui::IsMouseClicked(1) && !m_m0_down_on_camera_control_handle)
				)
			{
				ImGui::OpenPopup(popup_label.c_str());
			}
			
			
			if (ImGui::IsItemHovered()) m_vp_frame_buttons_focused = true;
			ImGui::PopFont();
			guiStyleManager::setStyle("viewports_popup_style");
			ImGuiWindowFlags flags = 0;
			flags |= ImGuiWindowFlags_NoMove;
			if (ImGui::BeginPopup(popup_label.c_str(), flags))
			{
				m_popup_viewport_settings = true;

				ImGui::Dummy(ImVec2(0.0f, 1.0f));

				but_size += ImVec2(20, 0);

				label = "Front" + std::string("##") + std::to_string(vp.id);
				if (ImGui::Button(label.c_str(), but_size))
				{
					ViewportManager::GetInstance().getViewportCamera(vp.id).set_interp_Camera_State(LOOK_FRONT);
					ImGui::CloseCurrentPopup();
					m_m0_down_on_camera_control_handle = false;
				}

				label = "Back" + std::string("##") + std::to_string(vp.id);
				if (ImGui::Button(label.c_str(), but_size))
				{
					ViewportManager::GetInstance().getViewportCamera(vp.id).set_interp_Camera_State(LOOK_BACK);
					ImGui::CloseCurrentPopup();
					m_m0_down_on_camera_control_handle = false;
				}

				ImGui::Separator();

				label = "Top" + std::string("##") + std::to_string(vp.id);
				if (ImGui::Button(label.c_str(), but_size))
				{
					ViewportManager::GetInstance().getViewportCamera(vp.id).set_interp_Camera_State(LOOK_TOP);
					//m_popup_viewport_settings = false;
					ImGui::CloseCurrentPopup();
					m_m0_down_on_camera_control_handle = false;
				}

				label = "Bottom" + std::string("##") + std::to_string(vp.id);
				if (ImGui::Button(label.c_str(), but_size))
				{
					ViewportManager::GetInstance().getViewportCamera(vp.id).set_interp_Camera_State(LOOK_BOTTOM);
					//m_popup_viewport_settings = false;
					ImGui::CloseCurrentPopup();
					m_m0_down_on_camera_control_handle = false;
				}

				ImGui::Separator();

				label = "Left" + std::string("##") + std::to_string(vp.id);
				if (ImGui::Button(label.c_str(), but_size))
				{
					ViewportManager::GetInstance().getViewportCamera(vp.id).set_interp_Camera_State(LOOK_LEFT);
					//m_popup_viewport_settings = false;
					ImGui::CloseCurrentPopup();
					m_m0_down_on_camera_control_handle = false;
				}

				label = "Right" + std::string("##") + std::to_string(vp.id);
				if (ImGui::Button(label.c_str(), but_size))
				{
					ViewportManager::GetInstance().getViewportCamera(vp.id).set_interp_Camera_State(LOOK_RIGHT);
					//m_popup_viewport_settings = false;
					ImGui::CloseCurrentPopup();
					m_m0_down_on_camera_control_handle = false;
				}


				ImGui::Separator();


				guiStyleManager::setStyle("viewports_popup_style2");
				ImGui::Dummy(ImVec2(0, 1));
				label = "   Target Camera" + std::string("##") + std::to_string(vp.id);
				if(ImGui::BeginMenu(label.c_str()))
				{
					ImGui::Dummy(ImVec2(0, 1));
					ImGuiStyle style = ImGui::GetStyle();
					style.ItemSpacing.y += 4.f;
					ImGui::PushItemWidth(160.0f);

					if (ImGui::MenuItem("Selected Object"))
					{
						if(m_selected_object_pack.raw_object != nullptr)
							ViewportManager::GetInstance().getViewportCamera(vp.id).Target(m_selected_object_pack.raw_object);
						ImGui::CloseCurrentPopup();
						m_m0_down_on_camera_control_handle = false;

						ViewportManager::GetInstance().getViewport(vp.id).markDirty();
					}

					ImGui::Separator();
					
					std::string combo_label = std::string("##") + "combo_list" + std::to_string(vp.id);
					char * preview_value = "";
					ImGuiComboFlags combo_flags = 0;
					//combo_flags |= ImGuiComboFlags_NoPreview;
					//ImGui::Text("          Pick Object     "); ImGui::SameLine();
					
					if (ImGui::BeginCombo(combo_label.c_str(), "Pick Object", combo_flags))
					{
						ImGui::PushItemWidth(120.0f);
						ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Text];

						ImGui::GetStyle().FramePadding.y += 1.0f;

						ImGui::Dummy(ImVec2(0, 1));
						std::vector<SceneObject*> objects = Mediator::RequestSceneObjects();
						for (SceneObject* obj : objects)
						{
							Type obj_type = obj->getType();
							bool is_valid = obj->isActive();
							bool is_type_valid = obj_type == IMPORTED_OBJECT ||  obj_type == GROUP_PARENT_DUMMY;
							if (!is_valid || !is_type_valid) continue;

							ImGuiTreeNodeFlags tree_node_flags = 0;
							tree_node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
							tree_node_flags |= ImGuiTreeNodeFlags_FramePadding;
							tree_node_flags |= ImGuiTreeNodeFlags_Selected;
							

							// utility childs vector
							std::vector<SceneObject*> utility_childs = obj->getUtilityChilds(true);
							if(utility_childs.size() > 0) 
								tree_node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
							else 
								tree_node_flags |= ImGuiTreeNodeFlags_Bullet;

							ImVec2 pos = ImGui::GetCursorScreenPos();
							ImVec4 hov_color = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
							ImVec4 sel_color = ImGui::GetStyle().Colors[ImGuiCol_HeaderActive];
							ImVec4 def_color = ImGui::GetStyle().Colors[ImGuiCol_Header];

							std::string obj_label = get_IconLabel(obj_type) + obj->getName() + std::string("##") + std::to_string(obj->getId()) + std::to_string(vp.id);
							guiStyleManager::setIconFont(MDI, 16);
							ImGuiID tree_node_ID = ImGui::GetCurrentWindow()->GetID(obj_label.c_str());
							bool open_before = ImGui::TreeNodeBehaviorIsOpen(tree_node_ID);
							if (ImGui::TreeNodeEx_Original(obj_label.c_str(), tree_node_flags) )
							{
								if (ImGui::IsItemActivated())
								{
									ImVec2 min = ImVec2(pos);
									ImVec2 max = ImVec2(pos + ImVec2(26, 20));

									if (!ImGui::IsMouseHoveringRect(min, max))
									{
										ViewportManager::GetInstance().getViewportCamera(vp.id).Target(obj);
										ImGui::CloseCurrentPopup();
										m_m0_down_on_camera_control_handle = false;
									}
								}
								for (SceneObject* ut_obj : utility_childs)
								{

									ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.5, 0.5, 0.5, 1);
									ImGui::Text("|__");ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;
									ImGui::SameLine(); 
									std::string ut_obj_label = get_IconLabel(ut_obj->getType()) + ut_obj->getName() + std::string("##") + std::to_string(ut_obj->getId()) + std::to_string(vp.id);
									if (ImGui::Selectable(ut_obj_label.c_str()))
									{
										ViewportManager::GetInstance().getViewportCamera(vp.id).Target(ut_obj);
										ImGui::CloseCurrentPopup();
										m_m0_down_on_camera_control_handle = false;
										ViewportManager::GetInstance().getViewport(vp.id).markDirty();
										//m_vp_frame_buttons_focused |= true;
									}
								}

								ImGui::TreePop();
							}
							bool open_after = ImGui::TreeNodeBehaviorIsOpen(tree_node_ID);

							bool toggled_tree_node = open_before != open_after;
							if (ImGui::IsItemActivated())
							{
								ImVec2 min = ImVec2(pos);
								ImVec2 max = ImVec2(pos + ImVec2(26, 20));
								
								if (!ImGui::IsMouseHoveringRect(min, max))
								{
									ViewportManager::GetInstance().getViewportCamera(vp.id).Target(obj);
									ImGui::CloseCurrentPopup();
									m_m0_down_on_camera_control_handle = false;
									ViewportManager::GetInstance().getViewport(vp.id).markDirty();
								}
							}


							
						}
						ImGui::Dummy(ImVec2(0, 1));
						ImGui::EndCombo();

						ImGui::GetStyle().FramePadding.y -= 1.0f;
						ImGui::PopItemWidth();
					}
					ImGui::PopItemWidth();
					
					ImGui::Dummy(ImVec2(0, 1));
					style.ItemSpacing.y -= 4.f;

					ImGui::EndMenu();
				}
				ImGui::Dummy(ImVec2(0, 1));
				guiStyleManager::setStyle("viewports_popup_style");

				ImGui::Separator();

				label = "Reset Camera" + std::string("##") + std::to_string(vp.id);
				if (ImGui::Button(label.c_str(), but_size))
				{
					ViewportManager::GetInstance().getViewportCamera(vp.id).Target(ZERO_3f, 1000.0f);
					//m_popup_viewport_settings = false;
					ImGui::CloseCurrentPopup();
					m_m0_down_on_camera_control_handle = false;
				}


				ImGui::Dummy(ImVec2(0.0f, 1.0f));
				ImGui::EndPopup();
			}

		}
		ImGui::End();

	}


}
void InterfaceManager::construct_BottomPanel()
{
	return construct_BottomPanel_Test();

	guiStyleManager::setStyle("main_style_bot_panel");

	ImGui_Window_Data window = imgui_windows[BOTTOM_PANEL];
	ImGui::SetNextWindowPos(ImVec2(0, m_window_height - window.size.y + 2));
	ImGui::SetNextWindowSize(ImVec2(m_window_width, window.size.y));

	ImGuiWindowFlags window_flags = 0;

	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoDecoration;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::Begin("Bottom Window", NULL, window_flags);
	{
		ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Text];

		SceneObject* object = m_selected_object_pack.raw_object;
		bool  is_null = object == nullptr ? true : false;

		optix::float3 attributes;
		optix::float3 attributes_delta = ZERO_3f;

		std::string abs_label = "Translation";
		std::string rel_label = "Delta:";


		if (is_null)
			Geometry::RAY::Intersect_Plane(Geometry::SHAPES::createMouseRay(),
										   Geometry::SHAPES::createPlane(ZERO_3f, GLOBAL_AXIS[1]),
										   attributes);
		else
		{

			if (m_selected_utility.group != g_TRANSFORMATIONS)
				attributes = object->getTranslationAttributes();

			else
			{
				if (m_selected_utility.id == u_TRANSLATION)
				{
					attributes = object->getTranslationAttributes();
					attributes_delta = object->getAttributesDelta(0);
					abs_label = "Translation";
				}
				else if (m_selected_utility.id == u_ROTATION)
				{
					attributes = object->getRotationAttributes();
					attributes_delta = object->getAttributesDelta(1);
					abs_label = "Rotation";
				}
				else if (m_selected_utility.id == u_SCALE)
				{
					attributes = object->getScaleAttributes();
					attributes_delta = object->getAttributesDelta(2);
					abs_label = "Scale";
				}
			}
			
			
		}
		
		optix::Matrix4x4 translation_matrix = optix::Matrix4x4::identity();
		optix::Matrix4x4 rot_matrix   = is_null ? optix::Matrix4x4::identity() : object->get_local_parent_rot_matrix_chain();
		optix::Matrix4x4 scale_matrix = is_null ? optix::Matrix4x4::identity() : object->getScaleMatrix_value();
		float pos[3] = { attributes.x, attributes.y, attributes.z };
		
		{
			float * abs_attr = (float*)&attributes;

			ImGui::Dummy(ImVec2(0.0f, 0.0f));
			ImGui::Dummy(ImVec2(280.0f, 0.0f));
			ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.7f;
			ImGui::SameLine();
			ImGui::Text(abs_label.c_str());
			ImGui::GetStyle().Colors[ImGuiCol_Text] = col;
			ImGui::SameLine();
			ImGui::SameLine();
			ImGui::PushItemWidth(100.0f);
			ImGui::Text("X:");
			ImGui::SameLine();
			ImGui::DragFloat("##", &abs_attr[0], 0.0f, 0.0f, 0.0f);
			ImGui::SameLine();
			ImGui::Text("Y:");
			ImGui::SameLine();
			ImGui::DragFloat("##", &abs_attr[1], 0.0f, 0.0f, 0.0f);
			ImGui::SameLine();
			ImGui::Text("Z:");
			ImGui::SameLine();
			ImGui::DragFloat("##", &abs_attr[2], 0.0f, 0.0f, 0.0f);
			ImGui::PopItemWidth();
		}

		Type type;
		if (!is_null) type = object->getType();
		std::string obj_type;
		if (is_null)
			obj_type = "";
		else
		{
			if (type == GROUP_PARENT_DUMMY) obj_type = " GROUP ";
			if (type == IMPORTED_OBJECT) obj_type = " OBJECT ";
			if (type == SAMPLER_OBJECT) obj_type = " SAMPLER ";
			if (type == LIGHT_OBJECT)
			{
				Light_Type t = object->get_LightParameters().type;
				if (t == QUAD_LIGHT)
					obj_type = " QUAD LIGHT ";
				if (t == SPHERICAL_LIGHT)
					obj_type = " SPHERICAL LIGHT ";
				if (t == POINT_LIGHT)
					obj_type = " POINT LIGHT ";
				if (t == SPOTLIGHT)
					obj_type = " SPOTLIGHT ";
			}
		}

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(20.0f, 0.0f));
		ImGui::SameLine();
		ImGui::Custom_Separator(ImVec2(7.0f, -1.0f), 1.0f);
		ImGui::SameLine();
		

		
		ImGui::Text("     Type:");
		ImGui::SameLine();
		ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.7f;
		ImGui::Text(obj_type.c_str());
		ImGui::GetStyle().Colors[ImGuiCol_Text] = col;



		{
			float * attr_delta = (float*)&attributes_delta;

			ImGui::PushItemWidth(100.0f);
			ImGui::Dummy(ImVec2(280.0f, 1.0f));
			
			ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.7f;
			ImGui::SameLine();
			ImGui::Text(rel_label.c_str());
			ImGui::GetStyle().Colors[ImGuiCol_Text] = col;

			ImGui::SameLine();
			ImGui::Text("X:");
			ImGui::SameLine();
			ImGui::DragFloat("##", &attr_delta[0], 0.0f, 0.0f, 0.0f);
			ImGui::SameLine();
			ImGui::Text("Y:");
			ImGui::SameLine();
			ImGui::DragFloat("##", &attr_delta[1], 0.0f, 0.0f, 0.0f);
			ImGui::SameLine();
			ImGui::Text("Z:");
			ImGui::SameLine();
			ImGui::DragFloat("##", &attr_delta[2], 0.0f, 0.0f, 0.0f);
			ImGui::PopItemWidth();
		}

		if(false)
		{
		
		}

	}
	ImGui::End();
}
void InterfaceManager::construct_BottomPanel_Test()
{
	
	guiStyleManager::setStyle("main_style_bot_panel");
	guiStyleManager::setFont(ROBOTO, "Regular", 14);

	ImGui_Window_Data window = imgui_windows[BOTTOM_PANEL];
	ImGui::SetNextWindowPos(ImVec2(0, m_window_height - window.size.y + 2));
	ImGui::SetNextWindowSize(ImVec2(m_window_width, window.size.y));

	ImGuiWindowFlags window_flags = 0;

	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoDecoration;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::Begin("Bottom Window", NULL, window_flags);
	{
		
		SceneObject* object = m_selected_object_pack.raw_object;
		bool  is_null = object == nullptr ? true : false;

		if (is_null)
			object = m_focused_object_pack.raw_object;

		bool is_null2 = object == nullptr ? true : false;

		optix::float3 attributes;
		optix::float3 attributes_delta = ZERO_3f;

		std::string abs_label = " Cursor:";
		std::string rel_label = " Delta:";
		std::string name = (!is_null || !is_null2) ? object->getName() : "";
		std::string parent_name = (!is_null || !is_null2) ? (object->getParent() == nullptr ? "" : object->getParent()->getName()) : "";    //object->getParent()->getName() : "";
		std::string grp_name    = (!is_null || !is_null2) ? (object->getGroupParent() == nullptr ? "" : object->getGroupParent()->getName()) : "";

		if (is_null)
			Geometry::RAY::Intersect_Plane(Geometry::SHAPES::createMouseRay(),
										   Geometry::SHAPES::createPlane(ZERO_3f, GLOBAL_AXIS[1]),
										   attributes);
		else
		{
			bool gui_transforming = m_gui_transforming[0] || m_gui_transforming[1] || m_gui_transforming[2];
			if ( m_selected_utility.group != g_TRANSFORMATIONS && !gui_transforming )
			{
				abs_label = " Translation";
				attributes = object->getTranslationAttributes();
			}
			else
			{
				if (m_selected_utility.id      == u_TRANSLATION   || m_gui_transforming[0])
				{
					attributes = object->getTranslationAttributes();
					attributes_delta = object->getAttributesDelta(0);
					abs_label = " Translation";
				}
				else if (m_selected_utility.id == u_ROTATION      || m_gui_transforming[1])
				{
					//attributes = object->getRotationAttributes() * 57.2957795f;
					attributes_delta = object->getAttributesDelta(1) * 57.2957795f;
					abs_label = " Rotation";
					
					Geometry::GENERAL::EulerAnglesFromMatrix( object->getRotationMatrix_chain(), attributes );
					attributes = Geometry::GENERAL::toDegrees(attributes);
					
				}
				else if (m_selected_utility.id == u_SCALE         || m_gui_transforming[2])
				{
					attributes = object->getScaleAttributes();
					attributes_delta = object->getAttributesDelta(2);
					abs_label = " Scale";
				}
			}


		}

		Type type;
		std::string obj_type;
		if (!is_null || !is_null2) type = object->getType();
		if (is_null && is_null2)
			obj_type = "";
		else
		{
			if (type == GROUP_PARENT_DUMMY) obj_type = " GROUP ";
			if (type == IMPORTED_OBJECT)    obj_type = " OBJECT ";
			if (type == SAMPLER_OBJECT)     obj_type = " SAMPLER ";
			if (type == FACE_VECTOR)        obj_type = " FACE VECTOR ";
			if (type == LIGHT_OBJECT)
			{
				Light_Type t = object->get_LightParameters().type;
				if (t == QUAD_LIGHT)
					obj_type = " QUAD LIGHT ";
				if (t == SPHERICAL_LIGHT)
					obj_type = " SPHERICAL LIGHT ";
				if (t == POINT_LIGHT)
					obj_type = " POINT LIGHT ";
				if (t == SPOTLIGHT)
					obj_type = " SPOTLIGHT ";
			}
		}

		
		ImVec4 col_fbg = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
		ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = is_null ? col_fbg * 0.9f : col_fbg;
		//ImGui::GetStyle().Colors[ImGuiCol_Text   ]    = is_null ? ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] : ImGui::GetStyle().Colors[ImGuiCol_Text];
		ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Text];

		optix::Matrix4x4 translation_matrix = optix::Matrix4x4::identity();
		optix::Matrix4x4 rot_matrix = is_null ? optix::Matrix4x4::identity() : object->get_local_parent_rot_matrix_chain();
		optix::Matrix4x4 scale_matrix = is_null ? optix::Matrix4x4::identity() : object->getScaleMatrix_value();
		
		{
			float width_offset_a = 80.0f;
			float width_offset_b = 400.0f;
			float width_offset_c = 150.0f;
			ImGui::BeginColumns("##", 5);
			ImGui::SetColumnWidth(0, 280.0f );    
			ImGui::SetColumnWidth(1, width_offset_a * 1.0f);
			ImGui::SetColumnWidth(2, width_offset_b * 1.0f);  
			ImGui::SetColumnWidth(3, width_offset_c * 2.0f);
			
			// Dummy Spacing //


			// Labels //
			ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.7f;
			ImGui::NextColumn();
			ImGui::Dummy(ImVec2(4.0f, 0.0f));
			ImGui::SameLine();
			ImGui::Text(abs_label.c_str());
			
			if (!is_null)
			{
				ImGui::Dummy(ImVec2(4.0f, 0.0f));
				ImGui::SameLine();
				ImGui::Text(rel_label.c_str());
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col;
			}

			// Values //
			ImGui::NextColumn();
			{
				if(is_null)
					ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.75f;
				{
					float * abs_attr = (float*)&attributes;
					
					ImGui::Dummy(ImVec2(6.0f, 4.0f));
					ImGui::SameLine();

					ImGui::PushItemWidth(90.0f);
					ImGui::Text(" X: ");
					ImGui::SameLine();
					ImGui::DragFloat("##", &abs_attr[0], 0.0f, 0.0f, 0.0f);
					ImGui::SameLine();
					ImGui::Text(" Y: ");
					ImGui::SameLine();
					ImGui::DragFloat("##", &abs_attr[1], 0.0f, 0.0f, 0.0f);
					ImGui::SameLine();
					ImGui::Text(" Z: ");
					ImGui::SameLine();
					ImGui::DragFloat("##", &abs_attr[2], 0.0f, 0.0f, 0.0f);
					ImGui::PopItemWidth();
				}

				if (!is_null)
				{
					float * attr_delta = (float*)&attributes_delta;

					ImGui::Dummy(ImVec2(6.0f, 4.0f));
					ImGui::SameLine();

					ImGui::PushItemWidth(90.0f);
					ImGui::Text(" X: ");
					ImGui::SameLine();
					ImGui::DragFloat("##", &attr_delta[0], 0.0f, 0.0f, 0.0f);
					ImGui::SameLine();
					ImGui::Text(" Y: ");
					ImGui::SameLine();
					ImGui::DragFloat("##", &attr_delta[1], 0.0f, 0.0f, 0.0f);
					ImGui::SameLine();
					ImGui::Text(" Z: ");
					ImGui::SameLine();
					ImGui::DragFloat("##", &attr_delta[2], 0.0f, 0.0f, 0.0f);
					ImGui::PopItemWidth();
				}

				ImGui::GetStyle().Colors[ImGuiCol_Text] = col;
			}


			// Name - Type - Parent //
			ImGui::NextColumn();
			{

				// Name
				ImGui::Dummy(ImVec2(6.0f, 0.0f));
				ImGui::SameLine();
				ImGui::Text("Name:     ");
				ImGui::SameLine();
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.7f;
				ImGui::Text(name.c_str());
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col;

				// Type
				ImGui::Dummy(ImVec2(6.0f, 0.0f));
				ImGui::SameLine();
				ImGui::Text("Type:      ");
				ImGui::SameLine();
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.7f;
				ImGui::Text(obj_type.c_str());
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col;

				// Parent
				ImGui::Dummy(ImVec2(6.0f, 0.0f));
				ImGui::SameLine();
				ImGui::Text("Parent:    ");
				ImGui::SameLine();
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.7f;
				ImGui::Text(parent_name.c_str());
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col;

				// Group
				ImGui::Dummy(ImVec2(6.0f, 0.0f));
				ImGui::SameLine();
				ImGui::Text("Group:     ");
				ImGui::SameLine();
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.7f;
				ImGui::Text(grp_name.c_str());
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col;

			}

			ImGui::EndColumns();
		}

		
		
		ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = col_fbg;
		ImGui::GetStyle().Colors[ImGuiCol_Text]    = col;
	}
	ImGui::End();
	ImGui::PopFont();

	
}


void InterfaceManager::construct_FileBrowser_Test()
{

	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 16);
	guiStyleManager::setStyle("style_import_popup_2");

	bool open = m_fileChooser_open;
	int a_state = -1;

	if ( open )
	{
		
		std::string title = "";
		std::vector<const char*> exts;

		file_dialog_select_file.push_Flags(ImGuiFileBrowserFlags_CreateNewDir);
		file_dialog_select_file.push_Flags(ImGuiFileBrowserFlags_CloseOnEsc);
		file_dialog_select_file.push_Flags(ImGuiFileBrowserFlags_EnterNewFilename);

		
		if      (m_fileChooser_select_dir_action == IMPORT_OBJECT)
		{
			exts = std::vector<const char*>({ ".obj" });
			a_state = 0;
			title = "  Import Object";
		}
		else if (m_fileChooser_select_dir_action == LOAD_SCENE)
		{
			exts = std::vector<const char*>({ });
			a_state = 1;
			title = "  Load Scene";
		}
		else if (m_fileChooser_select_dir_action == SAVE_SCENE)
		{
			exts = std::vector<const char*>({ });
			a_state = 2;
			title = "  Save Scene";
		}
		else if (m_fileChooser_select_dir_action == EXPORT_SCENE)
		{
			exts = std::vector<const char*>({ });
			a_state = 3;
			title = "  Export Scene";
		}


		file_dialog_select_file.SetTitle( title );
		file_dialog_select_file.SetTypeFilters( exts );
		
		if (m_popup_import_file_toggle)
		{
			file_dialog_select_file.Open(&m_fileChooser_open);
			m_popup_import_file_toggle = false;
		}

		file_dialog_select_file.Display();

		if ( file_dialog_select_file.HasSelected() )
		{
			std::string filepath = file_dialog_select_file.GetSelected().string();
			std::replace(filepath.begin(), filepath.end(), '\\', '/');

			
			
			if      (a_state == 0) // import object
			{
				// check if mtl file exists //
				std::string path = Utilities::getFilePath( filepath );
				std::string filename = Utilities::getFileName(filepath, false);
				std::string mtl_file = path + filename + ".mtl";

				std::ifstream fs( mtl_file );
				bool mtl_file_exists = fs.good();
				if (!mtl_file_exists) // create default mtl file
				{
					// scan mesh file and get all mtl materials //
					std::vector<std::string> mtl_mats;

					std::ifstream file_in; file_in.open(filepath);
					std::string line    = "";
					std::string mtl_mat = "";
					while (std::getline(file_in, line))
					{
						std::vector<std::string> tks = Utilities::tokenize_String(line," ");
						if (tks.size() > 0)
						{
							if (tks[0] == "usemtl")
							{
								int index = 0;
								for (std::string t : tks)
								{
									if (index > 0)
										mtl_mat += t;
									index++;
								}


								bool push_mat = true;
								for (std::string mat : mtl_mats)
								{
									if (mat == mtl_mat)
									{
										push_mat = false;
										break;
									}
								}

								if(push_mat) 
									mtl_mats.push_back(mtl_mat);
								mtl_mat = "";
							}
						}
					}
					file_in.close();

					std::ofstream file;
					file.open( mtl_file );

					for (std::string mat : mtl_mats)
					{
						file << "\n";
						file << "newmtl " + mat + "\n";
						file << "Ns 0.000 \n";
						file << "Ka 0.700 0.700 0.700 \n";
						file << "Kd 0.700 0.700 0.700 \n";
						file << "Ks 0.700 0.700 0.700 \n";
						file << "\n";
					}
					file.close();
				}



				//
				SceneObject* obj_handle = 0;
				Mediator::addSceneObject( filepath.c_str() );
				obj_handle = Mediator::RequestSceneObjects()[Mediator::RequestSceneObjects().size() - 1];
				ViewportManager::GetInstance().getActiveViewportCamera().Target(obj_handle);

				Object_Package pack;
				pack.raw_object = obj_handle;
				pack.object_index = Mediator::requestObjectsArrayIndex(obj_handle->getId());
				setSelectedObject(pack);
				
			}
			else if (a_state == 1) // load scene
			{

#ifdef THREADING_IMPORT_SCENE

				m_scene_Import_THR = std::thread(THR_callFunc_Load_Scene, filename.c_str());
					
				// resetSelectedObject(false, false);
				// clear scene
				// apply new scene data
				//
				// resetSelectedObject(false, false);

#else


#define AUTO_SAVE_SCENE_BEFORE_IMPORTING
#ifdef AUTO_SAVE_SCENE_BEFORE_IMPORTING
				Mediator::AUTO_SAVE_TEMP_SCENE("last_working_scene");
#endif

				resetSelectedObject(false, false);
				Mediator::Import_Scene(filepath.c_str());
				resetSelectedObject(false, false);
				
				Mediator::restartAccumulation();
				Mediator::setSceneDirty();
#endif
				
			}
			else if (a_state == 2) // save scene
			{
				Mediator::Save_Scene(filepath.c_str(), filepath.c_str());
			}
			else if (a_state == 3) // export scene
			{
				Mediator::Export_Scene(filepath.c_str(), filepath.c_str());
			}


			m_skip_frame        = true;
			m_reselecting       = true;
			m_popup_import_file = false;
			m_fileChooser_open  = false;
			m_popup_import_file = false;

			file_dialog_select_file.Close();
			file_dialog_select_file.ClearSelected();

			//file_dialog_test.clear_Flags();
		}

	}

	if(!file_dialog_select_file.IsOpened() || !open)
		m_popup_import_file = false;
	

	ImGui::PopFont();

}
void InterfaceManager::construct_FileBrowser_select_Dir()
{
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 18);
	guiStyleManager::setStyle("style_import_popup_2");

	std::string label;
	std::string ext = "";
	FileBrowser_ActionID ac_id = ID_SAVE_FILE;
	if (m_fileChooser_select_dir_action == SAVE_SCENE)
	{
		label = "Save Scene";
	}
	else if (m_fileChooser_select_dir_action == EXPORT_SCENE)
	{
		label = "Export Scene";
	}
	else if (m_fileChooser_select_dir_action == LOAD_SCENE)
	{
		ac_id = ID_SELECT_FILE;
		label = "Load Scene";
		ext = ".r_scene,.R_SCENE,R_scene,R_Scene,r_SCENE,r_Scene";
	}

	if ( m_fileChooser_select_dir )
		ImGui::OpenPopup( label.c_str() );

	if ( file_dialog_select_dir.showFileDialog( label.c_str() , ImVec2(680, 410), &m_popup_crt_dir, ext.c_str() , ac_id ) )
	{
		std::string filepath = file_dialog_select_dir.selected_fn.c_str();
		std::string path = file_dialog_select_dir.selected_dir;
		std::string file = file_dialog_select_dir.selected_fn;

		
		
		if (m_fileChooser_select_dir_action == SAVE_SCENE)
		{
			Mediator::Export_Scene(path.c_str(), file.c_str());
		}
		else if (m_fileChooser_select_dir_action == EXPORT_SCENE)
		{
			Mediator::Export_Scene(path.c_str(), file.c_str());
		}
		else if (m_fileChooser_select_dir_action == LOAD_SCENE)
		{
			resetSelectedObject(false, false);
			Mediator::Import_Scene(filepath.c_str());
		}

		file_dialog_select_dir.reset_Data();
		m_skip_frame        = true;
		m_reselecting       = true;
		m_popup_crt_dir     = false;
		
	}


	ImGui::PopFont();
	m_fileChooser_select_dir = false;
}
void InterfaceManager::construct_FileBrowser()
{
	return construct_FileBrowser_Test();

	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 18);
	guiStyleManager::setStyle("style_import_popup");

	if ( m_fileChooser_open )
		ImGui::OpenPopup("Import Object");

	if (file_dialog.showFileDialog("Import Object", ImVec2(680, 410), &m_popup_import_file, ".obj,.OBJ"))
	{
		SceneObject* obj_handle = 0;
		Mediator::addSceneObject(file_dialog.selected_fn.c_str());
		obj_handle = Mediator::RequestSceneObjects()[Mediator::RequestSceneObjects().size() - 1];
		ViewportManager::GetInstance().getActiveViewportCamera().Target(obj_handle);
		
		Object_Package pack;
		pack.raw_object = obj_handle;
		pack.object_index = Mediator::requestObjectsArrayIndex(obj_handle->getId());
		setSelectedObject( pack );
		m_skip_frame = true;
		m_reselecting = true;
		m_popup_import_file = false;
	}

	
	
	ImGui::PopFont();
	m_fileChooser_open = false;
}
void InterfaceManager::construct_window_render_test()
{
	int window_w = Mediator::RequestWindowSize(0);
	int window_h = Mediator::RequestWindowSize(1);
	float a = window_w / window_h;

	int width = 400;
	int height = 400;


	ImGui::SetNextWindowPos(ImVec2(251, 45 + 29), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_MenuBar;
	ImGui::Begin("testing target rendering!##", NULL, window_flags);
	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;
	if (ImGui::IsWindowHovered())
	{
		OpenGLRenderer::GetInstace().update_Scene_Camera(0, mouse_data);
	}
	else
	{
		OpenGLRenderer::GetInstace().freeze_Scene_Camera(0);
	}
	GLuint tex = OpenGLRenderer::GetInstace().getSecondarySceneTex(0);
	if (tex != 0)
	{
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddImage(
			(void *)tex,
			ImVec2(pos),
			ImVec2(pos.x + 400 + 400 * a, pos.y + 400),
			ImVec2(0, 1),
			ImVec2(1, 0));
	}
	ImGui::End();



}

//
void InterfaceManager::construct_RightPanel()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "construct_RightPanel", "");
#endif

	guiStyleManager::setStyle("main_style");

	ImGui_Window_Data window = imgui_windows[RIGHT_PANEL];
	ImGui::SetNextWindowPos(ImVec2(m_window_width - window.size.x + 2, window.pos.y));
	ImGui::SetNextWindowSize(ImVec2(window.size.x, m_window_height - window.pos.y - imgui_windows[BOTTOM_PANEL].size.y - 1 ));

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags |= ImGuiWindowFlags_NoScrollWithMouse;

	ImGui::Begin("Right Window", NULL, window_flags);
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 16);
	
	// Main child window
	{

		// Main Child
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		guiStyleManager::setStyle("main_style_right_child00");
		

		ImGui::Dummy(ImVec2(2.0f, 4.0f));
		ImGui::SameLine();
		ImGui::BeginChild("main_child##", ImVec2(ImGui::GetWindowWidth() - 10.0f, ImGui::GetWindowHeight()), true, window_flags);
		
		

		// TABS //
		ImVec2 C_POS;
		ImVec2 c_pos;
		ImVec2 c_size      = ImVec2(28.0f, 10.0f);
		ImVec2 c_off       = ImVec2(-14.0f, -30.0f);
		ImVec2 c_off_w     = ImVec2(42.0f, 0.0f);
		ImVec2 c_off_w_pad = ImVec2(1.0f, 0.0f);

		bool is_any_tab_hovered = false;
		ImGuiTabBarFlags  flags = 0;
		flags |= ImGuiTabBarFlags_None;
		//flags |= ImGuiTabBarFlags_NoTooltip;
		
		ImGuiTabItemFlags tab_flags[10] = { 0,0,0,0,0,0,0,0,0,0 };
		if (m_force_tab_open_active)
		{
			for (int i = 0; i < 10; i++)
			{
				if (m_force_tab_open[i])
					tab_flags[i] |= ImGuiTabItemFlags_SetSelected;
			}
		}


		if (ImGui::BeginTabBar("tab_bar##", flags))
		{
			ImGui::Separator();
			ImGui::Separator();

			guiStyleManager::setStyle("main_style_right_child0");
			std::string label = "";
			
			
			// Object - general
			guiStyleManager::setIconFont(MDI, TAB_FONT_SIZE);
			label = ICON_MDI_CUBE_OUTLINE + std::string("##");
			if (ImGui::BeginTabItem2(label.c_str(), m_tooltip_label[GENERAL_INFO_TAB], nullptr, tab_flags[GENERAL_INFO_TAB]))
			{
				construct_Object_General_Settings_Window();
				reset_Force_Tabs_Open_Flags();
				ImGui::EndTabItem();
			}
			if (ImGui::IsItemHovered())
			{

				if (!tooltip_timer.isRunning())
				{
					tooltip_timer.start();
					m_item_hovered_timer = tooltip_timer.getTime();
				}
				else
				{
					float delta = tooltip_timer.getTime() - m_item_hovered_timer;
					//if (delta >= 0.3f) popup_Tooltip( ImGui::GetCursorPos(), " Tooltip! " );
				}

				is_any_tab_hovered |= true;
			}
			ImGui::PopFont();
			guiStyleManager::setStyle("main_style_right_child0");

			// Object - Transformation
			guiStyleManager::setIconFont(MDI, TAB_FONT_SIZE);
			label = ICON_MDI_CUBE_SCAN + std::string("##");
			if (ImGui::BeginTabItem2(label.c_str(), m_tooltip_label[TRANSFORMATIONS_TAB], nullptr, tab_flags[TRANSFORMATIONS_TAB]))
			{
				construct_Transformations_child_Window();
				reset_Force_Tabs_Open_Flags();
				ImGui::EndTabItem();
			}
			if (ImGui::IsItemHovered())
			{
				if (!tooltip_timer.isRunning())
				{
					tooltip_timer.start();
					m_item_hovered_timer = tooltip_timer.getTime();
				}
				else
				{
					float delta = tooltip_timer.getTime() - m_item_hovered_timer;
					//if (delta >= 0.3f) popup_Tooltip( ImGui::GetCursorPos(), " Tooltip! " );
				}

				is_any_tab_hovered |= true;
			}
			ImGui::PopFont();
			guiStyleManager::setStyle("main_style_right_child0");

			// Materials
			guiStyleManager::setIconFont(MDI, TAB_FONT_SIZE);
			label = ICON_MDI_HEXAGON_MULTIPLE + std::string("##");
			if (ImGui::BeginTabItem2(label.c_str(), m_tooltip_label[MATERIALS_TAB], nullptr, tab_flags[MATERIALS_TAB]))
			{
				construct_Materials_child_Window();
				reset_Force_Tabs_Open_Flags();
				ImGui::EndTabItem();
			}
			if (ImGui::IsItemHovered())
			{
				if (!tooltip_timer.isRunning())
				{
					tooltip_timer.start();
					m_item_hovered_timer = tooltip_timer.getTime();
				}
				else
				{
					float delta = tooltip_timer.getTime() - m_item_hovered_timer;
					//if (delta >= 0.3f) popup_Tooltip( ImGui::GetCursorPos(), " Tooltip! " );
				}

				is_any_tab_hovered |= true;
			}
			ImGui::PopFont();
			guiStyleManager::setStyle("main_style_right_child0");


			// Light Parameters
			guiStyleManager::setIconFont(MDI, TAB_FONT_SIZE);
			label = ICON_MDI_SPOTLIGHT_BEAM + std::string("##");
			if (ImGui::BeginTabItem2(label.c_str(), m_tooltip_label[LIGHT_PARAMS_TAB], nullptr, tab_flags[LIGHT_PARAMS_TAB]))
			{
				construct_Light_Parameters_child_Window();
				reset_Force_Tabs_Open_Flags();
				ImGui::EndTabItem();
			}
			if (ImGui::IsItemHovered())
			{
				if (!tooltip_timer.isRunning())
				{
					tooltip_timer.start();
					m_item_hovered_timer = tooltip_timer.getTime();
				}
				else
				{
					float delta = tooltip_timer.getTime() - m_item_hovered_timer;
					//if (delta >= 0.3f) popup_Tooltip( ImGui::GetCursorPos(), " Tooltip! " );
				}

				is_any_tab_hovered |= true;
			}
			ImGui::PopFont();
			guiStyleManager::setStyle("main_style_right_child0");


			// Sampler Parameters
			guiStyleManager::setIconFont(MDI, TAB_FONT_SIZE);
			label = ICON_MDI_SHAPE_OUTLINE + std::string("##");
			if (ImGui::BeginTabItem2(label.c_str(), m_tooltip_label[SAMPLER_PARAMS_TAB], nullptr, tab_flags[SAMPLER_PARAMS_TAB]))
			{
				construct_Sampler_Parameters_Child_Window();
				reset_Force_Tabs_Open_Flags();
				ImGui::EndTabItem();
			}
			if (ImGui::IsItemHovered())
			{
				if (!tooltip_timer.isRunning())
				{
					tooltip_timer.start();
					m_item_hovered_timer = tooltip_timer.getTime();
				}
				else
				{
					float delta = tooltip_timer.getTime() - m_item_hovered_timer;
					//if (delta >= 0.3f) popup_Tooltip( ImGui::GetCursorPos(), " Tooltip! " );
				}

				is_any_tab_hovered |= true;
			}
			ImGui::PopFont();
			guiStyleManager::setStyle("main_style_right_child0");


			// Relation Parameters
			guiStyleManager::setIconFont(MDI, TAB_FONT_SIZE);
			//label = ICON_MDI_VECTOR_ARRANGE_BELOW + std::string("##");
			label = ICON_MDI_LINK_VARIANT + std::string("##");
			if (ImGui::BeginTabItem2(label.c_str(), m_tooltip_label[RELATIONS_TAB], nullptr, tab_flags[RELATIONS_TAB]))
			{
				construct_Object_Relations_Window();
				reset_Force_Tabs_Open_Flags();
				ImGui::EndTabItem();
			}
			if (ImGui::IsItemHovered())
			{
				if (!tooltip_timer.isRunning())
				{
					tooltip_timer.start();
					m_item_hovered_timer = tooltip_timer.getTime();
				}
				else
				{
					float delta = tooltip_timer.getTime() - m_item_hovered_timer;
					//if (delta >= 0.3f) popup_Tooltip( ImGui::GetCursorPos(), " Tooltip! " );
				}

				is_any_tab_hovered |= true;
			}
			ImGui::PopFont();
			guiStyleManager::setStyle("main_style_right_child0");


			// Restriction Parameters
			guiStyleManager::setIconFont(MDI, TAB_FONT_SIZE);
			label = ICON_MDI_VECTOR_LINE + std::string("##");
			if (ImGui::BeginTabItem2(label.c_str(), m_tooltip_label[RESTRICTIONS_TAB], nullptr, tab_flags[RESTRICTIONS_TAB]))
			{
				construct_Object_Restrictions_Window();
				reset_Force_Tabs_Open_Flags();
				ImGui::EndTabItem();
			}
			if (ImGui::IsItemHovered())
			{
				if (!tooltip_timer.isRunning())
				{
					tooltip_timer.start();
					m_item_hovered_timer = tooltip_timer.getTime();
				}
				else
				{
					float delta = tooltip_timer.getTime() - m_item_hovered_timer;
					//if (delta >= 0.3f) popup_Tooltip( ImGui::GetCursorPos(), " Tooltip! " );
				}

				is_any_tab_hovered |= true;
			}
			ImGui::PopFont();
			guiStyleManager::setStyle("main_style_right_child0");

			
			// Scene Render Settings
			guiStyleManager::setIconFont(MDI, TAB_FONT_SIZE);
			label = ICON_MDI_SETTINGS_OUTLINE + std::string("##");
			if (ImGui::BeginTabItem2(label.c_str(), m_tooltip_label[SCENE_SETTINGS_TAB], nullptr, tab_flags[SCENE_SETTINGS_TAB]))
			{
				construct_Scene_Render_Settings_Window();
				reset_Force_Tabs_Open_Flags();
				ImGui::EndTabItem();
			}
			if (ImGui::IsItemHovered())
			{
				if (!tooltip_timer.isRunning())
				{
					tooltip_timer.start();
					m_item_hovered_timer = tooltip_timer.getTime();
				}
				else
				{
					float delta = tooltip_timer.getTime() - m_item_hovered_timer;
					//if (delta >= 0.3f) popup_Tooltip( ImGui::GetCursorPos(), " Tooltip! " );
				}

				is_any_tab_hovered |= true;
			}
			ImGui::PopFont();
			guiStyleManager::setStyle("main_style_right_child0");

			

			ImGui::EndTabBar();
		}
		ImGui::EndChild();

		if (!is_any_tab_hovered)
			tooltip_timer.stop();

	}

	
	ImGui::PopFont();
	ImGui::End();
}

void InterfaceManager::construct_Object_General_Settings_Window()
{
	
	process_TAB_SELECTION_STATE_change(GENERAL_INFO_TAB);
	reset_Tabs_State(GENERAL_INFO_TAB);
	m_hierarchy_setting = TRANSFORMATION_SETTING::TO_ALL;

	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
	std::string label   = "";
	SceneObject* object = m_selected_object_pack.raw_object;
	bool is_null = (object == nullptr);
	if (!is_null) is_null |= (object->isTemporary() || object->getType() == APP_OBJECT);

	ImGuiWindowFlags window_flags = 0;
	window_flags  |= ImGuiWindowFlags_NoTitleBar;
	window_flags  |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags  |= ImGuiWindowFlags_NoScrollbar;
	//window_flags  |= ImGuiWindowFlags_NoScrollWithMouse;
	ImGuiWindowFlags window_flags2 = 0;
	window_flags2 |= ImGuiWindowFlags_NoTitleBar;
	window_flags2 |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;

	ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Text];
	//  //
	{
		
		// Transformations - Child Window
		guiStyleManager::setStyle("main_style_right_child2");
		ImGui::BeginChild("General_info_window_child##", ImVec2(ImGui::GetWindowWidth() - 4.0f, ImGui::GetWindowHeight() - 20.0f), true, window_flags);
		{
			std::string name     = is_null ? " " : object->getName();
			std::string obj_type = "";
			Type type = Type::DUMMY;
			optix::float3 h_color = is_null? optix::make_float3(245, 225, 0) / 255.0f : object->get_Highlight_Color();		
			
			if ( !is_null )
			{
				type = object->getType();
				
				if      (type == GROUP_PARENT_DUMMY) obj_type = " GROUP ";
				else if (type == IMPORTED_OBJECT)    obj_type = " OBJECT ";
				else if (type == SAMPLER_OBJECT)     obj_type = " SAMPLER ";
				else if (type == FACE_VECTOR)        obj_type = " FACE VECTOR ";
				else if (type == LIGHT_OBJECT)
				{
					Light_Type t = object->get_LightParameters().type;
					if (t == QUAD_LIGHT)
						obj_type = " QUAD LIGHT ";
					if (t == SPHERICAL_LIGHT)
						obj_type = " SPHERICAL LIGHT ";
					if (t == POINT_LIGHT)
						obj_type = " POINT LIGHT ";
					if (t == SPOTLIGHT)
						obj_type = " SPOTLIGHT ";
				}
				else
				{
					obj_type = " UKNOWN ";
				}


			}
			
			bool changed = false;
			ImGui::Dummy(ImVec2(4.0f, 10.0f));
			
			// name
			{

				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("object_name##", ImVec2(ImGui::GetWindowWidth() - 12, 70.0f), true, window_flags);


				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::Text(" Name:   ");
				ImGui::PopFont();

				
				ImGui::Dummy(ImVec2(0.0f, 14.0f));


				// 
				if (!m_changing_name_is_active)
				{
					for (int i = 0; i < 101; i++)
						object_name[i] = '\0';

					for (int i = 0; i < name.size(); i++)
						object_name[i] = name[i];
				}
				//

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5f - 80.0f , 0.0f));
				ImGui::SameLine();
				ImGui::GetStyle().FrameRounding -= 4;
				ImGui::GetStyle().FramePadding += ImVec2(0.0f, 2.0f);
				ImGui::PushItemWidth(150.0f);
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col * 0.9f;
				ImGui::InputText("##", object_name , IM_ARRAYSIZE( object_name ));
				ImGui::GetStyle().Colors[ImGuiCol_Text] = col;
				ImGui::PopFont();
				bool is_activated = ImGui::IsItemActivated();
				bool is_active    = ImGui::IsItemActive();
				
				if (!is_null)
				{
					m_changing_name |= is_activated;
					m_changing_name_is_active = is_active;
				}

				ImGui::PopItemWidth();
				ImGui::GetStyle().FramePadding -= ImVec2(0.0f, 2.0f);
				ImGui::GetStyle().FrameRounding += 4;

				ImGui::EndChild();
			}

			ImGui::Dummy(ImVec2(0.0f, 0.0f));
			guiStyleManager::setStyle("main_style_right_child2");
			// type 
			{
				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("object_type##", ImVec2(ImGui::GetWindowWidth() - 12, 40.0f), true, window_flags);
				ImGui::Dummy(ImVec2(0.0f, 7.0f));
				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::Text(" Type:   ");
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::Text( obj_type.c_str() );
				ImGui::EndChild();
			}


			// LIGHT PARAMETERS WINDOW //
			if (type == LIGHT_OBJECT
				|| (m_selected_utility.id == u_CREATE_LIGHT) // or creating Light ... 
				)
			{
				optix::Context& m_context = Mediator::RequestContext();
				OptiXShader& shader       = Mediator::request_ActiveOptiXShaderId();
				std::string label = "";
				bool is_changed = false;
				bool is_null    = object == nullptr;
				if (!is_null)
					is_null = object->getType() != LIGHT_OBJECT ? true : false;

				Light_Parameters light_params;
				if (!is_null)
					light_params = object->get_LightParameters();

				Light_Type type;
				if (!is_null)
					type = light_params.type;


				bool is_create_light = m_selected_utility.id == u_CREATE_LIGHT;
				ImVec2 edit_size_3   = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 10.0f);
				ImVec2 edit_size_1   = ImVec2(ImGui::GetWindowWidth() - 12, 80.0f + 10.0f);
				ImVec2 edit_size_0   = ImVec2(ImGui::GetWindowWidth() - 12, 40.0f + 10.0f);

				ImGuiWindowFlags window_flags = 0;
				window_flags  |= ImGuiWindowFlags_NoTitleBar;
				window_flags  |= ImGuiWindowFlags_AlwaysAutoResize;
				//window_flags  |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
				ImGuiWindowFlags window_flags2 = 0;
				window_flags2 |= ImGuiWindowFlags_NoTitleBar;
				window_flags2 |= ImGuiWindowFlags_AlwaysAutoResize;
				window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;
				ImGuiColorEditFlags colorEdit_flags = 0;
				colorEdit_flags |= ImGuiColorEditFlags_NoLabel;
				colorEdit_flags |= ImGuiColorEditFlags_NoInputs;

				// input vars
				bool input_deactive = false;
				bool active_input_index_focused = false;
				optix::float2 m0_down_at = optix::make_float2(-1, -1);
				bool m0_down = false;
				bool focused_active_item = false;
				bool is_Changed = false;

				m_transformation_input_state_activating = false;

				int index_i = -1;
				int index_j = -1;
				float * v = 0;
				float prev_value = 0.0f;
				ImGuiID ID;
				float delta = 0.0f;
				bool was_INPUT_active = false;
				bool is_INPUT_active = false;
				//

				// CREATING LIGHT WINDOW //
				if (is_create_light)
				{

					guiStyleManager::setStyle("main_style_right_child3");
					ImGui::BeginChild("Sampler_Params_Child##", ImVec2(ImGui::GetWindowWidth() - 8.0f, 500 ), true, window_flags);
					{
						ImGui::Dummy(ImVec2(0.0f, 10.0f));

						guiStyleManager::setStyle("main_style_right_child_transformations");
						ImGui::BeginChild("light_type_child", edit_size_3 + ImVec2(0.0f, 20.0f), true, window_flags2);
						{
							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
							ImGui::Text(" Light Type ");
							ImGui::PopFont();

							ImGui::Dummy(ImVec2(0.0f, 28.0f));

							ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
							int e = m_interface_state.m_light_params.type;
							if (ImGui::RadioButton(" Quad Light", &e, 1))
								m_interface_state.m_light_params.type = QUAD_LIGHT;

							ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
							if (ImGui::RadioButton(" SpotLight", &e, 2))
								m_interface_state.m_light_params.type = SPOTLIGHT;

							ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
							if (ImGui::RadioButton(" Spherical Light", &e, 3))
								m_interface_state.m_light_params.type = SPHERICAL_LIGHT;

							ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
							if (ImGui::RadioButton(" Point Light", &e, 4))
								m_interface_state.m_light_params.type = POINT_LIGHT;

						}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");
						ImGui::Dummy(ImVec2(0.0f, 1.0f));

						guiStyleManager::setStyle("main_style_right_child_transformations");
						ImGui::BeginChild("light_alignement_child", edit_size_3 + ImVec2(0.0f, 10.0f), true, window_flags2);
						{

							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
							ImGui::Text(" Alignement ");
							ImGui::PopFont();

							ImGui::Dummy(ImVec2(0.0f, 28.0f));

							ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
							int e = m_interface_state.m_sa;
							if (ImGui::RadioButton(" Origin Plane", &e, 0))
								m_interface_state.m_sa = SA_GLOBAL;

							ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
							if (ImGui::RadioButton(" Surface Normal Align", &e, 2))
								m_interface_state.m_sa = SA_NORMAL;

							ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
							if (ImGui::RadioButton(" BBOX Align", &e, 3))
								m_interface_state.m_sa = SA_BBOX;


							ImGui::Dummy(ImVec2(0, 10));
							ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
							ImGui::Checkbox(" Flip Side", &m_interface_state.flip_side);


						}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");

						ImGui::Dummy(ImVec2(0.0f, 1.0f));

						guiStyleManager::setStyle("main_style_right_child_transformations");
						ImGui::BeginChild("plane_isec_light_child", edit_size_0, true, window_flags2);
						{
							ImGui::Dummy(ImVec2(0.0f, 10.0f));
							ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
							ImGui::Checkbox("Link To Object", &m_interface_state.link_to_object);

						}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");

						ImGui::Dummy(ImVec2(0.0f, 1.0f));


					}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child2");
					ImGui::Dummy(ImVec2(0.0f, 1.0f));
					
					//guiStyleManager::setStyle("main_style_right_child3");
					//ImGui::BeginChild("Sampler_Params_Child2223##", ImVec2(ImGui::GetWindowWidth() - 8.0f, ImGui::GetWindowHeight() - 20.0f - 600.0f - 2.0f - edit_size_0.y), true, window_flags);
					//ImGui::EndChild();
					//guiStyleManager::setStyle("main_style_right_child2");

				}
				
				// DEFAULT WINDOW //
				else
				{
					float MIN = 0.0f;
					float MAX = 999999.0f;
					float SP = 0.1f;
					float min = MIN;
					float max = MAX;
					float speed = SP;
					std::string child_labels[3] = { " Translation_Child##", "Rotation_Child##", "Scale_Child##" };
					std::string trs_labels[3]   = { " Translation", " Rotation", " Scale" };
					std::string axis_labels[3]  = { "R##", "G##", "B##" };
					ImVec2 materials_edit_size    = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 10.0f);
					ImVec2 materials_edit_size2   = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 40.0f);
					ImVec2 materials_edit_size_1f = ImVec2(ImGui::GetWindowWidth() - 12, 80.0f + 10.0f);


					guiStyleManager::setStyle("main_style_right_child2");
					float off_height  = 0.0f;
					float off_height2 = 0.0f;
					if (type == SPOTLIGHT) { off_height = 197.0f;  off_height2 = 90.0f; }
					else                   { off_height = 10.0f;   off_height2 = 40.0f; }
					ImGui::SetNextWindowContentSize(ImVec2(ImGui::GetWindowWidth() - 4.0f, ImGui::GetWindowHeight() + off_height ));
					ImGui::BeginChild("child_##", ImVec2(ImGui::GetWindowWidth() - 4.0f, ImGui::GetWindowHeight() - off_height2 ), true, window_flags);
					{
						ImGui::Dummy(ImVec2(0.0f, 4.0f));


						// Cast Shadows ( drag Int )
						{
							guiStyleManager::setStyle("main_style_right_child_transformations");
							ImGui::BeginChild("Is_On##", ImVec2(ImGui::GetWindowWidth() - 8, 40.0f + 10.0f), true, window_flags2);

							ImGui::Dummy(ImVec2(0, 13.0f));
							ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5 - 60.0f, 10.0f)); ImGui::SameLine();
							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Enabled: ");
							ImGui::PopFont();

							ImGui::SameLine();
							ImGui::Dummy(ImVec2(1.0f, 0.0f));
							ImGui::SameLine();
							ImGui::PushItemWidth(100.0f);


							// toggle button //
							{
								ImGuiStyle& style = ImGui::GetStyle();
								ImVec4 tcol = style.Colors[ImGuiCol_Text];
								ImVec4 ch_bg = style.Colors[ImGuiCol_ChildBg];
								ImVec4 bcol = style.Colors[ImGuiCol_Button];
								ImVec4 bcolh = style.Colors[ImGuiCol_ButtonHovered];// = ch_bg;
								ImVec4 bcola = style.Colors[ImGuiCol_ButtonActive]; //= ch_bg;

								float border = style.Colors[ImGuiCol_Border].w;

								style.Colors[ImGuiCol_Button] = ch_bg;
								style.Colors[ImGuiCol_ButtonHovered] = ch_bg;
								style.Colors[ImGuiCol_ButtonActive] = ch_bg;
								style.Colors[ImGuiCol_Border].w = 0.0f;

								ImVec2 pad_off = ImVec2(-2.0f, -2.0f);
								style.FramePadding += pad_off;

								ImVec2 button_offset = ImVec2(0, 0);
								ImGui::SameLine();
								ImGui::Dummy(button_offset);
								std::string toggle_but_label = light_params.is_on ? ICON_MDI_TOGGLE_SWITCH : ICON_MDI_TOGGLE_SWITCH_OFF;
								toggle_but_label += std::string("##");
								ImGui::SameLine();
								style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
								guiStyleManager::setIconFont(MDI, 20);

								if (!light_params.is_on)
								{
									style.Colors[ImGuiCol_Text] = ImVec4(0.65, 0.65, 0.65, 1);
								}
								if (ImGui::Button(toggle_but_label.c_str()))
								{
									light_params.is_on = !light_params.is_on;
									optix::Context context = Mediator::RequestContext();
									bool is_shadow = m_context["cast_shadow_ray"]->getInt();
									if (is_shadow)
										Mediator::restartAccumulation();
									is_Changed = true;
								}
								ImGui::PopFont();

								style.Colors[ImGuiCol_Text] = tcol;
								style.Colors[ImGuiCol_Button] = bcol;
								style.Colors[ImGuiCol_ButtonHovered] = bcolh;
								style.Colors[ImGuiCol_ButtonActive] = bcola;
								style.Colors[ImGuiCol_Border].w = border;
								style.FramePadding -= pad_off;
							}

							ImGui::PopItemWidth();

							ImGui::Dummy(ImVec2(2, 5));
							ImGui::EndChild();
						}


						// Emision 
						{
							float * var = ((float*)&light_params.emission_color);
							guiStyleManager::setStyle("main_style_right_child_transformations");
							ImGui::BeginChild("emission_window##", materials_edit_size2, true, window_flags2);

							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
							ImGui::Text(" Emission ");

							ImGui::SameLine();
							ImGui::GetStyle().FrameRounding -= 3;
							ImGui::GetStyle().FramePadding -= ImVec2(1.0f, 1.0f);
							if (ImGui::ColorEdit3("##", var, colorEdit_flags))
								is_Changed = true;
							ImGui::GetStyle().FrameRounding += 3;
							ImGui::GetStyle().FramePadding += ImVec2(1.0f, 1.0f);
							ImGui::PopFont();

							ImGui::Dummy(ImVec2(0, 10.0f));
							guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
							for (int j = 0; j < 3; j++)
							{
								//ImGui::Dummy(ImVec2(0.0f, 0.0f));
								ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
								ImGui::SameLine();
								ImGui::PushItemWidth(100.0f);
								float old_v = var[j];
								if (ImGui::DragFloat(axis_labels[j].c_str(), &var[j], 0.001f, 0.0f, 1.0f))
								{
									is_Changed = true;
									m_dragFloat_state = true;
									if (m_repositioning_cursor) var[j] = old_v;
								}

								ImGui::PopItemWidth();
							}

							ImGui::PushItemWidth(100.0f);
							ImGui::Dummy(ImVec2(0, 20.0f));
							ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
							ImGui::SameLine();
							float * energy = &light_params.radiant_exitance;
							float old_v = *energy;
							if (ImGui::DragFloat("Rdnt. Ext.", energy, 1.0f, 0.0f, 99999999.9f))
							{
								is_Changed = true;
								m_dragFloat_state = true;
							}
							if (m_repositioning_cursor) *energy = old_v;
							ImGui::PopItemWidth();

							ImGui::PopFont();

							ImGui::EndChild();
							guiStyleManager::setStyle("main_style_right_child2");
							
						}


						// Sizes : ( width , length , height , radius , etc.. )
						{
							float a = type == SPOTLIGHT ? 2.0f : 1.0f;

							light_params.du *= a;
							light_params.dv *= a;

							float * width = &light_params.du;
							float * length = &light_params.dv;
							float * radius = &light_params.radius;

							guiStyleManager::setStyle("main_style_right_child_transformations");
							ImGui::BeginChild("Light_Sizes## ", materials_edit_size + ImVec2(0.0f, 30.0f), true, window_flags2);

							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Dimensions ");
							ImGui::PopFont();

							ImGui::Dummy(ImVec2(0, 10.0f));
							guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);


							ImGui::Dummy(ImVec2(0.0f, 0.0f));

							if (type == QUAD_LIGHT || type == SPOTLIGHT)
							{

								ImGui::PushItemWidth(100.0f);

								// width
								ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
								ImGui::SameLine();

								//
								v = width;
								prev_value = *v;
								index_i = 0;
								index_j = -1;

								//
								ImGui::DragFloat(" Width ", v, speed, min, max);
								ID = ImGui::GetCurrentWindow()->GetID(" Width ");
								was_INPUT_active = m_transformation_input_state;
								is_INPUT_active = ImGui::TempInputTextIsActive(ID);
								if (m_repositioning_cursor) { *v = prev_value; }
								delta = *v - prev_value;
								if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
								{
									////std::cout << " - focused_active_item : " << index_i << std::endl;
									focused_active_item = true;
								}
								if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
								{

									bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
									if (activating_input_state && !m_transformation_input_state)
									{
										m_transformation_input_state = true;
										m_transformation_input_state_activating = true;
									}
									m_transformation_input_attr_index = index_j;
									m_transformation_input_i = index_i;
									m_transformation_input_m0_down = true;
									m_transformation_input_m0_release = false;
									m_transformation_input_data.starting_delta = prev_value;
								}
								if (delta != 0 && !is_null)
								{
									optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
									optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
									optix::float3 delta_3f = curr_attributes - obj_attributes;
									m_transformation_input_data.delta = delta_3f;
									m_transformation_input_data.delta_length = delta;

									if (delta != 0) m_dragFloat_state = true;
									is_Changed = true;
								}
								if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
								{
									m0_down_at = optix::make_float2(index_i, index_j);
									m0_down = true;
								}


								// length
								ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
								ImGui::SameLine();

								//
								v = length;
								prev_value = *v;
								index_i = 1;
								index_j = -1;

								//
								ImGui::DragFloat(" Length ", v, speed, min, max);
								ID = ImGui::GetCurrentWindow()->GetID(" Length ");
								was_INPUT_active = m_transformation_input_state;
								is_INPUT_active = ImGui::TempInputTextIsActive(ID);
								if (m_repositioning_cursor) { *v = prev_value; }
								delta = *v - prev_value;
								if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
								{
									focused_active_item = true;
								}
								if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
								{
									bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
									if (activating_input_state && !m_transformation_input_state)
									{
										m_transformation_input_state = true;
										m_transformation_input_state_activating = true;
									}
									m_transformation_input_attr_index = index_j;
									m_transformation_input_i = index_i;
									m_transformation_input_m0_down = true;
									m_transformation_input_m0_release = false;
									m_transformation_input_data.starting_delta = prev_value;
								}
								if (delta != 0 && !is_null)
								{
									optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
									optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
									optix::float3 delta_3f = curr_attributes - obj_attributes;
									m_transformation_input_data.delta = delta_3f;
									m_transformation_input_data.delta_length = delta;

									if (delta != 0) m_dragFloat_state = true;
									is_Changed = true;
								}
								if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
								{
									m0_down_at = optix::make_float2(index_i, index_j);
									m0_down = true;
								}
								delta = 0.0f;



								ImGui::PopItemWidth();

							}

							if (type == SPHERICAL_LIGHT)
							{

								// Radius
								ImGui::Dummy(ImVec2(0.0f, 20.0f));
								ImGui::PushItemWidth(100.0f);


								ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
								ImGui::SameLine();

								//
								v = radius;
								prev_value = *v;
								index_i = 2; index_j = -1;

								//
								ImGui::DragFloat(" Radius ", v, speed, min, max);
								ID = ImGui::GetCurrentWindow()->GetID(" Radius ");
								was_INPUT_active = m_transformation_input_state;
								is_INPUT_active = ImGui::TempInputTextIsActive(ID);
								if (m_repositioning_cursor) { *v = prev_value; }
								delta = *v - prev_value;
								if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
								{
									focused_active_item = true;
								}
								if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
								{
									bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
									if (activating_input_state && !m_transformation_input_state)
									{
										m_transformation_input_state = true;
										m_transformation_input_state_activating = true;
									}
									m_transformation_input_attr_index = index_j;
									m_transformation_input_i = index_i;
									m_transformation_input_m0_down = true;
									m_transformation_input_m0_release = false;
									m_transformation_input_data.starting_delta = prev_value;
								}
								if (delta != 0 && !is_null)
								{
									optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
									optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
									optix::float3 delta_3f = curr_attributes - obj_attributes;
									m_transformation_input_data.delta = delta_3f;
									m_transformation_input_data.delta_length = delta;

									if (delta != 0) m_dragFloat_state = true;
									is_Changed = true;
								}
								if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
								{
									m0_down_at = optix::make_float2(index_i, index_j);
									m0_down = true;
								}

								//

								ImGui::PopItemWidth();
							}


							ImGui::Dummy(ImVec2(2, 5));
							ImGui::PopFont();
							ImGui::EndChild();
						}


						// Cone Dimensions
						if (type == SPOTLIGHT)
						{
							float * cone_size = (float*)&light_params.cone_size;

							guiStyleManager::setStyle("main_style_right_child_transformations");
							ImGui::BeginChild("Cone_Dimensions## ", materials_edit_size + ImVec2(0.0f, 100.0f), true, window_flags2);

							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Cone Dimensions ");
							ImGui::PopFont();

							ImGui::Dummy(ImVec2(0, 10.0f));
							guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

							ImGui::PushItemWidth(100.0f);



							ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
							ImGui::SameLine();

							//
							v = &cone_size[0];
							prev_value = *v;
							index_i = 3; index_j = -1;

							//
							ImGui::DragFloat(" Umbra ", v, speed, min, max);
							ID = ImGui::GetCurrentWindow()->GetID(" Umbra ");
							was_INPUT_active = m_transformation_input_state;
							is_INPUT_active = ImGui::TempInputTextIsActive(ID);
							if (m_repositioning_cursor) { *v = prev_value; }
							delta = *v - prev_value;
							if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
							{
								focused_active_item = true;
							}
							if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
							{
								bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
								if (activating_input_state && !m_transformation_input_state)
								{
									m_transformation_input_state = true;
									m_transformation_input_state_activating = true;
								}
								m_transformation_input_attr_index = index_j;
								m_transformation_input_i = index_i;
								m_transformation_input_m0_down = true;
								m_transformation_input_m0_release = false;
								m_transformation_input_data.starting_delta = prev_value;
							}
							if (delta != 0 && !is_null)
							{
								optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
								optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
								optix::float3 delta_3f = curr_attributes - obj_attributes;
								m_transformation_input_data.delta = delta_3f;
								m_transformation_input_data.delta_length = delta;

								if (delta != 0) m_dragFloat_state = true;
								is_Changed = true;
							}
							if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
							{
								m0_down_at = optix::make_float2(index_i, index_j);
								m0_down = true;
							}






							ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
							ImGui::SameLine();

							//
							v = &cone_size[1];
							prev_value = *v;
							index_i = 4; index_j = -1;

							//
							ImGui::DragFloat(" Penumbra ", v, speed, min, max);
							ID = ImGui::GetCurrentWindow()->GetID(" Penumbra ");
							was_INPUT_active = m_transformation_input_state;
							is_INPUT_active = ImGui::TempInputTextIsActive(ID);
							if (m_repositioning_cursor) { *v = prev_value; }
							delta = *v - prev_value;
							if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
							{
								focused_active_item = true;
							}
							if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
							{
								bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
								if (activating_input_state && !m_transformation_input_state)
								{
									m_transformation_input_state = true;
									m_transformation_input_state_activating = true;
								}
								m_transformation_input_attr_index = index_j;
								m_transformation_input_i = index_i;
								m_transformation_input_m0_down = true;
								m_transformation_input_m0_release = false;
								m_transformation_input_data.starting_delta = prev_value;
							}
							if (delta != 0 && !is_null)
							{
								optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
								optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
								optix::float3 delta_3f = curr_attributes - obj_attributes;
								m_transformation_input_data.delta = delta_3f;
								m_transformation_input_data.delta_length = delta;

								if (delta != 0) m_dragFloat_state = true;
								is_Changed = true;
							}
							if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
							{
								m0_down_at = optix::make_float2(index_i, index_j);
								m0_down = true;
							}



							ImGui::PopItemWidth();


							//
							ImGui::Dummy(ImVec2(0.0f, 25.0f));
							ImGui::Dummy(ImVec2(12.0f, 0.0f));
							ImGui::SameLine();
							
							// is soft
							{
								guiStyleManager::setStyle("main_style_right_child_transformations");
								ImGui::BeginChild("Is_Soft##", ImVec2(ImGui::GetWindowWidth() - 8 - 30, 40.0f + 60.0f), true, window_flags2);

								guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14); ImGui::Dummy(ImVec2(40.0f, 0.0)); ImGui::SameLine(); ImGui::Text(" Interpolation Function: ");
								ImGui::PopFont();
								ImGui::Dummy(ImVec2(0.0f, 20.0f));

								int e = light_params.is_soft ? 1 : 0;

								ImGui::PushItemWidth(100.0f);
								ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 70.0f, 0.0f));
								ImGui::SameLine();

								if (ImGui::RadioButton("Linear Interpolation", &e, 0))
								{
									is_Changed = true;
									light_params.is_soft = 0;
								}
								ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 70.0f, 0.0f));
								ImGui::SameLine();
								if (ImGui::RadioButton("Cubic Interpolation", &e, 1))
								{
									is_Changed = true;
									light_params.is_soft = 1;
								}

								ImGui::PopItemWidth();
								ImGui::Dummy(ImVec2(2, 5));
								ImGui::EndChild();
							}
							//


							ImGui::Dummy(ImVec2(2, 5));
							ImGui::PopFont();
							ImGui::EndChild();
						}


						// Frustum Dimensions
#define light_frustum_dimensions
#ifdef light_frustum_dimensions

						{
							float * cp_size = (float*)&light_params.clipping_plane;

							guiStyleManager::setStyle("main_style_right_child_transformations");
							ImGui::BeginChild("Frustum_Dimensions## ", materials_edit_size + ImVec2(0.0f, 0.0f), true, window_flags2);

							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Clipping Plane ");
							ImGui::PopFont();

							ImGui::Dummy(ImVec2(0, 10.0f));
							guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

							ImGui::Dummy(ImVec2(0.0f, 0.0f));
							ImGui::PushItemWidth(100.0f);

							//

							ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
							ImGui::SameLine();

							//
							v = &cp_size[0];
							prev_value = *v;
							index_i = 5; index_j = -1;

							//
							ImGui::DragFloat(" Near ", v, speed, min, max);
							ID = ImGui::GetCurrentWindow()->GetID(" Near ");
							was_INPUT_active = m_transformation_input_state;
							is_INPUT_active = ImGui::TempInputTextIsActive(ID);
							if (m_repositioning_cursor) { *v = prev_value; }
							delta = *v - prev_value;
							if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
							{
								focused_active_item = true;
							}
							if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
							{
								bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
								if (activating_input_state && !m_transformation_input_state)
								{
									m_transformation_input_state = true;
									m_transformation_input_state_activating = true;
								}
								m_transformation_input_attr_index = index_j;
								m_transformation_input_i = index_i;
								m_transformation_input_m0_down = true;
								m_transformation_input_m0_release = false;
								m_transformation_input_data.starting_delta = prev_value;
							}
							if (delta != 0 && !is_null)
							{
								optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
								optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
								optix::float3 delta_3f = curr_attributes - obj_attributes;
								m_transformation_input_data.delta = delta_3f;
								m_transformation_input_data.delta_length = delta;

								if (delta != 0) m_dragFloat_state = true;
								is_Changed = true;
							}
							if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
							{
								m0_down_at = optix::make_float2(index_i, index_j);
								m0_down = true;
							}

							ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
							ImGui::SameLine();

							//
							v = &cp_size[1];
							prev_value = *v;
							index_i = 6; index_j = -1;

							//
							ImGui::DragFloat(" Far ", v, speed, min, max);
							ID = ImGui::GetCurrentWindow()->GetID(" Far ");
							was_INPUT_active = m_transformation_input_state;
							is_INPUT_active = ImGui::TempInputTextIsActive(ID);
							if (m_repositioning_cursor) { *v = prev_value; }
							delta = *v - prev_value;
							if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
							{
								focused_active_item = true;
							}
							if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
							{
								bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
								if (activating_input_state && !m_transformation_input_state)
								{
									m_transformation_input_state = true;
									m_transformation_input_state_activating = true;
								}
								m_transformation_input_attr_index = index_j;
								m_transformation_input_i = index_i;
								m_transformation_input_m0_down = true;
								m_transformation_input_m0_release = false;
								m_transformation_input_data.starting_delta = prev_value;
							}
							if (delta != 0 && !is_null)
							{
								optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
								optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
								optix::float3 delta_3f = curr_attributes - obj_attributes;
								m_transformation_input_data.delta = delta_3f;
								m_transformation_input_data.delta_length = delta;

								if (delta != 0) m_dragFloat_state = true;
								is_Changed = true;
							}
							if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
							{
								m0_down_at = optix::make_float2(index_i, index_j);
								m0_down = true;
							}


							//
							ImGui::PopItemWidth();



							//
							ImGui::Dummy(ImVec2(0.0f, 25.0f));
							ImGui::Dummy(ImVec2(12.0f, 0.0f));
							ImGui::SameLine();

							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14); ImGui::Text(" Visualize Distance Clipping:  "); ImGui::PopFont();
							ImGui::SameLine();
							if (ImGui::Checkbox("##", &light_params.is_clip))
							{
								optix::Context context = Mediator::RequestContext();
								bool is_shadow = m_context["cast_shadow_ray"]->getInt();
								if (is_shadow)
									Mediator::restartAccumulation();
								is_Changed = true;
							}

							ImGui::Dummy(ImVec2(2, 5));
							ImGui::PopFont();
							ImGui::EndChild();
						}

#endif

					}ImGui::EndChild();


#define light_inputs_update_block
#ifdef light_inputs_update_block

					if (!is_null)
					{
						bool vk_key = key_data[GLFW_KEY_ENTER].down || (key_data[GLFW_KEY_TAB].down && !m_transformation_input_state_activating);
						bool m0_DOWN = ImGui::IsMouseClicked(0);
						bool m0_REL = ImGui::IsMouseReleased(0);
						bool save_ac = 0;
						bool reset_state_1 = 0;
						bool reset_state_2 = 0;
						bool reset_state = 0;
						bool input_state = 0;

						bool dragging_state_end = (m_transformation_input_m0_down && !m_transformation_input_state_activating) || m_plus_minus_button_state;
						bool input_state_end = (m0_DOWN && !focused_active_item && !m_transformation_input_state_activating) || vk_key;

						int trs_i = m_transformation_input_i;
						int ind_j = m_transformation_input_attr_index;

						float delta = m_transformation_input_data.delta_length;
						optix::float3 delta_3f = GLOBAL_AXIS[ind_j] * delta;
						optix::float3 working_axis[3] = { GLOBAL_AXIS[0], GLOBAL_AXIS[1], GLOBAL_AXIS[2] };
						BASE_TRANSFORM::AXIS(object, LOCAL, working_axis);

						float x = type == SPOTLIGHT ? 0.5f : 1.0f;
						bool is_type_valid = type != SPOTLIGHT && type != POINT_LIGHT;


						// Width Var
						if (trs_i == 0)
						{
							delta_3f = optix::make_float3(delta, 0.0f, 0.0f) * x;
						}
						// Length Var
						else if (trs_i == 1)
						{
							delta_3f = optix::make_float3(0.0f, 0.0f, delta) * x;
						}
						// Radius Var
						else if (trs_i == 2)
						{
							delta_3f = optix::make_float3(delta) * 1.0f;
						}
						// ConeDim Var
						else if (trs_i == 3 || trs_i == 4)
						{
							if (trs_i == 3) delta_3f = optix::make_float3(delta, 0.0f, 0.0f);
							else            delta_3f = optix::make_float3(0.0f, delta, 0.0f);

						}
						// ClipPlanes Var
						else if (trs_i == 5 || trs_i == 6)
						{
							if (trs_i == 5) delta_3f = optix::make_float3(delta, 0.0f, 0.0f);
							else            delta_3f = optix::make_float3(0.0f, delta, 0.0f);

						}


						if (type == SPOTLIGHT)
						{
							light_params.du *= 0.5f;
							light_params.dv *= 0.5f;
						}

						
						if (is_Changed)
						{
							// apply Transformation here ...
							if (is_type_valid && (trs_i == 0 || trs_i == 1 || trs_i == 2))
							{
								object->setTransformation(delta_3f,
														  AXIS_ORIENTATION::LOCAL,
														  SCALE,
														  ONLY_THIS,
														  0,
														  working_axis);

							}

							// update Light's Vars
							object->set_LightParameters(light_params, true);


							// accumulate transformation delta
							m_transformation_input_data.overall_delta += delta_3f;
						}
						else
						{
							delta_3f = ZERO_3f;
							delta = 0.0f;
							m_transformation_input_data.delta_length = 0.0f;
							m_transformation_input_data.delta = ZERO_3f;
						}

						// transformation input active
						if (m_transformation_input_state)
						{
							if (input_state_end)
							{
								reset_state_1 = 1;
								input_state = 1;
							}
						}
						else
						{
							if (m0_REL)
							{
								if (dragging_state_end)
								{

									reset_state_2 = 1;
									input_state = 0;
								}

							}
						}

						reset_state = reset_state_1 || reset_state_2;
						if (reset_state)
						{
							
							optix::float3 overall_delta = m_transformation_input_data.overall_delta;
							float delta_length = length(overall_delta);
							save_ac = fabsf(delta_length) > 1e-5f ? 1 : 0;
							if (save_ac && is_type_valid && (trs_i == 0 || trs_i == 1 || trs_i == 2))
							{
								// pushing Action ... 
								Action * ac = 0;
								ACTION_ID ac_id = ACTION_TRANSFORMATION_SCALE;

								ac = new Action_TRANSFORMATION(
									object,
									overall_delta,
									working_axis,
									LOCAL,
									ONLY_THIS,
									ac_id);

								ActionManager::GetInstance().push(ac);
							}
							else if (trs_i == 3 || trs_i == 4)
							{
								Action * ac = new Action_Edit_LIGHT(object, overall_delta, 0);
								ActionManager::GetInstance().push(ac);
							}
							else if (trs_i == 5 || trs_i == 6)
							{
								Action * ac = new Action_Edit_LIGHT(object, overall_delta, 1);
								ActionManager::GetInstance().push(ac);
							}


							// reset data //
							{
								m_transformation_input_data.reset();
								m_transformation_input_i = -1;
								m_transformation_input_attr_index = -1;
								m_transformation_input_state = false;
								m_transformation_input_state_activating = false;
								m_plus_minus_button_state = false;
							}

						}
					}

					if ((is_Changed || is_changed)
						&& !is_null
						)
					{
						bool rr_acc = 0;
						optix::Context context = Mediator::RequestContext();
						bool is_shadow = m_context["cast_shadow_ray"]->getInt();
						rr_acc = is_shadow && light_params.is_on;
						if (rr_acc)
							Mediator::restartAccumulation();
					}

#endif

				}

			}


			// SAMPLER PARAMETERS WINDOW //
			if(type == SAMPLER_OBJECT)
			{

			}


			ImGui::Dummy(ImVec2(0.0f, 0.0f));
			guiStyleManager::setStyle("main_style_right_child2");
		}
		ImGui::EndChild();
	}


	ImGui::PopFont();
}
void InterfaceManager::construct_Transformations_child_Window()
{
	process_TAB_SELECTION_STATE_change(TRANSFORMATIONS_TAB);
	reset_Tabs_State(TRANSFORMATIONS_TAB);


	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

	std::string label = "";
	SceneObject* object = m_selected_object_pack.raw_object;
	bool is_null = object == nullptr;

	ImVec2 offset = ImVec2(11.0f, 0.0f);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	ImGuiWindowFlags window_flags2 = 0;
	window_flags2 |= ImGuiWindowFlags_NoTitleBar;
	window_flags2 |= ImGuiWindowFlags_AlwaysAutoResize;
	//window_flags2 |= ImGuiWindowFlags_NoScrollbar;
	window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;


	// TRANSFORMATIONS TAB //
	{
		std::string child_labels[3] = { " Translation_Child##", "Rotation_Child##", "Scale_Child##" };
		std::string trs_labels[3]   = { " Translation", " Rotation", " Scale" };
		std::string axis_labels[3]  = { " X:", " Y:", " Z:" };
		bool input_deactive = false;
		bool active_input_index_focused = false;
		optix::float2 m0_down_at = optix::make_float2(-1, -1);
		bool m0_down             = false;
		bool focused_active_item = false;
		bool is_Changed          = false;
		bool is_INPUT_active     = false;
		m_transformation_input_state_activating = false;


		// Transformations - Child Window
		guiStyleManager::setStyle("main_style_right_child2");
		ImGui::BeginChild("Transformation_child##", ImVec2(ImGui::GetWindowWidth() - 8.0f, ImGui::GetWindowHeight() - 40.0f), true, window_flags );
		{

			optix::float3 attributes[3] = { ZERO_3f, ZERO_3f, ZERO_3f };
			
			if ( !is_null )
			{
				attributes[0] = object->getTranslationAttributes();
				attributes[1] = object->getRotationAttributes();
				attributes[2] = object->getScaleAttributes();

				for (int i = 0; i < 3; i++)
				{
					float v = ((float*)&attributes[1])[i];
					((float*)&attributes[1])[i] = Geometry::GENERAL::toDegrees(v);
				}

				optix::float3 euler_angles;
				Geometry::GENERAL::EulerAnglesFromMatrix( object->getRotationMatrix_chain(), euler_angles );
				m_euler_starting_values = euler_angles;
				euler_angles = Geometry::GENERAL::toDegrees( euler_angles );
				attributes[1].x = euler_angles.x;
				attributes[1].y = euler_angles.y;
				attributes[1].z = euler_angles.z;
				
			}


			ImGui::Dummy(ImVec2(4.0f, 10.0f));
			{
				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("transformation_setting##", ImVec2(ImGui::GetWindowWidth() - 8, 125.0f + 10.0f), true, window_flags2);
				
				ImGui::Dummy(ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				guiStyleManager::setIconFont(MDI, 16);
				
				ImGui::Text(ICON_MDI_TRANSIT_CONNECTION_VARIANT);
				ImGui::PopFont();

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::SameLine();
				ImGui::Text(" Transformation Inheritance  ");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0.0f, 18.0f));

				ImGuiStyle& style = ImGui::GetStyle();
				ImVec4 def_col = style.Colors[ImGuiCol_Button];
				ImVec4 ac_col = style.Colors[ImGuiCol_ButtonActive];
				ImVec4 col[3];
				col[0] = m_hierarchy_setting == TO_ALL ? ac_col : def_col;
				col[1] = m_hierarchy_setting == ONLY_TO_CHILDS ? ac_col : def_col;
				col[2] = m_hierarchy_setting == ONLY_THIS ? ac_col : def_col;

				ImVec2 button_size = ImVec2(150.0f, 20.0f);
				ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 80.0f, 0.0f));
				ImGui::SameLine();
				style.Colors[ImGuiCol_Button] = col[0];
				if (ImGui::Button("Default", button_size))
					m_hierarchy_setting = TRANSFORMATION_SETTING::TO_ALL;
				
				ImGui::Dummy(ImVec2(0.0f, 1.0f));

				ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 80.0f, 0.0f));
				ImGui::SameLine();
				style.Colors[ImGuiCol_Button] = col[1];
				if (ImGui::Button("   Affect only Hierarchy   ", button_size))
					m_hierarchy_setting = TRANSFORMATION_SETTING::ONLY_TO_CHILDS;

				ImGui::Dummy(ImVec2(0.0f, 1.0f));
				
				ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 80.0f, 0.0f));
				ImGui::SameLine();
				style.Colors[ImGuiCol_Button] = col[2];
				if (ImGui::Button("    Affect only Object    ", button_size))
					m_hierarchy_setting = TRANSFORMATION_SETTING::ONLY_THIS;

				style.Colors[ImGuiCol_Button] = def_col;
				ImGui::EndChild();
			}
			
			ImGui::Dummy(ImVec2(0.0f, 0.0f));
			for (int i = 0; i < 3; i++)
			{
				m_transformation_input[i] = false;

				guiStyleManager::setStyle("main_style_right_child_transformations");
				float y_off = 0.0f;//i == 1 ? 170.0f : 0.0f;
				ImGui::BeginChild(child_labels[i].c_str(), ImVec2(ImGui::GetWindowWidth() - 8, 125.0f + 25.0f + y_off ), true, window_flags2);
				
				ImGui::Dummy(ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				guiStyleManager::setIconFont(MDI, 16);
				if (i == 0) ImGui::Text(ICON_MDI_AXIS_ARROW);
				if (i == 1) ImGui::Text(ICON_MDI_AXIS_Z_ROTATE_CLOCKWISE);
				if (i == 2) ImGui::Text(ICON_MDI_AXIS);
				ImGui::PopFont();

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::SameLine();
				ImGui::Text(trs_labels[i].c_str());
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0, 18.0f));

				guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
				for (int j = 0; j < 3; j++)
				{
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					float * attr_i = (float*)&attributes[i];

					optix::float3 prev_values = attributes[i];
					float prev_value = attr_i[j];
					
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 85.0f , 0.0f) + offset);
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);

					float speed = (i == 2) ? 0.01f : 0.1f;
					ImGui::Text(axis_labels[j].c_str());
					ImGui::SameLine();
					std::string label = std::string("##") + std::to_string(i) + " " + std::to_string(j);
					
					
					ImGui::DragFloat(label.c_str(), &attr_i[j], speed , -999999.0f, 9999999.0f);

					ImGuiID ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
					bool was_INPUT_active = m_transformation_input_state;
					bool is_INPUT_active  = ImGui::TempInputTextIsActive( ID );
					if (m_repositioning_cursor) { attr_i[j] = prev_value; }
					float delta           = attr_i[j] - prev_value;
					if (i == 1)
					{
						float pr_v = Geometry::GENERAL::toRadians(prev_value);
						float cr_v = Geometry::GENERAL::toRadians(attr_i[j]);
						float r_delta = cr_v - pr_v;
						delta = r_delta;
					}
					if (ImGui::IsItemHovered() && (m_transformation_input_i == i && m_transformation_input_attr_index == j))
					{
						focused_active_item = true;
					}
					if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
					{
						
					
						////std::cout << "\n - Activated Item : " << i << std::endl;
						////std::cout << " - transformation_input_i : " << m_transformation_input_i << std::endl;
						////std::cout << " - transformation_input_state : " << m_transformation_input_state << std::endl;
						////std::cout << " - is_INPUT_acitve : " << is_INPUT_active << std::endl;
						bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
						if ( activating_input_state && !m_transformation_input_state )
						{
							////std::cout << " - Activating Input on : Item : [ " << i << " , " << j << " ] " << std::endl;
							////std::cout << "    - INPUT_STATE : ON!" << std::endl;
							//focused_active_item = true;
							m_transformation_input_state = true;
							m_transformation_input_state_activating = true;
							m_transformation_input_data.starting_plane_delta = ( m_euler_starting_values );
						}

						m_transformation_input_attr_index = j;
						m_transformation_input_i          = i;
						
						m_transformation_input_m0_down    = true;
						m_transformation_input_m0_release = false;
						m_transformation_input_data.starting_delta       = prev_value;
					}
					if (delta != 0 && !is_null)
					{
						
						optix::float3 obj_attributes  = prev_values; 
						optix::float3 curr_attributes = optix::make_float3(attr_i[0], attr_i[1], attr_i[2]);
						optix::float3 delta_3f        = curr_attributes - obj_attributes;
						
						m_transformation_input_data.delta        = delta_3f;
						m_transformation_input_data.delta_length = delta;
						m_euler_starting_values = prev_values;
						//m_transformation_input_data.overall_delta += delta_3f;

						if (delta != 0)
							m_dragFloat_state = true;
						is_Changed = true;
					}
					if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
					{
						m0_down_at = optix::make_float2(i, j);
						m0_down    = true;
					}
					
					// plus - minus buttons //
					{
						guiStyleManager::setIconFont(MDI, 12);
						
						float var = 5.0f;
						int k = 0;
						ImGui::GetStyle().FramePadding -= ImVec2(1,1);
						float d_rd = ImGui::GetIO().KeyRepeatDelay;
						float d_rr = ImGui::GetIO().KeyRepeatRate;

						float speed = 1.0f;
						if (i == 1)      speed = 0.0174532925f;
						else if (i == 2) speed = 0.01f;

						ImGui::GetIO().KeyRepeatDelay = 0.15f;
						ImGui::GetIO().KeyRepeatRate  = 0.075f;
						ImGui::PushButtonRepeat(true);
						ImGui::SameLine();
						std::string l1 = ICON_MDI_PLUS + std::string("##") + std::to_string(i) + std::to_string(j);
						if (ImGui::Button(l1.c_str()) && !is_null)
						{
							m_plus_minus_button_state         = true;
							m_transformation_input_i          = i;
							m_transformation_input_attr_index = j;
							k = 1;

							m_transformation_input_data.delta_length = 5.0f * k * speed;
							is_Changed = true;
						}
						ImGui::SameLine();
						std::string l2 = ICON_MDI_MINUS + std::string("##") + std::to_string(i) + std::to_string(j);
						if (ImGui::Button(l2.c_str()) && !is_null)
						{
							m_plus_minus_button_state         = true;
							m_transformation_input_i          = i;
							m_transformation_input_attr_index = j;
							k = -1;

							m_transformation_input_data.delta_length = 5.0f * k * speed;
							is_Changed = true;
						}
						ImGui::PushButtonRepeat(false);
						ImGui::GetIO().KeyRepeatDelay = d_rd;
						ImGui::GetIO().KeyRepeatRate  = d_rr;
						ImGui::GetStyle().FramePadding += ImVec2(1,1);
						ImGui::PopFont();

						if (k != 0 && !is_null)
						{
							
						}
					}

					ImGui::PopItemWidth();	
				}


				ImGui::Dummy(ImVec2(0.0f, 4.0f));
				// Reset button
				{
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 40.0f, 0.0f) + offset);
					ImGui::SameLine();

					ImGui::GetStyle().FramePadding.y -= 0;
					ImGui::PushItemWidth( 90.0f );
					if (ImGui::Button(" Reset ") && !is_null)
					{
						bool is_valid = true;//!( (m_hierarchy_setting == ONLY_TO_CHILDS) || ( object->getType() == GROUP_PARENT_DUMMY && m_hierarchy_setting == ONLY_THIS) );

						optix::float3 e_angles;
						optix::float3 attr = object->getAttributes(i);
						if (i == 2) is_valid &= (attr.x != 1 || attr.y != 1 || attr.z != 1);
						if (i == 0) is_valid &= (attr.x != 0 || attr.y != 0 || attr.z != 0);
						if (i == 1)
						{
							Geometry::GENERAL::EulerAnglesFromMatrix( object->getRotationMatrix_chain(), e_angles );
							is_valid &= ((attr.x != 0 || attr.y != 0 || attr.z != 0) || ( fabsf(e_angles.x) > 1e-5 || fabsf(e_angles.y) > 1e-5 || fabsf(e_angles.z) > 1e-5 ));
						}


						is_valid &= m_hierarchy_setting != ONLY_TO_CHILDS;
						if ( is_valid )
						{
							////std::cout << "\n - angles : [ " <<  e_angles.x << " , " << e_angles.y << " , " << e_angles.z << " ] " << std::endl;
							////std::cout << " - attr   : [ " << attr.x << " , " << attr.y << " , " << attr.z << " ] " << std::endl;
							
							TRANSFORMATION_SETTING t_setting = m_hierarchy_setting;
							Action * ac = new Action_SET_MATRIX( object, ( TRANSFORMATION_TYPE )i, t_setting , true );
							ActionManager::GetInstance().push( ac );
						}
					}
					ImGui::PopItemWidth();
					ImGui::GetStyle().FramePadding.y += 0;
				}

				ImGui::PopFont();

				ImGui::Dummy(ImVec2(2, 5));
				ImGui::EndChild();
				
			}
			guiStyleManager::setStyle("main_style_right_child2");


			// update object's Transformation //
			if ( !is_null )
			{

				bool vk_key  = ( key_data[GLFW_KEY_ENTER].down || key_data[GLFW_KEY_KP_ENTER].down ) || (key_data[GLFW_KEY_TAB].down && !m_transformation_input_state_activating);
				bool m0_DOWN = ImGui::IsMouseClicked(0);
				bool m0_REL  = ImGui::IsMouseReleased(0);
				bool save_ac = 0;
				bool reset_state_1 = 0;
				bool reset_state_2 = 0;
				bool reset_state = 0;
				bool input_state = 0;

				bool dragging_state_end = (m_transformation_input_m0_down  && !m_transformation_input_state_activating) || m_plus_minus_button_state;
				bool input_state_end    = (m0_DOWN && !focused_active_item && !m_transformation_input_state_activating) || vk_key;

	
				int trs_i = m_transformation_input_i;
				int ind_j = m_transformation_input_attr_index;

				float delta            = m_transformation_input_data.delta_length;
				optix::float3 delta_3f = GLOBAL_AXIS[ind_j] * delta;
				optix::Matrix4x4 d_rot_mat = optix::Matrix4x4::identity();

				////std::cout << "\n  - transforming! " << std::endl;
				////std::cout << "  - delta : " << delta << std::endl;
				////std::cout << "  - trs_i : " << trs_i << std::endl;
				////std::cout << "  - ind_j : " << ind_j << std::endl;

				AXIS_ORIENTATION       selected_axis = m_selected_axis_orientation;
				TRANSFORMATION_TYPE    t_type        = (TRANSFORMATION_TYPE)trs_i;
				TRANSFORMATION_SETTING t_setting     = m_hierarchy_setting;
					
				optix::float3 working_axis[3] = { GLOBAL_AXIS[0], GLOBAL_AXIS[1], GLOBAL_AXIS[2] };
				BASE_TRANSFORM::AXIS(object, selected_axis, working_axis);

				Type obj_type = object->getType();
				bool is_valid_transformation = true;
				bool is_valid_type;
				bool is_group       = obj_type == GROUP_PARENT_DUMMY;
				bool is_only_this   = m_hierarchy_setting == ONLY_THIS;
				bool is_only_childs = m_hierarchy_setting == ONLY_TO_CHILDS;
				bool has_childs     = object->getChilds().size() > 0;
				is_valid_transformation = !((is_group && is_only_this) || (is_only_childs && !has_childs));

					
				// TRANSLATION //
				////std::cout << " - trs_i : " << trs_i << std::endl;
				if ( trs_i == 0 )
				{
					delta_3f = delta * working_axis[ind_j];
				}
				// ROTATION //
				else if ( trs_i == 1 )
				{
					if ( m_transformation_input_state )
					{
						if (!input_state_end)
						{
							is_valid_transformation = false;
						}
					}
				}
				// SCALE //
				else if ( trs_i == 2 )
				{
					if (obj_type == SAMPLER_OBJECT || obj_type == LIGHT_OBJECT) selected_axis = LOCAL;
					bool is_obj_scalable     = Utilities::is_object_type_SCALABLE(delta_3f, selected_axis, object);
					is_valid_transformation &= is_obj_scalable;
				}


				// pass transformation here //
				if     (is_Changed && is_valid_transformation)
				{
					if (m_transformation_input_state)
					{
						if (t_type == ROTATION)
						{

							optix::float3 new_angles = m_transformation_input_data.starting_plane_delta + delta_3f;

							if (selected_axis == WORLD)
							{

								//optix::Matrix4x4 new_Rot = MATRIX_TRANSFORM::Euler_Angles_To_Rotation_Matrix(new_angles);
								optix::Matrix4x4 Rx = optix::Matrix4x4::rotate(dot(new_angles, GLOBAL_AXIS[0]), GLOBAL_AXIS[0]);
								optix::Matrix4x4 Ry = optix::Matrix4x4::rotate(dot(new_angles, GLOBAL_AXIS[1]), GLOBAL_AXIS[1]);
								optix::Matrix4x4 Rz = optix::Matrix4x4::rotate(dot(new_angles, GLOBAL_AXIS[2]), GLOBAL_AXIS[2]);
								optix::Matrix4x4 R = Rz * Ry*Rx;
								optix::Matrix4x4 onb_inverse = object->getRotationMatrix_chain().inverse();
								optix::Matrix4x4 TRSmat = onb_inverse * R;
								object->setTransformation(TRSmat,
														  LOCAL,
														  t_type,
														  t_setting,
														  0,
														  working_axis);

								m_transformation_input_data.acc_mat = TRSmat;

							}
							else
							{
								optix::float3 ov_delta = new_angles * GLOBAL_AXIS[m_transformation_input_attr_index];
								object->setTransformation(ov_delta,
														  selected_axis,
														  t_type,
														  t_setting,
														  0,
														  working_axis);

								m_transformation_input_data.overall_delta = ov_delta;

							}
						}
						else
						{
							object->setTransformation(delta_3f,
													  selected_axis,
													  t_type,
													  t_setting,
													  0,
													  working_axis);

							m_transformation_input_data.overall_delta += delta_3f;
						}
					}
					else
					{
						object->setTransformation(delta_3f,
												  selected_axis,
												  t_type,
												  t_setting,
												  0,
												  working_axis);
						m_transformation_input_data.overall_delta += delta_3f;
					}

					//m_transformation_input_data.overall_delta += delta_3f;
				}
				else if( !is_valid_transformation )
				{
					delta_3f = ZERO_3f;
					delta    = 0.0f;
					m_transformation_input_data.delta_length = 0.0f;
					m_transformation_input_data.delta        = ZERO_3f;
				}


				// transformation input active
				if (m_transformation_input_state)
				{
					if ( input_state_end )
					{
						//save_ac     = 1;
						reset_state_1 = 1;
						input_state   = 1;
					}
				}
				// dragging state active
				else
				{
					if ( m0_REL )
					{
						if ( dragging_state_end )
						{
							//save_ac     = 1;
							reset_state_2 = 1;
							input_state   = 0;
						}
					}
				}
				
				reset_state = reset_state_1 || reset_state_2;
				if ( reset_state )
				{
					//std::cout << "\n ~ RESET_STATE: " << std::endl;
					

					if (m_transformation_input_state      // input state is 'active'
						&& t_type == ROTATION ) // Rotation is 'active'
					{
						// check before - after Euler Angles
						if (selected_axis == WORLD)
						{
							optix::float3 s_angles = m_transformation_input_data.starting_plane_delta;
							optix::float3 c_angles; Geometry::GENERAL::EulerAnglesFromMatrix(object->getRotationMatrix_chain(), c_angles);
							//if (c_angles.x == -180.0) c_angles.x = 0;
							//if (c_angles.y == -180.0) c_angles.y = 0;
							//if (c_angles.z == -180.0) c_angles.z = 0;
							optix::float3 delta = c_angles - s_angles;

							// action with rotation matrix
							if (fabsf(length(delta)) > 1e-5f)
							{
								//std::cout << " - saving action [ INPUT_WORLD ]!" << std::endl;
								Action * ac = new Action_TRANSFORMATION(object, 
																		m_transformation_input_data.acc_mat, 
																		working_axis, 
																		LOCAL, 
																		t_setting, 
																		ACTION_TRANSFORMATION_ROTATION_MAT);
								ActionManager::GetInstance().push(ac);
							}
						}
						// check if overall delta != 0
						else 
						{
							optix::float3 overall_delta = m_transformation_input_data.overall_delta;

							// action with normal delta3f
							if (fabsf(length(overall_delta)) > 1e-5f)
							{
								//std::cout << " - saving action [ INPUT ]!" << std::endl;

								Action * ac = new Action_TRANSFORMATION(object,
																		overall_delta,
																		working_axis,
																		selected_axis,
																		t_setting,
																		ACTION_TRANSFORMATION_ROTATION);
								ActionManager::GetInstance().push(ac);
							}
						}
					}
					else
					{
						optix::float3 overall_delta = m_transformation_input_data.overall_delta;
						float delta_length = length(overall_delta);
						save_ac = fabsf(delta_length) > 1e-5f ? 1 : 0;
						if (save_ac)
						{
							Action * ac = 0;
							ACTION_ID ac_id;
							if (t_type == TRANSLATION) { ac_id = ACTION_ID::ACTION_TRANSFORMATION_TRANSLATION; }
							if (t_type == ROTATION) { ac_id = ACTION_ID::ACTION_TRANSFORMATION_ROTATION; }
							if (t_type == SCALE) { ac_id = ACTION_ID::ACTION_TRANSFORMATION_SCALE; }

							ac = new Action_TRANSFORMATION(object, overall_delta, working_axis, selected_axis, t_setting, ac_id);
							ActionManager::GetInstance().push(ac);
						}
					}

					
					


					// reset data //
					{
					   // //std::cout << " >> __ resetting_data[0] __ " << std::endl;
						m_transformation_input_data.reset();
						m_transformation_input_i = -1;
						m_transformation_input_attr_index = -1;
						m_transformation_input_state = false;
						m_transformation_input_state_activating = false;
						m_plus_minus_button_state = false;
					}

				}

				
			}







		}
		ImGui::EndChild();

	}

	ImGui::PopFont();
}
void InterfaceManager::construct_Materials_child_Window()
{

	reset_Tabs_State(MATERIALS_TAB);
	process_TAB_SELECTION_STATE_change(MATERIALS_TAB);
	m_hierarchy_setting = TRANSFORMATION_SETTING::TO_ALL;


	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
	std::string label = "";
	SceneObject* object = m_selected_object_pack.raw_object;
	bool is_null = object == nullptr;
	if ( !is_null )
		is_null = object->getType() == GROUP_PARENT_DUMMY;
	if (!is_null)
		is_null = (object->getType() == FACE_VECTOR) || (object->getType() == LIGHT_OBJECT) || (object->getType() == SAMPLER_OBJECT);

	{
		
		
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		//window_flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
		ImGuiWindowFlags window_flags2 = 0;
		window_flags2 |= ImGuiWindowFlags_NoTitleBar;
		window_flags2 |= ImGuiWindowFlags_AlwaysAutoResize;
		//window_flags2 |= ImGuiWindowFlags_NoScrollbar;
		window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;

		// Materials - Child //
		guiStyleManager::setStyle("main_style_right_child2");
		ImGui::BeginChild("Materials_child##", ImVec2(ImGui::GetWindowWidth() - 4.0f, ImGui::GetWindowHeight() - 40.0f), true, window_flags2 );
		{

			
			bool  is_changed = false;
			std::string axis_labels[3] = { "R##", "G##", "B##" };
			GeometryProperties g_inst;

			std::vector<MaterialParameters> mat_params;
			std::vector<std::string> mat_names;
			if ( !is_null )
			{
				g_inst = object->getGeomProperties();

				for (std::string name : g_inst.mat_names)
					mat_names.push_back(name);

				for (int i = 0; i < object->getMaterialCount(); i++)
				{
					MaterialParameters mat;
					for (int j = 0; j < 3; j++)
					{
						mat.Kd[j] = object->getMaterialParams(i).Kd[j];
						mat.Ks[j] = object->getMaterialParams(i).Ks[j];
						mat.Kr[j] = object->getMaterialParams(i).Kr[j];
						mat.Ka[j] = object->getMaterialParams(i).Ka[j];
					}

					mat.phong_exp = object->getMaterialParams(i).phong_exp;
					////mat.name = mat_names[i];
					mat_params.push_back(mat);
				}

			}


			ImGui::Dummy(ImVec2(4.0f, 10.0f));
			// Materials list Child
			{
				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("Materials_list_child", ImVec2(ImGui::GetWindowWidth() - 8, 130.0f), true, 0 );
				//ImGui::Dummy(ImVec2(0, 0));

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::Text(" Materials");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0, 4.0f));

				// Inner List child
				{
					guiStyleManager::setStyle("main_style_right_child3");
					ImGui::BeginChild("Materials_child2", ImVec2(ImGui::GetWindowWidth() - 8, 95), true,  0 );
					ImGui::Dummy(ImVec2(0, 4.0f));
					bool selected = false;
					int index = 0;
					std::string mat_label = "";

					guiStyleManager::setIconFont(MDI, 14);
					for (std::string name : mat_names)
					{
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() - 50, 2.0f));
						ImGui::Dummy(ImVec2(2.0f, 2.0f));
						ImGui::SameLine();

						mat_label = std::string(" ") + ICON_MDI_SHARE_VARIANT + std::string("  ") + std::string(name) + std::string("##");
						selected = selected_material_index == index ? true : false;
						if (ImGui::Selectable(mat_label.c_str(), &selected))
						{
							selected_material_index = index;
						}

						index++;
					}

					ImGui::PopFont();
					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child_transformations");
				}

				ImGui::Dummy(ImVec2(2, 5));
				ImGui::EndChild();
			}

			guiStyleManager::setStyle("main_style_right_child2");
			ImGui::Dummy(ImVec2(2, 5));


			// A child Window for each Material Parameter now
			bool is_valid_mat = false;
			MaterialParameters mat = MaterialParameters();
			if (selected_material_index >= 0 && selected_material_index < mat_params.size())
			{
				mat = mat_params[selected_material_index];
				is_valid_mat = true;
			}
			else
				is_null = true;
			

			// Materials Editor Child Window
			guiStyleManager::setStyle("main_style_right_child3");
			ImGui::BeginChild("Materials_editor_child##", ImVec2(ImGui::GetWindowWidth() - 0.0f, ImGui::GetWindowHeight() - 170 ), true, window_flags );
			{

				ImVec2 materials_edit_size = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 10.0f);
				ImVec2 materials_edit_size_1f = ImVec2(ImGui::GetWindowWidth() - 12, 80.0f + 10.0f);


				ImGuiColorEditFlags colorEdit_flags = 0;
				colorEdit_flags |= ImGuiColorEditFlags_NoLabel;
				colorEdit_flags |= ImGuiColorEditFlags_NoInputs;
				
				// Diffuse
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("diffuse_window", materials_edit_size, true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::Text(" Diffuse ");

					ImGui::SameLine();
					ImGui::GetStyle().FrameRounding -= 3;
					ImGui::GetStyle().FramePadding -= ImVec2(1.0f, 1.0f);
					
					if (ImGui::ColorEdit3("##", &mat.Kd[0], colorEdit_flags))
					{
						is_changed = true;
						
					}
					//
					ImGui::GetStyle().FrameRounding += 3;
					ImGui::GetStyle().FramePadding += ImVec2(1.0f, 1.0f);
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					for (int j = 0; j < 3; j++)
					{
						ImGui::Dummy(ImVec2(0.0f, 0.0f));
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
						ImGui::SameLine();
						ImGui::PushItemWidth(100.0f);

						float old_v = mat.Kd[j];
						if (ImGui::DragFloat(axis_labels[j].c_str(), &mat.Kd[j], 0.001f, 0.0f, 1.0f))
						{
							m_dragFloat_state = true;
							is_changed        = true;
							if (m_repositioning_cursor) { mat.Kd[j] = old_v; }
						}
						

						ImGui::PopItemWidth();
					}
					ImGui::PopFont();

					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child2");
					//ImGui::Dummy(ImVec2(2, 0));
				}


//#define SPECULAR_MAT_USE
#ifdef SPECULAR_MAT_USE
				// Specular
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("specular_window##", materials_edit_size, true, 0 );

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::Text(" Specular ");
					ImGui::SameLine();
					ImGui::GetStyle().FrameRounding -= 3;
					ImGui::GetStyle().FramePadding -= ImVec2(1.0f, 1.0f);
					if (ImGui::ColorEdit3("##", &mat.Ks[0], colorEdit_flags))
						is_changed = true;
					ImGui::GetStyle().FrameRounding += 3;
					ImGui::GetStyle().FramePadding += ImVec2(1.0f, 1.0f);
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					for (int j = 0; j < 3; j++)
					{
						ImGui::Dummy(ImVec2(0.0f, 0.0f));
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
						ImGui::SameLine();
						ImGui::PushItemWidth(100.0f);
						if (ImGui::DragFloat(axis_labels[j].c_str(), &mat.Ks[j], 0.05f, -999999.0f, 9999999.0f))
						{
							is_changed = true;
						}
						ImGui::PopItemWidth();
					}
					ImGui::PopFont();

					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child2");
					//ImGui::Dummy(ImVec2(2, 5));

				}

#endif


				// Reflectivity
				{
					float refl[3] = { mat.Kr[0], mat.Kr[0], mat.Kr[0] };

					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("refl_window", materials_edit_size_1f, true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::Text(" Reflectivity ");
					ImGui::SameLine();
					ImGui::GetStyle().FrameRounding -= 3;
					ImGui::GetStyle().FramePadding -= ImVec2(1.0f, 1.0f);
					if (ImGui::ColorEdit3("##", &refl[0], colorEdit_flags))
						is_changed = true;
					ImGui::GetStyle().FrameRounding += 3;
					ImGui::GetStyle().FramePadding += ImVec2(1.0f, 1.0f);
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 24.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					float old_v = mat.Kr[0];
					if (ImGui::DragFloat("##Reflectivity", &mat.Kr[0], 0.001f, 0.0f, 1.0f))
					{
						m_dragFloat_state = true;
						if (mat.Kr[0] > 1.0f) mat.Kr[0] = 1.0f;
						is_changed = true;
					}
					if (m_repositioning_cursor) mat.Kr[0] = old_v;

					ImGui::PopItemWidth();

					ImGui::PopFont();

					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child2");
					//ImGui::Dummy(ImVec2(2, 5));
				}


				// Refraction
				{
					float refr[3] = { mat.Ka[0], mat.Ka[0], mat.Ka[0] };

					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("refr_window", materials_edit_size_1f, true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::Text(" Refraction ");
					ImGui::SameLine();
					ImGui::GetStyle().FrameRounding -= 3;
					ImGui::GetStyle().FramePadding -= ImVec2(1.0f, 1.0f);
					if (ImGui::ColorEdit3("##", &refr[0], colorEdit_flags))
						is_changed = true;
					ImGui::GetStyle().FrameRounding += 3;
					ImGui::GetStyle().FramePadding += ImVec2(1.0f, 1.0f);
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 24.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					//for (int j = 0; j < 3; j++)
					{
						ImGui::Dummy(ImVec2(0.0f, 0.0f));
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
						ImGui::SameLine();
						ImGui::PushItemWidth(100.0f);
						float old_v = mat.Ka[0];
						if (ImGui::DragFloat( "##Refraction", &mat.Ka[0], 0.001f, 1.0f, 10.0f))
						{
							m_dragFloat_state = true;
							is_changed = true;
							if (mat.Ka[0] > 10.0f) mat.Ka[0] = 10.0f;
						}
						if (m_repositioning_cursor) mat.Ka[0] = old_v;
						ImGui::PopItemWidth();
					}
					ImGui::PopFont();

					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child2");
					//ImGui::Dummy(ImVec2(2, 5));
				}


				// Phong Exp ( = Roughness )
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("phong_window", materials_edit_size_1f, true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::Text(" Roughness ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 24.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::Dummy(ImVec2(0.0f, 2.0f));
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					float old_v = mat.phong_exp;
					if (ImGui::DragFloat("##roughness", &mat.phong_exp, 0.001f, 0.0f, 9999999.0f))
					{
						m_dragFloat_state = true;
						is_changed = true;
					}
					if (m_repositioning_cursor) mat.phong_exp = old_v;
					ImGui::PopItemWidth();

					ImGui::PopFont();

					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child2");

				}


			}
			ImGui::EndChild();
			guiStyleManager::setStyle("main_style_right_child2");
			//ImGui::Dummy(ImVec2(2, 20));


			if ( is_changed && !is_null )
			{
				MaterialParameters m = object->getMaterialParams(selected_material_index);
				float Ks[3] = { mat.Ks[0], mat.Ks[1], mat.Ks[2] };
				for (int i = 0; i < 3; i++)
				{
					////std::cout << " Ks : [ " << Ks[0] << " , " << Ks[1] << " , " << Ks[2] << " ]" << std::endl;
					m.Kd[i] = mat.Kd[i];
					m.Kr[i] = mat.Kr[i];
					//m.Ks[i] = mat.Ks[i];
					m.Ks[i] = Ks[i];
					m.Ka[i] = mat.Ka[i];
				}

				m.phong_exp = mat.phong_exp;
				object->UpdateMaterial(m, selected_material_index);
				Mediator::restartAccumulation();
				Mediator::setSceneDirty();
				ViewportManager::GetInstance().markViewportsDirty();

			}


			guiStyleManager::setStyle("main_style_right_child2");
		}
		ImGui::EndChild();
	}

	ImGui::PopFont();
}
void InterfaceManager::construct_Object_Relations_Window()
{
	reset_Tabs_State(RELATIONS_TAB);
	process_TAB_SELECTION_STATE_change(RELATIONS_TAB);
	m_hierarchy_setting = TRANSFORMATION_SETTING::TO_ALL;

	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
	std::string label = "";
	SceneObject* object = m_selected_object_pack.raw_object;
	bool is_null = object == nullptr;
	if (!is_null)
	{
		is_null = (object->getType() == FACE_VECTOR || object->getType() == SAMPLER_OBJECT || !object->isActive() || object->isTemporary());
	}
	bool is_temp = is_null ? false : object->isTemporary();
	bool is_changed = false;

	std::string postfix_label = "";
	{

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		ImGuiWindowFlags window_flags2 = 0;
		window_flags2 |= ImGuiWindowFlags_NoTitleBar;
		window_flags2 |= ImGuiWindowFlags_AlwaysAutoResize;
		//window_flags2 |= ImGuiWindowFlags_NoScrollbar;
		window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;

		//
		guiStyleManager::setStyle("main_style_right_child2");
		ImGui::BeginChild("Materials_child##", ImVec2(ImGui::GetWindowWidth() - 4.0f, ImGui::GetWindowHeight() - 40.0f), true, window_flags);
		{
			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImVec2 materials_edit_size    = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 10.0f);
			ImVec2 materials_edit_size_1f = ImVec2(ImGui::GetWindowWidth() - 12, 80.0f + 10.0f);

			SceneObject* parent = 0;
			SceneObject* grp_parent = 0;
			int parent_id = -1;
			int grp_parent_id = -1;
			std::vector<SceneObject*> childs;
			std::vector<SceneObject*> grp_childs;
			std::vector<SceneObject*> fvs;
			std::vector<SceneObject*> samplers;

			std::vector<SceneObject*> fv_to_rem;
			std::vector<SceneObject*> smplr_to_rem;
			bool has_parent = false;
			bool has_grp_parent = false;

			std::string parent_name = "";
			std::string grp_parent_name = "";

			if (!is_null)
			{
				parent = object->getParent();
				grp_parent = object->getGroupParent();
				childs = object->getChilds();
				grp_childs = object->getGroupSelectionChilds_and_Groups();

				for (SceneObject* obj : object->get_Face_Vectors()) if (obj->isActive())fvs.push_back(obj);
				for (SceneObject* obj : object->get_Samplers()) if (obj->isActive())samplers.push_back(obj);

				if (parent != nullptr)
					has_parent = true;
				if (grp_parent != nullptr)
					has_grp_parent = true;

				parent_name     = parent     == 0 ? "" : parent->getName();
				grp_parent_name = grp_parent == 0 ? "" : grp_parent->getName();

				if (has_parent)
					parent_id = parent->getId();
				if (has_grp_parent)
					grp_parent_id = grp_parent->getId();
			}


			ImGui::Dummy(ImVec2(0.0f, 0.0f));
			
			// 
			guiStyleManager::setStyle("main_style_right_child3");
			ImGui::BeginChild("child_window_1##", ImVec2(ImGui::GetWindowWidth() - 10.0f, 182.0f), true, window_flags2);
			{

				
				// parent window
				{
					guiStyleManager::setStyle("main_style_right_child_transformations_parent");
					ImGui::BeginChild("parent_window##", materials_edit_size_1f, true, window_flags2);

					ImGui::Dummy(ImVec2(1.0f, 1.0f));

					ImGui::Dummy(ImVec2(1.0f, 0.0f));
					ImGui::SameLine();


					ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Text];
					ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);

					guiStyleManager::setIconFont(MDI, 16);
					ImGui::Text(ICON_MDI_ARRANGE_BRING_FORWARD);
					ImGui::PopFont();

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::SameLine();
					ImGui::Text(" Parent  ");
					ImGui::PopFont();
					ImGui::GetStyle().Colors[ImGuiCol_Text] = col;

					ImGui::Dummy(ImVec2(0, 18.0f));
					

					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 95.0f, 0.0f));
					ImGui::SameLine();

					ImGuiStyle& style = ImGui::GetStyle();
					style.FramePadding.x -= 2.0f;
					style.FramePadding.y -= 2.0f;
					style.FrameRounding  -= 2.0f;

					ImVec4 but = ImGui::GetStyle().Colors[ImGuiCol_Button];
					ImVec4 but_hov = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
					ImVec4 but_sel = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
					style.Colors[ImGuiCol_ButtonHovered] = but;
					style.Colors[ImGuiCol_ButtonActive]  = but;
					ImGui::PushItemWidth(100.0f);

					if (parent != nullptr && !is_null)
					{
						postfix_label = get_LabelIcon_by_Type(parent->getType());
						parent_name = postfix_label + std::string("  ") + parent_name + std::string("##");
					}
					ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.07f, 0.5f));
					guiStyleManager::setIconFont(MDI, 14);
					if (ImGui::Button(parent_name.c_str() , ImVec2(150.0f, 20.0f)))
					{
						
					}
					if (has_parent && ImGui::IsItemHovered() && !is_null)
					{
						int arr_index = Mediator::requestObjectsArrayIndex(parent_id);
						if (!m_reselecting && !m_skip_frame)
						{
							m_ui_focused = true;
							Mediator::setFocusedObject(arr_index);
							if (ImGui::IsMouseClicked(0))
							{
								Mediator::setSelectedObject(arr_index);
								m_reselecting = true;
								m_skip_frame = true;
							}
						}
					}
					ImGui::PopStyleVar();
					ImGui::PopFont();

					style.Colors[ImGuiCol_ButtonHovered] = but_hov;
					style.Colors[ImGuiCol_ButtonActive] = but_sel;

					guiStyleManager::setIconFont(MDI, 14);
					std::string remove_label = ICON_MDI_LINK_VARIANT_OFF + std::string("##");
					
					ImGui::SameLine();
					ImGui::Dummy(ImVec2(2.0f, 0.0f));
					ImGui::SameLine();
					style.FrameRounding += 1.0f;

					if (ImGui::Button(remove_label.c_str() , ImVec2(20.0f,20.0f)))
					{
						// unlink parent action
						if (!is_null && has_parent)
						{
							Action* ac = new Action_LINKING(m_selected_object_pack.raw_object, parent, ACTION_ID::ACTION_LINKING_UNLINK, true, true, true, false);
							ActionManager::GetInstance().push(ac);
							is_changed = true;
						}
					}

					style.FramePadding.x += 2.0f;
					style.FramePadding.y += 2.0f;
					style.FrameRounding  += 1.0f;

					ImGui::PopFont();
					ImGui::PopItemWidth();
					
					ImGui::PopFont();
					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child2");
				}

				ImGui::Dummy(ImVec2(0.0f, 0.0f));

				// grp_parent window
				{
					guiStyleManager::setStyle("main_style_right_child_transformations_parent");
					ImGui::BeginChild("grp_parent_window##", materials_edit_size_1f, true, window_flags2);

					ImGui::Dummy(ImVec2(1.0f, 1.0f));
					
					ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Text];
					ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);

					ImGui::Dummy(ImVec2(1.0f, 0.0f));
					ImGui::SameLine();
					guiStyleManager::setIconFont(MDI, 16);
					ImGui::Text(ICON_MDI_GROUP);
					ImGui::PopFont();

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::SameLine();
					ImGui::Text(" Group   ");
					ImGui::PopFont();
					ImGui::GetStyle().Colors[ImGuiCol_Text] = col;
					
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					

					ImGui::Dummy(ImVec2(0, 18.0f));



					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 95.0f, 0.0f));
					ImGui::SameLine();

					ImGuiStyle& style = ImGui::GetStyle();
					style.FramePadding.x -= 2.0f;
					style.FramePadding.y -= 2.0f;
					style.FrameRounding -= 2.0f;

					ImVec4 but = ImGui::GetStyle().Colors[ImGuiCol_Button];
					ImVec4 but_hov = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
					ImVec4 but_sel = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
					style.Colors[ImGuiCol_ButtonHovered] = but;
					style.Colors[ImGuiCol_ButtonActive]  = but;
					
					ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.07f, 0.5f));
					if (ImGui::Button(grp_parent_name.c_str(), ImVec2(150.0f, 20.0f)))
					{
						int arr_index = Mediator::requestObjectsArrayIndex(grp_parent_id);
						if (!m_reselecting && !m_skip_frame)
						{
							m_ui_focused = true;
							Mediator::setFocusedObject(arr_index);
							if (ImGui::IsMouseClicked(0))
							{
								Mediator::setSelectedObject(arr_index);
								m_reselecting = true;
								m_skip_frame = true;
							}
						}
					}
					if (has_grp_parent && ImGui::IsItemHovered() && !is_null)
					{
						int arr_index = Mediator::requestObjectsArrayIndex(grp_parent_id);
						Mediator::setFocusedObject(arr_index);
					}
					ImGui::PopStyleVar();

					style.Colors[ImGuiCol_ButtonHovered] = but_hov;
					style.Colors[ImGuiCol_ButtonActive]  = but_sel;

					guiStyleManager::setIconFont(MDI, 14);
					std::string remove_label = ICON_MDI_LINK_VARIANT_OFF + std::string("##");

					ImGui::SameLine();
					ImGui::Dummy(ImVec2(2.0f, 0.0f));
					ImGui::SameLine();
					style.FrameRounding += 1.0f;

					if (ImGui::Button(remove_label.c_str(), ImVec2(20.0f, 20.0f)))
					{
						// unlink grp_parent action
						if (!is_null && (has_grp_parent || has_parent))
						{
							Action* ac = new Action_LINKING(m_selected_object_pack.raw_object, parent, ACTION_ID::ACTION_LINKING_UNLINK, true, true, true, false);
							ActionManager::GetInstance().push(ac);
							is_changed = true;
						}
					}

					style.FramePadding.x += 2.0f;
					style.FramePadding.y += 2.0f;
					style.FrameRounding += 1.0f;

					ImGui::PopFont();
					

					ImGui::PopFont();
					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child2");
				}


				ImGui::EndChild();
			}
			guiStyleManager::setStyle("main_style_right_child2");
			



			ImGui::Dummy(ImVec2(0, 1.0f));
			//ImGui::Dummy(ImVec2(0, 3.0f));
			//ImGui::Dummy(ImVec2(0, 6.0f));

			std::vector<SceneObject*> childs_to_unlink;
			std::vector<SceneObject*> grp_childs_to_unlink;
			// childs list
			{
				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("childs_listt##", ImVec2(ImGui::GetWindowWidth() - 10, 163.0f), true, window_flags2);
				
				ImVec4 but = ImGui::GetStyle().Colors[ImGuiCol_Button];
				ImVec4 but_hov = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
				ImVec4 but_sel = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

				ImGui::Dummy(ImVec2(1.0f, 1.0f));

				ImGui::Dummy(ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				guiStyleManager::setIconFont(MDI, 16);
				ImGui::Text(ICON_MDI_SOURCE_BRANCH);
				ImGui::PopFont();

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::SameLine();
				ImGui::Text(" Childs:   ");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0, 4.0f));
				{
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					guiStyleManager::setStyle("main_style_right_child3_list");
					
					ImVec4 but_l     = ImGui::GetStyle().Colors[ImGuiCol_Button];
					ImVec4 but_hov_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
					ImVec4 but_sel_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
					
					ImGui::BeginChild("childs_child2", ImVec2(ImGui::GetWindowWidth() - 8, 125), true, 0);
					ImGui::Dummy(ImVec2(0, 10.0f));

					bool selected = false;
					for (SceneObject* obj : childs)
					{
						
						ImGuiStyle& style = ImGui::GetStyle();
						ImGui::Dummy(ImVec2(4.0f, 0.0f));
						ImGui::SameLine();
						
						std::string name_label = obj->getName();
						if ( !is_null )
						{
							postfix_label = get_LabelIcon_by_Type(obj->getType());
							name_label    = postfix_label + std::string("  ") + name_label + std::string("##") + std::to_string(obj->getId());
						}

						ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.07f, 0.5f));
						guiStyleManager::setIconFont(MDI, 14);
						if (ImGui::Button(name_label.c_str(), ImVec2(150.0f, 20.0f)))
						{
							
						}
						if (ImGui::IsItemHovered())
						{
							if (!m_reselecting && !m_skip_frame)
							{
								m_ui_focused = true;
								int arr_index = Mediator::requestObjectsArrayIndex(obj->getId());
								Mediator::setFocusedObject(arr_index);
								m_UI_focused_Relation = obj;
								if (ImGui::IsMouseClicked(0))
								{
									Mediator::setSelectedObject(arr_index);
									m_reselecting = true;
									m_skip_frame = true;
								}
							}
						}

						ImGui::PopStyleVar();
						ImGui::PopFont();

						style.Colors[ImGuiCol_Button] = but;
						style.Colors[ImGuiCol_ButtonHovered] = but_hov;
						style.Colors[ImGuiCol_ButtonActive] = but_sel;

						guiStyleManager::setIconFont(MDI, 14);
						std::string remove_label = ICON_MDI_LINK_VARIANT_OFF + std::string("##") + std::to_string(obj->getId());

						ImGui::SameLine();
						ImGui::Dummy(ImVec2(2.0f, 0.0f));
						ImGui::SameLine();
						style.FrameRounding += 2.0f;
						float bw = style.Colors[ImGuiCol_BorderShadow].w;
						style.Colors[ImGuiCol_BorderShadow].w = 0.0f;
						style.Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);
						if (ImGui::Button(remove_label.c_str(), ImVec2(20.0f, 20.0f)))
						{
							// unlink child
							if (!is_null)
							{
								childs_to_unlink.push_back(obj);
							}
						}
						style.Colors[ImGuiCol_Text] = ImVec4(0,0,0,1);
						style.Colors[ImGuiCol_BorderShadow].w = bw;
						style.FrameRounding -= 2.0f;
						style.Colors[ImGuiCol_Button] = but_l;
						style.Colors[ImGuiCol_ButtonHovered] = but_hov_l;
						style.Colors[ImGuiCol_ButtonActive] = but_sel_l;

						ImGui::PopFont();
						ImGui::Dummy(ImVec2(0.0f, 2.0f));

					}

					ImGui::PopFont();
					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child_transformations");
				}


				ImGui::Dummy(ImVec2(2, 5));
				ImGui::EndChild();
			}
			guiStyleManager::setStyle("main_style_right_child2");


			ImGui::Dummy(ImVec2(0, 0.0f));

			// group childs list
			{
				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("grp_childs_listt##", ImVec2(ImGui::GetWindowWidth() - 10, 163.0f), true, window_flags2);

				ImVec4 but = ImGui::GetStyle().Colors[ImGuiCol_Button];
				ImVec4 but_hov = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
				ImVec4 but_sel = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

				ImGui::Dummy(ImVec2(1.0f, 1.0f));

				ImGui::Dummy(ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				guiStyleManager::setIconFont(MDI, 16);
				ImGui::Text(ICON_MDI_SOURCE_REPOSITORY_MULTIPLE);
				ImGui::PopFont();

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::SameLine();
				ImGui::Text(" Group Childs:   ");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0, 4.0f));

				// Inner List childs
				{
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					guiStyleManager::setStyle("main_style_right_child3_list");

					ImVec4 but_l = ImGui::GetStyle().Colors[ImGuiCol_Button];
					ImVec4 but_hov_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
					ImVec4 but_sel_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

					ImGui::BeginChild("grp_childs_child2##", ImVec2(ImGui::GetWindowWidth() - 8, 125), true, 0);
					ImGui::Dummy(ImVec2(0, 4.0f));

					for( SceneObject* obj : grp_childs )
					{

						ImGuiStyle& style = ImGui::GetStyle();
						ImGui::Dummy(ImVec2(4.0f, 0.0f));
						ImGui::SameLine();

						std::string name_label = obj->getName();
						if (obj != nullptr && !is_null)
						{
							postfix_label = get_LabelIcon_by_Type(obj->getType());
							name_label = postfix_label + std::string("  ") + name_label + std::string("##") + std::to_string(obj->getId());
						}

						ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.07f, 0.5f));
						guiStyleManager::setIconFont(MDI, 14);
						if (ImGui::Button(name_label.c_str(), ImVec2(150.0f, 20.0f)))
						{
							
						}
						if (ImGui::IsItemHovered())
						{
							int arr_index = Mediator::requestObjectsArrayIndex(obj->getId());
							if (!m_reselecting && !m_skip_frame)
							{
								m_ui_focused = true;
								Mediator::setFocusedObject(arr_index);
								m_UI_focused_Relation = obj;
								if (ImGui::IsMouseClicked(0))
								{
									Mediator::setSelectedObject(arr_index);
									m_reselecting = true;
									m_skip_frame = true;
								}
							}
						}
						ImGui::PopStyleVar();
						ImGui::PopFont();

						style.Colors[ImGuiCol_Button] = but;
						style.Colors[ImGuiCol_ButtonHovered] = but_hov;
						style.Colors[ImGuiCol_ButtonActive] = but_sel;

						guiStyleManager::setIconFont(MDI, 14);
						std::string remove_label = ICON_MDI_LINK_VARIANT_OFF + std::string("##") + std::to_string(obj->getId());

						ImGui::SameLine();
						ImGui::Dummy(ImVec2(2.0f, 0.0f));
						ImGui::SameLine();
						style.FrameRounding += 2.0f;
						float bw = style.Colors[ImGuiCol_BorderShadow].w;
						style.Colors[ImGuiCol_BorderShadow].w = 0.0f;
						style.Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);
						if (ImGui::Button(remove_label.c_str(), ImVec2(20.0f, 20.0f)))
						{
							// unlink child
							if (!is_null)
							{
								grp_childs_to_unlink.push_back(obj);
							}
						}
						style.Colors[ImGuiCol_Text] = ImVec4(0, 0, 0, 1);
						style.Colors[ImGuiCol_BorderShadow].w = bw;
						style.FrameRounding -= 2.0f;
						style.Colors[ImGuiCol_Button] = but_l;
						style.Colors[ImGuiCol_ButtonHovered] = but_hov_l;
						style.Colors[ImGuiCol_ButtonActive] = but_sel_l;

						ImGui::PopFont();
						ImGui::Dummy(ImVec2(0.0f, 2.0f));

					}

					ImGui::PopFont();
					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child_transformations");
				}

				// DO THE UNLINKS //
				for (SceneObject* obj : childs_to_unlink)
				{
					Action* ac = new Action_LINKING(obj, obj->getParent(), ACTION_ID::ACTION_LINKING_UNLINK, true, true, true, false);
					ActionManager::GetInstance().push(ac);
					is_changed = true;
				}
				for (SceneObject* obj : grp_childs_to_unlink)
				{
					Action* ac = new Action_LINKING(obj, obj->getParent(), ACTION_ID::ACTION_LINKING_UNLINK, true, true, true, false);
					ActionManager::GetInstance().push(ac);
					is_changed = true;
				}
				//

				ImGui::Dummy(ImVec2(2, 5));
				ImGui::EndChild();
			}
			guiStyleManager::setStyle("main_style_right_child2");


			// fv list
			{
				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("facevectors_listt##", ImVec2(ImGui::GetWindowWidth() - 10, 163.0f), true, window_flags2);

				ImVec4 but = ImGui::GetStyle().Colors[ImGuiCol_Button];
				ImVec4 but_hov = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
				ImVec4 but_sel = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

				ImGui::Dummy(ImVec2(1.0f, 1.0f));

				ImGui::Dummy(ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				guiStyleManager::setIconFont(MDI, 16);
				ImGui::Text(get_LabelIcon_by_Type(FACE_VECTOR).c_str());
				ImGui::PopFont();

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::SameLine();
				ImGui::Text(" Attached Facevectors:");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0, 4.0f));
				{
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					guiStyleManager::setStyle("main_style_right_child3_list");

					ImVec4 but_l = ImGui::GetStyle().Colors[ImGuiCol_Button];
					ImVec4 but_hov_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
					ImVec4 but_sel_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

					ImGui::BeginChild("childs_child2asdsa_fv", ImVec2(ImGui::GetWindowWidth() - 8, 125), true, 0);
					ImGui::Dummy(ImVec2(0, 10.0f));

					bool selected = false;
					for (SceneObject* obj : fvs)
					{

						ImGuiStyle& style = ImGui::GetStyle();
						ImGui::Dummy(ImVec2(4.0f, 0.0f));
						ImGui::SameLine();

						std::string name_label = obj->getName();
						if (!is_null)
						{
							postfix_label = get_LabelIcon_by_Type(obj->getType());
							name_label = postfix_label + std::string("  ") + name_label + std::string("##") + std::to_string(obj->getId());
						}

						ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.07f, 0.5f));
						guiStyleManager::setIconFont(MDI, 14);
						if (ImGui::Button(name_label.c_str(), ImVec2(150.0f, 20.0f)))
						{

						}
						if (ImGui::IsItemHovered())
						{
							if (!m_reselecting && !m_skip_frame)
							{
								m_ui_focused = true;
								int arr_index = Mediator::requestObjectsArrayIndex(obj->getId());
								Mediator::setFocusedObject(arr_index);
								m_UI_focused_Relation = obj;
								if (ImGui::IsMouseClicked(0))
								{
									Mediator::setSelectedObject(arr_index);
									m_reselecting = true;
									m_skip_frame = true;
								}
							}
						}

						ImGui::PopStyleVar();
						ImGui::PopFont();

						style.Colors[ImGuiCol_Button] = but;
						style.Colors[ImGuiCol_ButtonHovered] = but_hov;
						style.Colors[ImGuiCol_ButtonActive] = but_sel;

						guiStyleManager::setIconFont(MDI, 14);
						std::string remove_label = ICON_MDI_LINK_VARIANT_OFF + std::string("##") + std::to_string(obj->getId());

						ImGui::SameLine();
						ImGui::Dummy(ImVec2(2.0f, 0.0f));
						ImGui::SameLine();
						style.FrameRounding += 2.0f;
						float bw = style.Colors[ImGuiCol_BorderShadow].w;
						style.Colors[ImGuiCol_BorderShadow].w = 0.0f;
						style.Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);
						if (ImGui::Button(remove_label.c_str(), ImVec2(20.0f, 20.0f)))
						{
							// unlink child
							if (!is_null)
							{
								//childs_to_unlink.push_back(obj);
								if(Utilities::is_object_type_Muttable_To_Delete(obj))
									fv_to_rem.push_back(obj);
							}
						}
						style.Colors[ImGuiCol_Text] = ImVec4(0, 0, 0, 1);
						style.Colors[ImGuiCol_BorderShadow].w = bw;
						style.FrameRounding -= 2.0f;
						style.Colors[ImGuiCol_Button] = but_l;
						style.Colors[ImGuiCol_ButtonHovered] = but_hov_l;
						style.Colors[ImGuiCol_ButtonActive] = but_sel_l;

						ImGui::PopFont();
						ImGui::Dummy(ImVec2(0.0f, 2.0f));

					}

					ImGui::PopFont();
					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child_transformations");
				}


				ImGui::Dummy(ImVec2(2, 5));
				ImGui::EndChild();
			}
			guiStyleManager::setStyle("main_style_right_child2");


			// smplr list
			{
				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("samplers_listt##", ImVec2(ImGui::GetWindowWidth() - 10, 163.0f), true, window_flags2);

				ImVec4 but = ImGui::GetStyle().Colors[ImGuiCol_Button];
				ImVec4 but_hov = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
				ImVec4 but_sel = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

				ImGui::Dummy(ImVec2(1.0f, 1.0f));

				ImGui::Dummy(ImVec2(1.0f, 0.0f));
				ImGui::SameLine();
				guiStyleManager::setIconFont(MDI, 16);
				ImGui::Text(get_LabelIcon_by_Type(SAMPLER_OBJECT).c_str());
				ImGui::PopFont();

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::SameLine();
				ImGui::Text(" Attached Samplers:");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0, 4.0f));
				{
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					guiStyleManager::setStyle("main_style_right_child3_list");

					ImVec4 but_l = ImGui::GetStyle().Colors[ImGuiCol_Button];
					ImVec4 but_hov_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
					ImVec4 but_sel_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

					ImGui::BeginChild("childs_child2asdsa_smplrs", ImVec2(ImGui::GetWindowWidth() - 8, 125), true, 0);
					ImGui::Dummy(ImVec2(0, 10.0f));

					bool selected = false;
					for (SceneObject* obj : samplers)
					{

						ImGuiStyle& style = ImGui::GetStyle();
						ImGui::Dummy(ImVec2(4.0f, 0.0f));
						ImGui::SameLine();

						std::string name_label = obj->getName();
						if (!is_null)
						{
							postfix_label = get_LabelIcon_by_Type(obj->getType());
							name_label = postfix_label + std::string("  ") + name_label + std::string("##") + std::to_string(obj->getId());
						}

						ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.07f, 0.5f));
						guiStyleManager::setIconFont(MDI, 14);
						if (ImGui::Button(name_label.c_str(), ImVec2(150.0f, 20.0f)))
						{

						}
						if (ImGui::IsItemHovered())
						{
							if (!m_reselecting && !m_skip_frame)
							{
								m_ui_focused  = true;
								m_ui_focused_object_id = obj->getId();
								int arr_index = Mediator::requestObjectsArrayIndex(obj->getId());
								Mediator::setFocusedObject(arr_index);
								m_UI_focused_Relation = obj;
								if (ImGui::IsMouseClicked(0))
								{
									Mediator::setSelectedObject(arr_index);
									m_reselecting = true;
									m_skip_frame = true;
								}
							}
						}

						ImGui::PopStyleVar();
						ImGui::PopFont();

						style.Colors[ImGuiCol_Button] = but;
						style.Colors[ImGuiCol_ButtonHovered] = but_hov;
						style.Colors[ImGuiCol_ButtonActive] = but_sel;

						guiStyleManager::setIconFont(MDI, 14);
						std::string remove_label = ICON_MDI_LINK_VARIANT_OFF + std::string("##") + std::to_string(obj->getId());

						ImGui::SameLine();
						ImGui::Dummy(ImVec2(2.0f, 0.0f));
						ImGui::SameLine();
						style.FrameRounding += 2.0f;
						float bw = style.Colors[ImGuiCol_BorderShadow].w;
						style.Colors[ImGuiCol_BorderShadow].w = 0.0f;
						style.Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);
						if (ImGui::Button(remove_label.c_str(), ImVec2(20.0f, 20.0f)))
						{
							// unlink child
							if (!is_null)
							{
								//childs_to_unlink.push_back(obj);
								smplr_to_rem.push_back(obj);
							}
						}
						style.Colors[ImGuiCol_Text] = ImVec4(0, 0, 0, 1);
						style.Colors[ImGuiCol_BorderShadow].w = bw;
						style.FrameRounding -= 2.0f;
						style.Colors[ImGuiCol_Button] = but_l;
						style.Colors[ImGuiCol_ButtonHovered] = but_hov_l;
						style.Colors[ImGuiCol_ButtonActive] = but_sel_l;

						ImGui::PopFont();
						ImGui::Dummy(ImVec2(0.0f, 2.0f));

					}

					ImGui::PopFont();
					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child_transformations");
				}


				ImGui::Dummy(ImVec2(2, 5));
				ImGui::EndChild();
			}
			guiStyleManager::setStyle("main_style_right_child2");


			for (SceneObject* obj : fv_to_rem)
			{
				Action* action = new Action_CRT_DLT_FACE_VECTOR(obj, 0, 0);
				ActionManager::GetInstance().push(action);
				//else if (is_sampler)     action = new Action_CRT_DLT_SAMPLER(m_selected_object_pack.raw_object, 0);
			}
			for (SceneObject* obj : smplr_to_rem)
			{
				Action*action = new Action_CRT_DLT_SAMPLER(obj, 0, 0);
				ActionManager::GetInstance().push(action);
			}
		}
		ImGui::EndChild();
	}
	ImGui::PopFont();


	if (is_changed && !is_null)
	{
		bool should_unselect = object->isActive() ? false : true;
		if ( should_unselect )
		{
			resetSelectedObject(false, true);
			//Action* ac = new Action_Selection()
		}
	}


}
void InterfaceManager::construct_Object_Restrictions_Window()
{
	//std::cout << "\n - construct_Object_Restrictions_Window():" << std::endl;

	reset_Tabs_State(RESTRICTIONS_TAB);
	process_TAB_SELECTION_STATE_change(RESTRICTIONS_TAB);
	m_hierarchy_setting = TRANSFORMATION_SETTING::TO_ALL;


	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
	std::string label = "";
	SceneObject* object = m_selected_object_pack.raw_object;
	bool is_null = object == nullptr;
	bool is_facevector = is_null ? false : object->getType() == FACE_VECTOR; //|| object->getType() == SAMPLER_OBJECT;
	object = is_facevector ? object->getParent() : object;
	bool is_object_null = object == nullptr;
	bool is_temp     = is_null ? false : object->isTemporary();
	int  selected_id = is_null ? -1 : object->getId();
	bool is_any_Restriction_focused = false;
	bool is_R_list_hovered = false;
	bool is_selected_R_hovered = false;

	bool mouse_down = false;
	bool mouse_down_on_R = false;

	bool is_changed = false;
	Restriction* R_active  = nullptr;
	Restriction* R_changed = nullptr;

	bool m0_down = false;
	bool m0_down_on_activated_item = false;
	bool focused_active_item       = false;

	m_transformation_input_state_activating = false;

	int index_i = -1;
	int index_j = -1;
	ImGuiID ID;
	float delta = 0.0f;
	bool was_INPUT_active = false;
	bool is_INPUT_active  = false;


	std::string postfix_label = "";
	{

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

		ImGuiWindowFlags window_flags2 = 0;
		window_flags2 |= ImGuiWindowFlags_NoTitleBar;
		window_flags2 |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags2 |= ImGuiWindowFlags_NoScrollbar;
		//window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;

		ImGuiWindowFlags window_flags3 = 0;
		window_flags3 |= ImGuiWindowFlags_NoTitleBar;
		window_flags3 |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags3 |= ImGuiWindowFlags_NoScrollbar;
		window_flags3 |= ImGuiWindowFlags_NoScrollWithMouse;

		ImGuiWindowFlags window_flags4 = 0;
		window_flags4 |= ImGuiWindowFlags_NoTitleBar;
		window_flags4 |= ImGuiWindowFlags_AlwaysAutoResize;
		//window_flags4 |= ImGuiWindowFlags_NoScrollWithMouse;
		window_flags4 |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
		

		ImGuiWindowFlags child_flags = 0;
		child_flags |= ImGuiWindowFlags_NoScrollWithMouse;

		ImGuiWindowFlags child_flags2 = 0;
		child_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;
		child_flags2 |= ImGuiWindowFlags_AlwaysVerticalScrollbar;

		//
		guiStyleManager::setStyle("style_restrictions_panel");
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4 text_col = style.Colors[ImGuiCol_Text];
		ImVec4 text_not_approved_col = style.Colors[ImGuiCol_CheckMark];
		ImVec4 text_d_col = style.Colors[ImGuiCol_TextDisabled];
		ImVec4 frm_hov = style.Colors[ImGuiCol_FrameBgHovered];
		ImVec4 frm_act = style.Colors[ImGuiCol_FrameBgActive];
		ImVec4 ch_col = style.Colors[ImGuiCol_ChildWindowBg];
		ImVec4 bord_col = style.Colors[ImGuiCol_Border];
		ImVec4 but = style.Colors[ImGuiCol_Button];
		ImVec4 but_hov = style.Colors[ImGuiCol_ButtonHovered];
		ImVec4 but_act = style.Colors[ImGuiCol_ButtonActive];
		float b_alpha = style.Colors[ImGuiCol_Border].w;

		ImVec4 col_is_valid     = style.Colors[ImGuiCol_Tab];
		ImVec4 col_isnt_valid   = style.Colors[ImGuiCol_TabHovered];
		ImVec4 col_is_disabled  = style.Colors[ImGuiCol_TabActive];
		ImVec4 frm_hov_disabled = style.Colors[ImGuiCol_TabUnfocused];
		ImVec4 frm_act_disabled = style.Colors[ImGuiCol_TabUnfocusedActive];
		//

		guiStyleManager::setStyle("main_style_right_child2");
		ImGui::BeginChild("Restrictions_Child##", ImVec2(ImGui::GetWindowWidth() - 4.0f, ImGui::GetWindowHeight() - 20.0f), true, window_flags);
		{
			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImVec2 materials_edit_size    = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 10.0f);
			ImVec2 materials_edit_size_1f = ImVec2(ImGui::GetWindowWidth() - 12, 80.0f + 10.0f );

			std::vector<Restriction*> scene_Restrictions = Mediator::Request_Scene_Restrictions();
			std::vector<Restriction*> object_Restrictions;
			if (!is_null || is_object_null)
			{
				for (Restriction* R : scene_Restrictions)
				{
					if (!R->is_Active())
						continue;

					Restriction_ID res_id = R->getId();
					SceneObject* oA  = R->get_Owner(0);
					SceneObject* oB  = R->get_Owner(1);
					SceneObject* sub = R->getSubject();
					SceneObject* obj = R->getObject();

					int oA_id  = oA->getId();
					int oB_id  = oB->getId();
					int obj_id = obj->getId();
					int sub_id = sub->getId();
					SceneObject* obj_parent = obj->getParent();
					SceneObject* sub_parent = sub->getParent();

					int obj_parent_id = obj_parent == nullptr ? -1 : obj_parent->getId();
					int sub_parent_id = sub_parent == nullptr ? -1 : sub_parent->getId();

					bool r_is_valid = false;
					if      (res_id == FOCUS_R)
					{
						if (sub_parent_id == selected_id || obj_id == selected_id)
							r_is_valid = true;

						if (!is_null)
						{
							r_is_valid |= Utilities::is_object_in_List(m_selected_object_pack.raw_object, oA->get_Face_Vectors());
						}
					}
					else if (res_id == CONVERSATION_R)
					{
						if (sub_parent_id == selected_id || obj_parent_id == selected_id)
							r_is_valid = true;

						if (!is_null && !r_is_valid)
						{
							r_is_valid |= Utilities::is_object_in_List(m_selected_object_pack.raw_object, oA->get_Face_Vectors());
							r_is_valid |= Utilities::is_object_in_List(m_selected_object_pack.raw_object, oB->get_Face_Vectors());
						}
					}
					else if (res_id == DISTANCE_R)
					{
						if (obj_id == selected_id || sub_id == selected_id)
							r_is_valid = true;
					}

					if (r_is_valid || is_object_null)
					{
						object_Restrictions.push_back(R);
					}
				}

			}


			

			ImGui::Dummy(ImVec2(0.0f, 0.0f));
			// Restrictions List //
			{
				float R_focus_window_size        = 129.0f;
				float R_conversation_window_size = 152.0f + 2.0f;
				float R_distance_window_size     = 106.0f;

				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("Restrictions_List##", ImVec2(ImGui::GetWindowWidth() - 10, 400.0f + 6 - 100.0f ), true, window_flags2);

				ImVec4 but     = ImGui::GetStyle().Colors[ImGuiCol_Button];
				ImVec4 but_hov = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
				ImVec4 but_sel = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

				ImGui::Dummy(ImVec2(1.0f, 1.0f));

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::SameLine();
				ImGui::Text("Restrictions   ");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0, 1.0f));
				{
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					guiStyleManager::setStyle("style_restrictions_list");

					
					ImVec4 but_l = ImGui::GetStyle().Colors[ImGuiCol_Button];
					ImVec4 but_hov_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
					ImVec4 but_sel_l = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

					ImGui::BeginChild("child_listtt##", ImVec2(ImGui::GetWindowWidth() - 8, 260 + 20 ), true, window_flags2);
					ImGui::Dummy(ImVec2(0, 2.0f));


					if (ImGui::IsMouseHoveringWindow())
						is_R_list_hovered = true;

					bool selected = false;
					for (Restriction* R : object_Restrictions)
					{

						ImGui::Dummy(ImVec2(4.0f, 0.0f));

						Restriction_ID res_id = R->getId();
						int Id = R->get_Unique_Id();

						optix::float2 error = optix::make_float2(0.0f);
						bool is_R_approved = R->is_Valid( &error, true );
						std::string res_type;
						std::string sub_name = R->getSubject()->getName();
						std::string obj_name = R->getObject()->getName();
						std::string oA_name = R->get_Owner(0)->getName();
						std::string oB_name = R->get_Owner(1)->getName();
						std::string icon_label = "";

						Type A_type = R->get_Owner(0)->getType();
						Type B_type = R->get_Owner(1)->getType();
						Type a_type = R->getSubject()->getType();
						Type b_type = R->getObject()->getType();

						int  A_array_index = Mediator::requestObjectsArrayIndex(R->get_Owner(0)->getId());
						int  B_array_index = Mediator::requestObjectsArrayIndex(R->get_Owner(1)->getId());
						int  a_array_index = Mediator::requestObjectsArrayIndex(R->getSubject()->getId());
						int  b_array_index = Mediator::requestObjectsArrayIndex(R->getObject()->getId() );

						float  window_height = 0;
						ImVec2 button_offset = ImVec2(0.0f,0.0f);
						GLuint * button_icon = 0;
						if      (res_id == FOCUS_R)
						{
							button_icon   = AssetManager::GetInstance().GetTexture("restriction_focus_button");
							res_type      = "Focus";
							icon_label    = ICON_MDI_ARROW_RIGHT;
							button_offset = ImVec2(130.0f + 5.0f, 0.0f); //ImVec2(170.0f, 0.0f);
							window_height = R->is_GUI_Open() ? R_focus_window_size : 80.0f;
						}
						else if (res_id == CONVERSATION_R)
						{
							button_icon = AssetManager::GetInstance().GetTexture("restriction_conversation_button");
							res_type = "Conversation";
							icon_label = ICON_MDI_ARROW_RIGHT +std::string("") + ICON_MDI_ARROW_LEFT;
							button_offset = ImVec2(90.0f + 6.0f, 0.0f);
							window_height = R->is_GUI_Open() ? R_conversation_window_size : 80.0f;
						}
						else if (res_id == DISTANCE_R)
						{
							button_icon = AssetManager::GetInstance().GetTexture("restriction_distance_button");
							res_type = "Distance";
							icon_label = ICON_MDI_ARROW_EXPAND_HORIZONTAL;
							button_offset = ImVec2(120.0f, 0.0f);
							window_height = R->is_GUI_Open() ? R_distance_window_size : 80.0f;
						}
						
						std::string child_label = "sub_child" + std::string("##") + std::to_string(Id);
						guiStyleManager::setStyle("style_restrictions_panel");
	
						ImGui::Dummy(ImVec2(2.0f, 0.0f));
						ImGui::SameLine();

						ImVec4 ch_bg_id = ch_col;
						ImVec4 brd_col_id = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
						ImVec4 icon_brd_col_id;
						if (is_R_approved)
						{
							//ch_bg_id = col_is_valid;
							icon_brd_col_id = col_is_valid;
						}
						else
						{
							//ch_bg_id = col_isnt_valid;
							icon_brd_col_id = col_isnt_valid;
						}
						if (!R->isEnabled())
						{
							ch_bg_id = col_is_disabled;
						}

						if      (R->is_Focused() && !R->is_Selected())
						{
							ch_bg_id = R->isEnabled()? frm_hov : frm_hov_disabled;
						}
						else if (R->is_Selected())
						{
							ch_bg_id = R->isEnabled()? frm_act : frm_act_disabled;
							brd_col_id = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
						}
						else
						{
							ch_bg_id = ch_bg_id;
							brd_col_id = ImVec4(43.0f / 255.0f, 43.0f / 255.0f, 43.0f / 255.0f, 1.0f);
						}

						style.Colors[ImGuiCol_ChildBg] = ch_bg_id;
						style.Colors[ImGuiCol_Border]  = brd_col_id;

						
						ImGui::BeginChild(child_label.c_str(), ImVec2(ImGui::GetWindowWidth() - 6.0f , window_height ), true, child_flags);
						{
							m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();
							style.Colors[ImGuiCol_Border] = bord_col;
							if (ImGui::IsMouseHoveringWindow())
							{
								bool only_focus = true;
								m_UI_hovered_Restriction_id = Id;
								
								if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
								{
									only_focus = false;
									Mediator::Reset_Restrictions_UI_States();
									R->set_Focused( true );
									R->set_Selected( true );
									m_UI_selected_Restriction_id = Id;
									m_UI_selected_Restriction = R;
								}
								else
								{
									Mediator::Reset_Restrictions_UI_States(true);
									R->set_Focused(true);
								}

								is_any_Restriction_focused = true;
								if ( R->is_Selected() )
									is_selected_R_hovered = true;

							}

							// type Label //
							if ( R->isEnabled() && !is_R_approved )
								style.Colors[ImGuiCol_Text] = text_not_approved_col;
							else
								style.Colors[ImGuiCol_Text] = R->isEnabled() ? text_col : text_d_col;

							guiStyleManager::setIconFont(MDI, 14);
							
							// Type_ImageButton //
							{
								style.Colors[ImGuiCol_Button].w = 0.0f;
								style.Colors[ImGuiCol_ButtonHovered].w = 0.0f;
								style.Colors[ImGuiCol_ButtonActive].w = 0.0f;
								style.Colors[ImGuiCol_Border] = icon_brd_col_id;//ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

								ImGui::Dummy(ImVec2(0.0f, 1.0f));
								ImGui::ImageButton((void*)*button_icon, ImVec2(24.0f,24.0f) , ImVec2(0.0f, 0.0f), BUTTON_RATIO , 1.0f , ch_bg_id);

								style.Colors[ImGuiCol_Border] = bord_col;
								style.Colors[ImGuiCol_Button].w = 1.0f;
								style.Colors[ImGuiCol_ButtonHovered].w = 1.0f;
								style.Colors[ImGuiCol_ButtonActive].w = 1.0f;

								ImGui::SameLine();
								ImGui::Text("  ");
								ImGui::SameLine();
								ImGui::Text(res_type.c_str());
								ImGui::SameLine();
								ImGui::Text(":");
							}


							// toggle auto update //
							{

								ImVec4 ch_bg = style.Colors[ImGuiCol_ChildBg];
								style.Colors[ImGuiCol_Button] = ch_bg;
								style.Colors[ImGuiCol_ButtonHovered] = ch_bg;
								style.Colors[ImGuiCol_ButtonActive] = ch_bg;
								style.Colors[ImGuiCol_Border].w = 0.0f;
								ImVec2 pad_off = ImVec2(2.0f, 2.0f);
								style.FramePadding += pad_off;

								ImGui::SameLine();
								ImGui::Dummy(button_offset - ImVec2(30,0.0f));
								std::string toggle_but_label = R->isAutoUpdate() ? ICON_MDI_SYNC : ICON_MDI_SYNC_OFF;
								toggle_but_label += std::string("##") + std::to_string(Id);
								ImGui::SameLine();
								style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
								guiStyleManager::setIconFont(MDI, 16);
								if (ImGui::Button(toggle_but_label.c_str()))
									R->toggle_AutoUpdate();
								ImGui::PopFont();

								style.FramePadding -= pad_off;
								style.Colors[ImGuiCol_Border].w = b_alpha;
								style.Colors[ImGuiCol_Button] = but;
								style.Colors[ImGuiCol_ButtonHovered] = but_hov;
								style.Colors[ImGuiCol_ButtonActive] = but_act;
							}


							// toggle button //
							{
								ImGui::SameLine();

								ImVec4 ch_bg = style.Colors[ImGuiCol_ChildBg];
								style.Colors[ImGuiCol_Button] = ch_bg;
								style.Colors[ImGuiCol_ButtonHovered] = ch_bg;
								style.Colors[ImGuiCol_ButtonActive] = ch_bg;
								style.Colors[ImGuiCol_Border].w = 0.0f;
								ImVec2 pad_off = ImVec2(2.0f, 2.0f);
								style.FramePadding += pad_off;

								ImGui::SameLine();
								ImGui::Dummy(ImVec2(8.0f, 0)); //button_offset);
								std::string toggle_but_label = R->isEnabled() ? ICON_MDI_TOGGLE_SWITCH : ICON_MDI_TOGGLE_SWITCH_OFF;
								toggle_but_label += std::string("##") + std::to_string(Id);
								ImGui::SameLine();
								style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
								guiStyleManager::setIconFont(MDI, 16);
								if (ImGui::Button(toggle_but_label.c_str()))
									R->toggle_State();
								ImGui::PopFont();

								style.FramePadding -= pad_off;
								style.Colors[ImGuiCol_Border].w = b_alpha;
								style.Colors[ImGuiCol_Button] = but;
								style.Colors[ImGuiCol_ButtonHovered] = but_hov;
								style.Colors[ImGuiCol_ButtonActive] = but_act;
							}

							ImGui::Dummy(ImVec2(0.0f, 0.0f));
							style.Colors[ImGuiCol_Text] = ImVec4(1.0f,1.0f,1.0f,1.0f);

							{

								std::string from_A, from_a, to_B, to_b;
								from_A = std::string("") + get_IconLabel(A_type) + oA_name;
								from_a = get_IconLabel(a_type) + sub_name;
								to_B   = get_IconLabel(B_type) + oB_name;
								to_b   = get_IconLabel(b_type) + obj_name;

								if      (res_id == FOCUS_R)
								{
									ImGuiTreeNodeFlags node_flags = 0;
									//node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
									node_flags |= ImGuiTreeNodeFlags_FramePadding;
									node_flags |= ImGuiTreeNodeFlags_Framed;
									node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;
									
								
									ImGui::Dummy(ImVec2(20.0f, 0.0f));
									ImGui::SameLine();
									ImGui::Text("From:");
									ImGui::SameLine();
									ImGui::Dummy(ImVec2(100.0f,0.0f));
									ImGui::SameLine();
									ImGui::Text("To:");

									std::string node_label = std::string("##") + std::to_string(R->get_Unique_Id());
									ImGuiID ID = ImGui::GetCurrentWindow()->GetID(node_label.c_str());
									bool is_gui_open = ImGui::TreeNodeBehaviorIsOpen(ID, node_flags);
									R->set_GUI_Open(is_gui_open);
									if (ImGui::TreeNodeEx_2( node_label.c_str(), 17.0f, node_flags ) )
									{
										style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_Header];
										style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_HeaderHovered];
										style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_HeaderActive];
										ImGui::SameLine();
										ImGui::Dummy(ImVec2(0.0f, 0.0f));
										ImGui::SameLine();
										ImGui::Button(from_A.c_str(), ImVec2(90.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											m_ui_focused = true;
											Mediator::setFocusedObject(A_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(A_array_index);
											}
										}
										style.Colors[ImGuiCol_Button] = but;
										style.Colors[ImGuiCol_ButtonHovered] = but_hov;
										style.Colors[ImGuiCol_ButtonActive] = but_act;


										ImGui::Text("  |__");
										ImGui::SameLine();
										ImGui::Button(from_a.c_str(), ImVec2(85.0f,20.0f));
										if (ImGui::IsItemHovered())
										{
											Mediator::setFocusedObject(a_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(a_array_index);
											}
										}
										ImGui::SameLine();
										guiStyleManager::setIconFont(MDI, 12);
										ImGui::Text(icon_label.c_str());
										ImGui::PopFont();
										ImGui::SameLine();
										ImGui::Button(to_b.c_str(), ImVec2(85.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											m_ui_focused = true;
											Mediator::setFocusedObject(b_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(b_array_index);
											}
										}

#ifdef edit_res_tol_var_in_menu

										ImGui::Text("  |__");	
										{
											float prev_value = R->get_DeltaLimit().x;
											float v          = prev_value;
											index_i = 0;
											index_j = -1;

											std::string label = std::string("##") + std::to_string(R->get_Unique_Id());
											ImGui::SameLine();
											ImGui::PushItemWidth(85.0f);

											ImGui::DragFloat(label.c_str(), &v, 0.1f, 0.0f, 89.9f);
											ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
											was_INPUT_active = m_transformation_input_state;
											is_INPUT_active  = ImGui::TempInputTextIsActive(ID);
											if (m_repositioning_cursor) v = prev_value;
											delta = v - prev_value;
											if (ImGui::IsItemHovered())
											{
												if (R->get_Unique_Id() == m_transformation_input_i)
													focused_active_item = true;
											}
											if (ImGui::IsItemActivated() && m_transformation_input_i == -1 )
											{
												
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;

												bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
												if (activating_input_state && !m_transformation_input_state)
												{
													m_transformation_input_state            = true;
													m_transformation_input_state_activating = true;
												}



												m_transformation_input_m0_down    = true;
												m_transformation_input_m0_release = false;
												m_transformation_input_data.starting_delta = prev_value;
												R_active = R;
												m_transformation_input_i = R->get_Unique_Id();
												m_transformation_input_attr_index = 0;
											}											
											if ( delta != 0 )
											{
												//R->set_DeltaLimit(optix::make_float2(v));
												if (delta != 0) m_dragFloat_state = true;
												m_transformation_input_data.delta_length = delta;
												is_changed = true;
											}

											ImGui::PopItemWidth();
										}
										
#endif
										
										ImGui::TreePop();

									}
									else
									{
										style.Colors[ImGuiCol_Button]        = style.Colors[ImGuiCol_Header];
										style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_HeaderHovered];
										style.Colors[ImGuiCol_ButtonActive]  = style.Colors[ImGuiCol_HeaderActive];

										ImGui::SameLine();
										ImGui::Dummy(ImVec2(0.0f, 0.0f));
										ImGui::SameLine();
										ImGui::Button(from_A.c_str(), ImVec2(95.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											m_ui_focused = true;
											Mediator::setFocusedObject(A_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(A_array_index);
											}
										}

										ImGui::SameLine();
										ImGui::Text(icon_label.c_str());
										ImGui::SameLine();
										
										ImGui::Button(to_B.c_str() , ImVec2(95.0f,20.0f));
										if (ImGui::IsItemHovered())
										{
											m_ui_focused = true;
											Mediator::setFocusedObject(B_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(B_array_index);
											}
										}

										style.Colors[ImGuiCol_Button]        = but;
										style.Colors[ImGuiCol_ButtonHovered] = but_hov;
										style.Colors[ImGuiCol_ButtonActive]  = but_act;

									}

								}
								else if (res_id == CONVERSATION_R)
								{
									ImGuiTreeNodeFlags node_flags = 0;
									//node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
									node_flags |= ImGuiTreeNodeFlags_FramePadding;
									node_flags |= ImGuiTreeNodeFlags_Framed;
									node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;

									ImGui::Dummy(ImVec2(20.0f, 0.0f));
									ImGui::SameLine();
									ImGui::Text("From:");
									ImGui::SameLine();
									ImGui::Dummy(ImVec2(100.0f, 0.0f));
									ImGui::SameLine();
									ImGui::Text("To:");

									std::string node_label = std::string("##") + std::to_string(R->get_Unique_Id());
									ImGuiID ID = ImGui::GetCurrentWindow()->GetID(node_label.c_str());
									bool is_gui_open = ImGui::TreeNodeBehaviorIsOpen(ID, node_flags);
									R->set_GUI_Open(is_gui_open);
									if (ImGui::TreeNodeEx_2(node_label.c_str(), 17.0f, node_flags))
									{
										style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_Header];
										style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_HeaderHovered];
										style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_HeaderActive];
										ImGui::SameLine();
										ImGui::Dummy(ImVec2(0.0f, 0.0f));
										ImGui::SameLine();
										ImGui::Button(from_A.c_str(), ImVec2(84.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											Mediator::setFocusedObject(A_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(A_array_index);
											}
										}

										ImGui::SameLine();
										ImGui::Dummy(ImVec2(0.0f, 0.0f));
										ImGui::SameLine();
										ImGui::Text(icon_label.c_str());
										ImGui::SameLine();

										ImGui::Button(to_B.c_str(), ImVec2(84.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											Mediator::setFocusedObject(B_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(B_array_index);
											}
										}
										style.Colors[ImGuiCol_Button] = but;
										style.Colors[ImGuiCol_ButtonHovered] = but_hov;
										style.Colors[ImGuiCol_ButtonActive] = but_act;

										ImGui::Text("   |_");
										ImGui::SameLine();
										ImGui::Button(from_a.c_str(), ImVec2(70.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											Mediator::setFocusedObject(a_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(a_array_index);
											}
										}
										ImGui::SameLine();

										guiStyleManager::setIconFont(MDI, 12);
										ImGui::Text(icon_label.c_str());
										ImGui::PopFont();

										ImGui::SameLine();
										ImGui::Button(to_b.c_str(), ImVec2(70.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											Mediator::setFocusedObject(b_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(b_array_index);
											}
										}
										ImGui::SameLine();
										
#ifdef edit_res_tol_var_in_menu

										ImGui::Text("_| ");
										{
											ImGui::Text("   |_");
											{
												float prev_value_a = R->get_DeltaLimit().x;
												float va           = prev_value_a;
												index_i = 1;
												index_j = 0;

												std::string label = std::string("##") + std::to_string(R->get_Unique_Id());
												ImGui::SameLine();
												ImGui::PushItemWidth(70.0f);
												ImGui::DragFloat(label.c_str(), &va, 0.1f, 0.0f, 89.9f); // Left DragFloat ( index = 0 )
												ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
												was_INPUT_active = m_transformation_input_state;
												is_INPUT_active  = ImGui::TempInputTextIsActive(ID);
												if (m_repositioning_cursor) va = prev_value_a;
												delta = va - prev_value_a;
												if (ImGui::IsItemHovered() 
													//&& m_transformation_input_attr_index == 0
													)
												{
														if (R->get_Unique_Id() == m_transformation_input_i && m_transformation_input_attr_index == 0)
															focused_active_item = true;
												}
												if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
												{
													Mediator::Reset_Restrictions_UI_States();
													R->set_Selected(true);
													m_UI_selected_Restriction = R;

													bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
													if (activating_input_state && !m_transformation_input_state)
													{
														m_transformation_input_state = true;
														m_transformation_input_state_activating = true;
													}

													m_transformation_input_m0_down = true;
													m_transformation_input_m0_release = false;
													m_transformation_input_data.starting_delta = prev_value_a;
													R_active = R;
													m_transformation_input_i = R->get_Unique_Id();
													m_transformation_input_attr_index = 0;
												}
												if (delta != 0)
												{
														//R->set_DeltaLimit(optix::make_float2(va));
														if (delta != 0) m_dragFloat_state = true;
														m_transformation_input_data.delta_length = delta;
														is_changed = true;
												}
												


												ImGui::SameLine();
												ImGui::Dummy(ImVec2(24.0f, 0.0f));

												//
												float prev_value_b = R->get_DeltaLimit_2().x;
												float vb = prev_value_b;
												index_i = 1;
												index_j = 1;


												label = std::string("##") + std::to_string(R->get_Unique_Id()) + "2";
												ImGui::SameLine();
												ImGui::PushItemWidth(70.0f);

												ImGui::DragFloat(label.c_str(), &vb, 0.1f, 0.0f, 89.9f); // Right DragFloat ( index = 1 )
												ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
												was_INPUT_active = m_transformation_input_state;
												is_INPUT_active = ImGui::TempInputTextIsActive(ID);
												if (m_repositioning_cursor) vb = prev_value_b;
												delta = vb - prev_value_b;
												
												if (ImGui::IsItemHovered())
												{
													if (R->get_Unique_Id() == m_transformation_input_i && m_transformation_input_attr_index == 1)
														focused_active_item = true;
												}
												if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
												{
													Mediator::Reset_Restrictions_UI_States();
													R->set_Selected(true);
													m_UI_selected_Restriction = R;

													bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
													if (activating_input_state && !m_transformation_input_state)
													{
														m_transformation_input_state = true;
														m_transformation_input_state_activating = true;
													}

													m_transformation_input_m0_down = true;
													m_transformation_input_m0_release = false;
													m_transformation_input_data.starting_delta = prev_value_b;
													R_active = R;
													m_transformation_input_i = R->get_Unique_Id();
													m_transformation_input_attr_index = 1;
												}
												if (delta != 0)
												{
													//R->set_DeltaLimit_2(optix::make_float2(vb));
													//float delta = vb - prev_value_b;
													if (delta != 0 /* || mouse_data.m0_down */)
														m_dragFloat_state = true;

													m_transformation_input_data.delta_length = delta;
													is_changed = true;
												}
												


												ImGui::SameLine();
												ImGui::Text("_| ");
												ImGui::PopItemWidth();
											}

										}

#endif

										ImGui::TreePop();
									}
									else
									{
										style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_Header];
										style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_HeaderHovered];
										style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_HeaderActive];

										ImGui::SameLine();
										ImGui::Dummy(ImVec2(0.0f, 0.0f));
										ImGui::SameLine();
										ImGui::Button(from_A.c_str(), ImVec2(88.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											Mediator::setFocusedObject(A_array_index);
											if (ImGui::IsMouseClicked(0))
												Mediator::setSelectedObject(A_array_index);
										}

										ImGui::SameLine();
										ImGui::Dummy(ImVec2(0.0f, 0.0f));
										ImGui::SameLine();
										ImGui::Text(icon_label.c_str());
										ImGui::SameLine();

										
										ImGui::Button(to_B.c_str(), ImVec2(88.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											Mediator::setFocusedObject(B_array_index);
											if (ImGui::IsMouseClicked(0))
												Mediator::setSelectedObject(B_array_index);
										}
										style.Colors[ImGuiCol_Button] = but;
										style.Colors[ImGuiCol_ButtonHovered] = but_hov;
										style.Colors[ImGuiCol_ButtonActive] = but_act;
									}

									

									// distance var //
#ifdef edit_res_tol_var_in_menu
									{
										ImGui::Dummy(ImVec2(0.0f, 2.0f));
										ImGui::Dummy(ImVec2(20.0f, 0.0)); ImGui::SameLine(); ImGui::Text("Distance : "); ImGui::SameLine();
										float prev_value_d = R->get_DistLimit().y;
										float vd = prev_value_d;
										index_i = 0;
										index_j = 2;

										std::string label = std::string("##") + std::to_string(R->get_Unique_Id()) + "dist_limit_var";
										ImGui::SameLine();
										ImGui::PushItemWidth(70.0f);
										ImGui::DragFloat(label.c_str(), &vd, 0.5f, 0.0f, 9999999.9f);
										ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
										was_INPUT_active = m_transformation_input_state;
										is_INPUT_active = ImGui::TempInputTextIsActive(ID);
										if (m_repositioning_cursor) vd = prev_value_d;
										delta = vd - prev_value_d;

										if (ImGui::IsItemHovered())
										{
											if (R->get_Unique_Id() == m_transformation_input_i && m_transformation_input_attr_index == 2)
												focused_active_item = true;
										}
										if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
										{
											Mediator::Reset_Restrictions_UI_States();
											R->set_Selected(true);
											m_UI_selected_Restriction = R;

											bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
											if (activating_input_state && !m_transformation_input_state)
											{
												m_transformation_input_state = true;
												m_transformation_input_state_activating = true;
											}

											m_transformation_input_m0_down = true;
											m_transformation_input_m0_release = false;
											m_transformation_input_data.starting_delta = prev_value_d;
											R_active = R;
											m_transformation_input_i = R->get_Unique_Id();
											m_transformation_input_attr_index = 2;
										}
										if (delta != 0)
										{
											//R->set_DistLimit(optix::make_float2(vd));
											if (delta != 0)
												m_dragFloat_state = true;

											m_transformation_input_data.delta_length = delta;
											is_changed = true;
										}

									}

#endif
									
								}
								else if (res_id == DISTANCE_R)
								{

									ImGuiTreeNodeFlags node_flags = 0;
									//node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
									node_flags |= ImGuiTreeNodeFlags_FramePadding;
									node_flags |= ImGuiTreeNodeFlags_Framed;
									node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;


									ImGui::Dummy(ImVec2(20.0f, 0.0f));
									ImGui::SameLine();
									ImGui::Text("From:");
									ImGui::SameLine();
									ImGui::Dummy(ImVec2(100.0f, 0.0f));
									ImGui::SameLine();
									ImGui::Text("To:");

									std::string node_label = std::string("##") + std::to_string(R->get_Unique_Id());
									ImGuiID ID = ImGui::GetCurrentWindow()->GetID(node_label.c_str());
									bool is_gui_open = ImGui::TreeNodeBehaviorIsOpen(ID, node_flags);
									R->set_GUI_Open(is_gui_open);
									if (ImGui::TreeNodeEx_2(node_label.c_str(), 17.0f, node_flags))
									{
										style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_Header];
										style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_HeaderHovered];
										style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_HeaderActive];

										ImGui::SameLine();
										ImGui::Dummy(ImVec2(0.0f, 0.0f));
										ImGui::SameLine();
										ImGui::Button(from_a.c_str(), ImVec2(95.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											m_ui_focused = true;
											Mediator::setFocusedObject(a_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;

												Mediator::setSelectedObject(a_array_index);
											}
										}

										ImGui::SameLine();
										ImGui::Text(icon_label.c_str());
										ImGui::SameLine();

										ImGui::Button(to_b.c_str(), ImVec2(95.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											m_ui_focused = true;
											Mediator::setFocusedObject(b_array_index);
											if (ImGui::IsMouseClicked(0))
											{
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;
												Mediator::setSelectedObject(b_array_index);
											}
										}

										style.Colors[ImGuiCol_Button] = but;
										style.Colors[ImGuiCol_ButtonHovered] = but_hov;
										style.Colors[ImGuiCol_ButtonActive] = but_act;


#ifdef edit_res_tol_var_in_menu

										ImGui::Text("               |__  ");
										{
											float prev_value = R->get_DistLimit().y;
											float v = prev_value;
											index_i = 2;
											index_j = -1;

											std::string label = std::string("##") + std::to_string(R->get_Unique_Id()) + "3";
											ImGui::SameLine();
											ImGui::PushItemWidth(85.0f);
											ImGui::DragFloat(label.c_str(), &v, 0.5f, 0.0f, 9999999.9f);
											ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
											was_INPUT_active = m_transformation_input_state;
											is_INPUT_active = ImGui::TempInputTextIsActive(ID);
											if (m_repositioning_cursor) v = prev_value;
											delta = v - prev_value;
											if (ImGui::IsItemHovered())
												{
													if (R->get_Unique_Id() == m_transformation_input_i)
														focused_active_item = true;
												}
											if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
											{
												//
												Mediator::Reset_Restrictions_UI_States();
												R->set_Selected(true);
												m_UI_selected_Restriction = R;

												bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
												if (activating_input_state && !m_transformation_input_state)
												{
													m_transformation_input_state = true;
													m_transformation_input_state_activating = true;
												}

												m_transformation_input_m0_down = true;
												m_transformation_input_m0_release = false;
												m_transformation_input_data.starting_delta = prev_value;
												R_active = R;
												m_transformation_input_i = R->get_Unique_Id();
												m_transformation_input_attr_index = 0;
											}
											if (delta != 0)
											{
												//R->set_DistLimit(optix::make_float2(0.0f,v));
												//float delta = v - prev_value;
												if (delta != 0) m_dragFloat_state = true;

												m_transformation_input_data.delta_length = delta;
												is_changed = true;
											}

											ImGui::PopItemWidth();
										}

										ImGui::SameLine();
										ImGui::Text("  __|               ");

#endif

										ImGui::TreePop();
									}
									else
									{
										style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_Header];
										style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_HeaderHovered];
										style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_HeaderActive];

										ImGui::SameLine();
										ImGui::Dummy(ImVec2(0.0f, 0.0f));
										ImGui::SameLine();
										ImGui::Button(from_a.c_str(), ImVec2(95.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											m_ui_focused = true;
											Mediator::setFocusedObject(a_array_index);
											if (ImGui::IsMouseClicked(0))
												Mediator::setSelectedObject(a_array_index);
										}

										ImGui::SameLine();
										ImGui::Text(icon_label.c_str());
										ImGui::SameLine();

										ImGui::Button(to_b.c_str(), ImVec2(95.0f, 20.0f));
										if (ImGui::IsItemHovered())
										{
											m_ui_focused = true;
											Mediator::setFocusedObject(b_array_index);
											if (ImGui::IsMouseClicked(0))
												Mediator::setSelectedObject(b_array_index);
										}

										style.Colors[ImGuiCol_Button] = but;
										style.Colors[ImGuiCol_ButtonHovered] = but_hov;
										style.Colors[ImGuiCol_ButtonActive] = but_act;

									}
								}

							}


							style.Colors[ImGuiCol_Text] = text_col;
							ImGui::PopFont();
						}
						
						
						guiStyleManager::setStyle("style_restrictions_list");
						style.Colors[ImGuiCol_Border] = brd_col_id;
						ImGui::EndChild();
						style.Colors[ImGuiCol_ChildBg] = ch_col;
						style.Colors[ImGuiCol_ChildWindowBg] = ch_col;
						style.Colors[ImGuiCol_Border] = bord_col;
					}

					ImGui::PopFont();
					ImGui::Dummy(ImVec2(0.0f, 4.0f));
					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child_transformations");
				}


				ImGui::Dummy(ImVec2(2, 5));
				ImGui::EndChild();
			}
			guiStyleManager::setStyle("main_style_right_child2");


			ImGui::Dummy(ImVec2(0, 4.0f));

			// Restrictions Settings 
			Restriction* R = m_UI_selected_Restriction;
			if( R != nullptr )
			{
				
				
				bool is_valid   = R != nullptr;
				int r_unique_id = is_valid ? R->get_Unique_Id() : -1;
				bool changed = false;
				Restriction_ID r_id = is_valid ? R->getId() : NONE_R;
				std::string A_name, B_name, a_name, b_name;
				int Aid, Bid, aid, bid, Aind, Bind, aind, bind;
				SceneObject* A;
				SceneObject* B;
				SceneObject* a;
				SceneObject* b;
				if (is_valid)
				{
					A = R->get_Owner(0);
					B = R->get_Owner(1);
					a = R->getSubject();
					b = R->getObject();

					Aid = A->getId();
					Bid = B->getId();
					aid = a->getId();
					bid = b->getId();

					Aind = Mediator::requestObjectsArrayIndex(A);
					Bind = Mediator::requestObjectsArrayIndex(B);
					aind = Mediator::requestObjectsArrayIndex(a);
					bind = Mediator::requestObjectsArrayIndex(b);


					A_name = get_IconLabel(A->getType()) + A->getName();// + std::string("##") + std::to_string(r_unique_id);
					B_name = get_IconLabel(B->getType()) + B->getName();// + std::string("##") + std::to_string(r_unique_id);
					a_name = get_IconLabel(a->getType()) + a->getName();// + std::string("##") + std::to_string(r_unique_id);
					b_name = get_IconLabel(b->getType()) + b->getName();// + std::string("##") + std::to_string(r_unique_id);

				}

				//
				guiStyleManager::setStyle("main_style_right_child_transformations");
				window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;
				float window_height_size;
				if      (r_id == FOCUS_R)        window_height_size = 525.0f;
				else if (r_id == CONVERSATION_R) window_height_size = 525.0f;
				else if (r_id == DISTANCE_R)     window_height_size = 525.0f;

				ImGui::BeginChild("restriction_params_child##", ImVec2(ImGui::GetWindowWidth() - 10, window_height_size), true, child_flags);

				m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();

				ImVec4 but     = ImGui::GetStyle().Colors[ImGuiCol_Button];
				ImVec4 but_hov = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
				ImVec4 but_sel = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

				ImGui::Dummy(ImVec2(1.0f, 0.0f));

				guiStyleManager::setIconFont(MDI, 14);
				ImGui::PushItemWidth(100.0f);
				
				if (is_valid)
				{
					
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
										
					float window_height = 0.0f;
					float window_height_offset = 200.0f;
					if      (R->getId() == Restriction_ID::FOCUS_R)        window_height = 145.0f;
					else if (R->getId() == Restriction_ID::CONVERSATION_R) window_height = 185 + 42.0f + 24.0f;
					else if (R->getId() == Restriction_ID::DISTANCE_R)     window_height = 125.0f;


					guiStyleManager::setStyle("main_style_right_child_transformations2");
					ImGui::BeginChild("error_child##", ImVec2(ImGui::GetWindowSize().x - 10.0f, window_height ), true, child_flags); // 150
					{

						m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();
						ImGui::Dummy(ImVec2(0.0f, 1.0f));
						
						guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
						ImGui::SameLine();
						ImGui::Text("Current Solution:");
						ImGui::PopFont();

						ImGui::Dummy(ImVec2(0.0f, 10.0f));

						optix::float2 er = optix::make_float2(0.0f);
						R->is_Valid(&er);
						ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Text];

						bool changed_fv = false;
						std::string fv_name_off = "      ";

						if      (r_id == FOCUS_R)
						{

							optix::float2 d_limit_1 = R->get_DeltaLimit();

							float max_delta1 = d_limit_1.x;
							float delta1 = er.x;
							float error1 = 0.0f;
							if (delta1 > max_delta1)
								error1 = delta1 - max_delta1;

							ImGui::Button(A_name.c_str(), ImVec2(100.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Aind);m_ui_focused = true;}
							

							ImGui::Dummy(ImVec2(5.0f, 0.0f)); ImGui::SameLine();
							ImGui::Text("|__"); ImGui::SameLine();
							ImGui::Button(a_name.c_str(), ImVec2(90.0f - 30.0f, 20.0f));
							if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(aind);m_ui_focused = true;}
							// combo list
							{
								guiStyleManager::setStyle("main_style_right_child_transformations22");
								ImGui::SameLine();
								ImGuiComboFlags comb_flags = 0;
								comb_flags |= ImGuiComboFlags_NoPreview;
								std::string comb_label = "" + std::string("##") + std::to_string(R->get_Unique_Id());

								ImGuiID ID = ImGui::GetCurrentWindow()->GetID(comb_label.c_str());
								bool popup_open_before = ImGui::IsPopupOpen(ID);
								if (ImGui::BeginCombo(comb_label.c_str(), "", comb_flags))
								{ 
									if (!popup_open_before)
									{
										R->set_old_Subject(a);
										R->set_old_Object(b);
									}

									for (SceneObject * fv : A->get_Face_Vectors_Active())
									{
										std::string fv_label = fv_name_off + fv->getName() + fv_name_off + std::string("##") + std::to_string(R->get_Unique_Id());
										ImGui::Selectable(fv_label.c_str());
										if (ImGui::IsItemHovered())
											R->set_Subject(fv);
										if (ImGui::IsItemActivated())
										{
											R->set_Subject(fv);
											R->set_old_Subject(fv);
											changed_fv = true;
										}
									}

									ImGui::EndCombo();
								}
								bool popup_open_after = ImGui::IsPopupOpen(ID);
								if (!popup_open_after && !changed_fv)
								{
									SceneObject* old_subject = R->get_old_Subject();
									if(old_subject != 0) R->set_Subject( old_subject );
									R->set_old_Subject(0);
								}
								
								guiStyleManager::setStyle("main_style_right_child_transformations2");
							}
							ImGui::SameLine(); ImGui::GetStyle().Colors[ImGuiCol_Text] = R->is_Valid() ? col_is_valid : col_isnt_valid; ImGui::Text(ICON_MDI_ARROW_RIGHT); ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col; ImGui::SameLine();
							ImGui::SameLine(); ImGui::Button(B_name.c_str(), ImVec2(100.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Bind);m_ui_focused = true;}



							//
							ImGui::Dummy(ImVec2(0.0f, 4.0f));

							guiStyleManager::setStyle("main_style_right_child_transformations3");
							ImGui::Dummy(ImVec2(0.0f, 0.0f));
							ImGui::SameLine();
							ImGui::BeginChild("info_child_fv1##", ImVec2(110.0f, 65.0f), true, child_flags);
							{
								std::string l1, l2, l3; l1 = std::to_string(max_delta1); l2 = std::to_string(delta1); l3 = std::to_string(error1);
								if (l1.size() > 8) l1.erase(l1.begin() + 7, l1.end());
								if (l2.size() > 8) l2.erase(l2.begin() + 7, l2.end());
								if (l3.size() > 8) l3.erase(l3.begin() + 7, l3.end());

								ImGui::Text(" Angle: ");
								ImGui::SameLine();

								ImGui::GetStyle().Colors[ImGuiCol_Text] = (error1 != 0.0f) ? ImGui::GetStyle().Colors[ImGuiCol_TabActive] : ImGui::GetStyle().Colors[ImGuiCol_TabHovered];
								ImGui::Text(l2.c_str());
								ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;

								ImGui::Text(" Tol:  ");
								{
									guiStyleManager::setStyle("style_restrictions_panel2");
									ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Text];
									ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);

									float prev_value = R->get_DeltaLimit().x;
									float v = prev_value;
									index_i = 0;
									index_j = -1;

									std::string label = std::string("##") + std::to_string(R->get_Unique_Id());
									ImGui::SameLine();
									ImGui::PushItemWidth(65.0f);

									ImGui::DragFloat(label.c_str(), &v, 0.1f, 0.0f, 89.9f);
									ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
									was_INPUT_active = m_transformation_input_state;
									is_INPUT_active  = ImGui::TempInputTextIsActive(ID);
									if (m_repositioning_cursor) v = prev_value;
									delta = v - prev_value;
									if (ImGui::IsItemHovered())
									{
										if (R->get_Unique_Id() == m_transformation_input_i)
											focused_active_item = true;
									}
									if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
									{

										Mediator::Reset_Restrictions_UI_States();
										R->set_Selected(true);
										m_UI_selected_Restriction = R;

										bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
										if (activating_input_state && !m_transformation_input_state)
										{
											m_transformation_input_state = true;
											m_transformation_input_state_activating = true;
										}

										m_transformation_input_m0_down = true;
										m_transformation_input_m0_release = false;
										m_transformation_input_data.starting_delta = prev_value;
										R_active = R;
										m_transformation_input_i = R->get_Unique_Id();
										m_transformation_input_attr_index = 0;
									}
									if (delta != 0)
									{
										if (delta != 0) m_dragFloat_state = true;
										m_transformation_input_data.delta_length = delta;
										is_changed = true;
									}

									ImGui::PopItemWidth();
									ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;
									guiStyleManager::setStyle("main_style_right_child_transformations3");
								}
								ImGui::Text(" Error: "); ImGui::SameLine(); ImGui::Text(l3.c_str());
								ImGui::Dummy(ImVec2(0, 2));

							}ImGui::EndChild();
							guiStyleManager::setStyle("main_style_right_child_transformations2");

						}
						else if (r_id == CONVERSATION_R)
						{

							optix::float2 d_limit_1 = R->get_DeltaLimit();
							optix::float2 d_limit_2 = R->get_DeltaLimit_2();
							optix::float2 d_limit_3 = R->get_DistLimit();

							float max_delta1 = d_limit_1.x;
							float max_delta2 = d_limit_2.x;
							float max_delta3 = d_limit_3.y;
							float delta1 = er.x;
							float delta2 = er.y;
							float delta3 = length(A->getCentroid_Transformed() - B->getCentroid_Transformed());
							float error1 = 0.0f;
							float error2 = 0.0f;
							float error3 = 0.0f;
							if (delta1 > max_delta1)
								error1 = delta1 - max_delta1;
							if (delta2 > max_delta2)
								error2 = delta2 - max_delta2;
							if (delta3 > max_delta3)
								error3 = delta3 - max_delta3;

							ImGui::Button(A_name.c_str(), ImVec2(100.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Aind);m_ui_focused = true;}
							ImGui::SameLine();
							ImGui::Dummy(ImVec2(53.0f - 20.0f, 0.0f));
							ImGui::SameLine();
							ImGui::Button(B_name.c_str(), ImVec2(100.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Bind);m_ui_focused = true;}


							ImGui::Dummy(ImVec2(5.0f, 0.0f)); ImGui::SameLine();
							ImGui::Text("|__"); ImGui::SameLine();
							ImGui::Button(a_name.c_str(), ImVec2(90.0f - 30.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(aind);m_ui_focused = true;}
							
							// combo list A
							{
								guiStyleManager::setStyle("main_style_right_child_transformations22");
								ImGui::SameLine();
								ImGuiComboFlags comb_flags = 0;
								comb_flags |= ImGuiComboFlags_NoPreview;
								std::string comb_label = "" + std::string("##") + "1" + std::to_string(R->get_Unique_Id());

								ImGuiID ID = ImGui::GetCurrentWindow()->GetID(comb_label.c_str());
								bool popup_open_before = ImGui::IsPopupOpen(ID);
								if (ImGui::BeginCombo(comb_label.c_str(), "", comb_flags))
								{
									if (!popup_open_before)
									{
										R->set_old_Subject(a);
										R->set_old_Object(b);
									}

									for (SceneObject * fv : A->get_Face_Vectors_Active())
									{
										std::string fv_label = fv_name_off + fv->getName() + fv_name_off + std::string("##") + "1"+ std::to_string(R->get_Unique_Id());
										ImGui::Selectable(fv_label.c_str());
										if (ImGui::IsItemHovered())
											R->set_Subject(fv);
										if (ImGui::IsItemActivated())
										{
											R->set_Subject(fv);
											R->set_old_Subject(fv);
											changed_fv = true;
										}
									}

									ImGui::EndCombo();
								}
								bool popup_open_after = ImGui::IsPopupOpen(ID);
								if (!popup_open_after && !changed_fv)
								{
									SceneObject* old_subject = R->get_old_Subject();
									if (old_subject != 0) R->set_Subject(old_subject);
									R->set_old_Subject(0);
								}
								guiStyleManager::setStyle("main_style_right_child_transformations2");

							}
							
							
							ImGui::SameLine();
							ImGui::SameLine(); ImGui::GetStyle().Colors[ImGuiCol_Text] = R->is_Valid() ? col_is_valid : col_isnt_valid; ImGui::Text(ICON_MDI_ARROW_RIGHT);ImGui::SameLine(); ImGui::Text(ICON_MDI_ARROW_LEFT); ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col; ImGui::SameLine();
							ImGui::SameLine();

							// combo list B
							{
								guiStyleManager::setStyle("main_style_right_child_transformations22");
								ImGui::SameLine();
								ImGuiComboFlags comb_flags = 0;
								comb_flags |= ImGuiComboFlags_NoPreview;
								std::string comb_label = "" + std::string("##") + "2" + std::to_string(R->get_Unique_Id());

								ImGuiID ID = ImGui::GetCurrentWindow()->GetID(comb_label.c_str());
								bool popup_open_before = ImGui::IsPopupOpen(ID);
								if (ImGui::BeginCombo(comb_label.c_str(), "", comb_flags))
								{
									if (!popup_open_before)
									{
										R->set_old_Subject(a);
										R->set_old_Object(b);
									}

									for (SceneObject * fv : B->get_Face_Vectors_Active())
									{
										std::string fv_label = fv_name_off + fv->getName() + fv_name_off + std::string("##") + "2" + std::to_string(R->get_Unique_Id());
										ImGui::Selectable(fv_label.c_str());
										if (ImGui::IsItemHovered())
											R->set_Object(fv);
										if (ImGui::IsItemActivated())
										{
											R->set_Object(fv);
											R->set_old_Object(fv);
											changed_fv = true;
										}
									}

									ImGui::EndCombo();
								}
								bool popup_open_after = ImGui::IsPopupOpen(ID);
								if (!popup_open_after && !changed_fv)
								{
									SceneObject* old_object = R->get_old_Object();
									if (old_object != 0) R->set_Object(old_object);
									R->set_old_Object(0);
								}
								guiStyleManager::setStyle("main_style_right_child_transformations2");
							}


							ImGui::SameLine();
							ImGui::Button(b_name.c_str(), ImVec2(90.0f - 30.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(bind);m_ui_focused = true;}
						
							ImGui::SameLine();
							ImGui::Text("__|");


							ImGui::Dummy(ImVec2(0.0f, 4.0f));


							guiStyleManager::setStyle("main_style_right_child_transformations3");
							ImGui::Dummy(ImVec2(0.0f, 0.0f));
							ImGui::SameLine();
							ImGui::BeginChild("info_child_fv1##", ImVec2(110.0f, 65.0f), true, child_flags);
							{
								m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();
								std::string l1, l2, l3; l1 = std::to_string(max_delta1); l2 = std::to_string(delta1); l3 = std::to_string(error1);
								if (l1.size() > 8) l1.erase(l1.begin() + 7, l1.end());
								if (l2.size() > 8) l2.erase(l2.begin() + 7, l2.end());
								if (l3.size() > 8) l3.erase(l3.begin() + 7, l3.end());

								ImGui::Text(" Angle: ");
								ImGui::SameLine();

								ImGui::GetStyle().Colors[ImGuiCol_Text] = (error1 != 0.0f) ? ImGui::GetStyle().Colors[ImGuiCol_TabActive] : ImGui::GetStyle().Colors[ImGuiCol_TabHovered];
								ImGui::Text(l2.c_str());
								ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;


								//ImGui::Text(" Max : "); ImGui::SameLine(); ImGui::Text(l1.c_str());
								ImGui::Text(" Tol: ");
								{
									guiStyleManager::setStyle("style_restrictions_panel2");
									ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Text];
									ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);

									float prev_value_a = R->get_DeltaLimit().x;
									float va = prev_value_a;
									index_i = 1;
									index_j = 0;

									std::string label = std::string("##") + std::to_string(R->get_Unique_Id());
									ImGui::SameLine();
									ImGui::PushItemWidth(70.0f);
									ImGui::DragFloat(label.c_str(), &va, 0.1f, 0.0f, 89.9f); // Left DragFloat ( index = 0 )
									ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
									was_INPUT_active = m_transformation_input_state;
									is_INPUT_active = ImGui::TempInputTextIsActive(ID);
									if (m_repositioning_cursor) va = prev_value_a;
									delta = va - prev_value_a;
									if (ImGui::IsItemHovered()
										//&& m_transformation_input_attr_index == 0
										)
									{
										if (R->get_Unique_Id() == m_transformation_input_i && m_transformation_input_attr_index == 0)
											focused_active_item = true;
									}
									if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
									{
										Mediator::Reset_Restrictions_UI_States();
										R->set_Selected(true);
										m_UI_selected_Restriction = R;

										bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
										if (activating_input_state && !m_transformation_input_state)
										{
											m_transformation_input_state = true;
											m_transformation_input_state_activating = true;
										}

										m_transformation_input_m0_down = true;
										m_transformation_input_m0_release = false;
										m_transformation_input_data.starting_delta = prev_value_a;
										R_active = R;
										m_transformation_input_i = R->get_Unique_Id();
										m_transformation_input_attr_index = 0;
									}
									if (delta != 0)
									{
										if (delta != 0) m_dragFloat_state = true;
										m_transformation_input_data.delta_length = delta;
										is_changed = true;
									}

									ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;
									guiStyleManager::setStyle("main_style_right_child_transformations3");

								}
								ImGui::Text(" Error: "); ImGui::SameLine(); ImGui::Text(l3.c_str());

								ImGui::Dummy(ImVec2(0, 2));

							}ImGui::EndChild();
							guiStyleManager::setStyle("main_style_right_child_transformations2");

							ImGui::SameLine();
							ImGui::Dummy(ImVec2(10.0f, 0.0f));
							ImGui::SameLine();

							guiStyleManager::setStyle("main_style_right_child_transformations3");
							ImGui::BeginChild("info_child_fv2##", ImVec2(110.0f, 65.0f), true, child_flags);
							{
								m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();
								std::string l1, l2, l3; l1 = std::to_string(max_delta2); l2 = std::to_string(delta2); l3 = std::to_string(error2);
								if (l1.size() > 8) l1.erase(l1.begin() + 7, l1.end());
								if (l2.size() > 8) l2.erase(l2.begin() + 7, l2.end());
								if (l3.size() > 8) l3.erase(l3.begin() + 7, l3.end());

								ImGui::Text(" Angle: ");
								ImGui::SameLine();

								ImGui::GetStyle().Colors[ImGuiCol_Text] = (error2 != 0.0f) ? ImGui::GetStyle().Colors[ImGuiCol_TabActive] : ImGui::GetStyle().Colors[ImGuiCol_TabHovered];
								ImGui::Text(l2.c_str());
								ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;

								ImGui::Text(" Tol: ");
								{
									guiStyleManager::setStyle("style_restrictions_panel2");
									ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Text];
									ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);

									float prev_value_b = R->get_DeltaLimit_2().x;
									float vb = prev_value_b;
									index_i = 1;
									index_j = 1;


									label = std::string("##") + std::to_string(R->get_Unique_Id()) + "2";
									ImGui::SameLine();
									ImGui::PushItemWidth(70.0f);

									ImGui::DragFloat(label.c_str(), &vb, 0.1f, 0.0f, 89.9f); // Right DragFloat ( index = 1 )
									ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
									was_INPUT_active = m_transformation_input_state;
									is_INPUT_active = ImGui::TempInputTextIsActive(ID);
									if (m_repositioning_cursor) vb = prev_value_b;
									delta = vb - prev_value_b;

									if (ImGui::IsItemHovered())
									{
										if (R->get_Unique_Id() == m_transformation_input_i && m_transformation_input_attr_index == 1)
											focused_active_item = true;
									}
									if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
									{
										Mediator::Reset_Restrictions_UI_States();
										R->set_Selected(true);
										m_UI_selected_Restriction = R;

										bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
										if (activating_input_state && !m_transformation_input_state)
										{
											m_transformation_input_state = true;
											m_transformation_input_state_activating = true;
										}

										m_transformation_input_m0_down = true;
										m_transformation_input_m0_release = false;
										m_transformation_input_data.starting_delta = prev_value_b;
										R_active = R;
										m_transformation_input_i = R->get_Unique_Id();
										m_transformation_input_attr_index = 1;
									}
									if (delta != 0)
									{
										if (delta != 0 /* || mouse_data.m0_down */)
											m_dragFloat_state = true;

										m_transformation_input_data.delta_length = delta;
										is_changed = true;
									}

									ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;
									guiStyleManager::setStyle("main_style_right_child_transformations3");

								}
								ImGui::Text(" Error: "); ImGui::SameLine(); ImGui::Text(l3.c_str());

								ImGui::Dummy(ImVec2(0, 2));

							}ImGui::EndChild();
							guiStyleManager::setStyle("main_style_right_child_transformations2");


							ImGui::Dummy(ImVec2(0.0f, 2.0f));
							ImGui::Dummy(ImVec2(60.0f - 15.0f - 9.0f, 0.0f));
							ImGui::SameLine();


							// total error child
							ImGui::Text("|_"); ImGui::SameLine();
							guiStyleManager::setStyle("main_style_right_child_transformations3");
							ImGui::GetStyle().WindowPadding -= ImVec2(8.0f, 4.0f);
							ImGui::BeginChild("info_child_total_error##", ImVec2(110.0f + 30.0f, 18.0f), true, child_flags);
							{
								m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();
								float total_error = error1 + error2;
								std::string l1; l1 = std::to_string(total_error);
								if (l1.size() > 8) l1.erase(l1.begin() + 7, l1.end());
								
								ImGui::Text("    Total Error: ");
								ImGui::SameLine();

								ImGui::Text(l1.c_str());


							}ImGui::EndChild(); ImGui::SameLine(); ImGui::Text("_|");


							// distance child info
							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
							ImGui::Dummy(ImVec2(0.0f, 6.0f));
							ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5f - 35.0f, 0.0f)); ImGui::SameLine(); ImGui::Text("Distance: ");
							ImGui::Dummy(ImVec2(47.0f, 0.0f)); ImGui::SameLine();
							ImGui::PopFont();

							
							ImGui::BeginChild("info_child_dist_total_error##", ImVec2(140.0f, 60.0f), true, child_flags);
							{
								m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();
								ImGui::Dummy(ImVec2(0.0f, 1.0f));

								
								std::string l1, l2, l3; 
								l1 = std::to_string(error3);
								l2 = std::to_string(max_delta3);
								l3 = std::to_string(delta3);
								if (l1.size() > 8) l1.erase(l1.begin() + 7, l1.end());
								if (l2.size() > 8) l2.erase(l2.begin() + 7, l2.end());
								if (l3.size() > 8) l3.erase(l3.begin() + 7, l3.end());

								ImGui::Text("    Dist: "); ImGui::SameLine();
								
								ImGui::GetStyle().Colors[ImGuiCol_Text] = (error3 != 0.0f) ? ImGui::GetStyle().Colors[ImGuiCol_TabActive] : ImGui::GetStyle().Colors[ImGuiCol_TabHovered];
								ImGui::Text((l3).c_str());
								ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;

								ImGui::Text("    Tol: "); //ImGui::SameLine(); ImGui::Text((l2).c_str());
								{
									guiStyleManager::setStyle("style_restrictions_panel2");
									ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Text];
									ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);

									float prev_value_d = R->get_DistLimit().y;
									float vd = prev_value_d;
									index_i = 0;
									index_j = 2;

									std::string label = std::string("##") + std::to_string(R->get_Unique_Id()) + "dist_limit_var";
									ImGui::SameLine();
									ImGui::PushItemWidth(70.0f);
									ImGui::DragFloat(label.c_str(), &vd, 0.5f, 0.0f, 9999999.9f);
									ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
									was_INPUT_active = m_transformation_input_state;
									is_INPUT_active = ImGui::TempInputTextIsActive(ID);
									if (m_repositioning_cursor) vd = prev_value_d;
									delta = vd - prev_value_d;

									if (ImGui::IsItemHovered())
									{
										if (R->get_Unique_Id() == m_transformation_input_i && m_transformation_input_attr_index == 2)
											focused_active_item = true;
									}
									if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
									{
										Mediator::Reset_Restrictions_UI_States();
										R->set_Selected(true);
										m_UI_selected_Restriction = R;

										bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
										if (activating_input_state && !m_transformation_input_state)
										{
											m_transformation_input_state = true;
											m_transformation_input_state_activating = true;
										}

										m_transformation_input_m0_down = true;
										m_transformation_input_m0_release = false;
										m_transformation_input_data.starting_delta = prev_value_d;
										R_active = R;
										m_transformation_input_i = R->get_Unique_Id();
										m_transformation_input_attr_index = 2;
									}
									if (delta != 0)
									{
										if (delta != 0)
											m_dragFloat_state = true;

										m_transformation_input_data.delta_length = delta;
										is_changed = true;
									}


									ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;
									guiStyleManager::setStyle("main_style_right_child_transformations3");
								}

								ImGui::Text("    Error: "); ImGui::SameLine(); ImGui::Text((l1).c_str());

							}ImGui::EndChild();

							ImGui::GetStyle().WindowPadding += ImVec2(8.0f, 4.0f);
							guiStyleManager::setStyle("main_style_right_child_transformations2");

						}
						else if (r_id == DISTANCE_R)
						{

							optix::float2 d_limit_1 = R->get_DistLimit();

							float max_delta1 = d_limit_1.y;
							float delta1 = er.x;
							float error1 = 0.0f;
							if (delta1 > max_delta1)
								error1 = delta1 - max_delta1;

							ImGui::Button(A_name.c_str(), ImVec2(100.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Aind);m_ui_focused = true;}
							
							//ImGui::Dummy(ImVec2(53.0f - 20.0f, 0.0f));
							ImGui::SameLine();ImGui::Dummy(ImVec2(5.0f, 0.0f));
							ImGui::SameLine(); ImGui::GetStyle().Colors[ImGuiCol_Text] = R->is_Valid() ? col_is_valid : col_isnt_valid; ImGui::Text(ICON_MDI_ARROW_EXPAND_HORIZONTAL); ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col; ImGui::SameLine();
							ImGui::Dummy(ImVec2(5.0f, 0.0f));ImGui::SameLine();

							ImGui::Button(B_name.c_str(), ImVec2(100.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Bind);m_ui_focused = true;}

							ImGui::Dummy(ImVec2(5.0f, 0.0f)); ImGui::SameLine();
							ImGui::Dummy(ImVec2(0.0f, 4.0f));


							guiStyleManager::setStyle("main_style_right_child_transformations3");
							ImGui::Dummy(ImVec2(55.0f, 0.0f));
							ImGui::SameLine();
							ImGui::BeginChild("info_child_fv1##", ImVec2(120.0f, 65.0f), true, child_flags);
							{
								m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();
								std::string l1, l2, l3; l1 = std::to_string(max_delta1); l2 = std::to_string(delta1); l3 = std::to_string(error1);
								if (l1.size() > 8) l1.erase(l1.begin() + 7, l1.end());
								if (l2.size() > 8) l2.erase(l2.begin() + 7, l2.end());
								if (l3.size() > 8) l3.erase(l3.begin() + 7, l3.end());



								ImGui::Text(" Dist: ");
								ImGui::SameLine();

								ImGui::GetStyle().Colors[ImGuiCol_Text] = (error1 != 0.0f) ? ImGui::GetStyle().Colors[ImGuiCol_TabActive] : ImGui::GetStyle().Colors[ImGuiCol_TabHovered];
								ImGui::Text(l2.c_str());
								ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;

								ImGui::Text(" Tol: "); 
								{
									guiStyleManager::setStyle("style_restrictions_panel2");
									ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Text];
									ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);

									float prev_value = R->get_DistLimit().y;
									float v = prev_value;
									index_i = 2;
									index_j = -1;

									std::string label = std::string("##") + std::to_string(R->get_Unique_Id()) + "3";
									ImGui::SameLine();
									ImGui::PushItemWidth(80.0f);
									ImGui::DragFloat(label.c_str(), &v, 0.5f, 0.0f, 9999999.9f);
									ID = ImGui::GetCurrentWindow()->GetID(label.c_str());
									was_INPUT_active = m_transformation_input_state;
									is_INPUT_active = ImGui::TempInputTextIsActive(ID);
									if (m_repositioning_cursor) v = prev_value;
									delta = v - prev_value;
									if (ImGui::IsItemHovered())
									{
										if (R->get_Unique_Id() == m_transformation_input_i)
											focused_active_item = true;
									}
									if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
									{
										//
										Mediator::Reset_Restrictions_UI_States();
										R->set_Selected(true);
										m_UI_selected_Restriction = R;

										bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
										if (activating_input_state && !m_transformation_input_state)
										{
											m_transformation_input_state = true;
											m_transformation_input_state_activating = true;
										}

										m_transformation_input_m0_down = true;
										m_transformation_input_m0_release = false;
										m_transformation_input_data.starting_delta = prev_value;
										R_active = R;
										m_transformation_input_i = R->get_Unique_Id();
										m_transformation_input_attr_index = 0;
									}
									if (delta != 0)
									{
										if (delta != 0) m_dragFloat_state = true;

										m_transformation_input_data.delta_length = delta;
										is_changed = true;
									}

									ImGui::PopItemWidth();

									ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col;
									guiStyleManager::setStyle("main_style_right_child_transformations3");
								}

								ImGui::Text(" Error: "); ImGui::SameLine(); ImGui::Text(l3.c_str());

								ImGui::Dummy(ImVec2(0.0f, 2.0f));
							}ImGui::EndChild();
							guiStyleManager::setStyle("main_style_right_child_transformations2");


						}


					}ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child_transformations");

					// any solution child //
					if(r_id == FOCUS_R || r_id == CONVERSATION_R)
					{

						ImGui::Dummy(ImVec2(0.0f, 1.0f));

						bool is_focus = r_id == FOCUS_R;
						bool is_conv  = r_id == CONVERSATION_R;
					
						guiStyleManager::setStyle("main_style_right_child_transformations2");
						ImGui::BeginChild("error_child2##", ImVec2(ImGui::GetWindowSize().x - 10.0f, 145.0f), true, window_flags4); // 150
						{

							m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();
							ImGui::Dummy(ImVec2(0.0f, 1.0f));

							guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
							ImGui::SameLine();
							ImGui::Text("Any Solution:");
							ImGui::PopFont();

							ImGui::Dummy(ImVec2(0.0f, 10.0f));

							ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Text];
							bool changed_fv  = false;
							bool changed_fv2 = false;
							bool changed_fv3 = false;
							std::string fv_name_off = "      ";
							optix::float2 d_limit_1 = R->get_DeltaLimit();
							optix::float2 er = optix::make_float2(0.0f);

							std::vector<SceneObject*> A_fvs = A->get_Face_Vectors_Active();
							std::vector<SceneObject*> B_fvs = B->get_Face_Vectors_Active();
							SceneObject* original_a = R->getSubject();
							SceneObject* original_b = R->getObject();

							struct R_Pair
							{
								SceneObject* p_a = 0;
								SceneObject* p_b = 0;
							};
							std::vector<R_Pair> pairs;

							if      (is_focus)
							{
								for (SceneObject* fv : A_fvs)
								{
									R->set_Subject(fv);
									bool is_valid = R->is_Valid(&er);
									if (is_valid) pairs.push_back(R_Pair{ fv, B });
								}
							}
							else if (is_conv)
							{
								for (SceneObject* fva : A_fvs)
								{
									R->set_Subject(fva);
									for (SceneObject* fvb : B_fvs)
									{
										R->set_Object(fvb);
										bool is_valid = R->is_Valid(&er);
										if (is_valid)
										{
											int fva_id = fva->getId();
											int fvb_id = fvb->getId();

											bool exists = false;
											for (R_Pair pair : pairs)
											{
												int pa_id = pair.p_a->getId();
												int pb_id = pair.p_b->getId();
												
												if (
													   (pa_id == fva_id && pb_id == fvb_id)
													|| (pb_id == fva_id && pa_id == fvb_id)
													)
												{
													exists = true;
													break;
												}
											}

											if(!exists) pairs.push_back(R_Pair{ fva, fvb });
										}
									}
									
								}
							}
							R->set_Subject(original_a);
							R->set_Object(original_b);
							//

							float max_delta1 = d_limit_1.x;
							float delta1 = er.x;
							float error1 = 0.0f;
							if (delta1 > max_delta1)
								error1 = delta1 - max_delta1;

							if      (is_focus)
							{
								if (pairs.size() > 0)
								{
									std::string A_label = A_name + "##" + std::to_string(R->get_Unique_Id()) + std::to_string(A->getId());
									ImGui::Button(A_label.c_str(), ImVec2(100.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Aind);m_ui_focused = true; }
								}
								
								SceneObject* original_old_subj = R->get_old_Subject2();
								bool is_any_hovered = false;
								for (R_Pair pair : pairs)
								{
									ImGui::Dummy(ImVec2(5.0f, 0.0f)); ImGui::SameLine();
									ImGui::Text("|__"); ImGui::SameLine();
									std::string a_label = get_IconLabel(pair.p_a->getType()) + pair.p_a->getName() + "##" + std::to_string(R->get_Unique_Id()) + std::to_string(pair.p_a->getId());
									ImGui::Button(a_label.c_str(), ImVec2(90.0f - 30.0f, 20.0f));
									if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Mediator::requestObjectsArrayIndex(pair.p_a));m_ui_focused = true; }

									std::string button_labela = ICON_MDI_CIRCLE_SMALL + std::string("##") + a_label + " button ";
									
									// button a //
									{
										ImGui::SameLine();
										ImGui::Button(button_labela.c_str());
										if (ImGui::IsItemHovered())
										{
											is_any_hovered = true;
											is_button_hovered = true;

											if (button_hovered_id != button_labela)
											{
												SceneObject* old_subject = R->get_old_Subject2();
												if (old_subject == 0) old_subject = a;
												if (old_subject != 0)
													R->set_old_Subject2(old_subject);
												else
													R->set_old_Subject2(a);
											}
											button_hovered_id = button_labela;
											R->set_Subject(pair.p_a);
										}
										if (ImGui::IsItemActivated())
										{
											R->set_Subject(pair.p_a);
											R->set_old_Subject2(pair.p_a);
											changed_fv = true;
										}
									}
									//
									ImGui::SameLine(); ImGui::GetStyle().Colors[ImGuiCol_Text] = col_is_valid;  ImGui::Text(ICON_MDI_ARROW_RIGHT); ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col; ImGui::SameLine();

									std::string b_label = get_IconLabel(pair.p_b->getType()) + pair.p_b->getName() + "##" + std::to_string(R->get_Unique_Id()) + std::to_string(pair.p_b->getId());
									ImGui::Button(b_label.c_str(), ImVec2(100.0f, 20.0f));
									if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Mediator::requestObjectsArrayIndex(pair.p_b));m_ui_focused = true; }

								}

								// reset button states
								if (!is_any_hovered && !changed_fv)
								{
									is_button_hovered = false;
									button_hovered_id = "";
									
									SceneObject* old_subject = R->get_old_Subject2();
									if (original_old_subj != 0)
										old_subject = original_old_subj;
									if (old_subject != 0) R->set_Subject(old_subject);
									R->set_old_Subject2(0);
								}
								if (!is_any_hovered)
								{
									button_hovered_id = "";
									is_button_hovered = false;
									R->set_old_Subject2(0);
									R->set_old_Subject(0);
								}
							}
							else if (is_conv)
							{
								std::string A_label = A_name + "##" + std::to_string(R->get_Unique_Id()) + std::to_string(A->getId());
								std::string B_label = B_name + "##" + std::to_string(R->get_Unique_Id()) + std::to_string(B->getId());

								if (pairs.size() > 0)
								{
									ImGui::Button(A_label.c_str(), ImVec2(100.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Aind);m_ui_focused = true; }
									ImGui::SameLine();
									ImGui::Dummy(ImVec2(53.0f - 28.0f, 0.0f));
									ImGui::SameLine();
									ImGui::Button(B_label.c_str(), ImVec2(100.0f, 20.0f));if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Bind);m_ui_focused = true; }
								}

								SceneObject* original_old_subj = R->get_old_Subject3();
								SceneObject* original_old_obj  = R->get_old_Object3();
								bool is_any_hovered = false;
								for (R_Pair pair : pairs)
								{
									ImGui::Dummy(ImVec2(5.0f, 0.0f)); ImGui::SameLine();
									ImGui::Text("|_"); ImGui::SameLine();
									std::string a_label = get_IconLabel(pair.p_a->getType()) + pair.p_a->getName() + "##" + std::to_string(R->get_Unique_Id()) + std::to_string(pair.p_a->getId());
									std::string b_label = get_IconLabel(pair.p_b->getType()) + pair.p_b->getName() + "##" + std::to_string(R->get_Unique_Id()) + std::to_string(pair.p_b->getId());

									ImGui::Button(a_label.c_str(), ImVec2(90.0f - 30.0f, 20.0f));
									if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Mediator::requestObjectsArrayIndex(pair.p_a));m_ui_focused = true; }

									std::string button_labela = ICON_MDI_CIRCLE_SMALL +std::string("##") + a_label + " button ";
									std::string button_labelb = ICON_MDI_CIRCLE_SMALL + std::string("##") + b_label + " button ";
									
									
									// button a //
									{
										ImGui::SameLine();
										ImGui::Button(button_labela.c_str());
										if (ImGui::IsItemHovered())
										{
											is_any_hovered = true;
											is_buttona_hovered = true;

											if (buttona_hovered_id != button_labela)
											{
												SceneObject* old_subject = R->get_old_Subject3() != 0 ? R->get_old_Subject3() : a;
												R->set_old_Subject3(old_subject);
											}

											buttona_hovered_id = button_labela;
											R->set_Subject(pair.p_a);
										}
										if (ImGui::IsItemActivated())
										{
											R->set_Subject(pair.p_a);
											R->set_old_Subject3(pair.p_a);
											changed_fv2 = true;
										}
									}
									//
									
									ImGui::SameLine(); ImGui::GetStyle().Colors[ImGuiCol_Text] = col_is_valid;  ImGui::Text(ICON_MDI_ARROW_RIGHT); ImGui::SameLine(); ImGui::Text(ICON_MDI_ARROW_LEFT); ImGui::GetStyle().Colors[ImGuiCol_Text] = t_col; ImGui::SameLine();
									
									// button b //
									{
										ImGui::Button(button_labelb.c_str());
										if (ImGui::IsItemHovered())
										{
											is_any_hovered     = true;
											is_buttonb_hovered = true;

											if (buttonb_hovered_id != button_labelb)
											{
												SceneObject* old_object  = R->get_old_Object3()  != 0 ? R->get_old_Object3()  : b;
												R->set_old_Object3(old_object);
											}
											buttonb_hovered_id = button_labelb;
											R->set_Object(pair.p_b);
										}
										if (ImGui::IsItemActivated())
										{
											R->set_Object(pair.p_b);
											R->set_old_Object3(pair.p_b);
											changed_fv3 = true;
										}
										ImGui::SameLine();
									}
									//
									
									ImGui::Button(b_label.c_str(), ImVec2(90.0f - 30.0f, 20.0f));
									if (ImGui::IsItemHovered()) { Mediator::setFocusedObject(Mediator::requestObjectsArrayIndex(pair.p_b));m_ui_focused = true; }
									ImGui::SameLine();ImGui::Text("_|");
								}

								// reset button states
								if (!is_any_hovered && !changed_fv2)
								{
									is_buttona_hovered = false;
									//is_buttonb_hovered = false;
									buttona_hovered_id = "";
									//buttonb_hovered_id = "";

									SceneObject* old_subject = R->get_old_Subject3();
									//SceneObject* old_object  = R->get_old_Object3();
									if (original_old_subj != 0) old_subject = original_old_subj;
									if (old_subject != 0) R->set_Subject(old_subject);
									R->set_old_Subject3(0);
								}
								if (!is_any_hovered && !changed_fv3)
								{
									//is_buttona_hovered = false;
									is_buttonb_hovered = false;
									//buttona_hovered_id = "";
									buttonb_hovered_id = "";

									
									SceneObject* old_object  = R->get_old_Object3();
									if (original_old_obj != 0) old_object = original_old_obj;
									if (old_object != 0) R->set_Object(old_object);
									R->set_old_Object3(0);
								}
								if (!is_any_hovered)
								{
									buttona_hovered_id = "";
									buttonb_hovered_id = "";
									is_buttona_hovered = false;
									is_buttonb_hovered = false;

									R->set_old_Subject3(0);
									R->set_old_Object3(0);
									//R->set_old_Subject2(0);
									//R->set_old_Subject(0);
								}
							}


							//
							ImGui::Dummy(ImVec2(0.0f, 4.0f));
							guiStyleManager::setStyle("main_style_right_child_transformations2");


						}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child_transformations");

					}
					//

					ImGui::Dummy(ImVec2(0.0f, 1.0f));

					// Remove Button //
					guiStyleManager::setStyle("main_style_right_child_transformations_inner1");
					ImGui::BeginChild("remove_button_child__##", ImVec2(ImGui::GetWindowSize().x - 10.0f, 70.0f ), true, child_flags);
					{
						m_ui_restrictions_panel_focused |= ImGui::IsWindowFocused();
						
						ImGui::Dummy(ImVec2(0.0f, 1.f));
						//ImGui::SameLine();
						guiStyleManager::setIconFont(MDI, 16);
						ImGui::Text(ICON_MDI_SETTINGS);
						ImGui::PopFont();
						


						ImGui::Dummy(ImVec2(0.0f, 12.0f));

						ImGui::Dummy(ImVec2(35.0f, 0.0f));
						ImGui::SameLine();
						if (ImGui::Button("Remove Restriction", ImVec2(170.0f, 23.0f)))
						{
							Action* ac = new Action_CRT_DLT_RESTRICTION(R, 0);
							ActionManager::GetInstance().push(ac);
							Mediator::Reset_Restrictions_UI_States();
							m_UI_selected_Restriction = nullptr;
						}

					}ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child_transformations");

					


				}

				ImGui::PopItemWidth();
				ImGui::PopFont();
				

				ImGui::Dummy(ImVec2(2, 5));
				ImGui::EndChild();
			}
			guiStyleManager::setStyle("main_style_right_child2");


			// actions //
			if( !is_null ||  m_UI_selected_Restriction != 0 )
			{
				
#define new_action_code
#ifdef new_action_code

				

				bool vk_key = key_data[GLFW_KEY_ENTER].down || (key_data[GLFW_KEY_TAB].down && !m_transformation_input_state_activating);
				bool m0_DOWN = ImGui::IsMouseClicked(0);
				bool m0_REL = ImGui::IsMouseReleased(0);
				bool save_ac = 0;
				bool reset_state_1 = 0;
				bool reset_state_2 = 0;
				//bool reset_state = 0;
				bool input_state = 0;

				bool dragging_state_end = (m_transformation_input_m0_down && !m_transformation_input_state_activating);
				bool input_state_end = (m0_DOWN && !focused_active_item && !m_transformation_input_state_activating) || vk_key;

				int ind_i = m_transformation_input_i;
				int ind_j = m_transformation_input_attr_index;

				float delta = m_transformation_input_data.delta_length;
				optix::float3 delta_3f = optix::make_float3(delta);

				if (is_changed && m_UI_selected_Restriction != nullptr)
				{

					Restriction_ID r_type = m_UI_selected_Restriction->getId();

					// focus
					if      (r_type == Restriction_ID::FOCUS_R)
					{
						optix::float2 v = m_UI_selected_Restriction->get_DeltaLimit() + optix::make_float2(delta);
						m_UI_selected_Restriction->set_DeltaLimit(v);
					}
					// conversation
					else if (r_type == Restriction_ID::CONVERSATION_R)
					{
						// subject
						if (ind_j == 0)
						{
							optix::float2 v = m_UI_selected_Restriction->get_DeltaLimit() + optix::make_float2(delta);
							m_UI_selected_Restriction->set_DeltaLimit(v);
						}

						// object
						else if (ind_j == 1)
						{
							optix::float2 v = m_UI_selected_Restriction->get_DeltaLimit_2() + optix::make_float2(delta);
							m_UI_selected_Restriction->set_DeltaLimit_2(v);
						}

						// distance var
						else if (ind_j == 2)
						{
							optix::float2 v = m_UI_selected_Restriction->get_DistLimit() + optix::make_float2(delta);
							m_UI_selected_Restriction->set_DistLimit(v);
						}
					}
					// distance
					else if (r_type == Restriction_ID::DISTANCE_R)
					{
						optix::float2 v = m_UI_selected_Restriction->get_DistLimit() + optix::make_float2(delta);
						m_UI_selected_Restriction->set_DistLimit(v);

					}

					// accumulate transformation delta
					m_transformation_input_data.overall_delta += delta_3f;
				}
				else
				{
					delta_3f = ZERO_3f;
					delta    = 0.0f;
					m_transformation_input_data.delta_length = 0.0f;
					m_transformation_input_data.delta        = ZERO_3f;
				}

				// transformation input active
				if (m_transformation_input_state)
				{
					if (input_state_end)
					{
						reset_state_1 = 1;
						input_state = 1;
					}
				}
				else
				{
					if (m0_REL)
					{
						if (dragging_state_end)
						{

							reset_state_2 = 1;
							input_state = 0;
						}

					}
				}


				if (reset_state_1 || reset_state_2)
				{
					
					
					optix::float3 overall_delta = m_transformation_input_data.overall_delta;
					float delta_length = length(overall_delta);
					save_ac = fabsf(delta_length) > 1e-5f ? 1 : 0;

					////std::cout << "\n - Reset_State : " << std::endl;
					////std::cout << " - m_UI_selected_Restriction : " << (m_UI_selected_Restriction != nullptr) << std::endl;
					////std::cout << " - ind_i : " << ind_i << std::endl;
					////std::cout << " - save_ac : " << save_ac << std::endl;


					if (save_ac && ind_i != -1 && m_UI_selected_Restriction != nullptr)
					{

						optix::float2 overall_delta = optix::make_float2(m_transformation_input_data.overall_delta.x);

						// pushing Action ... 
						Action * ac     = 0;
						ac = new Action_Edit_RESTRICTION( m_UI_selected_Restriction, overall_delta , ind_j );
						ActionManager::GetInstance().push(ac);
					}
					


					// reset data //
					{
						m_transformation_input_data.reset();
						m_transformation_input_i = -1;
						m_transformation_input_attr_index = -1;
						m_transformation_input_state = false;
						m_transformation_input_state_activating = false;
						m_plus_minus_button_state = false;
					}

				}
#endif

#ifdef old_action_code

				// m0_down
				bool vk_enter = (key_data[GLFW_KEY_ENTER].down )? true : false;
				if ( ImGui::IsMouseDown(0) || vk_enter )
				{
					
					if ( (m_transformation_input_state && !focused_active_item && !m_transformation_input_state_activating) || vk_enter )
					{
						int r_id = m_transformation_input_i;
						Restriction*R = 0;
						for (Restriction*R_ : Mediator::Request_Scene_Restrictions())
						{
							if (r_id == R_->get_Unique_Id())
							{
								R = R_;
								break;
							}
						}
						if (R != 0)
						{
							// action here 
							Action * ac = new Action_Edit_RESTRICTION(R, optix::make_float2(m_transformation_input_data.delta_length), m_transformation_input_attr_index);
							ActionManager::GetInstance().push( ac );
							//
						}

						// reset data //
						m_transformation_input_data.reset();
						m_transformation_input_i = -1;
						m_transformation_input_state = false;
						m_transformation_input_state_activating = false;
					}
				}

				// m0_released
				if ( ImGui::IsMouseReleased(0) && !m_transformation_input_state )
				{

					if ( m_transformation_input_m0_down  )
					{
						int r_id = m_transformation_input_i;
						Restriction*R = 0;
						for (Restriction*R_ : Mediator::Request_Scene_Restrictions())
						{
							if (r_id == R_->get_Unique_Id())
							{
								R = R_;
								break;
							}
						}

						if (R != 0)
						{
							// action here 
							Action * ac = new Action_Edit_RESTRICTION( R, optix::make_float2(m_transformation_input_data.delta_length), m_transformation_input_attr_index );
							ActionManager::GetInstance().push( ac );
							//
						}

					}
					

					// reset data //
					m_transformation_input_data.reset();
					m_transformation_input_i = -1;
					m_transformation_input_state = false;
					m_transformation_input_state_activating = false;
					
				}

#endif
			}


			if (is_changed)
			{
				Restriction* R = m_UI_selected_Restriction;
			}

		}
		ImGui::EndChild();
	}

	// UN- SELECT // UN - FOCUS //
	if ( !is_any_Restriction_focused )
	{
		if (ImGui::IsMouseDown(0) && is_R_list_hovered)
		{
			//	Mediator::Reset_Restrictions_UI_States();
		}
		else
			Mediator::Reset_Restrictions_UI_States( true );
	}
	if (ImGui::IsMouseDown(1) && (is_R_list_hovered || is_any_Restriction_focused) && !is_selected_R_hovered)
	{
		
		Mediator::Reset_Restrictions_UI_States();
		m_UI_selected_Restriction = nullptr;
	}
	
	ImGui::PopFont();
}
void InterfaceManager::construct_Light_Parameters_child_Window()
{
	reset_Tabs_State(LIGHT_PARAMS_TAB);
	process_TAB_SELECTION_STATE_change(LIGHT_PARAMS_TAB);
	m_hierarchy_setting = TRANSFORMATION_SETTING::TO_ALL;


	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
	SceneObject* object = m_selected_object_pack.raw_object;
	optix::Context& m_context = Mediator::RequestContext();
	OptiXShader& shader = Mediator::request_ActiveOptiXShaderId();
	std::string label = "";
	bool is_changed = false;
	bool is_null = object == nullptr;
	if (!is_null)
		is_null = object->getType() != LIGHT_OBJECT ? true : false;

	Light_Parameters light_params;
	if ( !is_null )
		light_params = object->get_LightParameters();

	Light_Type type;
	if (!is_null)
		type = light_params.type;


	bool is_create_light = m_selected_utility.id == u_CREATE_LIGHT;
	ImVec2 edit_size_3 = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 10.0f);
	ImVec2 edit_size_1 = ImVec2(ImGui::GetWindowWidth() - 12, 80.0f + 10.0f);
	ImVec2 edit_size_0 = ImVec2(ImGui::GetWindowWidth() - 12, 40.0f + 10.0f);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	ImGuiWindowFlags window_flags2 = 0;
	window_flags2 |= ImGuiWindowFlags_NoTitleBar;
	window_flags2 |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;


	ImGuiColorEditFlags colorEdit_flags = 0;
	colorEdit_flags |= ImGuiColorEditFlags_NoLabel;
	colorEdit_flags |= ImGuiColorEditFlags_NoInputs;

	{


		bool input_deactive = false;
		bool active_input_index_focused = false;
		optix::float2 m0_down_at = optix::make_float2(-1, -1);
		bool m0_down             = false;
		bool focused_active_item = false;
		bool is_Changed          = false;
		
		m_transformation_input_state_activating = false;

		int index_i = -1;
		int index_j = -1;
		float * v = 0;
		float prev_value = 0.0f;
		ImGuiID ID;
		float delta = 0.0f;
		bool was_INPUT_active = false;
		bool is_INPUT_active  = false;

		if (is_create_light)
		{

			guiStyleManager::setStyle("main_style_right_child3");
			ImGui::BeginChild("Sampler_Params_Child##", ImVec2(ImGui::GetWindowWidth() - 8.0f, 600.0f + edit_size_0.y), true, window_flags);
			{
				ImGui::Dummy(ImVec2(0.0f, 10.0f));


				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("light_type_child", edit_size_3 + ImVec2(0.0f, 20.0f) , true, window_flags);
				{

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::Text(" Light Type ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0.0f, 28.0f));

					ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
					int e = m_interface_state.m_light_params.type;
					if (ImGui::RadioButton(" Quad Light", &e, 1))
					{
						//toggle_Attach_Sampler_Planar();
						m_interface_state.m_light_params.type = QUAD_LIGHT;
					}

					ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
					if (ImGui::RadioButton(" SpotLight", &e, 2))
					{
						//toggle_Attach_Sampler_Volume();
						m_interface_state.m_light_params.type = SPOTLIGHT;
					}

					ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
					if (ImGui::RadioButton(" Spherical Light", &e, 3))
					{
						//toggle_Attach_Sampler_Directional();
						m_interface_state.m_light_params.type = SPHERICAL_LIGHT;
					}

					ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
					if (ImGui::RadioButton(" Point Light", &e, 4))
					{
						//toggle_Attach_Sampler_Directional();
						m_interface_state.m_light_params.type = POINT_LIGHT;
					}

				}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");
				ImGui::Dummy(ImVec2(0.0f, 1.0f));

				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("light_alignement_child", edit_size_3 + ImVec2(0.0f,10.0f)  , true, window_flags);
				{

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::Text(" Alignement ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0.0f, 28.0f));

					ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
					int e = m_interface_state.m_sa;
					if (ImGui::RadioButton(" Origin Plane", &e, 0))
					{
						m_interface_state.m_sa = SA_GLOBAL;
						//m_interface_state.isec_p->reset();
						//m_utility_isec_p.reset();
					}

					ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
					if (ImGui::RadioButton(" Surface Normal Align", &e, 2))
					{
						m_interface_state.m_sa = SA_NORMAL;
						//m_interface_state.isec_p->reset();
						//m_utility_isec_p.reset();
					}

					ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
					if (ImGui::RadioButton(" BBOX Align", &e, 3))
					{
						m_interface_state.m_sa = SA_BBOX;
						//m_interface_state.isec_p->reset();
						//m_utility_isec_p.reset();
					}


					ImGui::Dummy(ImVec2(0, 10));
					ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
					ImGui::Checkbox(" Flip Side", &m_interface_state.flip_side);


				}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");

				ImGui::Dummy(ImVec2(0.0f, 1.0f));


				guiStyleManager::setStyle("main_style_right_child_transformations");
				ImGui::BeginChild("plane_isec_light_child", edit_size_0, true, window_flags);
				{
					ImGui::Dummy(ImVec2(0.0f, 10.0f));
					ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
					if (ImGui::Checkbox("Link To Object", &m_interface_state.link_to_object))
					{
						//m_interface_state.link_to_object = link_to_object;
					}

				}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");

				ImGui::Dummy(ImVec2(0.0f, 1.0f));


			}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child2");

			ImGui::Dummy(ImVec2(0.0f, 1.0f));

			guiStyleManager::setStyle("main_style_right_child3");



			ImGui::BeginChild("Sampler_Params_Child2223##", ImVec2(ImGui::GetWindowWidth() - 8.0f, ImGui::GetWindowHeight() - 20.0f - 600.0f - 2.0f - edit_size_0.y), true, window_flags);
			{



			}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child2");


		}
		else
		{



			float MIN = 0.0f;
			float MAX = 999999.0f;
			float SP = 0.1f;

			float min = MIN;
			float max = MAX;
			float speed = SP;

			std::string child_labels[3] = { " Translation_Child##", "Rotation_Child##", "Scale_Child##" };
			std::string trs_labels[3] = { " Translation", " Rotation", " Scale" };

			std::string axis_labels[3] = { "R##", "G##", "B##" };
			ImVec2 materials_edit_size = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 10.0f);
			ImVec2 materials_edit_size2 = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 40.0f);
			ImVec2 materials_edit_size_1f = ImVec2(ImGui::GetWindowWidth() - 12, 80.0f + 10.0f);

			// 
			guiStyleManager::setStyle("main_style_right_child2");
			ImGui::BeginChild("child_##", ImVec2(ImGui::GetWindowWidth() - 4.0f, ImGui::GetWindowHeight() - 40.0f), true, window_flags);
			{
				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				// Type
				{

					std::string l_type;
					std::string l_types[4] = { "QUAD LIGHT", "SPHERICAL LIGHT", "POINT LIGHT", "SPOTLIGHT" };
					if (!is_null)
					{
						if (type == QUAD_LIGHT)
							l_type = l_types[0];
						else if (type == SPHERICAL_LIGHT)
							l_type = l_types[1];
						else if (type == POINT_LIGHT)
							l_type = l_types[2];
						else if (type == SPOTLIGHT)
							l_type = l_types[3];
					}
					else
						l_type = " ";

					
					{
						guiStyleManager::setStyle("main_style_right_child_transformations");
						ImGui::BeginChild("light_type##", ImVec2(ImGui::GetWindowWidth() - 8, 40.0f + 10.0f), true, window_flags2);

						ImGui::Dummy(ImVec2(0, 13.0f));
						guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Type: ");
						ImGui::PopFont();

						ImGui::SameLine();
						ImGui::Dummy(ImVec2(4.0f, 0.0f));
						ImGui::SameLine();
						ImGui::PushItemWidth(100.0f);

						ImGui::Text(l_type.c_str());

						ImGui::PopItemWidth();

						ImGui::Dummy(ImVec2(2, 5));
						ImGui::EndChild();
					}

				}


				// Cast Shadows ( drag Int )
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("Is_On##", ImVec2(ImGui::GetWindowWidth() - 8, 40.0f + 10.0f), true, window_flags2);

					ImGui::Dummy(ImVec2(0, 13.0f));
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5 - 60.0f, 10.0f)); ImGui::SameLine();
					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Enabled: ");
					ImGui::PopFont();

					ImGui::SameLine();
					ImGui::Dummy(ImVec2(1.0f , 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);

					
					// toggle button //
					{
						ImGuiStyle& style = ImGui::GetStyle();
						ImVec4 tcol  = style.Colors[ImGuiCol_Text];
						ImVec4 ch_bg = style.Colors[ImGuiCol_ChildBg];
						ImVec4 bcol  = style.Colors[ImGuiCol_Button];
						ImVec4 bcolh = style.Colors[ImGuiCol_ButtonHovered];// = ch_bg;
						ImVec4 bcola = style.Colors[ImGuiCol_ButtonActive]; //= ch_bg;

						float border = style.Colors[ImGuiCol_Border].w;
						
						style.Colors[ImGuiCol_Button] = ch_bg;
						style.Colors[ImGuiCol_ButtonHovered] = ch_bg;
						style.Colors[ImGuiCol_ButtonActive] = ch_bg;
						style.Colors[ImGuiCol_Border].w = 0.0f;

						ImVec2 pad_off = ImVec2(-2.0f, -2.0f);
						style.FramePadding += pad_off;

						ImVec2 button_offset = ImVec2(0, 0);
						ImGui::SameLine();
						ImGui::Dummy(button_offset);
						std::string toggle_but_label = light_params.is_on ? ICON_MDI_TOGGLE_SWITCH : ICON_MDI_TOGGLE_SWITCH_OFF;
						toggle_but_label += std::string("##");
						ImGui::SameLine();
						style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
						guiStyleManager::setIconFont(MDI, 20);

						if (!light_params.is_on)
						{
							style.Colors[ImGuiCol_Text] = ImVec4(0.65,0.65,0.65,1);
						}
						if (ImGui::Button(toggle_but_label.c_str()))
						{
							light_params.is_on = !light_params.is_on;
							optix::Context context = Mediator::RequestContext();
							bool is_shadow = m_context["cast_shadow_ray"]->getInt();
							if (is_shadow)
								Mediator::restartAccumulation();
							is_Changed = true;
						}
						ImGui::PopFont();

						style.Colors[ImGuiCol_Text] = tcol;
						style.Colors[ImGuiCol_Button] = bcol;
						style.Colors[ImGuiCol_ButtonHovered] = bcolh;
						style.Colors[ImGuiCol_ButtonActive] = bcola;
						style.Colors[ImGuiCol_Border].w = border;
						style.FramePadding -= pad_off;
					}

					ImGui::PopItemWidth();

					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}


				// Emision 
				{
					float * var = ((float*)&light_params.emission_color);
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("emission_window##", materials_edit_size2, true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
					ImGui::Text(" Emission ");

					ImGui::SameLine();
					ImGui::GetStyle().FrameRounding -= 3;
					ImGui::GetStyle().FramePadding -= ImVec2(1.0f, 1.0f);
					if (ImGui::ColorEdit3("##", var, colorEdit_flags))
						is_Changed = true;
					ImGui::GetStyle().FrameRounding += 3;
					ImGui::GetStyle().FramePadding += ImVec2(1.0f, 1.0f);
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);
					for (int j = 0; j < 3; j++)
					{
						//ImGui::Dummy(ImVec2(0.0f, 0.0f));
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
						ImGui::SameLine();
						ImGui::PushItemWidth(100.0f);
						float old_v = var[j];
						if (ImGui::DragFloat(axis_labels[j].c_str(), &var[j], 0.001f, 0.0f, 1.0f))
						{
							is_Changed = true;
							m_dragFloat_state = true;
							if (m_repositioning_cursor) var[j] = old_v;
						}
						
						ImGui::PopItemWidth();
					}

					ImGui::PushItemWidth(100.0f);
					ImGui::Dummy(ImVec2(0, 20.0f));
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					float * energy = &light_params.radiant_exitance;
					float old_v = *energy;
					if (ImGui::DragFloat("Rdnt. Ext.", energy, 1.0f, 0.0f, 99999999.9f))
					{
						is_Changed = true;
						m_dragFloat_state = true;
					}
					if (m_repositioning_cursor) *energy = old_v;
					ImGui::PopItemWidth();

					ImGui::PopFont();

					ImGui::EndChild();
					guiStyleManager::setStyle("main_style_right_child2");
					//ImGui::Dummy(ImVec2(2, 0));
				}


				// Sizes : ( width , length , height , radius , etc.. )
				{
					float a = type == SPOTLIGHT ? 2.0f : 1.0f;

					light_params.du *= a;
					light_params.dv *= a;

					float * width  = &light_params.du;
					float * length = &light_params.dv;
					float * radius = &light_params.radius;

					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("Light_Sizes## ", materials_edit_size + ImVec2(0.0f, 30.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Dimensions ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);


					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					
					if (type == QUAD_LIGHT || type == SPOTLIGHT)
					{
						
						ImGui::PushItemWidth(100.0f);

						// width
#define Light_Width_Var
#ifdef Light_Width_Var

						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
						ImGui::SameLine();
						
						//
						v          = width;
						prev_value = *v;
						index_i = 0;
						index_j = -1;

						//
						ImGui::DragFloat(" Width ", v, speed, min, max);
						ID = ImGui::GetCurrentWindow()->GetID(" Width ");
						was_INPUT_active = m_transformation_input_state;
						is_INPUT_active  = ImGui::TempInputTextIsActive(ID);
						if (m_repositioning_cursor) { *v = prev_value; }
						delta = *v - prev_value;
						if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
						{
							////std::cout << " - focused_active_item : " << index_i << std::endl;
							focused_active_item = true;
						}
						if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
						{

							bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
							if (activating_input_state && !m_transformation_input_state)
							{
								m_transformation_input_state            = true;
								m_transformation_input_state_activating = true;
							}
							m_transformation_input_attr_index = index_j;
							m_transformation_input_i = index_i;
							m_transformation_input_m0_down    = true;
							m_transformation_input_m0_release = false;
							m_transformation_input_data.starting_delta = prev_value;
						}
						if (delta != 0 && !is_null)
						{
							optix::float3 obj_attributes  = GLOBAL_AXIS[index_i] * prev_value;
							optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
							optix::float3 delta_3f        = curr_attributes - obj_attributes;
							m_transformation_input_data.delta        = delta_3f;
							m_transformation_input_data.delta_length = delta;
							
							if (delta != 0) m_dragFloat_state = true;
							is_Changed = true;
						}
						if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
						{
							m0_down_at = optix::make_float2(index_i, index_j);
							m0_down = true;
						}
						

#endif


						// length
#define Light_Length_Var
#ifdef Light_Length_Var

						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
						ImGui::SameLine();
						
						//
						v          = length;
						prev_value = *v;
						index_i = 1;
						index_j = -1;

						//
						ImGui::DragFloat(" Length ", v, speed, min, max);
						ID = ImGui::GetCurrentWindow()->GetID(" Length ");
						was_INPUT_active = m_transformation_input_state;
						is_INPUT_active  = ImGui::TempInputTextIsActive(ID);
						if (m_repositioning_cursor) { *v = prev_value; }
						delta = *v - prev_value;
						if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
						{
							focused_active_item = true;
						}
						if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
						{
							bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
							if (activating_input_state && !m_transformation_input_state)
							{
								m_transformation_input_state = true;
								m_transformation_input_state_activating = true;
							}
							m_transformation_input_attr_index = index_j;
							m_transformation_input_i = index_i;
							m_transformation_input_m0_down = true;
							m_transformation_input_m0_release = false;
							m_transformation_input_data.starting_delta = prev_value;
						}
						if (delta != 0 && !is_null)
						{
							optix::float3 obj_attributes  = GLOBAL_AXIS[index_i] * prev_value;
							optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
							optix::float3 delta_3f = curr_attributes - obj_attributes;
							m_transformation_input_data.delta = delta_3f;
							m_transformation_input_data.delta_length = delta;

							if (delta != 0) m_dragFloat_state = true;
							is_Changed = true;
						}
						if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
						{
							m0_down_at = optix::make_float2(index_i, index_j);
							m0_down = true;
						}
						delta = 0.0f;
#endif


						ImGui::PopItemWidth();
						
					}

					if (type == SPHERICAL_LIGHT)
					{
						
						// Radius
						ImGui::Dummy(ImVec2(0.0f, 20.0f));
						ImGui::PushItemWidth(100.0f);


#define Light_Radius_Var
#ifdef Light_Radius_Var
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
						ImGui::SameLine();
						
						//
						v          = radius;
						prev_value = *v;
						index_i = 2; index_j = -1;

						//
						ImGui::DragFloat(" Radius ", v, speed, min, max);
						ID = ImGui::GetCurrentWindow()->GetID(" Radius ");
						was_INPUT_active = m_transformation_input_state;
						is_INPUT_active  = ImGui::TempInputTextIsActive(ID);
						if (m_repositioning_cursor) { *v = prev_value; }
						delta = *v - prev_value;
						if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
						{
							focused_active_item = true;
						}
						if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
						{
							bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
							if (activating_input_state && !m_transformation_input_state)
							{
								m_transformation_input_state = true;
								m_transformation_input_state_activating = true;
							}
							m_transformation_input_attr_index = index_j;
							m_transformation_input_i = index_i;
							m_transformation_input_m0_down = true;
							m_transformation_input_m0_release = false;
							m_transformation_input_data.starting_delta = prev_value;
						}
						if (delta != 0 && !is_null)
						{
							optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
							optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
							optix::float3 delta_3f = curr_attributes - obj_attributes;
							m_transformation_input_data.delta = delta_3f;
							m_transformation_input_data.delta_length = delta;

							if (delta != 0) m_dragFloat_state = true;
							is_Changed = true;
						}
						if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
						{
							m0_down_at = optix::make_float2(index_i, index_j);
							m0_down = true;
						}
#endif
						//

						ImGui::PopItemWidth();
					}

					
					ImGui::Dummy(ImVec2(2, 5));
					ImGui::PopFont();
					ImGui::EndChild();
				}


				// Cone Dimensions
				if(type == SPOTLIGHT)
				{
					float * cone_size = (float*)&light_params.cone_size;

					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("Cone_Dimensions## ", materials_edit_size + ImVec2(0.0f,100.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Cone Dimensions ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::PushItemWidth(100.0f);

#define Light_ConeDimX_Var
#ifdef Light_ConeDimX_Var

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();

					//
					v = &cone_size[0];
					prev_value = *v;
					index_i = 3; index_j = -1;

					//
					ImGui::DragFloat(" Umbra ", v, speed, min, max);
					ID = ImGui::GetCurrentWindow()->GetID(" Umbra ");
					was_INPUT_active = m_transformation_input_state;
					is_INPUT_active = ImGui::TempInputTextIsActive(ID);
					if (m_repositioning_cursor) { *v = prev_value; }
					delta = *v - prev_value;
					if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
					{
						focused_active_item = true;
					}
					if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
					{
						bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
						if (activating_input_state && !m_transformation_input_state)
						{
							m_transformation_input_state = true;
							m_transformation_input_state_activating = true;
						}
						m_transformation_input_attr_index = index_j;
						m_transformation_input_i = index_i;
						m_transformation_input_m0_down = true;
						m_transformation_input_m0_release = false;
						m_transformation_input_data.starting_delta = prev_value;
					}
					if (delta != 0 && !is_null)
					{
						optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
						optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
						optix::float3 delta_3f = curr_attributes - obj_attributes;
						m_transformation_input_data.delta = delta_3f;
						m_transformation_input_data.delta_length = delta;

						if (delta != 0) m_dragFloat_state = true;
						is_Changed = true;
					}
					if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
					{
						m0_down_at = optix::make_float2(index_i, index_j);
						m0_down = true;
					}

#endif

				
#define Light_ConeDimY_Var
#ifdef Light_ConeDimY_Var

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();

					//
					v = &cone_size[1];
					prev_value = *v;
					index_i = 4; index_j = -1;

					//
					ImGui::DragFloat(" Penumbra ", v, speed, min, max);
					ID = ImGui::GetCurrentWindow()->GetID(" Penumbra ");
					was_INPUT_active = m_transformation_input_state;
					is_INPUT_active = ImGui::TempInputTextIsActive(ID);
					if (m_repositioning_cursor) { *v = prev_value; }
					delta = *v - prev_value;
					if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
					{
						focused_active_item = true;
					}
					if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
					{
						bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
						if (activating_input_state && !m_transformation_input_state)
						{
							m_transformation_input_state = true;
							m_transformation_input_state_activating = true;
						}
						m_transformation_input_attr_index = index_j;
						m_transformation_input_i = index_i;
						m_transformation_input_m0_down = true;
						m_transformation_input_m0_release = false;
						m_transformation_input_data.starting_delta = prev_value;
					}
					if (delta != 0 && !is_null)
					{
						optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
						optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
						optix::float3 delta_3f = curr_attributes - obj_attributes;
						m_transformation_input_data.delta = delta_3f;
						m_transformation_input_data.delta_length = delta;

						if (delta != 0) m_dragFloat_state = true;
						is_Changed = true;
					}
					if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
					{
						m0_down_at = optix::make_float2(index_i, index_j);
						m0_down = true;
					}

#endif

					ImGui::PopItemWidth();


					

					//
					ImGui::Dummy(ImVec2(0.0f, 25.0f));
					ImGui::Dummy(ImVec2(12.0f, 0.0f));
					ImGui::SameLine();
					// is soft
					{
						guiStyleManager::setStyle("main_style_right_child_transformations");
						ImGui::BeginChild("Is_Soft##", ImVec2(ImGui::GetWindowWidth() - 8 - 30 , 40.0f + 60.0f), true, window_flags2);

						guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14); ImGui::Dummy(ImVec2(40.0f, 0.0)); ImGui::SameLine(); ImGui::Text(" Interpolation Function: ");
						ImGui::PopFont();
						ImGui::Dummy(ImVec2(0.0f, 20.0f));

						int e = light_params.is_soft ? 1 : 0;

						ImGui::PushItemWidth(100.0f);
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 70.0f, 0.0f));
						ImGui::SameLine();
						
						if (ImGui::RadioButton("Linear Interpolation", &e, 0))
						{
							is_Changed = true;
							light_params.is_soft = 0;
						}
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 70.0f, 0.0f));
						ImGui::SameLine();
						if (ImGui::RadioButton("Cubic Interpolation", &e, 1))
						{
							is_Changed = true;
							light_params.is_soft = 1;
						}

						ImGui::PopItemWidth();
						ImGui::Dummy(ImVec2(2, 5));
						ImGui::EndChild();
					}
					//

					
					ImGui::Dummy(ImVec2(2, 5));
					ImGui::PopFont();
					ImGui::EndChild();
				}


				// Frustum Dimensions
				{
					float * cp_size = (float*)&light_params.clipping_plane;

					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("Frustum_Dimensions## ", materials_edit_size + ImVec2(0.0f, 0.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Clipping Plane ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::PushItemWidth(100.0f);

					//

#define Light_ClipPlaneX_Var
#ifdef Light_ClipPlaneX_Var

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();

					//
					v = &cp_size[0];
					prev_value = *v;
					index_i = 5; index_j = -1;

					//
					ImGui::DragFloat(" Near ", v, speed, min, max);
					ID = ImGui::GetCurrentWindow()->GetID(" Near ");
					was_INPUT_active = m_transformation_input_state;
					is_INPUT_active = ImGui::TempInputTextIsActive(ID);
					if (m_repositioning_cursor) { *v = prev_value; }
					delta = *v - prev_value;
					if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
					{
						focused_active_item = true;
					}
					if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
					{
						bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
						if (activating_input_state && !m_transformation_input_state)
						{
							m_transformation_input_state = true;
							m_transformation_input_state_activating = true;
						}
						m_transformation_input_attr_index = index_j;
						m_transformation_input_i = index_i;
						m_transformation_input_m0_down = true;
						m_transformation_input_m0_release = false;
						m_transformation_input_data.starting_delta = prev_value;
					}
					if (delta != 0 && !is_null)
					{
						optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
						optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
						optix::float3 delta_3f = curr_attributes - obj_attributes;
						m_transformation_input_data.delta = delta_3f;
						m_transformation_input_data.delta_length = delta;

						if (delta != 0) m_dragFloat_state = true;
						is_Changed = true;
					}
					if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
					{
						m0_down_at = optix::make_float2(index_i, index_j);
						m0_down = true;
					}

#endif


#define Light_ClipPlaneY_Var
#ifdef Light_ClipPlaneY_Var

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();

					//
					v = &cp_size[1];
					prev_value = *v;
					index_i = 6; index_j = -1;

					//
					ImGui::DragFloat(" Far ", v, speed, min, max);
					ID = ImGui::GetCurrentWindow()->GetID(" Far ");
					was_INPUT_active = m_transformation_input_state;
					is_INPUT_active = ImGui::TempInputTextIsActive(ID);
					if (m_repositioning_cursor) { *v = prev_value; }
					delta = *v - prev_value;
					if (ImGui::IsItemHovered() && (m_transformation_input_i == index_i))
					{
						focused_active_item = true;
					}
					if (ImGui::IsItemActivated() && m_transformation_input_i == -1 && !is_null)
					{
						bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
						if (activating_input_state && !m_transformation_input_state)
						{
							m_transformation_input_state = true;
							m_transformation_input_state_activating = true;
						}
						m_transformation_input_attr_index = index_j;
						m_transformation_input_i = index_i;
						m_transformation_input_m0_down = true;
						m_transformation_input_m0_release = false;
						m_transformation_input_data.starting_delta = prev_value;
					}
					if (delta != 0 && !is_null)
					{
						optix::float3 obj_attributes = GLOBAL_AXIS[index_i] * prev_value;
						optix::float3 curr_attributes = GLOBAL_AXIS[index_i] * *v;
						optix::float3 delta_3f = curr_attributes - obj_attributes;
						m_transformation_input_data.delta = delta_3f;
						m_transformation_input_data.delta_length = delta;

						if (delta != 0) m_dragFloat_state = true;
						is_Changed = true;
					}
					if (ImGui::IsMouseDown(0) && ImGui::IsItemHovered() && !is_null)
					{
						m0_down_at = optix::make_float2(index_i, index_j);
						m0_down = true;
					}

#endif

					//
					ImGui::PopItemWidth();



					//
					ImGui::Dummy(ImVec2(0.0f, 25.0f));
					ImGui::Dummy(ImVec2(12.0f, 0.0f));
					ImGui::SameLine();

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14); ImGui::Text(" Visualize Distance Clipping:  "); ImGui::PopFont();
					ImGui::SameLine();
					if (ImGui::Checkbox("##", &light_params.is_clip))
					{
						optix::Context context = Mediator::RequestContext();
						bool is_shadow = m_context["cast_shadow_ray"]->getInt();
						if (is_shadow)
							Mediator::restartAccumulation();
						is_Changed = true;
					}


					
					ImGui::Dummy(ImVec2(2, 5));
					ImGui::PopFont();
					ImGui::EndChild();
				}

			

			}
			ImGui::EndChild();



			if (!is_null)
			{
				bool vk_key  = key_data[GLFW_KEY_ENTER].down || (key_data[GLFW_KEY_TAB].down && !m_transformation_input_state_activating);
				bool m0_DOWN = ImGui::IsMouseClicked(0);
				bool m0_REL  = ImGui::IsMouseReleased(0);
				bool save_ac = 0;
				bool reset_state_1 = 0;
				bool reset_state_2 = 0;
				bool reset_state   = 0;
				bool input_state   = 0;

				bool dragging_state_end = (m_transformation_input_m0_down  && !m_transformation_input_state_activating) || m_plus_minus_button_state;
				bool input_state_end    = (m0_DOWN && !focused_active_item && !m_transformation_input_state_activating) || vk_key;

				int trs_i = m_transformation_input_i;
				int ind_j = m_transformation_input_attr_index;

				float delta = m_transformation_input_data.delta_length;
				optix::float3 delta_3f = GLOBAL_AXIS[ind_j] * delta;
				optix::float3 working_axis[3] = { GLOBAL_AXIS[0], GLOBAL_AXIS[1], GLOBAL_AXIS[2] };
				BASE_TRANSFORM::AXIS(object, LOCAL, working_axis);

				float x = type == SPOTLIGHT ? 0.5f : 1.0f;
				bool is_type_valid = type != SPOTLIGHT && type != POINT_LIGHT;


				// Width Var
				if (trs_i == 0)
				{
					delta_3f = optix::make_float3(delta, 0.0f, 0.0f) * x;
				}
				// Length Var
				else if (trs_i == 1)
				{
					delta_3f = optix::make_float3(0.0f, 0.0f, delta) * x;
				}
				// Radius Var
				else if (trs_i == 2)
				{
					delta_3f = optix::make_float3(delta) * 1.0f;
				}
				// ConeDim Var
				else if (trs_i == 3 || trs_i == 4)
				{
					if (trs_i == 3) delta_3f = optix::make_float3(delta, 0.0f,0.0f);
					else            delta_3f = optix::make_float3(0.0f, delta, 0.0f);
					
				}
				// ClipPlanes Var
				else if (trs_i == 5 || trs_i == 6)
				{
					if (trs_i == 5) delta_3f = optix::make_float3(delta, 0.0f, 0.0f);
					else            delta_3f = optix::make_float3(0.0f, delta, 0.0f);

				}

				
				if (type == SPOTLIGHT)
				{
					light_params.du *= 0.5f;
					light_params.dv *= 0.5f;
				}

				////std::cout << "\n";
				////std::cout << " - m_input_state : " << m_transformation_input_state << std::endl;
				////std::cout << " - trs_i : " << trs_i << std::endl;
				////std::cout << " - delta : " << delta << std::endl;
				////std::cout << " - delta_3f = [ " << delta_3f.x << " , " << delta_3f.y << " , " << delta_3f.z << " ] " << std::endl;

				if (is_Changed)
				{
					// apply Transformation here ...
					if ( is_type_valid && (trs_i == 0 || trs_i == 1 || trs_i == 2) )
					{
						object->setTransformation(delta_3f,
												  AXIS_ORIENTATION::LOCAL,
												  SCALE,
												  ONLY_THIS,
												  0,
												  working_axis);

					}

					// update Light's Vars
					object->set_LightParameters( light_params, true );
					

					// accumulate transformation delta
					m_transformation_input_data.overall_delta += delta_3f;
				}
				else
				{
					delta_3f = ZERO_3f;
					delta    = 0.0f;
					m_transformation_input_data.delta_length = 0.0f;
					m_transformation_input_data.delta = ZERO_3f;
				}

				// transformation input active
				if (m_transformation_input_state)
				{
					if (input_state_end)
					{
						reset_state_1 = 1;
						input_state   = 1;
					}
				}
				else
				{
					if (m0_REL)
					{
						if (dragging_state_end)
						{
							
							reset_state_2 = 1;
							input_state   = 0;
						}

					}
				}

				reset_state = reset_state_1 || reset_state_2;
				if (reset_state)
				{
					////std::cout << "\n ~ RESET_STATE: " << std::endl;
					optix::float3 overall_delta = m_transformation_input_data.overall_delta;
					float delta_length          = length(overall_delta);
					save_ac = fabsf(delta_length) > 1e-5f ? 1 : 0;
					if ( save_ac && is_type_valid && (trs_i == 0 || trs_i == 1 || trs_i == 2) )
					{
						// pushing Action ... 
						Action * ac     = 0;
						ACTION_ID ac_id = ACTION_TRANSFORMATION_SCALE;

						ac = new Action_TRANSFORMATION(
							object, 
							overall_delta,
							working_axis, 
							LOCAL, 
							ONLY_THIS, 
							ac_id);

						ActionManager::GetInstance().push(ac);
					}
					else if (trs_i == 3 || trs_i == 4)
					{
						Action * ac = new Action_Edit_LIGHT(object, overall_delta, 0);
						ActionManager::GetInstance().push(ac);
					}
					else if (trs_i == 5 || trs_i == 6)
					{
						Action * ac = new Action_Edit_LIGHT(object, overall_delta, 1);
						ActionManager::GetInstance().push(ac);
					}


					// reset data //
					{
						////std::cout << " >> __ resetting_data[0] __ " << std::endl;
						m_transformation_input_data.reset();
						m_transformation_input_i          = -1;
						m_transformation_input_attr_index = -1;
						m_transformation_input_state      = false;
						m_transformation_input_state_activating = false;
						m_plus_minus_button_state = false;
					}

				}
			}

			if ( (is_Changed || is_changed) 
				&& !is_null
				)
			{
				bool rr_acc  = 0;
				optix::Context context = Mediator::RequestContext();
				bool is_shadow         = m_context["cast_shadow_ray"]->getInt();
				rr_acc = is_shadow && light_params.is_on;
				if( rr_acc ) 
					Mediator::restartAccumulation();
			}

		}
	}

	ImGui::PopFont();

}
void InterfaceManager::construct_Sampler_Parameters_Child_Window()
{
	reset_Tabs_State( SAMPLER_PARAMS_TAB );
	process_TAB_SELECTION_STATE_change( SAMPLER_PARAMS_TAB );
	m_hierarchy_setting = TRANSFORMATION_SETTING::TO_ALL;


	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

	
	SceneObject* object = m_selected_object_pack.raw_object;
	bool is_null    = (object != nullptr) ? object->getType() != SAMPLER_OBJECT : true;
	bool is_changed = false;
	optix::float3 is_axis = optix::make_float3(0.0f);
	Sampler_Type s_type   = s_NONE;

	Utility ut = m_selected_utility.id;
	bool is_attach_sampler = (ut == u_ATTACH_SAMPLER_PLANAR) || (ut == u_ATTACH_SAMPLER_DIRECTIONAL) || (ut == u_ATTACH_SAMPLER_VOLUME);

	
	bool mouse_down = false;
	bool m0_down    = false;
	bool m0_down_on_activated_item = false;
	bool focused_active_item       = false;
	m_transformation_input_state_activating = false;

	int index_i = -1;
	int index_j = -1;
	float prev_value = 0.0f;
	ImGuiID ID;
	float delta = 0.0f;
	bool was_INPUT_active = false;
	bool is_INPUT_active = false;

	ImVec2 edit_size_3 = ImVec2(ImGui::GetWindowWidth() - 12, 125.0f + 10.0f);
	ImVec2 edit_size_1 = ImVec2(ImGui::GetWindowWidth() - 12, 80.0f + 10.0f);
	ImVec2 edit_size_0 = ImVec2(ImGui::GetWindowWidth() - 12, 40.0f + 10.0f);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	ImGuiWindowFlags window_flags2 = 0;
	window_flags2 |= ImGuiWindowFlags_NoTitleBar;
	window_flags2 |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags2 |= ImGuiWindowFlags_NoScrollbar;
	window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;
	
	guiStyleManager::setStyle("main_style_right_child2");
	
	if (is_attach_sampler)
	{
		
		guiStyleManager::setStyle("main_style_right_child3");
		ImGui::BeginChild("Sampler_Params_Child##", ImVec2(ImGui::GetWindowWidth() - 8.0f, 600.0f + edit_size_0.y ), true, window_flags);
		{
			ImGui::Dummy(ImVec2(0.0f, 10.0f));


			guiStyleManager::setStyle("main_style_right_child_transformations");
			ImGui::BeginChild("sampler_type_child", edit_size_3, true, window_flags);
			{

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::Text(" Sampler Type ");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0.0f, 28.0f));

				ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
				int e = m_sampler_crt_def_type;
				if (ImGui::RadioButton(" Planar", &e, 1))
				{
					toggle_Attach_Sampler_Planar();
					m_sampler_crt_def_type = 1;
				}

				ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
				if (ImGui::RadioButton(" Volume", &e, 2))
				{
					toggle_Attach_Sampler_Volume();
					m_sampler_crt_def_type = 2;
				}

				ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
				if (ImGui::RadioButton(" Directional", &e, 3))
				{
					toggle_Attach_Sampler_Directional();
					m_sampler_crt_def_type = 3;
				}

			}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			
			guiStyleManager::setStyle("main_style_right_child_transformations"); 
			ImGui::BeginChild("sampler_alignement_child", edit_size_3 , true, window_flags);
			{


				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::Text(" Alignement ");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0.0f, 28.0f));

				ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
				int e = m_interface_state.m_sa;
				if (ImGui::RadioButton(" Global Axis", &e, 0))
				{
					m_interface_state.m_sa = SA_GLOBAL;
				}

				ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
				if (ImGui::RadioButton(" Local Axis", &e, 1))
				{
					m_interface_state.m_sa = SA_LOCAL;
				}

				ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
				if (ImGui::RadioButton(" Surface Normal Align", &e, 2))
				{
					m_interface_state.m_sa = SA_NORMAL;
				}

			}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");
			
			ImGui::Dummy(ImVec2(0.0f, 1.0f));


			guiStyleManager::setStyle("main_style_right_child_transformations");
			ImGui::BeginChild("sampler_bbox_align_child", edit_size_0, true, window_flags);
			{
				ImGui::Dummy(ImVec2(0.0f, 10.0f));
				ImGui::Dummy(ImVec2(20.0f, 0.0f));ImGui::SameLine();
				if (ImGui::Checkbox("Place on BBOX", &m_sampler_init_isec_object_bbox))
				{
					m_interface_state.m_sampler_init_use_bbox = m_sampler_init_isec_object_bbox;
				}

			}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");

			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			

		}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child2");

		ImGui::Dummy(ImVec2(0.0f, 1.0f));

		guiStyleManager::setStyle("main_style_right_child3");
		
		
		
		ImGui::BeginChild("Sampler_Params_Child2223", ImVec2(ImGui::GetWindowWidth() - 8.0f, ImGui::GetWindowHeight() - 20.0f - 600.0f - 2.0f - edit_size_0.y), true, window_flags);
		{



		}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child2");
	}
	else
	{
		guiStyleManager::setStyle("main_style_right_child3");
		ImGui::BeginChild("Sampler_Params_Child##", ImVec2(ImGui::GetWindowWidth() - 8.0f, ImGui::GetWindowHeight() - 40.0f), true, window_flags);
		{

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			guiStyleManager::setStyle("main_style_right_child_transformations");
			ImGui::BeginChild("sampler_type_child", edit_size_3, true, window_flags);
			{

				guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
				ImGui::Text(" Dimensions ");
				ImGui::PopFont();

				ImGui::Dummy(ImVec2(0.0f, 30.0f));
				float v = 0.0f;

				float length = 0.0f;
				float width  = 0.0f;
				float height = 0.0f;
				optix::float2 theta = optix::make_float2(0.0f);

				float prev_length = 0.0f;
				float prev_width  = 0.0f;
				float prev_height = 0.0f;
				optix::float2 prev_theta = optix::make_float2(0.0f);

				//if (!is_null)
				{
					Sampler_Parameters params = is_null? Sampler_Parameters() : object->get_SamplerParameters();
					//if (is_null) params.type  = s_VOLUME;
					s_type = params.type;
					
					optix::float3 scale = is_null? ZERO_3f : object->getScaleAttributes();
					
					float a = s_type == s_PLANE ? 1.0f : 2.0f;
					length  = scale.x * 5.0f * a;
					width   = scale.z * 5.0f * a;
					height  = scale.y * 5.0f * a;

					length = fabsf(length);
					width  = fabsf(width);
					height = fabsf(height);

					theta.x = params.cone_size.x;
					theta.y = params.cone_size.y;

					prev_length = length;
					prev_width  = width;
					prev_height = height;
					prev_theta  = theta;
				}

				ImGui::PushItemWidth(100.0f);
				if      (s_type == s_PLANE || s_type == s_VOLUME)
				{

					// length param
					float v    = length;
					prev_value = v;
					index_i = 0;
					index_j = -1;

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5f - 65, 0.0f));ImGui::SameLine();
					ImGui::DragFloat("Length##_samplers", &v, 0.2f, 0.0f,0.0f);

					ID = ImGui::GetCurrentWindow()->GetID("Length##_samplers");
					was_INPUT_active = m_transformation_input_state;
					is_INPUT_active = ImGui::TempInputTextIsActive(ID);
					if (m_repositioning_cursor) v = prev_value;
					delta = v - prev_value;
					if (ImGui::IsItemHovered() && m_transformation_input_i == index_i)
					{
						focused_active_item = true;
					}
					if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
					{
						bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
						if (activating_input_state && !m_transformation_input_state)
						{
							m_transformation_input_state = true;
							m_transformation_input_state_activating = true;
						}

						m_transformation_input_m0_down = true;
						m_transformation_input_m0_release = false;
						m_transformation_input_data.starting_delta = prev_value;
						
						m_transformation_input_i = index_i;
						m_transformation_input_attr_index = index_j;
					}
					if (delta != 0)
					{
						if (delta != 0)
							m_dragFloat_state = true;

						m_transformation_input_data.delta_length = delta;
						is_changed = true;
						is_axis.x  = 1;
					}
					

					// width param
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5f - 65, 0.0f));ImGui::SameLine();

					v = width;
					prev_value = v;
					index_i = 2;
					index_j = -1;

					ImGui::DragFloat("Width##_samplers", &v, 0.2f, 0.0f, 0.0f);
					ID = ImGui::GetCurrentWindow()->GetID("Width##_samplers");
					was_INPUT_active = m_transformation_input_state;
					is_INPUT_active = ImGui::TempInputTextIsActive(ID);
					if (m_repositioning_cursor) v = prev_value;
					delta = v - prev_value;
					if (ImGui::IsItemHovered() && m_transformation_input_i == index_i)
					{
						focused_active_item = true;
					}
					if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
					{

						bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
						if (activating_input_state && !m_transformation_input_state)
						{
							m_transformation_input_state = true;
							m_transformation_input_state_activating = true;
						}

						m_transformation_input_m0_down = true;
						m_transformation_input_m0_release = false;
						m_transformation_input_data.starting_delta = prev_value;

						m_transformation_input_i = index_i;
						m_transformation_input_attr_index = index_j;
					}
					if ( delta != 0)
					{
						if (delta != 0)
							m_dragFloat_state = true;

						m_transformation_input_data.delta_length = delta;
						is_changed = true;
						is_axis.z = 1;
					}
					
					
					if (s_type == s_VOLUME)
					{
						
						// height param
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5f - 65, 0.0f));ImGui::SameLine();

						v = height;
						prev_value = v;
						index_i = 1;
						index_j = -1;


						ImGui::DragFloat("Height##_samplers", &v, 0.2f, 0.0f, 0.0f);
						ID = ImGui::GetCurrentWindow()->GetID("Height##_samplers");
						was_INPUT_active = m_transformation_input_state;
						is_INPUT_active  = ImGui::TempInputTextIsActive(ID);
						if (m_repositioning_cursor) v = prev_value;
						delta = v - prev_value;
						if (ImGui::IsItemHovered() && m_transformation_input_i == index_i)
						{
							focused_active_item = true;
						}
						if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
						{

							bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
							if (activating_input_state && !m_transformation_input_state)
							{
								m_transformation_input_state = true;
								m_transformation_input_state_activating = true;
							}

							m_transformation_input_m0_down = true;
							m_transformation_input_m0_release = false;
							m_transformation_input_data.starting_delta = prev_value;

							m_transformation_input_i = index_i;
							m_transformation_input_attr_index = index_j;

						}
						if (delta != 0)
						{
							if (delta != 0)
								m_dragFloat_state = true;

							m_transformation_input_data.delta_length = delta;
							is_changed = true;
							is_axis.y = 1;
						}

					}
				}
				else if (s_type == s_DIRECTIONAL)
				{
				    ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5f - 65, 0.0f));ImGui::SameLine();

					float v = theta.x;
					prev_value = v;
					index_i = 3;
					index_j = -1;


					ImGui::DragFloat("Angle##_samplers", &v, 0.05f, 0.0f, 89.9f);
					ID = ImGui::GetCurrentWindow()->GetID("Angle##_samplers");
					was_INPUT_active = m_transformation_input_state;
					is_INPUT_active = ImGui::TempInputTextIsActive(ID);
					if (m_repositioning_cursor) v = prev_value;
					delta = v - prev_value;
					if (ImGui::IsItemHovered() && m_transformation_input_i == index_i)
					{
						focused_active_item = true;
					}
					if (ImGui::IsItemActivated() && m_transformation_input_i == -1)
					{
						bool activating_input_state = ImGui::IsMouseDoubleClicked(0) || is_INPUT_active;
						if (activating_input_state && !m_transformation_input_state)
						{
							m_transformation_input_state = true;
							m_transformation_input_state_activating = true;
						}
						m_transformation_input_m0_down = true;
						m_transformation_input_m0_release = false;
						m_transformation_input_data.starting_delta = prev_value;

						m_transformation_input_i = index_i;
						m_transformation_input_attr_index = index_j;

						// is_changed = true;
						// is_axis.x = 1;
					}
					if (delta != 0)
					{
						if (delta != 0)
							m_dragFloat_state = true;

						m_transformation_input_data.delta_length = delta;
						is_changed = true;
						is_axis.x = 2;
					}
					
				}
			
				ImGui::PopItemWidth();

				if (!is_null)
				{


#define new_action_code2
#ifdef new_action_code2

					bool vk_key = key_data[GLFW_KEY_ENTER].down || (key_data[GLFW_KEY_TAB].down && !m_transformation_input_state_activating);
					bool m0_DOWN = ImGui::IsMouseClicked(0);
					bool m0_REL = ImGui::IsMouseReleased(0);
					bool save_ac = 0;
					bool reset_state_1 = 0;
					bool reset_state_2 = 0;
					bool reset_state = 0;
					bool input_state = 0;

					bool dragging_state_end = (m_transformation_input_m0_down && !m_transformation_input_state_activating);
					bool input_state_end = (m0_DOWN && !focused_active_item && !m_transformation_input_state_activating) || vk_key;

					int ind_i = m_transformation_input_i;
					int ind_j = m_transformation_input_attr_index;

					float delta = m_transformation_input_data.delta_length;
					optix::float3 delta_3f = optix::make_float3(delta);
					optix::float3 working_axis[3] = { GLOBAL_AXIS[0], GLOBAL_AXIS[1], GLOBAL_AXIS[2] };
					BASE_TRANSFORM::AXIS(object, LOCAL, working_axis);


					if (is_changed)
					{

						if ((s_type == s_PLANE || s_type == s_VOLUME))
						{
							optix::float3 scale = object->getScaleAttributes();
							optix::float3 scale_delta = optix::make_float3(0.0f);
							float _length = length;
							float _width  = width;
							float _height = height;

							if      (ind_i == 0) _length += delta;
							else if (ind_i == 1) _height += delta;
							else if (ind_i == 2) _width += delta;


							float a = s_type == s_PLANE ? 0.2 : 0.1f;
							scale_delta.x = _length * a - scale.x;
							scale_delta.y = _height * a - scale.y;
							scale_delta.z = _width  * a - scale.z;

							object->setTransformation(scale_delta * is_axis, LOCAL, SCALE);
						}
						else if (s_type == s_DIRECTIONAL)
						{
							float _theta = theta.x + delta;
							object->get_SamplerParameters_Raw().cone_size = optix::make_float2(_theta);
						}

						// accumulate transformation delta
						m_transformation_input_data.overall_delta += delta_3f;

					}
					else
					{
						delta_3f = ZERO_3f;
						delta = 0.0f;
						m_transformation_input_data.delta_length = 0.0f;
						m_transformation_input_data.delta = ZERO_3f;
					}



					// transformation input active
					if (m_transformation_input_state)
					{
						if (input_state_end)
						{
							reset_state_1 = 1;
							input_state = 1;
						}
					}
					else
					{
						if (m0_REL)
						{
							if (dragging_state_end)
							{

								reset_state_2 = 1;
								input_state = 0;
							}

						}
					}
					

					reset_state = reset_state_1 || reset_state_2;
					if (reset_state)
					{

						optix::float3 overall_delta = m_transformation_input_data.overall_delta;
						float delta_length = optix::length(overall_delta);
						save_ac = fabsf(delta_length) > 1e-5f ? 1 : 0;
						if (save_ac && ind_i != -1)
						{
							optix::float3 delta_3f = overall_delta;
							Action * ac = new Action_Edit_SAMPLER(object, delta_3f, m_transformation_input_i);
							ActionManager::GetInstance().push(ac);
						}


						// reset data //
						{
							////std::cout << " >> __ resetting_data[0] __ " << std::endl;
							m_transformation_input_data.reset();
							m_transformation_input_i = -1;
							m_transformation_input_attr_index = -1;
							m_transformation_input_state = false;
							m_transformation_input_state_activating = false;
							m_plus_minus_button_state = false;
						}
					}


#endif

					//#define old_action_code2
#ifdef old_action_code2
					// case : planar or volumetric sampler -> need to scale accordingly
					// case : directional -> need to just change delta value
					if (is_changed && !is_null)
					{
						if (s_type == s_PLANE || s_type == s_VOLUME)
						{
							optix::float3 scale = object->getScaleAttributes();
							optix::float3 scale_delta = optix::make_float3(0.0f);
							float a = s_type == s_PLANE ? 0.2 : 0.1f;
							scale_delta.x = length * a - scale.x;
							scale_delta.y = height * a - scale.y;
							scale_delta.z = width * a - scale.z;

							////std::cout << "\n - length : " << length << std::endl;
							////std::cout << " - width : " << width << std::endl;
							////std::cout << " - height : " << height << std::endl;
							////std::cout << " - scale_delta : [ " << scale_delta.x << " , " << scale_delta.y << " , " << scale_delta.z << " ] " << std::endl;
							////std::cout << " - is_axis : [ " << is_axis.x << " , " << is_axis.y << " , " << is_axis.z << " ] " << std::endl;
							object->setTransformation(scale_delta * is_axis, LOCAL, SCALE);

						}
						else if (s_type == s_DIRECTIONAL)
						{
							object->get_SamplerParameters_Raw().cone_size = optix::make_float2(theta.x);
						}
					}

					bool vk_enter = key_data[GLFW_KEY_ENTER].down;
					if ((ImGui::IsMouseDown(0) || vk_enter) && !is_null)
					{

						if ((m_transformation_input_state && !focused_active_item && !m_transformation_input_state_activating) || vk_enter)
						{

							float delta = m_transformation_input_data.delta_length;
							if (fabsf(delta) > 1e-4)
							{
								optix::float3 delta_3f = optix::make_float3(delta);
								Action * ac = new Action_Edit_SAMPLER(object, delta_3f, m_transformation_input_i);
								ActionManager::GetInstance().push(ac);
							}

							// reset data //
							m_transformation_input_data.reset();
							m_transformation_input_i = -1;
							m_transformation_input_state = false;
							m_transformation_input_state_activating = false;
						}
					}

					if (ImGui::IsMouseReleased(0) && !m_transformation_input_state || (ImGui::IsMouseReleased(0) && is_null))
					{

						if (m_transformation_input_m0_down && !is_null)
						{
							// pass action here 
							float delta = m_transformation_input_data.delta_length;
							if (fabsf(delta) > 1e-4)
							{
								////std::cout << "   - delta : " << delta << std::endl;
								////std::cout << "   - saving action here!" << std::endl;
								optix::float3 delta_3f = optix::make_float3(delta);
								Action * ac = new Action_Edit_SAMPLER(object, delta_3f, m_transformation_input_i);
								ActionManager::GetInstance().push(ac);
							}
						}


						// reset data //
						m_transformation_input_data.reset();
						m_transformation_input_i = -1;
						m_transformation_input_state = false;
						m_transformation_input_state_activating = false;

					}

#endif

				}


			}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child3");
			ImGui::Dummy(ImVec2(0.0f, 1.0f));

			

		}ImGui::EndChild();guiStyleManager::setStyle("main_style_right_child2");


		
	}

	
	

	ImGui::PopFont();
}

void InterfaceManager::construct_Scene_Render_Settings_Window()
{
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

	reset_Tabs_State(SCENE_SETTINGS_TAB);
	process_TAB_SELECTION_STATE_change(SCENE_SETTINGS_TAB);
	m_hierarchy_setting = TRANSFORMATION_SETTING::TO_ALL;


	optix::Context& m_context = Mediator::RequestContext();
	OptiXShader& shader       = Mediator::request_ActiveOptiXShaderId();
	std::string label         = "";
	bool changed              = false;
	
	{
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

		ImGuiWindowFlags window_flags2 = 0;
		window_flags2 |= ImGuiWindowFlags_NoScrollWithMouse;

		
		guiStyleManager::setStyle("main_style_right_child2");
		ImGui::BeginChild("child_##", ImVec2(ImGui::GetWindowWidth() - 4.0f, ImGui::GetWindowHeight() - 40.0f), true, window_flags);
		{
			ImGui::Dummy(ImVec2(0.0f, 1.0f));


			guiStyleManager::setStyle("main_style_right_child_transformations");
			ImGui::BeginChild("child_render_settings##", ImVec2(ImGui::GetWindowWidth() - 4.0f, 600.0f + 38.0f), true, window_flags2);
			{


				ImGui::Dummy(ImVec2(0, 2));
				guiStyleManager::setFont(ROBOTO, "Bold", 14);
				ImGui::Text(" Renderer Settings:");
				ImGui::PopFont();
				ImGui::Dummy(ImVec2(0, 10));

				// Scene Renderer epsilon 
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("Renderer epsilon##", ImVec2(ImGui::GetWindowWidth() - 14, 80.0f + 10.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Renderer epsilon: ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 14.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::Dummy(ImVec2(0.0f, 2.0f));
					float var = m_context["sys_SceneEpsilon"]->getFloat();
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					if (ImGui::DragFloat("##", &var, 0.001f, 0.0f, 9999999.0f))
					{
						m_dragFloat_state = true;
						m_context["sys_SceneEpsilon"]->setFloat(var);
					}
					ImGui::PopItemWidth();

					ImGui::PopFont();

					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}


				// Min - Max Depth ( drag Float )
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("Ray Depth## ", ImVec2(ImGui::GetWindowWidth() - 14, 80.0f + 10.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Ray Depth: ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					if (ImGui::InputInt(" Min ", &shader.data.min_depth))
					{
						shader.data.min_depth < 0 ? shader.data.min_depth = 0 : shader.data.min_depth;
						m_context["min_depth"]->setUint(shader.data.min_depth);
						changed = true;
					}

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::InputInt(" Max ", &shader.data.max_depth))
					{
						shader.data.max_depth < 1 ? shader.data.max_depth = 1 : shader.data.max_depth;
						m_context["max_depth"]->setUint(shader.data.max_depth);
						changed = true;
					}

					ImGui::PopItemWidth();

					ImGui::PopFont();

					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}


				// Cast Shadows ( drag Int )
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("Custom Lighting##", ImVec2(ImGui::GetWindowWidth() - 14, 80.0f + 10.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Custom Lighting: ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					
					ImGui::PushItemWidth(100.0f);
					int cast_shadow = m_context["cast_shadow_ray"]->getInt();

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::RadioButton("Off##", &cast_shadow, 0))
					{
						m_context["cast_shadow_ray"]->setInt(0);
						changed = true;
					}

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::RadioButton("On##", &cast_shadow, 1))
					{
						m_context["cast_shadow_ray"]->setInt(1);
						changed = true;
					}


					ImGui::PopItemWidth();
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}

			
				// cast Reflections ( drag Int )
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("Reflections##", ImVec2(ImGui::GetWindowWidth() - 14, 80.0f + 120.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Reflections: ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					//ImGui::Dummy(ImVec2(0.0f, 2.0f));
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					int cast_refl = m_context["sys_cast_reflections"]->getInt();
					
					if (ImGui::RadioButton("Off##", &cast_refl, 0))
					{
						m_context["sys_cast_reflections"]->setInt(0);
						changed = true;
					}

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::RadioButton("On##", &cast_refl, 1))
					{
						m_context["sys_cast_reflections"]->setInt(1);
						changed = true;
					}

					ImGui::PopItemWidth();
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 18));
					ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine();
					ImGui::BeginChild("glossy_refl_child##", ImVec2(ImGui::GetWindowWidth() - 50, 90), true, window_flags2);
					{
						
						ImGui::Dummy(ImVec2(65, 0)); ImGui::SameLine();
						guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
						ImGui::Text(" Glossiness:");
						ImGui::PopFont();

						ImGui::Dummy(ImVec2(0, 15));

						int glossy_index = m_context["sys_sample_glossy_index"]->getUint();
						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 75.0f, 0.0f));
					    ImGui::SameLine();
						if (ImGui::RadioButton("Off##", &glossy_index, 0))
						{
							m_context["sys_sample_glossy_index"]->setUint(glossy_index);
							changed = true;
						}

						ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 75.0f, 0.0f));
						ImGui::SameLine();
						if (ImGui::RadioButton("On##", &glossy_index, 1))
						{
							m_context["sys_sample_glossy_index"]->setUint(glossy_index);
							changed = true;
						}

						//ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 75.0f, 0.0f));
						//ImGui::SameLine();
						//if (ImGui::RadioButton("on2##", &glossy_index, 2))
						//{
						//	m_context["sys_sample_glossy_index"]->setUint(glossy_index);
						//	changed = true;
						//}

					}ImGui::EndChild();


					//
					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}


				// Use Fresnel ( drag Int )
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("Use Fresnel##", ImVec2(ImGui::GetWindowWidth() - 14, 80.0f + 10.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Fresnel Equation:");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					int fresnel = m_context["sys_use_fresnel_equations"]->getInt();

					if (ImGui::RadioButton("Off##", &fresnel, 0))
					{
						m_context["sys_use_fresnel_equations"]->setInt(0);
						changed = true;
					}

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::RadioButton("On##", &fresnel, 1))
					{
						m_context["sys_use_fresnel_equations"]->setInt(1);
						changed = true;
					}

					ImGui::PopItemWidth();
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}

				//ImGui::Dummy(ImVec2(2, 5));

				ImGui::Dummy(ImVec2(0, 5));
				ImGui::Dummy(ImVec2(75.0f, 0.0f)); ImGui::SameLine();
				if (ImGui::Button(" Save Preset " , ImVec2(100, 25)))
				{
					SceneRenderSettings& settings = m_scene_mode == Edit_Mode ? m_sceneEdit_settings : m_sceneRender_settings;

					optix::int2 ray_depth = optix::make_int2(shader.data.min_depth, shader.data.max_depth);
					int custom_lighting = m_context["cast_shadow_ray"]->getInt();
					int refl    = m_context["sys_cast_reflections"]->getInt();
					int glos    = m_context["sys_sample_glossy_index"]->getUint();
					int fresnel = m_context["sys_use_fresnel_equations"]->getInt();
					settings.ray_depth = ray_depth;
					settings.custom_lighting = custom_lighting;
					settings.reflections = refl;
					settings.glossiness = glos;
					settings.fresnel = fresnel;
				}
			}
			ImGui::EndChild(); guiStyleManager::setStyle("main_style_right_child2");
			
			ImGui::Dummy(ImVec2(0, 2));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 2));

			// Scene Settings
			guiStyleManager::setStyle("main_style_right_child_transformations");
			ImGui::BeginChild("child_scene_asd_settings##", ImVec2(ImGui::GetWindowWidth() - 4.0f, 600.0f), true, window_flags2);
			{

				bool sceneGrid_changed     = false;
				bool sceneSettings_changed = false;

				ImGui::Dummy(ImVec2(0, 2));
				guiStyleManager::setFont(ROBOTO, "Bold", 14);
				ImGui::Text(" Scene Settings:");
				ImGui::PopFont();
				ImGui::Dummy(ImVec2(0, 10));


				//SceneGrid_Paramaters pars    = OpenGLRenderer::GetInstace().getSceneGridParameters();
				SceneParameters scene_params = Mediator::Request_SceneParameters();
				SceneGrid_Paramaters pars    = scene_params.grid_params;

				// Scene Dimensions
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("scene_sizeasdas## ", ImVec2(ImGui::GetWindowWidth() - 14,
									  80.0f + 50.0f + 60.0f + 60.0f),
									  true, 
									  window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Scene Dimensions: ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					if (ImGui::DragFloat(" Width(X)## ", &scene_params.max_dimensions.x, 1.0f, 0, 99999.9f))
					{
						m_dragFloat_state = true;
						scene_params.max_dimensions.x < 0 ? scene_params.max_dimensions.x = 0 : scene_params.max_dimensions.x;
						sceneSettings_changed = true;
					}

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::DragFloat(" Height(Y)## ", &scene_params.max_dimensions.y, 1.0f, 0, 99999.9f))
					{
						m_dragFloat_state = true;
						scene_params.max_dimensions.y < 0 ? scene_params.max_dimensions.y = 0 : scene_params.max_dimensions.y;
						sceneSettings_changed = true;
					}

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::DragFloat(" Length(Z)## ", &scene_params.max_dimensions.z, 1.0f, 0, 99999.9f))
					{
						m_dragFloat_state = true;
						scene_params.max_dimensions.z < 0 ? scene_params.max_dimensions.z = 0 : scene_params.max_dimensions.z;
						sceneSettings_changed = true;
					}
					//
					ImGui::PopItemWidth();

					ImGui::Dummy(ImVec2(0, 18));
					ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine();
					ImGui::BeginChild("scene_dim_pars_child##", ImVec2(ImGui::GetWindowWidth() - 45, 120), true, window_flags2);
					{

						ImGui::Dummy(ImVec2(65, 0)); ImGui::SameLine();
						guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);
						ImGui::Text(" Parameters:");
						ImGui::PopFont();
						 
						ImGui::Dummy(ImVec2(0, 15));

						ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine();
						if (ImGui::Checkbox(" GL_POLYGON", &scene_params.vis_planes)) sceneSettings_changed = true;

						ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine();
						ImGui::PushItemWidth(100.0f);
						if (ImGui::DragFloat(" Opacity", &scene_params.alpha, 0.01f, 0.0f, 1.0f))
						{
							m_dragFloat_state = true;
							sceneSettings_changed = true;
						}ImGui::PopItemWidth();

						ImGuiColorEditFlags flags = 0;
						flags |= ImGuiColorEditFlags_NoInputs;
						float * col = ((float*)&scene_params.vis_color);
						ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine();
						if (ImGui::ColorEdit3(" Color", col, flags))
						{
							sceneSettings_changed = true;
						}

						ImGui::Dummy(ImVec2(0, 5));
						ImGui::Dummy(ImVec2(15, 0)); ImGui::SameLine();
						if (ImGui::Checkbox(" Visualize ##", &scene_params.visualize)) sceneSettings_changed = true;
							


					}ImGui::EndChild();

					// --------------------------------------------------------------



					//
					//ImGui::PopItemWidth();
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}

				ImGui::Dummy(ImVec2(0, 2));
				ImGui::Dummy(ImVec2(0, 2));

				// Number of blocks [ Width x Height ]
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("block_sizeasda2232s## ", ImVec2(ImGui::GetWindowWidth() - 14, 80.0f + 10.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Grid Dimensions:");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					if (ImGui::InputInt(" Width(X)##", &pars.block_numb.x))
					{
						pars.block_numb.x < 0 ? pars.block_numb.x = 0 : pars.block_numb.x;
						sceneGrid_changed = true;
					}

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::InputInt(" Length(Z)##", &pars.block_numb.y))
					{
						pars.block_numb.y < 0 ? pars.block_numb.y = 0 : pars.block_numb.y;
						sceneGrid_changed = true;
					}

					ImGui::PopItemWidth();
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}


				// Block Size
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("block_sizeasdas## ", ImVec2(ImGui::GetWindowWidth() - 14, 80.0f + 10.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Grid-Block Dimensions: ");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					if (ImGui::DragFloat(" Width(X)## ", &pars.block_size.x, 1.0f, 0, 999999.9f))
					{
						m_dragFloat_state = true;
						pars.block_size.x < 0 ? pars.block_size.x = 0 : pars.block_size.x;
						sceneGrid_changed = true;
					}

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::DragFloat(" Length(Z)## ", &pars.block_size.y, 1.0f, 0, 999999.9f))
					{
						m_dragFloat_state = true;
						pars.block_size.y < 0 ? pars.block_size.y = 0 : pars.block_size.y;
						sceneGrid_changed = true;
					}

					ImGui::PopItemWidth();
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}


				// Render Scene Grid
				{
					guiStyleManager::setStyle("main_style_right_child_transformations");
					ImGui::BeginChild("draw_scene_grid_bool##", ImVec2(ImGui::GetWindowWidth() - 14, 80.0f + 10.0f), true, window_flags2);

					guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 14);ImGui::Text(" Render Grid:");
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(0, 10.0f));
					guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					ImGui::PushItemWidth(100.0f);
					int render = pars.render;

					if (ImGui::RadioButton("Off##", &render, 0))
					{
						pars.render = 0;
						sceneGrid_changed = true;
					}

					ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
					ImGui::SameLine();
					if (ImGui::RadioButton("On##", &render, 1))
					{
						pars.render = 1;
						sceneGrid_changed = true;
					}

					ImGui::PopItemWidth();
					ImGui::PopFont();

					ImGui::Dummy(ImVec2(2, 5));
					ImGui::EndChild();
				}



				if (sceneGrid_changed || sceneSettings_changed)
				{
					scene_params.grid_params = pars;
					Mediator::setSceneParamaters(scene_params);
					Mediator::markViewportsDirty();
				}
				
			}
			ImGui::EndChild(); guiStyleManager::setStyle("main_style_right_child2");

		}
		ImGui::EndChild();


		if (changed)
		{
			Mediator::restartAccumulation();
		}
	}

	ImGui::PopFont();
}
void InterfaceManager::construct_Scene_Options_Window()
{
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);


	ImGui::PopFont();
}


// pop Ups
bool InterfaceManager::pop_Object_cloning_Approval_window()
{
	
	guiStyleManager::setStyle("style_popup");
	bool decided = false;

	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoDecoration;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	//ImGui::SetNextWindowSize(ImVec2(0, 0));

	float a = ImGui::GetStyle().Colors[ImGuiCol_Border].w;
	ImGui::GetStyle().Colors[ImGuiCol_Border].w = 0.0f;
	ImGui::Begin("popup_object_approval_base_window", NULL, flags);
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

	bool approve = false;
	if (m_popup_object_cloning_approval)
		ImGui::OpenPopup("Cloning Options");

	flags = 0;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;
	
	ImGui::GetStyle().Colors[ImGuiCol_Border].w = a;
	if (ImGui::BeginPopupModal("Cloning Options", NULL, flags))
	{
		

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		

		float fp_y = ImGui::GetStyle().FramePadding.y;
		ImGui::GetStyle().FramePadding.y = 0.0f;
		ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Text];
		ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		ImGui::Text(" Clone As:                |   Cloning options:");
		ImGui::Dummy(ImVec2(0.0f, 1.0f));
		ImGuiWindowFlags c_flags = 0;
		ImGui::BeginChild("popup_clone_object_child##", ImVec2(ImGui::GetWindowWidth() - 23.f, ImGui::GetWindowHeight() / 3.0f), true, c_flags);
		ImGui::Dummy(ImVec2(0.0f, 7.0f));
		
		int e = as_copy ? 0 : 1;

		ImGui::Dummy(ImVec2(4.0f, 0.0f));
		ImGui::SameLine();
		if (ImGui::RadioButton(" Copy ", &e, 0))
		{
			as_copy     = true;
			as_instance = false;
		}

		ImGui::SameLine(); ImGui::Custom_Separator2(ImVec2(25.0f, 0.0), 35.0f, 1.0f, ImVec4(0.2f,0.2f,0.2f,1.0f));

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(30.0f + 7.0f, 0.0f));
		ImGui::SameLine();
		{
			ImGui::Checkbox(" clone: Facevectors", &cloning_options_clone_fvs);
		}

		ImGui::Dummy(ImVec2(0.0f, 2.0f));

		ImGui::Dummy(ImVec2(4.0f, 0.0f));
		ImGui::SameLine();
		if (ImGui::RadioButton(" Instance ", &e,1))
		{
			as_instance = true;
			as_copy = false;
		}

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(11.0f + 7.0f, 0.0f));
		ImGui::SameLine();
		{
			ImGui::Checkbox(" clone: Samplers", &cloning_options_clone_samplers);
		}

		ImGui::EndChild();
		

		// Name input field //
		bool is_area_selection = m_cloning_active_type == 2 ? true : false;
		if (!is_area_selection)
		{
			ImGui::GetStyle().FrameRounding -= 6;
			ImGui::Dummy(ImVec2(0.0f, 3.0f));
			ImGui::Text("Name:"); ImGui::SameLine();
			ImGui::GetStyle().FramePadding.y = 3.0f;
			ImGui::PushItemWidth(218.0f);
			ImGui::InputText("##", cloning_object_name , IM_ARRAYSIZE( cloning_object_name ));
			ImGui::PopItemWidth();
			ImGui::GetStyle().FrameRounding += 6;
		}
		else
		{
			ImGui::Dummy(ImVec2(0.0f,20.0f));
		}
		//

		int enter = glfwGetKey(Mediator::RequestWindow(), GLFW_KEY_ENTER);
		int esc   = glfwGetKey(Mediator::RequestWindow(), GLFW_KEY_ESCAPE);
		//std::cout << "\n - enter : " << enter << std::endl;
		//std::cout << " - escpae : " << esc << std::endl;

		ImGui::GetStyle().FramePadding.y = 2.0f;
		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		ImGui::Dummy(ImVec2(109.0f, 0.0f));
		ImGui::SameLine();
		if (
			(  ImGui::Button("Ok", ImVec2(70,22))
			|| enter == GLFW_PRESS )
			&& (as_copy || as_instance))
		{
			ImGui::CloseCurrentPopup();
			approve = true;
			decided = true;
		}

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(70,22)) || esc == GLFW_PRESS )
		{
			ImGui::CloseCurrentPopup();
			approve = false;
			decided = true;
		}
		
		ImGui::GetStyle().Colors[ImGuiCol_Text] = col;
		ImGui::GetStyle().FramePadding.y = fp_y;
		ImGui::EndPopup();
	}

	ImGui::PopFont();
	ImGui::End();
	

	if (decided)
	{
		handle_CLONE( approve, as_instance, m_cloned_object_pack, m_prev_selected_object_pack, cloning_object_name );
	}

	return true;
}
bool InterfaceManager::pop_Group_creating_Approval_window()
{
	guiStyleManager::setStyle("style_popup_group");
	bool decided = false;

	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoDecoration;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::SetNextWindowSize(ImVec2(0, 0));

	
	ImGui::Begin("popup_group_creating_approval_base_window", NULL, flags);
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);

	bool approve = false;
	if (m_popup_group_creating_approval)
		ImGui::OpenPopup("Group Creation");

	flags = 0;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;
	

	if (ImGui::BeginPopupModal("Group Creation", NULL, flags))
	{

		ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Text];
		ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		ImGui::Dummy(ImVec2(250.0f, 0.0f));
		{
			ImGui::GetStyle().FrameRounding -= 6;
			ImGui::Dummy(ImVec2(0.0f, 3.0f));
			ImGui::Text("Name: "); ImGui::SameLine();
			ImGui::GetStyle().FramePadding.y = 2.0f;
			ImGui::PushItemWidth(218.0f);
			ImGui::InputText("##", group_name , IM_ARRAYSIZE(group_name));
			ImGui::PopItemWidth();
			ImGui::GetStyle().FrameRounding += 6;
		}
		ImGui::Dummy(ImVec2(0.0f, 40.0f));

		ImGui::Dummy(ImVec2(118.0f, 0.0f));
		ImGui::SameLine();
		if (ImGui::Button("Ok", ImVec2(70, 22)) || key_data[GLFW_KEY_ENTER].pressed)
		{
			// edw check an yparxei to name se allo group
			m_popup_group_creating_approval = false;
			ImGui::CloseCurrentPopup();
			approve = true;
			decided = true;
		}

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(70, 22)) || key_data[GLFW_KEY_ESCAPE].pressed)
		{
			m_popup_group_creating_approval = false;
			ImGui::CloseCurrentPopup();
			approve = false;
			decided = true;
		}



		ImGui::EndPopup();
	}

	ImGui::PopFont();
	ImGui::End();


	if (decided && approve)
	{
		handle_GROUP(group_name);
		return true;
	}
	else if (decided && !approve)
	{
		return false;
	}

	return true;
}
bool InterfaceManager::pop_Create_Light_Window()
{
	guiStyleManager::setStyle("style_popup");
	bool decided = false;

	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoDecoration;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::SetNextWindowSize(ImVec2(0, 0));


	ImGui::Begin("popup_create_Light", NULL, flags);
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 16);

	bool approve = false;
	if (m_popup_create_light)
		ImGui::OpenPopup("Create Light");

	flags = 0;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;
	
	if (ImGui::BeginPopupModal("Create Light", NULL, flags))
	{

		ImGui::Dummy(ImVec2(250.0f, 0.0f));
		

		// Light Type ...


		ImGui::Dummy(ImVec2(0.0f, 30.0f));
		if (ImGui::Button("Ok", ImVec2(40, 30)))
		{
			m_popup_create_light = false;
			ImGui::CloseCurrentPopup();
			approve = true;
			decided = true;
		}

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(200.0f, 0.0f));
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 30)))
		{
			m_popup_create_light = false;
			ImGui::CloseCurrentPopup();
			approve = false;
			decided = true;
		}



		ImGui::EndPopup();
	}

	ImGui::PopFont();
	ImGui::End();


	if (decided && approve)
	{
		// handle Create Light ?
		return true;
	}
	else if (decided && !approve)
	{

		return false;
	}

	return true;
}
bool InterfaceManager::pop_Create_Sampler_Window()
{

	guiStyleManager::setStyle("style_popup");
	bool decided = false;


	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoDecoration;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::SetNextWindowSize(ImVec2(0, 0));


	ImGui::Begin("popup_attach_Sampler", NULL, flags);
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 16);

	bool approve = false;
	if (m_popup_create_light)
		ImGui::OpenPopup("Attach Sampler");

	flags = 0;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;

	if (ImGui::BeginPopupModal("Attach Sampler", NULL, flags))
	{

		ImGui::Dummy(ImVec2(250.0f, 0.0f));


		ImGui::Dummy(ImVec2(0.0f, 30.0f));
		if (ImGui::Button("Ok", ImVec2(40, 30)))
		{
			// edw check an yparxei to name se allo group
			m_popup_create_sampler = false;
			ImGui::CloseCurrentPopup();
			approve = true;
			decided = true;
		}

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(200.0f, 0.0f));
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(60, 30)))
		{
			m_popup_create_sampler = false;
			ImGui::CloseCurrentPopup();
			approve = false;
			decided = true;
		}



		ImGui::EndPopup();
	}

	ImGui::PopFont();
	ImGui::End();


	if (decided && approve)
	{
		// handle Create Light ?
		return true;
	}
	else if (decided && !approve)
	{

		return false;
	}

	return true;
}
bool InterfaceManager::pop_Object_Align_Window()
{

	guiStyleManager::setStyle("style_popup2");
	bool decided = false;

	SceneObject* object = m_align_object;
	SceneObject* target = m_align_target;

	ImGuiWindowFlags flags = 0;
	flags |= ImGuiWindowFlags_NoDecoration;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	flags |= ImGuiWindowFlags_NoScrollbar;

	float a = ImGui::GetStyle().Colors[ImGuiCol_Border].w;
	ImGui::GetStyle().Colors[ImGuiCol_Border].w = 0.0f;
	ImGui::Begin("popup_object_align_window", NULL, flags);
	guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 14);


	std::string popup_label = "Align Object(" + m_selected_object_pack.raw_object->getName() + ")";
	bool approve = false;
	if (m_popup_object_align_window)
		ImGui::OpenPopup(popup_label.c_str());

	flags = 0;
	flags |= ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::GetStyle().Colors[ImGuiCol_Border].w = a;

	float w_off = 60;
	float in_off = 7.0f;
	ImGui::SetNextWindowSize(ImVec2(400 - w_off, 400 + 100 + 50 + 10 - 20 ));

	bool changed = false;
	bool changed2 = false;

	bool pos_changed = false;
	bool or_changed  = false;

	bool pos_x = m_pos_x;
	bool pos_y = m_pos_y;
	bool pos_z = m_pos_z;
	bool axis_x = m_axis_x;
	bool axis_y = m_axis_y;
	bool axis_z = m_axis_z;

	bool pos_align_enabled = m_pos_align_enabled;
	bool rot_align_enabled = m_rot_align_enabled;

	ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0, 0, 0, 0);
	if (ImGui::BeginPopupModal(popup_label.c_str(), NULL, flags))
	{

		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		float fp_y = ImGui::GetStyle().FramePadding.y;
		ImGui::GetStyle().FramePadding.y = 0.0f;
		ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Text];
		ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImGuiWindowFlags c_flags = 0;



		// Hierarchy Setting
		guiStyleManager::setIconFont(MDI, 14);
		ImGui::Text(ICON_MDI_TRANSIT_CONNECTION_VARIANT); ImGui::SameLine();
		ImGui::PopFont();
		ImGui::Text("Transformation Inheritance:");

		bool hierarchy_changed = false;
		int hierarchy_setting = m_align_hierarchy_setting;
		ImGui::BeginChild("Hierarchy_Setting_child##", ImVec2(ImGui::GetWindowWidth() - 23.f , 100), true, c_flags);
		{

			ImGui::Dummy(ImVec2(0, 10));
			

			ImVec4 t_col = ImGui::GetStyle().Colors[ImGuiCol_Button];

			ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5 - 80, 0.0f)); ImGui::SameLine();
			if (m_align_hierarchy_setting == TO_ALL) ImGui::GetStyle().Colors[ImGuiCol_Button] = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
			if (ImGui::Button("Affect Hierarchy##", ImVec2(150,20)))
			{
				m_align_hierarchy_setting = TO_ALL;
			}ImGui::GetStyle().Colors[ImGuiCol_Button] = t_col;

			
			ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5 - 80, 0.0f)); ImGui::SameLine();
			if (m_align_hierarchy_setting == ONLY_THIS) ImGui::GetStyle().Colors[ImGuiCol_Button] = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
			if (ImGui::Button("Affect Only Object##", ImVec2(150, 20)))
			{
				m_align_hierarchy_setting = ONLY_THIS;
			}ImGui::GetStyle().Colors[ImGuiCol_Button] = t_col;

			ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5 - 80, 0.0f)); ImGui::SameLine();
			if (m_align_hierarchy_setting == ONLY_TO_CHILDS) ImGui::GetStyle().Colors[ImGuiCol_Button] = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
			if (ImGui::Button("Affect Only Hierarchy##", ImVec2(150, 20)))
			{
				m_align_hierarchy_setting = ONLY_TO_CHILDS;
			}ImGui::GetStyle().Colors[ImGuiCol_Button] = t_col;

			
		}ImGui::EndChild();
		hierarchy_changed = hierarchy_setting != (int)m_align_hierarchy_setting;

		ImGui::Dummy(ImVec2(0, 8.0f));

		// Position Alignement
		{
			ImGui::Dummy(ImVec2(in_off - 7, 0)); ImGui::SameLine();
			ImGui::Text("Align Position( World ):"); ImGui::SameLine(); ImGui::Checkbox("##pos_align_enabled", &pos_align_enabled); 
			
			ImGui::Dummy(ImVec2(0.0f, 1.0f));
			
			ImGui::BeginChild("align_posititon_popup_child_window##", ImVec2(ImGui::GetWindowWidth() - 23.f, 212 ), true, c_flags);
			{

				ImGui::Dummy(ImVec2(0.0f, 14.0f));

				ImGui::Dummy(ImVec2(1, 0));
				ImGui::Dummy(ImVec2(in_off + 1, 0));
				ImGui::GetStyle().ItemInnerSpacing.x -= 5.0f;
				ImGui::SameLine(); ImGui::Checkbox("X Position ##", &pos_x); ImGui::SameLine(); ImGui::Dummy(ImVec2(18, 0));
				ImGui::SameLine(); ImGui::Checkbox("Y Position ##", &pos_y); ImGui::SameLine(); ImGui::Dummy(ImVec2(18, 0));
				ImGui::SameLine(); ImGui::Checkbox("Z Position ##", &pos_z); ImGui::SameLine(); ImGui::Dummy(ImVec2(18, 0));
				ImGui::GetStyle().ItemInnerSpacing.x += 5.0f;

				ImGui::Dummy(ImVec2(0, 6));

				ImGui::Dummy(ImVec2(in_off, 0)); ImGui::SameLine();
				ImGui::Text(" Current Object:");
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * 0.5f - 115.0f - 20.0f + w_off * 0.5 + 9.0f, 0));
				ImGui::SameLine();
				ImGui::Text("Target Object:");


				// child A
				ImGui::Dummy(ImVec2(in_off, 0)); ImGui::SameLine();
				ImGui::BeginChild("Current Object:##", ImVec2(ImGui::GetWindowWidth() * 0.5f - 65.0f + w_off * 0.5, 120.0f), true, 0);
				{
					ImGui::Dummy(ImVec2(0, 13));

					int e = A_pos_align;
					if (ImGui::RadioButton("Min##A", &e, 0))
					{
						if (A_pos_align != 0) changed = true;
						A_pos_align = 0;
					}ImGui::Dummy(ImVec2(0, 2));
					if (ImGui::RadioButton("Centroid##A", &e, 1))
					{
						if (A_pos_align != 1) changed = true;
						A_pos_align = 1;
					}ImGui::Dummy(ImVec2(0, 2));
					if (ImGui::RadioButton("Pivot Position##A", &e, 2))
					{
						if (A_pos_align != 2) changed = true;
						A_pos_align = 2;
					}ImGui::Dummy(ImVec2(0, 2));
					if (ImGui::RadioButton("Max##A", &e, 3))
					{
						if (A_pos_align != 3) changed = true;
						A_pos_align = 3;
					}ImGui::Dummy(ImVec2(0, 2));

				}ImGui::EndChild();

				ImGui::SameLine();
				ImGui::Dummy(ImVec2(10, 0));
				ImGui::SameLine();

				// child B
				ImGui::Dummy(ImVec2(in_off, 0)); ImGui::SameLine();
				ImGui::BeginChild("Target Object:##", ImVec2(ImGui::GetWindowWidth() * 0.5f - 65.0f + w_off * 0.5, 120.0f), true, 0);
				{
					ImGui::Dummy(ImVec2(0, 13));

					int e = B_pos_align;
					if (ImGui::RadioButton("Min##B", &e, 0))
					{
						if (B_pos_align != 0) changed2 = true;
						B_pos_align = 0;
					}ImGui::Dummy(ImVec2(0, 2));
					if (ImGui::RadioButton("Centroid##B", &e, 1))
					{
						if (B_pos_align != 1) changed2 = true;
						B_pos_align = 1;
					}ImGui::Dummy(ImVec2(0, 2));
					if (ImGui::RadioButton("Pivot Position##B", &e, 2))
					{
						if (B_pos_align != 2) changed2 = true;
						B_pos_align = 2;
					}ImGui::Dummy(ImVec2(0, 2));
					if (ImGui::RadioButton("Max##B", &e, 3))
					{
						if (B_pos_align != 3) changed2 = true;
						B_pos_align = 3;
					}ImGui::Dummy(ImVec2(0, 2));

				}ImGui::EndChild();

			} ImGui::EndChild();

		}

		
		//ImGui::Dummy(ImVec2(0, 3));
		ImGui::Dummy(ImVec2(0, 8.0f));

		// Orientation Alignement
		{
			
			ImGui::Text(" Align Orientation( Local ):"); ImGui::SameLine(); ImGui::Checkbox("##rot_align_enabled", &rot_align_enabled);
			ImGui::Dummy(ImVec2(0.0f, 1.0f));

			ImGui::BeginChild("align_orientation_popup_child##", ImVec2(ImGui::GetWindowWidth() - 23.f, 45.0f) , true, c_flags);
			{
				ImGui::Dummy(ImVec2(0, 10));

				ImGui::Dummy(ImVec2(in_off , 0)); 
				ImGui::GetStyle().ItemInnerSpacing.x -= 5.0f;
				ImGui::SameLine(); ImGui::Checkbox("X Axis ##", &axis_x); ImGui::SameLine(); ImGui::Dummy(ImVec2(14, 0));
				ImGui::SameLine(); ImGui::Checkbox("Y Axis ##", &axis_y); ImGui::SameLine(); ImGui::Dummy(ImVec2(14, 0));
				ImGui::SameLine(); ImGui::Checkbox("Z Axis ##", &axis_z); ImGui::SameLine(); ImGui::Dummy(ImVec2(14, 0));
				ImGui::GetStyle().ItemInnerSpacing.x += 5.0f;

				ImGui::Dummy(ImVec2(0, 2));

			}ImGui::EndChild();
		}
		
		if (hierarchy_changed) { pos_changed = true; or_changed = true; }
		if (m_pos_align_enabled != pos_align_enabled) pos_changed = true;
		if (m_rot_align_enabled != rot_align_enabled) or_changed  = true;
		if ((m_pos_x != pos_x)   || (m_pos_y != pos_y)   || (m_pos_z != pos_z))   pos_changed = true; //&& pos_align_enabled;
		if ((m_axis_x != axis_x) || (m_axis_y != axis_y) || (m_axis_z != axis_z)) or_changed  = true; //&& rot_align_enabled;
		m_pos_x  = pos_x;  m_pos_y  = pos_y;  m_pos_z  = pos_z;
		m_axis_x = axis_x; m_axis_y = axis_y; m_axis_z = axis_z;
		m_pos_align_enabled = pos_align_enabled;
		m_rot_align_enabled = rot_align_enabled;

		pos_x  &= m_pos_align_enabled; pos_y  &= m_pos_align_enabled; pos_z  &= m_pos_align_enabled;
		axis_x &= m_rot_align_enabled; axis_y &= m_rot_align_enabled; axis_z &= m_rot_align_enabled;

		// On popup Appearing : Align position on XYZ ( on pivot point )
		if (m_popup_object_align_toggled)
		{
			pos_changed = true;
			changed     = true;
		}


		
		optix::float3 axis_ = ZERO_3f;
		if (changed || changed2 || pos_changed 
			|| or_changed
			)
		{
			// pos [XYZ] changed
			axis_.x = pos_x ? 1 : 0;
			axis_.y = pos_y ? 1 : 0;
			axis_.z = pos_z ? 1 : 0;

			// undo local actions 
			if (m_align_pos_action != 0)
			{
				m_align_pos_action->undo();
				delete m_align_pos_action;
				m_align_pos_action = 0;
			}
		}

		// pos align A changed
		if (   changed || changed2 ||  pos_changed 
			|| or_changed
			)
		{
			
			optix::float3 pos_delta = ZERO_3f;

			// min
			if      (A_pos_align == 0)
			{
				optix::float3 * bbox = object->getBBOX_Transformed( true );
				optix::float3 p1 = bbox[0];
				delete bbox;

				optix::float3 p2 = ZERO_3f;
				if      (B_pos_align == 0)
				{
					optix::float3 * bbox2 = target->getBBOX_Transformed(true);
					p2 = bbox2[0];
					delete bbox2;
				}
				else if (B_pos_align == 2)
				{
					p2 = target->getTranslationAttributes();
				}
				else if (B_pos_align == 1)
				{
					p2 = target->getCentroid_Transformed();
				}
				else if (B_pos_align == 3)
				{
					optix::float3 * bbox2 = target->getBBOX_Transformed(true);
					p2 = bbox2[1];
					delete bbox2;
				}
				
				pos_delta = p2 - p1;
			}

			// centroid
			else if (A_pos_align == 1)
			{
				optix::float3 p1 = object->getCentroid_Transformed();
				
				optix::float3 p2 = ZERO_3f;
				if      (B_pos_align == 0)
				{
					optix::float3 * bbox2 = target->getBBOX_Transformed(true);
					p2 = bbox2[0];
					delete bbox2;
				}
				else if (B_pos_align == 2)
				{
					p2 = target->getTranslationAttributes();
				}
				else if (B_pos_align == 1)
				{
					p2 = target->getCentroid_Transformed();
				}
				else if (B_pos_align == 3)
				{
					optix::float3 * bbox2 = target->getBBOX_Transformed(true);
					p2 = bbox2[1];
					delete bbox2;
				}

				pos_delta = p2 - p1;
			}

			// pivot
			else if (A_pos_align == 2)
			{
				optix::float3 p1 = object->getTranslationAttributes();

				optix::float3 p2 = ZERO_3f;
				if      (B_pos_align == 0)
				{
					optix::float3 * bbox2 = target->getBBOX_Transformed(true);
					p2 = bbox2[0];
					delete bbox2;
				}
				else if (B_pos_align == 2)
				{
					p2 = target->getTranslationAttributes();
				}
				else if (B_pos_align == 1)
				{
					p2 = target->getCentroid_Transformed();
				}
				else if (B_pos_align == 3)
				{
					optix::float3 * bbox2 = target->getBBOX_Transformed(true);
					p2 = bbox2[1];
					delete bbox2;
				}

				pos_delta = p2 - p1;
			}

			// max
			else if (A_pos_align == 3)
			{
				optix::float3 * bbox = object->getBBOX_Transformed(true);
				optix::float3 p1 = bbox[1];
				delete bbox;

				optix::float3 p2 = ZERO_3f;
				if      (B_pos_align == 0)
				{
					optix::float3 * bbox2 = target->getBBOX_Transformed(true);
					p2 = bbox2[0];
					delete bbox2;
				}
				else if (B_pos_align == 2)
				{
					p2 = target->getTranslationAttributes();
				}
				else if (B_pos_align == 1)
				{
					p2 = target->getCentroid_Transformed();
				}
				else if (B_pos_align == 3)
				{
					optix::float3 * bbox2 = target->getBBOX_Transformed(true);
					p2 = bbox2[1];
					delete bbox2;
				}

				pos_delta = p2 - p1;
			}


			// pass Action //
			pos_delta *= axis_;
			optix::float3 axis[3]; BASE_TRANSFORM::AXIS(object, LOCAL, axis);
			object->setTransformation(pos_delta, WORLD, TRANSLATION, m_align_hierarchy_setting);
			
			Action* ac = new Action_TRANSFORMATION(object, pos_delta, axis, WORLD, m_align_hierarchy_setting, ACTION_TRANSFORMATION_TRANSLATION);
			m_align_pos_action = ac;
		}
				

		// rot [XYZ] changed
		if ( or_changed )
		{
			
			optix::float3 r_axis = ZERO_3f;
			r_axis.x = axis_x ? 1 : 0;
			r_axis.y = axis_y ? 1 : 0;
			r_axis.z = axis_z ? 1 : 0;


			if ( m_align_rot_changed )
			{

				if (m_align_rot_action != 0)
				{
					m_align_rot_action->undo();
					delete m_align_rot_action;
					m_align_rot_action = 0;
				}

				if (r_axis.x == 0 && r_axis.y == 0 && r_axis.z == 0)
				{
					m_align_rot_changed = false;
				}
			}


			// align axis
			bool needs_align = (r_axis.x || r_axis.y || r_axis.z);
			if (needs_align)
			{
				optix::float3 obj_axis[3]; BASE_TRANSFORM::AXIS(object, LOCAL, obj_axis);
				optix::float3 trg_axis[3]; BASE_TRANSFORM::AXIS(target, LOCAL, trg_axis);

				optix::float3 raw_axis_dots;
				raw_axis_dots.x = dot(obj_axis[0], trg_axis[0]);
				raw_axis_dots.y = dot(obj_axis[1], trg_axis[1]);
				raw_axis_dots.z = dot(obj_axis[2], trg_axis[2]);

				optix::float3 new_axis[3];
				optix::Matrix4x4 new_basis;

				int count_axis_align = r_axis.x + r_axis.y + r_axis.z;
				if      (count_axis_align == 1)
				{
					int axis_index = 0;
					int ind_i      = 1;
					int ind_j      = 2;
					if      (r_axis.y == 1) { axis_index = 1; ind_i = 0; ind_j = 2; }
					else if (r_axis.z == 1) { axis_index = 2; ind_i = 0; ind_j = 1; }

					//
					new_axis[axis_index] = trg_axis[axis_index];
					new_axis[ind_i]      = obj_axis[ind_i];
					new_axis[ind_j]      = obj_axis[ind_j];

					
					float d_align = fabsf(dot(obj_axis[axis_index], trg_axis[axis_index]));
					bool is_axis_already_aligned = 1 - d_align < 1e-7;
					
					bool is_orthogonal = true;
					for (int i = 0; i < 3; i++)
					{
						for (int j = 0; j < 3; j++)
						{
							if (i == j) continue;
							float d = dot(new_axis[i], new_axis[j]);
							if (fabsf(d) >= 1e-7) //(d != 0)
							{
								is_orthogonal = false;
								break;
							}
						}
					}

					if (!is_orthogonal && !is_axis_already_aligned)
					{
						// check for partial orthogonallity
						int not_ortho_axis;
						int ortho_axis;
						bool di = fabsf(dot(new_axis[axis_index], new_axis[ind_i])) < 1e-7;
						bool dj = fabsf(dot(new_axis[axis_index], new_axis[ind_j])) < 1e-7;

						bool partial_orthogonallity = (di && !dj) || (!di && dj);
						if (partial_orthogonallity 
							//&& false
							)
						{
						
							not_ortho_axis = di ? ind_j : ind_i;
							ortho_axis     = di ? ind_i : ind_j;
							new_axis[not_ortho_axis] = normalize(optix::cross(new_axis[axis_index], new_axis[ortho_axis]));
						}
						else
						{
							
							optix::float3 cross_axis = GLOBAL_AXIS[1];
							
							new_axis[ind_i] = normalize(optix::cross(new_axis[axis_index], GLOBAL_AXIS[1]));
							new_axis[ind_j] = normalize(optix::cross(new_axis[axis_index], new_axis[ind_i]));
							new_axis[ind_i] = normalize(optix::cross(new_axis[axis_index], new_axis[ind_j]));

							float di  = fabsf(dot(new_axis[axis_index], new_axis[ind_i]));
							float dj  = fabsf(dot(new_axis[axis_index], new_axis[ind_j]));
							float dij = fabsf(dot(new_axis[ind_i], new_axis[ind_j]));

						}
					}

				}
				else if (count_axis_align >= 2)
				{
					new_axis[0] = trg_axis[0];
					new_axis[1] = trg_axis[1];
					new_axis[2] = trg_axis[2];
				}


				
				bool is_orthogonal_base = true;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						if (i != j)
						{
							float d = dot(new_axis[i], new_axis[j]);
							if (d != 0)
							{
								is_orthogonal_base = false;
								break;
							}
						}

					}
				}
				
				new_basis = optix::Matrix4x4::fromBasis(new_axis[0], new_axis[1], new_axis[2], ZERO_3f);
				optix::Matrix4x4 rot_mat = object->getRotationMatrix_chain().inverse() * new_basis;
				object->setTransformation( rot_mat, LOCAL, ROTATION, m_align_hierarchy_setting, 0 );
				m_align_rot_mat     = optix::Matrix4x4( rot_mat.getData() );
				m_align_rot_changed = true;

				m_align_rot_action = new Action_TRANSFORMATION(object, rot_mat, obj_axis, WORLD, m_align_hierarchy_setting, ACTION_TRANSFORMATION_ROTATION_MAT);
			}

		}

		ImGui::GetStyle().FramePadding.y = 2.0f;
		ImGui::Dummy(ImVec2(0.0f, 25.0f));
		ImGui::Dummy(ImVec2(165.0f, 0.0f));
		ImGui::SameLine();
		
		if (ImGui::Button("Ok", ImVec2(70, 22)) || key_data[GLFW_KEY_ENTER].pressed)
		{
			m_axis_x = m_axis_y = m_axis_z = 0;
			m_pos_x  = m_pos_y  = m_pos_z = 1;

			// pernaw to local action ston action manager
			int p = 0;
			int r = 0;
			if (m_align_pos_action != 0)
			{
				p = 1;
				ActionManager::GetInstance().push(m_align_pos_action);
			}
			if (m_align_rot_action != 0)
			{
				r = 1;
				ActionManager::GetInstance().push(m_align_rot_action);
			}

			if (p && r)
			{
				m_align_pos_action->set_Stacked_Init(true);
				m_align_pos_action->set_Stacked(true);
				m_align_rot_action->set_Stacked_End(true);
				m_align_rot_action->set_Stacked(true);
			}

			m_align_pos_action = 0;
			m_align_rot_action = 0;

			m_align_target = 0;
			m_align_object = 0;

			//m_pos_align_enabled = true;
			//m_rot_align_enabled = true;

			m_popup_object_align_window = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(0.0f, 0.0f));
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(70, 22)) || key_data[GLFW_KEY_ESCAPE].pressed)
		{
			m_axis_x = m_axis_y = m_axis_z = 0;
			m_pos_x = m_pos_y = m_pos_z = 1;

			// undo k delete ta local actions
			if (m_align_pos_action != 0)
			{
				m_align_pos_action->undo();
				delete m_align_pos_action;
				m_align_pos_action = 0;
			}
			if (m_align_rot_action != 0)
			{
				m_align_rot_action->undo();
				delete m_align_rot_action;
				m_align_rot_action = 0;
			}


			m_align_target = 0;
			m_align_object = 0;

			//m_pos_align_enabled = true;
			//m_rot_align_enabled = true;

			m_popup_object_align_window = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::GetStyle().Colors[ImGuiCol_Text] = col;
		ImGui::GetStyle().FramePadding.y = fp_y;
		ImGui::EndPopup();
	}

	ImGui::PopFont();
	ImGui::End();


	if (decided)
	{
		//handle_CLONE(approve, as_instance, m_cloned_object_pack, m_prev_selected_object_pack, cloning_object_name);
	}

	if (m_popup_object_align_toggled) 
		m_popup_object_align_toggled = false;
	return true;


}
bool InterfaceManager::pop_Camera_Align_Window()
{
	return false;
}

void InterfaceManager::popup_Tooltip(ImVec2 pos, std::string label)
{
	m_popup_tooltip = true;
	tooltip_label = label;
	tooltip_pos = pos;
}
void InterfaceManager::show_Tooltip()
{
	guiStyleManager::setFont( ROBOTO, "Regular", 16);
	int width = tooltip_label.size() * 8.0f;
	ImVec4 col = ImGui::GetStyle().Colors[ImGuiCol_Text];
	ImVec4 bg = ImGui::GetStyle().Colors[ImGuiCol_PopupBg];

	float fp = ImGui::GetStyle().FramePadding.y;
	float wp = ImGui::GetStyle().WindowPadding.y;

	ImGui::GetStyle().FramePadding.y  = 2;
	ImGui::GetStyle().WindowPadding.y = 4;

	
	ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImGui::SetNextWindowBgAlpha(1.f);
	ImGui::BeginTooltip();
	
	ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	

	ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f); //ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImGui::Text(tooltip_label.c_str());
	ImGui::GetStyle().Colors[ImGuiCol_Text] = col;
	ImGui::EndTooltip();

	ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = bg;

	ImGui::GetStyle().FramePadding.y = fp;
	ImGui::GetStyle().WindowPadding.y = wp;
	ImGui::PopFont();

	m_popup_tooltip = false;
}
void InterfaceManager::show_Tooltip(std::string label)
{
	InterfaceManager::GetInstance().gui_set_Font(0, "Regular", 14);
	int width = label.size() * 8.0f;

	ImGui::GetStyle().FramePadding.y = 0;
	ImGui::GetStyle().WindowPadding.y = 0;


	ImVec2 p = ImGui::GetCursorPos();
	ImGui::SetNextWindowPos(p + ImVec2(5.0f, 5.0f));
	ImGui::SetNextWindowSize(ImVec2(200.0f, 25.0f));


	float fp = ImGui::GetStyle().FramePadding.y;
	float wp = ImGui::GetStyle().WindowPadding.y;

	ImGui::SetNextWindowBgAlpha(1.f);
	ImGui::Begin("tooltip test#");
	{
		ImGui::Button(label.c_str(), ImVec2(width, 20.0f));
	}
	ImGui::End();


	ImGui::GetStyle().FramePadding.y = fp;
	ImGui::GetStyle().WindowPadding.y = wp;

	ImGui::PopFont();
}

void InterfaceManager::construct_Scene_Import_Reporting_Window()
{

	int state;
	std::string msg = "";
	Mediator::Request_Scene_Import_State(state, msg);

	ImGui::SetNextWindowPos(ImVec2(200, 200));
	ImGui::SetNextWindowSize(ImVec2(400, 200));
	ImGui::Begin(" Thread Test Window ");

	ImGui::Text(" msg : "); ImGui::SameLine(); ImGui::Text(msg.c_str());
	ImGui::End();

}

void InterfaceManager::construct_Child_Window_3f_var(std::string title, ImVec2 size, std::string& var_title, optix::float3& var, bool &is_changed, bool is_vertical, bool is_colorEdit)
{
	
}
void InterfaceManager::construct_Child_Window_1f_var(std::string title, ImVec2 size, std::string& var_title, float& var, bool &is_changed, bool is_colorEdit)
{
	const char * window_title = (title + "##").c_str();
	const char * var_title_c  = (var_title + "##").c_str();

	{
		guiStyleManager::setStyle("main_style_right_child_transformations");
		ImGui::BeginChild(window_title, size , true, 0);

		guiStyleManager::setFont(guiFont::ROBOTO, "Bold", 16);
		ImGui::Text(" Specular ");
		ImGui::SameLine();
		ImGui::GetStyle().FrameRounding -= 3;
		ImGui::GetStyle().FramePadding -= ImVec2(1.0f, 1.0f);

		
		ImGui::GetStyle().FrameRounding += 3;
		ImGui::GetStyle().FramePadding += ImVec2(1.0f, 1.0f);
		ImGui::PopFont();

		ImGui::Dummy(ImVec2(0, 10.0f));
		guiStyleManager::setFont(guiFont::ROBOTO, "Regular", 16);
		
		ImGui::Dummy(ImVec2(0.0f, 2.0f));
		ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 2.0f - 65.0f, 0.0f));
		ImGui::SameLine();
		ImGui::PushItemWidth(100.0f);
		if (ImGui::DragFloat( var_title_c , &var , 0.05f, -999999.0f, 9999999.0f))
			is_changed = true;
		ImGui::PopItemWidth();
		
		ImGui::PopFont();

		ImGui::EndChild();
		guiStyleManager::setStyle("main_style_right_child2");
	}


	
}

void InterfaceManager::edit_style()
{
	
	guiStyleManager::setStyle("main_style");

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

	ImGui::SameLine();
	ImGui::Separator();

	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(500, 500), ImGuiCond_Once);

	guiStyleManager::setFont(guiFont::ROBOTO,"Regular",14);
	if (ImGui::BeginTabBar("tabs222sdasdasd##", ImGuiTabBarFlags_None))
	{
		std::string style_id;
		ImGuiStyle& style = ImGui::GetStyle();
		guiStyleManager::StyleInstance * style_inst;

		std::string label_sizes, label_colors;

		style_id = "main_style_right_child2";
		label_sizes = style_id + "_sizes";
		label_colors = style_id + "_colors";
		style_inst = &guiStyleManager::styleInstances[style_id.c_str()];
		if (ImGui::BeginTabItem(label_sizes.c_str()))
		{

			ImGui::Text("Main");


			ImGui::SliderFloat2("WindowPadding", (float*)&style_inst->windowPadding, 0.0f, 20.0f, "%.0f");

			//ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style_inst->FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style_inst->ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style_inst->ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style_inst->TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style_inst->IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style_inst->ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style_inst->GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::Text("Borders");
			ImGui::SliderFloat("WindowBorderSize", &style_inst->WindowBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("ChildBorderSize", &style_inst->ChildBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("PopupBorderSize", &style_inst->PopupBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("FrameBorderSize", &style_inst->FrameBorderSize, 0.0f, 1.0f, "%.0f");
			//ImGui::SliderFloat("TabBorderSize", &style_inst.TabBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::Text("Rounding");
			ImGui::SliderFloat("WindowRounding", &style_inst->WindowRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ChildRounding", &style_inst->ChildRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style_inst->FrameRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("PopupRounding", &style_inst->PopupRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style_inst->ScrollbarRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style_inst->GrabRounding, 0.0f, 12.0f, "%.0f");
			//ImGui::SliderFloat("TabRounding", &style_inst.TabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::Text("Alignment");
			ImGui::SliderFloat2("WindowTitleAlign", (float*)&style_inst->WindowTitleAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("ButtonTextAlign", (float*)&style_inst.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("SelectableTextAlign", (float*)&style_inst.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine();
			ImGui::Text("Safe Area Padding"); ImGui::SameLine();
			ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style_inst->DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(label_colors.c_str()))
		{
			static int output_dest = 0;
			static bool output_only_modified = true;
			if (ImGui::Button("Export Unsaved"))
			{
				guiStyleManager::exportStyleToFile("test_file");
			}
			ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = 0;
			ImGui::RadioButton("Opaque", &alpha_flags, 0); ImGui::SameLine();
			ImGui::RadioButton("Alpha", &alpha_flags, ImGuiColorEditFlags_AlphaPreview); ImGui::SameLine();
			ImGui::RadioButton("Both", &alpha_flags, ImGuiColorEditFlags_AlphaPreviewHalf); ImGui::SameLine();

			ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
			ImGui::PushItemWidth(-160);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4("##color", (float*)&(*style_inst)[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);

				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(name);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();

			ImGui::EndTabItem();
		}


		style_id = "main_style2";
		label_sizes = style_id + "_sizes";
		label_colors = style_id + "_colors";
		style_inst = &guiStyleManager::styleInstances[style_id.c_str()];
		if (ImGui::BeginTabItem(label_sizes.c_str()))
		{

			ImGui::Text("Main");


			ImGui::SliderFloat2("WindowPadding", (float*)&style_inst->windowPadding, 0.0f, 20.0f, "%.0f");

			//ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style_inst->FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style_inst->ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style_inst->ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style_inst->TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style_inst->IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style_inst->ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style_inst->GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::Text("Borders");
			ImGui::SliderFloat("WindowBorderSize", &style_inst->WindowBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("ChildBorderSize", &style_inst->ChildBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("PopupBorderSize", &style_inst->PopupBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("FrameBorderSize", &style_inst->FrameBorderSize, 0.0f, 1.0f, "%.0f");
			//ImGui::SliderFloat("TabBorderSize", &style_inst.TabBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::Text("Rounding");
			ImGui::SliderFloat("WindowRounding", &style_inst->WindowRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ChildRounding", &style_inst->ChildRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style_inst->FrameRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("PopupRounding", &style_inst->PopupRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style_inst->ScrollbarRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style_inst->GrabRounding, 0.0f, 12.0f, "%.0f");
			//ImGui::SliderFloat("TabRounding", &style_inst.TabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::Text("Alignment");
			ImGui::SliderFloat2("WindowTitleAlign", (float*)&style_inst->WindowTitleAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("ButtonTextAlign", (float*)&style_inst.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("SelectableTextAlign", (float*)&style_inst.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine();
			ImGui::Text("Safe Area Padding"); ImGui::SameLine();
			ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style_inst->DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(label_colors.c_str()))
		{
			static int output_dest = 0;
			static bool output_only_modified = true;
			if (ImGui::Button("Export Unsaved"))
			{
				guiStyleManager::exportStyleToFile("test_file");
			}
			ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = 0;
			ImGui::RadioButton("Opaque", &alpha_flags, 0); ImGui::SameLine();
			ImGui::RadioButton("Alpha", &alpha_flags, ImGuiColorEditFlags_AlphaPreview); ImGui::SameLine();
			ImGui::RadioButton("Both", &alpha_flags, ImGuiColorEditFlags_AlphaPreviewHalf); ImGui::SameLine();

			ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
			ImGui::PushItemWidth(-160);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4("##color", (float*)&(*style_inst)[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);

				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(name);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();

			ImGui::EndTabItem();
		}


		style_id = "main_style_right_child_transformations3";
		label_sizes = style_id + "_sizes";
		label_colors = style_id + "_colors";
		style_inst = &guiStyleManager::styleInstances[style_id.c_str()];
		if (ImGui::BeginTabItem(label_sizes.c_str()))
		{

			ImGui::Text("Main");


			ImGui::SliderFloat2("WindowPadding", (float*)&style_inst->windowPadding, 0.0f, 20.0f, "%.0f");

			//ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style_inst->FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style_inst->ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style_inst->ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style_inst->TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style_inst->IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style_inst->ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style_inst->GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::Text("Borders");
			ImGui::SliderFloat("WindowBorderSize", &style_inst->WindowBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("ChildBorderSize", &style_inst->ChildBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("PopupBorderSize", &style_inst->PopupBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("FrameBorderSize", &style_inst->FrameBorderSize, 0.0f, 1.0f, "%.0f");
			//ImGui::SliderFloat("TabBorderSize", &style_inst.TabBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::Text("Rounding");
			ImGui::SliderFloat("WindowRounding", &style_inst->WindowRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ChildRounding", &style_inst->ChildRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style_inst->FrameRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("PopupRounding", &style_inst->PopupRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style_inst->ScrollbarRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style_inst->GrabRounding, 0.0f, 12.0f, "%.0f");
			//ImGui::SliderFloat("TabRounding", &style_inst.TabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::Text("Alignment");
			ImGui::SliderFloat2("WindowTitleAlign", (float*)&style_inst->WindowTitleAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("ButtonTextAlign", (float*)&style_inst.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("SelectableTextAlign", (float*)&style_inst.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine();
			ImGui::Text("Safe Area Padding"); ImGui::SameLine();
			ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style_inst->DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(label_colors.c_str()))
		{
			static int output_dest = 0;
			static bool output_only_modified = true;
			if (ImGui::Button("Export Unsaved"))
			{
				guiStyleManager::exportStyleToFile("test_file");
			}
			ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = 0;
			ImGui::RadioButton("Opaque", &alpha_flags, 0); ImGui::SameLine();
			ImGui::RadioButton("Alpha", &alpha_flags, ImGuiColorEditFlags_AlphaPreview); ImGui::SameLine();
			ImGui::RadioButton("Both", &alpha_flags, ImGuiColorEditFlags_AlphaPreviewHalf); ImGui::SameLine();

			ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
			ImGui::PushItemWidth(-160);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4("##color", (float*)&(*style_inst)[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);

				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(name);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

	
		style_id = "main_style_right_child_transformations";
		label_sizes = style_id + "_sizes";
		label_colors = style_id + "_colors";
		style_inst = &guiStyleManager::styleInstances[style_id.c_str()];
		if (ImGui::BeginTabItem(label_sizes.c_str()))
		{

			ImGui::Text("Main");


			ImGui::SliderFloat2("WindowPadding", (float*)&style_inst->windowPadding, 0.0f, 20.0f, "%.0f");

			//ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style_inst->FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style_inst->ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style_inst->ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style_inst->TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style_inst->IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style_inst->ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style_inst->GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::Text("Borders");
			ImGui::SliderFloat("WindowBorderSize", &style_inst->WindowBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("ChildBorderSize", &style_inst->ChildBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("PopupBorderSize", &style_inst->PopupBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("FrameBorderSize", &style_inst->FrameBorderSize, 0.0f, 1.0f, "%.0f");
			//ImGui::SliderFloat("TabBorderSize", &style_inst.TabBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::Text("Rounding");
			ImGui::SliderFloat("WindowRounding", &style_inst->WindowRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ChildRounding", &style_inst->ChildRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style_inst->FrameRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("PopupRounding", &style_inst->PopupRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style_inst->ScrollbarRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style_inst->GrabRounding, 0.0f, 12.0f, "%.0f");
			//ImGui::SliderFloat("TabRounding", &style_inst.TabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::Text("Alignment");
			ImGui::SliderFloat2("WindowTitleAlign", (float*)&style_inst->WindowTitleAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("ButtonTextAlign", (float*)&style_inst.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("SelectableTextAlign", (float*)&style_inst.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine();
			ImGui::Text("Safe Area Padding"); ImGui::SameLine();
			ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style_inst->DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(label_colors.c_str()))
		{
			static int output_dest = 0;
			static bool output_only_modified = true;
			if (ImGui::Button("Export Unsaved"))
			{
				guiStyleManager::exportStyleToFile("test_file");
			}
			ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = 0;
			ImGui::RadioButton("Opaque", &alpha_flags, 0); ImGui::SameLine();
			ImGui::RadioButton("Alpha", &alpha_flags, ImGuiColorEditFlags_AlphaPreview); ImGui::SameLine();
			ImGui::RadioButton("Both", &alpha_flags, ImGuiColorEditFlags_AlphaPreviewHalf); ImGui::SameLine();

			ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
			ImGui::PushItemWidth(-160);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4("##color", (float*)&(*style_inst)[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);

				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(name);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();

			ImGui::EndTabItem();
		}



		style_id = "main_style_right_child0";
		label_sizes = style_id + "_sizes";
		label_colors = style_id + "_colors";
		style_inst = &guiStyleManager::styleInstances[style_id.c_str()];
		if (ImGui::BeginTabItem(label_sizes.c_str()))
		{

			ImGui::Text("Main");


			ImGui::SliderFloat2("WindowPadding", (float*)&style_inst->windowPadding, 0.0f, 20.0f, "%.0f");

			//ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style_inst->FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style_inst->ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style_inst->ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style_inst->TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style_inst->IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style_inst->ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style_inst->GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::Text("Borders");
			ImGui::SliderFloat("WindowBorderSize", &style_inst->WindowBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("ChildBorderSize", &style_inst->ChildBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("PopupBorderSize", &style_inst->PopupBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("FrameBorderSize", &style_inst->FrameBorderSize, 0.0f, 1.0f, "%.0f");
			//ImGui::SliderFloat("TabBorderSize", &style_inst.TabBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::Text("Rounding");
			ImGui::SliderFloat("WindowRounding", &style_inst->WindowRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ChildRounding", &style_inst->ChildRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style_inst->FrameRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("PopupRounding", &style_inst->PopupRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style_inst->ScrollbarRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style_inst->GrabRounding, 0.0f, 12.0f, "%.0f");
			//ImGui::SliderFloat("TabRounding", &style_inst.TabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::Text("Alignment");
			ImGui::SliderFloat2("WindowTitleAlign", (float*)&style_inst->WindowTitleAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("ButtonTextAlign", (float*)&style_inst.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
			//ImGui::SliderFloat2("SelectableTextAlign", (float*)&style_inst.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine();
			ImGui::Text("Safe Area Padding"); ImGui::SameLine();
			ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style_inst->DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(label_colors.c_str()))
		{
			static int output_dest = 0;
			static bool output_only_modified = true;
			if (ImGui::Button("Export Unsaved"))
			{
				guiStyleManager::exportStyleToFile("test_file");
			}
			ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = 0;
			ImGui::RadioButton("Opaque", &alpha_flags, 0); ImGui::SameLine();
			ImGui::RadioButton("Alpha", &alpha_flags, ImGuiColorEditFlags_AlphaPreview); ImGui::SameLine();
			ImGui::RadioButton("Both", &alpha_flags, ImGuiColorEditFlags_AlphaPreviewHalf); ImGui::SameLine();

			ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
			ImGui::PushItemWidth(-160);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4("##color", (float*)&(*style_inst)[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);

				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(name);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	ImGui::PopItemWidth();
	ImGui::PopFont();

}


#endif

#ifdef INTERFACE_UPDATE_FUNCTIONS

void InterfaceManager::Update(float dt)
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "Update", "");
#endif

	////std::cout << "\n - Interface::Update():" << std::endl;

	Catch_User_Input_General_Callback_Func();

	Update_WindowSize();

	timer = dt;

	if (m_skip_user_input)
	{
		////std::cout << "   - m_skip_user_input : " << m_skip_user_input << std::endl;
		return;
	}

	// Update Io (Mouse , Keyboards, etc ) events
	mouse_data = Utilities::getMouseData(mouse_data);
	Utilities::getKeyData(&key_data, Mediator::RequestWindow());

#define manual_cursor_repositioning
#ifdef manual_cursor_repositioning
	m_repositioning_cursor = false;

	bool clicked_over_menu_bar = (mouse_data.m0_clicked && fabsf(mouse_data.y) <= 2) ? true : false;
	bool transforming = m_selected_utility.group == g_TRANSFORMATIONS && m_selected_utility.id != u_TRANSLATION;
	if (   ((mouse_data.m0_down &&  m_m0_down_on_camera_control_handle)
		|| (mouse_data.m0_down && !m_selected_utility.active && !m_m0_down_on_gui)
		|| (mouse_data.m0_down && m_selected_utility.active && transforming && !m_m0_down_on_gui)
		|| (m_dragFloat_state))
		&& !clicked_over_menu_bar
		)
	{

		optix::float2 w = optix::make_float2(Mediator::RequestWindowSize(0), Mediator::RequestWindowSize(1));
		optix::float2 curr = optix::make_float2(mouse_data.x, mouse_data.y);
		optix::float2 prev = optix::make_float2(mouse_data.prev_x, mouse_data.prev_y);
		optix::float2 delta = optix::make_float2(mouse_data.dx, mouse_data.dy);

		bool needs_repositioning = false;

		// case [y]
		if      (curr.y <= 0)
		{
			curr.y = w.y - delta.y - 25;
			prev.y = w.y - 25;
			mouse_data.x = curr.x;
			mouse_data.y = curr.y;
			mouse_data.prev_x = prev.x;
			mouse_data.prev_y = prev.y;

			needs_repositioning = true;
		}
		else if (curr.y > w.y - 5)
		{
			curr.y = 0 + delta.y + 5;
			prev.y = 5;
			mouse_data.x = curr.x;
			mouse_data.y = curr.y;
			mouse_data.prev_x = prev.x;
			mouse_data.prev_y = prev.y;

			needs_repositioning = true;
		}

		// case [x]
		else if (curr.x <= 0)
		{
			curr.x = w.x - delta.x - 25;
			prev.x = w.x - 25;
			mouse_data.x = curr.x;
			mouse_data.y = curr.y;
			mouse_data.prev_x = prev.x;
			mouse_data.prev_y = prev.y;

			needs_repositioning = true;
		}
		else if (curr.x > w.x - 5)
		{
			
			curr.x = delta.x + 5;
			prev.x = 5;
			mouse_data.x = curr.x;
			mouse_data.y = curr.y;
			mouse_data.prev_x = prev.x;
			mouse_data.prev_y = prev.y;

			needs_repositioning = true;
		}

		if (needs_repositioning)
		{
			m_repositioning_cursor = true;
			Mediator::RequestCameraInstance().setBaseCoordinates(prev.x, prev.y);
			glfwSetCursorPos(Mediator::RequestWindow(), curr.x, curr.y);
		}

	}
	
#endif

	Update_User_Input();

	
	
	// proceed with intersection routines with utility-gui-objects
	// check for utility interaction only if active-utility-manipulation != true
	if (  !m_selected_utility.active
		|| m_selected_utility.group == g_ATTACH
		|| m_selected_utility.group == g_CREATE)
	{
		if (ViewportManager::GetInstance().getFocusedViewport() != ViewportManager::GetInstance().getActiveViewport().id
			&& m_selected_utility.group != g_CREATE
			)
			m_utility_interaction = false;
		else
			m_utility_interaction = get_Utility_Interaction();

	}
	
	
	// Update current interface state data (for --> OpenGL_Renderer)
	Update_current_Interface_State();
	
	
	Update_ViewportManager();
	//OpenGLRenderer now gets updated only before rendering..
	//Update_Renderer_Data();
	//OpenGLRenderer::GetInstace().Update(dt);
	

	//if (print_debug_info)
	if (key_data[GLFW_KEY_V].pressed)
	{
		//print_Object_TRS_info();
		//print_SceneObjects_Info();
	}
	//if (key_data[GLFW_KEY_B].pressed)
	//	print_Debug_Info(1);

	//m_ui_focused = false;

	if (m_skip_frame)
		m_skip_frame = false;


	
	
}
void InterfaceManager::Update_WindowSize()
{
	m_window_width  = Mediator::RequestWindowSize(0);
	m_window_height = Mediator::RequestWindowSize(1);
}
void InterfaceManager::Update_group_button_state(Utility_Group group, Utility selected_utility)
{
	// //std::cout << "\n Update_group_button_state( group = " << group << " , Utility = " << selected_utility << " ) " << std::endl;
	for (auto& it : tool_buttons)
	{
		
		Button * button = it.second;
		if (button->ut_group == group && button->utility != selected_utility)
		{
			button->selected = false;
		}

		
		if (button->utility != selected_utility)
		{
			bool ignore_callibration = group == g_CALLIBRATION || button->ut_group == g_CALLIBRATION;
			bool ignore_scene_mode   = group == g_SCENE_MODE   || button->ut_group == g_SCENE_MODE;
			bool ignore_g_lights     = group == g_LIGHTS_TOGGLE || button->ut_group == g_LIGHTS_TOGGLE;
			bool ignore_selection    = selected_utility == u_SELECTION || selected_utility == u_SELECTION_GROUP || button->utility == u_SELECTION || button->utility == u_SELECTION_GROUP;
			bool ignore_unlink       = selected_utility == u_UNLINK;
			bool ignore = ignore_callibration || ignore_selection || ignore_unlink || ignore_scene_mode || ignore_g_lights;

			if( !ignore )
				button->selected = false;
		}

	

	}

}

void InterfaceManager::Update_User_Input()
{

#define MOUSE_BUTTON_2


	

	
	//if ( (m_gui_focused || m_popup_open) && !(m_force_input_update || m_ui_focused) && m_m0_down_on_gui )
	if(m_popup_open)
	{
		Update_Mouse_Wheel_Input();
		return;
	}
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "handle_User_Input", "");
#endif

	// get focused object from SceneManager
	//m_prev_selected_object_pack.raw_object   = m_selected_object_pack.raw_object;
	//m_prev_selected_object_pack.object_index = m_selected_object_pack.object_index; // EDW //
	m_prev_INTERFACE_selected_object_pack.raw_object   = m_selected_object_pack.raw_object;
	m_prev_INTERFACE_selected_object_pack.object_index = m_selected_object_pack.object_index; // EDW //
	

	m_focused_object_pack.raw_object   = Mediator::RequestFocusedObject();
	m_focused_object_pack.object_index = Mediator::RequestFocusedObjectIndex();
	m_focused_object_pack.selected     = false;
	process_FocusedObjectPack( m_focused_object_pack );
	check_viewports_SELECT_FOCUS_state();

	m_prev_INTERFACE_focused_object_pack.raw_object   = m_focused_object_pack.raw_object;
	m_prev_INTERFACE_focused_object_pack.object_index = m_focused_object_pack.object_index;
	//m_ui_focused_object_id = m_focused_object_pack.raw_object == nullptr ? -1 : m_focused_object_pack.raw_object->getId();



#define KEY_INPUT_EVENTS
#ifdef KEY_INPUT_EVENTS


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	//   Update UI_KEY_GROUP Events [ EDIT, CLONE ]
	//
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ui_key_grp_state = UI_KEY_GRP_NONE; // reset UI_KEY_GRP_STATE to NONE

	// Key: [ LEFT_CONTROL ]
	// Grp: [ EDIT ]
	// Possible Actions : [ copy, paste, cut, undo, redo ]
	if (   key_data[GLFW_KEY_LEFT_CONTROL].pressed
		|| key_data[GLFW_KEY_RIGHT_CONTROL].pressed
		|| key_data[GLFW_KEY_LEFT_ALT].pressed
		|| key_data[GLFW_KEY_RIGHT_ALT].pressed
		)
	{
		ui_key_grp_state = UI_KEY_GRP_EDIT;
	}
	

	// Key: [ LEFT_SHIFT ]
	// Grp: [ CLONE ]
	// Possible Actions : [ ]
	if (key_data[GLFW_KEY_LEFT_SHIFT].pressed || key_data[GLFW_KEY_RIGHT_SHIFT].pressed)
	{
		ui_key_grp_state = UI_KEY_GRP_CLONE;
	}


	// Key: [ DELETE ]
	// Grp: [ DELETE ]
	// Possible Actions : [ ]
	if (key_data[GLFW_KEY_LEFT_SHIFT].pressed || key_data[GLFW_KEY_RIGHT_SHIFT].pressed)
	{
		ui_key_grp_state = UI_KEY_GRP_NONE;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


	


	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	//   Update UI_KEY_COMB Events [ copy, paste, ... ]
	//
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ui_key_comb_state = UI_KEY_COMB_NONE;

	
	// GROUP      : [ KEY_GRP_EDIT ]
	// PARENT_KEY : [ Ctrl ]
	// CHILD_KEYS : [ c, v, x, z, y ]
	// ACTIONS    : [ Copy, Paste, Cut, Undo, Redo ]
	if (ui_key_grp_state == UI_KEY_GRP_EDIT)
	{
		
		// Key:  [ C ]
		// Comb: [ Ctrl + C ]
		// Possible actions : [ copy ]
		if (key_data[GLFW_KEY_C].down)
		{
			ui_key_comb_state = UI_KEY_COMB_COPY;
		}

		
		// Key:  [ V ]
		// Comb: [ Ctrl + V ]
		// Possible actions : [ paste ]
		if (key_data[GLFW_KEY_V].down)
		{
			if (m_selected_object_pack.raw_object != nullptr )//&& ui_key_comb_state == UI_KEY_COMB_COPY)
			{
				
				//SceneObject * clone = new SceneObject(m_selected_object_pack.raw_object);
				//Mediator::addSceneObject(clone);
			}
			ui_key_comb_state = UI_KEY_COMB_PASTE;

		}

		
		// Key:  [ X ]
		// Comb: [ Ctrl + X ]
		// Possible actions : [ cut ]
		if (key_data[GLFW_KEY_X].down)
		{
			ui_key_comb_state = UI_KEY_COMB_CUT;
		}

		
		// Key:  [ Z ]
		// Comb: [ Ctrl + Z ]
		// Possible actions : [ undo ]
		
		
		if ( key_data[GLFW_KEY_Z].down )
		{
			ui_key_comb_state = UI_KEY_COMB_UNDO;
			if (!m_selected_utility.active)
			{
				handle_UNDO();
			}
		}

		
		// Key:  [ Y ]
		// Comb: [ Ctrl + Y ]
		// Possible actions : [ redo ]
		if ( key_data[GLFW_KEY_Y].down )
		{
			ui_key_comb_state = UI_KEY_COMB_REDO;
			if (!m_selected_utility.active)
			{
				handle_REDO();
			}
		}

	}
	

	
	if (mouse_data.m4_pressed)
		handle_UNDO();
	if (mouse_data.m5_pressed)
		handle_REDO();

	// GROUP      : [ KEY_GRP_CLONE ]
	// PARENT_KEY : [ Shift ]
	// CHILD_KEYS : []
	// ACTIONS    : []
	if (ui_key_grp_state == UI_KEY_GRP_CLONE)
	{

	}
	

	// GROUP      : [ KEY_GRP_NONE ]
	// PARENT_KEY : []
	// CHILD_KEYS : [ Delete, Enter ]
	// ACTIONS    : [ Delete, Accept ]
	if (ui_key_grp_state == UI_KEY_GRP_NONE)
	{

		// Key:  [ Delete ]
		// Comb: [ ]
		// Possible actions : [ delete ]
		if (key_data[GLFW_KEY_DELETE].down)
		{
			ui_key_comb_state = UI_KEY_COMB_DELETE;
			handle_DELETE();
		}


		// Key:  [ Enter ]
		// Comb: [ ]
		// Possible actions : [ Accept ]
		if (key_data[GLFW_KEY_ENTER].down )
		{
			ui_key_comb_state = UI_KEY_COMB_ACCEPT;
		}


		// Key:  [ Escape ]
		// Comb: [ ]
		// Possible actions : [ Cancel ]
		if (key_data[GLFW_KEY_ESCAPE].down)
		{
			ui_key_comb_state = UI_KEY_COMB_CANCEL;
		}

	}


	int v_state = glfwGetKey(Mediator::RequestWindow(), GLFW_KEY_V);
	if (v_state == GLFW_PRESS)
		print_debug_info = true;
	else
		print_debug_info = false;
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	if (!m_selected_utility.active)
	{
		int ctrl = glfwGetKey(Mediator::RequestWindow(), GLFW_KEY_LEFT_CONTROL);
		int z = glfwGetKey(Mediator::RequestWindow(), GLFW_KEY_Z);
		int y = glfwGetKey(Mediator::RequestWindow(), GLFW_KEY_Y);
		if (ctrl == GLFW_PRESS && z == GLFW_PRESS)
		{
			//ActionManager::GetInstance().undo();
			//Mediator::restartAccumulation();
		}
		else if (ctrl == GLFW_PRESS && y == GLFW_PRESS)
		{
			//ActionManager::GetInstance().redo();
			//Mediator::restartAccumulation();
		}
	}

#endif


#define MOUSE_INPUT_EVENTS
#ifdef MOUSE_INPUT_EVENTS


   
   #ifdef MOUSE_BUTTON_2
	Update_Mouse_Wheel_Input();
   #endif

   #define MOUSE_BUTTON_0
   #ifdef MOUSE_BUTTON_0

	


	/*
	// - - - - - - - - - - - - - - - - - - - - - - - - -
		** [MOUSE_EVENT] :: mouse(0) => (( DOUBLE_CLICKED )) **
	// - - - - - - - - - - - - - - - - - - - - - - - - -

		~  ||ACTIONS|| :

			 o - focus camera on selected object

	// - - - - - - - - - - - - - - - - - - - - - - - - - */
	if (mouse_data.m0_double_clicked)
	{

		// o ACTION : [ focus camera on selected object ]
		if (   m_focused_object_pack.raw_object != nullptr
			&& m_focused_object_pack.object_index == m_selected_object_pack.object_index)
		{
			
			optix::float3 orbit_center = m_selected_object_pack.raw_object->getCentroid_Transformed( true );
			bool in_frustum = !Utilities::frustumCull(orbit_center, ViewportManager::GetInstance().getActiveViewport());
			if (in_frustum 
				&& !m_selected_utility.active
				&& !key_data[GLFW_KEY_SPACE].pressed) 
			{ 
				ViewportManager::GetInstance().getActiveViewportCamera().setOrbitCenter(orbit_center, false, true);
			}

			ViewportManager::GetInstance().Update_Camera_Viewports();
			//ViewportManager::GetInstance().Update_Cameras()
			mouse_data.m0_down = false;
			mouse_data.m0_released = false;
			m_selected_utility.active = false;
			m_utility_interaction = false;
			m_retargeting_camera = true;
		}
	}




	/*
	// - - - - - - - - - - - - - - - - - - - - - - - - -
	** [MOUSE_EVENT] :: mouse(0) => (( DOWN )) **
	// - - - - - - - - - - - - - - - - - - - - - - - - -

	~  ||ACTIONS|| :

#ifdef OBJECT_SELECT_ON_MOUSE_PRESS
	o - single selection
	o - multiple selection ( Ctrl - is - pressed )
#endif

	o - activate object_manipulation in case utility interaction is true
	o - clone selected object if shift is pressed while interacting
	o - activate object_manipulation if selected obj is focused and selected_utility_plane != NONE
	o - link subject to -> object if <Linking> is activated
	o - activate Area Selection if it is current selected utility
	o - get focused Viewport
	//o - set camera's new orbit center if object

	
	// - - - - - - - - - - - - - - - - - - - - - - - - - */
	if (mouse_data.m0_down)
	{
		handle_m0_DOWN();
	}



	/*
	// - - - - - - - - - - - - - - - - - - - - - - - - -
		** [MOUSE_EVENT] :: mouse(0) => (( RELEASED )) **
	// - - - - - - - - - - - - - - - - - - - - - - - - -

		~  ||ACTIONS|| :

		#ifdef OBJECT_SELECT_ON_MOUSE_RELEASE
			 o - single selection
			 o - multiple selection ( Ctrl - is - pressed )
		#endif

			 o - handle object cloning in case there was no actual delta -> cancel cloning
			 o - retarget camera
			 o - "release object"
			 o - reset object_manipulation_data
			 o - set selected utility
			 o - push new Action to ActionManager
			 o - reset selected object
			 o - link subject to object if linking is active
			 o - finish with Area Selection if it was already active
			 o - release viewport camera

	// - - - - - - - - - - - - - - - - - - - - - - - - - */
	if (mouse_data.m0_released)
	{
		handle_m0_RELEASED();
	}



   #endif 

	

   #define MOUSE_BUTTON_1
   #ifdef MOUSE_BUTTON_1


	/*

	// - - - - - - - - - - - - - - - - - - - - - - - - -
	** [MOUSE_EVENT] :: mouse(1) => (( DOWN )) **
	// - - - - - - - - - - - - - - - - - - - - - - - - -

	~  ||ACTIONS|| :

	o -

	// - - - - - - - - - - - - - - - - - - - - - - - - - */
	if (mouse_data.m1_down)
	{
		//handle_m1_DOWN();
	}




	/*
	// - - - - - - - - - - - - - - - - - - - - - - - - -
		** [MOUSE_EVENT] :: mouse(1) => (( RELEASED )) **
	// - - - - - - - - - - - - - - - - - - - - - - - - -

		~  ||ACTIONS|| :

			 o - clear single selection
			 o - clear multiple_selection list
			 o - show object's properties window
			 o - reset m_selected_utility_plane if object is deselected

	// - - - - - - - - - - - - - - - - - - - - - - - - - */
	if (mouse_data.m1_released)
	{
		handle_m1_RELEASED();
	}

   #endif

	


#endif

	// Update app's global selected object
	// if Area_Selection : active -> select by id 
	// else : select by array index
	
	bool is_group = m_selected_object_pack.raw_object == nullptr ? false : m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY;
	if( m_selected_object_pack.raw_object != nullptr )
		m_selected_object_pack.object_index = Mediator::requestObjectsArrayIndex( m_selected_object_pack.raw_object );
	//if (!m_gui_focused)
	{
		
		if (m_multiple_selection_active || is_group)
		{
			int object_id = m_selected_object_pack.raw_object->getId();
			Mediator::setSelectedObject(object_id, true);
		}
		else
			Mediator::setSelectedObject(m_selected_object_pack.object_index);
	}


	//if (m_force_input_update)
		m_force_input_update = false;

		
	
}
void InterfaceManager::Update_Mouse_Wheel_Input()
{
	// mouse_wheel //
	if (mouse_data.m_wheel != 0)
	{
		m_m0_down_on_gui = false;
		if (m_gui_focused || m_ui_focused)
			mouse_data.m_wheel = 0;
	}

	// mouse_wheel down //
	if (mouse_data.m2_down)
	{
		m_m0_down_on_gui = false;
		if (m_focused_object_pack.raw_object != nullptr)
			mouse_data.m2_down_on_focused_object = true;
		else
			mouse_data.m2_down_on_focused_object = false;

		if (m_selected_object_pack.raw_object != nullptr)
			if (m_selected_object_pack.object_index == m_focused_object_pack.object_index)
				mouse_data.m2_down_on_selected_object = true;
			else
				mouse_data.m2_down_on_selected_object = false;
		else
			mouse_data.m2_down_on_selected_object = false;

		if (m_gui_focused || m_ui_focused || m_wait_to_release_m2)
		{
			mouse_data.m2_down   = false;
			m_wait_to_release_m2 = true;
		}

		
		if (!mouse_data.prev_m0_down)
		{
			if (m_gui_focused || m_focusing_menu_bar_items)
				m_m2_down_on_gui = true;
		}

	}


	//if(ImGui::IsMouseReleased(2))

	// mouse_wheel released //
	if (mouse_data.m2_released)
	{
		
		m_m2_down_on_gui = false;
		m_wait_to_release_m2 = false;
		mouse_data.m2_down_on_focused_object = false;
		mouse_data.m2_down_on_selected_object = false;
	}

}
void InterfaceManager::Update_current_Interface_State()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "Update_current_Interface_State", "");
#endif

	m_interface_state.isec_p                    = &m_utility_isec_p;
	m_interface_state.active_utility            = &m_selected_utility;
	m_interface_state.selected_object_pack      = &m_selected_object_pack;
	m_interface_state.active_axis_orientation   = m_selected_axis_orientation;
	m_interface_state.axis_orientation_changed  = m_selected_axis_orientation_changed;
	m_interface_state.selected_object_changed   = false;//m_selected_object_changed;
	m_interface_state.m_camera_inv_scale_factor = m_renderer_camera_inv_scale_factor;
	m_interface_state.m_gui_transforming        = (m_gui_transforming[0] || m_gui_transforming[1] || m_gui_transforming[2]);

}

void InterfaceManager::Update_ViewportManager()
{
	
	// set Active Viewport 
	if (
		mouse_data.m0_clicked
		|| mouse_data.m1_clicked
		|| mouse_data.m_wheel != 0
		|| (mouse_data.m2_down && !mouse_data.m2_down_prev)
		&& !m_gui_focused && !m_popup_open
		)
	{
		
		bool gui_interaction = false;
		if (mouse_data.m1_clicked || mouse_data.m2_down)
			gui_interaction = mouse_data.interaction_on_gui;

		if (   !gui_interaction 
			&& !m_focusing_menu_bar_items
			)
		{
			ViewportManager::GetInstance().ActivateFocusedViewport();
		}
	}

	////std::cout << "\n - Update_ViewportManager():" << std::endl;
	////std::cout << "   - m_selected_utility.active : " << m_selected_utility.active << std::endl;
	bool m0_down    = mouse_data.m0_down;
	bool m0_clicked = mouse_data.m0_clicked;
	bool m2_clicked = mouse_data.m2_down && !mouse_data.prev_m2_down;
	bool focus_bar  = m_focusing_menu_bar_items;
	bool hovering_over_gui = ImGui::IsMouseHoveringAnyWindow();


	////std::cout << "\n" << std::endl;
	////std::cout << " - gui_focused : " << m_gui_focused << std::endl;
	////std::cout << " - bar_focused : " << m_focusing_menu_bar_items << std::endl;
	////std::cout << " - AnyItemActive : " << ImGui::IsAnyItemActive() << std::endl;
	////std::cout << " - m0_down_on_gui : " << m_m0_down_on_gui << std::endl;
	////std::cout << " - m2_down_on_gui : " << m_m2_down_on_gui << std::endl;
	////std::cout << " - UPDATE : [ " << !(m_gui_focused || m_focusing_menu_bar_items) << std::endl;

	ViewportManager::GetInstance().Update( m_selected_utility.active, ( ((m_gui_focused || m_m0_down_on_gui || m_m2_down_on_gui) || m_popup_open) && !m_m0_down_on_camera_control_handle) );
	ViewportManager::GetInstance().setFocusedViewport( (VIEWPORT_ID)getFocusedViewport() );
	
	
	if( Mediator::is_AppWindowSizeChanged() )
		ViewportManager::GetInstance().Update_Viewports();

	
}
void InterfaceManager::Update_Renderer_Data()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "Update_Renderer_Data", "");
#endif

	
	
	Update_Renderer_RenderSize();
	//Update_Renderer_Viewport();

	Update_Renderer_Highlight_RenderList();
	Update_Renderer_Utility_RenderList();
	Update_Renderer_Utility_Cursor();

}

void InterfaceManager::Update_Renderer_RenderSize()
{

	// default renderSize = [ m_window_width, m_window_height ]
#if I_DEFAULT_VIEWPORT      == 1

	m_renderer_renderSize.x = m_window_width;
	m_renderer_renderSize.y = m_window_height;

#elif I_CUSTOM_VIEWPORT_TEST == 1

	// custom renderSize = [ m_visible_width, m_visible_height ]
	m_renderer_renderSize.x = m_window_width  - (imgui_windows[LEFT_PANEL].size.x + imgui_windows[RIGHT_PANEL].size.x);
	m_renderer_renderSize.y = m_window_height - (imgui_windows[LEFT_PANEL].pos.y + imgui_windows[BOTTOM_PANEL].size.y); //(imgui_windows[MAIN_TOOLS].size.y + imgui_windows[MAIN_TOOLS_HELPERS].size.y + imgui_windows[BOTTOM_PANEL].size.y);

#endif

	OpenGLRenderer::GetInstace().setRenderSize(m_renderer_renderSize.x, m_renderer_renderSize.y);
}
void InterfaceManager::Update_Renderer_Viewport()
{

	
	int screen_width  = m_window_width;
	int screen_height = m_window_height;

	int width  = screen_width  - (imgui_windows[LEFT_PANEL].size.x + imgui_windows[RIGHT_PANEL].size.x);
	int height = screen_height - (imgui_windows[LEFT_PANEL].pos.y); //(imgui_windows[MAIN_TOOLS].size.y + imgui_windows[MAIN_TOOLS_HELPERS].size.y + imgui_windows[BOTTOM_PANEL].size.y);

	int offset_x = imgui_windows[LEFT_PANEL].size.x;
	int offset_y = imgui_windows[BOTTOM_PANEL].size.y; //imgui_windows[MAIN_TOOLS].size.y + imgui_windows[MAIN_TOOLS_HELPERS].size.y;


	//OpenGLRenderer::GetInstace().setActiveViewPort( VIEWPORT_ID::MAIN_VIEWPORT );
	
#if I_DEFAULT_VIEWPORT       == 1

	//OpenGLRenderer::GetInstace().setViewport(0, 0, 0, m_renderer_renderSize.x, m_renderer_renderSize.y);

#elif I_CUSTOM_VIEWPORT_TEST == 1

	//OpenGLRenderer::GetInstace().setViewport( VIEWPORT_ID::MAIN_VIEWPORT, offset_x, offset_y , m_renderer_renderSize.x, m_renderer_renderSize.y);
	//OpenGLRenderer::GetInstace().setViewport(MAIN_VIEWPORT, 240, 80, height , width);

#endif

	

}

void InterfaceManager::Update_Renderer_Highlight_RenderList()
{

#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "Update_Renderer_Highlight_RenderList", "");
#endif

#ifdef PRINT_DEBUG_INFO_CLONE_DELETE
	//std::cout << "\n\nInterfaceManager::UPDATE_RENDERER_HIGHLIGHT_RENDERLIST" << std::endl;
#endif

	

	// case single selection - focus
	if (m_selected_object_pack.raw_object != nullptr
		//&& !m_selected_object_pack.raw_object->getType() != Type::DUMMY
		//&& !m_selected_object_pack.raw_object->getType() != Type::GROUP_PARENT_DUMMY
		)
	{
		if (m_selected_object_pack.raw_object->isActive() 
			&& m_selected_object_pack.raw_object->isVisible()
			)
		{
			OpenGLRenderer::GetInstace().add_Object_To_HighlightList(&m_selected_object_pack, &Mediator::RequestCameraInstance());
			if ( 
				   m_selected_object_pack.raw_object->getType() == FACE_VECTOR
				|| m_selected_object_pack.raw_object->getType() == SAMPLER_OBJECT
				)
			{
				Object_Package parent_pack;
				parent_pack.raw_object   = m_selected_object_pack.raw_object->getParent();
				parent_pack.object_index = Mediator::requestObjectsArrayIndex(parent_pack.raw_object->getId());
				parent_pack.selected     = true;
				if(parent_pack.raw_object->isVisible())
					OpenGLRenderer::GetInstace().add_Object_To_HighlightList(&parent_pack, &Mediator::RequestCameraInstance());
			}
		}
	}


	if (m_focused_object_pack.raw_object != nullptr && m_focused_object_pack.object_index != m_selected_object_pack.object_index
		//&& !m_focused_object_pack.raw_object->getType() != Type::DUMMY
		//&& !m_focused_object_pack.raw_object->getType() != Type::GROUP_PARENT_DUMMY
		)
	{
		
		if (   m_focused_object_pack.raw_object->isActive()
			&& m_focused_object_pack.raw_object->isVisible()
			)
		{
			OpenGLRenderer::GetInstace().add_Object_To_HighlightList(&m_focused_object_pack, &Mediator::RequestCameraInstance());
			
			if (   m_focused_object_pack.raw_object->getType() == FACE_VECTOR
				|| m_focused_object_pack.raw_object->getType() == SAMPLER_OBJECT
				)
			{
				//std::cout << "\n - m_focused_object : " << m_focused_object_pack.raw_object->getName() << std::endl;
				//std::cout << " - m_focused_object type : " << m_focused_object_pack.raw_object->getType() << std::endl;
				//std::cout << " - m_focused_object parent : " << (m_focused_object_pack.raw_object->getParent() != nullptr) << std::endl;
				
				Object_Package temp_pack;
				temp_pack.raw_object   = m_focused_object_pack.raw_object->getParent();
				temp_pack.object_index = Mediator::requestObjectsArrayIndex(temp_pack.raw_object->getId());
				if(temp_pack.raw_object->isVisible())
					OpenGLRenderer::GetInstace().add_Object_To_HighlightList(&temp_pack, &Mediator::RequestCameraInstance());

			}
		}
	}

	if (m_utility_isec_p.is_lightButton_hit)
	{
		Object_Package temp_pack;
		int index = Mediator::requestObjectsArrayIndex(m_utility_isec_p.light_obj_id);
		temp_pack.raw_object   = Mediator::RequestSceneObjects()[index];
		temp_pack.object_index = index;
		if (temp_pack.raw_object->isVisible() && m_focused_object_pack.object_index != index 
			&& m_selected_object_pack.object_index != index)
			OpenGLRenderer::GetInstace().add_Object_To_HighlightList(&temp_pack, &Mediator::RequestCameraInstance());
	}

	
}
void InterfaceManager::Update_Renderer_Utility_RenderList()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "Update_Renderer_Utility_RenderList", "");
#endif

	switch ( m_selected_utility.group )
	{


	case g_SELECTIONS:
	{

		if (m_selected_utility.id == u_SELECTION_AREA && m_selected_utility.active == true)
		{

			optix::float2 a = optix::make_float2(mouse_data.m0_down_x, mouse_data.m0_down_y);
			optix::float2 b = optix::make_float2(mouse_data.x, mouse_data.y);

			a = Utilities::getMousePosRelativeToViewport(ViewportManager::GetInstance().getActiveViewport().id, a);
			b = Utilities::getMousePosRelativeToViewport(ViewportManager::GetInstance().getActiveViewport().id, b);

			Shape rect = Geometry::SHAPES::createRect2D(a, b);
			OpenGLRenderer::GetInstace().add_Shape2D_To_RenderList(rect, SHAPE_ID::RECTANGLE, &m_selected_utility);

		}
		
	}
	break;

	case g_TRANSFORMATIONS:
	{
		
		if ( m_selected_object_pack.raw_object == nullptr )
			return;
		
		
		calculate_Object_Pack_Data(&m_selected_object_pack, m_selected_utility);
		OpenGLRenderer::GetInstace().add_Utility_To_RenderList( &m_selected_utility, &m_selected_object_pack, nullptr , &Mediator::RequestCameraInstance() );
	}
	break;

	case g_LINKING:
	{
		
		if (m_selected_object_pack.raw_object == nullptr || m_selected_utility.active == false)
			return;

		OpenGLRenderer::GetInstace().add_Utility_To_RenderList(&m_selected_utility, &m_selected_object_pack, &m_focused_object_pack, &Mediator::RequestCameraInstance());
	}
	break;

	case g_ATTACH:
	{
		bool is_obj_valid = m_selected_object_pack.raw_object != nullptr;
		bool is_type_valid = true;
		if (is_obj_valid)
		{
			Type obj_type = m_selected_object_pack.raw_object->getType();
			bool is_temp  = m_selected_object_pack.raw_object->isTemporary();
			is_type_valid &= (obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT && !is_temp);
		}

		if ( is_obj_valid && is_type_valid )
		{
			bool is_attach_sampler = m_selected_utility.id == u_ATTACH_SAMPLER_PLANAR || m_selected_utility.id == u_ATTACH_SAMPLER_DIRECTIONAL || m_selected_utility.id == u_ATTACH_SAMPLER_VOLUME;
			if (!is_attach_sampler)
			{
				if (m_selected_utility.id == u_ATTACH_FACE_VECTOR_BBOX_ALIGNED || m_selected_utility.id == u_ATTACH_FACE_VECTOR_BBOX_ALIGNED_PARALLEL)
					OpenGLRenderer::GetInstace().add_Utility_To_RenderList(&m_selected_utility, &m_selected_object_pack, nullptr, &Mediator::RequestCameraInstance());
				else if (m_selected_utility.id == u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED)
				{
					if (m_focused_object_pack.object_index == m_selected_object_pack.object_index && !(m_ui_focused || m_gui_focused)
						|| m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY)
						OpenGLRenderer::GetInstace().add_Utility_To_RenderList(&m_selected_utility, &m_selected_object_pack, nullptr, &Mediator::RequestCameraInstance());
				}
			}
			else
			{
				OpenGLRenderer::GetInstace().add_Utility_To_RenderList(&m_selected_utility, &m_selected_object_pack, nullptr, &Mediator::RequestCameraInstance());
			}

		}
	}
	break;

	case g_CREATE:
	{
		OpenGLRenderer::GetInstace().add_Utility_To_RenderList(&m_selected_utility, &m_selected_object_pack, nullptr, &Mediator::RequestCameraInstance());
	}
	break;

	case g_RESTRICTIONS:
	{
		if (m_selected_object_pack.raw_object == nullptr || m_selected_utility.active == false)
			return;

		OpenGLRenderer::GetInstace().add_Utility_To_RenderList(&m_selected_utility, &m_selected_object_pack, &m_focused_object_pack, &Mediator::RequestCameraInstance());
	}
	break;


	case g_ALIGN:
	{
		if ( (m_selected_object_pack.raw_object == nullptr || (m_selected_utility.active == false && !m_popup_object_align_window))
			&& (m_selected_utility.id != u_ALIGN_CAMERA)
			)
			return;

		if      (m_selected_utility.id == u_ALIGN)
		{
			OpenGLRenderer::GetInstace().add_Utility_To_RenderList(&m_selected_utility, &m_selected_object_pack, &m_focused_object_pack, &Mediator::RequestCameraInstance());
		}
		else if (m_selected_utility.id == u_ALIGN_CAMERA)
		{
			OpenGLRenderer::GetInstace().add_Utility_To_RenderList(&m_selected_utility, &m_selected_object_pack, &m_focused_object_pack, &Mediator::RequestCameraInstance());
		}

	}
	break;

	}


	if ( m_selected_object_pack.raw_object != nullptr )
	{
		if ( m_selected_object_pack.raw_object->getType() == LIGHT_OBJECT )
		{
			OpenGLRenderer::GetInstace().add_Utility_To_RenderList(&m_selected_object_pack, UTIL_RENDER_TYPE::RENDER_LIGHT_OBJECT_PROPERTIES, &Mediator::RequestCameraInstance());
		}
	}


}
void InterfaceManager::Update_Renderer_Utility_Cursor()
{

	if (!m_selected_utility.active && mouse_data.m0_down && m_m0_down_on_camera_control_handle)
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		ImGui::GetIO().MouseDrawCursor = 0;
		OpenGLRenderer::GetInstace().set_Cursor_To_Render(CURSOR_TYPE::CURSOR_NONE);
		return;
	}

	if (m_dragFloat_state)
	{
		OpenGLRenderer::GetInstace().set_Cursor_To_Render(CURSOR_TYPE::CURSOR_NONE);
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		m_dragFloat_state = false;
		return;
	}

	
	if ( m_skip_user_input == 1 || m_gui_focused || m_ui_focused )
	{
		OpenGLRenderer::GetInstace().set_Cursor_To_Render(CURSOR_TYPE::CURSOR_NONE);
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
		return;
	}

	

	Camera_State cam_state = ViewportManager::GetInstance().getActiveViewportCamera().getState();
	if (cam_state == Camera_State::CAM_PAN)
	{
		
		CURSOR_TYPE cursor_type;
		cursor_type = CURSOR_TYPE::CURSOR_CAMERA_PAN;
		
		OpenGLRenderer::GetInstace().set_Cursor_To_Render( cursor_type );
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		return;
	}
	

	bool is_link_cursor = m_selected_utility.id == u_LINK || ( m_selected_utility.group == g_RESTRICTIONS );
	if ( ( is_link_cursor && m_selected_utility.active) && m_focused_object_pack.raw_object != nullptr )
	{
		if (m_focused_object_pack.object_index != m_selected_object_pack.object_index)
		{
			OpenGLRenderer::GetInstace().set_Cursor_To_Render(CURSOR_TYPE::CURSOR_LINK_ACTIVE);
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		}
		else 
		{
			OpenGLRenderer::GetInstace().set_Cursor_To_Render(CURSOR_TYPE::CURSOR_LINK);
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		}
		return;
	}
	else if ( is_link_cursor && m_focused_object_pack.raw_object == nullptr && m_selected_utility.active )
	{
		OpenGLRenderer::GetInstace().set_Cursor_To_Render(CURSOR_TYPE::CURSOR_NONE);
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
		return;
	}


	if (
		   //( // comment this if CURSOR_BUG occurs
		   m_selected_utility.active
		|| m_utility_interaction
		|| m_interface_state.selected_utility_plane != UtilityPlane::NONE
		 //)   // comment this if CURSOR_BUG occurs

		&& m_selected_object_pack.object_index == m_focused_object_pack.object_index
		&& m_focused_object_pack.object_index >= 0
		&& !m_utility_isec_p.is_lightButton_hit

		)
	{
		
		CURSOR_TYPE ct = Utilities::Utility_to_Cursor_Type( m_selected_utility );
		if (ct == CURSOR_TYPE::CURSOR_NONE)
			ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
		else
		{
			OpenGLRenderer::GetInstace().set_Cursor_To_Render(ct);
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		}
	}
	else
	{
		OpenGLRenderer::GetInstace().set_Cursor_To_Render(CURSOR_TYPE::CURSOR_NONE);
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
	}


}

void InterfaceManager::Catch_User_Input_General_Callback_Func()
{


	if (m_changing_name && !m_changing_name_is_active)
	{
		std::string name = std::string(object_name);
		if( name != "" )
			Utilities::setObjectName(m_selected_object_pack.raw_object,  name , true);
		m_changing_name = false;
	}

}

#endif

#ifdef INTERFACE_RENDER_FUNCTIONS

void InterfaceManager::Render_Interface(bool render_utility)
{

#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "Render_Interface", "");
#endif

	// Render Utility objects
	if(render_utility)
		Render_Utility();

	// NewFrame //
	m_window_width  = Mediator::RequestWindowSize(0);
	m_window_height = Mediator::RequestWindowSize(1);
	ImGui_ImplGlfwGL2_NewFrame();



	// Construct_Gui //
	construct_Interface();
	

#ifdef DEBUGGING
	//showDebugWindow();
#endif

	// Render //
	ImGui::Render();
	ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());

}
void InterfaceManager::Render_Utility()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "Render_Utility", "");
#endif

	Update_Renderer_Data();
	OpenGLRenderer::GetInstace().Update(timer);
	OpenGLRenderer::GetInstace().Render();
	OpenGLRenderer::GetInstace().Display();
}

#endif

#ifdef INTERFACE_HANDLE_FUNCTIONS

void InterfaceManager::HandleEvents()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "HandleEvents", "");
#endif

	if (m_skip_user_input)
		return;

	// handle user - utility Interaction
	handle_Utility_Interaction();

}
void InterfaceManager::handle_Utility_Interaction()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "handle_Utility_Interaction", "");
#endif


	if (!m_selected_utility.active)
		return;

	// case : g_TRANSFORMATIONS : { Translation, Rotation, Scale }
	if (m_selected_utility.group == g_TRANSFORMATIONS)
	{
		//if (!mouse_data.m0_dragging)
			//return;
		handle_TRANSFORMATION();
	}
	
	// case : g_LINKING : { Link, Unlink }
	else if (m_selected_utility.group == g_LINKING)
	{
		
	}


}

#endif

#ifdef INTERFACE_UTIL_FUNCTIONS


void InterfaceManager::test_thread_func()
{
	////std::cout << " - InterfaceManager::test_thread_func():" << std::endl;
	m_Scene_Import_STATE = 1;
	int state = -1;
	std::string msg = "";
	

	Mediator::Request_Scene_Import_State(state, msg);
	while (state != 1)
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
		Mediator::Request_Scene_Import_State(state, msg);
		
		ImGui_ImplGlfwGL2_NewFrame();

		construct_Interface();

		ImGui::Render();
		ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
	}

	m_Scene_Import_STATE = 0;

}
void InterfaceManager::IMPORT_SCENE_THREADED(const char * filename)
{
	
	// Mediator::Import_Scene_Threaded( filepath.c_str() )
}


void InterfaceManager::process_TAB_SELECTION_STATE_change(TAB_ID to_tab)
{
	m_UI_focused_Relation = nullptr;
	m_active_tab = to_tab;

	if (to_tab == TAB_ID::SAMPLER_PARAMS_TAB)
	{
		if (m_interface_state.m_sa == SA_BBOX)
			m_interface_state.m_sa = SA_NORMAL;
	}
}
void InterfaceManager::process_SELECTION_STATE_change(SceneObject* to_obj)
{

	

	// RESTRICTIONS //
	{
		Restriction* R = m_UI_selected_Restriction;
		bool is_R_selected = (R != nullptr);

		// if there is a restriction selected //
		if (is_R_selected && to_obj != nullptr)
		{
			std::vector<SceneObject*> relative_objects;
			Restriction_ID r_id = R->getId();

			if      (r_id == FOCUS_R)
			{
				relative_objects.push_back(R->getSubject());
				relative_objects.push_back(R->getObject());
				relative_objects.push_back(R->get_Owner(0));
				relative_objects.push_back(R->get_Owner(1));
				for (SceneObject* fv : R->get_Owner(0)->get_Face_Vectors()) relative_objects.push_back(fv);
			}
			else if (r_id == CONVERSATION_R)
			{
				relative_objects.push_back(R->getSubject());
				relative_objects.push_back(R->getObject());
				relative_objects.push_back(R->get_Owner(0));
				relative_objects.push_back(R->get_Owner(1));
				for (SceneObject* fv : R->get_Owner(0)->get_Face_Vectors()) relative_objects.push_back(fv);
				for (SceneObject* fv : R->get_Owner(1)->get_Face_Vectors()) relative_objects.push_back(fv);
			}
			else if (r_id == DISTANCE_R)
			{
				relative_objects.push_back(R->getSubject());
				relative_objects.push_back(R->getObject());
				relative_objects.push_back(R->get_Owner(0));
				relative_objects.push_back(R->get_Owner(1));
			}

			bool is_relative_obj = false;
			for (SceneObject* obj : relative_objects)
			{
				if (obj->getId() == to_obj->getId())
				{
					is_relative_obj = true;
					break;
				}
			}

			// new selected object is relative to prev //
			if (is_relative_obj)
			{

			}
			// !
			else
			{
				m_UI_selected_Restriction = nullptr;
				Mediator::Reset_Restrictions_UI_States();
			}
		}
		else if (is_R_selected && to_obj == nullptr)
		{
			m_UI_selected_Restriction = nullptr;
			Mediator::Reset_Restrictions_UI_States();
		}
	}




}
void InterfaceManager::process_SELECTION_STATE_change_post_order(SceneObject* to_obj)
{
	// g_Attach_Samplers //
	bool is_utility_valid = is_utility_toggle_valid(m_selected_utility.id);
	if (!is_utility_valid)
	{
		toggle(m_prev_selected_utility.id, false, true);
	}



	bool is_attach_samplers = (   m_selected_utility.id == u_ATTACH_SAMPLER_PLANAR
							   || m_selected_utility.id == u_ATTACH_SAMPLER_VOLUME
							   || m_selected_utility.id == u_ATTACH_SAMPLER_DIRECTIONAL);
	if ( is_attach_samplers )
	{
		int crt_type = m_sampler_crt_def_type;
		toggle( m_selected_utility.id, false, false );
		if (crt_type == 1) toggle_Attach_Sampler_Planar();
		if (crt_type == 2) toggle_Attach_Sampler_Volume();
		if (crt_type == 3) toggle_Attach_Sampler_Directional();
		m_sampler_crt_def_type = crt_type;
	}

}
void InterfaceManager::reset_Tabs_State( TAB_ID tab_id )
{
	if ( !m_force_tab_open_active || m_force_tab_open[tab_id] )
	{
		if (tab_id != RESTRICTIONS_TAB && tab_id != TRANSFORMATIONS_TAB)
		{
			m_UI_selected_Restriction = nullptr;
			Mediator::Reset_Restrictions_UI_States();
		}
	}
	else
	{

	}

}
void InterfaceManager::reset_Force_Tabs_Open_Flags()
{
	m_force_tab_open_active = false;
	for (int i = 0; i < 10; i++)
		m_force_tab_open[i] = false;
}
void InterfaceManager::calculate_Object_Pack_Data(Object_Package * obj_pack, UtilityProperties utility)
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "calculate_Object_Pack_Data", "");
#endif


	return;


	// case Utility_group = Transformations //
	if (utility.group == g_TRANSFORMATIONS)
	{

		SceneObject * raw_object = obj_pack->raw_object;
		// update selected object's base axis if rotation changed
		if (raw_object->isRotationChanged() || m_selected_axis_orientation_changed)
		{
			if (m_selected_axis_orientation != AXIS_ORIENTATION::SCREEN)
				BASE_TRANSFORM::AXIS(raw_object, m_selected_axis_orientation, &obj_pack->base_axis[0]);
	
		}


		glm::mat4 object_rotation_matrix = glm::mat4(1.0f);
		glm::vec3 world_pos = VECTOR_TYPE_TRANSFORM::TO_GLM_3f(raw_object->getTranslationAttributes("calc_obj_pack_data"));
		float obj_dist = raw_object->getDistanceFromCamera();
		optix::float3 delta = raw_object->getAttributesDelta(0);

		switch (m_selected_axis_orientation)
		{
		case AXIS_ORIENTATION::WORLD:
			object_rotation_matrix = glm::mat4(1.0f);
			break;
		case AXIS_ORIENTATION::LOCAL:
		{
#ifdef CANCER_DEBUG
			bool s = (raw_object == nullptr || raw_object == 0);
			
			if (s)
			{
				//std::cout << "\n InterfaceManager::calc_obj_data():" << std::endl;
				//std::cout << "          - obj : " << !s << std::endl;
				system("pause");
			}
#endif
			object_rotation_matrix = MATRIX_TYPE_TRANSFORM::TO_GLM_MATRIX(raw_object->getRotationMatrix_chain());
		}
			break;
		}

		

		float m_camera_inv_scale_factor;
		if ((utility.id == Utility::u_TRANSLATION || utility.id == Utility::u_SCALE) && (delta.x == 0 && delta.y == 0 && delta.z == 0))
			m_camera_inv_scale_factor = obj_dist * TRANSLATION_AXIS_SCALE_FACTOR;
		else if (utility.id == Utility::u_ROTATION)
			m_camera_inv_scale_factor = obj_dist * ROTATION_AXIS_SCALE_FACTOR;

		glm::mat4 object_transformation_matrix =
			glm::translate(glm::mat4(1.0f), world_pos)
			* object_rotation_matrix
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.6f * m_renderer_camera_inv_scale_factor));

		glm::mat4 object_def_transformation_matrix =
			glm::translate(glm::mat4(1.0f), world_pos)
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.6f * m_renderer_camera_inv_scale_factor));

		glm::mat4 object_translation_matrix_scaled =
			glm::translate(glm::mat4(1.0f), world_pos)
			* glm::scale(glm::mat4(1.0f), glm::vec3(1.3f * 0.6f * m_renderer_camera_inv_scale_factor));

		
		obj_pack->transformation_matrix = object_transformation_matrix;
		obj_pack->translation_matrix_scaled = object_translation_matrix_scaled;
		obj_pack->cam_inv_scale_factor = m_camera_inv_scale_factor;
		obj_pack->def_transformation_matrix = object_def_transformation_matrix;
	}
	if (utility.group == g_LINKING)
	{

	}

}
void InterfaceManager::calculate_new_utility_isec_point_relative_on_selected_utility_plane()
{
	m_utility_isec_p.reset();

	optix::float3 * M_WORKING_AXIS = m_selected_object_pack.base_axis;
	optix::float3 plane_n;
	optix::float3 c = m_selected_object_pack.raw_object->getTranslationAttributes("calc_new_ut_isec_point");
	optix::float3 cam_w = Mediator::RequestCameraInstance().getW();
	UtilityPlane selected_plane = m_interface_state.selected_utility_plane;
	Ray mouse_ray = Geometry::SHAPES::createMouseRay();

	switch (m_selected_utility.id)
	{

	case u_TRANSLATION:
	{
		
		float d_u = optix::dot(cam_w, M_WORKING_AXIS[0]);
		float d_y = optix::dot(cam_w, M_WORKING_AXIS[1]);
		float d_z = optix::dot(cam_w, M_WORKING_AXIS[2]);

		if      (selected_plane == UtilityPlane::X)
		{
			if (fabs(d_y) < fabs(d_z))
				plane_n = M_WORKING_AXIS[2];
			else
				plane_n = M_WORKING_AXIS[1];
		}
		else if (selected_plane == UtilityPlane::Y)
		{
			if (fabs(d_u) < fabs(d_z))
				plane_n = M_WORKING_AXIS[2];
			else
				plane_n = M_WORKING_AXIS[0];
		}
		else if (selected_plane == UtilityPlane::Z)
		{
			if (fabs(d_u) < fabs(d_y))
				plane_n = M_WORKING_AXIS[1];
			else
				plane_n = M_WORKING_AXIS[0];
		}
		else if (selected_plane == UtilityPlane::XY)
		{
			plane_n = M_WORKING_AXIS[2];
		}
		else if (selected_plane == UtilityPlane::XZ)
		{
			plane_n = M_WORKING_AXIS[1];
		}
		else if (selected_plane == UtilityPlane::YZ)
		{
			plane_n = M_WORKING_AXIS[0];
		}
		else if (selected_plane == UtilityPlane::XYZ)
		{
			plane_n = -cam_w;
		}

		Plane projection_plane = Geometry::SHAPES::createPlane(c, plane_n);
		optix::float3 p;
		Geometry::RAY::Intersect_Plane(mouse_ray, projection_plane, p);

#ifdef UTILITY_ISEC_P_REPROJECT_TO_CENTER
		m_utility_isec_p.p = p - c;
#else
		m_utility_isec_p.p = p;
#endif
		m_utility_isec_p.intersection_plane = plane_n;
		m_utility_isec_p.plane_id = selected_plane;
		m_utility_isec_p.center = c;
	}
	break;
	
	case u_ROTATION:
	{

		if (selected_plane == UtilityPlane::X)
		{
			plane_n = M_WORKING_AXIS[0];
		}
		else if (selected_plane == UtilityPlane::Y)
		{
			plane_n = M_WORKING_AXIS[1];
		}
		else if (selected_plane == UtilityPlane::Z)
		{
			plane_n = M_WORKING_AXIS[2];
		}
		else if (selected_plane == UtilityPlane::XYZ)
		{
			plane_n = -cam_w;
		}

		float r = 1.0f * 0.6 * m_renderer_camera_inv_scale_factor;
		Plane projection_plane = Geometry::SHAPES::createPlane(c, plane_n);
		optix::float3 p;
		Geometry::RAY::Intersect_Plane(mouse_ray, projection_plane, p);
		optix::float3 normal = optix::normalize(p - c);
		if (optix::dot(cam_w, normal) > 0)
		{
			// mirror normal if same direction with cam_w
			//normal = -normal;
			//optix::float3 tan = -optix::normalize(optix::cross(normal, plane_n));
			//normal = normal - 2 * (optix::dot(tan, normal))*normal;
		}

		optix::float3 arc_point = c + normal * r;
		m_utility_isec_p.p = arc_point;
		m_utility_isec_p.center = c;
		m_utility_isec_p.normal = normal;
		optix::float3 tangent = Geometry::VECTOR_SPACE::getCircleTangentOnPlane(m_utility_isec_p, M_WORKING_AXIS, selected_plane);
		m_utility_isec_p.tangent = tangent;
		m_utility_isec_p.plane_id = selected_plane;
		m_utility_isec_p.out_normal = optix::cross(normal, tangent);

	}
	break;
	
	case u_SCALE:
	{

	}
	break;

	
	}
}
void InterfaceManager::fill_pre_object_manipulation_Data()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "fill_pre_object_manipulation_Data", "");
#endif

	////std::cout << "\n - InterfaceManager::fill_pre_object_manipulation_Data():" << std::endl;

	switch (m_selected_utility.id)
	{

	case u_TRANSLATION:
	{
		m_object_transformations_manip_data.isec_p = m_utility_isec_p;
		m_object_transformations_manip_data.starting_point = m_utility_isec_p.p;
		m_object_transformations_manip_data.plane_n = m_utility_isec_p.intersection_plane;
		m_object_transformations_manip_data.start_translation = m_selected_object_pack.raw_object->getTranslationAttributes("fill_pre_obj_manip_data");
		UtilityPlane plane_id = m_utility_isec_p.plane_id;


		m_object_transformations_manip_data.utility_plane = Geometry::SHAPES::createPlane(
			m_object_transformations_manip_data.start_translation,
			m_object_transformations_manip_data.plane_n);

		m_object_transformations_manip_data.plane_id = plane_id;
		optix::float3 * M_WORKING_AXIS = &m_selected_object_pack.base_axis[0];
		if (plane_id == X || plane_id == Y || plane_id == Z)
			m_object_transformations_manip_data.starting_delta = optix::dot(M_WORKING_AXIS[plane_id - 1], m_object_transformations_manip_data.starting_point);
		else
		{
			m_object_transformations_manip_data.starting_plane_delta = m_object_transformations_manip_data.starting_point;
		}

	}
	break;

	case u_ROTATION:
	{
		UtilityPlane plane_id = m_utility_isec_p.plane_id;
		m_object_transformations_manip_data.plane_id = plane_id;
		Ray utility_ray = Geometry::SHAPES::createMouseRay();
		optix::float3 plane_o = m_utility_isec_p.p;
		float plane_tang_dot = optix::dot(m_utility_isec_p.tangent, utility_ray.direction);
		float plane_dot = optix::dot(GLOBAL_AXIS[plane_id - 1], utility_ray.direction);

		optix::float3 plane_n = -Mediator::RequestCameraInstance().getW();
		Plane utility_plane = Geometry::SHAPES::createPlane(plane_o, plane_n);

		optix::float3 starting_point;
		Geometry::RAY::Intersect_Plane(utility_ray, utility_plane, starting_point);

		m_object_transformations_manip_data.utility_plane = utility_plane;
		m_object_transformations_manip_data.isec_p = m_utility_isec_p;
		m_object_transformations_manip_data.plane_n = plane_n;
		m_object_transformations_manip_data.starting_point = starting_point;
		m_object_transformations_manip_data.start_rotation = m_selected_object_pack.raw_object->getRotationAttributes();
		m_object_transformations_manip_data.start_rot_theta = 0.0f;

		// fill mat_data in case UtilityPlane::XYZ for the ActionManager ( in case of : [undo/redo] )
		if (plane_id == UtilityPlane::XYZ)
		{
			m_object_transformations_manip_data.starting_dir = Geometry::SHAPES::createMouseRay().direction;
			optix::float3 center = m_selected_object_pack.raw_object->getTranslationAttributes();
			float dist_factor = ViewportManager::GetInstance().getActiveViewportCamera().getDistanceFrom(m_selected_object_pack.raw_object->getTranslationAttributes());
			float renderSize_factor = ViewportManager::GetInstance().getActiveViewportCamera().getInvScaleFactor(ViewportManager::GetInstance().getActiveViewport());
			Plane view_plane  = Geometry::SHAPES::createPlane(utility_ray.origin + utility_ray.direction * dist_factor, -utility_ray.direction);
			optix::float3 isec_p;
			Geometry::RAY::Intersect_Plane(utility_ray, view_plane, isec_p);
			optix::float3 P = center - isec_p;
			P = (utility_ray.direction * dist_factor / renderSize_factor);
			m_object_transformations_manip_data.starting_point = P;

			m_object_transformations_manip_data.init_matrices = true;
			m_object_transformations_manip_data.mat_data = new float[16];

			for (int i = 0; i < 16; i++)
			{
				m_object_transformations_manip_data.mat_data[i] = m_selected_object_pack.raw_object->getRotationMatrix().getData()[i];
			}

			// stack method //
			std::stack<SceneObject*> tree_stack;
			tree_stack.push(m_selected_object_pack.raw_object);
			while (!tree_stack.empty())
			{
				SceneObject* parent = tree_stack.top();
				tree_stack.pop();

				
				std::vector<SceneObject*> childs;
				if (m_hierarchy_setting == TO_ALL)
					childs = parent->getChilds_EveryType();
				else if (m_hierarchy_setting == ONLY_TO_CHILDS)
					childs = parent->getChilds();
				else if (m_hierarchy_setting == ONLY_THIS)
					childs = parent->getUtilityChilds();
				int num_childs = childs.size();

				if (num_childs > 0)
				{
					childs = parent->getChilds_EveryType();
					for (SceneObject * child : childs)
					{
						tree_stack.push(child);

						float * child_data = new float[16];
						for (int j = 0; j < 16; j++)
						{
							if (m_hierarchy_setting == ONLY_TO_CHILDS)
								child_data[j] = child->getTranslationMatrix_value().getData()[j];
							else
								child_data[j] = child->get_parent_rot_matrix_as_value().getData()[j];
						}

						m_object_transformations_manip_data.childs_relative_positions.push_back(child->getRelativeTranslationAttributes());
						m_object_transformations_manip_data.child_mat_data.push_back(child_data);
						m_object_transformations_manip_data.child_index.push_back(child);
						m_object_transformations_manip_data.child_attri[child] = child->getRotationAttributes();
						
					}
				}
			}
			//

		}

	}
	break;

	case u_SCALE:
	{
		m_object_transformations_manip_data.isec_p = m_utility_isec_p;
		m_object_transformations_manip_data.starting_point = m_utility_isec_p.p;
		m_object_transformations_manip_data.plane_n = m_utility_isec_p.intersection_plane;
		m_object_transformations_manip_data.start_scale = m_selected_object_pack.raw_object->getScaleAttributes();
		m_object_transformations_manip_data.start_translation = m_selected_object_pack.raw_object->getTranslationAttributes("fill_pre_obj_manip_data2");
		UtilityPlane plane_id = m_utility_isec_p.plane_id;

		//if (plane_id == XYZ)
			//m_object_transformations_manip_data.plane_n = -Mediator::RequestCameraInstance().getW();

		m_object_transformations_manip_data.utility_plane = Geometry::SHAPES::createPlane(
			m_object_transformations_manip_data.start_translation,
			m_object_transformations_manip_data.plane_n);

		m_object_transformations_manip_data.plane_id = plane_id;
		optix::float3 * M_WORKING_AXIS = &m_selected_object_pack.base_axis[0];
		if (plane_id == X || plane_id == Y || plane_id == Z)
			m_object_transformations_manip_data.starting_delta = optix::dot(M_WORKING_AXIS[plane_id - 1], m_object_transformations_manip_data.starting_point);
		else
		{
			m_object_transformations_manip_data.starting_plane_delta = m_object_transformations_manip_data.starting_point;
			if (plane_id == XY)
			{
				m_object_transformations_manip_data.plane_id1 = UtilityPlane::X;
				m_object_transformations_manip_data.plane_id2 = UtilityPlane::Y;
				m_object_transformations_manip_data.starting_delta1 = optix::dot(M_WORKING_AXIS[0], m_object_transformations_manip_data.starting_point);
				m_object_transformations_manip_data.starting_delta2 = optix::dot(M_WORKING_AXIS[1], m_object_transformations_manip_data.starting_point);
			}
			else if (plane_id == XZ)
			{
				m_object_transformations_manip_data.plane_id1 = UtilityPlane::X;
				m_object_transformations_manip_data.plane_id2 = UtilityPlane::Z;
				m_object_transformations_manip_data.starting_delta1 = optix::dot(M_WORKING_AXIS[0], m_object_transformations_manip_data.starting_point);
				m_object_transformations_manip_data.starting_delta2 = optix::dot(M_WORKING_AXIS[2], m_object_transformations_manip_data.starting_point);
			}
			else if (plane_id = YZ)
			{
				m_object_transformations_manip_data.plane_id1 = UtilityPlane::Y;
				m_object_transformations_manip_data.plane_id2 = UtilityPlane::Z;
				m_object_transformations_manip_data.starting_delta1 = optix::dot(M_WORKING_AXIS[1], m_object_transformations_manip_data.starting_point);
				m_object_transformations_manip_data.starting_delta2 = optix::dot(M_WORKING_AXIS[2], m_object_transformations_manip_data.starting_point);
			}
			else if (plane_id == XYZ)
			{
				m_object_transformations_manip_data.plane_id = UtilityPlane::XYZ;
				m_object_transformations_manip_data.starting_delta1 = optix::dot(M_WORKING_AXIS[0], m_object_transformations_manip_data.starting_point);
				m_object_transformations_manip_data.starting_delta2 = optix::dot(M_WORKING_AXIS[1], m_object_transformations_manip_data.starting_point);
				m_object_transformations_manip_data.starting_delta3 = optix::dot(M_WORKING_AXIS[2], m_object_transformations_manip_data.starting_point);
				m_object_transformations_manip_data.starting_delta = optix::length(m_object_transformations_manip_data.starting_point - m_object_transformations_manip_data.start_translation);
				
			}

		}
	}
	break;

	}

	//if (m_selected_axis_orientation == AXIS_ORIENTATION::SCREEN)
	//{
		//m_object_transformations_manip_data.base_axis = BASE_TRANSFORM::SCREEN_AXIS(m_selected_object_pack.raw_object, Mediator::RequestCameraInstance());
	//}

}
bool InterfaceManager::is_finally_object_cloning_aproved(Object_Transformations_Manipulation_Data& data)
{
	bool approved = false;
	float transformation_delta = data.delta_length;
	float e = 1e-4;
	approved = (transformation_delta <= e) ? 0 : 1;
	return approved;
}


void InterfaceManager::push_Temp_Group_Disbad_Action( int selection_id, int to_index )
{
	SceneObject* parent = Mediator::RequestSceneObject( m_area_selections_data[selection_id].parent_id );
	Action* ac = new Action_DISBAND_TEMP_GROUP( parent,  selection_id, to_index );
	ac->set_Should_Peek( m_should_peek_ac_flag );
	ActionManager::GetInstance().push( ac );
}
Action * InterfaceManager::get_Temp_Group_Disband_Action(int selection_id, int to_index)
{
	////std::cout << "\n  - InterfaceManager::push_Temp_Group_Disband_Action( " << selection_id << " , " << to_index << " ) " << std::endl;
	//int index_from_id = getArea_Selection_DataIndex_FromParent(selection_id);
	SceneObject* parent = Mediator::RequestSceneObject(m_area_selections_data[selection_id].parent_id);
	Action* ac = new Action_DISBAND_TEMP_GROUP(parent, selection_id, to_index);
	return ac;
}

void InterfaceManager::recover_selections_state(int index, bool reset_state)
{
	////std::cout << "\n  - InterfaceManager::recover_selections_state( " << index << " , " << reset_state<<" ) " << std::endl;
	Multiple_Selection_Data data = m_area_selections_data[getArea_Selection_DataIndex_FromParent(index)];
	for (SceneObject* obj : data.selection_objects)
	{
		
		if (obj->getParent() != nullptr)
			obj->removeParent();
		
		SceneObject* parent = data.parents_bucket[obj->getId()];
		if (parent != nullptr)
		{
			parent->addChild(obj);
			obj->setParent(parent, data.parents_data[obj->getId()]);
		}

		SceneObject* group_parent = data.group_parents_bucket[obj->getId()];
		int grp_id = group_parent == nullptr ? -1 : group_parent->getId();
		obj->setGroupParent(group_parent);
		
	}

	if ( reset_state )
		m_multiple_selection_active = false;



	//Utilities::Update_Selections_Group_Topology( data ); // EDW PAIZEI TO THEMA //

}
void InterfaceManager::recover_selections_state_flat(int index)
{
	////std::cout << "\n  - InterfaceManager::recover_selections_state_flat( " << index <<  " ) " << std::endl;
	Multiple_Selection_Data data = m_area_selections_data[getArea_Selection_DataIndex_FromParent(index)];
	for (SceneObject* obj : data.selection_objects)
	{
		
		if (obj->getParent() != nullptr)
			obj->removeParent();
		
		SceneObject* parent = data.parents_bucket[obj->getId()];
		if (parent != nullptr)
		{
			parent->addChild(obj);
			obj->setParent(parent);
		}

		SceneObject* group_parent = data.group_parents_bucket[obj->getId()];
		int grp_id = group_parent == nullptr ? -1 : group_parent->getId();
		obj->setGroupParent(group_parent);
		
	}

	m_multiple_selection_active = false;
	//Utilities::Update_Selections_Group_Topology( data );

}
void InterfaceManager::remove_Multiple_Selection_Data(int index)
{
	int id = -1;
	for (int i = 0; i < m_area_selections_data.size(); i++)
	{
		if (m_area_selections_data[i].parent_id == index)
		{
			id = i;
			break;
		}
	}

	m_area_selections_data.erase(m_area_selections_data.begin() + id);
}

void InterfaceManager::create_SceneGroup(std::vector<SceneObject*>& top_level_entities, std::vector<SceneObject*>& top_level_groups, std::vector<SceneObject*>& area_selection, std::string name)
{

	SceneObject* group_parent_dummy = Utilities::create_SceneGroupFrom(area_selection);
	group_parent_dummy->setName(name);
	group_parent_dummy->setNamePostfix("");
	
	int array_index = Mediator::requestObjectsArrayIndex(group_parent_dummy);
	push_Temp_Group_Disbad_Action(m_multiple_selection_active_id, -1);

	
	Action * action = new Action_CREATE_GROUP(
		group_parent_dummy, 
		m_selected_object_pack.object_index, 
		top_level_entities, 
		top_level_groups,
		m_area_selections_data[m_multiple_selection_active_id].selection_objects);

	ActionManager::GetInstance().push(action);
	//setSelectedObject(group_parent_dummy);
}
void InterfaceManager::create_SceneGroup(SceneObject* object, std::string name)
{
	// this fixes the undo / redo issues when grouping a single object consecutive times
	// resetSelectedObject(true,true);


	// pre process object //
	Multiple_Selection_Data data;
	std::vector<SceneObject*> virtual_selection;
	if (object->getType() == GROUP_PARENT_DUMMY)
	{
		for (SceneObject* child : object->getGroupSelectionChilds())
			data.selection_objects.push_back(child);
	}
	else
	{
		data.selection_objects.push_back(object);
	}
	process_AreaSelection(data);

	SceneObject* group_parent_dummy = Utilities::create_SceneGroupFrom( data.selection_objects );
	group_parent_dummy->setName( name );
	group_parent_dummy->setNamePostfix("");

	Action * action = new Action_CREATE_GROUP(
		group_parent_dummy,
		m_selected_object_pack.raw_object->getId(),
		//m_selected_object_pack.object_index,
		data.top_level_entities,
		data.top_level_groups,
		data.selection_objects);

	ActionManager::GetInstance().push(action);
}


void InterfaceManager::process_FocusedObjectPack(Object_Package& obj_pack)
{
	if (obj_pack.raw_object == nullptr)
		return;

	SELECTION_SETTING setting = m_selection_setting;
	
	int alt_l_key_pressed = glfwGetKey(Mediator::RequestWindow(), GLFW_KEY_LEFT_ALT);
	int alt_r_key_pressed = glfwGetKey(Mediator::RequestWindow(), GLFW_KEY_RIGHT_ALT);
	bool alt_pressed = false;
	if (   alt_l_key_pressed == GLFW_PRESS
		|| alt_r_key_pressed == GLFW_PRESS)
		alt_pressed = true;

	if (alt_pressed)
		return;

	if (m_ui_focused || m_gui_focused)
	{
		return;
		//setting = S_OBJECT;
	}
	
	switch ( setting )
	{

	case S_OBJECT:
	{
		// case : Area selection is active
		if (    m_multiple_selection_active
			&& !m_cloning_group
			&& !m_ui_focused
			&& !alt_pressed )
		{
			bool is_in_area_selection = false;
			////std::cout << "\n  - m_multiple_selection_active : " << m_multiple_selection_active << std::endl;
			////std::cout << "  - m_focused_object : " << m_focused_object_pack.raw_object->getId() << std::endl;
			////std::cout << "\n  - m_focused_obj : " << m_focused_object_pack.raw_object->getId() << std::endl;
			bool run = true;
			for (SceneObject * obj : m_area_selections_data[m_multiple_selection_active_id].selection_objects)
			{
				////std::cout << "    - selection_obj : " << obj->getId() << std::endl;
				if (obj_pack.raw_object->getId() == obj->getId())
				{
					is_in_area_selection = true;
					break;
				}

			}

			if (!is_in_area_selection)
			{
				for (SceneObject* obj : m_selected_object_pack.raw_object->getGroupSelectionChilds())
				{
					if (obj_pack.raw_object->getId() == obj->getId())
					{
						is_in_area_selection = true;
						break;
					}
				}
			}


			if (is_in_area_selection)
			{
				obj_pack.raw_object = m_selected_object_pack.raw_object;
				obj_pack.object_index = m_selected_object_pack.object_index;
				return;
			}
		}
		else if ( !m_ui_focused )
		{
			SceneObject* selected = m_selected_object_pack.raw_object;
			if ( selected != nullptr )
			{
				if ( selected->getType() == GROUP_PARENT_DUMMY )
				{
					std::vector<SceneObject*> group_selection = selected->getGroupSelectionChilds_and_Groups();
					for (SceneObject* selection_obj : group_selection)
					{
						if (selection_obj->getId() == obj_pack.raw_object->getId())
						{
							obj_pack.raw_object   = m_selected_object_pack.raw_object;
							obj_pack.object_index = m_selected_object_pack.object_index;
							return;
						}
					}
				}
			}
		}
	}
	break;

	case S_GROUP:
	{
		
		// case : Area selection is active
		if (m_multiple_selection_active)
		{
			bool is_in_area_selection = false;
			////std::cout << "\n  - m_focused_obj : " << m_focused_object_pack.raw_object->getId() << std::endl;
			bool run = true;
			for (SceneObject * obj : m_area_selections_data[m_multiple_selection_active_id].selection_objects)
			{
				////std::cout << "    - selection_obj : " << obj->getId() << std::endl;
				if (obj_pack.raw_object->getId() == obj->getId())
				{
					is_in_area_selection = true;
					break;
				}

				/*
				std::vector<SceneObject*> selection_objs = obj->getGroupSelectionChilds();
				for (SceneObject* selection_obj : selection_objs)
				{
					if (m_focused_object_pack.raw_object->getId() == selection_obj->getId())
					{
						is_in_area_selection = true;
						run = false;
						break;
					}
				}

				if (!run) break;
				*/
			}

			if (!is_in_area_selection)
			{
				for (SceneObject* obj : m_selected_object_pack.raw_object->getGroupSelectionChilds())
				{
					if (obj_pack.raw_object->getId() == obj->getId())
					{
						is_in_area_selection = true;
						break;
					}
				}
			}


			if (is_in_area_selection)
			{
				obj_pack.raw_object   = m_selected_object_pack.raw_object;
				obj_pack.object_index = m_selected_object_pack.object_index;
				return;
			}
		}

		// case : Check if focused object is child of already selected Group.
		else
		{
			SceneObject* selected = m_selected_object_pack.raw_object;
			if (selected != nullptr)
			{
				std::vector<SceneObject*> selection_childs = selected->getGroupSelectionChilds_and_Groups();
				for (SceneObject* obj : selection_childs)
				{
					if (obj_pack.raw_object->getId() == obj->getId())
					{
						obj_pack.raw_object = m_selected_object_pack.raw_object;
						obj_pack.object_index = m_selected_object_pack.object_index;
						return;
					}
				}
			}
		}

		// case : ( default )
		if ( obj_pack.raw_object->getType() != GROUP_PARENT_DUMMY )
		{
			SceneObject* group_parent = obj_pack.raw_object->getGroupParent();
			if (group_parent != nullptr)
			{
				////std::cout << " - Object[ "<<obj_pack.raw_object->getId()<<" ]->Group_parent : " << group_parent->getId() << std::endl;

				obj_pack.raw_object = group_parent;
				obj_pack.object_index = Mediator::requestObjectsArrayIndex(group_parent->getId());
			}
		}

		
	}
	break;

	}


}

void InterfaceManager::process_pre_AreaSelection(std::vector<SceneObject*>& selection)
{
	std::vector<SceneObject*> temp; 
	for (SceneObject* obj : selection) 
		temp.push_back(obj);
	
	selection.clear();

	for (SceneObject* obj : temp)
	{
		if (obj->getType() != GROUP_PARENT_DUMMY)
			selection.push_back(obj);
	}

	for (SceneObject* obj : temp)
	{
		if (obj->getType() == GROUP_PARENT_DUMMY)
		{
			std::vector<SceneObject*> selection_childs = obj->getGroupSelectionChilds();
			for (SceneObject* child : selection_childs)
			{
				if (!Utilities::is_object_in_List(child, selection))
					selection.push_back(child);
			}
		}
	}

}
void InterfaceManager::process_AreaSelection(Multiple_Selection_Data& data, bool as_temp_group)
{

//#define SELECT_GROUP_OBJECTS_FROM_GROUPS
#ifdef SELECT_GROUP_OBJECTS_FROM_GROUPS

	std::vector<SceneObject*> to_add;
	std::vector<SceneObject*> to_remove;

	std::vector<SceneObject*> temp; for (SceneObject* obj : data.selection_objects) temp.push_back(obj);

	for (SceneObject* obj : temp)
	{
		if (obj->getType() == GROUP_PARENT_DUMMY)
		{
			for (SceneObject* child : obj->getGroupSelectionChilds())
			{
				if (!Utilities::is_object_in_List(child, data.selection_objects))
				{
					to_add.push_back(child);
				}
			}

			to_remove.push_back(obj);
		}
		else
			to_add.push_back(obj);
	}

	data.selection_objects.clear();
	for (SceneObject* obj : to_add)
		data.selection_objects.push_back(obj);

#endif

	get_Grouping_Information(data);

	
	bool fill_top_level = false;
	if (data.top_level_entities.size() == 0)
	{
		
		for ( SceneObject* obj : data.selection_objects )
			data.top_level_entities.push_back(obj);

		fill_top_level = true;
	}


	for (SceneObject* obj : data.selection_objects)
	{
		bool is_covered = false;
		for (SceneObject* group : data.top_level_groups)
		{
			if ( Utilities::is_object_in_List(obj, group->getGroupSelectionChilds_and_Groups()) )
			{
				is_covered = true;
				break;
			}
		}

		if ( !is_covered )
		{
			for ( SceneObject* sub_group : data.top_level_subgroups )
			{
				if ( Utilities::is_object_in_List(obj, sub_group->getGroupSelectionChilds_and_Groups()) )
				{
					is_covered = true;
					break;
				}
			}
		}

		if ( !is_covered )
			data.solo_objects.push_back(obj);
	}



	

// #define PRINT_AREA_SELECTION_INFO
#ifdef PRINT_AREA_SELECTION_INFO
	//std::cout << "\n\n - processing_AreaSelection():" << std::endl;
	//std::cout << "\n   > SELECTION:" << std::endl;
	for (SceneObject* obj : data.selection_objects)
	{
		//std::cout << "     - selection_obj : " << obj->getId() << std::endl;
	}

	//std::cout << "\n   > TOP_LEVEL_OBJECTS:" << std::endl;
	for (SceneObject* obj : data.top_level_entities)
	{
		//std::cout << "     - top_level_obj : " << obj->getId() << std::endl;
	}


	//std::cout << "\n   > TOP_LEVEL_GROUPS:" << std::endl;
	for (SceneObject* obj : data.top_level_groups)
	{
		//std::cout << "     - top_level_group : " << obj->getId() << std::endl;
	}

	//std::cout << "\n   > TOP_LEVEL_SUBGROUPS:" << std::endl;
	for (SceneObject* obj : data.top_level_subgroups)
	{
		//std::cout << "     - top_level_subgroup : " << obj->getId() << std::endl;
	}

	//std::cout << "\n   > SOLO_OBJECTS:" << std::endl;
	for (SceneObject* obj : data.solo_objects)
	{
		//std::cout << "     - solo_object : " << obj->getId() << std::endl;
	}


#endif
	
	
}
void InterfaceManager::get_Grouping_Information(Multiple_Selection_Data& group_data)
{

	group_data.top_level_entities = Utilities::get_top_level_objects(nullptr, group_data.selection_objects, group_data.top_level_groups, group_data.top_level_subgroups);
	if (group_data.top_level_entities.size() > 0)
		group_data.can_group = true;
	else
		group_data.can_group = false;

}
bool InterfaceManager::get_Grouping_Information(SceneObject* obj)
{
	bool can_group = obj == nullptr ? false : true;
	
	if (can_group)
		can_group = obj->isTemporary() ? false : true;

	if( can_group )
		can_group = obj->getGroupParent() == nullptr ? true : false;

	return can_group;
}

void InterfaceManager::print_SceneObjects_Info()
{


#define PRINT_SCENE_INFO
#ifdef PRINT_SCENE_INFO
	int i = 0;
	//std::cout << "\n\n DEBUGGING::print_SceneObjects_Info(): " << std::endl;
	for (SceneObject* obj : Mediator::RequestSceneObjects() )
	{
		if (obj->getId() == -100)
			continue;

		std::cout << " \n" << std::endl;
		std::cout << " [o] OBJ : [ " << obj->getId() << " ] " << std::endl;
		std::cout << "   - name : " << obj->getName() << std::endl;
		//std::cout << "   - is_Group      : " << (obj->getType() == GROUP_PARENT_DUMMY) << std::endl;
		//std::cout << "   - is_temporary  : " << obj->isTemporary() << std::endl;
		//std::cout << "   - active        : " << obj->isActive() << std::endl;
		//std::cout << "   - source_id     : " << obj->getSourceId() << std::endl;
		
		//std::cout << "   - parent        : " << (obj->getParent() == nullptr ? -1 : obj->getParent()->getId()) << std::endl;
		//std::cout << "   - grp_parent    : " << (obj->getGroupParent() == nullptr ? -1 : obj->getGroupParent()->getId()) << std::endl;

		//std::cout << "   - childs        : {  ";
		//for (SceneObject* child : obj->getChilds()) std::cout << child->getId() << " ";
		//std::cout << " } " << std::endl;

		//std::cout << "   - grp_childs    : {  ";
		//for (SceneObject* child : obj->getGroupSelectionChilds_and_Groups()) std::cout << child->getId() << " ";
		//std::cout << " } " << std::endl;


		optix::Matrix4x4 R = obj->getRotationMatrix_chain();
		optix::Matrix4x4 S = obj->getScaleMatrix_value();
		optix::Matrix4x4 RS = R * S;

		optix::float3 RSu = optix::make_float3(RS.getRow(0));
		optix::float3 RSv = optix::make_float3(RS.getRow(1));
		optix::float3 RSw = optix::make_float3(RS.getRow(2));

		optix::float3 Su = optix::make_float3(S.getRow(0));
		optix::float3 Sv = optix::make_float3(S.getRow(1));
		optix::float3 Sw = optix::make_float3(S.getRow(2));

		//Utilities::printVec(&RSu, "RSu");
		//Utilities::printVec(&RSv, "RSv");
		//Utilities::printVec(&RSw, "RSw");

		//std::cout << "\n";
		//Utilities::printVec(&Su, "Su");
		//Utilities::printVec(&Sv, "Sv");
		//Utilities::printVec(&Sw, "Sw");

		i++;
	}
#endif

}
void InterfaceManager::print_Debug_Info(bool ut_childs)
{

	//std::cout << "\n\n - - - - - - - - - - - - - -  ________DEBUG_INFO________  - - - - - - - - - - - - - - - - - \n";
	int i = 0;
	//std::cout << " ---- PRINTING OBJECTS INFO ----  " << std::endl;
	for (SceneObject* obj : Mediator::RequestSceneObjects())
	{
		Type obj_type = obj->getType();
		bool is_ut_child = false;
		
		if (obj_type == FACE_VECTOR || obj_type == SAMPLER_OBJECT)
			is_ut_child = true;
		
		if (is_ut_child && !ut_childs)
			continue;
		
		//std::cout << "\n   -------- OBJECT : [[[[[ " << i << " ]]]]]" << std::endl;
		print_Debug_Info_(obj);
		i++;
	}
	return;

	SceneObject* selected_obj = m_selected_object_pack.raw_object;
	SceneObject* focused_obj = m_focused_object_pack.raw_object;
	int selected_id = selected_obj == nullptr ? -1 : selected_obj->getId();
	int focused_id = focused_obj == nullptr ? -1 : focused_obj->getId();
	std::string ut = Utilities::convert_Utility_To_Utility_id(m_selected_utility.id);
	
	//std::cout << "\n\n - - - - - - - - - - - - - -  ________DEBUG_INFO________  - - - - - - - - - - - - - - - - - \n";
	//std::cout << "   - selected_object : " << (selected_obj != nullptr) << std::endl;
	//std::cout << "   - selected_id : " << selected_id << std::endl;
	//std::cout << "   - focused_object : " << (focused_obj != nullptr) << std::endl;
	//std::cout << "   - focused_id << " << focused_id << std::endl;
	//std::cout << "   - utility : " << ut << std::endl;
	//std::cout << "   - selection_setting : " << m_selection_setting << std::endl;
	//std::cout << "   - area selection active : " << m_multiple_selection_active << std::endl;


	if ( m_selected_object_pack.raw_object == nullptr )
		return;

	SceneObject* obj  = m_selected_object_pack.raw_object;
	Type obj_type     = obj->getType();
	bool is_active    = obj->isActive();
	bool is_temporary = obj->isTemporary();
	float avg_spread  = obj->getGeometryAverageSpread();
	optix::float3 bbox_min, bbox_max, pos, rot, scale, rel_pos;
	bbox_min = obj->getBBoxMin();
	bbox_max = obj->getBBoxMax();
	pos      = obj->getTranslationAttributes();
	rel_pos  = obj->getRelativeTranslationAttributes();
	rot      = obj->getRotationAttributes();
	scale    = obj->getScaleAttributes();

	SceneObject* parent     = obj->getParent();
	SceneObject* grp_parent = obj->getGroupParent();
	int parent_id           = parent == nullptr     ? -1 : parent->getId();
	int grp_parent_id       = grp_parent == nullptr ? -1 : grp_parent->getId();
	

	std::vector<SceneObject*> childs                          = obj->getChilds();
	std::vector<SceneObject*> group_selection_childs          = obj->getGroupSelectionChilds();
	std::vector<SceneObject*> group_selection_childs_n_groups = obj->getGroupSelectionChilds_and_Groups();


	

	//std::cout << "\n";
	//std::cout << "   - object          : " << obj->getId() << std::endl;
	//std::cout << "   - parent_id       : " << parent_id << std::endl;
	//std::cout << "   - group_parent_id : " << grp_parent_id << std::endl;

	//std::cout << "\n";
	//std::cout << "   - relative_pos : [ " << rel_pos.x << " , " << rel_pos.y << " , " << rel_pos.z << " ] " << std::endl;
	//std::cout << "   - translation  : [ " << pos.x << " , " << pos.y << " , " << pos.z << " ] " << std::endl;
	//std::cout << "   - rotation     : [ " << rot.x << " , " << rot.y << " , " << rot.z << " ] " << std::endl;
	//std::cout << "   - scale        : [ " << scale.x << " , " << scale.y << " , " << scale.z << " ] " << std::endl;


	
	//std::cout << "\n -     -       -      -       -    _MATRICES_    -       -     -       -     \n" << std::endl;
	//std::cout << " - - - - - TRANLSATION_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->getTranslationMatrix().getData(), 16, "Translation_Matrix");
	//std::cout << "\n - - - - - ROTATION_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->getRotationMatrix().getData(), 16, "Rotation_Matrix");
	//std::cout << "\n - - - - - SCALE_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->getScaleMatrix().getData(), 16, "Scale_Matrix");
	//std::cout << "\n - - - - - PARENT_ROT_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->get_local_parent_rot_matrix_value().getData(), 16, "Parent_Rot_Matrix");
	//std::cout << "\n - - - - - PARENT_SCALE_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->get_local_parent_scale_matrix_value().getData(), 16, "Parent_Scale_Matrix");
	//std::cout << " -     -       -      -       -     -       -     -       -     " << std::endl;
	
	
	//std::cout << "\n";
	//std::cout << "   - BBOX_min : [ " << bbox_min.x << " , " << bbox_min.y << " , " << bbox_min.z << " ] " << std::endl;
	//std::cout << "   - BBOX_max : [ " << bbox_max.x << " , " << bbox_max.y << " , " << bbox_max.z << " ] " << std::endl;

	//std::cout << "\n";
	//std::cout << "   - childs.size()                          : " << childs.size() << std::endl;
	//std::cout << "   - group_selection_childs.size()          : " << group_selection_childs.size() << std::endl;
	//std::cout << "   - group_selection_childs_n_groups.size() : " << group_selection_childs_n_groups.size() << std::endl;

	/*
	//std::cout << "\n";
	//std::cout << "    > CHILDS:" << std::endl;
	for (SceneObject* o : childs)
	{
		int p_id = o->getParent()      == nullptr ? -1 : o->getParent()->getId();
		int g_id = o->getGroupParent() == nullptr ? -1 : o->getGroupParent()->getId();
		//std::cout << "       - child : " << o->getId() << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].parent     = " << p_id << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].grp_parent = " << g_id << std::endl;
		//std::cout << "\n";
	}


	//std::cout << "\n";
	//std::cout << "    > GROUP_SELECTION_CHILDS:" << std::endl;
	for (SceneObject* o : group_selection_childs)
	{
		int p_id = o->getParent() == nullptr ? -1 : o->getParent()->getId();
		int g_id = o->getGroupParent() == nullptr ? -1 : o->getGroupParent()->getId();
		//std::cout << "       - child : " << o->getId() << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].parent     = " << p_id << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].grp_parent = " << g_id << std::endl;
		//std::cout << "\n";
	}


	//std::cout << "\n";
	//std::cout << "    > GROUP_SELECTION_CHILDS_N_GROUPS:" << std::endl;
	for (SceneObject* o : group_selection_childs_n_groups)
	{
		int p_id = o->getParent() == nullptr ? -1 : o->getParent()->getId();
		int g_id = o->getGroupParent() == nullptr ? -1 : o->getGroupParent()->getId();
		//std::cout << "       - child : " << o->getId() << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].parent     = " << p_id << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].grp_parent = " << g_id << std::endl;
		//std::cout << "\n";
	}

	*/

	if (obj_type == GROUP_PARENT_DUMMY && false)
	{
		for (SceneObject* o : obj->getGroupSelectionChilds())
			print_Debug_Info(o);
	}
	

}
void InterfaceManager::print_Debug_Info_(SceneObject* obj)
{
	if (obj == 0)
	{
		//std::cout << "  ~ !! ERROR !! Oops! -> obj == nullptr!" << std::endl;
		return;
	}

	Type obj_type = obj->getType();
	bool is_active = obj->isActive();
	bool is_temporary = obj->isTemporary();
	float avg_spread = obj->getGeometryAverageSpread();
	optix::float3 bbox_min, bbox_max, pos, rot, scale, rel_pos;
	bbox_min = obj->getBBoxMin();
	bbox_max = obj->getBBoxMax();
	pos = obj->getTranslationAttributes();
	rel_pos = obj->getRelativeTranslationAttributes();
	rot = obj->getRotationAttributes();
	scale = obj->getScaleAttributes();

	SceneObject* parent = obj->getParent();
	SceneObject* grp_parent = obj->getGroupParent();
	int parent_id = parent == nullptr ? -1 : parent->getId();
	int grp_parent_id = grp_parent == nullptr ? -1 : grp_parent->getId();


	std::vector<SceneObject*> childs = obj->getChilds();
	std::vector<SceneObject*> group_selection_childs = obj->getGroupSelectionChilds();
	std::vector<SceneObject*> group_selection_childs_n_groups = obj->getGroupSelectionChilds_and_Groups();


	//std::cout << "\n\n - - - - - - - - - - - - - -  ________CHILD_INFO[ "<< obj->getId()<<" ]________  - - - - - - - - - - - - - - - - - \n";
	//std::cout << "\n";
	//std::cout << "   - object       : " << obj->getName() << std::endl;
	//std::cout << "   - id           : " << obj->getId() << std::endl;
	//std::cout << "   - Source_id    :  " << obj->getSourceId() << std::endl;
	//std::cout << "   - type         : " << obj_type << std::endl;
	//std::cout << "   - light_type   : " << obj->get_LightParameters().type << std::endl;
	//std::cout << "   - sampler_type : " << obj->get_SamplerParameters().type << std::endl;

	//std::cout << "   - is Clone          : " << obj->is_Clone() << std::endl;
	//std::cout << "   - is Active         : " << obj->isActive() << std::endl;
	//std::cout << "   - is Temporary      : " << obj->isTemporary() << std::endl;
	//std::cout << "   - is Group          : " << (obj_type == GROUP_PARENT_DUMMY) << std::endl;
	//std::cout << "   - is Utility Object : " << (obj_type == SAMPLER_OBJECT || obj_type == FACE_VECTOR) << std::endl;



	//std::cout << "   - parent          : " << ((parent != nullptr) ? parent->getName() : "null") << std::endl;
	//std::cout << "   - parent_id       : " << parent_id << std::endl;
	//std::cout << "   - group_parent    : " << ((grp_parent != nullptr) ? grp_parent->getName() : "null") << std::endl;
	//std::cout << "   - group_parent_id : " << grp_parent_id << std::endl;

	//std::cout << "\n";
	////std::cout << "   - relative_pos : [ " << rel_pos.x << " , " << rel_pos.y << " , " << rel_pos.z << " ] " << std::endl;
	////std::cout << "   - translation  : [ " << pos.x << " , " << pos.y << " , " << pos.z << " ] " << std::endl;
	////std::cout << "   - rotation     : [ " << rot.x << " , " << rot.y << " , " << rot.z << " ] " << std::endl;
	////std::cout << "   - scale        : [ " << scale.x << " , " << scale.y << " , " << scale.z << " ] " << std::endl;

	/*
	//std::cout << "\n -     -       -      -       -    _MATRICES_    -       -     -       -     \n" << std::endl;
	//std::cout << " - - - - - TRANLSATION_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->getTranslationMatrix().getData(), 16, "Translation_Matrix");
	//std::cout << "\n - - - - - ROTATION_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->getRotationMatrix().getData(), 16, "Rotation_Matrix");
	//std::cout << "\n - - - - - SCALE_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->getScaleMatrix().getData(), 16, "Scale_Matrix");
	//std::cout << "\n - - - - - PARENT_ROT_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->get_local_parent_rot_matrix_value().getData(), 16, "Parent_Rot_Matrix");
	//std::cout << "\n - - - - - PARENT_SCALE_MATRIX - - - - - - " << std::endl;
	Utilities::printMatrix(obj->get_local_parent_scale_matrix_value().getData(), 16, "Parent_Scale_Matrix");
	//std::cout << " -     -       -      -       -     -       -     -       -     " << std::endl;
	
	//std::cout << "\n";
	//std::cout << "   - BBOX_min : [ " << bbox_min.x << " , " << bbox_min.y << " , " << bbox_min.z << " ] " << std::endl;
	//std::cout << "   - BBOX_max : [ " << bbox_max.x << " , " << bbox_max.y << " , " << bbox_max.z << " ] " << std::endl;
	*/

	//std::cout << "\n";
	//std::cout << "   - childs.size()                          : " << childs.size() << std::endl;
	//std::cout << "   - group_selection_childs.size()          : " << group_selection_childs.size() << std::endl;
	//std::cout << "   - group_selection_childs_n_groups.size() : " << group_selection_childs_n_groups.size() << std::endl;
	//std::cout << "   - Face_Vectors() : " << obj->get_Face_Vectors().size() << std::endl;
	//std::cout << "   - Samplers()     : " << obj->get_Samplers().size() << std::endl;

	/*
	//std::cout << "\n";
	//std::cout << "    > CHILDS:" << std::endl;
	for (SceneObject* o : childs)
	{
		int p_id = o->getParent() == nullptr ? -1 : o->getParent()->getId();
		int g_id = o->getGroupParent() == nullptr ? -1 : o->getGroupParent()->getId();
		//std::cout << "       - child : " << o->getId() << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].parent     = " << p_id << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].grp_parent = " << g_id << std::endl;
		//std::cout << "\n";
	}


	//std::cout << "\n";
	//std::cout << "    > GROUP_SELECTION_CHILDS:" << std::endl;
	for (SceneObject* o : group_selection_childs)
	{
		int p_id = o->getParent() == nullptr ? -1 : o->getParent()->getId();
		int g_id = o->getGroupParent() == nullptr ? -1 : o->getGroupParent()->getId();
		//std::cout << "       - child : " << o->getId() << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].parent     = " << p_id << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].grp_parent = " << g_id << std::endl;
		//std::cout << "\n";
	}


	//std::cout << "\n";
	//std::cout << "    > GROUP_SELECTION_CHILDS_N_GROUPS:" << std::endl;
	for (SceneObject* o : group_selection_childs_n_groups)
	{
		int p_id = o->getParent() == nullptr ? -1 : o->getParent()->getId();
		int g_id = o->getGroupParent() == nullptr ? -1 : o->getGroupParent()->getId();
		//std::cout << "       - child : " << o->getId() << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].parent     = " << p_id << std::endl;
		//std::cout << "       - child[ " << o->getId() << " ].grp_parent = " << g_id << std::endl;
		//std::cout << "\n";
	}
	*/




}
void InterfaceManager::print_Object_TRS_info()
{

	//std::cout << "\n\n - - - -   Object_Transformation_INFO   - - - - " << std::endl;

	SceneObject* object = m_selected_object_pack.raw_object;
	if (object != nullptr)
	{

		SceneObject* parent = object->getParent();
		std::vector<SceneObject*> childs = object->getChilds();
		bool has_parent = parent != nullptr;
		bool has_childs = childs.size() > 0;
		std::string object_name = object->getName();
		std::string parent_name = has_parent ? parent->getName() : "null";

		// testing data //
		

	}
	//std::cout << "\n - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n" << std::endl;

}

void InterfaceManager::check_viewports_SELECT_FOCUS_state(bool force_focus, bool force_select)
{
	////std::cout << "\n - check_Viewports_SELECT_FOCUS_state():" << std::endl;
	bool focused_changed = false;
	bool selected_changed = false;
	if ( (m_prev_focused_object_pack.object_index  != m_focused_object_pack.object_index ) || force_focus ) focused_changed  = true;
	//Render_Settings r_settings = ViewportManager::GetInstance().get_Viewports_Render_Settings_Accumulative();

	if ((focused_changed || selected_changed || force_focus || force_select))
	{
		std::vector<VIEWPORT> viewports = ViewportManager::GetInstance().getAllEnabledViewports();
		for (VIEWPORT vp : viewports)
		{
			if ( vp.render_settings.draw_bbox_only )
				ViewportManager::GetInstance().getViewport(vp.id).markDirty();
		}
	}

}


#endif


// -------------------------------------------------------
//
//
//
// -------------------------------------------------------

#ifdef INTERFACE_SET_FUNCTIONS

void InterfaceManager::set_UI_Selected_Restriction( int id )
{

}
void InterfaceManager::set_UI_Selected_Restriction( Restriction* R )
{
	if (R != nullptr)
	{
		R->set_Selected(true);
	}

	m_UI_selected_Restriction = R;
		
}

void InterfaceManager::resetSelectedObject(bool save_action, bool dummy_action)
{
	Mediator::RequestCameraInstance().resetOrbitCenter();

	check_viewports_SELECT_FOCUS_state(0, 1);
	process_SELECTION_STATE_change(nullptr);
	
	if ( m_multiple_selection_active )
	{
		if ( save_action )
			m_should_peek_ac_flag = false;
		push_Temp_Group_Disbad_Action( m_multiple_selection_active_id , -1);
		m_should_peek_ac_flag = true;

		if( !dummy_action )
			save_action = false;
	}


	int from_index = m_selected_object_pack.object_index;
	int to_index = -1;

	m_selected_object_pack.reset();
	mouse_data.m1_dragging_down = 0;

	if (save_action && from_index != to_index) // EDW ALLAGH
	{
		
		if (dummy_action)
		{
			Action * action = new Action_Selection(from_index, to_index, ACTION_ID::ACTION_DUMMY_SELECTION);
			ActionManager::GetInstance().push( action );
		}
		else
		{
			Action * action = new Action_Selection( from_index, to_index );
			ActionManager::GetInstance().push(action);
		}
	}

	process_SELECTION_STATE_change_post_order();
}
void InterfaceManager::setSelectedObject( Object_Package object_pack , bool save_action, bool reset)
{
#ifdef CANCER_DEBUG
	if (object_pack.object_index >= 0)
	{
		if (Mediator::RequestSceneObjects( )[object_pack.object_index] == nullptr)
		{
			//std::cout << " InterfaceManager::setSelectedObject( Object_Package ):" << std::endl;
			//std::cout << "       - object_pack.object_index = " << object_pack.object_index << std::endl;
			//std::cout << "       - requested object = nullptr!" << std::endl;
			system("pause");
		}
		
	}
#endif

	check_viewports_SELECT_FOCUS_state(0,1);
	process_SELECTION_STATE_change( object_pack.raw_object );
	
	int from_index = m_selected_object_pack.object_index;
	int to_index   = object_pack.object_index;

	if (m_multiple_selection_active && to_index != from_index && reset)
	{
		if (!(object_pack.raw_object->getType() == GROUP_PARENT_DUMMY && object_pack.raw_object->isTemporary()))
			m_should_peek_ac_flag = false;
		push_Temp_Group_Disbad_Action(m_multiple_selection_active_id, to_index);
		m_should_peek_ac_flag = true;
		save_action = false;
	}

	m_selected_object_pack.raw_object   = object_pack.raw_object;
	m_selected_object_pack.object_index = object_pack.object_index;
	m_selected_object_pack.selected = true;
	mouse_data.m0_dragging_down = 0;

	
	if ( save_action )
	{
		Action * action = new Action_Selection(from_index, to_index);
		ActionManager::GetInstance().push(action);
	}
	
	process_SELECTION_STATE_change_post_order();
}
void InterfaceManager::setSelectedObject( int object_index, bool reset, bool by_id )
{

#ifdef CANCER_DEBUG
	if (object_index >= 0)
	{
		SceneObject* obj = Mediator::RequestSceneObjects(  )[object_index];
		if (obj == nullptr || obj == 0)
		{
			//std::cout << " InterfaceManager::setSelectedObject( "<<object_index<<" ):" << std::endl;
			//std::cout << "       - request object = nullptr!" << std::endl;
			system("pause");
		}
		
	}
#endif
	
	check_viewports_SELECT_FOCUS_state(0, 1);

	SceneObject * obj = nullptr;
	if (object_index >= 0) obj = Mediator::RequestSceneObjects()[object_index];

	if (m_multiple_selection_active && object_index != m_selected_object_pack.object_index && reset)
	{
		if (obj != nullptr)
		{
			if (!(obj->getType() == GROUP_PARENT_DUMMY && obj->isTemporary()))
				m_should_peek_ac_flag = false;
		}
		push_Temp_Group_Disbad_Action(m_multiple_selection_active_id, object_index);
		m_should_peek_ac_flag = true;
	}

	m_selected_object_pack.reset();
	mouse_data.m1_dragging_down = 0;
	
	process_SELECTION_STATE_change(obj);

	m_selected_object_pack.raw_object = obj;
	m_selected_object_pack.object_index = object_index;
	m_selected_object_pack.selected = true;
	mouse_data.m0_dragging_down = 0;

	process_SELECTION_STATE_change_post_order();
}
void InterfaceManager::setSelectedObject(SceneObject* obj, bool reset)
{

	check_viewports_SELECT_FOCUS_state(0, 1);
	process_SELECTION_STATE_change(obj);

	m_selected_object_pack.reset();
	mouse_data.m1_dragging_down = 0;

#ifdef CANCER_DEBUG
	if (obj == nullptr || obj == 0)
	{
		//std::cout << " InterfaceManager::setSelectedObject( object ):" << std::endl;
		//std::cout << "          - object = nullptr!" << std::endl;
	}
	else
	{
		int index = Mediator::requestObjectsArrayIndex(obj);
		if (index == -1)
		{
			//std::cout << " InterfaceManager::setSelectedObject( object["<<obj->getId()<<"] ):" << std::endl;
			//std::cout << "          - object_index = invalid!" << std::endl;
		}
		else
		{
			////std::cout << " InterfaceManager::setSelectedObject( object[" << obj->getId() << "] ):" << std::endl;
			////std::cout << "          - object_index_in_array : " << index << std::endl;
		}
	}
#endif
	
	int obj_index = Mediator::requestObjectsArrayIndex(obj);

	if ( m_multiple_selection_active && m_selected_object_pack.object_index != obj_index && reset )
	{
		if (obj != nullptr)
		{
			if (!(obj->getType() == GROUP_PARENT_DUMMY && obj->isTemporary()))
				m_should_peek_ac_flag = false;
		}
		push_Temp_Group_Disbad_Action( m_multiple_selection_active_id, obj_index );
		m_should_peek_ac_flag = true;
	}

	m_selected_object_pack.raw_object   = obj;
	m_selected_object_pack.object_index = obj_index;
	m_selected_object_pack.selected     = true;
	mouse_data.m0_dragging_down         = 0;
	process_SELECTION_STATE_change_post_order();
}

void InterfaceManager::setSelectedGroup(SceneGroup* group, bool reset)
{
	//m_prev_selected_object_pack = m_selected_object_pack;
	//check_viewports_SELECT_FOCUS_state();

	m_selected_object_pack.reset();
	mouse_data.m1_dragging_down = 0;

	m_selected_object_pack.raw_object = group->getDummyParent();
	m_selected_object_pack.object_index = Mediator::requestObjectsArrayIndex(group->getDummyParent()->getId());
	m_selected_object_pack.selected = true;
	mouse_data.m0_dragging_down = 0;

}

void InterfaceManager::set_active_Utility(Utility utility)
{
	toggle(utility, false);
	
	int ut_index = (int)utility - 1;
	if (ut_index > 0)
		tool_buttons[Utility_ids[ut_index]]->setSelected(true);
	//tool_buttons[ Utility_ids[ (int)utility - 1 ] ]->setSelected(true);
}
void InterfaceManager::set_active_Axis_Orientation(AXIS_ORIENTATION axis)
{

	if (m_selected_axis_orientation != axis)
		m_selected_axis_orientation_changed = true;
	m_selected_axis_orientation = axis;
	
}
void InterfaceManager::set_active_multiple_selection(int index)
{
	m_multiple_selection_active    = true;
	m_multiple_selection_active_id = getArea_Selection_DataIndex_FromParent(index);
}

void InterfaceManager::gui_set_Style(std::string style_id)
{
	guiStyleManager::setStyle(style_id);
}
void InterfaceManager::gui_set_Font(int id, std::string font_attr, int size)
{
	guiStyleManager::setFont(id, font_attr, size);
}
void InterfaceManager::set_FaceVector_States(UtilityProperties ut)
{

	switch (ut.group)
	{

	case g_NONE:
	{
		m_FaceVector_Intersection_State = FIS_NONE;
		m_FaceVector_view_State = FVS_NONE;
	}

	case g_ACTION:
	{
		m_FaceVector_Intersection_State = FIS_NONE;
		m_FaceVector_view_State = FVS_NONE;
	}

	case g_TRANSFORMATIONS:
	{
		m_FaceVector_Intersection_State = FIS_NONE;
		m_FaceVector_view_State = FVS_NONE;
	}

	case g_CALLIBRATION:
	{

	}

	case g_LINKING:
	{
		m_FaceVector_view_State = FVS_NONE;
		m_FaceVector_Intersection_State = FIS_NONE;
	}

	case g_ATTACH:
	{
	
		if      (ut.id == u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED)
		{
			m_FaceVector_view_State = FVS_NONE;
			m_FaceVector_Intersection_State = FIS_NONE;
		}
		else if (ut.id == u_ATTACH_FACE_VECTOR_BBOX_ALIGNED)
		{
			m_FaceVector_view_State = FVS_NONE;
			m_FaceVector_Intersection_State = FIS_NONE;
		}
		else if (ut.id == u_ATTACH_FACE_VECTOR_BBOX_ALIGNED_PARALLEL)
		{
			m_FaceVector_view_State = FVS_NONE;
			m_FaceVector_Intersection_State = FIS_NONE;
		}


	}break;

	case g_RESTRICTIONS:
	{

#ifdef SHOW_FVS_WHEN_G_RESTRICTIONS
		if      (ut.id == u_ATTACH_RESTRICTION_FOCUS)
		{
			m_FaceVector_view_State = FVS_ALL;
			m_FaceVector_Intersection_State = FIS_ALL;
		}
		else if (ut.id == u_ATTACH_RESTRICTION_CONVERSATION)
		{
			m_FaceVector_view_State = FVS_ALL;
			m_FaceVector_Intersection_State = FIS_ALL;
		}
		else if (ut.id == u_ATTACH_RESTRICTION_DISTANCE)
		{
			m_FaceVector_view_State         = FVS_NONE;
			m_FaceVector_Intersection_State = FIS_NONE;
		}
#endif

	}break;

	case g_ALIGN:
	{
		if (ut.id == u_ALIGN_CAMERA)
		{
			m_FaceVector_view_State = FVS_ALL;
			m_FaceVector_Intersection_State = FIS_ALL;
		}
	}break;


	}

}
void InterfaceManager::reset_FaceVector_States()
{
	m_FaceVector_view_State         = FVS_NONE;
	m_FaceVector_Intersection_State = FIS_NONE;
}

#endif

#ifdef INTERFACE_GET_FUNCTIONS


bool InterfaceManager::is_utility_toggle_valid(Utility ut_id)
{
	bool  is_valid = true;
	Utility_Group ut_group = Utilities::get_UtilityGroup(ut_id);
	bool is_g_attach_obj   = (ut_group == g_ATTACH
							&& (ut_id != u_ATTACH_RESTRICTION_FOCUS && ut_id != u_ATTACH_RESTRICTION_CONVERSATION && ut_id != u_ATTACH_RESTRICTION_DISTANCE));
	bool is_g_attach_smplr = (ut_id == u_ATTACH_SAMPLER_PLANAR || ut_id == u_ATTACH_SAMPLER_VOLUME || ut_id == u_ATTACH_SAMPLER_DIRECTIONAL);
	bool is_g_attach_R     = ut_id == u_ATTACH_RESTRICTION_FOCUS || ut_id == u_ATTACH_RESTRICTION_CONVERSATION || ut_id == u_ATTACH_RESTRICTION_DISTANCE;
	if      (is_g_attach_obj)
	{

		bool obj_type_valid  = false;
		bool is_obj_selected = m_selected_object_pack.raw_object != nullptr;
		if (is_obj_selected)
		{
			Type obj_type = m_selected_object_pack.raw_object->getType();
			bool is_area_seletion = obj_type == GROUP_PARENT_DUMMY && m_selected_object_pack.raw_object->isTemporary();
			if (is_g_attach_smplr)
				obj_type_valid = obj_type == IMPORTED_OBJECT || (obj_type == GROUP_PARENT_DUMMY && !is_area_seletion);
			else
				obj_type_valid = obj_type == IMPORTED_OBJECT || (obj_type == GROUP_PARENT_DUMMY && !is_area_seletion) 
				//|| obj_type == LIGHT_OBJECT
				;
		}

		is_valid = obj_type_valid;
	}
	else if (is_g_attach_R)
	{

	}

	return is_valid;
}


bool InterfaceManager::is_Hovering_Over_Camera_Orbit()
{
	return m_hovering_over_camera_orbit;
}
bool InterfaceManager::is_Camera_Orbit_Active()
{
	return m_m0_down_on_camera_control_handle;
}
TAB_ID InterfaceManager::get_Active_Tab()
{
	return m_active_tab;
}
SceneObject* InterfaceManager::get_UI_Focused_Relation()
{
	return m_UI_focused_Relation;
}


Restriction* InterfaceManager::get_UI_Selected_Restriction()
{
	return m_UI_selected_Restriction;
}
int InterfaceManager::get_UI_Selected_Restriction_Id()
{
	return m_UI_selected_Restriction == nullptr? -1 : m_UI_selected_Restriction->get_Unique_Id();
}

Multiple_Selection_Data& InterfaceManager::get_Area_Selection_Data(int parent_id)
{
	int index = 0;
	int i = 0;
	for (Multiple_Selection_Data data : m_area_selections_data)
	{
		if (data.parent_id == parent_id)
		{
			index = i;
			break;
		}

		i++;
	}

	return m_area_selections_data[index];

}
const Multiple_Selection_Data& InterfaceManager::get_Active_Area_Selection_Data()
{
	return m_area_selections_data[m_multiple_selection_active_id];
}
int InterfaceManager::getArea_Selection_DataIndex_FromParent(int id)
{
	int i = 0;
	for (Multiple_Selection_Data data : m_area_selections_data)
	{
		if (data.parent_id == id)
			return i;
		i++;
	}

	return -1;
}
int InterfaceManager::get_Area_Selection_Active_Id()
{
	return m_multiple_selection_active_id;
}
bool InterfaceManager::get_Utility_Interaction()
{
#if defined(DEBUGGING) || defined(FUNCTION_CALL_REPORT_INTERFACE_MANAGER)
	streamHandler::Report("InterfaceManager", "get_Utility_Interaction", "");
#endif

	
	m_utility_isec_p.reset();
	
	

	if(
		//(is_obj_valid && is_type_valid) 
	  // || is_create
	  //||
		true
	   )
	{
		UtilityProperties interaction_utility = m_selected_utility;
		int utility_interaction = get_active_utility_intersectionPoint(  
			 Mediator::RequestWindow(), 
			&Mediator::RequestCameraInstance(), 
			&m_selected_object_pack,
			&m_utility_isec_p,
			 interaction_utility);

		return utility_interaction;
	}

	return false;
}
Interface_State * InterfaceManager::get_current_Interface_State()
{
	return &m_interface_state;
}
AXIS_ORIENTATION InterfaceManager::get_active_Axis_Orientation()
{
	return m_selected_axis_orientation;
}
bool InterfaceManager::is_selected_utility_active()
{
	return m_selected_utility.active;
}
bool InterfaceManager::isGuiFocused()
{
	return m_gui_focused;
}
int InterfaceManager::get_Ui_Focused_Object()
{
	return m_ui_focused_object_id;
}
Io_Mouse InterfaceManager::getMouseData()
{
	return mouse_data;
}
optix::float2 InterfaceManager::get_MousePos_RelativeTo(int id) // VIEWPORT_ID
{
	//ImGui_Window_Data window = imgui_windows[window_id];
	float* global_mousePos  = Mediator::RequestMousePos();
	optix::float2 relative_mousePos;


	// offsets for Main Viewport only
	int offset_x = imgui_windows[LEFT_PANEL].pos.x + imgui_windows[LEFT_PANEL].size.x;
	int offset_y = imgui_windows[LEFT_PANEL].pos.y;

	relative_mousePos.x = global_mousePos[0] - (offset_x);
	relative_mousePos.y = global_mousePos[1] - (offset_y);

	delete global_mousePos;
	return relative_mousePos;
}
Dimension_Constraints InterfaceManager::getViewportSpaceConstraints()
{
	Dimension_Constraints con;
	con.LEFT   = imgui_windows[LEFT_PANEL  ].size.x;
	con.RIGHT  = imgui_windows[RIGHT_PANEL ].size.x;
	con.TOP    = imgui_windows[LEFT_PANEL  ].pos.y;
	con.BOTTOM = imgui_windows[BOTTOM_PANEL].size.y;
	return con;
}
int InterfaceManager::getFocusedViewport()
{
	////std::cout << "\n - getFocusedViewport():" << std::endl;

	optix::float2 mousePos = optix::make_float2(Mediator::RequestMousePos(0), Mediator::RequestMousePos(1));
	////std::cout << "  > MOUSE_POS : [ " << mousePos.x << " , " << mousePos.y << " ] " << std::endl;

	std::vector<VIEWPORT> viewports = ViewportManager::GetInstance().getAllEnabledViewports();
	for (VIEWPORT vp : viewports)
	{
		int x = mousePos.x;
		int y = mousePos.y;
		
		
		int xmin = vp.dimensions.offset.x;
		int xmax = (xmin + vp.renderSize.x);
		int ymin = m_window_height - ( vp.dimensions.offset.y + vp.renderSize.y );
		int ymax = ymin + vp.renderSize.y;
		
		////std::cout << "\n - Viewport        : " << Utilities::getViewportNameFromID(vp.id) << std::endl;
		////std::cout << "    - size       : ( " << vp.dimensions.size.x << " , " << vp.dimensions.size.y << " ) " << std::endl;
		////std::cout << "    - renderSize : ( " << vp.renderSize.x << " , " << vp.renderSize.y << " ) " << std::endl;
		////std::cout << "    - offset     : ( " << vp.dimensions.offset.x << " , " << vp.dimensions.offset.y << " ) " << std::endl;
		////std::cout << "    - min        : [ " << xmin << " , " << ymin << " ] " << std::endl;
		////std::cout << "    - max        : [ " << xmax << " , " << ymax << " ] " << std::endl;

		if (x >= xmin && x <= xmax)
			if (y >= ymin && y <= ymax)
			{
				return (int)vp.id;
			}
	}

	return -1;
}

bool InterfaceManager::is_multiple_selection_Active()
{
	return m_multiple_selection_active;
}
bool InterfaceManager::checkIntegrity()
{
	bool is_ok = true;

	// interface_state
	bool interface_state_ok = true;
	int sel_index = m_interface_state.selected_object_pack->object_index;
	if (sel_index >= 0 && Mediator::requestObjectsArrayIndex(sel_index) == -1)
	{
		interface_state_ok = false;
	}

	// m_selected_object_pack && m_focused_object_pack && m_prev_selected_object_pack
	bool m_selected_object_ok = (m_selected_object_pack.object_index >= 0 && Mediator::requestObjectsArrayIndex(m_selected_object_pack.object_index) != -1)
		&& (m_selected_object_pack.raw_object == nullptr && m_selected_object_pack.object_index == -1);
	bool m_focused_object_ok = (m_focused_object_pack.object_index >= 0 && Mediator::requestObjectsArrayIndex(m_focused_object_pack.object_index) != -1)
		&& (m_focused_object_pack.raw_object == nullptr && m_focused_object_pack.object_index == -1);
	bool m_prev_selected_object_ok = (m_prev_selected_object_pack.object_index >= 0 && Mediator::requestObjectsArrayIndex(m_prev_selected_object_pack.object_index) != -1)
		&& (m_prev_selected_object_pack.raw_object == nullptr && m_prev_selected_object_pack.object_index == -1);
	
	if (   !interface_state_ok
		|| !m_selected_object_ok
		|| !m_focused_object_ok
		|| !m_prev_selected_object_ok
		)
	{
		is_ok = false;
		//std::cout << " InterfaceManager::checkIntegrity():" << std::endl;
		//std::cout << "      - interface_state_ok             : " << interface_state_ok << std::endl;
		//std::cout << "                  - Interface_State.selected_object.object_index               : " << m_interface_state.selected_object_pack->object_index << std::endl;
		//std::cout << "                  - Interface_State.selected_object.raw_object ( != nullptr? ) : " << !(m_interface_state.selected_object_pack->raw_object == nullptr) << std::endl;

		//std::cout << "      - m_selected_object_pack_ok      : " << m_selected_object_ok << std::endl;
		//std::cout << "                  - m_selected_object_pack.object_index               : " << m_selected_object_pack.object_index << std::endl;
		//std::cout << "                  - m_selected_object_pack.raw_object ( != nullptr? ) : " << !(m_selected_object_pack.raw_object == nullptr) << std::endl;


		//std::cout << "      - m_focused_object_pack_ok       : " << m_focused_object_ok << std::endl;
		//std::cout << "                  - m_focused_object_pack.object_index               : " << m_focused_object_pack.object_index << std::endl;
		//std::cout << "                  - m_focused_object_pack.raw_object ( != nullptr? ) : " << !(m_focused_object_pack.raw_object == nullptr) << std::endl;

		//std::cout << "      - m_prev_selected_object_pack_ok : " << m_prev_selected_object_ok << std::endl;
		//std::cout << "                  - m_prev_selected_object_pack.object_index               : " << m_prev_selected_object_pack.object_index << std::endl;
		//std::cout << "                  - m_prev_selected_object_pack.raw_object ( != nullptr? ) : " << !(m_prev_selected_object_pack.raw_object == nullptr) << std::endl;
	}


	return is_ok;
}
std::string InterfaceManager::get_LabelIcon_by_Type(Type type)
{
	switch (type)
	{

	case Type::FACE_VECTOR:
		return ICON_MDI_AXIS_Y_ARROW;
	case Type::GROUP_PARENT_DUMMY:
		return ICON_MDI_GROUP;
	case Type::IMPORTED_OBJECT:
		return ICON_MDI_CUBE_OUTLINE;
	case Type::LIGHT_OBJECT:
		return ICON_MDI_SPOTLIGHT_BEAM;
	case Type::SAMPLER_OBJECT:
		return ICON_MDI_SHAPE;
	case Type::UTILITY_OBJECT:
		return ICON_MDI_CUBE;

	}

	std::cout << " ~InterfaceManager::get_LabelIcon_by_Type(): returning nullptr!" << std::endl;
	system("pause");
}

FaceVector_View_State InterfaceManager::get_FaceVector_View_State()
{
	return m_FaceVector_view_State;
}
FaceVector_Intersection_State InterfaceManager::get_FaceVector_Intersection_State()
{
	return m_FaceVector_Intersection_State;
}

SceneObject* InterfaceManager::Get_Focused_Object()
{
	return m_focused_object_pack.raw_object;
}
SceneObject* InterfaceManager::Get_Selected_Object()
{
	return m_selected_object_pack.raw_object;
}
Object_Package InterfaceManager::Get_Focused_Object_Pack()
{
	return m_focused_object_pack;
}
Object_Package InterfaceManager::Get_Selected_Object_Pack()
{
	return m_selected_object_pack;
}
SceneObject* InterfaceManager::Get_prev_Selected_Object()
{
	return m_prev_INTERFACE_selected_object_pack.raw_object;
}
SceneObject* InterfaceManager::Get_prev_Focused_Object()
{
	return m_prev_INTERFACE_focused_object_pack.raw_object;
}
int InterfaceManager::Get_prev_Selectecd_Object_Id()
{
	return (m_prev_INTERFACE_selected_object_pack.raw_object == nullptr) ? -1 : m_prev_INTERFACE_selected_object_pack.raw_object->getId();
}
int InterfaceManager::Get_prev_Focused_Object_id()
{
	return (m_prev_INTERFACE_focused_object_pack.raw_object == nullptr) ? -1 : m_prev_INTERFACE_focused_object_pack.raw_object->getId();
}

bool InterfaceManager::is_Object_Align_Active()
{
	return m_popup_object_align_window;
}
SceneObject* InterfaceManager::get_Object_Align_SourceObj()
{
	return m_align_object;
}
SceneObject* InterfaceManager::get_Object_Align_TargetObj()
{
	return m_align_target;
}

bool InterfaceManager::is_cam_align_PickingObject()
{
	return m_camera_align_picking_Object;
}
bool InterfaceManager::is_cam_align_PickingTarget()
{
	return m_camera_align_picking_Target;
}
bool InterfaceManager::is_cam_align_PickedObject()
{
	return m_camera_align_picked_Object;
}
bool InterfaceManager::is_cam_align_PickedTarget()
{
	return m_camera_align_picked_Target;
}
NormalVector_Obj InterfaceManager::get_cam_align_SourceObj()
{
	return m_camera_align_object;
}
NormalVector_Obj InterfaceManager::get_cam_align_TargetObj()
{
	return m_camera_align_target;
}

bool InterfaceManager::is_Object_Valid_For_Camera_Align(class SceneObject* object)
{
	if (object == 0) return false;
	Type type = object->getType();
	//if (type != FACE_VECTOR) return true;
	return true;
}


std::string InterfaceManager::get_IconLabel(Type obj_type)
{

	switch (obj_type)
	{
	case IMPORTED_OBJECT: return ICON_MDI_CUBE_OUTLINE;
	case GROUP_PARENT_DUMMY: return ICON_MDI_GROUP;
	case LIGHT_OBJECT: return ICON_MDI_SPOTLIGHT_BEAM;
	case SAMPLER_OBJECT: return ICON_MDI_SHAPE;
	case FACE_VECTOR: return ICON_MDI_RAY_START_ARROW;
	}

	return "get_IconLabel_uknown";
}
UtilityProperties InterfaceManager::get_active_Utility()
{
	return m_selected_utility;
}

#endif