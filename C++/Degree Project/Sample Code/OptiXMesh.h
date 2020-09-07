
#pragma once


#include "Mesh.h"
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_matrix_namespace.h>


//------------------------------------------------------------------------------
//
// OptiX mesh consisting of a single geometry instance with one or more
// materials.
//
//   Mesh buffer variables are set on Geometry node:
//   vertex_buffer  : float3 vertex positions
//   normal_buffer  : float3 per vertex normals, may be zero length 
//   texcoord_buffer: float2 vertex texture coordinates, may be zero length
//   index_buffer   : int3 indices shared by vertex, normal, texcoord buffers 
//   material_buffer: int indices into material list
//
//------------------------------------------------------------------------------

struct MeshBuffers
{
	optix::Buffer tri_indices;
	optix::Buffer mat_indices;
	optix::Buffer positions;
	optix::Buffer normals;
	optix::Buffer texcoords;
};

struct data_pointers
{
	float * positions;
	int * tri_indices;
	float * normals;
	float * texcoords;
};


struct OptiXMesh
{
 

  OptiXMesh()
    : use_tri_api( true )
    , ignore_mats( false )
    , num_triangles( 0 )
  {

  }

  // Input
  optix::Context               context;       // required
  optix::Material              material;      // optional single matl override

  optix::Program               intersection;  // optional 
  optix::Program               bounds;        // optional
  
  optix::Program               closest_hit;   // optional multi matl override
  optix::Program               any_hit;       // optional

  bool                         use_tri_api;   // optional
  bool                         ignore_mats;   // optional

  // Output
  MeshBuffers                  buffers;
  optix::GeometryInstance      geom_instance;
  optix::float3                bbox_min;
  optix::float3                bbox_max;
  float                        average_spread = 0.0f;

  int                          num_triangles;

  std::vector<std::string>     mat_names;
  std::vector<int>             mat_has_tex;
  std::vector<OptiXMesh>       optix_shapes;
  std::string filepath;

  std::vector<std::string> tex_maps;

  void clear();

};


void loadMesh(
    const std::string&        filename,
    OptiXMesh&                optix_mesh, 
	bool                      load_as_group,
    const optix::Matrix4x4&   load_xform = optix::Matrix4x4::identity()
    );
