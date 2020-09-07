#include "GeometryNode.h"
#include "GeometricMesh.h"
#include "TextureManager.h"
#include <glm/gtc/type_ptr.hpp>
#include "OptiXMesh.h"
#include <iostream>
#include <string>
#include "SceneObject.h"
#include "Defines.h"
#include "Tools.h"

int GeometryNode::OBJ_COUNT = 0;

GeometryNode::GeometryNode()
{
	
	m_vao = 0;
	m_vbo_positions = 0;
	m_vbo_normals = 0;
	m_vbo_texcoords = 0;

	m_ibo = 0;
	m_arrayTexture = 0;
	m_mat_id_buffer = 0;
	m_has_tex_id_buffer = 0;
	indirect_draw_buffer = 0;
	draw_index_buffer = 0;

	id = OBJ_COUNT++;
	
}
GeometryNode::~GeometryNode()
{
	// delete buffers
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo_positions);
	glDeleteBuffers(1, &m_vbo_normals);
	glDeleteBuffers(1, &m_vbo_texcoords);

	if (as_instance)
	{
		glDeleteBuffers(1, &indirect_draw_buffer);
		glDeleteBuffers(1, &draw_index_buffer);
		glDeleteBuffers(1, &m_mat_id_buffer);
	}

	clear();
}

void GeometryNode::Init(GeometricMesh* mesh)
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo_positions);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(glm::vec3), &mesh->vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,				// attribute index
		3,              // number of elements per vertex, here (x,y,z)
		GL_FLOAT,       // the type of each element
		GL_FALSE,       // take our values as-is
		0,		         // no extra data between each position
		0				// pointer to the C array or an offset to our buffer
	);

	glGenBuffers(1, &m_vbo_normals);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, mesh->normals.size() * sizeof(glm::vec3), &mesh->normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,				// attribute index
		3,              // number of elements per vertex, here (x,y,z)
		GL_FLOAT,		// the type of each element
		GL_FALSE,       // take our values as-is
		0,		        // no extra data between each position
		0				// pointer to the C array or an offset to our buffer
	);

	if (!mesh->textureCoord.empty())
	{
		glGenBuffers(1, &m_vbo_texcoords);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_texcoords);
		glBufferData(GL_ARRAY_BUFFER, mesh->textureCoord.size() * sizeof(glm::vec2), &mesh->textureCoord[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,				// attribute index
			2,              // number of elements per vertex, here (x,y,z)
			GL_FLOAT,		// the type of each element
			GL_FALSE,       // take our values as-is
			0,		         // no extra data between each position
			0				// pointer to the C array or an offset to our buffer
		);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// *********************************************************************

	////std::cout << "\n - mat_names : " << std::endl;
	for (int i = 0; i < mesh->objects.size(); i++)
	{
		Objects part;
		part.start_offset = mesh->objects[i].start;
		part.count        = mesh->objects[i].end - mesh->objects[i].start;
		auto material     = mesh->materials[mesh->objects[i].material_id];
		part.mat_name     = mesh->materials[mesh->objects[i].material_id].name;

		part.diffuseColor  = glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0);
		part.specularColor = glm::vec3(material.specular[0], material.specular[1], material.specular[2]);
		part.shininess     = material.shininess;
		part.textureID     = (material.texture.empty())? 0 : TextureManager::GetInstance().RequestTexture( material.texture.c_str() );
		
		parts.push_back(part);

		////std::cout << "    - mat_name[ " << i << " ] = " << part.mat_name << std::endl;
	}
}
void GeometryNode::Init_Instance(GeometricMesh* mesh)
{
	as_instance = true;

#ifdef GL_CHECKERROR_CATCH

	//std::cout << "\n - Init_Instance( ):" << std::endl;
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ Init_Instance::Error!" << std::endl;
	}

#endif

