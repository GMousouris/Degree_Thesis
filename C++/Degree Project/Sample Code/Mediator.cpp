#include "streamHandler.h"
#include "OptiXRenderer.h"
#include "InterfaceManager.h"
#include "Defines.h"
#include "Restrictions.h";


#ifdef OPENGL_RENDERER_OLD
//#include "OpenGLRenderer.h"
#endif

#include "OpenGL_Renderer.h"

#include "OptiXSceneEditor.h"
#include "SceneManager.h"
#include "SceneObject.h"
#include "ViewportManager.h"

#ifdef GUI_MANAGER
#include "guiManager.h"
#endif


#include <GLFW/glfw3.h>

#include "Mediator.h"

#include <iostream>



OptiXSceneEditor * Mediator::main_app = 0;

//------------------------------------------------------------------------------
//
// Init Functions
//
//------------------------------------------------------------------------------

void Mediator::Init( OptiXSceneEditor * app )
{
#if defined( DEBUGGING ) || defined(INIT_REPORT)
	streamHandler::Report("Mediator", "Init", "");
#endif

	main_app = app;

}

//------------------------------------------------------------------------------
//
// OptiXRenderer Functions
//
//------------------------------------------------------------------------------

GLuint Mediator::Request_Optix_DepthBuffer()
{
	return Renderer().get_Optix_DepthBuffer();
}

PinholeCamera& Mediator::RequestCameraInstance()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestCameraInstance()" << std::endl;
#endif
	return ViewportManager::GetInstance().getActiveViewportCamera();
	//return Renderer().getCameraInstance();
}

optix::Context& Mediator::RequestContext()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestContext()" << std::endl;
#endif
	return Renderer().getContext();
}

GLuint& Mediator::RequestOpenGLShaderProgram()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestOpenGLShaderProgram()" << std::endl;
#endif
	return Renderer().getOpenGLShaderProgram();
}

PostProcessParameters& Mediator::RequestPostProcessParameters()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestPostProcessParameters()" << std::endl;
#endif
	return Renderer().getPostProcessParamateres();
}

GLuint Mediator::RequestFrameBuffer()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestFrameBuffer()" << std::endl;
#endif

	return Renderer().getFrameBuffer();
}

void Mediator::restartAccumulation()
{
	Renderer().restartAccumulation();
	Scene().markDirty();
	ViewportManager::GetInstance().markViewportsDirty();
}

optix::Program Mediator::request_ActiveClosestHitProgram()
{
	return Renderer().getActiveClosestHitProgram();
}
optix::Program Mediator::request_ActiveClosestHitTexturedProgram()
{
	return Renderer().getActiveClosestHitTexturedProgram();
}
optix::Program Mediator::request_ActiveAnyHitProgram()
{
	return Renderer().getActiveAnyHitProgram();
}

OptiXShader& Mediator::request_ActiveOptiXShaderId()
{
	return Renderer().getActiveOptiXShader();
}
void Mediator::loadOptiXShader(std::string id)
{
	Renderer().setActiveOptixShader(id, true);
}

GLuint Mediator::requestRenderedSceneTexture()
{
	return Renderer().getRenderedSceneTexture();
}

GLuint Mediator::requestRenderedSceneMissTexture()
{
	return Renderer().getRenderedSceneMissTexture();
}

optix::float3 Mediator::request_Background_Colors(int index)
{
	optix::Context context = Renderer().getContext();
	if (index == 0)
	{
		return context["bg_color"]->getFloat3();
	}
	else
	{
		return context["bg_color2"]->getFloat3();
	}
}

optix::float3 * Mediator::request_MouseHit_Buffer_Data()
{
	
	return Renderer().getMouseHitBuffer_Data();
}

//------------------------------------------------------------------------------
//
// OpenGLRenderer Functions
//
//------------------------------------------------------------------------------

#ifdef OPENGL_RENDERER_OLD

int Mediator::RequestFocusedUtility()
{
	return UtilityRenderer().getFocusedUtility();
}

