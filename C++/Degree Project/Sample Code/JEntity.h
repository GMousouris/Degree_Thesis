#pragma once

#include <string.h>
#include <string>
#include <vector>
#include <cstdarg>

#include "optixu/optixu_math_namespace.h"
#include "optixu/optixu_matrix_namespace.h"



enum JType
{
	NULL_JTYPE,
	ENTITY,
	FIELD,
	JLIST,
	ENTITY_LIST,
	FIELD_LIST
};


enum ObjectField
{
	by_Name,
	by_ID,
	by_NameID
};


struct JEntity
{
	

private:


	std::string jlabel = "";
	std::string jdata = "";
	JType type;
	bool is_open = false;
	bool is_closed = false;
	std::vector<JEntity> sub_entities;
	std::vector<std::string> sub_data;

	class SceneExporter* exporter = 0;
	int offset_index = 0;

	int inner_index = 0;
	int has_parent = 0;

public:

	int id;

	

public:

	JEntity();
	JEntity(std::string jlabel);
	JEntity(std::string jlabel, JType type);
	JEntity(std::string jlabel, int id);

public:

	JEntity& open(class SceneExporter* exporter);
	JEntity& close();
	bool isOpen();
	bool isClosed();
	JType getType();

	std::string clause();
	std::string clause(int o);
	static std::string clause(JType type, bool is_open);

	std::string label_Raw();
	std::string label();
	static std::string quotent_label(std::string jlabel);

	std::string data();


	JEntity& asNull();

	JEntity& asEntity();
	JEntity& asEntityList();
	JEntity& asList();

	
	JEntity& asField(std::string field_data = "");
	JEntity& asField(int data);
	JEntity& asField(float data);
	JEntity& asField(optix::float2 f);
	JEntity& asField(optix::float3 f);
	JEntity& asField(optix::float4 f);
	JEntity& asField(optix::int2 f);
	JEntity& asField(optix::int3 f);
	JEntity& asField(optix::int4 f);
	JEntity& asField(optix::Matrix4x4 mat);
	
	JEntity& asFieldList(std::string list_data = "");
	JEntity& asFieldList(std::vector<std::string> data);
	JEntity& asFieldList(std::vector<int> data);
	JEntity& asFieldList(std::vector<class SceneObject*> data, ObjectField fieldtype);
	JEntity& asFieldList(std::vector<float> data);
	JEntity& asFieldList(optix::float2 f);
	JEntity& asFieldList(optix::float3 f);
	JEntity& asFieldList(optix::float4 f);
	JEntity& asFieldList(optix::int2 f);
	JEntity& asFieldList(optix::int3 f);
	JEntity& asFieldList(optix::int4 f);
	JEntity& asFieldList(optix::Matrix4x4 mat);

	JEntity& operator =(std::vector<JEntity> list);
	JEntity& include(std::vector<JEntity> list);
	JEntity& include( ... );

	std::vector<JEntity> subEntites()
	{
		return sub_entities;
	}
	JEntity& Spacing();

private:
	void Open(int depth);
	void Close(int depth);

public:
	void Export(bool open, int depth);

};