#define INIT_GPU_BUFFERS
#ifdef INIT_GPU_BUFFERS

	// m_vao
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// m_vbo
	glGenBuffers(1, &m_vbo_positions);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(glm::vec3), &mesh->vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,				// attribute index
		3,              // number of elements per vertex, here (x,y,z)
		GL_FLOAT,       // the type of each element
		GL_FALSE,       // take our values as-is
		0,		         // no extra data between each position
		0				// pointer to the C array or an offset to our buffer
	);

	// m_normals
	glGenBuffers(1, &m_vbo_normals);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, mesh->normals.size() * sizeof(glm::vec3), &mesh->normals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,				// attribute index
		3,              // number of elements per vertex, here (x,y,z)
		GL_FLOAT,		// the type of each element
		GL_FALSE,       // take our values as-is
		0,		         // no extra data between each position
		0				// pointer to the C array or an offset to our buffer
	);

	// m_texcoords
	if (!mesh->textureCoord.empty())
	{
		glGenBuffers(1, &m_vbo_texcoords);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_texcoords);
		glBufferData(GL_ARRAY_BUFFER, mesh->textureCoord.size() * sizeof(glm::vec2), &mesh->textureCoord[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,				// attribute index
			2,              // number of elements per vertex, here (x,y,z)
			GL_FLOAT,		// the type of each element
			GL_FALSE,       // take our values as-is
			0,		         // no extra data between each position
			0				// pointer to the C array or an offset to our buffer
		);
	}

	//glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#endif


	// - - - - - - - - - - - - - - - - - - - - - - - - - Multi - Draw Indirect - - - - - - - - - - - - - - - - - - - - - - - - -  //




#define INIT_ArraysIndirectCommand
#ifdef INIT_ArraysIndirectCommand

#ifdef GL_CHECKERROR_CATCH
	//std::cout << "    - Init_ArrayIndirectCommand:" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ Init_ArrayIndirectCommand::Error!" << std::endl;
	}

#endif
	
	typedef struct
	{
		GLuint vertexCount;
		GLuint instanceCount;
		GLuint firstVertex;
		GLuint baseInstance;
	} 
	DrawArraysIndirectCommand;


	// NUM_DRAWS = number of object parts
	NUM_DRAWS = mesh->objects.size();


	glGenBuffers(1, &indirect_draw_buffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, 
				 sizeof(DrawArraysIndirectCommand) * NUM_DRAWS, 
				 NULL ,
				 GL_STATIC_DRAW);

	
	DrawArraysIndirectCommand * cmd = (DrawArraysIndirectCommand *)glMapBufferRange(
		GL_DRAW_INDIRECT_BUFFER,
		0,
		NUM_DRAWS * sizeof(DrawArraysIndirectCommand),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);


	
	// For each Object Part -> Instance
	for (int i = 0; i < mesh->objects.size(); i++)
	{
		Objects part;
		part.start_offset = mesh->objects[i].start;
		part.count        = mesh->objects[i].end - mesh->objects[i].start;
		auto material     = mesh->materials[mesh->objects[i].material_id];
		part.mat_name     = mesh->materials[mesh->objects[i].material_id].name;
		part.mat_id       = mesh->objects[i].material_id;

		part.diffuseColor = glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0);
		part.specularColor = glm::vec3(material.specular[0], material.specular[1], material.specular[2]);
		part.shininess = material.shininess;
		part.textureID = (material.texture.empty()) ? 0 : TextureManager::GetInstance().RequestTexture(material.texture.c_str());
		parts.push_back(part);

		//
		cmd[i].firstVertex   = part.start_offset;
		cmd[i].vertexCount   = part.count;
		cmd[i].instanceCount = 1;
		cmd[i].baseInstance  = i;

		////std::cout << "   - part[ " << i << " ].hasTexture() : " << (part.textureID != 0) << std::endl;
	}


	for (int i = 0; i < mesh->materials.size(); i++)
		mesh_material.push_back(mesh->materials[i]);

	glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);


#ifdef GL_CHECKERROR_CATCH
	//std::cout << "    - Init_ArrayIndirectCommand_END():" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ Init_ArrayIndirectCommand_END()::Error!" << std::endl;
	}

#endif


#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


#define INIT_INSTANCE_ID_BUFFER
#ifdef INIT_INSTANCE_ID_BUFFER

#ifdef GL_CHECKERROR_CATCH
	//std::cout << "    - Init_INSTANCE_ID_BUFFER:" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ Init_INSTANCE_ID_BUFFER::Error!" << std::endl;
	}

#endif


	glGenBuffers(1, &draw_index_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, draw_index_buffer);
	glBufferData(GL_ARRAY_BUFFER,
				 NUM_DRAWS * sizeof(GLuint),
				 NULL,
				 GL_STATIC_DRAW);

	GLuint * draw_index =
		(GLuint *)glMapBufferRange(GL_ARRAY_BUFFER,
								   0,
								   NUM_DRAWS * sizeof(GLuint),
								   GL_MAP_WRITE_BIT |
								   GL_MAP_INVALIDATE_BUFFER_BIT);
	
	for (int i = 0; i < NUM_DRAWS; i++)
		draw_index[i] = i;

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glEnableVertexAttribArray(10);
	glVertexAttribIPointer(10, 1, GL_UNSIGNED_INT, 0, NULL);
	glVertexAttribDivisor(10, 1);