IntersectionPoint Mediator::RequestIsecPoint()
{
	
	return UtilityRenderer().getIsecPoint();
}

float Mediator::RequestInvCameraScaleFactor()
{
	
	return UtilityRenderer().getInvCameraScaleFactor();
}

#endif

ShaderProgram * Mediator::RequestShaderProgram()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestShaderProgram()" << std::endl;
#endif

	return util_Renderer().getShaderProgram();
	
}

void Mediator::convert_OptixObject_To_OpenGLGeometry(SceneObject * object)
{
	util_Renderer().createGeometryObject_OpenGL_FromSceneObject(object);
}

const optix::float2& Mediator::getRenderSize()
{
	return util_Renderer().getRenderSize();
}

void Mediator::Transform_World_To_Screen(optix::float3 p, int vp_id , float &x, float& y)
{
	util_Renderer().Transform_World_To_Screen(p, vp_id , x, y);
}




//------------------------------------------------------------------------------
//
// SceneManager Functions
//
//------------------------------------------------------------------------------

void Mediator::Update_SceneRestrictions_State(SceneObject* obj)
{
	Scene().Update_SceneRestrictions_State(obj);
}
void Mediator::setSceneParamaters(SceneParameters params)
{
	Scene().setSceneParameters(params);
}
SceneParameters Mediator::Request_SceneParameters()
{
	return Scene().get_SceneParameters();
}

void Mediator::Request_Scene_Import_State(int& state, std::string& msg)
{
	Scene().get_Scene_Import_State(state, msg);
}

void Mediator::setFocusedObject(int index)
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::setFocusedObject()" << std::endl;
#endif

#ifdef CANCER_DEBUG

	if (index != -1)
	{
		SceneObject* obj = Scene().getSceneObjects()[index];
		if (obj == nullptr || obj == 0)
		{

			//std::cout << "\n Mediator::setFocusedObject( " << index << " ) " << std::endl;
			//std::cout << "      - Invalid Index Requested!" << std::endl;
			system("pause");
		}
	}

#endif
	Scene().setFocusedObject(index);
}

void Mediator::setSelectedObject(int index, bool by_id)
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::setSelectedObject()" << std::endl;
#endif

#ifdef CANCER_DEBUG

	if (index != -1 && !by_id)
	{
		SceneObject* obj = Scene().getSceneObjects()[index];
		if (obj == nullptr || obj == 0)
		{
			//std::cout << "\n Mediator::setSelectedObject( " << index << " ) " << std::endl;
			//std::cout << "      - Invalid index requested!" << std::endl;
			system("pause");
		}
	}

#endif

	
	Scene().setSelectedObject(index, by_id);
}

void Mediator::addSceneObject(const char * filepath)
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::addSceneObject()" << std::endl;
#endif
	Scene().addSceneObject(filepath, ZERO_3f, ZERO_3f);
}




void Mediator::addSceneObject(SceneObject * object)
{
	Scene().addSceneObject(object);
}

void Mediator::addSceneLight(Light_Type type)
{
	Scene().addSceneLight(type);
}

void Mediator::addSceneLight(Light_Type type, optix::float3 p, optix::float3 * basis,bool link_to_object, SAMPLER_ALIGNEMENT sa)
{
	Scene().addSceneLight(type, p, basis, link_to_object, sa);
}

void Mediator::attachSamplerTo(class SceneObject* obj, Sampler_Type type)
{
	Scene().attachSamplerTo(obj, type);
}

void Mediator::clearSceneObjectsGuiFlags()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::clearSceneObjectsGuiFlags()" << std::endl;
#endif
	Scene().clearSceneObjectGuiFlags();
}

std::vector<class SceneObject *>& Mediator::RequestSceneObjects( bool state )
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestSceneObjects()" << std::endl;
#endif
	return Scene().getSceneObjects(state);
}

