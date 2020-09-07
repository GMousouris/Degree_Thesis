#pragma once
#include <unordered_map>
#include <string.h>
#include "GeometryNode.h"
#include "OptiXMesh.h"

struct Mesh_Data
{
	//float * vertices;
	//float * normals;
	//float * 
};

enum Mesh_Type
{
	OpenGL_Mesh,
	OptiX_Mesh
};

class Mesh_Pool
{

public:

	Mesh_Pool();
	~Mesh_Pool();

	static Mesh_Pool& GetInstance()
	{
		static Mesh_Pool mesh_pool;
		return mesh_pool;
	}

	//void Init();


public:

	void          Load_Mesh(std::string filename , bool as_instances = true, bool is_loading_scene = false );
	OptiXMesh *   request_Mesh_for_Optix(std::string key);
	OptiXMesh     request_Mesh_Instace_Optix(std::string key , class SceneObject * = 0 );

	GeometryNode* request_Mesh_for_OpenGL(std::string key);
	GeometryNode* request_Mesh_for_OpenGL(class SceneObject* source);
	struct GeometryObject_OpenGL* request_Mesh_for_OpenGL2(std::string key);
	struct GeometryObject_OpenGL* request_Mesh_for_OpenGL2(class SceneObject* source);

	void  delete_Mesh(std::string key);
	bool  is_Mesh_Available(std::string key);

	void  create_GeometryNode_Material(class SceneObject* obj);
	void  Update_GeometryNode_Material(class SceneObject* obj);
	void  remove_GeometryNode_Material(class SceneObject* obj);
	
	void clear_temp_files();
	void clear(std::string mesh_id);
	void clear();
	
	void Export_Mesh_Files(const char * path);

	std::string get_Mesh_File_To_String(std::string mesh_id);
	std::string get_Mtl_File_To_String(std::string mesh_id);
	std::string get_File_To_String(std::string texmap);
	std::vector<char> get_File_To_Bytes(std::string filename);

	std::vector<std::string> get_Mesh_TexMaps( std::string mesh_id );
	std::unordered_map<std::string, std::string> get_Mesh_TexMaps_To_String(std::string mesh_id);
	std::unordered_map<std::string, std::vector<char>> get_Mesh_TexMaps_To_Bytes(std::string mesh_id);

	std::string readFile(std::string filename);

private:

	std::unordered_map<std::string, GeometryNode*> m_OpenGL_meshes;
	std::unordered_map<std::string, struct GeometryObject_OpenGL*> m_OpenGL_meshes2;
	std::unordered_map<std::string, OptiXMesh>     m_Optix_meshes;
	std::unordered_map<std::string, std::string>   m_temp_mesh_paths;
	std::unordered_map<std::string, std::string>   m_temp_mtl_paths;
	std::unordered_map<std::string, std::vector<std::string>> m_temp_texmaps_paths;

};