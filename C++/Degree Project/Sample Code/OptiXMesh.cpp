

#include <optixu/optixu_math_namespace.h>

#include "Mesh.h"
#include "OptiXMesh.h"
#include "Utilities.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <fstream>


void OptiXMesh::clear()
{

	for (int i = 0; i < geom_instance->getMaterialCount(); i++)
		geom_instance->getMaterial(i)->destroy();
	

	geom_instance->destroy();
	buffers.mat_indices->destroy();
	buffers.normals->destroy();
	buffers.positions->destroy();
	buffers.texcoords->destroy();
	buffers.tri_indices->destroy();
}


namespace optix 
{
  float3 make_float3( const float* a )
  {
    return make_float3( a[0], a[1], a[2] );
  }
}


namespace
{





void setupMeshLoaderInputs(
    optix::Context            context, 
    MeshBuffers&              buffers,
    Mesh&                     mesh
    )
{

	
  buffers.tri_indices = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3,   mesh.num_triangles );
  buffers.mat_indices = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT,    mesh.num_triangles );
  buffers.positions   = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, mesh.num_vertices );
  buffers.normals     = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3,
                                               mesh.has_normals ? mesh.num_vertices : 0);
  buffers.texcoords   = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT2,
                                               mesh.has_texcoords ? mesh.num_vertices : 0);

  mesh.tri_indices = reinterpret_cast<int32_t*>( buffers.tri_indices->map() );
  mesh.mat_indices = reinterpret_cast<int32_t*>( buffers.mat_indices->map() );
  mesh.positions   = reinterpret_cast<float*>  ( buffers.positions->map() );
  mesh.normals     = reinterpret_cast<float*>  ( mesh.has_normals   ? buffers.normals->map()   : 0 );
  mesh.texcoords   = reinterpret_cast<float*>  ( mesh.has_texcoords ? buffers.texcoords->map() : 0 );
  mesh.mat_params  = new MaterialParams[ mesh.num_materials ];
  
}



void unmap( MeshBuffers& buffers, Mesh& mesh )
{
	
  buffers.tri_indices->unmap();
  buffers.mat_indices->unmap();
  buffers.positions->unmap();
  if( mesh.has_normals )
    buffers.normals->unmap();
  if( mesh.has_texcoords)
    buffers.texcoords->unmap();

  

  mesh.tri_indices = 0; 
  mesh.mat_indices = 0;
  mesh.positions   = 0;
  mesh.normals     = 0;
  mesh.texcoords   = 0;

  delete [] mesh.mat_params;
  mesh.mat_params = 0;
}


void createMaterialPrograms(
    optix::Context  context,
    bool            use_textures,
    optix::Program& closest_hit,
    optix::Program& any_hit
    )
{
	if(!use_textures)
		closest_hit = Mediator::request_ActiveClosestHitProgram();
	else
		closest_hit = Mediator::request_ActiveClosestHitTexturedProgram();

	any_hit = Mediator::request_ActiveAnyHitProgram();
    
	// const char * ptxPath = Utilities::getPtxPath("phong.cu").c_str();
    // WARNING, if this material is going to be used by triangles as well as custom primitives,
    // "closest_hit_radiance" and "closest_hit_radiance_textured" must be used.
    // In this sample, the material is only used by one of them
	
}