optix::Group& Mediator::RequestRootNode()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestRootNode()" << std::endl;
#endif
	return Scene().getRootNode();
}
optix::Group& Mediator::RequestRootShadowNode()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestRootNode()" << std::endl;
#endif
	return Scene().getRootShadowNode();
}

optix::Group& Mediator::RequestUtilityRootNode()
{
	return Scene().get_UtilityRootNode();
}
optix::Group& Mediator::Request_Utility_Samplers_RootNode()
{
	return Scene().get_Utility_Samplers_node();
}
optix::Group& Mediator::Request_Utility_FaceVector_RootNode()
{
	return Scene().get_Utility_FaceVectors_node();
}


void Mediator::scale_Lights(bool state)
{
	Scene().scale_Lights(state);
}
void Mediator::scale_Light(SceneObject* object, int vp_id, bool state)
{
	Scene().scale_Light(object, vp_id, state);
}

void Mediator::toggle_LightObjects(bool state, int r_q, int r_spot, int r_s, int r_p)
{
	Scene().toggle_Light_Objects(state, r_q, r_spot, r_s,r_p);
}
void Mediator::toggle_Bbox_Only_Scene_Render(bool state)
{
	Scene().toggle_Only_Bbox_Scene_Render(state);
}
void Mediator::toggle_Scene_RenderPass(bool state)
{
	Scene().toggleSceneRenderPass(state);
}

void Mediator::toggle_Utility_Root_Node_State(bool state)
{
	Scene().toggle_Utility_Root_Node_State(state);
}
void Mediator::toggle_FaceVector_State(bool state, bool only, bool scale )
{
	Scene().toggle_FaceVector_State(state, only, scale);
}
void Mediator::toggle_Samplers_State(bool state, int is_samplers, bool only, bool scale)
{
	Scene().toggle_Samplers_State(state, is_samplers, only, scale);
}
void Mediator::scale_FaceVectors(bool state)
{
	Scene().scale_FaceVectors(state);
}
void Mediator::toggle_Scene_grid_RenderPass(bool state)
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::toggleHighlightPass()" << std::endl;
#endif
	Scene().toggleScene_grid_RenderPass(state);
}

int Mediator::RequestFocusedObjectIndex()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestFocusedObjectIndex()" << std::endl;
#endif

#ifdef CANCER_DEBUG

	int index = Scene().getFocusedObjectIndex();

	if (index != -1)
	{
		SceneObject* obj = Scene().getSceneObjects()[index];
		if (obj == nullptr || obj == 0)
		{
			//std::cout << "\n Mediator::RequestFocusedObjectIndex( " << index << " ) " << std::endl;
			//std::cout << "      - Invalid Index Requested or returned nullptr!" << std::endl;
			system("pause");
		}
	}

#endif

	return Scene().getFocusedObjectIndex();
}

SceneObject* Mediator::RequestFocusedObject()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestFocusedObject()" << std::endl;
#endif

#ifdef CANCER_DEBUG

	/*
	int index = Scene().getFocusedObjectIndex();
	if (index != -1)
	{
		SceneObject* obj = Scene().getSceneObjects()[index];
		if (obj == nullptr || obj == 0)
		{
			//std::cout << "\n Mediator::RequestFocusedObject( " << index << " ) " << std::endl;
			//std::cout << "      - returned nullptr!" << std::endl;
			system("pause");
		}
	}
	*/

#endif

	int size = Scene().getSceneObjects().size();
	int obj_index = Scene().getFocusedObjectIndex();
	if (obj_index >= 0 && obj_index < size)
		return Scene().getSceneObjects()[obj_index];
	else
		return nullptr;
	
}

int Mediator::RequestSelectedObjectIndex()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestSelectedObjectIndex()" << std::endl;
#endif

#ifdef CANCER_DEBUG

	int index = Scene().getSelectedObjectIndex();
	if (index != -1)
	{
		SceneObject* obj = Scene().getSceneObjects()[index];
		if (obj == nullptr || obj == 0)
		{
			//std::cout << "\n Mediator::RequestSelectedObjectIndex( " << index << " ) " << std::endl;
			//std::cout << "      - selected_object == nullptr!" << std::endl;
			system("pause");
		}
	}