#ifdef GL_CHECKERROR_CATCH
	//std::cout << "    - Init_INSTANCE_ID_BUFFER_END():" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ Init_INSTANCE_ID_BUFFER_END()::Error!" << std::endl;
	}

#endif


#endif


#define INIT_MATERIAL_ARRAY_TEXTURE
#ifdef INIT_MATERIAL_ARRAY_TEXTURE


#ifdef GL_CHECKERROR_CATCH
	//std::cout << "    - INIT_MATERIAL_ARRAY_TEXTURE:" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ INIT_MATERIAL_ARRAY_TEXTURE::Error!" << std::endl;
	}

#endif



	NUM_MATERIALS = mesh->materials.size();

	// material id buffer //
	glGenBuffers(1, &m_mat_id_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_mat_id_buffer);
	glBufferData(GL_ARRAY_BUFFER,
				 NUM_DRAWS * sizeof(int),
				 NULL,
				 GL_STATIC_DRAW);

	GLuint * material_index_buffer =
		(GLuint *)glMapBufferRange(GL_ARRAY_BUFFER,
								   0,
								   NUM_DRAWS * sizeof(int),
								   GL_MAP_WRITE_BIT |
								   GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < NUM_DRAWS; i++)
		material_index_buffer[i] = mesh->objects[i].material_id;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, 0, NULL);
	glVertexAttribDivisor(4, 1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//

	// has tex id buffer //
	/*
	glGenBuffers(1, &m_has_tex_id_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_has_tex_id_buffer);
	glBufferData(GL_ARRAY_BUFFER,
				 NUM_DRAWS * sizeof(int),
				 NULL,
				 GL_STATIC_DRAW);
	GLuint * has_texture_index_buffer =
		(GLuint*)glMapBufferRange(GL_ARRAY_BUFFER,
								  0,
								  NUM_DRAWS * sizeof(int),
								  GL_MAP_WRITE_BIT |
								  GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < NUM_DRAWS; i++)
	{
		auto material = mesh->materials[mesh->objects[i].material_id];
		has_texture_index_buffer[i] = material.texture.empty() ? 0 : 1;
	}glUnmapBuffer(GL_ARRAY_BUFFER);

	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, 0, NULL);
	glVertexAttribDivisor(5, 1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	*/
	//


#ifdef GL_CHECKERROR_CATCH
	//std::cout << "    - INIT_MATERIAL_ARRAY_TEXTURE_END():" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ INIT_MATERIAL_ARRAY_TEXTURE_END()::Error!" << std::endl;
	}

#endif

#endif



#define INIT_TEXTURE_ARRAY
#ifdef INIT_TEXTURE_ARRAY


#endif
	



#ifdef GL_CHECKERROR_CATCH

	//std::cout << " - Init_Instance_END( ):" << std::endl;
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ Init_Instance_END()::Error!" << std::endl;
	}

#endif
	
}


// create GL_Texture_Array_2D with material parameters of the given object //
void GeometryNode::create_Material( SceneObject* obj )
{
	////std::cout << " - creating Material for Object : " << obj->getId() << std::endl;
#ifdef GL_CHECKERROR_CATCH
	//std::cout << "\n - create_Material( obj : "<< obj->getId()<<" ):" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ create_Material::Error!" << std::endl;
	}

#endif
	
	//
	GLuint material;
	glGenTextures(1, &material);
	glBindTexture(GL_TEXTURE_2D_ARRAY, material);

	GLsizei width  = 1;
	GLsizei height = 1;
	GLsizei layer_count = NUM_MATERIALS;

	
	GLubyte * texels = new GLubyte[ NUM_MATERIALS * 4 ];
	int k = 0;
	int j = 0;
	for (int i = 0; i < NUM_MATERIALS; i += 1)
	{
		
		std::string mat_name = mesh_material[i].name;
		bool is_valid = obj->is_Material_Valid(mat_name);

		float * diffuse;
		if (!is_valid)
		{
			diffuse = mesh_material[i].diffuse;
		}
		else
		{
			MaterialParameters mat_params = obj->getMaterialParams( mat_name );
			diffuse = mat_params.Kd;
		}

		
		texels[k    ] = diffuse[0] * 255.0f;
		texels[k + 1] = diffuse[1] * 255.0f;
		texels[k + 2] = diffuse[2] * 255.0f;
		texels[k + 3] = 255.0f;

		k += 4;
		j++;
	}

	glTexStorage3D(GL_TEXTURE_2D_ARRAY,
				   1,
				   GL_RGBA8,
				   width,
				   height,
				   layer_count);

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, layer_count, GL_RGBA, GL_UNSIGNED_BYTE, texels);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	delete texels;

	// store Material
	m_materials[obj->getId()] = material;

