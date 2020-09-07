#include "Defines.h"
#include "Utilities.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glew.h> //wglew.h
#  if defined( _WIN32 )
#    include <GL/glew.h>
#    include <GL/freeglut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

#include <optixu/optixu_math_namespace.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>


#include <common.h>
#include "Arcball.h"
#include "OptiXMesh.h"


// standard
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <fstream>


using namespace optix;


const char * mesh_path = "../Data/3dObjects/cow.obj";

//------------------------------------------------------------------------------
//
// Globals
//
//------------------------------------------------------------------------------

optix::Context        context;
uint32_t       width = 1024;
uint32_t       height = 768;
bool           use_pbo = true;
bool           use_tri_api = true;
bool           ignore_mats = false;
optix::Aabb    aabb;

// Camera state
float3         camera_up;
float3         camera_lookat;
float3         camera_eye;
Matrix4x4      camera_rotate;
sutil::Arcball arcball;

// Mouse state
int2           mouse_prev_pos;
int            mouse_button;


//------------------------------------------------------------------------------
//
// Forward decls 
//
//------------------------------------------------------------------------------

struct UsageReportLogger;

Buffer getOutputBuffer();
void destroyContext();
void registerExitHandler();
void createContext(int usage_report_level, UsageReportLogger* logger);
void loadMesh(const std::string& filename);
void setupCamera();
void setupLights();
void updateCamera();
void glutInitialize(int* argc, char** argv);
void glutRun();

void glutDisplay();
void glutKeyboardPress(unsigned char k, int x, int y);
void glutMousePress(int button, int state, int x, int y);
void glutMouseMotion(int x, int y);
void glutResize(int w, int h);


//------------------------------------------------------------------------------
//
//  Helper functions
//
//------------------------------------------------------------------------------

void resizeBuffer(optix::Buffer buffer, unsigned width, unsigned height)
{
	buffer->setSize(width, height);

	// Check if we have a GL interop display buffer
	const unsigned pboId = buffer->getGLBOId();
	if (pboId)
	{
		buffer->unregisterGLBuffer();
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, buffer->getElementSize() * width * height, 0, GL_STREAM_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		buffer->registerGLBuffer();
	}
}

Buffer getOutputBuffer()
{
	return context["sys_OutputBuffer"]->getBuffer();
}

/*----------------------------------------------------------------------------*/


void destroyContext()
{
	if (context)
	{
		context->destroy();
		context = 0;
	}
}


struct UsageReportLogger
{
	void log(int lvl, const char* tag, const char* msg)
	{
		std::cout << "[" << lvl << "][" << std::left << std::setw(12) << tag << "] " << msg;
	}
};

// Static callback
void usageReportCallback(int lvl, const char* tag, const char* msg, void* cbdata)
{
	// Route messages to a C++ object (the "logger"), as a real app might do.
	// We could have printed them directly in this simple case.

	UsageReportLogger* logger = reinterpret_cast<UsageReportLogger*>(cbdata);
	logger->log(lvl, tag, msg);
}

void registerExitHandler()
{
	// register shutdown handler
#ifdef _WIN32
	glutCloseFunc(destroyContext);  // this function is freeglut-only
#else
	atexit(destroyContext);
#endif
}



void createContext(int usage_report_level, UsageReportLogger* logger)
{

#ifdef DEBUGGING
	std::cout << "- [Procedure] : Creating Optix::Context.." << std::endl;
#endif

	// Set up context
	context = Context::create();
	context->setPrintEnabled(1);
	context->setPrintBufferSize(4096);
	context->setRayTypeCount(2);
	context->setEntryPointCount(1);


	if (usage_report_level > 0)
	{
		context->setUsageReportCallback(usageReportCallback, usage_report_level, logger);
	}

	context["sys_SceneEpsilon"]->setFloat(1.e-4f);
	

#ifdef DEBUGGING
	std::cout << "- [Procedure] : Creating Optix-OpenGL-OutputBuffer.." << std::endl;
#endif
	Buffer buffer = Utilities::createOutputBuffer(context, RT_FORMAT_UNSIGNED_BYTE4, width, height, use_pbo , RT_BUFFER_OUTPUT);
	context["sys_OutputBuffer"]->set(buffer);


	/* 
	  *
	  * 
	  *  CREATING & LINKING RT_PROGRAMS TO HOST
	  *
	  *
	*/


#ifdef DEBUGGING
	std::cout << "- [Procedure] : CREATING & LINKING RT_PROGRAMS TO HOST" << std::endl;
#endif

	// Ray generation program	
	Program ray_gen_program = context->createProgramFromPTXFile( Utilities::getPtxPath("pinhole_camera.cu") , "pinhole_camera");
	ray_gen_program->validate();
	context->setRayGenerationProgram(0, ray_gen_program);
#ifdef DEBUGGING
	std::cout << "- [Report] : Ray_generation program created & linked succesfully!" << std::endl;
#endif

	// Exception program
	Program exception_program = context->createProgramFromPTXFile(Utilities::getPtxPath("exception.cu") , "exception");
	exception_program->validate();
	context->setExceptionProgram(0, exception_program);
	context["bad_color"]->setFloat(1.0f, 0.0f, 1.0f);
#ifdef DEBUGGING
	std::cout << "- [Report] : Exception program created & linked succesfully!" << std::endl;
#endif

	// Miss program
	Program miss_program = context->createProgramFromPTXFile(Utilities::getPtxPath("miss.cu") , "miss");
	miss_program->validate();
	context->setMissProgram(0, miss_program);
	context["bg_color"]->setFloat(0.34f, 0.55f, 0.85f);
#ifdef DEBUGGING
	std::cout << "- [Report] : Miss program created & linked succesfully!" << std::endl;
#endif

}


