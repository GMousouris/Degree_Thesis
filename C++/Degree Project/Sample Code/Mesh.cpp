
#include <optixu/optixu_matrix.h>
#include <optixu/optixu_math_stream_namespace.h>

#include "Mesh.h" 
#include "rply-1.01/rply.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include <algorithm>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <stdint.h>
#include <vector>

//------------------------------------------------------------------------------
//
// Helpers 
//
//------------------------------------------------------------------------------

namespace
{

void clearMesh( Mesh& mesh )
{
  memset( &mesh, 0, sizeof( mesh ) );
}


bool checkValid( const Mesh& mesh )
{
  if( mesh.num_vertices  == 0 )
  {
    std::cerr << "Mesh not valid: num_vertices = 0" << std::endl;
    return false;
  }
  if( mesh.positions == 0 )
  {
    std::cerr << "Mesh not valid: positions = NULL" << std::endl;
    return false;
  }
  if( mesh.num_triangles == 0 )
  {
    std::cerr << "Mesh not valid: num_triangles = 0" << std::endl;
    return false;
  }
  if( mesh.tri_indices == 0 )
  {
    std::cerr << "Mesh not valid: tri_indices = NULL" << std::endl;
    return false;
  }
  if( mesh.mat_indices == 0 )
  {
    std::cerr << "Mesh not valid: mat_indices = NULL" << std::endl;
    return false;
  }
  if( mesh.has_normals && !mesh.normals )
  {
    std::cerr << "Mesh has normals, but normals is NULL" << std::endl;
    return false;
  }
  if( mesh.has_texcoords && !mesh.texcoords )
  {
    std::cerr << "Mesh has texcoords, but texcoords is NULL" << std::endl;
    return false;
  }
  if ( mesh.num_materials == 0 )
  {
    std::cerr << "Mesh not valid: num_materials = 0" << std::endl;
    return false;
  }
  if ( mesh.mat_params == 0 )
  {
    std::cerr << "Mesh not valid: mat_params = 0" << std::endl;
    return false;
  }

  return true;
}


std::string directoryOfFilePath( const std::string& filepath )                 
{                                                                              
  size_t slash_pos, backslash_pos;                                             
  slash_pos     = filepath.find_last_of( '/' );                                
  backslash_pos = filepath.find_last_of( '\\' );                               

  size_t break_pos;                                                            
  if( slash_pos == std::string::npos && backslash_pos == std::string::npos ) { 
    return std::string();                                                      
  } else if ( slash_pos == std::string::npos ) {                               
    break_pos = backslash_pos;                                                 
  } else if ( backslash_pos == std::string::npos ) {                           
    break_pos = slash_pos;                                                     
  } else {                                                                     
    break_pos = std::max(slash_pos, backslash_pos);                            
  }                                                                            

  // Include the final slash                                                   
  return filepath.substr(0, break_pos + 1);                                    
}


std::string getExtension( const std::string& filename )                        
{                                                                              
  // Get the filename extension                                                
  std::string::size_type extension_index = filename.find_last_of( "." );       
  std::string ext =  extension_index != std::string::npos ?                                
                     filename.substr( extension_index+1 ) :                                
                     std::string();                                                        
  std::locale loc;
  for ( std::string::size_type i=0; i < ext.length(); ++i )
    ext[i] = std::tolower( ext[i], loc );

  return ext;
}                                                                              


bool fileIsOBJ( const std::string& filename )                                  
{                                                                              
  return getExtension( filename ) == "obj";                                    
}                                                                              


bool fileIsPLY( const std::string& filename )                                  
{                                                                              
  return getExtension( filename ) == "ply";                                    
}
  

struct PlyData
{
  Mesh* mesh;
  int32_t cur_vertex;
  int32_t cur_index;
};


  
void applyLoadXForm( Mesh& mesh, const float* load_xform )
{
  if( !load_xform )
      return;

  bool have_matrix = false;
  for( int32_t i = 0; i < 16; ++i )
    if( load_xform[i] != 0.0f )
      have_matrix = true;

  if( have_matrix )
  {
	
    mesh.bbox_min[0] = mesh.bbox_min[1] = mesh.bbox_min[2] =  1e16f;
    mesh.bbox_max[0] = mesh.bbox_max[1] = mesh.bbox_max[2] = -1e16f;

    optix::Matrix4x4 mat( load_xform );

    float3* positions = reinterpret_cast<float3*>( mesh.positions );
    for( int32_t i = 0; i < mesh.num_vertices; ++i )
    {
      const float3 v = optix::make_float3( mat*optix::make_float4( positions[i], 1.0f ) );
      positions[i] = v; 
      mesh.bbox_min[0] = std::min<float>( mesh.bbox_min[0], v.x );
      mesh.bbox_min[1] = std::min<float>( mesh.bbox_min[1], v.y );
      mesh.bbox_min[2] = std::min<float>( mesh.bbox_min[2], v.z );
      mesh.bbox_max[0] = std::max<float>( mesh.bbox_max[0], v.x );
      mesh.bbox_max[1] = std::max<float>( mesh.bbox_max[1], v.y );
      mesh.bbox_max[2] = std::max<float>( mesh.bbox_max[2], v.z );
	  mesh.average_spread += length(v);
    }
	mesh.average_spread /= mesh.num_vertices;

    if( mesh.has_normals )
    {
      mat = mat.inverse().transpose();
      float3* normals = reinterpret_cast<float3*>( mesh.normals );
      for( int32_t i = 0; i < mesh.num_vertices; ++i )
        normals[i] = optix::make_float3( mat*optix::make_float4( normals[i], 1.0f ) );
    }
  }
}

} 