#ifdef GL_CHECKERROR_CATCH
	//std::cout << " - create_Material_END( obj : " << obj->getId() << " ):" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ create_Material_END( obj : "<< obj->getId()<<" )::Error!" << std::endl;
	}

#endif

	//
	create_TextureArray();
}
// Create Texture-Atlas or texture Array only with the number of unique used textures used per Mesh and not with the number of DRAWS ( could be huge )
void GeometryNode::create_TextureArray()
{
	return;

	std::vector<GLuint> textures;
	int max_w = 0;
	int max_h = 0;
	int count = 0;
	for (int i = 0; i < parts.size(); i++)
	{
		int w = 0;
		int h = 0;
		int miplevel = 0;

		bool has_tex = parts[i].textureID > 0 ? true : false;
		if (has_tex)
		{
			glBindTexture(GL_TEXTURE_2D, parts[i].textureID);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
			count++;
		}

		if (w > max_w)
			max_w = w;
		if (h > max_h)
			max_h = h;

		textures.push_back(parts[i].textureID);
	}


}

void GeometryNode::Update_Material( SceneObject* obj )
{
#ifdef GL_CHECKERROR_CATCH
	//std::cout << "\n - update_Material( obj : " << obj->getId() << " ):" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ update_Material::Error!" << std::endl;
	}

#endif

	//
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_materials[obj->getId()]);

	GLsizei width = 1;
	GLsizei height = 1;
	GLsizei layer_count = NUM_MATERIALS;


	GLubyte * texels = new GLubyte[NUM_MATERIALS * 4];
	int k = 0;
	int j = 0;
	for (int i = 0; i < NUM_MATERIALS; i += 1)
	{

		std::string mat_name = mesh_material[i].name;
		bool is_valid = obj->is_Material_Valid(mat_name);

		float * diffuse;
		if (!is_valid)
		{
			diffuse = mesh_material[i].diffuse;
		}
		else
		{
			MaterialParameters mat_params = obj->getMaterialParams(mat_name);
			diffuse = mat_params.Kd;
		}


		texels[k] = diffuse[0] * 255.0f;
		texels[k + 1] = diffuse[1] * 255.0f;
		texels[k + 2] = diffuse[2] * 255.0f;
		texels[k + 3] = 255.0f;

		k += 4;
		j++;
	}

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, layer_count, GL_RGBA, GL_UNSIGNED_BYTE, texels);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


#ifdef GL_CHECKERROR_CATCH
	//std::cout << " - update_Material_END( obj : " << obj->getId() << " ):" << std::endl;

	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		//std::cout << "    ~ update_Material_END( obj : "<< obj->getId()<<" )::Error!" << std::endl;
	}

#endif

}
// remove stored data only when object gets permanently removed from app //
void GeometryNode::remove_Material(int id)
{
	
	for (auto& it : m_materials)
	{
		if (it.first == id)
		{
			glDeleteTextures(1, &m_materials[id]);
			m_materials.erase(id);
		}
	}
	
}




void GeometryNode::bind_Vao()
{
	glBindVertexArray(m_vao);
}
void GeometryNode::bind_MDIBuffer()
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
	bind_indirect = true;
}
void GeometryNode::unbind_Vao()
{
	glBindVertexArray(0);
}
void GeometryNode::unbind_MDIBuffer()
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	bind_indirect = false;
}

void GeometryNode::bind_Material(int obj_id)
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_materials[obj_id]);
}
void GeometryNode::unbind_Material()
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
void GeometryNode::Draw()
{
	if (bind_indirect)
		glMultiDrawArraysIndirect(GL_TRIANGLES, NULL, NUM_DRAWS, 0);
	else
	{
		for (int j = 0; j < parts.size(); j++)
		{
			glDrawArrays
			(
				GL_TRIANGLES,
				parts[j].start_offset,
				parts[j].count
			);
		}
	}
}
void GeometryNode::clear()
{
	// delete all data
	for (auto& it : m_materials)
	{
		glDeleteTextures(1, &it.second);
	}

	m_materials.clear();
}