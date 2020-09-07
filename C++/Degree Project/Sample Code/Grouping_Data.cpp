#include "Grouping_Data.h"


void Grouping_Data::set_Data(Grouping_Data& data)
{
	for (SceneObject* obj : data.top_level_objects)
		top_level_objects.push_back(obj);
	for (SceneObject* obj : data.top_level_groups)
		top_level_groups.push_back(obj);
	for (SceneObject* obj : data.group_selection)
		group_selection.push_back(obj);
	group_name = data.group_name;
}

void Grouping_Data::clear()
{

	top_level_objects.clear();
	top_level_groups.clear();
	group_selection.clear();

	

}