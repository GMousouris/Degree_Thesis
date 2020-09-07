#include "Mesh_Pool.h"
#include <optixu/optixpp_namespace.h>
#include "OBJLoader.h"
#include "Utilities.h"
#include "SceneObject.h"
#include "OpenGL_Renderer.h"
#include "Defines.h"
#include <fstream>
#include <direct.h>
#include <streambuf>
#include <sstream>
#include <iterator>

Mesh_Pool::Mesh_Pool()
{

}

Mesh_Pool::~Mesh_Pool()
{

	//for (auto& it : m_OpenGL_meshes)
	//	delete it.second;

	//for (auto& it : m_Optix_meshes)
	//	it.second.clear();

	clear_temp_files();

	m_OpenGL_meshes.clear();
	m_Optix_meshes.clear();
	m_temp_mesh_paths.clear();
	m_temp_mtl_paths.clear();

	
}



void Mesh_Pool::Load_Mesh(std::string filename , bool as_instances , bool is_loading_scene )
{
	//std::cout << "\n\n - Mesh_Pool::Loading_Mesh( filename = " << filename << " ) "<< std::endl;

	// Load Mesh as OptixMesh //
	optix::Context m_context = Mediator::RequestContext();
	m_Optix_meshes[ filename ] = Utilities::LoadMesh( filename.c_str(), m_context, false );

	// Load Mesh as GeometryNode //
	OBJLoader loader;
	GeometryNode  * node = new GeometryNode();
	GeometricMesh * mesh = loader.load( filename.c_str() );
	node->Init_Instance( mesh );
	m_OpenGL_meshes[ filename ] = node;


	// copy mesh files ( .OBJ ) temporarily to temp data folder //
	std::string dest_path = "";
	std::string mesh_name        = Utilities::getFileName(filename, true);
	std::string mesh_name_wo_ext = Utilities::getFileName(filename, false);
	dest_path                    = TEMP_DATA_MESH_FOLDER_PATH + mesh_name;
	if (!is_loading_scene)
	{
		std::ifstream  src(filename , std::ios::binary);
		std::ofstream  dst(dest_path, std::ios::binary);
		dst << src.rdbuf();
	}
	m_temp_mesh_paths[filename] = dest_path;
	
	
	
	// copy mesh files ( .MTL ) temporarily to temp data folder //
	std::string mtl_path = Utilities::getFilePath(filename) + mesh_name_wo_ext + std::string(".mtl");
	dest_path            = TEMP_DATA_MESH_FOLDER_PATH + mesh_name_wo_ext + std::string(".mtl");
	if (!is_loading_scene)
	{
		std::ifstream check(mtl_path.c_str());
		if (check.good())
		{
			std::ifstream  src_mtl(mtl_path , std::ios::binary);
			std::ofstream  dst_mtl(dest_path, std::ios::binary);
			dst_mtl << src_mtl.rdbuf();
			m_temp_mtl_paths[filename] = dest_path;
		}
	}
	else
	{
		m_temp_mtl_paths[filename] = dest_path;
	}


	// copy map files temporarily to temp data folder //
   #define TEMP_STORE_TEX_MAP_FILES
#ifdef TEMP_STORE_TEX_MAP_FILES
	{
		////std::cout << "  > Temporarily Exporting Tex Maps!" << std::endl;
		dest_path = TEMP_DATA_MESH_FOLDER_TEX_MAP_PATH;
		std::vector<std::string> tex_maps_to_export = m_Optix_meshes[ filename ].tex_maps;
		for (std::string tex_map : tex_maps_to_export)
		{
			std::string tex_map_filename = Utilities::getFileName(tex_map, true);
			if (!is_loading_scene)
			{
				std::ifstream  src_texmap(tex_map, std::ios::binary);
				std::ofstream  dst_texmap(dest_path + tex_map_filename, std::ios::binary);
				dst_texmap << src_texmap.rdbuf();
			}
			m_temp_texmaps_paths[filename].push_back( dest_path + tex_map_filename );
		}
	}
#endif
	

}

void           Mesh_Pool::delete_Mesh(std::string key)
{

}