//------------------------------------------------------------------------------
//
// MeshLoader implementation class
//
//------------------------------------------------------------------------------

class MeshLoader::Impl
{
public:
  Impl( const std::string& filename );
  
  void scanMesh( Mesh& mesh );
  void loadMesh(Mesh& mesh, const float* load_xform, bool as_groups = false, int shape_index = 0);


  void scanMeshOBJ( Mesh& mesh );
  void loadMeshOBJ( Mesh& mesh , bool as_groups = false, int shape_index = 0);

private:
  enum FileType
  {
    OBJ = 0,
    PLY,
    UNKNOWN
  };
  std::string                         m_filename;
  FileType                            m_filetype;
  
  std::vector<tinyobj::shape_t>       m_shapes;
  std::vector<tinyobj::material_t>    m_materials;
};


MeshLoader::Impl::Impl( const std::string& filename )
  : m_filename( filename )
{
   if( fileIsOBJ( m_filename ) )
     m_filetype = OBJ;
   else if( fileIsPLY( m_filename ) )
     m_filetype = PLY;
   else 
     m_filetype = UNKNOWN;
}


void MeshLoader::Impl::scanMesh( Mesh& mesh )
{
  clearMesh( mesh );

  if( m_filetype == OBJ )
    scanMeshOBJ( mesh );
  
  else
    throw std::runtime_error( "MeshLoader: Unsupported file type for '" + m_filename + "'" );
}


void MeshLoader::Impl::loadMesh( Mesh& mesh, const float* load_xform , bool as_groups, int shape_index)
{
  if( !checkValid( mesh ) )
  {
    std::cerr << "MeshLoader - ERROR: Attempted to load mesh '" << m_filename
              << "' into invalid mesh struct:" << std::endl;
    printMeshInfo( mesh, std::cerr );
    return;
  }
  
  mesh.bbox_min[0] = mesh.bbox_min[1] = mesh.bbox_min[2] =  1e16f;
  mesh.bbox_max[0] = mesh.bbox_max[1] = mesh.bbox_max[2] = -1e16f;

  if( m_filetype == OBJ )
    loadMeshOBJ( mesh , as_groups, shape_index );
 
  else
    throw std::runtime_error( "MeshLoader: Unsupported file type for '" + m_filename + "'" );

  applyLoadXForm( mesh, load_xform );
}


