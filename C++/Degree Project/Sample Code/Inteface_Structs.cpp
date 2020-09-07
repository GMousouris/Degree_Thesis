#include "Interface_Structs.h"
#include "InterfaceManager.h"
#include "InterfaceManager_HelperFunctions.h"
#include "AssetManager.h"
#include "Utilities.h"


#ifdef UI_BUTTON

Button::Button()
	:
	focused(false),
	selected(false),
	pressed(false),
	label(""),
	icon(nullptr),
	icon_id(""),
	size( BUTTON_SIZE ),
	ratio( BUTTON_RATIO ),
	padding( BUTTON_PADDING ),
	bg_color( BUTTON_COLOR_BG )
{

}

Button::Button(std::string label,
			   Utility ut,
			   ImVec2 size,
			   ImVec2 ratio,
			   float padding,
			   ImVec4 bg_color)

	:
	focused(false),
	selected(false),
	pressed(false),
	label(label),
	utility(ut),
	size(size),
	ratio(ratio),
	padding(padding),
	bg_color(bg_color)

{
	
	icon_id = label;
	icon = AssetManager::GetInstance().GetTexture(icon_id.c_str());
	ut_group = Utilities::get_UtilityGroup(utility);


}
Button::Button(std::string label, 
			   Utility ut,
			   GLuint * icon, 
			   ImVec2 size, 
			   ImVec2 ratio, 
			   float padding, 
			   ImVec4 bg_color)
	:
	focused(false),
	selected(false),
	pressed(false),
	label(label),
	utility(utility),
	icon(icon),
	icon_id(label),
	size(size),
	ratio(ratio),
	padding(padding),
	bg_color(bg_color)

{

}

void Button::construct()
{
	
	updateIcon();
	ImGui::ImageButton((void*)*icon, size, ImVec2(0.0f, 0.0f), ratio, padding, bg_color);
	bool is_clicked = ImGui::IsItemClicked();

	if (ImGui::IsItemHovered())
	{
		if (!timer.isRunning())
		{
			timer.start();
			focused_timer = timer.getTime();
		}
		else
		{
			float delta = timer.getTime() - focused_timer;
			if(delta >= 0.3f)
				InterfaceManager::GetInstance().popup_Tooltip(ImGui::GetCursorPos(), tooltip);
		}

		setFocused(true);
		if (is_clicked)
			setPressed(true);
	}
	else
	{
		setFocused(false);
		timer.stop();
	}

	
	if (label == "undo" || label == "redo")
	{
		if (focused && is_clicked)
		{
			toggle();
		}
	}
	else if (label == "scene_toggle_lights")
	{
		if (focused && is_clicked)
		{
			bool is_valid = false;
			toggle(&is_valid);
			selected = !selected;
		}
	}
	else if ( focused && pressed  )
	{
		bool is_valid = false;
		toggle(&is_valid);
		if (is_valid)
		{
			setSelected(true);
		}
	}

	if (ImGui::IsMouseReleased(0) && pressed)
		setPressed(false);

	// special cases for Utilities like : { undo, redo, unlink, etc.. } that we dont want them to stay pressed
	if (utility == u_UNLINK)
	{
		if (ImGui::IsMouseReleased(0))
			setSelected(false);
	}

	
}
void Button::toggle(bool * is_valid)
{
	InterfaceManager::GetInstance().toggle(utility,true,false, is_valid);
}
void Button::setFocused(bool state)
{
	focused = state;
}
void Button::setSelected(bool state)
{
	selected = state;
	if (selected)
		InterfaceManager::GetInstance().Update_group_button_state(ut_group, utility);
}
void Button::setPressed(bool state) 
{ 
	pressed = state;
}
void Button::updateIcon()
{
	
	std::string tex;

		

	if (pressed || (focused && selected))
		tex = label + "_selected_focused";
	else if (focused)
		tex = label + "_focused";
	else if (selected)
		tex = label + "_selected";
	else
		tex = label;

	icon = (AssetManager::GetInstance().GetTexture(tex.c_str()));
}

#endif