#endif

	return Scene().getSelectedObjectIndex();
}

SceneObject* Mediator::RequestSelectedObject()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestSelectedObject()" << std::endl;
#endif

#ifdef CANCER_DEBUG

	if (Scene().getSelectedObject() == nullptr)
	{
		//SceneObject* obj = Scene().getSelectedObject();
		////std::cout << "\n Mediator::RequestSelectedObject( SceneObject* )" << std::endl;
		////std::cout << "      - Returned nullptr!" << std::endl;
		//system("pause");
	}

#endif

	return Scene().getSelectedObject();
}

int Mediator::RequestSceneObjectsCount()
{
	return Scene().getObjectsCount();
}

void Mediator::setFocusedGroup(int index)
{
	Scene().setFocusedGroup(index);
}
void Mediator::setSelectedGroup(int index)
{
	Scene().setSelectedGroup(index);
}
int Mediator::Request_FocusedGroupIndex()
{
	return Scene().getFocusedGroupIndex();
}
int Mediator::Request_SelectedGroupIndex()
{
	return Scene().getSelectedGroupIndex();
}
SceneObject* Mediator::Request_FocusedGroup()
{
	return Scene().getFocusedGroup();
}
SceneObject* Mediator::Request_SelectedGroup()
{
	return Scene().getSelectedGroup();
}
void Mediator::create_SceneGroup(std::vector<class SceneObject*>& group_childs)
{
	Scene().create_SceneGroup(group_childs);
}
void Mediator::create_SceneGroup(std::vector<int>& group_childs)
{
	Scene().create_SceneGroup(group_childs);
}
void Mediator::remove_SceneGroup(int index)
{
	Scene().remove_SceneGroup(index);
}
SceneObject* Mediator::Request_SceneGroup(int index)
{
	return Scene().getSceneGroups()[index];
}
std::vector<SceneObject*>& Mediator::Request_SceneGroups()
{
	return Scene().getSceneGroups();
}

int Mediator::requestObjectsArrayIndex(SceneObject* object)
{
	
	for (int i = 0; i < Scene().getSceneObjects().size(); i++)
	{
		if (Scene().getSceneObjects()[i]->getId() == object->getId())
			return i;
	}

	return -1;
}

void Mediator::removeObjectFromScene( SceneObject* object )
{
#ifdef CANCER_DEBUG

	if (object == nullptr || object == 0
		|| object->getId() >= 0 && requestObjectsArrayIndex(object) == -1)
	{
		//std::cout << "\n Mediator::removeObjectFromScene( " << object->getId() << " ) " << std::endl;
		//std::cout << "       - invalid index requested or object == nullptr!" << std::endl;
		system("pause");
	}

#endif

	Scene().removeObject(object);
}

int Mediator::requestObjectsArrayIndex(int obj_id)
{

	for (int i = 0; i < Scene().getSceneObjects().size(); i++)
	{
		if (Scene().getSceneObjects()[i]->getId() == obj_id)
			return i;
	}

	return -1;
}

TreeNode* Mediator::request_HierarchyTree_Root()
{
	return Scene().getHierarchyTreeRoot();
}

Hierarchy_Tree* Mediator::request_HierarchyTreeHandle()
{
	return Scene().getHierarchyTreeHandle();
}

SceneObject* Mediator::RequestSceneObject(int id)
{
	for (SceneObject* obj : Scene().getSceneObjects())
		if (obj->getId() == id)
			return obj;

	return nullptr;
}

