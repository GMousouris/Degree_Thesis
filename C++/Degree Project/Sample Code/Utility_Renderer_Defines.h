#pragma once
#include <string>
#include <optixu/optixpp_namespace.h>

#define LINKING_SCALE_FACTOR 0.018f
#define UTILITY_SCALE_FACTOR 0.78f
#define TRANSLATION_PLANES_SCALE_FACTOR 0.78f
#define ROTATION_AXIS_SCALE_FACTOR 0.082823f // 0.079
#define TRANSLATION_AXIS_SCALE_FACTOR 0.02933f
#define SCALE_AXIS_SCALE_FACTOR 0.037833f


/*
#define UTILITY_SCALE_FACTOR 0.78f
#define ROTATION_AXIS_SCALE_FACTOR 0.076823f
#define TRANSLATION_AXIS_SCALE_FACTOR 0.03333f
#define SCALE_AXIS_SCALE_FACTOR 0.03333f
*/


#define NEAR 0.1f  // 0.1
#define FAR 9999999999.f // 1500
#define M_FOV 45.0f

#define PLANE_HIGHLIGHT_COLOR  glm::vec3(1.0f,1.0f,1.0f)
#define HIGHLIGHT_COLOR        glm::vec3(1.0f,1.0f,0.0f)
#define SELECTED_COLOR         glm::vec3(0, 255.0f, 195.0f)/255.0f
#define SELECTED_COLOR_3f      optix::make_float3(0.0f,255.0f,195.0f)/255.0f
#define DEF_AXIS_COLOR_X       glm::vec3(1.0f,0.0f,0.0f) // glm::vec3(0.75,0,0)
#define DEF_AXIS_COLOR_Y       glm::vec3(0.0f,1.f,0.0f)  // glm::vec3(0,0.8,0)
#define DEF_AXIS_COLOR_Z       glm::vec3(0.0,0.0,1.0f)   // glm::vec3(0.1,0.1,0.8)

#define CIRCLE_SEGMENTS 35
#define UTILITY_LINE_WIDTH 1.0f // 1.45f //
#define OUTLINE_LINE_WIDTH 5.0f //       //

#define M_FOCUS_SPHERE_SIZE 0.0037f

#define BBOX_TEST

#define FACE_VECTOR_ARROW_SCALE_FACTOR 1.f
#define FACE_VECTOR_LINE_WIDTH 1.0f
#define FACE_VECTOR_SCALE_FACTOR 4.0f
#define RES_CONE_FRUSTUM_COLOR glm::vec3(1.0f,1.0f,0.45f)

#define SAMPLER_COLOR_FILL glm::vec3(0.1f, 1.0f, 0.4f)
#define SAMPLER_FILL_ALPHA 0.22

#define SAMPLER_OUTLINE_THICKNESS 2.0f
#define SAMPLER_OUTLINE_COLOR glm::vec3(0.15f)
#define SAMPLER_OUTLINE_ALPHA 0.5


//#define distance_R_visualization_old
#define distance_R_visualization_new

//#define GET_RENDER_TIME_METRICS