// EDW KANW OTI EINAI NA KANW ME TA SHAPES
void MeshLoader::Impl::scanMeshOBJ( Mesh& mesh )
{
  if( m_shapes.empty() )
  {
    std::string err;
    bool ret = tinyobj::LoadObj( 
        m_shapes,
        m_materials,
        err, 
        m_filename.c_str(),
        directoryOfFilePath( m_filename ).c_str()
        );

    if( !err.empty() )
      std::cerr << err << std::endl;

    if( !ret )
      throw std::runtime_error( "MeshLoader: " + err );

	//std::cout << " shapes : " << m_shapes.size() << std::endl;
  }

  //
  // Iterate over all shapes and sum up number of vertices and triangles
  //
  uint64_t num_groups_with_normals   = 0;
  uint64_t num_groups_with_texcoords = 0;
  for( std::vector<tinyobj::shape_t>::const_iterator it = m_shapes.begin();
       it < m_shapes.end();
       ++it )
  {
    const tinyobj::shape_t & shape = *it;

    mesh.num_triangles += static_cast<int32_t>(shape.mesh.indices.size()) / 3;
    mesh.num_vertices  += static_cast<int32_t>(shape.mesh.positions.size()) / 3;

	if (!shape.mesh.normals.empty())
	{
		++num_groups_with_normals;
	}

	if (!shape.mesh.texcoords.empty())
	{
		++num_groups_with_texcoords;
	}
	// create mesh.shape
	Mesh_Shape m_shape;
	m_shape.has_texcoords = !shape.mesh.texcoords.empty();
	m_shape.has_normals   = !shape.mesh.normals.empty();
	m_shape.num_vertices  = static_cast<int32_t>(shape.mesh.positions.size() / 3);
	m_shape.num_indices   = static_cast<int32_t>(shape.mesh.indices.size());
	m_shape.num_triangles = static_cast<int32_t>(shape.mesh.indices.size() / 3);
	m_shape.mat_ind_size  = static_cast<int32_t>(shape.mesh.material_ids.size());
	//m_shape.num_materials = static_cast<int32_t>(shape.mesh.material_ids.size());

	int diff_mats = 0;
	for (int i = 0; i < shape.mesh.material_ids.size(); i++)
	{
		int ind = shape.mesh.material_ids[i];
		bool diff = true;
		for (int j = 0; j < i; j++)
		{
			int ind_j = shape.mesh.material_ids[j];
			if (ind_j == ind)
			{
				diff = false;
				break;
			}
		}
		if (diff)
			diff_mats++;
	}
	m_shape.num_materials = static_cast<int32_t>(diff_mats);

	mesh.shapes.push_back(m_shape);

  }

  //
  // We ignore normals and texcoords unless they are present for all shapes
  //

  if( num_groups_with_normals != 0 )
  {
    if( num_groups_with_normals != m_shapes.size() )
      std::cerr << "MeshLoader - WARNING: mesh '" << m_filename 
                << "' has normals for some groups but not all.  "
                << "Ignoring all normals." << std::endl;
    else
      mesh.has_normals = true;

  }
  
  if( num_groups_with_texcoords != 0 )
  {
    if( num_groups_with_texcoords != m_shapes.size() )
      std::cerr << "MeshLoader - WARNING: mesh '" << m_filename 
                << "' has texcoords for some groups but not all.  "
                << "Ignoring all texcoords." << std::endl;
    else
      mesh.has_texcoords = true;
  }

  
  mesh.num_materials = (int32_t) m_materials.size();
 
}

void MeshLoader::Impl::loadMeshOBJ( Mesh& mesh , bool as_groups, int shape_index)