void Mediator::Update_SceneLight(Light_Parameters light, int index)
{
	Scene().Update_SceneLight(light, index);
}
void Mediator::addSceneLight_To_Buffer(Light_Parameters light)
{
	Scene().addSceneLight_To_Buffer(light);
}
void Mediator::removeSceneLight_From_Buffer(int index)
{
	Scene().removeSceneLight_From_Buffer(index);
}

 SceneObject* Mediator::create_FaceVector(optix::float3 pos, optix::float3 scale, bool save_action , class SceneObject* parent )
{
	 return Scene().create_FaceVector(pos, scale, save_action , parent );
}

 SceneObject* Mediator::create_Sampler(optix::float3 pos, optix::float3 scale, Sampler_Type s_type, bool save_action, class SceneObject* parent)
 {
	 return Scene().create_Sampler(pos, scale, s_type, save_action, parent);
 }


 std::vector<Restriction*> Mediator::Request_Scene_Restrictions()
 {
	 return Scene().get_Scene_Objects_Restrictions();
 }
 void Mediator::add_Restriction(Restriction* R)
 {
	 Scene().add_Restriction(R);
 }
 void Mediator::Remove_Restriction(int id, bool by_id)
 {
	 Scene().Remove_Restriction(id, by_id);
 }
 Restriction* Mediator::Request_Restriction(int id, bool by_id )
 {
	 return Scene().get_Restriction(id, by_id);
 }


 void Mediator::Reset_Restrictions_UI_States(bool only_focus)
 {
	 Scene().Reset_Restrictions_UI_states(only_focus);
 }




 void Mediator::Export_Scene(const char * path, const char* filename)
 {
	 Scene().Export_Scene_as_JSON( path, filename );
 }
 void Mediator::Save_Scene(const char * path, const char* filename)
 {
	 Scene().Export_Scene( path, filename );
 }
 void Mediator::Import_Scene(const char* filepath)
 {
	 Scene().Import_Scene( filepath );
 }


//------------------------------------------------------------------------------
//
// InterfaceManager Functions
//
//------------------------------------------------------------------------------

bool Mediator::RequestSelected_Utility_is_Active_var()
{
	return Interface().is_selected_utility_active();
}
AXIS_ORIENTATION Mediator::Request_Active_Axis_Orientation()
{
	return Interface().get_active_Axis_Orientation();
}
void Mediator::set_Axis_Orientation(AXIS_ORIENTATION axis_orientation)
{
	Interface().set_active_Axis_Orientation(axis_orientation);
}
void Mediator::set_Active_Utility(Utility utility)
{
	Interface().set_active_Utility(utility);
}
bool Mediator::isGuiFocused()
{
	return Interface().isGuiFocused();
}
optix::float2 Mediator::Request_MousePosRelativeTo(int id)
{
	return Interface().get_MousePos_RelativeTo(id);
}
Dimension_Constraints Mediator::Request_ViewportScape_Constraints()
{
	return Interface().getViewportSpaceConstraints();
}
void Mediator::setSceneDirty()
{
	Scene().markDirty();
}
void Mediator::markViewportsDirty(int code)
{
	ViewportManager::GetInstance().markViewportsDirty(code);
}
bool Mediator::isSceneDirty( bool reset )
{
	return Scene().isDirty( reset );
}
bool Mediator::isSceneDirty(bool *prev_state , bool reset )
{
	return Scene().isDirty(prev_state, reset );
}
bool Mediator::wasSceneDirty()
{
	return Scene().isDirty();
}
void Mediator::removeMultipleSelectionData(int index)
{
	Interface().remove_Multiple_Selection_Data(index);
}
void Mediator::Recover_Selections_State(int index)
{
	Interface().recover_selections_state(index);
}
void Mediator::Recover_Selections_State(int index, bool reset)
{
	Interface().recover_selections_state(index, reset);
}
void Mediator::Recover_Selections_State_flat(int index)
{
	Interface().recover_selections_state_flat(index);
}
void Mediator::set_Active_Multiple_Selection(int index)
{
	Interface().set_active_multiple_selection(index);
}
bool Mediator::is_Multiple_Selection_Active()
{
	return Interface().is_multiple_selection_Active();
}
int Mediator::Request_Area_Selection_Active_Id()
{
	return Interface().get_Area_Selection_Active_Id();
}
const Multiple_Selection_Data& Mediator::Request_Active_Area_Selection_Data()
{
	return Interface().get_Active_Area_Selection_Data();
}
const Multiple_Selection_Data& Mediator::Request_Area_Selection_Data(int parent_id)
{
	return Interface().get_Area_Selection_Data(parent_id);
}
void Mediator::force_Execute_Scene_Remove_Calls()
{
	Scene().force_execute_remove_calls();
}
Io_Mouse Mediator::Request_App_MouseData()
{
	return Interface().getMouseData();
}
FaceVector_View_State Mediator::Request_FaceVector_View_State()
{
	return Interface().get_FaceVector_View_State();
}
FaceVector_Intersection_State Mediator::Request_FaceVector_Intersection_State()
{
	return Interface().get_FaceVector_Intersection_State();
}
int Mediator::Request_Ui_Focused_Object()
{
	return Interface().get_Ui_Focused_Object();
}