void loadMesh(const std::string& filename)
{

	OptiXMesh mesh;
	mesh.context = context;
	mesh.use_tri_api = use_tri_api;
	mesh.ignore_mats = ignore_mats;
	loadMesh(filename, mesh);

	aabb.set(mesh.bbox_min, mesh.bbox_max);

	GeometryGroup geometry_group = context->createGeometryGroup();
	geometry_group->addChild(mesh.geom_instance);


	geometry_group->setAcceleration(context->createAcceleration("Trbvh"));
	context["top_object"]->set(geometry_group);
	context["top_shadower"]->set(geometry_group);
}


void setupCamera()
{
	const float max_dim = fmaxf(aabb.extent(0), aabb.extent(1)); // max of x, y components

	camera_eye = aabb.center() + make_float3(0.0f, 0.0f, max_dim*1.5f);
	camera_lookat = aabb.center();
	camera_up = make_float3(0.0f, 1.0f, 0.0f);

	camera_rotate = Matrix4x4::identity();
}


void setupLights()
{
	const float max_dim = fmaxf(aabb.extent(0), aabb.extent(1)); // max of x, y components

	BasicLight lights[] = {
		{ make_float3(-0.5f,  0.25f, -1.0f), make_float3(0.2f, 0.2f, 0.25f), 0, 0 },
		{ make_float3(-0.5f,  0.0f ,  1.0f), make_float3(0.1f, 0.1f, 0.10f), 0, 0 },
		{ make_float3(0.5f,  0.5f ,  0.5f), make_float3(0.7f, 0.7f, 0.65f), 1, 0 }
	};
	lights[0].pos *= max_dim * 10.0f;
	lights[1].pos *= max_dim * 10.0f;
	lights[2].pos *= max_dim * 10.0f;

	Buffer light_buffer = context->createBuffer(RT_BUFFER_INPUT);
	light_buffer->setFormat(RT_FORMAT_USER);
	light_buffer->setElementSize(sizeof(BasicLight));
	light_buffer->setSize(sizeof(lights) / sizeof(lights[0]));
	memcpy(light_buffer->map(), lights, sizeof(lights));
	light_buffer->unmap();

	context["lights"]->set(light_buffer);
}


void updateCamera()
{

	const float vfov = 35.0f;
	const float aspect_ratio = static_cast<float>(width) /
		static_cast<float>(height);

	float3 camera_u, camera_v, camera_w;
	Utilities::calculateCameraVariables(
		camera_eye, camera_lookat, camera_up, vfov, aspect_ratio,
		camera_u, camera_v, camera_w, true);

	const Matrix4x4 frame = Matrix4x4::fromBasis(
		normalize(camera_u),
		normalize(camera_v),
		normalize(-camera_w),
		camera_lookat);
	const Matrix4x4 frame_inv = frame.inverse();
	// Apply camera rotation twice to match old SDK behavior
	const Matrix4x4 trans = frame * camera_rotate*frame_inv;

	camera_eye = make_float3(trans*make_float4(camera_eye, 1.0f));
	camera_lookat = make_float3(trans*make_float4(camera_lookat, 1.0f));
	camera_up = make_float3(trans*make_float4(camera_up, 0.0f));

	Utilities::calculateCameraVariables(
		camera_eye, camera_lookat, camera_up, vfov, aspect_ratio,
		camera_u, camera_v, camera_w, true);

	camera_rotate = Matrix4x4::identity();

	context["sys_CameraPosition"]->setFloat(camera_eye);
	context["sys_CameraU"]->setFloat(camera_u);
	context["sys_CameraV"]->setFloat(camera_v);
	context["sys_CameraW"]->setFloat(camera_w);
}


void glutInitialize(int* argc, char** argv)
{
	glutInit(argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Project");
	glutHideWindow();
}


void glutRun()
{
	// Initialize GL state                                                            
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, width, height);

	glutShowWindow();
	glutReshapeWindow(width, height);

	// register glut callbacks
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutDisplay);
	glutReshapeFunc(glutResize);
	glutKeyboardFunc(glutKeyboardPress);
	glutMouseFunc(glutMousePress);
	glutMotionFunc(glutMouseMotion);

	registerExitHandler();

	glutMainLoop();
}