{
  uint32_t vrt_offset = 0;
  uint32_t tri_offset = 0;

 
  // load OBJ as a whole mesh object
  if (!as_groups)
  {
	  for (std::vector<tinyobj::shape_t>::const_iterator it = m_shapes.begin();
		   it < m_shapes.end();
		   ++it)
	  {

		  const tinyobj::shape_t & shape = *it;
		  for (uint64_t i = 0; i < shape.mesh.positions.size() / 3; ++i)
		  {
			  const float x = shape.mesh.positions[i * 3 + 0];
			  const float y = shape.mesh.positions[i * 3 + 1];
			  const float z = shape.mesh.positions[i * 3 + 2];

			  mesh.bbox_min[0] = std::min<float>(mesh.bbox_min[0], x);
			  mesh.bbox_min[1] = std::min<float>(mesh.bbox_min[1], y);
			  mesh.bbox_min[2] = std::min<float>(mesh.bbox_min[2], z);

			  mesh.bbox_max[0] = std::max<float>(mesh.bbox_max[0], x);
			  mesh.bbox_max[1] = std::max<float>(mesh.bbox_max[1], y);
			  mesh.bbox_max[2] = std::max<float>(mesh.bbox_max[2], z);

			  mesh.positions[vrt_offset * 3 + i * 3 + 0] = x;
			  mesh.positions[vrt_offset * 3 + i * 3 + 1] = y;
			  mesh.positions[vrt_offset * 3 + i * 3 + 2] = z;

			  //mesh.mat_indices_vert[vrt_offset * 3 + i * 3 + 0] = 0;
			  //mesh.mat_indices_vert[vrt_offset * 3 + i * 3 + 1] = 0;
			  //mesh.mat_indices_vert[vrt_offset * 3 + i * 3 + 2] = 0;
			  
		  }

		  if (mesh.has_normals)
			  for (uint64_t i = 0; i < shape.mesh.normals.size(); ++i)
				  mesh.normals[vrt_offset * 3 + i] = shape.mesh.normals[i];

		  if (mesh.has_texcoords)
			  for (uint64_t i = 0; i < shape.mesh.texcoords.size(); ++i)
				  mesh.texcoords[vrt_offset * 2 + i] = shape.mesh.texcoords[i];

		  for (uint64_t i = 0; i < shape.mesh.indices.size() / 3; ++i)
		  {
			  mesh.tri_indices[tri_offset * 3 + i * 3 + 0] = shape.mesh.indices[i * 3 + 0] + vrt_offset;
			  mesh.tri_indices[tri_offset * 3 + i * 3 + 1] = shape.mesh.indices[i * 3 + 1] + vrt_offset;
			  mesh.tri_indices[tri_offset * 3 + i * 3 + 2] = shape.mesh.indices[i * 3 + 2] + vrt_offset;
			  mesh.mat_indices[tri_offset + i            ] = shape.mesh.material_ids[i] >= 0 ? shape.mesh.material_ids[i] : 0;
			  
		  }

		  vrt_offset += static_cast<uint32_t>(shape.mesh.positions.size()) / 3;
		  tri_offset += static_cast<uint32_t>(shape.mesh.indices.size()) / 3;
	  }

	  for (uint64_t i = 0; i < m_materials.size(); ++i)
	  {
		  MaterialParams mat_params;

		  mat_params.name   = m_materials[i].name;
		  mat_params.Kd_map = m_materials[i].diffuse_texname.empty() ? "" :
			  directoryOfFilePath(m_filename) + m_materials[i].diffuse_texname;

		  mat_params.Kd[0] = m_materials[i].diffuse[0];
		  mat_params.Kd[1] = m_materials[i].diffuse[1];
		  mat_params.Kd[2] = m_materials[i].diffuse[2];

		  mat_params.Ks[0] = m_materials[i].specular[0];
		  mat_params.Ks[1] = m_materials[i].specular[1];
		  mat_params.Ks[2] = m_materials[i].specular[2];

		  mat_params.Ka[0] = m_materials[i].ambient[0];
		  mat_params.Ka[1] = m_materials[i].ambient[1];
		  mat_params.Ka[2] = m_materials[i].ambient[2];

		  mat_params.Kr[0] = m_materials[i].specular[0];
		  mat_params.Kr[1] = m_materials[i].specular[1];
		  mat_params.Kr[2] = m_materials[i].specular[2];

		  mat_params.exp = m_materials[i].shininess;

		  mesh.mat_params[i] = mat_params;
	  }
  }

  // load OBJ as seperate mesh - shapes objects
  else
  {

	  
	  tinyobj::shape_t shape = m_shapes[shape_index];
	  for (uint64_t i = 0; i < shape.mesh.positions.size() / 3; ++i)
	  {
		  const float x = shape.mesh.positions[i * 3 + 0];
		  const float y = shape.mesh.positions[i * 3 + 1];
		  const float z = shape.mesh.positions[i * 3 + 2];

		  mesh.bbox_min[0] = std::min<float>(mesh.bbox_min[0], x);
		  mesh.bbox_min[1] = std::min<float>(mesh.bbox_min[1], y);
		  mesh.bbox_min[2] = std::min<float>(mesh.bbox_min[2], z);

		  mesh.bbox_max[0] = std::max<float>(mesh.bbox_max[0], x);
		  mesh.bbox_max[1] = std::max<float>(mesh.bbox_max[1], y);
		  mesh.bbox_max[2] = std::max<float>(mesh.bbox_max[2], z);

		  mesh.positions[vrt_offset * 3 + i * 3 + 0] = x;
		  mesh.positions[vrt_offset * 3 + i * 3 + 1] = y;
		  mesh.positions[vrt_offset * 3 + i * 3 + 2] = z;
	  }

	  if (mesh.has_normals)
		  for (uint64_t i = 0; i < shape.mesh.normals.size(); ++i)
			  mesh.normals[vrt_offset * 3 + i] = shape.mesh.normals[i];

	  if (mesh.has_texcoords)
		  for (uint64_t i = 0; i < shape.mesh.texcoords.size(); ++i)
			  mesh.texcoords[vrt_offset * 2 + i] = shape.mesh.texcoords[i];

	  for (uint64_t i = 0; i < shape.mesh.indices.size() / 3; ++i)
	  {
		  mesh.tri_indices[tri_offset * 3 + i * 3 + 0] = shape.mesh.indices[i * 3 + 0] + vrt_offset;
		  mesh.tri_indices[tri_offset * 3 + i * 3 + 1] = shape.mesh.indices[i * 3 + 1] + vrt_offset;
		  mesh.tri_indices[tri_offset * 3 + i * 3 + 2] = shape.mesh.indices[i * 3 + 2] + vrt_offset;
		  mesh.mat_indices[tri_offset + i] = shape.mesh.material_ids[i] >= 0 ? shape.mesh.material_ids[i] : 0;
	  }

	  vrt_offset += static_cast<uint32_t>(shape.mesh.positions.size()) / 3;
	  tri_offset += static_cast<uint32_t>(shape.mesh.indices.size()) / 3;

	  // na ftia3w auto na mhn ginetai 2 fores
	  int diff_mats = 0;
	  for (int i = 0; i < shape.mesh.material_ids.size(); i++)
	  {
		  int ind = shape.mesh.material_ids[i];
		  bool diff = true;
		  for (int j = 0; j < i; j++)
		  {
			  int ind_j = shape.mesh.material_ids[j];
			  if (ind_j == ind)
			  {
				  diff = false;
				  break;
			  }
		  }
		  if (diff)
			  diff_mats++;
	  }

	  for (uint64_t i = 0; i < diff_mats; ++i)
	  {
		  MaterialParams mat_params;
		  int mat_index = shape.mesh.material_ids[i];
		  if (mat_index < 0)
			  mat_index = 0;

		  ////std::cout << "shape_index[" << shape_index << "] mat_index : " << mat_index << std::endl;

		  mat_params.name = m_materials[mat_index].name;
		  mat_params.Kd_map = m_materials[mat_index].diffuse_texname.empty() ? "" :
			  directoryOfFilePath(m_filename) + m_materials[mat_index].diffuse_texname;

		  mat_params.Kd[0] = m_materials[mat_index].diffuse[0];
		  mat_params.Kd[1] = m_materials[mat_index].diffuse[1];
		  mat_params.Kd[2] = m_materials[mat_index].diffuse[2];

		  mat_params.Ks[0] = m_materials[mat_index].specular[0];
		  mat_params.Ks[1] = m_materials[mat_index].specular[1];
		  mat_params.Ks[2] = m_materials[mat_index].specular[2];

		  mat_params.Ka[0] = m_materials[mat_index].ambient[0];
		  mat_params.Ka[1] = m_materials[mat_index].ambient[1];
		  mat_params.Ka[2] = m_materials[mat_index].ambient[2];

		  mat_params.Kr[0] = m_materials[mat_index].specular[0];
		  mat_params.Kr[1] = m_materials[mat_index].specular[1];
		  mat_params.Kr[2] = m_materials[mat_index].specular[2];

		  mat_params.exp = m_materials[mat_index].shininess;
		  mesh.mat_params[i] = mat_params;
	  }
  }
  

 
}