bool           Mesh_Pool::is_Mesh_Available(std::string key)
{
	for ( auto& it : m_Optix_meshes )
		if ( it.first == key )
			return true;

	return false;
}

OptiXMesh    * Mesh_Pool::request_Mesh_for_Optix(std::string key)
{
	return &m_Optix_meshes[key];
}
OptiXMesh      Mesh_Pool::request_Mesh_Instace_Optix(std::string key , SceneObject* source )
{
	return Utilities::getMeshInstance_from_Mesh( &m_Optix_meshes[key] , source );
}
GeometryNode * Mesh_Pool::request_Mesh_for_OpenGL(std::string key)
{
	return m_OpenGL_meshes[key];
}
GeometryNode * Mesh_Pool::request_Mesh_for_OpenGL( SceneObject* source )
{
	return request_Mesh_for_OpenGL( source->get_MeshId() );
}
GeometryObject_OpenGL*Mesh_Pool::request_Mesh_for_OpenGL2(std::string key)
{
	return m_OpenGL_meshes2[key];
}
GeometryObject_OpenGL*Mesh_Pool::request_Mesh_for_OpenGL2(SceneObject* source)
{
	return request_Mesh_for_OpenGL2(source->get_MeshId());
}


void  Mesh_Pool::create_GeometryNode_Material( SceneObject* obj )
{
	GeometryNode* node = request_Mesh_for_OpenGL(obj);
	node->create_Material(obj);
}
void  Mesh_Pool::Update_GeometryNode_Material(class SceneObject* obj)
{
	GeometryNode* node = request_Mesh_for_OpenGL(obj);
	//if(node != nullptr)
		node->Update_Material(obj);
}
void  Mesh_Pool::remove_GeometryNode_Material( SceneObject* obj )
{
	GeometryNode* node = request_Mesh_for_OpenGL(obj);
	if( node != nullptr )
		node->remove_Material(obj->getId());
}

void Mesh_Pool::clear_temp_files()
{

	for (auto& it : m_temp_mesh_paths)
	{
		std::string mesh_path = it.second;
		if (remove(mesh_path.c_str()) != 0)
		{

		}
	}
	m_temp_mesh_paths.clear();


	for (auto& it : m_temp_mtl_paths)
	{
		std::string mtl_path = it.second;
		if (remove(mtl_path.c_str()) != 0)
		{

		}
	}
	m_temp_mtl_paths.clear();


	for (auto& it : m_temp_texmaps_paths)
	{
		for (std::string tex_map_file : it.second)
		{
			if (remove(tex_map_file.c_str()) != 0)
			{

			}
		}
		it.second.clear();
	}
	m_temp_texmaps_paths.clear();

}
void Mesh_Pool::clear(std::string mesh_id)
{
	delete m_OpenGL_meshes[mesh_id];
	m_Optix_meshes[mesh_id].clear();

	m_OpenGL_meshes.erase(mesh_id);
	m_Optix_meshes.erase(mesh_id);
}
void Mesh_Pool::clear()
{
	for (auto& it : m_OpenGL_meshes)
	{
		delete it.second;
		m_OpenGL_meshes.erase(it.first);

	}

	for (auto& it : m_Optix_meshes)
	{
		it.second.clear();
		m_Optix_meshes.erase(it.first);
	}

	m_OpenGL_meshes.clear();
	m_Optix_meshes.clear();

	clear_temp_files();

		
}
void Mesh_Pool::Export_Mesh_Files( const char * path )
{

	for (auto& it : m_Optix_meshes)
	{

		std::string src_mesh_filename = Utilities::getFileName(it.first, true);
		std::string src_mtl_filename  = Utilities::getFileName(it.first, false) + ".mtl";

		//std::string filename  = it.first;
		std::string src_mesh_file = TEMP_DATA_MESH_FOLDER_PATH + src_mesh_filename;
		std::string src_mtl_file  = TEMP_DATA_MESH_FOLDER_PATH + src_mtl_filename;


		std::string dest_mesh_file = std::string( path ) + "/" + src_mesh_filename;
		std::string dest_mtl_file  = std::string( path ) + "/" + src_mtl_filename;

		std::ifstream  src_mesh( src_mesh_file,  std::ios::binary );
		std::ofstream  dst_mesh( dest_mesh_file, std::ios::binary );

		std::ifstream  src_mtl( src_mtl_file, std::ios::binary );
		std::ofstream  dst_mtl( dest_mtl_file, std::ios::binary );

		dst_mesh << src_mesh.rdbuf();
		dst_mtl  << src_mtl.rdbuf();
		
	}

}