optix::Material createOptiXMaterial(
    optix::Context         context,
    optix::Program         closest_hit,
    optix::Program         any_hit,
    const MaterialParams&  mat_params,
    bool                   use_textures,
	bool                   check,
	OptiXMesh&             optix_mesh
    )
{

	if (false)
	{
		int code;
		context->checkError((RTresult)code);	
	}

  optix::Material mat = context->createMaterial();
  mat->setClosestHitProgram( 0u, closest_hit );             
  mat->setAnyHitProgram( 1u, any_hit ) ;    

  
  if ( use_textures )
  {

	  std::string Kd_map_file = mat_params.Kd_map;
	  std::replace(Kd_map_file.begin(), Kd_map_file.end(), '\\', '/');

	  ////std::cout << "\n - Requesting Texture : " << Kd_map_file << std::endl;
	  std::string   file = Kd_map_file;
	  std::ifstream check( file.c_str() );
	  bool exists = check.good();
	  if (!exists
		  //&& false
		  )
	  {
		  ////std::cout << "  - Tex doesn't exist!" << std::endl;
		  std::string check_loc = TEMP_DATA_MESH_FOLDER_TEX_MAP_PATH + Utilities::getFileName(Kd_map_file, true);
		  std::ifstream check(check_loc.c_str());
		  bool exists = check.good();
		  ////std::cout << "  - checking location : " << check_loc << std::endl;
		  if (exists)
		  {
			  ////std::cout << "  - location B : Exists!" << std::endl;
			  Kd_map_file = check_loc;
		  }
		  else
		  {
			  ////std::cout << " - location B : Not Exists!" << std::endl;
		  }
	  }

	  
	  optix_mesh.tex_maps.push_back(Kd_map_file);
	  ////std::cout << " ~ tex_map : " << Kd_map_file << std::endl;

	  mat["Kd_map"]->setTextureSampler(Utilities::loadTexture(context, Kd_map_file , optix::make_float3(mat_params.Kd)));
	  
  }
  else
  {
	  mat["Kd_map"]->setTextureSampler(Utilities::loadTexture(context, "", optix::make_float3(mat_params.Kd)));
  }

  mat[ "Kd_mapped" ]->setInt( use_textures  );
  mat[ "Kd"        ]->set3fv( mat_params.Kd );
  mat[ "Ks"        ]->set3fv( mat_params.Ks );
  mat[ "Kr"        ]->set3fv( mat_params.Kr );
  mat[ "Ka"        ]->set3fv( mat_params.Ka );
  mat[ "phong_exp" ]->setFloat( mat_params.exp ); 
  
  return mat;

}


optix::Program createBoundingBoxProgram( optix::Context context )
{
    return context->createProgramFromPTXFile( Utilities::getPtxPath("triangle_mesh.cu"), "mesh_bounds" );
}


optix::Program createIntersectionProgram( optix::Context context )
{
    return context->createProgramFromPTXFile( Utilities::getPtxPath("triangle_mesh.cu"), "mesh_intersect" );
}


optix::Program createAttributesProgram( optix::Context context )
{
	return context->createProgramFromPTXFile( Utilities::getPtxPath("triangle_mesh.cu"), "mesh_attributes" );
}

void translateShapeToMesh(
	const Mesh_Shape* shape,
	Mesh& mesh)
{
	mesh.num_materials = shape->num_materials;
	mesh.num_triangles = shape->num_triangles;
	mesh.num_vertices = shape->num_vertices;
	mesh.has_normals = shape->has_normals;
	mesh.has_texcoords = shape->has_texcoords;
}

