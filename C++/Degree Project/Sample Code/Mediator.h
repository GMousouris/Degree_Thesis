#pragma once
#include "Defines.h"

#include <GL/glew.h>

#include <optixu/optixpp_namespace.h>
#include "common_structs.h"
#include "HierarchyTree.h"
#include "Interface_Structs.h"
#include "OptiXRenderer_structs.h"
#include "Utility_Renderer_Defines.h"
#include "SceneGroup.h"
#include <vector>

class Mediator
{

public:

	Mediator () {}
	~Mediator() {}

	static void Init( class OptiXSceneEditor * app );
//------------------------------------------------------------------------------
//
// HELPER Functions
//
//------------------------------------------------------------------------------
private:


	static class OptiXRenderer& Renderer();
#ifdef OPENGL_RENDERER_OLD
	static class OpenGLRenderer& UtilityRenderer();
#endif

	static class InterfaceManager& Interface();

	static class OpenGLRenderer& util_Renderer();

	static class SceneManager& Scene();

#ifdef GUI_MANAGER
	static class guiManager& gui();
#endif


public:
//------------------------------------------------------------------------------
//
// OptiXRenderer Functions
//
//------------------------------------------------------------------------------

	static GLuint Request_Optix_DepthBuffer();

	static class PinholeCamera& RequestCameraInstance();

	static optix::Context& RequestContext();

	static  GLuint& RequestOpenGLShaderProgram();

	static class PostProcessParameters& RequestPostProcessParameters();

	static GLuint RequestFrameBuffer();

	static void restartAccumulation();

	static optix::Program request_ActiveClosestHitProgram();
	static optix::Program request_ActiveClosestHitTexturedProgram();
	static optix::Program request_ActiveAnyHitProgram();

	static OptiXShader& request_ActiveOptiXShaderId();
	static void loadOptiXShader(std::string shader_id);

	static GLuint requestRenderedSceneTexture();
	static GLuint requestRenderedSceneMissTexture();


	static optix::float3 request_Background_Colors(int index);

	static optix::float3 * request_MouseHit_Buffer_Data();


//------------------------------------------------------------------------------
//
// OpenGLRenderer Functions
//
//------------------------------------------------------------------------------

#ifdef OPENGL_RENDERER_OLD
	static int RequestFocusedUtility();

	static IntersectionPoint RequestIsecPoint();

	static float RequestInvCameraScaleFactor();
#endif

	static class ShaderProgram * RequestShaderProgram();

	static void convert_OptixObject_To_OpenGLGeometry( SceneObject * object );

	static const optix::float2& getRenderSize();

	static void Transform_World_To_Screen(optix::float3 p, int vp_id , float &x, float& y);




//------------------------------------------------------------------------------
//
// SceneManager Functions
//
//------------------------------------------------------------------------------


	static void Update_SceneRestrictions_State(class SceneObject* obj = 0);
	static void setSceneParamaters(SceneParameters params);
	static SceneParameters Request_SceneParameters();

	static void Request_Scene_Import_State(int& state, std::string& msg);

	static std::vector<class SceneObject *>& RequestSceneObjects( bool state = true );

	static optix::Group& RequestRootNode();
	static optix::Group& RequestRootShadowNode();
	static optix::Group& Mediator::RequestUtilityRootNode();
	static optix::Group& Mediator::Request_Utility_Samplers_RootNode();
	static optix::Group& Mediator::Request_Utility_FaceVector_RootNode();

	static void scale_Lights(bool state);
	static void scale_Light(SceneObject* object, int vp_id, bool state);

	static void toggle_LightObjects(bool state, int r_q, int r_spot, int r_s, int r_p);
	static void toggle_Bbox_Only_Scene_Render(bool state);
	static void toggle_Utility_Root_Node_State(bool state);
	static void toggle_FaceVector_State(bool state , bool only = false, bool scale = false);
	static void toggle_Samplers_State(bool state, int is_samplers,   bool only = false, bool scale = false);
	static void scale_FaceVectors(bool state);
	static void toggle_Scene_RenderPass(bool state);
	static void toggle_Scene_grid_RenderPass(bool state);

	static void setFocusedObject(int index);

	static void setSelectedObject(int index, bool by_id = false);

	static int RequestFocusedObjectIndex();

	static SceneObject * RequestFocusedObject();

	static int RequestSelectedObjectIndex();

	static SceneObject * RequestSelectedObject();

	static void addSceneObject(const char * filepath);
	
	static void addSceneObject(SceneObject * object);

	static void addSceneLight(Light_Type type);
	static void addSceneLight(Light_Type type, optix::float3 p, optix::float3 * basis, bool link_to_object = false, SAMPLER_ALIGNEMENT sa = SA_GLOBAL);

	static void attachSamplerTo(class SceneObject* obj, Sampler_Type type);

	static void clearSceneObjectsGuiFlags();

	static int  RequestSceneObjectsCount();

	static SceneObject* RequestSceneObject(int id);

	static void setFocusedGroup(int index);

	static void setSelectedGroup(int index);

	static int Request_FocusedGroupIndex();

	static int Request_SelectedGroupIndex();

	static SceneObject* Request_FocusedGroup();