std::string Mesh_Pool::get_Mesh_File_To_String(std::string mesh_id)
{
	////std::cout << " - Mesh_Pool::get_Mesh_File_To_String( mesh_id = " << mesh_id << " ):" << std::endl;
	
	std::string file = TEMP_DATA_MESH_FOLDER_PATH + Utilities::getFileName( mesh_id, true );
	std::ifstream t(file);
	std::string str((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
	////std::cout << "   - Mesh_Data: [ " << str << " ] " << std::endl;
	return str;

}
std::string Mesh_Pool::get_Mtl_File_To_String(std::string mesh_id)
{
	std::string file = TEMP_DATA_MESH_FOLDER_PATH + Utilities::getFileName(mesh_id, false) + std::string(".mtl");

	std::ifstream t(file);
	std::string str((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
	return str;
}
std::string Mesh_Pool::get_File_To_String(std::string file)
{


	
	std::ifstream t(file);
	
	//std::string line;
	//while (std::getline(t, line))
	{

	}
	
	std::string str((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
	//std::string str;
	return str;

}
std::vector<char> Mesh_Pool::get_File_To_Bytes(std::string filename)
{
	//std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	//std::ifstream::pos_type pos = ifs.tellg();
	//std::vector<char>  result(pos);
	//ifs.seekg(0, std::ios::beg);
	//ifs.read(&result[0], pos);
	//return result;

	std::ifstream input(filename, std::ios::binary);

	//std::vector<char> bytes;
	std::vector<char> bytes(
		(std::istreambuf_iterator<char>(input)),
		(std::istreambuf_iterator<char>()));
	input.close(); 
	
#ifdef test
	std::ifstream infile;
	infile.open(filename, std::ifstream::binary);
	int k = 0;
	while (true)
	{

		int peek = infile.peek();
		if (peek == EOF)
		{
			//std::cout << " ~ reaced EOF! Exiting!" << std::endl;
		}
		else
		{
			int c = infile.get();
			//std::cout << "\n";
			//std::cout << "  - int[ "<< k <<" ] : "      << c << std::endl;
			//std::cout << "  - char[ "<< k <<"] : "     << (char)c << std::endl;
			//std::cout << "  - uchar[ " << k << " ] : " << (unsigned char)c << std::endl;
			system("pause");
		}

		k++;
	}
	infile.close();
#endif

	return bytes;

}
std::string Mesh_Pool::readFile(std::string fileName)
{
	std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

	std::ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	std::vector<char> bytes(fileSize);
	ifs.read(bytes.data(), fileSize);

	return std::string(bytes.data(), fileSize);
}
std::vector<std::string> Mesh_Pool::get_Mesh_TexMaps( std::string mesh_id )
{
	return m_temp_texmaps_paths[mesh_id];
}
std::unordered_map<std::string, std::string> Mesh_Pool::get_Mesh_TexMaps_To_String(std::string mesh_id)
{
	std::unordered_map<std::string, std::string> texmap_strings;
	for (std::string texmap : m_temp_texmaps_paths[mesh_id])
	{
		texmap_strings[Utilities::getFileName(texmap, true)] = get_File_To_String(texmap);
	}

	return texmap_strings;
}
std::unordered_map<std::string, std::vector<char>> Mesh_Pool::get_Mesh_TexMaps_To_Bytes(std::string mesh_id)
{
	std::unordered_map<std::string, std::vector<char>> texmap_bytes;
	for (std::string texmap : m_temp_texmaps_paths[mesh_id])
	{
		texmap_bytes[Utilities::getFileName(texmap, true)] = get_File_To_Bytes(texmap);
	}

	return texmap_bytes;
}