void translateMeshToOptiX(
    const Mesh&        mesh,
    const MeshBuffers& buffers,
    OptiXMesh&         optix_mesh
    )
{
  optix::Context ctx        = optix_mesh.context;
  optix_mesh.bbox_min       = optix::make_float3( mesh.bbox_min );
  optix_mesh.bbox_max       = optix::make_float3( mesh.bbox_max );
  optix_mesh.num_triangles  = mesh.num_triangles;
  optix_mesh.average_spread = mesh.average_spread;

  std::vector<optix::Material> optix_materials;
  if( optix_mesh.ignore_mats )
  {
    bool have_textures = false;
    MaterialParams default_params = {
        "", "",
        { 0.7f, 0.7f, 0.7f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        1.0f
    };

    optix::Program closest_hit = optix_mesh.closest_hit;
    optix::Program any_hit     = optix_mesh.any_hit;
    createMaterialPrograms( ctx, have_textures, closest_hit, any_hit );

    optix::Material mtl = createOptiXMaterial( ctx,
                                               closest_hit,
                                               any_hit,
                                               default_params,
                                               have_textures, 
											   false,
											   optix_mesh);

    optix_materials.push_back( mtl );
	optix_mesh.mat_names.push_back(std::string("material"));
	optix_mesh.mat_has_tex.push_back((have_textures)? 1 : 0);

    // Rewrite all mat_indices to point to single override material
    memset( mesh.mat_indices, 0, mesh.num_triangles*sizeof(int32_t) );
  }
  else if( optix_mesh.material )
  {
    // Rewrite all mat_indices to point to single override material
    memset( mesh.mat_indices, 0, mesh.num_triangles*sizeof(int32_t) );

    optix_materials.push_back( optix_mesh.material );
	optix_mesh.mat_names.push_back(std::string("material"));
	optix_mesh.mat_has_tex.push_back(0);
  }
  else
  {

    bool have_textures = false;
    for( int32_t i = 0; i < mesh.num_materials; ++i )
      if( !mesh.mat_params[i].Kd_map.empty() )
        have_textures = true;

	

    optix::Program closest_hit = optix_mesh.closest_hit;
    optix::Program any_hit     = optix_mesh.any_hit;
    createMaterialPrograms( ctx, have_textures, closest_hit, any_hit );

	
	for (int32_t i = 0; i < mesh.num_materials; ++i)
	{
		optix_materials.push_back(createOptiXMaterial(
			ctx,
			closest_hit,
			any_hit,
			mesh.mat_params[i],
			have_textures, false,
			optix_mesh));

		optix_mesh.mat_names.push_back(std::string(mesh.mat_params[i].name));
		optix_mesh.mat_has_tex.push_back((int)have_textures);
		
	}
  }

  if( optix_mesh.use_tri_api )
  {
	
    optix::GeometryTriangles geom_tri = ctx->createGeometryTriangles();
    geom_tri->setPrimitiveCount(  mesh.num_triangles );
    geom_tri->setTriangleIndices( buffers.tri_indices, RT_FORMAT_UNSIGNED_INT3 );
    geom_tri->setVertices( mesh.num_vertices, buffers.positions, buffers.positions->getFormat() );
    geom_tri->setBuildFlags( RTgeometrybuildflags(0) );
    geom_tri->setAttributeProgram( createAttributesProgram( ctx ) );

    size_t num_matls = optix_materials.size();
    geom_tri->setMaterialCount( num_matls );
    geom_tri->setMaterialIndices( buffers.mat_indices, 0, sizeof( unsigned ), RT_FORMAT_UNSIGNED_INT );

    optix_mesh.geom_instance = ctx->createGeometryInstance();
    optix_mesh.geom_instance->setGeometryTriangles( geom_tri );

    // Set the materials
    optix_mesh.geom_instance->setMaterialCount( num_matls );
    for( size_t idx = 0; idx < num_matls; ++idx )
    {
      optix_mesh.geom_instance->setMaterial( idx, optix_materials[idx] );
    }
  }
  else
  {
    optix::Geometry geometry = ctx->createGeometry();
    geometry->setPrimitiveCount     ( mesh.num_triangles );
    geometry->setBoundingBoxProgram ( optix_mesh.bounds ?
                                      optix_mesh.bounds :
                                      createBoundingBoxProgram( ctx ) );
    geometry->setIntersectionProgram( optix_mesh.intersection ?
                                      optix_mesh.intersection :
                                      createIntersectionProgram( ctx ) );

    optix_mesh.geom_instance = ctx->createGeometryInstance(
      geometry,
      optix_materials.begin(),
      optix_materials.end()
      );
  }

  

  // Put these on the GeometryInstance, not Geometry, to be compatible with the Triangle API.
  optix_mesh.geom_instance[ "vertex_buffer"   ]->setBuffer( buffers.positions   );
  optix_mesh.geom_instance[ "normal_buffer"   ]->setBuffer( buffers.normals     );
  optix_mesh.geom_instance[ "texcoord_buffer" ]->setBuffer( buffers.texcoords   );
  optix_mesh.geom_instance[ "index_buffer"    ]->setBuffer( buffers.tri_indices );
  optix_mesh.geom_instance[ "material_buffer" ]->setBuffer( buffers.mat_indices );

  // save buffers to OptixMesh object
  optix_mesh.buffers.positions   = buffers.positions;
  optix_mesh.buffers.normals     = buffers.normals;
  optix_mesh.buffers.texcoords   = buffers.texcoords;
  optix_mesh.buffers.tri_indices = buffers.tri_indices;
  optix_mesh.buffers.mat_indices = buffers.mat_indices;

}


} // namespace end


void loadMesh(
    const std::string&          filename,
    OptiXMesh&                  optix_mesh,
	bool                        load_as_group,
    const optix::Matrix4x4&     load_xform
    )
{
  if( !optix_mesh.context )
  {
    throw std::runtime_error( "OptiXMesh: loadMesh() requires valid OptiX context" );
  }

  optix::Context context = optix_mesh.context;

  Mesh mesh;
  MeshLoader loader( filename );
  loader.scanMesh( mesh );

  
  MeshBuffers buffers;
  setupMeshLoaderInputs( context, buffers, mesh );
  loader.loadMesh(mesh, load_xform.getData());
  translateMeshToOptiX(mesh, buffers, optix_mesh);
  unmap(buffers, mesh);
 

}