SceneObject* Mediator::Request_INTERFACE_prev_Focused_Object()
{
	return Interface().Get_prev_Focused_Object();
}
int Mediator::Request_INTERFACE_prev_Focused_Object_Id()
{
	return Interface().Get_prev_Focused_Object_id();
}
SceneObject* Mediator::Request_INTERFACE_prev_Selected_Object()
{
	return Interface().Get_prev_Selected_Object();
}
int Mediator::Request_INTERFACE_prev_Selected_Object_Id()
{
	return Interface().Get_prev_Selectecd_Object_Id();
}
SceneObject* Mediator::Request_INTERFACE_Selected_Object()
{
	return Interface().Get_Selected_Object();
}
SceneObject* Mediator::Request_INTERFACE_Focused_Object()
{
	return Interface().Get_Focused_Object();
}
Object_Package Mediator::Request_INTERFACE_Selected_Object_Pack()
{
	return Interface().Get_Focused_Object_Pack();
}
Object_Package Mediator::Request_INTERFACE_Focused_Object_Pack()
{
	return Interface().Get_Selected_Object_Pack();
}

Restriction* Mediator::get_UI_Selected_Restriction()
{
	return Interface().get_UI_Selected_Restriction();
}
void Mediator::set_UI_Selected_Restriction(Restriction*R)
{
	Interface().set_UI_Selected_Restriction(R);
}

UtilityProperties Mediator::Request_INTERFACE_Active_Utility()
{
	return Interface().get_active_Utility();
}

//------------------------------------------------------------------------------
//
// General Functions
//
//------------------------------------------------------------------------------

void Mediator::AUTO_SAVE_TEMP_SCENE(std::string name)
{
	OptiXSceneEditor::GetInstance().SAVE_TEMP_DATA(name);
}

bool Mediator::is_AppWindowSizeChanged()
{
	return OptiXSceneEditor::GetInstance().isWindowSizeChanged();
}

float * Mediator::RequestMousePos()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestMousePos()" << std::endl;
#endif
	double x, y;
	glfwGetCursorPos(OptiXSceneEditor::GetInstance().getAppWindow(), &x, &y);

	float * xy = new float[2];
	xy[0] = (float)x;
	xy[1] = (float)y;

#ifdef CANCER_DEBUG

	if (xy == 0 || xy == nullptr)
	{
		//std::cout << "\n float* Mediator::RequestMousePos()" << std::endl;
		//std::cout << "         - return nullptr!" << std::endl;
		system("pause");
	}
#endif

	return xy;
}

float   Mediator::RequestMousePos(int index)
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestMousePos(int index)" << std::endl;
#endif
	double x, y;
	glfwGetCursorPos(OptiXSceneEditor::GetInstance().getAppWindow(), &x, &y);

	return (index == 0) ? (float)x : (float)y;
}

GLFWwindow * Mediator::RequestWindow()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestWindow()" << std::endl;
#endif

