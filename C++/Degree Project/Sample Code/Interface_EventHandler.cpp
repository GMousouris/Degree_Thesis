
#include "InterfaceManager.h"
#include "common_structs.h"
#include "GeometryFunctions.h"
#include "Utilities.h"
#include "Mediator.h"
#include "PinholeCamera.h"
#include "SceneObject.h"
#include "ActionManager.h"
#include "Action_Childs.h"
#include "InterfaceManager_HelperFunctions.h"
#include "Transformations.h"
#include "Restriction_Types.h"
#include "Defines.h"

#include <stack>

#ifdef ButtonHandler


// outside API button event handler
void InterfaceManager::toggle(Utility ut_id, bool push_action, bool toggle_button, bool * is_valid )
{

#ifdef Action_UTILITY_SELECTION_Enabled
	if (push_action)
	{
		Action * action = new Action_UTILITY_SELECTION(m_selected_utility.id, ut_id);
		ActionManager::GetInstance().push(action);
	}
#endif

	bool is_toggling_valid = is_utility_toggle_valid(ut_id);
	if ( !is_toggling_valid )
	{
		std::string ut_label = Utilities::convert_Utility_To_Utility_id(ut_id);
		tool_buttons[ut_label]->setSelected(false);
		ut_id = u_NONE;
	}
	if (is_valid != 0)  
		*is_valid = is_toggling_valid;

	switch (ut_id)
	{

	case u_UNDO:
		toggle_UNDO();
		break;
	case u_REDO:
		toggle_REDO();
		break;



	case u_LINK:
		toggle_LINK();
		break;
	case u_UNLINK:
		toggle_UNLINK();
		break;




	case u_SELECTION:
		toggle_SELECTION();
		break;
	case u_SELECTION_GROUP:
		toggle_SELECTION_GROUP();
		break;
	case u_SELECTION_AREA:
		toggle_SELECTION_AREA();
		break;




	case u_TRANSLATION:
		toggle_TRANSLATION();
		break;
	case u_ROTATION:
		toggle_ROTATION();
		break;
	case u_SCALE:
		toggle_SCALE();
		break;




	case u_CALLIBRATION_ANALOGUE:
		toggle_Callibration_Analogue();
		break;
	case u_CALLIBRATION_FIXED:
		toggle_Callibration_Fixed();
		break;


	case u_CREATE_LIGHT:
		toggle_Create_Light();
		break;



	case u_ATTACH_SAMPLER_PLANAR:
		toggle_Attach_Sampler_Planar();
		break;
	case u_ATTACH_SAMPLER_VOLUME:
		toggle_Attach_Sampler_Volume();
		break;
	case u_ATTACH_SAMPLER_DIRECTIONAL:
		toggle_Attach_Sampler_Directional();
		break;




	case u_ATTACH_FACE_VECTOR_BBOX_ALIGNED:
		toggle_Attach_Face_Vector_BBOX_Aligned();
		break;
	case u_ATTACH_FACE_VECTOR_BBOX_ALIGNED_PARALLEL:
		toggle_Attach_Face_Vector_BBOX_Aligned_Parallel();
		break;
	case u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED:
		toggle_Attach_Face_Vector_Normal_Aligned();
		break;





	case u_ATTACH_RESTRICTION_FOCUS:
		toggle_Attach_Restriction_Focus();
		break;
	case u_ATTACH_RESTRICTION_CONVERSATION:
		toggle_Attach_Restriction_Conversation();
		break;
	case u_ATTACH_RESTRICTION_DISTANCE: 
		toggle_Attach_Restriction_Distance();
		break;


	case u_ALIGN:
		toggle_Object_Align();
		break;
	case u_ALIGN_CAMERA:
		toggle_Camera_Align();
		break;


	case u_SCENE_EDIT_MODE:
		toggle_Scene_Edit_Mode();
		break;
	case u_SCENE_RENDER_MODE:
		toggle_Scene_Render_Mode();
		break;

	case u_TOGGLE_LIGHTS:
		toggle_Scene_Lights_Toggle();
		break;

	case u_OTHER:
		break;

	}


	if (ut_id == u_TOGGLE_LIGHTS)
	{
		return;
	}

	if ( toggle_button && ut_id != u_NONE )
	{
		std::string ut_label = Utilities::convert_Utility_To_Utility_id( ut_id );
		tool_buttons[ ut_label.c_str() ]->setSelected( true );
	}

	

	


}

#ifdef MainMenuBar_Events

#endif


#ifdef Tools_Events

void InterfaceManager::toggle_UNDO()
{
	//m_selected_utility.id = Utility::u_UNDO;
	//m_selected_utility.group = Utility_Group::g_ACTION;
	
	handle_UNDO();
}
void InterfaceManager::toggle_REDO()
{

	//m_selected_utility.id = Utility::u_REDO;
	//m_selected_utility.group = Utility_Group::g_ACTION;
	handle_REDO();
}
void InterfaceManager::toggle_LINK()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
		//m_prev_selected_utility = m_selected_utility;

	m_selected_utility.id = Utility::u_LINK;
	m_selected_utility.group = Utility_Group::g_LINKING;
	reset_FaceVector_States();
}
void InterfaceManager::toggle_UNLINK()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
		//m_prev_selected_utility = m_selected_utility;

	//m_selected_utility.id = Utility::u_UNLINK;
	//m_selected_utility.group = Utility_Group::g_LINKING;
	handle_UNLINK();
	//reset_FaceVector_States();
}
void InterfaceManager::toggle_SELECTION()
{
	
	if (m_selected_utility.id == u_SELECTION_AREA)
	{
		m_selected_utility.id   = u_NONE;
		m_selected_utility.group = g_NONE;
	}

	m_selection_setting = S_OBJECT;
	reset_FaceVector_States();
}
void InterfaceManager::toggle_SELECTION_GROUP()
{
	
	if (m_selected_utility.id == u_SELECTION_AREA)
	{
		m_selected_utility.id = u_NONE;
		m_selected_utility.group = g_NONE;
	}

	m_selection_setting = S_GROUP;
	reset_FaceVector_States();
}
void InterfaceManager::toggle_SELECTION_AREA()
{
	
	if ( m_selected_utility.id != u_SELECTION && m_selected_utility.id != u_SELECTION_GROUP )
		m_prev_selected_utility = m_selected_utility;

	if( m_selected_utility.group != g_SELECTIONS )
		m_prev_utility = m_selected_utility.id;

	
	m_selected_utility.id    = Utility::u_SELECTION_AREA;
	m_selected_utility.group = Utility_Group::g_SELECTIONS;
	
	//m_selection_setting = S_OBJECT; // EDW //
	reset_FaceVector_States();
}
void InterfaceManager::toggle_TRANSLATION()
{
	//
	if (m_selected_utility.group == g_TRANSFORMATIONS)
		m_prev_selected_utility = m_selected_utility;
	m_selected_utility.id    = Utility::u_TRANSLATION;
	m_selected_utility.group = Utility_Group::g_TRANSFORMATIONS;
	reset_FaceVector_States();
}
void InterfaceManager::toggle_ROTATION()
{
	if (m_selected_utility.group == g_TRANSFORMATIONS)
		m_prev_selected_utility = m_selected_utility;
	m_selected_utility.id = Utility::u_ROTATION;
	m_selected_utility.group = Utility_Group::g_TRANSFORMATIONS;
	reset_FaceVector_States();
	//set_FaceVector_States(m_selected_utility);
}
void InterfaceManager::toggle_SCALE()
{
	if (m_selected_utility.group == g_TRANSFORMATIONS)
		m_prev_selected_utility = m_selected_utility;
	m_selected_utility.id = Utility::u_SCALE;
	m_selected_utility.group = Utility_Group::g_TRANSFORMATIONS;
	reset_FaceVector_States();
	//set_FaceVector_States(m_selected_utility);
}
void InterfaceManager::toggle_AXIS_SELECTION(bool push_action)
{
	//m_prev_selected_utility = m_selected_utility;

	bool is_selected = false;
	AXIS_ORIENTATION before_axis = m_selected_axis_orientation;
	m_selected_axis_orientation_changed = false;

	//ImGui::Dummy(ImVec2(0.0f, 4.0f));
	if (ImGui::Selectable("world aligned ", &is_selected))
	{
		if (m_selected_axis_orientation != AXIS_ORIENTATION::WORLD)
			m_selected_axis_orientation_changed = true;
		m_selected_axis_orientation = AXIS_ORIENTATION::WORLD;
	}
	if (ImGui::Selectable("local     ", &is_selected))
	{
		if (m_selected_axis_orientation != AXIS_ORIENTATION::LOCAL)
			m_selected_axis_orientation_changed = true;
		m_selected_axis_orientation = AXIS_ORIENTATION::LOCAL;
	}
	if (ImGui::Selectable("screen ", &is_selected))
	{
		if (m_selected_axis_orientation != AXIS_ORIENTATION::SCREEN)
			m_selected_axis_orientation_changed = true;
		m_selected_axis_orientation = AXIS_ORIENTATION::SCREEN;
	}
	
	ImGui::Dummy(ImVec2(0.0f, 4.0f));




}

void InterfaceManager::toggle_Callibration_Analogue()
{
	m_analogue_callibration = true;
	//m_prev_selected_utility = m_selected_utility;
	//m_selected_utility.id = Utility::u_CALLIBRATION_ANALOGUE;
	//m_selected_utility.group = Utility_Group::g_CALLIBRATION;
}
void InterfaceManager::toggle_Callibration_Fixed()
{
	m_analogue_callibration = false;
	
	//m_prev_selected_utility = m_selected_utility;
	//m_selected_utility.id = Utility::u_CALLIBRATION_FIXED;
	//m_selected_utility.group = Utility_Group::g_CALLIBRATION;
}

void InterfaceManager::toggle_Create_Light()
{
	
	m_selected_utility.id    = Utility::u_CREATE_LIGHT;
	m_selected_utility.group = Utility_Group::g_CREATE;
	reset_FaceVector_States();

	reset_Force_Tabs_Open_Flags();
	
	m_force_tab_open_active            = true;
	//m_force_tab_open[LIGHT_PARAMS_TAB] = true;
	m_force_tab_open[GENERAL_INFO_TAB] = true;
	
}

void InterfaceManager::toggle_Attach_Sampler_Planar()
{
	
	m_selected_utility.id    = Utility::u_ATTACH_SAMPLER_PLANAR;
	m_selected_utility.group = Utility_Group::g_ATTACH;
	reset_FaceVector_States();

	reset_Force_Tabs_Open_Flags();
	SceneObject* obj = m_selected_object_pack.raw_object;
	bool is_valid = obj != nullptr;
	if (is_valid) is_valid = obj->getType() != FACE_VECTOR
		&& obj->getType() != LIGHT_OBJECT
		&& obj->getType() != SAMPLER_OBJECT;
		
	if (is_valid)
	{
		m_force_tab_open_active = true;
		m_force_tab_open[SAMPLER_PARAMS_TAB] = true;
		
		optix::float3 * bbox = obj->getBBOX_Transformed();
		optix::float3 b_min  = bbox[0];
		optix::float3 b_max  = bbox[1];
		delete bbox;
		float size = length(b_max - b_min) * 0.1f;
		m_interface_state.m_sampler_params.dim_2f = optix::make_float2(size);
		
	}

	m_sampler_crt_def_type = 1;
	
}
void InterfaceManager::toggle_Attach_Sampler_Volume()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
		//m_prev_selected_utility = m_selected_utility;

	m_selected_utility.id = Utility::u_ATTACH_SAMPLER_VOLUME;
	m_selected_utility.group = Utility_Group::g_ATTACH;
	reset_FaceVector_States();

	reset_Force_Tabs_Open_Flags();
	SceneObject* obj = m_selected_object_pack.raw_object;
	bool is_valid = obj != nullptr;
	if (is_valid) is_valid = obj->getType() != FACE_VECTOR && obj->getType() != SAMPLER_OBJECT;
	if (is_valid)
	{
		m_force_tab_open_active = true;
		m_force_tab_open[SAMPLER_PARAMS_TAB] = true;
		//m_interface_state.m_sa = SA_GLOBAL;

		optix::float3 * bbox = obj->getBBOX_Transformed();
		optix::float3 b_min = bbox[0];
		optix::float3 b_max = bbox[1];
		delete bbox;
		float size = length(b_max - b_min) * 0.1f * 2.0f;
		m_interface_state.m_sampler_params.dim_3f = optix::make_float3(size);
	}
	
	//m_sampler_init_isec_object_bbox = 0;
	//m_interface_state.m_sampler_init_use_bbox = 0;
}
void InterfaceManager::toggle_Attach_Sampler_Directional()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
		//m_prev_selected_utility = m_selected_utility;

	m_selected_utility.id = Utility::u_ATTACH_SAMPLER_DIRECTIONAL;
	m_selected_utility.group = Utility_Group::g_ATTACH;
	reset_FaceVector_States();

	reset_Force_Tabs_Open_Flags();
	SceneObject* obj = m_selected_object_pack.raw_object;
	bool is_valid = obj != nullptr;
	if (is_valid) is_valid = obj->getType() != FACE_VECTOR && obj->getType() != SAMPLER_OBJECT;
	if (is_valid)
	{
		m_force_tab_open_active = true;
		m_force_tab_open[SAMPLER_PARAMS_TAB] = true;
		//m_interface_state.m_sa = SA_NORMAL;

		optix::float3 * bbox = obj->getBBOX_Transformed();
		optix::float3 b_min = bbox[0];
		optix::float3 b_max = bbox[1];
		delete bbox;
		float size = length(b_max - b_min) * 0.15f;
		m_interface_state.m_sampler_params.dim_2f = optix::make_float2(size);
		m_interface_state.m_sampler_params.cone_size = optix::make_float2(25.0f, 25.0f);
	}

	//m_sampler_init_isec_object_bbox = 0;
	//m_interface_state.m_sampler_init_use_bbox = 0;
}

void InterfaceManager::toggle_Attach_Face_Vector_BBOX_Aligned()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
	//	m_prev_selected_utility = m_selected_utility;
	m_selected_utility.id    = Utility::u_ATTACH_FACE_VECTOR_BBOX_ALIGNED;
	m_selected_utility.group = Utility_Group::g_ATTACH;
	reset_FaceVector_States();
}
void InterfaceManager::toggle_Attach_Face_Vector_BBOX_Aligned_Parallel()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
		//m_prev_selected_utility = m_selected_utility;

	m_selected_utility.id    = Utility::u_ATTACH_FACE_VECTOR_BBOX_ALIGNED_PARALLEL;
	m_selected_utility.group = Utility_Group::g_ATTACH;
	reset_FaceVector_States();
}
void InterfaceManager::toggle_Attach_Face_Vector_Normal_Aligned()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
	//	m_prev_selected_utility = m_selected_utility;
	m_selected_utility.id = Utility::u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED;
	m_selected_utility.group = Utility_Group::g_ATTACH;
	reset_FaceVector_States();
}

void InterfaceManager::toggle_Attach_Restriction_Focus()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
		//m_prev_selected_utility = m_selected_utility;

	m_selected_utility.id = Utility::u_ATTACH_RESTRICTION_FOCUS;
	m_selected_utility.group = Utility_Group::g_RESTRICTIONS;
	set_FaceVector_States(m_selected_utility);
}
void InterfaceManager::toggle_Attach_Restriction_Conversation()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
		//m_prev_selected_utility = m_selected_utility;

	m_selected_utility.id    = Utility::u_ATTACH_RESTRICTION_CONVERSATION;
	m_selected_utility.group = Utility_Group::g_RESTRICTIONS;
	set_FaceVector_States(m_selected_utility);
}
void InterfaceManager::toggle_Attach_Restriction_Distance()
{
	//if (m_prev_selected_utility.id != m_selected_utility.id)
		//m_prev_selected_utility = m_selected_utility;

	m_selected_utility.id = Utility::u_ATTACH_RESTRICTION_DISTANCE;
	m_selected_utility.group = Utility_Group::g_RESTRICTIONS;
	set_FaceVector_States(m_selected_utility);
}

void InterfaceManager::toggle_Object_Align()
{
	
	m_selected_utility.id = Utility::u_ALIGN;
	m_selected_utility.group = Utility_Group::g_ALIGN;
	reset_FaceVector_States();

}
void InterfaceManager::toggle_Camera_Align()
{

	m_selected_utility.id    = Utility::u_ALIGN_CAMERA;
	m_selected_utility.group = Utility_Group::g_ALIGN;
	set_FaceVector_States(m_selected_utility);
}

void InterfaceManager::toggle_Scene_Edit_Mode()
{

	
	m_scene_mode = Edit_Mode;

	optix::Context m_context = Mediator::RequestContext();
	OptiXShader& shader      = Mediator::request_ActiveOptiXShaderId();

	m_context["min_depth"]->setUint(m_sceneEdit_settings.ray_depth.x);
	m_context["max_depth"]->setUint(m_sceneEdit_settings.ray_depth.y);
	shader.data.min_depth = m_sceneEdit_settings.ray_depth.x;
	shader.data.max_depth = m_sceneEdit_settings.ray_depth.y;
	m_context["cast_shadow_ray"]->setInt(m_scene_lights_on);
	m_context["sys_cast_reflections"]->setInt(m_sceneEdit_settings.reflections);
	m_context["sys_sample_glossy_index"]->setUint(m_sceneEdit_settings.glossiness);
	m_context["sys_use_fresnel_equations"]->setInt(m_sceneEdit_settings.fresnel);


	Mediator::restartAccumulation();
}
void InterfaceManager::toggle_Scene_Render_Mode()
{
	

	m_scene_mode = Render_Mode;

	optix::Context m_context = Mediator::RequestContext();
	OptiXShader& shader      = Mediator::request_ActiveOptiXShaderId();
	
	
	m_context["min_depth"]->setUint(m_sceneRender_settings.ray_depth.x);
	m_context["max_depth"]->setUint(m_sceneRender_settings.ray_depth.y);
	shader.data.min_depth = m_sceneRender_settings.ray_depth.x;
	shader.data.max_depth = m_sceneRender_settings.ray_depth.y;
	m_context["cast_shadow_ray"]->setInt(m_scene_lights_on);
	m_context["sys_cast_reflections"]->setInt(m_sceneRender_settings.reflections);
	m_context["sys_sample_glossy_index"]->setUint(m_sceneRender_settings.glossiness);
	m_context["sys_use_fresnel_equations"]->setInt(m_sceneRender_settings.fresnel);
	Mediator::restartAccumulation();
}
void InterfaceManager::toggle_Scene_Lights_Toggle()
{
	m_scene_lights_on = !m_scene_lights_on;
	optix::Context m_context = Mediator::RequestContext();
	m_context["cast_shadow_ray"]->setInt((int)m_scene_lights_on);
	Mediator::restartAccumulation();

	//tool_buttons["scene_toggle_lights"]->selected = m_scene_lights_on;
}

#endif



#ifdef Tools_Helpers_Events

void InterfaceManager::toggle_Linking_Settings()
{
	
}
void InterfaceManager::toggle_Hierarchy_Settings()
{
	std::string label = std::string("") + Utilities::getTRS_setting_toString(m_hierarchy_setting);
	if (ImGui::BeginCombo("hiearchy_settings##", label.c_str() ))
	{
		//ImGui::Dummy(ImVec2(0.0f, 4.0f));
		if (ImGui::Selectable("DEFAULT      "))
		{
			m_hierarchy_setting = TO_ALL;
		}
		if (ImGui::Selectable("OBJECT ONLY "))
		{
			m_hierarchy_setting = ONLY_THIS;
		}
		if (ImGui::Selectable("CHILDS ONLY "))
		{
			m_hierarchy_setting = ONLY_TO_CHILDS;
		}

		if (m_multiple_selection_active)
		{
			if (m_hierarchy_setting == ONLY_THIS)
				m_hierarchy_setting = TO_ALL;
		}

		ImGui::Dummy(ImVec2(0.0f, 4.0f));
		ImGui::EndCombo();
	}
}

#endif



#ifdef LeftPanel_Events

//void InterfaceManager::toggle_CollapsingHeader_SceneObjects() {}

#endif



#ifdef RightPanel_Events

#endif


#endif

#ifdef EventHandler


