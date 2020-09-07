#pragma once

#include <string>
#include "common_structs.h"
#include "Interface_Structs.h"
#include "Utility_Renderer_Defines.h"
#include "ViewportManager_structs.h"
#include <vector>


std::string AxisOrientationToString(enum AXIS_ORIENTATION orientation);
std::vector<int> getSelectedObjectsFromArea(Rect2D rect, class PinholeCamera* cam_instance);
std::vector<SceneObject*> getSelectedObjectsFromFrustum(Rect2D rect, class PinholeCamera* cam_instance);
std::vector<SceneObject*> getSelectedObjectsFromFrustum(Rect2D rect, const VIEWPORT& viewport);



int getArea_Selection_DataIndex_FromParent(int id);