//------------------------------------------------------------------------------
//
//  Mesh API free functions
//
//------------------------------------------------------------------------------

void printMaterialInfo( const MaterialParams& mat, std::ostream& out )
{
  out << "MaterialParams[ " << mat.name << " ]:" << std::endl
      << "\tmat.Kd map: '" << mat.Kd_map << "'" << std::endl 
      << "\tmat.Kd val: ( " << mat.Kd[0] << ", " << mat.Kd[1] << ", " << mat.Kd[2] << " )" << std::endl
      << "\tmat.Ks val: ( " << mat.Ks[0] << ", " << mat.Ks[1] << ", " << mat.Ks[2] << " )" << std::endl
      << "\tmat.Kr val: ( " << mat.Kr[0] << ", " << mat.Kr[1] << ", " << mat.Kr[2] << " )" << std::endl
      << "\tmat.Ka val: ( " << mat.Ka[0] << ", " << mat.Ka[1] << ", " << mat.Ka[2] << " )" << std::endl
      << "\tExp   : " << mat.exp << std::endl;
}


void printMeshInfo( const Mesh& mesh, std::ostream& out )
{
  out << "Mesh:" << std::endl
      << "\tnum vertices : " << mesh.num_vertices  << std::endl
      << "\thas normals  : " << mesh.has_normals   << std::endl
      << "\thas texcoords: " << mesh.has_texcoords << std::endl
      << "\tnum triangles: " << mesh.num_triangles << std::endl
      << "\tnum materials: " << mesh.num_materials << std::endl
      << "\tbbox min     : ( " << mesh.bbox_min[0] << ", "
                               << mesh.bbox_min[1] << ", "
                               << mesh.bbox_min[2] << " )"
                               << std::endl 
      << "\tbbox max     : ( " << mesh.bbox_max[0] << ", "
                               << mesh.bbox_max[1] << ", "
                               << mesh.bbox_max[2] << " )"
                               << std::endl;
  /*
  if( mesh.positions )
  {
    out << "\tpositions:" << std::endl;
    for( uint64_t i = 0; i < mesh.num_vertices; ++i )
      out << "\t\t( " << mesh.positions[i*3+0] << ", "
                      << mesh.positions[i*3+1] << ", "
                      << mesh.positions[i*3+2] << " )"
                      << std::endl;
  }
  if( mesh.tri_indices )
  {
    out << "\ttri indices:" << std::endl;
    for( uint64_t i = 0; i < mesh.num_triangles; ++i )
      out << "\t\t( " << mesh.tri_indices[i*3+0] << ", "
                      << mesh.tri_indices[i*3+1] << ", "
                      << mesh.tri_indices[i*3+2] << " )"
                      << std::endl;
  }
  if( mesh.mat_indices )
  {
    out << "\tmat indices:" << std::endl;
    for( uint64_t i = 0; i < mesh.num_triangles; ++i )
      out << "\t\t" << mesh.mat_indices[i] << std::endl; 
  }
  */
}