#ifdef CANCER_DEBUG

	if (OptiXSceneEditor::GetInstance().getAppWindow() == nullptr)
	{
		//std::cout << "\n Mediator::RequestWindow():" << std::endl;
		//std::cout << "      - returned nullptr!" << std::endl;
		system("pause");
	}

#endif
	return OptiXSceneEditor::GetInstance().getAppWindow();
}

int * Mediator::RequestWindowSize()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestWindowSize()" << std::endl;
#endif
	
#ifdef CANCER_DEBUG

	if (OptiXSceneEditor::GetInstance().getWindowSize() == nullptr)
	{
		//std::cout << "\n int* Mediator::RequestWindowSize():" << std::endl;
		//std::cout << "      - returned nullptr!" << std::endl;
		system("pause");
	}

#endif
	return OptiXSceneEditor::GetInstance().getWindowSize();
}

int   Mediator::RequestWindowSize(int index)
{

#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestWindowSize(int index)" << std::endl;
#endif

	int width, height;
	glfwGetWindowSize(OptiXSceneEditor::GetInstance().getAppWindow(),&width, &height);
	
	return (index == 0) ? width : height;
	
}

int Mediator::RequestOptiXstackSize()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestOptiXstackSize()" << std::endl;
#endif
	return OptiXSceneEditor::GetInstance().getOptiXstackSize();
}

bool Mediator::RequestOpenGL_Interop()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::RequestOpenGL_Interop()" << std::endl;
#endif
	return OptiXSceneEditor::GetInstance().getOpenGL_interop();
}

void Mediator::setGlobalSelectedObject(int object_index)
{
	Scene().setSelectedObject(object_index); 
	Interface().setSelectedObject(object_index);
}

void Mediator::setGlobalSelectedObject(int object_index, bool reset, bool by_id)
{
	Scene().setSelectedObject(object_index, by_id);

	int real_object_index;
	if (by_id)
		real_object_index = Mediator::requestObjectsArrayIndex(object_index);
	else
		real_object_index = object_index;

	Interface().setSelectedObject(real_object_index, reset, by_id);
}

float Mediator::Request_Global_Time_Delta()
{
	return OptiXSceneEditor::GetInstance().get_Global_TimeDelta();
}

void Mediator::clear( bool state  )
{
	OptiXSceneEditor::GetInstance().clear( state );
}

//------------------------------------------------------------------------------
//
// guiManager Functions
//
//------------------------------------------------------------------------------

#ifdef GUI_MANAGER

bool Mediator::isWindowHovered()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::isWindowHovered()" << std::endl;
#endif
	return gui().isWindowHovered();
}

int Mediator::requestGuiFocusedObject()
{
#if defined(FUNCTION_CALL_REPORT) || defined(FUNCTION_CALL_REPORT_MEDIATOR)
	//std::cout << "  --Mediator::requestGuiFocusedObject()" << std::endl;
#endif
	return gui().getFocusedObject();
}

int Mediator::requestActiveUtility()
{
	return gui().getActiveUtility();
}

int Mediator::requestGuiState()
{
	return gui().getGuiState();
}

bool Mediator::requestAxisOrientation(AXIS_ORIENTATION& id)
{
	return gui().getAxisOrientation(id);
}



#endif
//------------------------------------------------------------------------------
//
// HELPER Functions
//
//------------------------------------------------------------------------------



OptiXRenderer& Mediator::Renderer()
{   
	return OptiXRenderer::GetInstace();
}

#ifdef OPENGL_RENDERER_OLD
OpenGLRenderer& Mediator::UtilityRenderer()
{
	return OpenGLRenderer::GetInstace();
}
#endif

OpenGLRenderer& Mediator::util_Renderer()
{
	return OpenGLRenderer::GetInstace();
}

SceneManager& Mediator::Scene()
{
	return SceneManager::GetInstance();
}

InterfaceManager& Mediator::Interface()
{
	return InterfaceManager::GetInstance();
}

#ifdef GUI_MANAGER
guiManager& Mediator::gui()
{
	return guiManager::GetInstance();
}
#endif