//------------------------------------------------------------------------------
//
// ImGui
//
//------------------------------------------------------------------------------

void ImGuiInitialize()
{
	
}

void ImGuiCleanUp()
{
	
}

void NewFrameGui()
{
	
}

void DisplayGui()
{

}

void RenderGui()
{
	
}

//------------------------------------------------------------------------------
//
//  GLUT callbacks
//
//------------------------------------------------------------------------------

void glutDisplay()
{
	// Optix
	updateCamera();
	context->launch(0, width, height);
	Utilities::displayBufferGL(getOutputBuffer());
	//

	// ImGui
	//NewFrameGui();
	//DisplayGui();
	//RenderGui();
	
	glutSwapBuffers();
	//glutPostRedisplay();

}


void glutKeyboardPress(unsigned char k, int x, int y)
{

	switch (k)
	{
	case('q'):
	case(27): // ESC
	{
		destroyContext();
		exit(0);
	}
	
	}
}


void glutMousePress(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		mouse_button = button;
		mouse_prev_pos = make_int2(x, y);
	}
	else
	{
		// nothing
	}
}


void glutMouseMotion(int x, int y)
{
	if (mouse_button == GLUT_RIGHT_BUTTON)
	{
		const float dx = static_cast<float>(x - mouse_prev_pos.x) /
			static_cast<float>(width);
		const float dy = static_cast<float>(y - mouse_prev_pos.y) /
			static_cast<float>(height);
		const float dmax = fabsf(dx) > fabs(dy) ? dx : dy;
		const float scale = fminf(dmax, 0.9f);
		camera_eye = camera_eye + (camera_lookat - camera_eye)*scale;
	}
	else if (mouse_button == GLUT_LEFT_BUTTON)
	{
		const float2 from = { static_cast<float>(mouse_prev_pos.x),
							  static_cast<float>(mouse_prev_pos.y) };
		const float2 to = { static_cast<float>(x),
							  static_cast<float>(y) };

		const float2 a = { from.x / width, from.y / height };
		const float2 b = { to.x / width, to.y / height };

		camera_rotate = arcball.rotate(b, a);
	}

	mouse_prev_pos = make_int2(x, y);
}


void glutResize(int w, int h)
{
	if (w == (int)width && h == (int)height) return;

	if (w <= 0) w = 1;
	if (h <= 0) h = 1;

	

	width = w;
	height = h;


	resizeBuffer(getOutputBuffer(), width, height);

	glViewport(0, 0, width, height);

	glutPostRedisplay();
}

//------------------------------------------------------------------------------
//
// Main
//
//------------------------------------------------------------------------------

void printUsageAndExit(const std::string& argv0)
{
	std::cerr << "\nUsage: " << argv0 << " [options]\n";
	std::cerr <<
		"App Options:\n"
		"  -h | --help               Print this usage message and exit.\n"
		"  -f | --file               Save single frame to file and exit.\n"
		"  -n | --nopbo              Disable GL interop for display buffer.\n"
		"  -m | --mesh <mesh_file>   Specify path to mesh to be loaded.\n"
		"  -r | --report <LEVEL>     Enable usage reporting and report level [1-3].\n"
		"  -i | --ignore-materials   Ignore materials in the mesh file.\n"
		"       --no-triangle-api    Disable the Triangle API.\n"
		"App Keystrokes:\n"
		"  q  Quit\n"
		<< std::endl;

	exit(1);
}

int main2(int argc, char** argv)
{
	std::cout
		<< std::endl
		<< "      - - - - - - - - - - - - - - - - - - -     - - - - - - - - - - - - - - - - - - -    \n"
		<< "  - - - - - - - - - - - - - - - - - - - -   INIT   - - - - - - - - - - - - - - - - - - - - \n"
		<< "  - - - - - - - - - - - - - - - - - - - -          - - - - - - - - - - - - - - - - - - - -   \n"
		<< "      - - - - - - - - - - - - - - - - - - -     - - - - - - - - - - - - - - - - - - -    \n"
		<< "\n\n";

	std::string out_file;
	std::string mesh_file = std::string(mesh_path);
	int usage_report_level = 0;
	

		glutInitialize(&argc, argv);
		ImGuiInitialize();

#ifndef __APPLE__
		glewInit();
#endif

		UsageReportLogger logger;
		
		// create context && init RT_PROGRAMS
		createContext(usage_report_level, &logger);

		// load Mesh
		loadMesh(mesh_file);
		std::cout << "- [Report] : Object '"<< mesh_path<<"' loaded succsefully!" << std::endl;
				
		setupCamera();

		setupLights();

		context->validate();
		
		glutRun();

		return 0;
	
}