void InterfaceManager::handle_m0_CLICKED()
{
	//if (m_gui_focused || m_popup_open)
	//	return;

}
void InterfaceManager::handle_m0_DOWN()
{


	//if (m_gui_focused || m_popup_open)
	//	return;

	if (!m_selected_utility.active 
		&& m_m0_down_on_camera_control_handle)
	{
		glfwSetInputMode(Mediator::RequestWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}

	
	if (m_retargeting_camera)
		return;

	if (m_wait_to_release)
		return;

	
	// o ACTION : [ o - activate Area Selection if it is current selected utility ]
	if (    m_selected_utility.id == u_SELECTION_AREA
		&& !m_selected_utility.active
		&& !m_gui_focused
		&& m_selected_utility.group != g_ATTACH
		)
	{
		m_selected_utility.active = true;
	}


	
	////std::cout << " m0_down : " << mouse_data.m0_clicked << std::endl;
	if (m_selected_utility.group == g_ATTACH
		&& !m_gui_focused
		)
	{
		bool is_obj_valid = m_selected_object_pack.raw_object != nullptr;
		/* ATTACHABLE OBJECT TYPES */
		bool is_type_valid = is_obj_valid ? (m_selected_object_pack.raw_object->getType() != FACE_VECTOR && m_selected_object_pack.raw_object->getType() != SAMPLER_OBJECT) : false;
		bool is_temp = is_obj_valid ? m_selected_object_pack.raw_object->isTemporary() : true;
		bool is_valid = is_obj_valid && is_type_valid && !is_temp;

		bool is_attach_sampler = m_selected_utility.id == u_ATTACH_SAMPLER_VOLUME || m_selected_utility.id == u_ATTACH_SAMPLER_DIRECTIONAL || m_selected_utility.id == u_ATTACH_SAMPLER_PLANAR;
		if ( is_attach_sampler )
		{
			bool use_bbox = m_sampler_init_isec_object_bbox;
			SceneObject* focused_obj  = m_focused_object_pack.raw_object;
			SceneObject* selected_obj = m_selected_object_pack.raw_object;
			int focused_obj_id  = focused_obj  != nullptr ? focused_obj->getId()  : -1;
			int selected_obj_id = selected_obj != nullptr ? selected_obj->getId() : -1;
			bool is_group = selected_obj_id != -1 ? selected_obj->getType() == GROUP_PARENT_DUMMY : false;
			bool is_active = selected_obj != nullptr ? (selected_obj_id == focused_obj_id) : false;
			if (is_group && !is_active && focused_obj != nullptr)
			{
				is_active = Utilities::is_object_in_List(focused_obj, selected_obj->getGroupSelectionChilds_and_Groups());
			}

			if (!use_bbox)
			{
				if (is_active && mouse_data.m0_clicked)
					m_selected_utility.active = true;
			}
			else
			{
				if (m_utility_isec_p.hit_index >= 0 && mouse_data.m0_clicked && is_valid)
					m_selected_utility.active = true;
			}
		}
		else if (m_utility_isec_p.hit_index >= 0 && mouse_data.m0_clicked && is_valid)
			m_selected_utility.active = true;
	}

	// g_create
	if (   m_selected_utility.group == g_CREATE
		&& mouse_data.m0_clicked
		&& !m_gui_focused)
	{
		//bool is_norm = m_interface_state.m_sa != SA_GLOBAL;
		//if (is_norm && m_focused_object_pack.raw_object != nullptr)
		//	m_selected_utility.active = true;
	}


	

#ifdef OBJECT_SELECT_ON_MOUSE_PRESS

	// o ACTION : [ single selection ]
	// o ACTION : [ add object to -> multiple selection ( Ctrl - is - pressed ) ]
	// o ACTION : [ subtract object from -> multiple selection ( alt - is - pressed ) ]
	// o ACTION : [ link subject to -> object if <Linking> is active ]
	// o ACTION : [ link subject to -> object if <FaceVector_Linking> is active ]
	if (    m_focused_object_pack.raw_object != nullptr && !mouse_data.m0_dragging_down 
		&& !m_vp_frame_buttons_focused
		)
	{
		optix::float3 orbit_center = m_focused_object_pack.raw_object->getCentroid_Transformed( true );
		bool in_frustum = !Utilities::frustumCull( orbit_center , ViewportManager::GetInstance().getActiveViewport());
		if (in_frustum 
			&& !m_selected_utility.active
			&& !m_utility_isec_p.is_lightButton_hit
			&& !key_data[GLFW_KEY_SPACE].pressed
			)
		{
			ViewportManager::GetInstance().getActiveViewportCamera().setOrbitCenter(orbit_center, false, true);
		}
	}

	if ( m_focused_object_pack.raw_object != nullptr && !mouse_data.m0_dragging_down 
		&& !m_utility_interaction 
		&& m_utility_isec_p.hit_index == -1
		&& !m_utility_isec_p.is_lightButton_hit
		 //&& !m_gui_focused 
	   )
	{

		Type obj_type = m_focused_object_pack.raw_object->getType();
		bool is_selected_valid = (m_selected_object_pack.raw_object == nullptr) ? true : 
			m_selected_object_pack.raw_object->getType() != FACE_VECTOR 
			&& m_selected_object_pack.raw_object->getType() != SAMPLER_OBJECT;

		
		// o ACTION : [ single selection ]
		// o ACTION : [ link subject to -> object if <Linking> is active
		// o ACTION : [ unlink object ]
		// o ACTION : [ g_ALIGN ]
		if (     ui_key_grp_state != UI_KEY_GRP_EDIT
			|| ( ui_key_grp_state == UI_KEY_GRP_EDIT && m_selected_object_pack.raw_object == nullptr)
			&&  !mouse_data.m0_dragging)
		{
			
			// o ACTION : [ link subject to -> object if <Linking> is active ]
			if (m_selected_utility.group == g_LINKING && ( obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT ) 
				&& mouse_data.m0_clicked
				)
			{
				
				if ( m_selected_object_pack.object_index != m_focused_object_pack.object_index && m_selected_utility.id == u_LINK
					&& !create_area_selection)
				{
					setSelectedObject(m_focused_object_pack, !m_cloning_active);
					m_reselecting = true;
					handle_m0_RELEASED();
				}
				m_selected_utility.active = true;
			}

			// o ACTION : [ g_ALIGN ]
			else if (m_selected_utility.group == g_ALIGN 
					 && !m_gui_focused
					 && mouse_data.m0_clicked
					 )
			{
				if (m_selected_object_pack.object_index != m_focused_object_pack.object_index
					&& !create_area_selection)
				{
					setSelectedObject(m_focused_object_pack, !m_cloning_active);
					m_reselecting = true;
					handle_m0_RELEASED();
				}
				m_selected_utility.active = true;
			}
			else if (m_selected_utility.group == g_RESTRICTIONS && mouse_data.m0_clicked)
			{
				bool is_valid = true;
				// facevector -> object //
				if (m_selected_utility.id == u_ATTACH_RESTRICTION_FOCUS)
				{
					//is_valid = (obj_type == FACE_VECTOR);
				}
				// facevector -> facevector //
				else if (m_selected_utility.id == u_ATTACH_RESTRICTION_CONVERSATION)
				{
					//is_valid = (obj_type == FACE_VECTOR);
				}
				// object -> object //
				else if (m_selected_utility.id == u_ATTACH_RESTRICTION_DISTANCE)
				{
					is_valid = (obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT);
				}

				
				if (m_selected_object_pack.object_index != m_focused_object_pack.object_index
					&& !create_area_selection)
				{
					setSelectedObject(m_focused_object_pack, !m_cloning_active);
					m_reselecting = true;
					handle_m0_RELEASED();
				}

				if( is_valid )
					m_selected_utility.active = true;
			}

			// o ACTION : [ single selection ]
			else if (m_selected_object_pack.object_index != m_focused_object_pack.object_index
					 && !m_vp_frame_buttons_focused
					 && mouse_data.m0_clicked
					 && !create_area_selection)
			{
				setSelectedObject(m_focused_object_pack, !m_cloning_active);

				m_reselecting             = true;
				m_selected_utility.active = false;
			}


		}

		// o ACTION : [ add object to -> multiple selection ( Ctrl - is - pressed ) ]
		else if (    (key_data[GLFW_KEY_LEFT_CONTROL].pressed || key_data[GLFW_KEY_RIGHT_CONTROL].pressed)
				  && !m_reselecting
				  && !mouse_data.prev_m0_down
				  && obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT
				  && is_selected_valid
				  && mouse_data.m0_clicked
				  && !create_area_selection
				 )
		{

			if ( m_selected_object_pack.object_index != m_focused_object_pack.object_index )
			{
				m_multiple_selected_objects.clear();
				if (m_multiple_selection_active)
				{
					bool is_already_in_active_selection = false;
					if (m_focused_object_pack.raw_object->getType() != GROUP_PARENT_DUMMY)
					{
						is_already_in_active_selection = Utilities::is_object_in_List(m_focused_object_pack.raw_object,
														  m_area_selections_data[m_multiple_selection_active_id].selection_objects);
						if (is_already_in_active_selection)
							return;
					}
					else
					{
						bool in = true;
						for (SceneObject* group_child : m_focused_object_pack.raw_object->getGroupSelectionChilds())
							in &= Utilities::is_object_in_List(group_child, m_area_selections_data[m_multiple_selection_active_id].selection_objects);
						if (in)
							return;
					}

					for (SceneObject* obj : m_area_selections_data[m_multiple_selection_active_id].selection_objects)
						m_multiple_selected_objects.push_back(obj);

					m_multiple_selected_objects.push_back( m_focused_object_pack.raw_object );

				}
				else
				{
					m_multiple_selected_objects.push_back(m_selected_object_pack.raw_object);
					m_multiple_selected_objects.push_back(m_focused_object_pack.raw_object);
				}

				process_pre_AreaSelection(m_multiple_selected_objects);
				handle_MULTIPLE_SELECTION();
				m_reselecting = true;
			}
		}
		// o ACTION : [ subtract object from -> multiple selection ( alt - is - pressed ) ]
		else if ( (key_data[GLFW_KEY_LEFT_ALT].pressed || key_data[GLFW_KEY_RIGHT_ALT].pressed)
				 && !m_reselecting
				 &&  m_selected_object_pack.raw_object != nullptr
				 && !mouse_data.prev_m0_down
				 && obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT
				 && is_selected_valid
				 && mouse_data.m0_clicked
				 && !create_area_selection
				 )
		{
			if (m_wait_to_release)
				if (!mouse_data.m0_released)
					return;

			m_multiple_selected_objects.clear();
			{
				if (m_multiple_selection_active)
				{
					if (!Utilities::is_object_in_List(m_focused_object_pack.raw_object, m_area_selections_data[m_multiple_selection_active_id].selection_objects))
						return;

					for (SceneObject* obj : m_area_selections_data[m_multiple_selection_active_id].selection_objects)
					{
						if (obj->getId() != m_focused_object_pack.raw_object->getId())
						{
							m_multiple_selected_objects.push_back(obj);
						}
					}


					bool same = true;
					for (SceneObject* obj1 : m_area_selections_data[m_multiple_selection_active_id].selection_objects)
						for (SceneObject* obj2 : m_multiple_selected_objects)
							if (obj1->getId() != obj2->getId())
							{
								same = false;
								break;
							}

					if (same)
					{
						m_multiple_selected_objects.clear();
						return;
					}

				}

				if      (!m_multiple_selection_active)
				{
					if ( m_selected_object_pack.raw_object->getId() == m_focused_object_pack.raw_object->getId() )
					{
						resetSelectedObject(true);
					}
				}
				else if ( m_multiple_selected_objects.size() == 1 )
				{
					setSelectedObject(m_multiple_selected_objects[0], true);
				}
				else
				{
					process_pre_AreaSelection( m_multiple_selected_objects );
					handle_MULTIPLE_SELECTION();
				}

				m_reselecting = true;
				m_wait_to_release = true;

			}
		}

	}


#endif




	// o ACTION : [ activate object_manipulation in case utility interaction is true ]
	// o ACTION : [ clone selected object if shift is pressed while interacting ]
	if ( m_utility_interaction &&  !m_vp_frame_buttons_focused
		//&& Mediator::RequestCameraInstance().getState() == Camera_State::CAM_NONE
		)
	{

		// o ACTION : [ clone selected object if shift is pressed while interacting ]
		// fill nescessairy data for coming object manipulation
		if ( !m_selected_utility.active )
		{
			bool is_valid_obj = m_selected_object_pack.raw_object->getType() != FACE_VECTOR && m_selected_object_pack.raw_object->getType() != SAMPLER_OBJECT;
			// o ACTION : [ clone selected object if shift is pressed while interacting ]
			if ( (key_data[GLFW_KEY_LEFT_SHIFT].pressed || key_data[GLFW_KEY_RIGHT_SHIFT].pressed)
				&& m_selected_utility.group == g_TRANSFORMATIONS
				&& is_valid_obj
				&& Mediator::RequestCameraInstance().getState() == Camera_State::CAM_NONE
				)
			{
				handle_preCLONE();
			}

			fill_pre_object_manipulation_Data();
		}

		m_selected_utility.active = true;

	}

	// o ACTION : [ activate object_manipulation if selected obj is focused and selected_utility_plane != NONE ]
	if (m_focused_object_pack.object_index == m_selected_object_pack.object_index
		&& m_focused_object_pack.object_index >= 0 // [FIX] : select out mouse_press BUG here seems fixed
		&& m_interface_state.selected_utility_plane != UtilityPlane::NONE
		&& !m_selected_utility.active
		&& !m_cloning_finished
		&& !m_reselecting
		&& !mouse_data.m0_dragging
		&& !m_gui_focused
		&& !m_utility_isec_p.is_lightButton_hit
		&& Mediator::RequestCameraInstance().getState() == Camera_State::CAM_NONE
		)
	{
		calculate_new_utility_isec_point_relative_on_selected_utility_plane();
		fill_pre_object_manipulation_Data();
		m_selected_utility.active = true;

	}


	// o Action : [ Align_Camera -> init vars ]
	if (mouse_data.m0_clicked)
	{
		bool mouse_over_window_bar = (mouse_data.y >= -10 && mouse_data.y <= 21);
		m_gui_focused |= mouse_over_window_bar;

		m_mouse_movement_while_m0_down   = 0;
		m_m0_down_on_gui                 = m_gui_focused ? true : false;
		m_m0_down_on_utility_interaction = m_selected_utility.active ? true : false;
		m_m0_down_on_focused_object      = m_focused_object_pack.raw_object != nullptr; //m_focused_object_pack.object_index == m_selected_object_pack.object_index ? true : false;
        
		if (m_selected_utility.id == u_ALIGN_CAMERA)
		{
			SceneObject* object = m_focused_object_pack.raw_object;
			bool is_valid = is_Object_Valid_For_Camera_Align(object);
			if (object != 0)
			{
				m_camera_align_object.object = object;

				Type obj_type = object->getType();

				// set Align object -> Normal Vector
				if (obj_type != FACE_VECTOR)
				{
					optix::float3 * mouseHit_buffer = Mediator::request_MouseHit_Buffer_Data();
					optix::float3 hit_origin = mouseHit_buffer[0];
					optix::float3 hit_dir    = mouseHit_buffer[1];
					delete mouseHit_buffer;

					
					m_camera_align_object.origin = hit_origin;
					m_camera_align_object.dir = hit_dir;
					m_camera_align_object.obj_pos = object->getTranslationAttributes();
					m_camera_align_object.is_object = 1;

					// rasterize p1 Position //
					if(false)
					{
						float obj_dist     = Mediator::RequestCameraInstance().getDistanceFrom(object->getTranslationAttributes());
						float scale_factor = LINKING_SCALE_FACTOR * obj_dist * ViewportManager::GetInstance().getActiveViewport().scale_factor;
						optix::float3 p_world = hit_origin;//hit_origin + normalize(hit_dir) * scale_factor * 3.5f;

						float x = -1;
						float y = -1;
						Mediator::Transform_World_To_Screen(p_world, (int)ViewportManager::GetInstance().getActiveViewport().id, x, y);
						//std::cout << "\n - screen : [ " << x << " , " << y << " ] " << std::endl;
						//std::cout << " - mouse : [ " << mouse_data.x << " , " << mouse_data.y << " ] " << std::endl;

						mouse_data.m0_down_x = x;
						mouse_data.m0_down_y = y;
					}
					
				}
				else
				{
					m_camera_align_object.is_object = 0;
				}

				m_camera_align_picked_Object = true;
			}
		}

	}


	m_mouse_movement_while_m0_down += fabsf(mouse_data.dx + mouse_data.dy);
	////std::cout << " - Handle_m0_DOWN_END():" << std::endl;
}
void InterfaceManager::handle_m0_RELEASED()
{


	//if (m_gui_focused || m_popup_open)
	//	return;
	
	glfwSetInputMode(Mediator::RequestWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	if ((m_m0_down_on_camera_control_handle && m_mouse_movement_while_m0_down != 0)
		//|| m_dragFloat_state
		)
	{
		////std::cout << " asd a" << std::endl;
		glfwSetCursorPos(Mediator::RequestWindow(), mouse_data.m0_down_x, mouse_data.m0_down_y);
	}
	m_dragFloat_state = false;


	if (m_wait_to_release)
		m_wait_to_release = false;

	if (m_retargeting_camera)
	{
		m_retargeting_camera = false;
		return;
	}

	if (m_reselecting)
	{
		m_reselecting = false;
		return;
	}

	if (m_transforming_active)
		m_transforming_active = false;

	// o ACTION : [ release viewport camera ]
	if (m_viewport_camera_active)
		m_viewport_camera_active = false;

	if (!m_selected_utility.active)
	{
		if (m_m0_down_on_LightButton == m_utility_isec_p.light_obj_id && m_m0_down_on_LightButton != -1)
		{
			SceneObject* light_obj = Mediator::RequestSceneObjects()[Mediator::requestObjectsArrayIndex(m_m0_down_on_LightButton)];
			Light_Parameters params = light_obj->get_LightParameters();
			params.is_on = !params.is_on;
			light_obj->set_LightParameters(params);
			Mediator::restartAccumulation();
		}
	}

#ifdef OBJECT_SELECT_ON_MOUSE_RELEASE

	// o ACTION : [ single selection ]
	// o ACTION : [ multiple selection ( Ctrl - is - pressed ) ]
	if (m_focused_object_pack.raw_object != nullptr && !mouse_data.m0_dragging_down && !m_utility_interaction)
	{

		// o ACTION : [ single selection ]
		if (ui_key_grp_state != UI_KEY_GRP_EDIT)
		{
			setSelectedObject(m_focused_object_pack, !m_cloning_active);
		}

		// o ACTION : [ multiple selection ( Ctrl - is - pressed ) ]
		else
		{

			// select focused object and add it to multiple_selection list
			// if there is already a single selection active, add it to multiple_selection list
			if (m_selected_object_pack.raw_object != nullptr && m_selected_object_pack.object_index >= 0)
			{
				m_multiple_object_selection.push_back(m_selected_object_pack);
				m_multiple_object_selection[m_multiple_object_selection.size() - 1].selected = true;
				//m_selected_object_pack.reset();
			}

			// push back to multiple_selection list focused object
			m_multiple_object_selection.push_back(m_focused_object_pack);
			m_multiple_object_selection[m_multiple_object_selection.size() - 1].selected = true;
			mouse_data.m0_dragging_down = 0;

		}

	}

#endif

	

	// o ACTION : [ finish with Area Selection if it was already active ]
	{

		if (m_selected_utility.id == u_SELECTION_AREA && m_selected_utility.active)
		{
			handle_AREA_SELECTION();
			return;
		}

	}
	
	// o ACTION : [ g_ATTACH ]
	// handle_Attach_FaceVector()
	if ( m_selected_utility.group == g_ATTACH )
	{
		
		// o ACTION : [ Attach Face Vector ]
		bool is_ff_vector  = m_selected_utility.id == u_ATTACH_FACE_VECTOR_BBOX_ALIGNED || m_selected_utility.id == u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED || m_selected_utility.id == u_ATTACH_FACE_VECTOR_BBOX_ALIGNED_PARALLEL;
		bool is_obj_valid  = m_selected_object_pack.raw_object != nullptr;
		bool is_type_valid = false;
		bool is_attach_sampler = m_selected_utility.id == u_ATTACH_SAMPLER_DIRECTIONAL || m_selected_utility.id == u_ATTACH_SAMPLER_PLANAR || m_selected_utility.id == u_ATTACH_SAMPLER_VOLUME;
		
		bool is_area_selection = is_obj_valid ? 
			(m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY && m_selected_object_pack.raw_object->isTemporary())
			: false;

		if ( is_obj_valid )
		{
			Type obj_type = m_selected_object_pack.raw_object->getType();
			bool is_temp  = m_selected_object_pack.raw_object->isTemporary();
			is_type_valid = (obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT && !is_temp);
		}

		if      ( is_ff_vector      && is_type_valid )
		{
			// 2 cases  Here :
			// First , if : Bbox_Aligned -> need to check if mouse_release happens between the object's bbox faces
			// Second, if : Normal_Aligned -> need to check only if hovering over the selected objects when mouse_release
			bool is_ff_vector_creation_valid;
			bool m0_down_was_down;
			bool is_group_n_Normal = (m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY && m_selected_utility.id == u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED);
			if (m_selected_utility.id == u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED && !is_group_n_Normal)
			{
				is_ff_vector_creation_valid = ((m_focused_object_pack.object_index == m_selected_object_pack.object_index) && (m_selected_object_pack.object_index != -1));
				m0_down_was_down = m_m0_down_on_focused_object;
			}
			else
			{
				is_ff_vector_creation_valid = m_utility_isec_p.hit_index >= 0;
				m0_down_was_down            = m_m0_down_on_utility_interaction;
			}
				
			if ( m0_down_was_down && is_ff_vector_creation_valid )
			{
				// create Face Vector //
				optix::float3 pos = m_interface_state.isec_p->p1;
				optix::float3 dir = normalize( m_interface_state.isec_p->p - m_interface_state.isec_p->p1 );
				int hit_index = (u_ATTACH_FACE_VECTOR_NORMAL_ALIGNED && !is_group_n_Normal) ? -1 : m_utility_isec_p.hit_index;
				m_selected_object_pack.raw_object->attach_FaceVector( 
					m_selected_object_pack.raw_object->getCentroid_Transformed(), //pos,
					dir,
					hit_index );
			}
			
		}
		else if ( is_attach_sampler && is_type_valid )
		{
			bool is_attach_sampler_valid = m_utility_isec_p.hit_index >= 0;
			bool m0_was_down             = m_m0_down_on_utility_interaction;
			bool use_bbox                = m_sampler_init_isec_object_bbox;

			SceneObject* focused_obj  = m_focused_object_pack.raw_object;
			SceneObject* selected_obj = m_selected_object_pack.raw_object;
			int focused_id  = focused_obj  != nullptr ? focused_obj->getId()  : -1;
			int selected_id = selected_obj != nullptr ? selected_obj->getId() : -1;
			bool is_valid_f = false;
			if (
				(selected_id != -1 && focused_id == selected_id)
				|| (use_bbox && is_attach_sampler_valid)
				) is_valid_f = true;

			if (is_valid_f && m0_was_down)
			{
				if      (m_selected_utility.id == u_ATTACH_SAMPLER_DIRECTIONAL) handle_Attach_Sampler_Directional();
				else if (m_selected_utility.id == u_ATTACH_SAMPLER_PLANAR)      handle_Attach_Sampler_Planar();
				else if (m_selected_utility.id == u_ATTACH_SAMPLER_VOLUME)      handle_Attach_Sampler_Volume();
			}


		}
		m_selected_utility.active = false;
	}
	if ( m_selected_utility.group == g_CREATE )
	{
		if (m_selected_utility.id == u_CREATE_LIGHT)
		{
			bool is_valid = m_interface_state.m_sa == SA_GLOBAL ? 1 : m_m0_down_on_focused_object;
			////std::cout << "\n - m_m0_down_on_focused_object : " << m_m0_down_on_focused_object << std::endl;
			////std::cout << " - m_mouse_movement : " << m_mouse_movement_while_m0_down << std::endl;
			if (is_valid && m_mouse_movement_while_m0_down == 0 
				&& !m_gui_focused
				&& !m_focusing_menu_bar_items)
			{
				////std::cout << " - creating Light!" << std::endl;
				handle_Create_Light();
			}
			
			m_selected_utility.active = false;
		}
	}
	if ( m_selected_utility.group == g_ALIGN  )
	{

	}

	// o ACTION : [ set selected utility ]
	{
		if (m_utility_interaction)
		{
			m_interface_state.selected_utility_plane = m_utility_isec_p.plane_id;
		}
	}
	

	// o ACTION : [ push new Action to ActionManager ]
	if (m_selected_utility.active)
	{
	
		if ( m_selected_utility.group == g_TRANSFORMATIONS && !m_cloning_finished )
		{
			// if m_cloning == active -> proceed to user-dialog for finalization
			bool object_cloning_approved = false;
			if (m_cloning_active)
			{
				object_cloning_approved = is_finally_object_cloning_aproved(m_object_transformations_manip_data);
				bool finalize = false;
				if (object_cloning_approved)
				{
					
					// fill cloned object transformation //
					{
						switch (m_selected_utility.id)
						{

						case u_TRANSLATION:
						{

						}break;

						case u_ROTATION:
						{
							// xyz stacked rotation
							if (m_object_transformations_manip_data.plane_id == UtilityPlane::XYZ)
							{
								m_cloned_object_pack.trs_pack.clear();
								m_cloned_object_pack.trs_pack.object_id = m_cloned_object_pack.raw_object->getId();
								for (optix::float3 d : m_object_transformations_manip_data.stack_rotations) m_cloned_object_pack.trs_pack.m_stacked_rotations.push_back(d);
								m_cloned_object_pack.trs_pack.axis_orientation = m_selected_axis_orientation;
								m_cloned_object_pack.trs_pack.t_setting        = m_hierarchy_setting;
								m_cloned_object_pack.trs_pack.t_type           = ROTATION;
							}
							else
							{

							}
						}break;

						case u_SCALE:
						{

						}break;

						}
					}
					finalize = pop_Object_cloning_Approval_window();
					if (finalize)
					{
						m_popup_object_cloning_approval = true;
						m_skip_user_input = true;
						return;
					}
				}
				else
				{
					handle_CLONE(false, false, m_cloned_object_pack, m_prev_selected_object_pack, "");
					Update_User_Input();
				}
			}

			if (!m_cloning_active)
			{
				optix::float3 start, end;
				optix::float3 delta;
				SceneObject* obj = m_selected_object_pack.raw_object;
				Action * action;

				bool is_only_this = m_hierarchy_setting   == ONLY_THIS;
				bool is_only_childs = m_hierarchy_setting == ONLY_TO_CHILDS;
				bool is_group = m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY;
				bool has_childs = m_selected_object_pack.raw_object->getChilds().size() > 0;
				bool is_valid_transformation = !( (is_group && is_only_this) || (is_only_childs && !has_childs) );
				
				optix::float3 overall_delta = m_object_transformations_manip_data.overall_delta;
				float l_delta = length(overall_delta);
				bool has_delta = l_delta > 1e-5;

				switch (m_selected_utility.id)
				{

				case u_TRANSLATION:
				{

					if (has_delta)
					{
						action = new Action_TRANSFORMATION(obj, overall_delta, m_selected_object_pack.base_axis, m_selected_axis_orientation, m_hierarchy_setting, ACTION_ID::ACTION_TRANSFORMATION_TRANSLATION);
						ActionManager::GetInstance().push(action);
						
					}
				}
				break;

				case u_ROTATION:
				{

					
					if (m_object_transformations_manip_data.plane_id == UtilityPlane::XYZ)
					{
						if (has_delta && is_valid_transformation)
						{
							action = new Action_TRANSFORMATION_XYZ_ROTATION(obj, &m_object_transformations_manip_data, m_hierarchy_setting);
							ActionManager::GetInstance().push( action );
						}
					}
					else
					{
						if (has_delta && is_valid_transformation)
						{
							action = new Action_TRANSFORMATION( obj, overall_delta, m_selected_object_pack.base_axis, m_selected_axis_orientation, m_hierarchy_setting, ACTION_ID::ACTION_TRANSFORMATION_ROTATION);
							ActionManager::GetInstance().push(action);
						}
					}
				}
				break;

				case u_SCALE:
				{
					
					//obj->Update_Group_Topology(false, true);
					if (has_delta && is_valid_transformation)
					{
					   	
						AXIS_ORIENTATION axis_orientation = (m_selected_object_pack.raw_object->getType() == SAMPLER_OBJECT || m_selected_object_pack.raw_object->getType() == LIGHT_OBJECT ) ? LOCAL : m_selected_axis_orientation;
						action = new Action_TRANSFORMATION(obj, overall_delta, m_selected_object_pack.base_axis, axis_orientation, m_hierarchy_setting, ACTION_ID::ACTION_TRANSFORMATION_SCALE);
						ActionManager::GetInstance().push(action);
					}

				}
				break;

				}

			}
		}
		
	}
	

	// o ACTION : [ reset object_manipulation_data ]
	// o ACTION : [ link subject to object if linking is active ]
	if (m_selected_utility.active)
	{

		bool is_obj_valid  = m_selected_object_pack.raw_object != nullptr;
		bool is_type_valid = true;
		bool is_area_selection = is_obj_valid ? 
			(m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY && m_selected_object_pack.raw_object->isTemporary())
			: false;
		if (is_obj_valid)
		{
			Type obj_type = m_selected_object_pack.raw_object->getType();
			is_type_valid &= (obj_type != FACE_VECTOR && obj_type != SAMPLER_OBJECT);
		}

		if (m_selected_utility.group == g_TRANSFORMATIONS)
		{
			m_selected_object_pack.raw_object->setAttributesDelta(0, optix::make_float3(0));
			m_selected_object_pack.raw_object->setAttributesDelta(1, optix::make_float3(0));
			m_selected_object_pack.raw_object->setAttributesDelta(2, optix::make_float3(0));
		}

		// o ACTION : [ link subject to object if linking is active ]
		else if ( m_selected_utility.id == u_LINK && is_type_valid )
		{
			if (m_focused_object_pack.raw_object != nullptr
				&& m_selected_object_pack.raw_object != nullptr
				&& m_focused_object_pack.object_index != m_selected_object_pack.object_index)
			{
				if(m_focused_object_pack.raw_object->getType() != FACE_VECTOR && m_focused_object_pack.raw_object->getType() != SAMPLER_OBJECT)
					handle_LINK();
			}
		}
		else if (m_selected_utility.id == u_ALIGN && is_obj_valid)
		{
			is_type_valid = m_selected_object_pack.raw_object->getType() != FACE_VECTOR;
			if (is_type_valid)
			{
				if (m_focused_object_pack.raw_object != nullptr
					&& m_selected_object_pack.raw_object != nullptr
					&& m_focused_object_pack.object_index != m_selected_object_pack.object_index)
				{
					if (m_focused_object_pack.raw_object->getType() != FACE_VECTOR
						//&& m_focused_object_pack.raw_object->getType() != SAMPLER_OBJECT
						)
						handle_Object_Align();
				}
			}

		}
		else if (m_selected_utility.id == u_ALIGN_CAMERA)
		{

			// case : focusing same object

			if (   m_focused_object_pack.raw_object   != nullptr
				&& m_selected_object_pack.raw_object  != nullptr
				&& m_focused_object_pack.object_index != m_selected_object_pack.object_index)
			{

				m_camera_align_target.object = m_focused_object_pack.raw_object;

				Type obj_type = m_focused_object_pack.raw_object->getType();
				if (obj_type != FACE_VECTOR)
				{
					m_camera_align_target.is_object = true;
					optix::float3 * data = Mediator::request_MouseHit_Buffer_Data();
					m_camera_align_target.origin = data[0];
					m_camera_align_target.dir    = data[1];
					delete data;
				}
				else
				{
					m_camera_align_target.is_object = false;
				}

				bool is_same_object = false;
				int objP_id = -1;
				int trgP_id = -1;
				objP_id = m_camera_align_object.is_object ? m_camera_align_object.object->getId() : m_camera_align_object.object->getParent()->getId();
				trgP_id = m_camera_align_target.is_object ? m_camera_align_target.object->getId() : m_camera_align_target.object->getParent()->getId();
				is_same_object = (objP_id == trgP_id);

				if(!is_same_object)
					handle_Camera_Align();
			}

		}
		else if (m_selected_utility.group == g_RESTRICTIONS && !is_area_selection)
		{
			bool fObj_isValid           = m_focused_object_pack.raw_object != 0;
			bool fObj_is_area_selection = fObj_isValid ? 
				(m_focused_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY && m_focused_object_pack.raw_object->isTemporary())
				: false;
			if (   m_focused_object_pack.raw_object   != nullptr
				&& m_selected_object_pack.raw_object  != nullptr
				&& m_focused_object_pack.object_index != m_selected_object_pack.object_index
				&& !fObj_is_area_selection
				)
			{
				
				if (m_selected_utility.id == u_ATTACH_RESTRICTION_FOCUS)
					m_UI_selected_Restriction = handle_Attach_Restriction_Focus();
				else if (m_selected_utility.id == u_ATTACH_RESTRICTION_CONVERSATION)
					m_UI_selected_Restriction = handle_Attach_Restriction_Conversation();
				else if (m_selected_utility.id == u_ATTACH_RESTRICTION_DISTANCE)
					m_UI_selected_Restriction = handle_Attach_Restriction_Distance();

				reset_Force_Tabs_Open_Flags();

				if ( m_UI_selected_Restriction != nullptr )
				{
					//reset_Force_Tabs_Open_Flags();
					m_force_tab_open_active = true;
					m_force_tab_open[RESTRICTIONS_TAB] = true;
					Mediator::Reset_Restrictions_UI_States();
					m_UI_selected_Restriction->set_Selected(true);
				}
			}
		}

		m_selected_utility.active = false; // (?) den isxuei gia ola ta utilities (?)
		m_interface_state.inner_radius_hit = false;
		m_utility_isec_p.reset();
		m_object_transformations_manip_data.reset();

	}
	

	
	// o ACTION : [ reset m_cloning_active to = false ]
	if (m_cloning_active)
		m_cloning_active = false;

	if (m_cloning_finished)
		m_cloning_finished = false;

	if (m_skip_user_input_finished)
		m_skip_user_input_finished = false;

	if (m_finished_area_selection)
		m_finished_area_selection = false;


	if( m_transformation_input_m0_down )
		m_transformation_input_m0_release = true;
	m_transformation_input_m0_down = false;

	m_camera_align_picked_Object = false;
	m_camera_align_picked_Target = false;

	m_m0_down_on_camera_control_handle = false;
	m_m0_down_on_utility_interaction = false;
	m_m0_down_on_focused_object = false;
	m_m0_down_on_gui = false;
	m_m0_down_on_LightButton = -1;
	

	////std::cout << " - Handle_m0_RELEASED_END():" << std::endl;
}
void InterfaceManager::handle_m1_DOWN()
{
	//if (m_gui_focused || m_popup_open)
	//	return;
}
void InterfaceManager::handle_m1_RELEASED()
{
	//if (m_gui_focused || m_popup_open)
	//	return;



	// o ACTION : [ reset Action ]
	if (   !m_selected_utility.active 
		&& m_selected_utility.id    != u_NONE
		&& m_selected_utility.group != g_TRANSFORMATIONS
		//&& m_selected_utility.group != g_CREATE
		)
	{
		
		bool is_create = m_selected_utility.group == g_CREATE;
		bool is_attach = m_selected_utility.group == g_ATTACH;

		if (m_prev_selected_utility.group == g_TRANSFORMATIONS)
			toggle(m_prev_selected_utility.id, false, true);
		else
		{
			tool_buttons[ Utilities::convert_Utility_To_Utility_id(m_selected_utility.id) ]->setSelected(false);
			m_selected_utility.reset();
		}
		set_FaceVector_States(m_selected_utility);
		if(m_selected_utility.id != u_NONE || is_create || is_attach)
			return;
	}
	else if (  m_selected_utility.active
			 && m_selected_utility.id != u_NONE
			 && m_selected_utility.group != g_TRANSFORMATIONS
			 )
	{
		if      (m_selected_utility.group == g_LINKING || m_selected_utility.group == g_ALIGN)
		{
			m_selected_utility.active = false;
			return;
		}
		else if (m_selected_utility.group == g_ATTACH)
		{

			if (m_prev_selected_utility.group == g_TRANSFORMATIONS)
				toggle(m_prev_selected_utility.id, false, true);
			else
			{
				tool_buttons[Utilities::convert_Utility_To_Utility_id(m_selected_utility.id)]->setSelected(false);
				m_selected_utility.reset();
			}
			set_FaceVector_States(m_selected_utility);
			if (m_selected_utility.id != u_NONE)
				return;
			
			return;
		}
		else if (m_selected_utility.group == g_RESTRICTIONS)
		{
			m_selected_utility.active = false;
			return;
		}
		else if (m_selected_utility.group == g_CREATE)
		{
			m_selected_utility.active = false;
			return;
		}
	}


	// o ACTION : [ clear single selection ]
	// o ACTION : [ clear multiple_selection list ]
	if (   !mouse_data.m1_dragging_down && m_focused_object_pack.object_index != m_selected_object_pack.object_index && !m_utility_interaction
		&& !m_gui_focused )
	{


		//m_selected_object_pack.reset();
		//
		//mouse_data.m1_dragging_down = 0;
		resetSelectedObject();
		m_multiple_object_selection.clear();



		// o ACTION : [ reset m_selected_utility_plane if object is deselected ]
		m_interface_state.selected_utility_plane = UtilityPlane::NONE;

	}

	// o ACTION : [ clear multiple selected objects ]
	m_multiple_selected_objects.clear();

	// o ACTION : [ show object's properties window ]
	{

	}

	//
	//

	//m_m0_down_on_camera_control_handle = false;

}

void InterfaceManager::handle_Utility_m0_CLICKED( UtilityProperties ut_prop )
{

}
void InterfaceManager::handle_Utility_m0_DOWN( UtilityProperties ut_prop )
{

}
void InterfaceManager::handle_Utility_m0_RELEASED( UtilityProperties ut_prop )
{

}
void InterfaceManager::handle_Utility_m1_DOWN( UtilityProperties ut_prop )
{

}
void InterfaceManager::handle_Utility_m1_RELEASED( UtilityProperties ut_prop )
{

}

void InterfaceManager::handle_preCLONE()
{
	//std::cout << "\n\n --------- handle_preCLONE(): ----------- " << std::endl;

	m_cloned_faceVectors.clear();
	m_cloned_Samplers.clear();

	// set m_hierarchy_setting = TO_ALL to prevent errors
	m_cloning_finished  = false;
	m_hierarchy_setting = TO_ALL;

	// is group?
	bool is_selected_group = m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY && !m_selected_object_pack.raw_object->isTemporary();
	std::string source_name = m_selected_object_pack.raw_object->getName();


	bool is_single_selection = !m_multiple_selection_active && !is_selected_group;
	bool is_area_selection   = m_multiple_selection_active;
	bool is_group_selection  = is_selected_group;


#ifdef pre_clone_Utility_Childs
	// pre - process Utility childs 
	// traverse Hierarchy and locate every Group's utility object
	// create a clone of it ( and link it with already selected object ? )
	{
		
		Multiple_Selection_Data temp_data;
		if (is_single_selection)
		{
			temp_data.selection_objects.push_back(m_selected_object_pack.raw_object);
		}
		else if (is_area_selection)
		{
			for(SceneObject* obj : m_selected_object_pack.raw_object->getChilds())
				temp_data.selection_objects.push_back(obj);
		}
		else if (is_group_selection)
		{
			for (SceneObject* obj : m_selected_object_pack.raw_object->getGroupSelectionChilds())
				temp_data.selection_objects.push_back(obj);
			process_pre_AreaSelection(temp_data.selection_objects);
		}

		temp_data.top_level_groups.clear();
		temp_data.top_level_subgroups.clear();

		// phase 2
		{

			Multiple_Selection_Data data;
			if (is_area_selection) data = m_area_selections_data[m_multiple_selection_active_id];

			for (SceneObject* object : temp_data.selection_objects)
			{
				std::stack<SceneObject*> upper;

				SceneObject* parent = is_area_selection ? data.parents_bucket[object->getId()] : object->getParent();
				if (parent != 0)
					upper.push(parent);

				while (upper.size() > 0)
				{
					SceneObject* current = upper.top(); upper.pop();
					bool is_group     = current->getType() == GROUP_PARENT_DUMMY;
					bool in_Selection = Utilities::is_object_in_List(current, temp_data.selection_objects);

					// break;
					if      (!is_group && !in_Selection)
					{
						break;
					}
					else if (is_group)
					{
						if (!Utilities::is_object_in_List(current, temp_data.top_level_groups))
						{
							temp_data.top_level_groups.push_back(current);
							SceneObject* parent = is_area_selection && in_Selection ? data.parents_bucket[current->getId()] : current->getParent();
								//is_area_selection ? data.parents_bucket[current->getId()] : current->getParent();
							if (parent != 0)
								upper.push( parent );
						}
					}
				}
			}
		}

		// print info //
		//std::cout << "\n > Groups:" << std::endl;
		//for (SceneObject* group : temp_data.top_level_groups)
		//	std::cout << "   - group : " << group->getName() << std::endl;

		// for every group : copy utility childs
		for (SceneObject* group : temp_data.top_level_groups)
		{
			for (SceneObject* fv : group->get_Face_Vectors())
			{
				if (!fv->isActive())
					continue;

				SceneObject* fv_clone = Utilities::get_clone_Object_By_Type(fv);
				fv_clone->setName(fv->getName(false));
				fv_clone->setNamePostfix(fv->getNamePostfix());
				m_cloned_faceVectors.push_back({fv_clone, fv->getParent()});
				
				//std::cout << "\n > creating cloned_Facevector : " << fv_clone->getId() << std::endl;
				//std::cout << "   - parent : " << fv->getParent()->getName() << std::endl;
				//std::cout << "   - parent_id: " << fv->getParent()->getId() << std::endl;
			}
			for (SceneObject* smplr : group->get_Samplers())
			{
				if (!smplr->isActive())
					continue;

				SceneObject* smplr_clone = Utilities::get_clone_Object_By_Type(smplr);
				smplr_clone->setName(smplr->getName(false));
				smplr_clone->setNamePostfix(smplr->getNamePostfix());
				m_cloned_Samplers.push_back({smplr_clone, smplr->getParent()});
				
				//std::cout << "\n - creating cloned_Samplerr : " << smplr_clone->getId() << std::endl;
				//std::cout << " - parent : " << smplr->getParent()->getName() << std::endl;
				//std::cout << " - parent_id: " << smplr->getParent()->getId() << std::endl;
			}
		}
		
	}

#endif

	

	// single selection - no group
	if ( is_single_selection )
		handle_preCLONE_SINGLE_SELECTION();
	// area selection
	else if ( is_area_selection )
		handle_preCLONE_AREA_SELECTION();
	// group
	else if ( is_group_selection )
		handle_preCLONE_GROUP_SELECTION();

#ifdef pre_clone_Utility_Childs
	
	//std::cout << "\n > attaching cloned Facevectors : "<< std::endl;
	//std::cout << "   - attaching to : " << std::endl;
	//std::cout << "   - object : " << m_selected_object_pack.raw_object->getName() << std::endl;
	//std::cout << "   - id : " << m_selected_object_pack.raw_object->getId() << std::endl;

	for (Object_General_Package pack : m_cloned_faceVectors)
	{
		SceneObject* obj = pack.obj;
		obj->setVisible(false);
		obj->set_as_Temp_Clone(true);

		//std::cout << "\n - object : " << m_selected_object_pack.raw_object->getName() << std::endl;
		//std::cout << " - id : " << m_selected_object_pack.raw_object->getId() << std::endl;
		//std::cout << " - isTemp : " << m_selected_object_pack.raw_object->isTemporary() << std::endl;
		//std::cout << " - object->add_FaceVector( " << obj->getId() << " ) " << std::endl;
		m_selected_object_pack.raw_object->add_FaceVector(obj);
		//std::cout << "   - obj->add_FaceVector( " << obj->getId() << " ) " << std::endl;

	}
	for (Object_General_Package pack : m_cloned_Samplers)
	{
		SceneObject* obj = pack.obj;
		obj->setVisible(false);
		obj->set_as_Temp_Clone(true);

		//std::cout << "\n - object : " << m_selected_object_pack.raw_object->getName() << std::endl;
		//std::cout << " - id : " << m_selected_object_pack.raw_object->getId() << std::endl;
		//std::cout << " - isTemp : " << m_selected_object_pack.raw_object->isTemporary() << std::endl;
		//std::cout << " - object->add_Sampler( " << obj->getId() << " ) " << std::endl;
		m_selected_object_pack.raw_object->add_Sampler(obj);
	}
	
#endif

	if (m_cloning_active_type != 2)
	{
		std::string name = source_name + std::string("_copy");
		for (int i = 0; i < 101; i++)
			cloning_object_name[i] = '\0';
		for (int i = 0; i < name.size(); i++)
			cloning_object_name[i] = name[i];
		
	}

}
void InterfaceManager::handle_preCLONE_SINGLE_SELECTION()
{
	handle_preCLONE_GROUP_SELECTION_fromSingle();
	return;

	// create the cloned object from the selected one
	SceneObject* clone = Utilities::get_clone_Object( m_selected_object_pack.raw_object );
	m_cloned_object_pack.raw_object   = clone;
	m_cloned_object_pack.object_index = Mediator::requestObjectsArrayIndex(clone->getId());

	// save the previous selected object in case cloning get cancelled
	m_prev_selected_object_pack.raw_object   = m_selected_object_pack.raw_object;
	m_prev_selected_object_pack.object_index = m_selected_object_pack.object_index;

	Mediator::restartAccumulation();

	// set current selected object as the clone, and set cloning = active!
	setSelectedObject(clone);
	Update_current_Interface_State();
	Mediator::setSelectedObject(m_selected_object_pack.object_index);
	Mediator::setFocusedObject(m_selected_object_pack.object_index);
	m_cloning_active = true;
	m_cloning_active_type = 0;

}
void InterfaceManager::handle_preCLONE_AREA_SELECTION()
{
	SceneObject* group_clone = Utilities::get_clone_group(m_selected_object_pack.raw_object, m_area_selections_data[m_multiple_selection_active_id]);

	m_prev_selected_object_pack.raw_object   = m_selected_object_pack.raw_object;
	m_prev_selected_object_pack.object_index = m_selected_object_pack.object_index;
	resetSelectedObject(false, true);

	m_cloned_object_pack.raw_object   = group_clone;
	m_cloned_object_pack.object_index = Mediator::requestObjectsArrayIndex(group_clone->getId());

	setSelectedObject(group_clone);
	m_multiple_selection_active = true;

	Update_current_Interface_State();
	Mediator::setSelectedObject(m_selected_object_pack.object_index);
	Mediator::setFocusedObject(m_selected_object_pack.object_index);
	m_cloning_active = true;
	m_cloning_active_type = 2;
}
void InterfaceManager::handle_preCLONE_GROUP_SELECTION()
{
	

	m_multiple_selected_objects.clear();
	for ( SceneObject* obj : m_selected_object_pack.raw_object->getGroupSelectionChilds() )
		m_multiple_selected_objects.push_back( obj );
	

	//

	//

	for (SceneObject* obj : m_selected_object_pack.raw_object->getChilds())
		m_prev_selected_object_pack.childs.push_back(obj);
	for (SceneObject* obj : m_selected_object_pack.raw_object->getGroupSelectionChilds())
		m_prev_selected_object_pack.GroupSelectionChilds.push_back(obj);
	for (SceneObject* obj : m_selected_object_pack.raw_object->getGroupSelectionChilds_and_Groups())
		m_prev_selected_object_pack.GroupSelectionChilds_and_Groups.push_back(obj);

	SceneObject* group_clone = Utilities::get_clone_group( m_selected_object_pack.raw_object );

	m_prev_selected_object_pack.raw_object   = m_selected_object_pack.raw_object;
	m_prev_selected_object_pack.object_index = m_selected_object_pack.object_index;
	resetSelectedObject( false, true );

	m_cloned_object_pack.raw_object   = group_clone;
	m_cloned_object_pack.object_index = Mediator::requestObjectsArrayIndex( group_clone->getId() );

	setSelectedObject( group_clone );
	m_multiple_selection_active = true;

	Update_current_Interface_State();
	Mediator::setSelectedObject( m_selected_object_pack.object_index );
	Mediator::setFocusedObject(  m_selected_object_pack.object_index );
	m_cloning_group  = true;
	m_cloning_active = true;
	m_cloning_active_type = 1;

}
void InterfaceManager::handle_preCLONE_GROUP_SELECTION_fromSingle()
{


	m_multiple_selected_objects.clear();
	m_multiple_selected_objects.push_back(m_selected_object_pack.raw_object);
	//

	//
	SceneObject* group_clone = Utilities::get_clone_group( m_multiple_selected_objects );

	m_prev_selected_object_pack.raw_object = m_selected_object_pack.raw_object;
	m_prev_selected_object_pack.object_index = m_selected_object_pack.object_index;
	resetSelectedObject(false, true);

	m_cloned_object_pack.raw_object = group_clone;
	m_cloned_object_pack.object_index = Mediator::requestObjectsArrayIndex(group_clone->getId());

	setSelectedObject(group_clone);
	m_multiple_selection_active = true;

	Update_current_Interface_State();
	Mediator::setSelectedObject(m_selected_object_pack.object_index);
	Mediator::setFocusedObject(m_selected_object_pack.object_index);
	m_cloning_group  = true;
	m_cloning_active = true;
	m_cloning_active_type   = 1;
	m_cloning_asSingleGroup = 1;

}
void InterfaceManager::handle_CLONE(bool approve, int instance , Object_Package& cloned_object_pack, Object_Package& prev_object_pack, std::string name)
{
	if (prev_object_pack.raw_object->getType() == LIGHT_OBJECT)
		cloning_options_clone_fvs = true;

	// handle cloning cases : { Approve , Cancel }
	if ( approve )
		handle_CLONE_Approve(instance, cloned_object_pack, prev_object_pack, name);
	else
		handle_CLONE_Cancel(instance, cloned_object_pack, prev_object_pack, name);

	
	cloned_object_pack.reset();
	prev_object_pack.reset();
	as_instance = false;
	as_copy     = true;
	cloning_options_clone_fvs = true;
	cloning_options_clone_samplers = true;

	m_cloning_active_type = -1;
	m_skip_user_input               = 0;
	m_skip_user_input_finished      = true;
	m_popup_object_cloning_approval = false;
	m_cloning_finished              = true;
	m_cloning_group                 = false;
	m_cloning_asSingleGroup         = false;
	m_selected_utility.active       = false;

	m_object_transformations_manip_data.reset();

	

	handle_m0_RELEASED(); // gia na ksekolaei to  mouse m0 meta to cloning!
	//Mediator::restartAccumulation();
	
}
void InterfaceManager::handle_CLONE_Approve(int cloning_id, Object_Package& cloned_object_pack, Object_Package& prev_object_pack, std::string name)
{
	
	//
	//std::cout << "\n - handle_CLONE_Approve( ... ) " << std::endl;
	//std::cout << "   - cloning facevectors : " << cloning_options_clone_fvs << std::endl;
	//std::cout << "   - cloning samplers : " << cloning_options_clone_samplers << std::endl;
	//
	bool is_area_selection_active = m_multiple_selection_active ? true : false;
	

	// case : Area Selection - Group Cloning
	if (is_area_selection_active)
	{
		//std::cout << "   - area Selection Cloning!" << std::endl;
		//std::cout << "   - cloning_group : " << m_cloning_group << std::endl;
		SceneObject* group_parent_dummy = m_selected_object_pack.raw_object;                                             // current selected object ( area selection parent ) ( pre - cloned object )
		std::vector<SceneObject*> childs;                                                                                // current cloned objects
		for (SceneObject* child : group_parent_dummy->getChilds()) childs.push_back( child );                            // remove cloned objects from area selection group parent
		for (SceneObject* child : childs){ child->getParent()->removeChild(child->getId()); child->removeParent(); }     // 
		std::vector<SceneObject*> groups_created;

		// as_instance = TRUE //
		if ( as_instance || (m_cloning_group && !m_cloning_asSingleGroup))
		{
			
			Multiple_Selection_Data data_for_linking;
			if ( m_cloning_group ) // * if cloning a group object -> process selection , in any area selection case , this process has already been done
			{
				
				data_for_linking.selection_objects = m_multiple_selected_objects;
				process_pre_AreaSelection( data_for_linking.selection_objects );
				process_AreaSelection( data_for_linking );

				//for (SceneObject* grp : data_for_linking.top_level_groups)
				//	std::cout << "    - group : " << grp->getName() << std::endl;
				//for (SceneObject* grp : data_for_linking.top_level_subgroups)
				//	std::cout << "    - subgroup : " << grp->getName() << std::endl;
				if (!as_instance)
				{

					data_for_linking.top_level_groups.clear();
					data_for_linking.top_level_subgroups.clear();
					data_for_linking.top_level_groups.push_back(m_prev_selected_object_pack.raw_object);
				}
			}
			else
				data_for_linking = m_area_selections_data[m_area_selections_data.size() - 1];

			//std::cout << " > LINKING_CLONES_BASED_ON_SOURCES : " << std::endl;
			Utilities::link_clones_based_on_sources2 // Link prev - created Cloned Objects based on Source Objects // ( ? )
			(&m_cloned_object_pack,
			 childs,                  // clones ( prev - created cloned objects )
			 data_for_linking,        // source objects
			 groups_created);         // additional groups created
		}
		

#ifdef pre_clone_Utility_Childs

		//std::cout << "\n";
		//group_parent_dummy->keepUtilityChilds(true);
		//std::cout << "   - groups_created.size() : " << groups_created.size() << std::endl;
		//std::cout << "   - as_instance : " << as_instance << std::endl;
		//std::cout << "   - as_copy : " << as_copy << std::endl;
		//std::cout << "   - asSingleGroup : " << m_cloning_asSingleGroup << std::endl;
		bool let_ut_childs_ = ((as_instance || m_cloning_group) && groups_created.size() > 0 );//groups_created.size() > 0;
		//std::cout << "   - unlink ut childs : " << let_ut_childs_ << std::endl;
		if (let_ut_childs_)
		{

			std::vector<SceneObject*> fvs = group_parent_dummy->get_Face_Vectors();
			if (cloning_options_clone_fvs)
			{
				//std::cout << " > removing cloned_fvs from temp_group!" << std::endl;
				for (SceneObject* fv_cloned : fvs)
				{
					if (fv_cloned->isTempClone())
					{
						//std::cout << "   - group_parent_dummy[ " << group_parent_dummy->getId() << " ]->remove_FaceVector( " << fv_cloned->getId() << " ) " << std::endl;
						group_parent_dummy->remove_FaceVector(fv_cloned->getId(), true);
						fv_cloned->removeParent();
					}
				}
			}

			std::vector<SceneObject*> smplrs = group_parent_dummy->get_Samplers();
			if (cloning_options_clone_samplers)
			{
				//std::cout << " > removing cloned_smplrs from temp_group!" << std::endl;
				for (SceneObject* smplr_cloned : smplrs)
				{
					if (smplr_cloned->isTempClone())
					{
						//std::cout << "   - group_parent_dummy[ " << group_parent_dummy->getId() << " ]->remove_Sampler( " << smplr_cloned->getId() << " ) " << std::endl;
						group_parent_dummy->remove_Sampler(smplr_cloned->getId(), true);
						smplr_cloned->removeParent();
					}
				}
			}
		}


#endif

		//std::cout << "   - removing : group_parent_dummy[ " << group_parent_dummy->getId() << " ] " << std::endl;
		Mediator::removeObjectFromScene( group_parent_dummy ); // removing from scene ( completely ) the temporary area selection group - parent
		Mediator::force_Execute_Scene_Remove_Calls();          // forcing remove call

		// 
		bool is_as_instance = m_cloning_group && !m_cloning_asSingleGroup ? true : as_instance;
		bool force_at_A = false;
		if ( m_cloning_asSingleGroup && as_instance && groups_created.size() == 0 ) force_at_A = true;
		if ( !m_cloning_group || !is_as_instance || force_at_A )
		{
			
			// here we assume that we have multiple objects to deal with  ( > 1 )
			// but in case there was a single object with multiple ( >= 1 ) grouping , here will only pass the single object

			// case : [  single - object ] <=> childs = [ single_object ]
			if (childs.size() == 1)
			{
				m_multiple_selection_active = false;

				SceneObject* cloned_object  = childs[0]; cloned_object->setName(cloning_object_name); cloned_object->setNamePostfix("");
				setSelectedObject(cloned_object);
				// cloning utility options
				if (!cloning_options_clone_fvs && cloned_object->getType() != LIGHT_OBJECT)     
					cloned_object->remove_FaceVectors();
				if (!cloning_options_clone_samplers) 
					cloned_object->remove_Samplers();
				Action* action = new Action_CLONE(prev_object_pack.raw_object, cloned_object, true);
				ActionManager::GetInstance().push(action);
			}
			else
			{

				Multiple_Selection_Data area_selection_data;
				for (SceneObject* obj : childs)
				{
					area_selection_data.selection_objects.push_back(obj);
					Parent_Data parent_data; Utilities::fill_Parent_Data(obj, parent_data);
					area_selection_data.parents_data[obj->getId()] = parent_data;
					area_selection_data.parents_bucket[obj->getId()] = obj->getParent();
					area_selection_data.group_parents_bucket[obj->getId()] = obj->getRealGroupParent();
				}
				// cloning utility options
				for (SceneObject* child : area_selection_data.selection_objects)
				{
					if (!cloning_options_clone_fvs && child->getType() != LIGHT_OBJECT)      
						child->remove_FaceVectors();
					if (!cloning_options_clone_samplers) 
						child->remove_Samplers();
				}


				SceneObject* new_group_parent_dummy = Utilities::create_SceneGroupFrom(area_selection_data.selection_objects);
				new_group_parent_dummy->setTemporary( true );

				area_selection_data.parent_id = new_group_parent_dummy->getId();

				process_AreaSelection(area_selection_data);
				m_area_selections_data.push_back(area_selection_data);


				int prev_selected_obj_index = prev_object_pack.raw_object->getId();
				Action * action = new Action_CREATE_TEMP_GROUP(
					new_group_parent_dummy,
					childs,
					groups_created,
					m_area_selections_data.size() - 1,
					prev_selected_obj_index,
					true,
					true);


				setSelectedObject(new_group_parent_dummy, false);
				ActionManager::GetInstance().push( action );
				set_active_multiple_selection( new_group_parent_dummy->getId() );
			}
		}
		else
		{
			

			SceneObject* cloned_source = 0;
			bool found = false;
			for (SceneObject* group : groups_created)
			{

				std::vector<SceneObject*> childs = group->getGroupSelectionChilds();
				for (SceneObject* obj : childs)
				{
					if (obj->getSourceId() == prev_object_pack.raw_object->getId())
					{
						obj->setName(cloning_object_name);
						obj->setNamePostfix("");
						cloned_source = obj;
						found = true;
						break;
					}
				}
				if (found)break;
			}


			//std::cout << " - b " << std::endl;
			m_multiple_selection_active = false;
			SceneObject* cloned_group = groups_created[0];
			setSelectedObject(cloned_group);

			Action* action = new Action_CLONE( prev_object_pack.raw_object, cloned_group, true);
			ActionManager::GetInstance().push( action );
		}

		
#ifdef pre_clone_Utility_Childs
		
		//std::cout << "\n";
		// FaceVectors

		//bool let_ut_childs_ = !(as_instance || m_cloning_group);
		if (let_ut_childs_)
		{
			if (cloning_options_clone_fvs)
			{
				//std::cout << " > adding cloned fvs to created groups!" << std::endl;
				for (Object_General_Package pack : m_cloned_faceVectors)
				{
					SceneObject* fv = pack.obj;
					SceneObject* p = pack.parent;
					int pid = p->getId();
					SceneObject* cloned_group = 0;

					//std::cout << "   - cloned_fv : " << fv->getId() << std::endl;

					for (SceneObject* group : groups_created)
					{
						if (group->getSourceId() == pid)
						{
							cloned_group = group;
							break;
						}
					}

					if (cloned_group != 0)
					{
						fv->setVisible(true);
						fv->set_as_Temp_Clone(false);
						cloned_group->add_FaceVector(fv);
						//std::cout << "   - cloned_group[ " << cloned_group->getName() << " ]->add_FaceVector( " << fv->getId() << " ) : " << std::endl;
					}
				}
			}

			// Samplers
			if (cloning_options_clone_samplers)
			{
				//std::cout << " - adding cloned smplrs to created groups!" << std::endl;
				for (Object_General_Package pack : m_cloned_Samplers)
				{
					SceneObject* s = pack.obj;
					SceneObject* p = pack.parent;
					int pid = p->getId();
					SceneObject* cloned_group = 0;

					for (SceneObject* group : groups_created)
					{
						if (group->getSourceId() == pid)
						{
							cloned_group = group;
							break;
						}
					}

					if (cloned_group != 0)
					{
						s->setVisible(true);
						s->set_as_Temp_Clone(false);
						cloned_group->add_Sampler(s);
					}
				}
			}

		}

#endif

	}
	
	// case : Single Selection Cloning
	else
	{
	    //std::cout << "   - single Selection Cloning!" << std::endl;
		//std::cout << " - cloning_group : " << m_cloning_group << std::endl;

	    SceneObject* object = cloned_object_pack.raw_object;
	    // post config cloned object's utility childs ( clone facevectors / samplers ? )
		if (!cloning_options_clone_fvs)      object->remove_FaceVectors();   // clone facevectors ? 
		if (!cloning_options_clone_samplers) object->remove_Samplers();      // clone samplers ?

		//
		cloned_object_pack.raw_object->setNamePostfix("");
		cloned_object_pack.raw_object->setName( name );
		Action* action = new Action_CLONE(prev_object_pack.raw_object, cloned_object_pack.raw_object);
		ActionManager::GetInstance().push( action );
	}

	Update_current_Interface_State();
	cloned_object_pack.raw_object->setAttributesDelta(0, optix::make_float3(0));
	cloned_object_pack.raw_object->setAttributesDelta(1, optix::make_float3(0));
	cloned_object_pack.raw_object->setAttributesDelta(2, optix::make_float3(0));
	

	//std::cout << " - handle_CLONE()_END_::\n" << std::endl;
}
void InterfaceManager::handle_CLONE_Cancel(int cloning_id, Object_Package& cloned_object_pack, Object_Package& prev_object_pack, std::string name)
{
	
	m_force_input_update = true;

	bool is_area_selection_active = m_multiple_selection_active ? true : false;

	// case : Area Selection - Group Cloning
	if ( is_area_selection_active /* or group */ )
	{
		SceneObject* cloned_object = cloned_object_pack.raw_object;
		cloned_object->setActive(false);
		for (SceneObject* cloned_child : cloned_object->getGroupSelectionChilds_and_Groups())
		{
			cloned_child->setActive( false );
			Mediator::removeObjectFromScene( cloned_child );
		}

		Mediator::removeObjectFromScene( cloned_object );
		Mediator::force_Execute_Scene_Remove_Calls();

		// case : Area Selection -> Action Disband Temp Group Was pushed pre-cloning
		if ( !m_cloning_group )
		{
			// undo disband Temp Group Action 
			// And remove it from Stack
			ActionManager::GetInstance().undo();
			ActionManager::GetInstance().clear_stack_Above();
		}
		else
		{
			// set multiple selection active = false cause we just wanted it temporarily for the cloning phase
			// we dont want to revert any multiple_data state back
			m_multiple_selection_active = false;
			setSelectedObject( prev_object_pack , false, false );
			Mediator::setFocusedObject(prev_object_pack.object_index);
			Mediator::setSelectedObject(prev_object_pack.object_index);
		}
		
		
	}

	// case : Single Selection Cloning
	else
	{
		Mediator::removeObjectFromScene(cloned_object_pack.raw_object);
		cloned_object_pack.raw_object->setActive(false);
		
		

		setSelectedObject( prev_object_pack, false );
		Mediator::setFocusedObject(prev_object_pack.object_index);
		Mediator::setSelectedObject(prev_object_pack.object_index);
		
	}


	m_reselecting = true;
	Update_current_Interface_State();
	handle_m0_RELEASED();
	
}
void InterfaceManager::handle_DELETE()
{

	//m_ui_restrictions_panel_focused;
	//std::cout << " - should delete R : " << (m_ui_restrictions_panel_focused && m_UI_selected_Restriction != 0) << std::endl;
	bool should_delete_ui_R = (m_ui_restrictions_panel_focused && m_UI_selected_Restriction != 0);
	if (should_delete_ui_R)
	{
		Action* ac = new Action_CRT_DLT_RESTRICTION(m_UI_selected_Restriction, 0);
		ActionManager::GetInstance().push(ac);
		Mediator::Reset_Restrictions_UI_States();
		m_UI_selected_Restriction = nullptr;
		return;
	}

	if (m_selected_object_pack.raw_object == nullptr || m_selected_utility.active)
		return;

	if (!m_multiple_selection_active)
	{

		if (m_selected_object_pack.raw_object->isImmutable()) 
			return;

		bool is_face_vector = m_selected_object_pack.raw_object->getType() == FACE_VECTOR;
		bool is_sampler     = m_selected_object_pack.raw_object->getType() == SAMPLER_OBJECT;
		
		Action * action;
		if     (is_face_vector) 
			action = new Action_CRT_DLT_FACE_VECTOR(m_selected_object_pack.raw_object, 0, true);
		
		else if(is_sampler)     
			action = new Action_CRT_DLT_SAMPLER(m_selected_object_pack.raw_object, 0);
		
		else 
			action = new Action_DELETE(m_selected_object_pack.raw_object);
		
		ActionManager::GetInstance().push( action );

		if(!is_face_vector)
			Mediator::restartAccumulation();

		m_selected_utility.active = false;
		if (m_utility_interaction || true)
		{
			m_selected_object_pack.raw_object->setAttributesDelta(0, optix::make_float3(0));
			m_selected_object_pack.raw_object->setAttributesDelta(1, optix::make_float3(0));
			m_selected_object_pack.raw_object->setAttributesDelta(2, optix::make_float3(0));

			m_selected_utility.active = false; // (?) den isxuei gia ola ta utilities (?)
			m_interface_state.inner_radius_hit = false;
			m_utility_isec_p.reset();
			m_object_transformations_manip_data.reset();
		}

		mouse_data.m0_down = 0;
		mouse_data.m1_down = 0;
		mouse_data.m0_released = 0;
		mouse_data.m1_released = 0;
		m_interface_state.reset();


		m_utility_interaction = false;
		resetSelectedObject(false);
		return;

	}
	else
	{
		
		Action * ac = new Action_DELETE_TEMP_GROUP
		(m_selected_object_pack.raw_object,
		 m_area_selections_data[m_multiple_selection_active_id],
		 m_multiple_selection_active_id,
		 -1);

		ActionManager::GetInstance().push(ac);

		if (m_utility_interaction || true)
		{
			m_selected_object_pack.raw_object->setAttributesDelta(0, optix::make_float3(0));
			m_selected_object_pack.raw_object->setAttributesDelta(1, optix::make_float3(0));
			m_selected_object_pack.raw_object->setAttributesDelta(2, optix::make_float3(0));

			m_selected_utility.active = false; // (?) den isxuei gia ola ta utilities (?)
			m_interface_state.inner_radius_hit = false;
			m_utility_isec_p.reset();
			m_object_transformations_manip_data.reset();
		}

		mouse_data.m0_down     = 0;
		mouse_data.m1_down     = 0;
		mouse_data.m0_released = 0;
		mouse_data.m1_released = 0;
		m_interface_state.reset();
		m_utility_interaction = false;
		resetSelectedObject(false);

		return;
	}


}


void InterfaceManager::handle_UNDO()
{
	////std::cout << "\n  ______________UNDO():  ______________ " << std::endl;

	float l = m_transformation_input_data.delta_length;
	float l2 = length(m_transformation_input_data.overall_delta);
	float l3 = length(m_transformation_input_data.delta);
	////std::cout << "\n - m_input : " << m_transformation_input_i << std::endl;
	////std::cout << " - m_input_j : " << m_transformation_input_attr_index << std::endl;
	////std::cout << " - l : " << l << std::endl;
	////std::cout << " - l3 : " << l3 << std::endl;
	////std::cout << " - overall : " << l2 << std::endl;
	////std::cout << " ______________________________ " << std::endl;

	bool can_do = !(l != 0 || l2 != 0 || l3 != 0);
	if ( !can_do )
		return;

	
	ActionManager::GetInstance().undo();
	//Mediator::restartAccumulation();
}
void InterfaceManager::handle_REDO()
{
	////std::cout << "\n  ______________REDO():  ______________ " << std::endl;
	float l = m_transformation_input_data.delta_length;
	float l2 = length(m_transformation_input_data.overall_delta);
	float l3 = length(m_transformation_input_data.delta);
	////std::cout << "\n - m_input : " << m_transformation_input_i << std::endl;
	////std::cout << " - m_input_j : " << m_transformation_input_attr_index << std::endl;
	////std::cout << " - l : " << l << std::endl;
	////std::cout << " - l3 : " << l3 << std::endl;
	////std::cout << " - overall : " << l2 << std::endl;
	////std::cout << " ______________________________ " << std::endl;

	bool can_do = !(l != 0 || l2 != 0 || l3 != 0);
	if ( !can_do )
		return;


	ActionManager::GetInstance().redo();
	//Mediator::restartAccumulation();
}

void InterfaceManager::handle_LINK()
{
	
	bool is_multiple_link = false;
	if (m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY && m_selected_object_pack.raw_object->isTemporary())
		is_multiple_link = true;

	int is_linking_valid = true;

	if (is_multiple_link)
	{
		int active_area_selection = m_multiple_selection_active_id;
		Action_DISBAND_TEMP_GROUP temp_action = Action_DISBAND_TEMP_GROUP(m_selected_object_pack.raw_object, m_multiple_selection_active_id, -1);
		for (SceneObject * obj : m_area_selections_data[active_area_selection].top_level_entities)
		{
			is_linking_valid &= Utilities::is_Linking_valid(obj, m_focused_object_pack.raw_object);
		}
		temp_action.undo();
		
	}
	else
		is_linking_valid = Utilities::is_Linking_valid(m_selected_object_pack.raw_object, m_focused_object_pack.raw_object);
	

	if (is_linking_valid > 0)
	{
		
		
		ACTION_ID ac_id = ACTION_LINKING_LINK;
		Action* action;
		if (is_multiple_link)
		{
			ac_id = ACTION_MULTIPLE_LINK;
			int active_area_selection = m_multiple_selection_active_id;
			SceneObject* parent = m_focused_object_pack.raw_object;
			
			Action * ac_disband = get_Temp_Group_Disband_Action(m_multiple_selection_active_id, -1);
			m_multiple_selection_active = false;

			action = new Action_Multiple_LINKING( m_area_selections_data[active_area_selection].top_level_entities, parent, ac_id , true );
			ActionManager::GetInstance().push( action );
			
			// create new area selection with the same objects
			m_multiple_selected_objects.clear();
			for ( SceneObject* obj : m_area_selections_data[active_area_selection].selection_objects )
			{
				m_multiple_selected_objects.push_back(obj);
			}

			// handle_MULTIPLE_SELECTION();
			Action * ac_create_group = get_MULTIPLE_SELECTION();
			
			Action_Multiple_LINKING * multiple_link = (Action_Multiple_LINKING *)action;
			multiple_link->set_Action_Disband(ac_disband);
			multiple_link->set_Action_Create_Temp_Group(ac_create_group);

		}
		else
		{
			Utilities::object_Link(m_selected_object_pack.raw_object, m_focused_object_pack.raw_object);
			action = new Action_LINKING( m_selected_object_pack.raw_object, m_focused_object_pack.raw_object, ac_id , true );
			ActionManager::GetInstance().push( action );
		}
		
	}



}
void InterfaceManager::handle_UNLINK()
{

	if (m_selected_object_pack.raw_object != nullptr)
	{
		Type obj_type = m_selected_object_pack.raw_object->getType();
		if (obj_type == FACE_VECTOR || obj_type == SAMPLER_OBJECT)
			return;

		bool is_multiple_unlink = false;
		SceneObject* parent = m_selected_object_pack.raw_object->getParent();

		if (m_multiple_selection_active)
			is_multiple_unlink = true;

		if ( is_multiple_unlink )
		{
			int active_area_selection = m_multiple_selection_active_id;
			Action_DISBAND_TEMP_GROUP temp_ac = Action_DISBAND_TEMP_GROUP(m_selected_object_pack.raw_object, m_multiple_selection_active_id, - 1);
			std::vector<SceneObject*> unlinks;
			for (SceneObject* obj : m_area_selections_data[active_area_selection].selection_objects)
			{
				if (   Utilities::is_Unlinking_valid(obj)
					&& obj->getGroupParent() == nullptr
					//&& obj->getType() != GROUP_PARENT_DUMMY
					)
				{
					unlinks.push_back(obj);
				}
			}
			temp_ac.undo();

			if (unlinks.size() == 0)
				return;

			//resetSelectedObject(true, true);
			Action * ac_disband = get_Temp_Group_Disband_Action(m_multiple_selection_active_id, -1);
			m_multiple_selection_active = false;

			ACTION_ID ac_id = ACTION_MULTIPLE_UNLINK;
			Action * action = new Action_Multiple_LINKING(unlinks,
														  m_area_selections_data[active_area_selection].top_level_entities,
														  m_area_selections_data[active_area_selection].top_level_groups,
														  ACTION_ID::ACTION_MULTIPLE_UNLINK,
														  true);
			ActionManager::GetInstance().push( action );


			// create new area selection with the same objects
			m_multiple_selected_objects.clear();
			for (SceneObject* obj : m_area_selections_data[active_area_selection].selection_objects)
			{
				m_multiple_selected_objects.push_back(obj);
			}
			
			// handle_MULTIPLE_SELECTION();
			Action * ac_create = get_MULTIPLE_SELECTION();

			Action_Multiple_LINKING* ac_multiple_link = (Action_Multiple_LINKING*) action;
			ac_multiple_link->set_Action_Disband( ac_disband );
			ac_multiple_link->set_Action_Create_Temp_Group( ac_create );

		}
		else
		{
			if (Utilities::is_Unlinking_valid( m_selected_object_pack.raw_object ) )
			{
				ACTION_ID ac_id = ACTION_LINKING_UNLINK;
				Action* action = new Action_LINKING( m_selected_object_pack.raw_object, parent, ac_id, true, true, true , true );
				ActionManager::GetInstance().push( action );

#ifdef UNLINK_AFTER_ACTION
				Utilities::object_unLink( m_selected_object_pack.raw_object );
#endif
			}
		}

	}

}


void InterfaceManager::handle_SELECTION()
{

}
void InterfaceManager::handle_AREA_SELECTION()
{
	////std::cout << " - Disabling Area Selection!" << std::endl;
	optix::float2 a = optix::make_float2(mouse_data.m0_down_x, mouse_data.m0_down_y);
	optix::float2 b = optix::make_float2(mouse_data.x, mouse_data.y);
	////std::cout << "\n -- Rect : { ( " << a.x << " , " << a.y << " ) , ( " << b.x << " , " << b.y << " ) }" << std::endl;
	
	
	if (length(b - a) < 0.0001f)
	{
		m_selected_utility.active = false;
		is_m0_down = false;
		return;
	}

	a = Utilities::getMousePosRelativeToViewport(ViewportManager::GetInstance().getActiveViewport().id, a);
	b = Utilities::getMousePosRelativeToViewport(ViewportManager::GetInstance().getActiveViewport().id, b);
	Rect2D rect = Geometry::SHAPES::createRect2D(a, b);
	

	// vector with selected objs ids
	m_multiple_selected_objects = getSelectedObjectsFromFrustum(rect, ViewportManager::GetInstance().getActiveViewport());

	// in case there are more than 2 selected objects returned from the selected rectangular area
	// we create and store a temporary "SCENE GROUP".
	if (m_multiple_selected_objects.size() >= 2 )
	{


#define PUSH_TEMP_GROUP_TO_STACK
#ifdef PUSH_TEMP_GROUP_TO_STACK


		if (m_multiple_selection_active)
		{
			resetSelectedObject( false, true );
		}

		Multiple_Selection_Data area_selection_data;		
		for (SceneObject* obj : m_multiple_selected_objects)
		{
			area_selection_data.selection_objects.push_back(obj);
			Parent_Data parent_data; 
			Utilities::fill_Parent_Data(obj, parent_data);
			area_selection_data.parents_data[obj->getId()] = parent_data;
			area_selection_data.parents_bucket[obj->getId()] = obj->getParent();
			area_selection_data.group_parents_bucket[obj->getId()] = obj->getRealGroupParent();
		}
		process_AreaSelection(area_selection_data , true);
		
		SceneObject* group_parent_dummy = Utilities::create_SceneGroupFrom(area_selection_data.selection_objects);
		group_parent_dummy->setTemporary(true);
		m_temp_group_parent_id = Mediator::requestObjectsArrayIndex( group_parent_dummy );
		area_selection_data.parent_id = group_parent_dummy->getId();
		m_area_selections_data.push_back(area_selection_data);

		//int prev_selected_obj_index = m_selected_object_pack.object_index;
		int prev_selected_obj_index = m_selected_object_pack.raw_object == nullptr? -1 : m_selected_object_pack.raw_object->getId();

		
		Action * action = new Action_CREATE_TEMP_GROUP
		( 
			group_parent_dummy, 
			area_selection_data.selection_objects, //m_multiple_selected_objects, 
			m_area_selections_data.size() - 1, 
			prev_selected_obj_index,
			false,
			true
		);

		setSelectedObject( group_parent_dummy );

		ActionManager::GetInstance().push(action);
		set_active_multiple_selection( group_parent_dummy->getId() );

#endif

		
	}
	else if (m_multiple_selected_objects.size() == 1)
	{
		SceneObject* obj = m_multiple_selected_objects[0];
		//resetSelectedObject(false, true);
		setSelectedObject( obj );

		//m_reselecting = true;
		m_selected_utility.active   = false;
		m_multiple_selection_active = false;

	}
	else if(m_multiple_selected_objects.size() == 0)
	{
		resetSelectedObject();
		m_multiple_selection_active = false;
	}

	m_multiple_selected_objects.clear();
	m_selected_utility.active   = false;
	
	
	toggle(m_prev_utility, false, true);
	if (m_selection_setting == S_OBJECT)     toggle(u_SELECTION, false, true);
	else if (m_selection_setting == S_GROUP) toggle(u_SELECTION_GROUP, false, true);
	//toggle_SELECTION();
	
}
Action * InterfaceManager::get_AREA_SELECTION()
{
	return 0;
}
void InterfaceManager::handle_MULTIPLE_SELECTION()
{
	////std::cout << "\n - handle_Multiple_SELECTION():" << std::endl;

	// in case there are more than 2 selected objects returned from the selected rectangular area
	// we create and store a temporary "SCENE GROUP".
	if (m_multiple_selected_objects.size() >= 2)
	{

#define PUSH_TEMP_GROUP_TO_STACK
#ifdef PUSH_TEMP_GROUP_TO_STACK

		//int prev_selected_obj_index = m_selected_object_pack.object_index;
		int prev_selected_obj_index = m_selected_object_pack.raw_object == nullptr? -1 : m_selected_object_pack.raw_object->getId();
		
		if( m_multiple_selection_active )
			resetSelectedObject(false, true);

		Multiple_Selection_Data area_selection_data;
		for (SceneObject* obj : m_multiple_selected_objects)
		{
			area_selection_data.selection_objects.push_back(obj);
			Parent_Data parent_data; Utilities::fill_Parent_Data(obj, parent_data);
			area_selection_data.parents_data[obj->getId()] = parent_data;
			area_selection_data.parents_bucket[obj->getId()] = obj->getParent();
			area_selection_data.group_parents_bucket[obj->getId()] = obj->getRealGroupParent();
		}
		process_AreaSelection(area_selection_data, true);

		SceneObject* group_parent_dummy = Utilities::create_SceneGroupFrom(area_selection_data.selection_objects);
		group_parent_dummy->setTemporary(true);
		m_temp_group_parent_id = Mediator::requestObjectsArrayIndex(group_parent_dummy);
		area_selection_data.parent_id = group_parent_dummy->getId();
		m_area_selections_data.push_back(area_selection_data);
		//

		
		Action * action = new Action_CREATE_TEMP_GROUP
		(
			group_parent_dummy,
			area_selection_data.selection_objects, 
			m_area_selections_data.size() - 1,
			prev_selected_obj_index,
			false,
			true
		);

		setSelectedObject(group_parent_dummy);

		ActionManager::GetInstance().push(action);
		set_active_multiple_selection(group_parent_dummy->getId());

#endif


	}

	m_multiple_selected_objects.clear();
	m_selected_utility.active = false;
	
	toggle(m_selected_utility.id, false, true);
	if      (m_selection_setting == S_OBJECT) toggle(u_SELECTION, false, true);
	else if (m_selection_setting == S_GROUP)  toggle(u_SELECTION_GROUP, false, true);
}
Action * InterfaceManager::get_MULTIPLE_SELECTION()
{
	
	// in case there are more than 2 selected objects returned from the selected rectangular area
	// we create and store a temporary "SCENE GROUP".
	if (m_multiple_selected_objects.size() >= 2)
	{

#define PUSH_TEMP_GROUP_TO_STACK
#ifdef PUSH_TEMP_GROUP_TO_STACK

		//int prev_selected_obj_index = m_selected_object_pack.object_index;
		int prev_selected_obj_index = m_selected_object_pack.raw_object == nullptr ? -1 : m_selected_object_pack.raw_object->getId();

		if (m_multiple_selection_active)
			resetSelectedObject(false, true);

		Multiple_Selection_Data area_selection_data;
		for (SceneObject* obj : m_multiple_selected_objects)
		{
			area_selection_data.selection_objects.push_back(obj);
			Parent_Data parent_data; Utilities::fill_Parent_Data(obj, parent_data);
			area_selection_data.parents_data[obj->getId()] = parent_data;
			area_selection_data.parents_bucket[obj->getId()] = obj->getParent();
			area_selection_data.group_parents_bucket[obj->getId()] = obj->getRealGroupParent();
		}
		process_AreaSelection(area_selection_data, true);

		SceneObject* group_parent_dummy = Utilities::create_SceneGroupFrom(area_selection_data.selection_objects);
		group_parent_dummy->setTemporary(true);
		m_temp_group_parent_id = Mediator::requestObjectsArrayIndex(group_parent_dummy);
		area_selection_data.parent_id = group_parent_dummy->getId();
		m_area_selections_data.push_back(area_selection_data);
		//


		Action * action = new Action_CREATE_TEMP_GROUP
		(
			group_parent_dummy,
			area_selection_data.selection_objects,
			m_area_selections_data.size() - 1,
			prev_selected_obj_index,
			false,
			true
		);
		setSelectedObject(group_parent_dummy);
		set_active_multiple_selection(group_parent_dummy->getId());

#endif


		return action;
	}

	m_multiple_selected_objects.clear();
	m_selected_utility.active = false;
	return 0;
}

void InterfaceManager::handle_TRANSFORMATION()
{
	if (!m_transforming_active)
	{
		std::vector<VIEWPORT> viewports = ViewportManager::GetInstance().getAllEnabledViewports();
		for (VIEWPORT vp : viewports)
		{
			PinholeCamera cam_inst = ViewportManager::GetInstance().getViewportCamera(vp.id);
			float obj_dist = cam_inst.getDistanceFrom(m_selected_object_pack.raw_object->getTranslationAttributes());
			m_selected_object_pack.obj_dists[vp.id] = obj_dist;
		}
	}

	//Utility ut_id = m_selected_utility.id;
	//bool is_transformation_valid = true;
	//if (m_selected_utility.id == u_TRANSLATION && m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY)
	//	return;

	
	switch (m_selected_utility.id)
	{

	case u_TRANSLATION:
		handle_TRANSLATION();
		break;

	case u_ROTATION:
		handle_ROTATION();
		break;

	case u_SCALE:
		handle_SCALE();
		break;
	}

	m_transforming_active = true;

}
void InterfaceManager::handle_TRANSLATION()
{
#ifdef CANCER_DEBUG
	if (m_selected_object_pack.raw_object == nullptr || m_selected_object_pack.object_index < 0)
	{
		//std::cout << "\n handle_TRANSLATION():" << std::endl;
		//std::cout << " m_selected_object_pack.raw_object == nullptr!" << std::endl;
		//std::cout << " m_selected_object_pack.object_index < 0 " << std::endl;
		system("pause");
	}
#endif
	
	optix::float3 U = Mediator::RequestCameraInstance().getU();
	optix::float3 V = Mediator::RequestCameraInstance().getV();
	optix::float3 W = Mediator::RequestCameraInstance().getW();

	optix::float3 * M_WORKING_AXIS; // = &m_selected_object_pack.base_axis[0];
	M_WORKING_AXIS = &m_selected_object_pack.base_axis[0];

	Plane utility_plane = m_object_transformations_manip_data.utility_plane;
	UtilityPlane plane_id = m_object_transformations_manip_data.plane_id;
	bool is_axis = (plane_id == X || plane_id == Y || plane_id == Z);
	float starting_delta = m_object_transformations_manip_data.starting_delta;
	optix::float3 start_translation = m_object_transformations_manip_data.start_translation;
	optix::float3 starting_plane_delta = m_object_transformations_manip_data.starting_plane_delta;

	Ray m_utility_ray = Geometry::SHAPES::createMouseRay();
	float dotRayN = optix::dot(W, M_WORKING_AXIS[plane_id - 1]);

	optix::float3 iP; // Intersection Point
	if (Geometry::RAY::Intersect_Plane(m_utility_ray, utility_plane, iP) != 1
		|| fabs(fabs(dotRayN) - 1.0f) < 0.005f && is_axis) // EDW NA KANW CONSTRAINT KAI TO POSO MAKRIA PAEI EPISHS
	{
		mouse_data.m0_dragging = false;
		mouse_data.m0_down = false;
		m_selected_utility.active = false;
		return;
	}


#ifdef UTILITY_ISEC_P_REPROJECT_TO_CENTER
	iP = iP - m_utility_isec_p.center;
#else
	//
#endif

	float delta;
	optix::float3 plane_delta;
	optix::float3 translation = m_selected_object_pack.raw_object->getTranslationAttributes();

	if      (is_axis)
	{

		delta = optix::dot(M_WORKING_AXIS[plane_id - 1], iP); // actual delta on selected axis
		////std::cout << "\n - Translation Delta : " << (delta - starting_delta) << " " << std::endl;
		////std::cout << " - M_WORKING_AXIS[" << (plane_id - 1) << "] : ( " << M_WORKING_AXIS[plane_id - 1].x << " , " << M_WORKING_AXIS[plane_id - 1].y << " , " << M_WORKING_AXIS[plane_id - 1].z << " ) " << std::endl;
		optix::float3 trs_delta = M_WORKING_AXIS[plane_id - 1] * (delta - starting_delta);
		if (!Utilities::is_object_type_Muttable_To_Transformation(m_selected_object_pack.raw_object)) trs_delta = ZERO_3f;
		m_selected_object_pack.raw_object->setTransformation(trs_delta,
															 m_selected_axis_orientation, 
															 TRANSLATION,
															 m_hierarchy_setting, 
															 0, 
															 m_selected_object_pack.base_axis);
															 //m_object_transformations_manip_data.base_axis);

		m_selected_object_pack.raw_object->setAttributesDelta(0, translation - start_translation);
		m_object_transformations_manip_data.starting_delta = delta;
		m_object_transformations_manip_data.delta_length += fabsf(delta - starting_delta);
		m_object_transformations_manip_data.overall_delta += trs_delta;
		
	}
	else if (!is_axis)
	{
		plane_delta = iP;
		optix::float3 current_plane_delta = plane_delta - starting_plane_delta;
		float current_delta = optix::length( current_plane_delta );
		float dx = mouse_data.dx;
		float dy = mouse_data.dy;
		float mouse_delta = fabsf(dx) + fabsf(dy);
		float delta_ratio = mouse_delta / current_delta;
		if (delta_ratio < 0.2f || delta_ratio == INFINITY)
		{
			if (delta_ratio == INFINITY)
				return;

			////std::cout << " ---aligning!" << std::endl;
			//current_plane_delta *= 1e-5f;
		}

		//m_selected_object_pack.raw_object->setTranslation((current_plane_delta));
		if (!Utilities::is_object_type_Muttable_To_Transformation(m_selected_object_pack.raw_object)) current_plane_delta = ZERO_3f;
		m_selected_object_pack.raw_object->setTransformation(current_plane_delta, 
															 m_selected_axis_orientation, 
															 TRANSLATION,
															 m_hierarchy_setting,
															 0, 
															 m_selected_object_pack.base_axis);
															 //m_object_transformations_manip_data.base_axis);

		m_selected_object_pack.raw_object->setAttributesDelta(0, translation - start_translation);
		m_object_transformations_manip_data.starting_plane_delta = plane_delta;
		m_object_transformations_manip_data.delta_length += fabsf(length(plane_delta - starting_plane_delta));
		m_object_transformations_manip_data.overall_delta += current_plane_delta;
		////std::cout << " mouse_delta / delta : ( " << (mouse_delta/current_delta) << std::endl;
	}

	//m_object_transformations_manip_data.delta = m_selected_object_pack.raw_object->getTranslationAttributes() - m_object_transformations_manip_data.start_translation;
}
void InterfaceManager::handle_ROTATION()
{
#ifdef CANCER_DEBUG
	if (m_selected_object_pack.raw_object == nullptr || m_selected_object_pack.object_index < 0)
	{
		//std::cout << "\n handle_ROTATION():" << std::endl;
		//std::cout << " m_selected_object_pack.raw_object == nullptr!" << std::endl;
		//std::cout << " m_selected_object_pack.object_index < 0 " << std::endl;
		system("pause");
	}
#endif

	if (m_utility_isec_p.inner_radius_hit)
	{
		
		optix::float3 * M_WORKING_AXIS = m_selected_object_pack.base_axis;
		optix::float3 pos       = m_selected_object_pack.raw_object->getTranslationAttributes();
		optix::float3 center    = m_selected_object_pack.raw_object->getTranslationAttributes();
		float renderSize_factor = ViewportManager::GetInstance().getActiveViewportCamera().getInvScaleFactor(ViewportManager::GetInstance().getActiveViewport());
		float dist_factor       = ViewportManager::GetInstance().getActiveViewportCamera().getDistanceFrom(m_selected_object_pack.raw_object->getTranslationAttributes());
		
		// OLD WAY //
		/*
		Ray ray = Geometry::SHAPES::createMouseRay();
		Plane view_plane = Geometry::SHAPES::createPlane( ray.origin + ray.direction * dist_factor , -ray.direction);
		optix::float3 isec_p;
		Geometry::RAY::Intersect_Plane(ray, view_plane, isec_p);
		optix::float3 P = center - isec_p;
		P = (ray.direction * dist_factor / renderSize_factor);  //dist_factor / renderSize_factor);// - m_object_transformations_manip_data.starting_point;
		*/

		// New Way //
		PinholeCamera camera_inst         = Mediator::RequestCameraInstance();
		optix::float3 camera_focus_center = camera_inst.getFocusCenter();
		if (    camera_focus_center.x != center.x
			 || camera_focus_center.y != center.y
			 || camera_focus_center.z != center.z
			)
		{
			//camera_inst.setCenter( center );
		}
		
		camera_inst.setCameraPos_Deprecated(ZERO_3f - optix::normalize(camera_inst.getW()) * dist_factor);
		camera_inst.setCenter_Deprecated(ZERO_3f);
		//camera_inst.getFrustum_Deprecated();
		
		Ray ray          = Geometry::SHAPES::createMouseRay(camera_inst, ViewportManager::GetInstance().getActiveViewport());
		Plane view_plane = Geometry::SHAPES::createPlane
		( 
			ZERO_3f, 
		    -ray.direction
		); 
		optix::float3 isec_p;
		Geometry::RAY::Intersect_Plane(ray, view_plane, isec_p);
	    optix::float3 P = center - isec_p;
		P = (ray.direction * dist_factor / renderSize_factor);
		

		// check if in inner radius
		AXIS_ORIENTATION prev_axis;
		Ray r_ray  = Geometry::SHAPES::createMouseRay();
		bool in    = true;
		float dist = optix::length(r_ray.origin - center);
		optix::float3 mouse_pos = r_ray.origin + r_ray.direction * dist;
		float eq   = (mouse_pos.x - center.x) * (mouse_pos.x - center.x) + (mouse_pos.y - center.y) * (mouse_pos.y - center.y) + (mouse_pos.z - center.z) * (mouse_pos.z - center.z);
		float rad  = UTILITY_SCALE_FACTOR * m_renderer_camera_inv_scale_factor * 0.7f;
		float eq2  = rad * rad + 0.35f * m_renderer_camera_inv_scale_factor * m_renderer_camera_inv_scale_factor;
		

		if ( eq > eq2  
			//&& false
			)
		{
			in                          = false;
			prev_axis                   = m_selected_axis_orientation;
			m_selected_axis_orientation = AXIS_ORIENTATION::VIEW_OBJECT_DEPENDANT; // AXIS_ORIENTATION::SCREEN;
			BASE_TRANSFORM::AXIS(m_selected_object_pack.raw_object ,m_selected_axis_orientation, M_WORKING_AXIS);
		}
		center = ZERO_3f;
		//

		optix::float3 tang_x, tang_y, tang_z, tang_w;
		optix::float3 proj_x, proj_y, proj_z, proj_w;
		optix::float3 v = (P - center);
		
		float dist_x = optix::dot(M_WORKING_AXIS[0], v);
		proj_x = P - dist_x * optix::normalize(M_WORKING_AXIS[0]);

		float dist_y = optix::dot(M_WORKING_AXIS[1], v);
		proj_y = P - dist_y * optix::normalize(M_WORKING_AXIS[1]);

		float dist_z = optix::dot(M_WORKING_AXIS[2], v);
		proj_z = P - dist_z * optix::normalize(M_WORKING_AXIS[2]);


		int index  = 0;
		int index2 = 0;
		float max_dist = fabs(dist_x);
		float min_dist = max_dist;
		if (fabs(dist_y) > max_dist)
		{
			max_dist = fabs(dist_y);
			index = 1;
		}
		if (fabs(dist_z) > max_dist)
		{
			max_dist = fabs(dist_z);
			index = 2;
		}
		if (fabsf(dist_y) < min_dist)
		{
			min_dist = fabs(dist_y);
			index2 = 1;
		}
		if (fabs(dist_z) < max_dist)
		{
			min_dist = fabs(dist_z);
			index2 = 2;
		}

		optix::float3 weights = optix::make_float3(1.0f) - optix::normalize(optix::make_float3(fabs(dist_x), fabs(dist_y), fabs(dist_z)));
		((float*)&weights)[index] *= 2.0f;
		//optix::float3 weights = optix::make_float3(1.0f);
		
		
		
		IntersectionPoint ip;
		ip.p = proj_x;
		ip.normal = normalize(proj_x - center);
		tang_x = Geometry::VECTOR_SPACE::getCircleTangentOnPlane(ip, M_WORKING_AXIS, UtilityPlane::X);

		ip.p = proj_y;
		ip.normal = normalize(proj_y - center);
		tang_y = Geometry::VECTOR_SPACE::getCircleTangentOnPlane(ip, M_WORKING_AXIS, UtilityPlane::Y);

		ip.p = proj_z;
		ip.normal = normalize(proj_z - center);
		tang_z = Geometry::VECTOR_SPACE::getCircleTangentOnPlane(ip, M_WORKING_AXIS, UtilityPlane::Z);

		float scale_factor = 13.5f / ( dist_factor * 1.5f ); // 15.0f
		if (scale_factor > 13.5f) scale_factor = 13.5f;
		
		optix::float3 delta = (P - m_object_transformations_manip_data.starting_point);

		float dx, dy, dz;
		dx = optix::dot(delta, tang_x);
		dy = optix::dot(delta, tang_y);
		dz = optix::dot(delta, tang_z);



//#define B2_OUTER_LIMIT 1
//#define B2_OUTER_LIMIT 2
#define B2_SMOOTH_INTERPOLATION 1
		optix::float3 sign = optix::make_float3(-1.0f);
		if ( !in 
			//&& false
			)
		{
			optix::float3 pp    = ray.origin + ray.direction * m_renderer_camera_inv_scale_factor * renderSize_factor;
			optix::float3 delta = pp - center;

			float f1 = m_renderer_camera_inv_scale_factor;
			float f2 = sqrtf(dist_factor) * renderSize_factor;

			//float eq_max = rad * rad + 5.35f * f1 * f2 / (0.5f * renderSize_factor);
			float eq_max   = eq2;
			float eq_max_2;
			float a, b, b2;

#ifdef B2_OUTER_LIMIT

			eq_max_2 = rad * rad + 7.35f * m_renderer_camera_inv_scale_factor * m_renderer_camera_inv_scale_factor / (0.5f * renderSize_factor);
			b2 = optix::clamp(fabsf(eq / eq_max_2), 0.0f, 1.0f);
			b  = optix::clamp(fabsf(eq / eq_max), 0.0f, 1.0f);
			a  = 1 - b;

#elif B2_SMOOTH_INTERPOLATION

			eq_max_2 = rad * rad + 3.35f * m_renderer_camera_inv_scale_factor * m_renderer_camera_inv_scale_factor; // / (0.5f * renderSize_factor);
			b2 = optix::clamp(fabsf(eq / eq_max_2), 0.0f, 1.0f);
			b  = optix::clamp(fabsf(eq / eq_max  ), 0.0f, 1.0f);
			a  = 1 - b;
			// a = 1 - b2;

#else

			b = optix::clamp(fabsf(eq / eq_max), 0.0f, 1.0f);
			a = 1 - b;

#endif
			// find min-max rot_deltas
			float min_dt = dx;
			int i = 0;
			int j = 1;
			if (fabsf(dy) < fabsf(min_dt))
			{
				min_dt = dy;
				i = 1;
				j = 0;
			}


			// set weights //
//#define EVALUATE_WEIGHTS_BASED_ON_OFFSET
#ifdef EVALUATE_WEIGHTS_BASED_ON_OFFSET

			{
				optix::float3 starting_ray_dir = m_object_transformations_manip_data.starting_dir;
				optix::float3 o = Mediator::RequestCameraInstance().getCameraPos();
				Ray r0  = Geometry::SHAPES::createRay(o, starting_ray_dir);
				Ray r1  = Geometry::SHAPES::createRay(o, r_ray.direction);
				Plane p = Geometry::SHAPES::createPlane(pos, -optix::normalize(Mediator::RequestCameraInstance().getW()) ); //GLOBAL_AXIS[2]);
				optix::float3 p0, p1;
				Geometry::RAY::Intersect_Plane(r0, p, p0);
				Geometry::RAY::Intersect_Plane(r1, p, p1);
				optix::float3 dv = p1 - p0;
				float dv_u  = optix::dot(dv, GLOBAL_AXIS[0]);
				float dv_v  = optix::dot(dv, GLOBAL_AXIS[1]);
				float dv_w  = optix::dot(dv, M_WORKING_AXIS[2]);
				float max_d = fabsf(dv_u);
				int ind = 0;
				if (fabsf(dv_v) > max_d)
				{
					max_d = fabsf(dv_v);
					ind = 1;
				}
				optix::float3 o_offset = (r1.origin + r1.direction * dist_factor) - pos;
				float offset_ind = ((float*)&o_offset)[0];
				if (ind == 0)
					offset_ind = ((float*)&o_offset)[1];

				float c_x = (mouse_pos.x - pos.x) * (mouse_pos.x - pos.x);
				float c_y = (mouse_pos.y - pos.y) * (mouse_pos.y - pos.y);
				bool eqx = c_x < eq2;
				bool eqy = c_y < eq2;

				//std::cout << "\n" << std::endl;

				//std::cout << "    - dv : [ " << dv.x << " , " << dv.y << " , " << dv.z << " ] " << std::endl;
				//std::cout << "            - dv_u : " << dv_u << std::endl;
				//std::cout << "            - dv_v : " << dv_v << std::endl;
				//std::cout << "            - dv_w : " << dv_w << std::endl;
				//std::cout << "    - ( mouse.x - center.x ) ^ 2 : " << c_x << std::endl;
				//std::cout << "    - ( mouse.y - center.y ) ^ 2 : " << c_y << std::endl;
				//std::cout << "    - circle_radius              : " << eq2 << std::endl;
				//std::cout << "    - in [X] radius : " << eqx << std::endl;
				//std::cout << "    - in [Y] radius : " << eqy << std::endl;


				if ( !in )
				{
					if (eqx && ind == 1)
					{
						//((float*)&weights)[0] = 1.0f;
						((float*)&weights)[1] = 1 - b / 1.25f;
					}

					if (eqy && ind == 0)
					{
						//((float*)&weights)[1] = 1.0f;
						((float*)&weights)[0] = 1 - b / 1.25f;
					}

					if (!eqx)
					{
						((float*)&weights)[0] = 1 - b / 1.2f;
					}

					if (!eqy)
					{
						((float*)&weights)[1] = 1 - b / 1.2f;
					}

				}
		}

#endif

#define EVALUATE_WEIGHTS_BASED_ON_MOUSE_MOVEMENT
#ifdef EVALUATE_WEIGHTS_BASED_ON_MOUSE_MOVEMENT

			////std::cout << "\n - mouse_delta : ( " << mouse_data.dx << " , " << mouse_data.dy << " ) " << std::endl;
			////std::cout << " - delta_ratio : " << mouse_data.dx / (float)(mouse_data.dy+1) << std::endl;
			////std::cout << " - x-y diff    : " << fabsf(mouse_data.dx) - fabsf(mouse_data.dy) << std::endl;
			optix::float2 m           = optix::make_float2(mouse_data.dx, mouse_data.dy);
			optix::float2 prev_m      = optix::make_float2(mouse_data.prev_dx, mouse_data.prev_dy);
			optix::float2 diff        = optix::make_float2(fabsf(m.x) - fabsf(prev_m.x), fabsf(m.y) - fabsf(prev_m.y));
			optix::float2 norm_m      = normalize(m);      //+ optix::make_float2(0.1f);
			optix::float2 norm_prev_m = normalize(prev_m); //+ optix::make_float2(0.1f);
			float dot = optix::dot(norm_m, norm_prev_m);

			bool is_valid = true;
			if (dot != dot)
				is_valid = false;

			////std::cout << "\n  - current d : [ " << m.x << " , " << m.y << " ] " << std::endl;
			////std::cout <<   "  - prev    d : [ " << prev_m.x << " , " << prev_m.y << " ] " << std::endl;
			////std::cout <<   "  - diff      : [ " << fabsf(m.x) - fabsf(prev_m.x) << " , " << fabsf(m.y) - fabsf(prev_m.y) << " ] " << std::endl;
			////std::cout <<   "\n  > dot : " << dot << std::endl;

			if (is_valid)
			{

				
				float er = 0.9f;
				float d = fabsf( dot );
				bool is_continuous = d > er ? true : false;
				if (!is_continuous)
				{
					////std::cout << "     - adjusting!" << std::endl;
					//float t = 1.0f - d;
					//t = clamp(t, 0.0f, 1.0f);

					//((float*)&weights)[ j ] = 1.0f - b / (1.0f + d);
					//((float*)&weights)[ i ] = 1.0f - b / (1.0f + d);   //*= logf(b) * (-1.0f);

					((float*)&weights)[j] *= 0.5f;
					((float*)&weights)[i] *= 0.25f;
				}
			}
#endif


#ifdef B2_OUTER_LIMIT
			//((float*)&weights)[j] = 1 - b2 / 1.f;
#elif B2_SMOOTH_INTERPOLATION
				//((float*)&weights)[j] = 1 - b / 1.45f;
#else
			//((float*)&weights)[j] = 1 - b / 1.5f;
#endif
			//((float*)&weights)[i] *= logf(b) * (-1.0f);
			
			// clamp weights
			if (weights.x > 1)
				weights.x = 1.0f;
			if (weights.y > 1)
				weights.y = 1.0f;
			if (weights.x < 0.0f)
				weights.x = 0.0f;
			if (weights.y < 0.0f)
				weights.y = 0.0f;

			weights.z = b * renderSize_factor * 0.6f;
			sign      = optix::make_float3(-1.0f, -1.0f, 1.0f);

		}

		
		//
		optix::float3 dp = optix::make_float3(dx, dy, dz) * scale_factor * weights * sign;
		if (m_repositioning_cursor) dp = ZERO_3f;
		if (!Utilities::is_object_type_Muttable_To_Transformation(m_selected_object_pack.raw_object)) dp = ZERO_3f;
		m_selected_object_pack.raw_object->setTransformation(dp,
															 m_selected_axis_orientation,
															 ROTATION,
															 m_hierarchy_setting,
															 0,
															 m_selected_object_pack.base_axis);
															 //m_object_transformations_manip_data.base_axis);

		m_selected_object_pack.raw_object->setAttributesDelta(1, m_selected_object_pack.raw_object->getAttributesDelta(1) + dp);
		m_object_transformations_manip_data.starting_plane_delta = dp;
		m_object_transformations_manip_data.starting_point = P;
		m_object_transformations_manip_data.delta_length  += (length(dp));
		m_object_transformations_manip_data.starting_dir   = r_ray.direction;
		m_object_transformations_manip_data.delta += dp;
		m_object_transformations_manip_data.overall_delta += dp;
		//m_object_transformations_manip_data.stack_rotations.push_back(dp);

		if (!in 
			//&& false
			)
		{
			m_selected_axis_orientation = prev_axis;
			BASE_TRANSFORM::AXIS(m_selected_object_pack.raw_object, m_selected_axis_orientation, m_selected_object_pack.base_axis);
		}
	}
	else
	{
	
		float scale_factor = 0.75f / (1.0f * m_renderer_camera_inv_scale_factor); // 0.5f
		optix::float3 tang = optix::normalize(m_utility_isec_p.tangent);
		Ray m_utility_ray = Geometry::SHAPES::createMouseRay();

		optix::float3 iP;

		Plane utility_plane = m_object_transformations_manip_data.utility_plane;
		Geometry::RAY::Intersect_Plane(m_utility_ray, utility_plane, iP);

		//
		//
		optix::float3 starting_point = m_object_transformations_manip_data.starting_point;
		optix::float3 delta = iP - starting_point;
		float dot = optix::dot(delta, tang) + 0.0001f;
		float theta;
	
		float start_rot_theta = m_object_transformations_manip_data.start_rot_theta;
		UtilityPlane plane_id = m_object_transformations_manip_data.plane_id;

		theta = dot * scale_factor;
		float current_rot_theta = theta - start_rot_theta;
		optix::float3 rot = (current_rot_theta)* GLOBAL_AXIS[plane_id - 1];
		float rot_val = length(rot);
		if ( rot_val <= 1.e-4f )
			rot = ZERO_3f;

		float acc_rot = m_object_transformations_manip_data.acc_rot;
		float to_degrees = Geometry::GENERAL::toDegrees( rot_val );
		float sign = current_rot_theta >= 0.0f ? 1.0f : -1.0f; //to_degrees > m_object_transformations_manip_data.start_acc_rot ? 1.0f : -1.0f;
		float clamp_value = 0.0f;
		if ( !m_analogue_callibration )
		{
			clamp_value = 5.0f;
			if ( to_degrees < clamp_value )
				return;
			else
			{
				
				int times_rot = floor(to_degrees / clamp_value);
				rot = Geometry::GENERAL::toRadians( clamp_value ) * GLOBAL_AXIS[ plane_id-1 ] * sign * times_rot;
				m_object_transformations_manip_data.acc_rot = 0.0f;
				acc_rot    = 0.0f;
				to_degrees = 0.0f;
			}

		}
		
		if (m_repositioning_cursor) rot = ZERO_3f;
		if (!Utilities::is_object_type_Muttable_To_Transformation(m_selected_object_pack.raw_object)) rot = ZERO_3f;
		m_selected_object_pack.raw_object->setTransformation(rot, 
															 m_selected_axis_orientation, 
															 ROTATION,
															 m_hierarchy_setting,
															 0, 
															 m_selected_object_pack.base_axis);
															 //m_object_transformations_manip_data.base_axis);

		//m_selected_object_pack.raw_object->setRotation(rot);
		m_selected_object_pack.raw_object->setAttributesDelta(1, m_selected_object_pack.raw_object->getAttributesDelta(1) + rot);
		m_object_transformations_manip_data.start_rot_theta  = theta;
		m_object_transformations_manip_data.delta_length    += rot_val;
		m_object_transformations_manip_data.start_acc_rot    = to_degrees;
		m_object_transformations_manip_data.acc_rot         += (to_degrees - acc_rot);
		m_object_transformations_manip_data.delta += rot;
		m_object_transformations_manip_data.overall_delta += rot;

	}

}
void InterfaceManager::handle_SCALE()
{

	Type obj_type = m_selected_object_pack.raw_object->getType();
	AXIS_ORIENTATION axis_orientation = (obj_type == SAMPLER_OBJECT || obj_type == LIGHT_OBJECT) ? AXIS_ORIENTATION::LOCAL : m_selected_axis_orientation;

#ifdef CANCER_DEBUG
	if (m_selected_object_pack.raw_object == nullptr || m_selected_object_pack.object_index < 0)
	{
		//std::cout << "\n handle_SCALE():" << std::endl;
		//std::cout << " m_selected_object_pack.raw_object == nullptr!" << std::endl;
		//std::cout << " m_selected_object_pack.object_index < 0 " << std::endl;
		system("pause");
	}
#endif

	optix::float3 U = Mediator::RequestCameraInstance().getU();
	optix::float3 V = Mediator::RequestCameraInstance().getV();
	optix::float3 W = Mediator::RequestCameraInstance().getW();


#define SCALE_METHOD_2
#ifdef SCALE_METHOD_2

	optix::float3 * M_WORKING_AXIS = &m_selected_object_pack.base_axis[0];
	Plane utility_plane    = m_object_transformations_manip_data.utility_plane;
	UtilityPlane plane_id  = m_object_transformations_manip_data.plane_id;
	UtilityPlane plane_id1 = m_object_transformations_manip_data.plane_id1;
	UtilityPlane plane_id2 = m_object_transformations_manip_data.plane_id2;


	bool is_axis = (plane_id == X || plane_id == Y || plane_id == Z);
	float starting_delta = m_object_transformations_manip_data.starting_delta;
	
	float starting_delta1 = m_object_transformations_manip_data.starting_delta1;
	float starting_delta2 = m_object_transformations_manip_data.starting_delta2;
	float starting_delta3 = m_object_transformations_manip_data.starting_delta3;

	optix::float3 start_scale          = m_object_transformations_manip_data.start_scale;
	optix::float3 starting_plane_delta = m_object_transformations_manip_data.starting_plane_delta;

	Ray m_utility_ray = Geometry::SHAPES::createMouseRay();
	float dotRayN     = optix::dot(W, M_WORKING_AXIS[plane_id - 1]);

	optix::float3 iP; // Intersection Point
	if (Geometry::RAY::Intersect_Plane(m_utility_ray, utility_plane, iP) != 1
		|| fabs(fabs(dotRayN) - 1.0f) < 0.005f && is_axis) // EDW NA KANW CONSTRAINT KAI TO POSO MAKRIA PAEI EPISHS
	{
		mouse_data.m0_dragging = false;
		mouse_data.m0_down = false;
		m_selected_utility.active = false;
		return;
	}



	float delta, delta1, delta2;
	optix::float3 plane_delta;
	optix::float3 scale = m_selected_object_pack.raw_object->getScaleAttributes();
	float dist_from_camera = m_selected_object_pack.raw_object->getDistanceFromCamera();
	float avg_geom_spread = m_selected_object_pack.raw_object->getGeometryAverageSpread();
	float spread_factor = clamp(20.0f / avg_geom_spread , 0.0f, 1.0f);
	
	if (obj_type == LIGHT_OBJECT)       spread_factor = 2.0f;
	if (obj_type == SAMPLER_OBJECT)     spread_factor = 0.75f;
	if (obj_type == GROUP_PARENT_DUMMY) spread_factor = 0.25f;

	float scale_factor  = 0.05f * spread_factor * 20.0f / (sqrtf(dist_from_camera));


	if      (is_axis)
	{

		delta = optix::dot(M_WORKING_AXIS[plane_id - 1], iP);
		optix::float3 scale_delta = GLOBAL_AXIS[plane_id - 1] * (delta - starting_delta) * scale_factor;

		
		Type obj_type = m_selected_object_pack.raw_object->getType();
		if(!Utilities::is_object_type_SCALABLE(scale_delta, axis_orientation, m_selected_object_pack.raw_object))
			scale_delta = ZERO_3f;
		
		if (m_repositioning_cursor) 
			scale_delta = ZERO_3f;



		m_selected_object_pack.raw_object->setTransformation( scale_delta, 
															 axis_orientation,
															 SCALE, 
															 m_hierarchy_setting,
															 0,
															 m_selected_object_pack.base_axis);
		
		
		m_selected_object_pack.raw_object->setAttributesDelta(2, m_selected_object_pack.raw_object->getAttributesDelta(2) + scale_delta);
		m_object_transformations_manip_data.starting_delta = delta;
		m_object_transformations_manip_data.delta_length += fabsf(delta - starting_delta);
		m_object_transformations_manip_data.delta += scale_delta;
		m_object_transformations_manip_data.overall_delta += scale_delta;
	}
	else if (!is_axis && plane_id != UtilityPlane::XYZ) 
	{

		plane_delta = iP;
		delta1 = optix::dot(M_WORKING_AXIS[plane_id1 - 1], iP); // actual delta on selected axis
		delta2 = optix::dot(M_WORKING_AXIS[plane_id2 - 1], iP); // actual delta on selected axis
		
		float d1 = delta1 - starting_delta1;
		float d2 = delta2 - starting_delta2;
		// if i want uniform scaling
		if (fabs(d1) > fabs(d2))
			d2 = d1;
		else
			d1 = d2;


		optix::float3 dp1 = d1 * GLOBAL_AXIS[plane_id1 - 1];
		optix::float3 dp2 = d2 * GLOBAL_AXIS[plane_id2 - 1];

		optix::float3 scale_delta = scale_delta = (dp1 + dp2) * scale_factor;
		/* SCALABLE OBJECT TYPES */
		Type obj_type = m_selected_object_pack.raw_object->getType();
		//if (obj_type == LIGHT_OBJECT || obj_type == FACE_VECTOR || obj_type == SAMPLER_OBJECT)
		if (!Utilities::is_object_type_SCALABLE(scale_delta, axis_orientation, m_selected_object_pack.raw_object))
			scale_delta = ZERO_3f;
		//else
		//	scale_delta = (dp1 + dp2) * scale_factor;

		//m_selected_object_pack.raw_object->setScale( (dp1 + dp2) * scale_factor );
		if (m_repositioning_cursor) scale_delta = ZERO_3f;
		m_selected_object_pack.raw_object->setTransformation(scale_delta, 
															 axis_orientation, 
															 SCALE, 
															 m_hierarchy_setting,
															 0,
															 m_selected_object_pack.base_axis);
															 //m_object_transformations_manip_data.base_axis);

		//m_selected_object_pack.raw_object->setAttributesDelta(2, (scale - start_scale) / scale_factor);
		m_selected_object_pack.raw_object->setAttributesDelta(2, m_selected_object_pack.raw_object->getAttributesDelta(2) + scale_delta);
		m_object_transformations_manip_data.starting_plane_delta = plane_delta;
		m_object_transformations_manip_data.starting_delta1 = delta1;
		m_object_transformations_manip_data.starting_delta2 = delta2;
		m_object_transformations_manip_data.delta_length += length(dp1 + dp2);
		m_object_transformations_manip_data.delta += scale_delta;
		m_object_transformations_manip_data.overall_delta += scale_delta;
	}
	else if (plane_id == UtilityPlane::XYZ)
	{
		float speed = m_renderer_camera_inv_scale_factor * 0.001;
		plane_delta = iP;
		
		optix::float3 dp = plane_delta - m_object_transformations_manip_data.starting_point; // start_translation
		float sign = 1.0f;
		if (dp.y < 0)
			sign = -1.0f;
		
		float current_delta = optix::length(plane_delta - m_object_transformations_manip_data.starting_point); // start_translation
		float prev_delta = optix::length(starting_plane_delta - m_object_transformations_manip_data.starting_point); // start_translation

		optix::float3 scale_delta = optix::make_float3(current_delta - prev_delta)*sign * scale_factor;
		/* SCALABLE OBJECT TYPES */
		Type obj_type = m_selected_object_pack.raw_object->getType();
		//if (obj_type == LIGHT_OBJECT || obj_type == FACE_VECTOR || obj_type == SAMPLER_OBJECT)
		if (!Utilities::is_object_type_SCALABLE(scale_delta, axis_orientation, m_selected_object_pack.raw_object))
			scale_delta = ZERO_3f;
		//else
		//	scale_delta = optix::make_float3(current_delta - prev_delta)*sign * scale_factor;

		//m_selected_object_pack.raw_objec  t->setScale( optix::make_float3(current_delta - prev_delta)*sign * scale_factor );
		if (m_repositioning_cursor) scale_delta = ZERO_3f;
		m_selected_object_pack.raw_object->setTransformation(scale_delta,
															 axis_orientation, 
															 SCALE, 
															 m_hierarchy_setting,
															 0,
															 m_selected_object_pack.base_axis);
															 //m_object_transformations_manip_data.base_axis);

		m_selected_object_pack.raw_object->setAttributesDelta(2, m_selected_object_pack.raw_object->getAttributesDelta(2) + scale_delta);
		//m_selected_object_pack.raw_object->setAttributesDelta(2, (scale - start_scale) / scale_factor);
		m_object_transformations_manip_data.starting_plane_delta = plane_delta;
		m_object_transformations_manip_data.delta_length += fabsf(current_delta - prev_delta);
		m_object_transformations_manip_data.delta += scale_delta;
		m_object_transformations_manip_data.overall_delta += scale_delta;
	}

	//m_object_transformations_manip_data.delta = m_selected_object_pack.raw_object->getScaleAttributes() - m_object_transformations_manip_data.start_scale;
#endif

}

void InterfaceManager::handle_AXIS_SELECTION()
{

}
void InterfaceManager::handle_Callibration()
{


}

void InterfaceManager::handle_GROUP(std::string name )
{
	// case : Area Selection Active
	if (m_multiple_selection_active)
	{
		create_SceneGroup(m_area_selections_data[m_multiple_selection_active_id].top_level_entities,
						  m_area_selections_data[m_multiple_selection_active_id].top_level_groups,
						  m_area_selections_data[m_multiple_selection_active_id].selection_objects,
						  name);
	}

	// case : Single Selection
	else
	{
		create_SceneGroup(m_selected_object_pack.raw_object, name);
	}
}
void InterfaceManager::handle_UNGROUP()
{
	
	bool can_ungroup = false;
	if( m_selected_object_pack.raw_object != nullptr )
		if (     m_selected_object_pack.raw_object->getType() == GROUP_PARENT_DUMMY
			 && !m_selected_object_pack.raw_object->isTemporary() )
			can_ungroup = true;

	if (can_ungroup)
	{
		Action * ac = new Action_UNGROUP(m_selected_object_pack.raw_object);
		ActionManager::GetInstance().push(ac);
		resetSelectedObject(false);
	}
}

void InterfaceManager::handle_Create_Light()
{

	
	Light_Type l_type = m_interface_state.m_light_params.type;
	optix::float3 p = m_interface_state.m_light_params.position;
	optix::float3 u = m_interface_state.m_sampler_params.basis[0];
	optix::float3 v = m_interface_state.m_sampler_params.basis[1];
	optix::float3 w = m_interface_state.m_sampler_params.basis[2];
	
	bool isBBOX = m_interface_state.m_sa == SA_BBOX;
	if (m_interface_state.m_sa == SA_NORMAL || isBBOX)
	{
		optix::Matrix4x4 basis = optix::Matrix4x4::fromBasis(u, v, w, ZERO_3f);
		optix::Matrix4x4 rot   = optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(180.0f), optix::make_float3(0, 0, 1));
		basis = basis * rot;
		u = optix::make_float3(basis * optix::make_float4(GLOBAL_AXIS[0], 1.0f));
		v = optix::make_float3(basis * optix::make_float4(GLOBAL_AXIS[1], 1.0f));
		w = optix::make_float3(basis * optix::make_float4(GLOBAL_AXIS[2], 1.0f));
	}

	bool should_link            = (m_interface_state.m_sa == SA_NORMAL || isBBOX) && m_interface_state.link_to_object;
	SceneObject* focused_object = isBBOX? m_selected_object_pack.raw_object : m_focused_object_pack.raw_object;
	bool is_valid_type = false;
	if (focused_object != 0)
	{
		Type type    = focused_object->getType();
		bool is_temp = focused_object->isTemporary();
		bool is_area_selection = type == GROUP_PARENT_DUMMY && is_temp;
		bool is_fv    = type == FACE_VECTOR;
		bool is_smplr = type == SAMPLER_OBJECT;
		if (type == IMPORTED_OBJECT || (type == GROUP_PARENT_DUMMY && !is_temp))
			is_valid_type = true;
	}
	should_link &= is_valid_type;
	optix::float3 basis[3] = { u, v, w };

	SAMPLER_ALIGNEMENT sa = isBBOX ? SA_NORMAL : m_interface_state.m_sa;
	Mediator::addSceneLight(l_type, p, basis, should_link, sa);
	
}

void InterfaceManager::handle_Attach_Sampler_Planar()
{
	// sampler's position
	optix::float3 position = m_interface_state.m_sampler_params.origin;
	// sampler's orientation
	optix::float3 basis[3];
	basis[0] = m_interface_state.m_sampler_params.basis[0];
	basis[1] = m_interface_state.m_sampler_params.basis[1];
	basis[2] = m_interface_state.m_sampler_params.basis[2];
	// sampler's scale
	optix::float3 scale = m_interface_state.m_sampler_params.scale;

	// selected object
	SceneObject* object = m_selected_object_pack.raw_object;
	
	////std::cout << "\n - pos : [ " << position.x << "," << position.y << " , " << position.z << " ] " << std::endl;
	////std::cout << " - scale : [ " << scale.x << " , " << scale.y << " , " << scale.z << " ] " << std::endl;
	////std::cout << " - basis[0] : [ " << basis[0].x << " , " << basis[0].y << " , " << basis[0].z << " ] " << std::endl;
	////std::cout << " - basis[1] : [ " << basis[1].x << " , " << basis[1].y << " , " << basis[1].z << " ] " << std::endl;
	////std::cout << " - basis[2] : [ " << basis[2].x << " , " << basis[2].y << " , " << basis[2].z << " ] " << std::endl;
	////std::cout << " - object : " << (object != nullptr) << std::endl;

	Sampler_Parameters params;
	params.basis[0] = basis[0];params.basis[1] = basis[1];params.basis[2] = basis[2];
	params.origin = position;
	params.scale  = scale * 0.1f;
	params.type   = s_PLANE;

	object->attach_Sampler( params );
	//object->attach_Sampler(s_PLANE);
}
void InterfaceManager::handle_Attach_Sampler_Volume()
{
	// sampler's position
	optix::float3 position = m_interface_state.m_sampler_params.origin;
	// sampler's orientation
	optix::float3 basis[3];
	basis[0] = m_interface_state.m_sampler_params.basis[0];
	basis[1] = m_interface_state.m_sampler_params.basis[1];
	basis[2] = m_interface_state.m_sampler_params.basis[2];
	// sampler's scale
	optix::float3 scale = m_interface_state.m_sampler_params.scale;

	// selected object
	SceneObject* object = m_selected_object_pack.raw_object;

	////std::cout << "\n - pos : [ " << position.x << "," << position.y << " , " << position.z << " ] " << std::endl;
	////std::cout << " - scale : [ " << scale.x << " , " << scale.y << " , " << scale.z << " ] " << std::endl;
	////std::cout << " - basis[0] : [ " << basis[0].x << " , " << basis[0].y << " , " << basis[0].z << " ] " << std::endl;
	////std::cout << " - basis[1] : [ " << basis[1].x << " , " << basis[1].y << " , " << basis[1].z << " ] " << std::endl;
	////std::cout << " - basis[2] : [ " << basis[2].x << " , " << basis[2].y << " , " << basis[2].z << " ] " << std::endl;
	////std::cout << " - object : " << (object != nullptr) << std::endl;

	Sampler_Parameters params;
	params.basis[0] = basis[0];params.basis[1] = basis[1];params.basis[2] = basis[2];
	params.origin = position;
	params.scale = scale * 0.1f;
	params.type = s_VOLUME;

	object->attach_Sampler(params);
}
void InterfaceManager::handle_Attach_Sampler_Directional()
{
	// sampler's position
	optix::float3 position = m_interface_state.m_sampler_params.origin;
	// sampler's orientation
	optix::float3 basis[3];
	basis[0] = m_interface_state.m_sampler_params.basis[0];
	basis[1] = m_interface_state.m_sampler_params.basis[1];
	basis[2] = m_interface_state.m_sampler_params.basis[2];
	// sampler's scale
	optix::float3 scale = m_interface_state.m_sampler_params.scale;

	// selected object
	SceneObject* object = m_selected_object_pack.raw_object;

	////std::cout << "\n - pos : [ " << position.x << "," << position.y << " , " << position.z << " ] " << std::endl;
	////std::cout << " - scale : [ " << scale.x << " , " << scale.y << " , " << scale.z << " ] " << std::endl;
	////std::cout << " - basis[0] : [ " << basis[0].x << " , " << basis[0].y << " , " << basis[0].z << " ] " << std::endl;
	////std::cout << " - basis[1] : [ " << basis[1].x << " , " << basis[1].y << " , " << basis[1].z << " ] " << std::endl;
	////std::cout << " - basis[2] : [ " << basis[2].x << " , " << basis[2].y << " , " << basis[2].z << " ] " << std::endl;
	////std::cout << " - object : " << (object != nullptr) << std::endl;

	Sampler_Parameters params;
	params.basis[0] = basis[0];params.basis[1] = basis[1];params.basis[2] = basis[2];
	params.origin = position;
	params.scale = scale * 0.1f;
	params.type = s_DIRECTIONAL;

	object->attach_Sampler(params);
}

Restriction*  InterfaceManager::handle_Attach_Restriction_Focus()
{
	return handle_Attach_Restriction_Focus_ObjToObj();
	// [ focused ]   [ selected ] //
	// [ -> ] ---> [ object ] //

	SceneObject* from = m_focused_object_pack.raw_object;   // face_vector //
	SceneObject* to   = m_selected_object_pack.raw_object;  // object // 

	
	bool is_valid[2] = { false,false };
	is_valid[0] = from == nullptr ? false : true;
	is_valid[1] = to   == nullptr ? false : true;

	if (!is_valid[0] || !is_valid[1])
		return nullptr;

	Type obj_types[2];
	obj_types[0] = from->getType();
	obj_types[1] = to->getType();
	
	// swap objects //
	if (obj_types[0] != FACE_VECTOR && obj_types[1] == FACE_VECTOR)
	{
		SceneObject* temp = from;
		from = to;
		to   = temp;

		Type t_type  = obj_types[0];
		obj_types[0] = obj_types[1];
		obj_types[1] = t_type;
	}

	// from //
	is_valid[0] = (obj_types[0] == FACE_VECTOR); // from : [ FACE_VECTOR ]
	is_valid[1] = (obj_types[1] != FACE_VECTOR && obj_types[1] != SAMPLER_OBJECT); // to   : [ OBJECT ]

	// check if from,to can be linked to form a new Restriction //
	// 1. facevector is already part of another Restriction //
	// 2. from-to belong to same Parent or are the same object //
	bool is_R_Legit = true;
	if(is_valid[0] && is_valid[1])
	{
		// bool check_state0
		bool check_state0 = true;
		for (Restriction* R : Mediator::Request_Scene_Restrictions())
		{
			if (!R->is_Active()) 
				continue;

			Restriction_ID r_id = R->getId();
			SceneObject* A = R->get_Owner(0);
			SceneObject* B = R->get_Owner(1);
			SceneObject* a = R->getSubject();
			SceneObject* b = R->getObject();

			int Aid = A->getId();
			int Bid = B->getId();
			int aid = a->getId();
			int bid = b->getId();

			if (r_id == FOCUS_R)
			{
				int FromID = from->getParent()->getId();
				int toID   = to->getId();
				if (   (FromID == Aid && toID == Bid)
					|| (FromID == Bid && toID == Aid))
				{
					check_state0 = false;
					break;
				}
				
			}
			
		}
		is_R_Legit = check_state0;


		
		// check if [to] is parent of [from]
		if (is_R_Legit)
		{

			SceneObject* from_p = from->getParent();
			int p_id = from_p == nullptr ? -1 : from_p->getId();
			if (p_id == to->getId())
				is_R_Legit = false;
		}
		
		
	}
	if (!is_R_Legit)
		return nullptr;

	

	// create Restriction //
	if (is_valid[0] && is_valid[1])
	{
		SceneObject* Owner_A = from->getParent();
		SceneObject* Owner_B = to;

		Restriction * restriction = new Restriction_Focus(from, to);
		restriction->set_Owner(0, Owner_A); // facevector's parent 
		restriction->set_Owner(1, Owner_B); // object 
		restriction->set_DeltaLimit(optix::make_float2(25.0f));
		restriction->set_DeltaLimit_2(optix::make_float2(25.0f));

		
		Mediator::add_Restriction( restriction );
		Action * ac = new Action_CRT_DLT_RESTRICTION( restriction, 1 );
		ActionManager::GetInstance().push( ac );
		return restriction;
	}
	return nullptr;

}
Restriction*  InterfaceManager::handle_Attach_Restriction_Conversation()
{
	return handle_Attach_Restriction_Conversation_ObjToObj();
	// [ focused ]   [ selected ] //
	// [ -> ] ---> [ object ] //

	SceneObject* from = m_focused_object_pack.raw_object;   // face_vector //
	SceneObject* to = m_selected_object_pack.raw_object;  // object // 

	bool is_valid[2] = { false,false };
	is_valid[0] = from == nullptr ? false : true;
	is_valid[1] = to == nullptr ? false : true;

	if (!is_valid[0] || !is_valid[1])
		return nullptr;

	Type obj_types[2];
	obj_types[0] = from->getType();
	obj_types[1] = to->getType();

	// from //
	is_valid[0] = (obj_types[0] == FACE_VECTOR); // from : [ FACE_VECTOR ]
	is_valid[1] = (obj_types[1] == FACE_VECTOR); // to   : [ OBJECT ]

	// check if from,to can be linked to form a new Restriction //
	// 1. facevector is already part of another Restriction //
	// 2. from-to belong to same Parent or are the same object //
	bool is_R_Legit = true;
	if(is_valid[0] && is_valid[1])
	{


		// bool check_state0
		bool check_state0 = true;
		for (Restriction* R : Mediator::Request_Scene_Restrictions())
		{
			if (!R->is_Active())
				continue;

			Restriction_ID r_id = R->getId();
			SceneObject* A = R->get_Owner(0);
			SceneObject* B = R->get_Owner(1);
			SceneObject* a = R->getSubject();
			SceneObject* b = R->getObject();

			int Aid = A->getId();
			int Bid = B->getId();
			int aid = a->getId();
			int bid = b->getId();

			if (r_id == CONVERSATION_R)
			{
				int FromID = from->getParent()->getId();
				int toID   = to->getParent()->getId();
				if ((FromID == Aid && toID == Bid)
					|| (FromID == Bid && toID == Aid))
				{
					check_state0 = false;
					break;
				}

			}

		}
		is_R_Legit = check_state0;

		
		if (is_R_Legit)
		{
			// check if [to] is parent of [from]
			SceneObject* from_p = from->getParent();
			int p_id = from_p == nullptr ? -1 : from_p->getId();
			if (p_id == to->getId())
				is_R_Legit = false;
			// check if [from] is parent of [to]
			SceneObject* to_p = to->getParent();
			int p_id2 = to_p == nullptr ? -1 : to_p->getId();
			if (p_id2 == from->getId())
				is_R_Legit = false;

			// check if [to] has the same parent iwth [from]
			if (p_id == p_id2 && (p_id != -1))
				is_R_Legit = false;

		}
	}
	if (!is_R_Legit)
		return nullptr;


	// create Restriction //
	if (is_valid[0] && is_valid[1])
	{
		SceneObject* Owner_A = from->getParent();
		SceneObject* Owner_B = to->getParent();

		Restriction * restriction = new Restriction_Conversation(from, to);
		restriction->set_Owner(0, Owner_A); // facevector's parent //
		restriction->set_Owner(1, Owner_B);          
		
		restriction->set_DeltaLimit(optix::make_float2(25.0f));
		restriction->set_DeltaLimit_2(optix::make_float2(25.0f));
		restriction->set_DistLimit(optix::make_float2(0.0f, length(Owner_A->getCentroid_Transformed() - Owner_B->getCentroid_Transformed())));

		//Owner_A->push_Restriction(restriction);
		//Owner_B->push_Restriction(restriction);
		Mediator::add_Restriction( restriction );// object //
		Action * ac = new Action_CRT_DLT_RESTRICTION( restriction, 1 );
		ActionManager::GetInstance().push( ac );
		return restriction;
	}
	return nullptr;

}
Restriction*  InterfaceManager::handle_Attach_Restriction_Distance()
{


	SceneObject* from = m_focused_object_pack.raw_object;   // face_vector //
	SceneObject* to   = m_selected_object_pack.raw_object;  // object // 

	bool is_valid[2] = { false,false };
	is_valid[0] = from == nullptr ? false : true;
	is_valid[1] = to   == nullptr ? false : true;

	if (!is_valid[0] || !is_valid[1])
		return nullptr;

	Type obj_types[2];
	obj_types[0] = from->getType();
	obj_types[1] = to->getType();

	// from //
	is_valid[0] = (obj_types[0] != FACE_VECTOR && obj_types[0] != SAMPLER_OBJECT); // from : [ OBJECT ]
	is_valid[1] = (obj_types[1] != FACE_VECTOR && obj_types[1] != SAMPLER_OBJECT); // to   : [ OBJECT ]


	// check if from,to can be linked to form a new Restriction //
	// 1. facevector is already part of another Restriction //
	// 2. from-to belong to same Parent or are the same object //
	bool is_R_Legit = true;
	if(is_valid[0] && is_valid[1])
	{


		for (Restriction* R : Mediator::Request_Scene_Restrictions())
		{
			if (!R->is_Active()) 
				continue;

			Restriction_ID r_id = R->getId();
			SceneObject* A = R->get_Owner(0);
			SceneObject* B = R->get_Owner(1);
			SceneObject* a = R->getSubject();
			SceneObject* b = R->getObject();

			int Aid = A->getId();
			int Bid = B->getId();
			int aid = a->getId();
			int bid = b->getId();

			
			if (r_id == DISTANCE_R)
			{
				if (   (Aid == from->getId() && Bid == to->getId())
					|| (Aid == to->getId() && Bid == from->getId())
					)
				{
					is_R_Legit = false;
					break;
				}
			}
		}


	}
	if (!is_R_Legit)
		return nullptr;

	// create Restriction //
	if (is_valid[0] && is_valid[1])
	{
		SceneObject* Owner_A = from;
		SceneObject* Owner_B = to;

		Restriction * restriction = new Restriction_Distance(from, to);
		restriction->set_Owner(0, Owner_A); // facevector's parent //
		restriction->set_Owner(1, Owner_B);    
		optix::float2 d = optix::make_float2(0.0f, length(from->getCentroid_Transformed() - to->getCentroid_Transformed()));
		restriction->set_DistLimit(d);
		
		Mediator::add_Restriction( restriction );
		Action * ac = new Action_CRT_DLT_RESTRICTION(restriction, 1);
		ActionManager::GetInstance().push( ac );
		return restriction;
	}
	return nullptr;

	// create Restriction with object, subject, set ownerA, ownerB
	// push pointer to same Restriction on both owners A,B
	// Both objects will poin to same Restriction

}
Restriction* InterfaceManager::handle_Attach_Restriction_Focus_ObjToObj()
{
	// [ focused ]   [ selected ] //
	// [ -> ] ---> [ object ] //

	SceneObject* from = m_focused_object_pack.raw_object;   // face_vector //
	SceneObject* to   = m_selected_object_pack.raw_object;  // object // 

	if (from->getType() == SAMPLER_OBJECT || to->getType() == SAMPLER_OBJECT) return nullptr;
	if (from->getType() == FACE_VECTOR) from = from->getParent();
	if (to->getType() == FACE_VECTOR)   to   = to->getParent();
	if (from->getId() == to->getId()) return nullptr;
	
	
	bool hasFaceVectorsA = false;
	SceneObject* fva = 0;
	for(SceneObject* fv : from->get_Face_Vectors())
		if (fv->isActive()) { hasFaceVectorsA = true; fva = fv; break; }
	if (!hasFaceVectorsA) return nullptr;

	bool is_R_Legit = true;
	{
		// bool check_state0
		bool check_state0 = true;
		for (Restriction* R : Mediator::Request_Scene_Restrictions())
		{
			if (!R->is_Active())
				continue;

			Restriction_ID r_id = R->getId();
			SceneObject* A = R->get_Owner(0);
			SceneObject* B = R->get_Owner(1);
			
			int Aid = A->getId();
			int Bid = B->getId();
		
			if (r_id == FOCUS_R)
			{
				int FromID = from->getId();
				int toID   = to->getId();
				if (   (FromID == Aid && toID == Bid)
					|| (FromID == Bid && toID == Aid))
				{
					check_state0 = false;
					break;
				}

			}

		}
		is_R_Legit = check_state0;



	}
	if (!is_R_Legit)
		return nullptr;



	// create Restriction //
	{
		SceneObject* Owner_A = from;
		SceneObject* Owner_B = to;

		Restriction * restriction = new Restriction_Focus(fva, to);
		restriction->set_Owner(0, Owner_A); // facevector's parent 
		restriction->set_Owner(1, Owner_B); // object 
		restriction->set_DeltaLimit(optix::make_float2(25.0f));
		restriction->set_DeltaLimit_2(optix::make_float2(25.0f));
		restriction->checkForValidState();

		Mediator::add_Restriction(restriction);
		Action * ac = new Action_CRT_DLT_RESTRICTION(restriction, 1);
		ActionManager::GetInstance().push(ac);
		return restriction;
	}
	return nullptr;
}
Restriction* InterfaceManager::handle_Attach_Restriction_Conversation_ObjToObj()
{
	// [ focused ]   [ selected ] //
	// [ -> ] ---> [ object ] //

	SceneObject* from = m_focused_object_pack.raw_object;   // face_vector //
	SceneObject* to   = m_selected_object_pack.raw_object;  // object // 

	if (from->getType() == SAMPLER_OBJECT || to->getType() == SAMPLER_OBJECT) return nullptr;
	if (from->getType() == FACE_VECTOR) from = from->getParent();
	if (to->getType() == FACE_VECTOR)   to   = to->getParent();
	if (from->getId() == to->getId()) return nullptr;


	bool hasFaceVectorsA = false;
	bool hasFaceVectorsB = false;
	SceneObject* fva = 0;
	SceneObject* fvb = 0;
	for (SceneObject* fv : from->get_Face_Vectors())
		if (fv->isActive()) { hasFaceVectorsA = true; fva = fv; break; }
	for (SceneObject* fv : to->get_Face_Vectors())
		if (fv->isActive()) { hasFaceVectorsB = true; fvb = fv; break; }
	if (!hasFaceVectorsA) return nullptr;
	if (!hasFaceVectorsB) return nullptr;

	bool is_R_Legit = true;
	{
		// bool check_state0
		bool check_state0 = true;
		for (Restriction* R : Mediator::Request_Scene_Restrictions())
		{
			if (!R->is_Active())
				continue;

			Restriction_ID r_id = R->getId();
			SceneObject* A = R->get_Owner(0);
			SceneObject* B = R->get_Owner(1);

			int Aid = A->getId();
			int Bid = B->getId();

			if (r_id == CONVERSATION_R)
			{
				int FromID = from->getId();
				int toID   = to->getId();
				if (   (FromID == Aid && toID == Bid)
					|| (FromID == Bid && toID == Aid))
				{
					check_state0 = false;
					break;
				}

			}

		}
		is_R_Legit = check_state0;



	}
	if (!is_R_Legit)
		return nullptr;



	// create Restriction //
	{
		SceneObject* Owner_A = from;
		SceneObject* Owner_B = to;

		Restriction * restriction = new Restriction_Conversation(fva, fvb);
		restriction->set_Owner(0, Owner_A); // facevector's parent 
		restriction->set_Owner(1, Owner_B); // object 
		restriction->set_DeltaLimit(optix::make_float2(25.0f));
		restriction->set_DeltaLimit_2(optix::make_float2(25.0f));
		restriction->set_DistLimit(optix::make_float2(0.0f, length(Owner_A->getCentroid_Transformed() - Owner_B->getCentroid_Transformed())));
		restriction->checkForValidState();

		Mediator::add_Restriction(restriction);
		Action * ac = new Action_CRT_DLT_RESTRICTION(restriction, 1);
		ActionManager::GetInstance().push(ac);
		return restriction;
	}

	return nullptr;
}

void InterfaceManager::handle_Object_Align()
{

	m_popup_object_align_toggled = true;
	m_popup_object_align_window = true;

	m_align_object = m_selected_object_pack.raw_object;
	m_align_target = m_focused_object_pack.raw_object;

	m_align_object_data.reset();
	optix::float3 * bbox = m_align_object->getBBOX_Transformed(true);
	m_align_object_data.start_translation = m_align_object->getTranslationAttributes();
	m_align_object_data.start_bbox[0]     = bbox[0]; 
	m_align_object_data.start_bbox[1]     = bbox[1];
	m_align_object_data.start_centroid    = m_align_object->getCentroid_Transformed();
	m_align_object_data.start_rot_mat     = m_align_object->getRotationMatrix_chain();
	delete bbox;
}
void InterfaceManager::handle_Camera_Align()
{
	//std::cout << "\n - handle_Camera_Align():" << std::endl;
	
	
	bool is_normal_dir_obj = m_camera_align_object.is_object;
	bool is_normal_dir_trg = m_camera_align_target.is_object;
	
	SceneObject* temp_created_object = 0;
	SceneObject* temp_created_target = 0;

	SceneObject* object = 0;
	SceneObject* target = 0;

	// temporarily create face vector representing Object's Normal Dir
	if (is_normal_dir_obj)
	{
		SceneObject* objParent  = m_camera_align_object.object;
		SceneObject* created_fv = objParent->attach_FaceVector( m_camera_align_object.origin, m_camera_align_object.dir, false);
		//created_fv->setName("obj_temp_created_fv");
		created_fv->set_as_Temp_Clone(true);
		m_camera_align_object.object = created_fv;
		temp_created_object = created_fv;
	}
	
	// temporarily create face vector representing Target's Noirmal Dir
	if (is_normal_dir_trg)
	{
		SceneObject* trgParent  = m_camera_align_target.object;
		SceneObject* created_fv = trgParent->attach_FaceVector(m_camera_align_target.origin, m_camera_align_target.dir, false);
		//created_fv->setName("trg_temp_created_fv");
		created_fv->set_as_Temp_Clone(true);
		m_camera_align_target.object = created_fv;
		temp_created_target = created_fv;
	}
	
	//
	object = m_camera_align_object.object;
	target = m_camera_align_target.object;


	// get Alignement Vectors [ A -> B ]
	optix::float3 obj_dir;
	optix::float3 trg_dir;


	obj_dir =  optix::make_float3(object->getRotationMatrix_chain() * optix::make_float4(GLOBAL_AXIS[2], 0.0f));
	trg_dir = -optix::make_float3(target->getRotationMatrix_chain() * optix::make_float4(GLOBAL_AXIS[2], 0.0f)); 

	// check if axis already aligned //
	float ddot = optix::dot(obj_dir, trg_dir);
	float d    = 1 - ddot;
	//std::cout << " - d : " << d << std::endl;
	bool is_already_aligned =  fabsf(d) < 1e-5f;


	// Create Orthogonal Bases aligned with Alignement vectors
	optix::float3 obj_axis[3];   BASE_TRANSFORM::AXIS(object, LOCAL, obj_axis);
	optix::float3 old_Y_axis = obj_axis[1];
	optix::float3 obj_axis_1[3]; for (int i = 0; i < 3; i++) obj_axis_1[i] = obj_axis[i];
	optix::float3 obj_axis_2[3]; for (int i = 0; i < 3; i++) obj_axis_2[i] = obj_axis[i];

	optix::float3 trg_axis[3];  BASE_TRANSFORM::AXIS(target, LOCAL, trg_axis);
	optix::float3 trg_axis_1[3];
	optix::float3 trg_axis_2[3];
	optix::Matrix4x4 rot_mat   = target->getRotationMatrix_chain();
	optix::Matrix4x4 rot_mat_1 = rot_mat * optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(180.0f), GLOBAL_AXIS[0]);
	optix::Matrix4x4 rot_mat_2 = rot_mat * optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(180.0f), GLOBAL_AXIS[1]);
	for (int i = 0; i < 3; i++) { trg_axis_1[i] = optix::make_float3( rot_mat_1 * optix::make_float4(GLOBAL_AXIS[i], 0)); }
	for (int i = 0; i < 3; i++) { trg_axis_2[i] = optix::make_float3( rot_mat_2 * optix::make_float4(GLOBAL_AXIS[i], 0)); }

	

	// get Alignement OnB
	bool ortho1, ortho2, portho1, portho2;
	ortho1 = portho1 = ortho2 = portho2 = false;
	optix::Matrix4x4 align_onb1 = MATRIX_TRANSFORM::Align_axis_Transformation_Get(obj_axis_1, trg_axis_1, 2, &ortho1, &portho1);
	optix::Matrix4x4 align_onb2 = MATRIX_TRANSFORM::Align_axis_Transformation_Get(obj_axis_2, trg_axis_2, 2, &ortho2, &portho2);
	
	optix::Matrix4x4 align_onb;
	if     (ortho1)  align_onb = align_onb1;
	else if(ortho2)  align_onb = align_onb2;
	else if(portho1) align_onb = align_onb1;
	else if(portho2) align_onb = align_onb2;
	else align_onb = align_onb1;

	
	for(int i=0; i<3; i++) align_onb.setCol(i, align_onb.getCol(i) * -1.0f);
	optix::Matrix4x4 align_rot = 
		 align_onb 
		* object->getRotationMatrix_chain().inverse();



	SceneObject* pObj = object->getParent();
	SceneObject* pTrg = target->getParent();
	//

	
	// Rotation Transformation
	optix::Matrix4x4 post_align_rot   = optix::Matrix4x4::identity();
	optix::Matrix4x4 post_Y_align_rot = optix::Matrix4x4::identity();
	if (!is_already_aligned)
	{
		pObj->setTransformation(align_rot, WORLD, ROTATION, m_hierarchy_setting, 0);

		
		optix::float3 axis[3]; BASE_TRANSFORM::AXIS(object, LOCAL, axis);
		int axis_index = align_onb == align_onb1 ? 1 : 0;
		post_align_rot = optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(180.0f), axis[axis_index]);
		pObj->setTransformation(post_align_rot, WORLD, ROTATION, m_hierarchy_setting);

		BASE_TRANSFORM::AXIS(object, LOCAL, axis);
		optix::float3 new_Y_axis = axis[1];
		float dotY = optix::dot(old_Y_axis, new_Y_axis);
		if (dotY < 0)
		{
			//std::cout << " - flip y axis !" << std::endl;
			optix::float3 new_obj_dir = optix::make_float3(object->getRotationMatrix_chain() * optix::make_float4(GLOBAL_AXIS[2], 0.0f));
			
			post_Y_align_rot = optix::Matrix4x4::rotate(Geometry::GENERAL::toRadians(180.0f), normalize( new_obj_dir ) );
		    pObj->setTransformation(post_Y_align_rot, WORLD, ROTATION, m_hierarchy_setting);
		}
	}

	// Translation Transformation
	optix::float3 obj_pos  = object->getTranslationAttributes();
	optix::float3 trg_pos  = target->getTranslationAttributes();
	optix::float3 pObj_pos = object->getParent()->getTranslationAttributes();
	optix::float3 pTrg_pos = target->getParent()->getTranslationAttributes();
	optix::float3 pObj_centroid  = object->getParent()->getCentroid_Transformed();
	optix::float3 trg_obj_delta  = (trg_pos - obj_pos);
	optix::float3 trg_pObj_delta = (trg_pos - pObj_pos);
	optix::float3 pObj_obj_delta = (obj_pos - pObj_pos);
	optix::float3 new_pos   = trg_pos - normalize(trg_dir) * length(trg_obj_delta) - pObj_obj_delta;
	optix::float3 pos_delta = new_pos - pObj_pos;
	pObj->setTransformation(pos_delta, WORLD, TRANSLATION, m_hierarchy_setting);

	if (!is_already_aligned)
	{
		Action * ac = new Action_TRANSFORMATION(pObj, align_rot, obj_axis, WORLD, m_hierarchy_setting, ACTION_ID::ACTION_TRANSFORMATION_ROTATION_MAT);
		ac->set_Stacked_Init(true);
		ac->set_Stacked(true);

		Action * ac1 = new Action_TRANSFORMATION(pObj, post_align_rot, obj_axis, WORLD, m_hierarchy_setting, ACTION_ID::ACTION_TRANSFORMATION_ROTATION_MAT);
		ac1->set_Stacked(true);

		Action * ac2 = new Action_TRANSFORMATION(pObj, post_Y_align_rot, obj_axis, WORLD, m_hierarchy_setting, ACTION_ID::ACTION_TRANSFORMATION_ROTATION_MAT);
		ac2->set_Stacked(true);

		Action * ac3 = new Action_TRANSFORMATION(pObj, pos_delta, obj_axis, WORLD, m_hierarchy_setting, ACTION_ID::ACTION_TRANSFORMATION_TRANSLATION);
		ac3->set_Stacked_End(true);
		ac3->set_Stacked(true);

		ActionManager::GetInstance().push(ac);
		ActionManager::GetInstance().push(ac1);
		ActionManager::GetInstance().push(ac2);
		ActionManager::GetInstance().push(ac3);
	}
	else if(optix::length(pos_delta) > 1e-5f)
	{
		//std::cout << " - already Aligned!" << std::endl;
		//std::cout << " - pos_delta : " << length(pos_delta) << std::endl;
		Action * ac2 = new Action_TRANSFORMATION(pObj, pos_delta, obj_axis, WORLD, m_hierarchy_setting, ACTION_ID::ACTION_TRANSFORMATION_TRANSLATION);
		ActionManager::GetInstance().push(ac2);
	}

	// checking 
	{

	}
	

	if (temp_created_object != 0)
	{
		//std::cout << " ~ removing temp created object[ "<< temp_created_object->getId()<<" ]!" << std::endl;

		Action * ac = new Action_CRT_DLT_FACE_VECTOR(temp_created_object, temp_created_object->getParent(), 1, true);
		ac->undo();
		delete ac;
	}
	if (temp_created_target != 0)
	{
		//std::cout << " ~ removing temp created target[ " << temp_created_target->getId() << " ]!" << std::endl;
		Action * ac = new Action_CRT_DLT_FACE_VECTOR(temp_created_target, temp_created_target->getParent(), 1, true);
		ac->undo();
		delete ac;
	}


	m_camera_align_object.reset();
	m_camera_align_target.reset();
	
}


#endif