void allocMesh( Mesh& mesh )
{

  if( mesh.num_vertices == 0 || mesh.num_triangles == 0 )
  {
    clearMesh( mesh );
    return;
  }

  mesh.positions   = new float[ 3*mesh.num_vertices ];
  mesh.normals     = mesh.has_normals   ? new float[ 3*mesh.num_vertices ]   : 0;
  mesh.texcoords   = mesh.has_texcoords ? new float[ 2*mesh.num_vertices ]   : 0;
  mesh.tri_indices = new int32_t[ 3*mesh.num_triangles ]; 
  mesh.mat_indices = new int32_t[ 1*mesh.num_triangles ]; 

  mesh.mat_params  = new MaterialParams[ mesh.num_materials ];
}


void freeMesh( Mesh& mesh )
{
  delete [] mesh.positions;
  delete [] mesh.normals;
  delete [] mesh.texcoords;
  delete [] mesh.tri_indices;
  delete [] mesh.mat_indices;
  delete [] mesh.mat_params;

  clearMesh( mesh );
}


//------------------------------------------------------------------------------
//
//  Mesh API MeshLoader class 
//
//------------------------------------------------------------------------------

MeshLoader::MeshLoader( const std::string& filename )
  : p_impl( new Impl( filename ) )
{
}


MeshLoader::~MeshLoader()
{
  delete p_impl;
}


void MeshLoader::scanMesh( Mesh& mesh )
{
  p_impl->scanMesh( mesh );
}


void MeshLoader::loadMesh( Mesh& mesh, const float* load_xform , bool as_groups, int shape_index )
{
  p_impl->loadMesh( mesh, load_xform , as_groups, shape_index );
}

//------------------------------------------------------------------------------
//
// Mesh Loader convenience  functions
//
//------------------------------------------------------------------------------


void loadMesh( const std::string& filename, Mesh& mesh, const float* xform )
{
    MeshLoader loader( filename );
    loader.scanMesh( mesh );
    allocMesh( mesh );
    loader.loadMesh( mesh, xform );
}