	static SceneObject* Request_SelectedGroup();

	static void create_SceneGroup(std::vector<SceneObject*>& group_childs);

	static void create_SceneGroup(std::vector<int>& group_childs);

	static void remove_SceneGroup(int index);

	static SceneObject* Request_SceneGroup(int index);

	static std::vector<SceneObject*>& Request_SceneGroups();

	static int requestObjectsArrayIndex(class SceneObject* object);

	static void removeObjectFromScene(class SceneObject* object);

	static int requestObjectsArrayIndex(int obj_id);

	static TreeNode* request_HierarchyTree_Root();

	static Hierarchy_Tree* request_HierarchyTreeHandle();

	static void setSceneDirty();

	static void markViewportsDirty(int code = 0);

	static bool isSceneDirty( bool reset );

	static bool isSceneDirty(bool *prev_state, bool reset = true );

	static bool wasSceneDirty();


	static void force_Execute_Scene_Remove_Calls();

	static void Update_SceneLight(Light_Parameters light, int index);
	static void addSceneLight_To_Buffer(Light_Parameters light);
	static void removeSceneLight_From_Buffer(int index);

	static class SceneObject* create_FaceVector(optix::float3 pos, optix::float3 scale, bool save_action = true, class SceneObject* parent = 0);
	static class SceneObject* create_Sampler(optix::float3 pos, optix::float3 scale, Sampler_Type s_type, bool save_action = true, class SceneObject* parent = 0);


	static std::vector<class Restriction*> Request_Scene_Restrictions();
	static void add_Restriction(class Restriction* R);
	static void Remove_Restriction(int id, bool by_id = false);
	static class Restriction* Request_Restriction(int id, bool by_id = false);


	static void Reset_Restrictions_UI_States(bool only_focus = false);
	
	static void Export_Scene(const char * path, const char* filename);
	static void Save_Scene(const char* path, const char* filename);
	static void Import_Scene(const char* path);
	

//------------------------------------------------------------------------------
//
// guiManager Functions
//
//------------------------------------------------------------------------------

#ifdef GUI_MANAGER

	static bool isWindowHovered();

	static int  requestGuiFocusedObject();

	static int requestActiveUtility();

	static int requestGuiState();

	static bool requestAxisOrientation(AXIS_ORIENTATION& id);

#endif


//------------------------------------------------------------------------------
//
// InterfaceManager Functions
//
//------------------------------------------------------------------------------

	static bool RequestSelected_Utility_is_Active_var();

	static AXIS_ORIENTATION Request_Active_Axis_Orientation();

	static void set_Axis_Orientation(AXIS_ORIENTATION axis_orientation);

	static void set_Active_Utility(Utility utility);

	static bool isGuiFocused();

	static optix::float2 Request_MousePosRelativeTo(int id);

	static Dimension_Constraints Request_ViewportScape_Constraints();

	static void removeMultipleSelectionData(int index);

	static void Recover_Selections_State(int index);
	static void Recover_Selections_State(int index, bool reset);

	static void Recover_Selections_State_flat(int index);
	static void set_Active_Multiple_Selection(int index);
	static bool is_Multiple_Selection_Active();
	static int Request_Area_Selection_Active_Id();
	static const Multiple_Selection_Data& Request_Active_Area_Selection_Data();
	static const Multiple_Selection_Data& Request_Area_Selection_Data(int parent_id);

	static Io_Mouse Request_App_MouseData();

	static FaceVector_View_State Request_FaceVector_View_State();
	static FaceVector_Intersection_State Request_FaceVector_Intersection_State();

	static int Request_Ui_Focused_Object();

	static class SceneObject* Request_INTERFACE_prev_Focused_Object();
	static int Request_INTERFACE_prev_Focused_Object_Id();
	static class SceneObject* Request_INTERFACE_prev_Selected_Object();
	static int Request_INTERFACE_prev_Selected_Object_Id();

	static class SceneObject*    Request_INTERFACE_Selected_Object();
	static class SceneObject*    Request_INTERFACE_Focused_Object();
	static struct Object_Package Request_INTERFACE_Selected_Object_Pack();
	static struct Object_Package Request_INTERFACE_Focused_Object_Pack();

	static void set_UI_Selected_Restriction(class Restriction*R);
	static class Restriction* get_UI_Selected_Restriction();

	static UtilityProperties Request_INTERFACE_Active_Utility();

//------------------------------------------------------------------------------
//
// General Functions
//
//------------------------------------------------------------------------------
	static void AUTO_SAVE_TEMP_SCENE(std::string name = "");

	static void setGlobalSelectedObject(int object_index);
	static void setGlobalSelectedObject(int object_index, bool reset, bool by_id = false);
	static float * RequestMousePos();
	static float   RequestMousePos(int index);
	static struct GLFWwindow * RequestWindow();
	static int * RequestWindowSize();
	static int   RequestWindowSize(int index);
	static int RequestOptiXstackSize();
	static bool RequestOpenGL_Interop();
	static bool is_AppWindowSizeChanged();
	static float Request_Global_Time_Delta();


	static void clear( bool state = true );

private:
	static class OptiXSceneEditor * main_app;

};

