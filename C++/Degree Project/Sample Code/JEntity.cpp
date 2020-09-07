#include "JEntity.h"
#include "SceneExporter.h"
#include "SceneObject.h"

JEntity::JEntity() :jlabel(""), type(NULL_JTYPE) {}
JEntity::JEntity(std::string jlabel) : jlabel(jlabel), type(NULL_JTYPE) {}
JEntity::JEntity(std::string jlabel, JType type) : jlabel(jlabel), type(type) {}
JEntity::JEntity(std::string jlabel, int id) : jlabel(jlabel), id(id), type(NULL_JTYPE)
{ 
	if (jlabel[0] == '%' && jlabel[jlabel.size()-1] == '%') 
	{
		jlabel = "";
	}
}





JEntity& JEntity::open(SceneExporter* exp)
{

	exporter = exp;
	bool r_ = !is_open;
	is_open = true;


	JEntity& obj = *this;
	return obj;
	//return this;
}
JEntity& JEntity::close()
{
	bool r_   = is_open;
	is_closed = true;
	is_open   = false;

	//
	//std::cout << "\n - " << label_Raw() << "::close():" << std::endl;
	//std::cout << "   - type : " << type << std::endl;
	
	switch (type)
	{
	case ENTITY:      { exporter->passActivityToParent(); break;}
	case ENTITY_LIST: { exporter->passActivityToParent(); break;}
	case JLIST:       { exporter->passActivityToParent(); break;}
	}
	
	//

	JEntity& obj = *this;
	return obj;
}
bool JEntity::isOpen() { return is_open; }
bool JEntity::isClosed() { return is_closed; }
JType JEntity::getType() { return type; }

std::string JEntity::clause()
{
	switch (type)
	{
	case NULL_JTYPE: return "";
	case ENTITY:      if (isOpen()) return "{"; else return "}";
	case FIELD:       if (isOpen()) return ""; else return "";
	case JLIST:       if (isOpen()) return "["; else return "]";
	case ENTITY_LIST: if (isOpen()) return "["; else return "]";
	case FIELD_LIST:  if (isOpen()) return "["; else return "]";
	}

}
std::string JEntity::clause(int o)
{

	switch (type)
	{
	case NULL_JTYPE: return "";
	case ENTITY:      if (o) return "{"; else return "}";
	case FIELD:       if (o) return ""; else return "";
	case JLIST:       if (o) return "["; else return "]";
	case ENTITY_LIST: if (o) return "["; else return "]";
	case FIELD_LIST:  if (o) return "["; else return "]";
	}

}
std::string JEntity::clause(JType type, bool is_open)
{
	switch (type)
	{
	case NULL_JTYPE: return "";
	case ENTITY:      if (is_open) return "{"; else return "}";
	case FIELD:       if (is_open) return "{"; else return "}";
	case JLIST:       if (is_open) return "["; else return "]";
	case ENTITY_LIST: if (is_open) return "["; else return "]";
	case FIELD_LIST:  if (is_open) return "["; else return "]";
	}

}

std::string JEntity::label_Raw()
{
	return jlabel;
}
std::string JEntity::label()
{
	if (jlabel == "")
		return label_Raw();
	else if (jlabel[0] == '%' && jlabel[jlabel.size() - 1] == '%')
		return "";

	return quotent_label(jlabel) + ": ";
}
std::string JEntity::quotent_label(std::string jlabel)
{
	return _quote_ + jlabel + _quote_;
}

std::string JEntity::data()
{
	return jdata;
}

JEntity& JEntity::asNull()
{
	type = NULL_JTYPE;
	return *this;
}
JEntity& JEntity::asEntity()
{
	type = ENTITY;
	exporter->setActiveNode(id);
	return *this;
}
JEntity& JEntity::asEntityList()
{
	type = ENTITY_LIST;
	exporter->setActiveNode(id);
	return *this;
}
JEntity& JEntity::asList()
{
	type = JLIST;
	exporter->setActiveNode(id);
	return *this;
}

JEntity& JEntity::asField(std::string field_data)
{
	type = FIELD;
	jdata = _quote_+field_data+_quote_;

	return *this;
}
JEntity& JEntity::asField(int data)
{
	type = FIELD;
	jdata = std::to_string(data);

	return *this;
}
JEntity& JEntity::asField(float data)
{
	type = FIELD;
	std::to_string(data);

	return *this;
}
JEntity& JEntity::asField(optix::float2 f)
{
	std::vector<float> v;
	v.push_back(f.x);
	v.push_back(f.y);
	return asFieldList(v);
}
JEntity& JEntity::asField(optix::float3 f)
{
	std::vector<float> v;
	v.push_back(f.x);
	v.push_back(f.y);
	v.push_back(f.z);
	return asFieldList(v);
}
JEntity& JEntity::asField(optix::float4 f) {
	std::vector<float> v;
	v.push_back(f.x);
	v.push_back(f.y);
	v.push_back(f.z);
	v.push_back(f.w);
	return asFieldList(v);
}
JEntity& JEntity::asField(optix::int2 f) {
	std::vector<int> v;
	v.push_back(f.x);
	v.push_back(f.y);
	return asFieldList(v);
}
JEntity& JEntity::asField(optix::int3 f) {
	std::vector<int> v;
	v.push_back(f.x);
	v.push_back(f.y);
	v.push_back(f.z);
	return asFieldList(v);
}
JEntity& JEntity::asField(optix::int4 f) {
	std::vector<int> v;
	v.push_back(f.x);
	v.push_back(f.y);
	v.push_back(f.z);
	v.push_back(f.w);
	return asFieldList(v);
}
JEntity& JEntity::asField(optix::Matrix4x4 mat)
{
	std::vector<float>v;
	float*data = mat.getData();
	for (int i = 0; i < 16; i++)
	{
		v.push_back(data[i]);
	}
	return asFieldList(v);
}

JEntity& JEntity::asFieldList(std::string list_data)
{
	type = FIELD_LIST;
	jdata = list_data;


	return *this;
}
JEntity& JEntity::asFieldList(std::vector<std::string> data)
{
	type = FIELD_LIST;

	for (int i = 0; i < data.size(); i++)
	{
		jdata += _quote_+data[i]+_quote_;
		if (i < data.size() - 1)
		{
			jdata += ",";
		}
	}

	return *this;
}
JEntity& JEntity::asFieldList(std::vector<int> data)
{
	type = FIELD_LIST;

	for (int i = 0; i < data.size(); i++)
	{
		jdata += std::to_string(data[i]);
		if (i < data.size() - 1)
		{
			jdata += ",";
		}
	}

	return *this;
}
JEntity& JEntity::asFieldList(std::vector<float> data)
{
	type = FIELD_LIST;

	for (int i = 0; i < data.size(); i++)
	{
		jdata += std::to_string(data[i]);
		if (i < data.size() - 1)
		{
			jdata += ",";
		}
	}

	return *this;
}
JEntity& JEntity::asFieldList(std::vector<SceneObject*> data, ObjectField fieldtype)
{
	type = FIELD_LIST;

	for (int i = 0; i < data.size(); i++)
	{
		switch (fieldtype)
		{
		case by_Name:   jdata += (data[i]->getName()); break;
		case by_ID:     jdata += std::to_string(data[i]->getId()); break;
		case by_NameID: jdata += data[i]->getNameWithID(); break;
		}
		if (i < data.size() - 1)
		{
			jdata += ",";
		}
	}

}
JEntity& JEntity::asFieldList(optix::float2 f)
{
	return asField(f);
}
JEntity& JEntity::asFieldList(optix::float3 f)
{
	return asField(f);
}
JEntity& JEntity::asFieldList(optix::float4 f)
{
	return asField(f);
}
JEntity& JEntity::asFieldList(optix::int2 f)
{
	return asField(f);
}
JEntity& JEntity::asFieldList(optix::int3 f)
{
	return asField(f);
}
JEntity& JEntity::asFieldList(optix::int4 f)
{
	return asField(f);
}
JEntity& JEntity::asFieldList(optix::Matrix4x4 mat)
{
	std::vector<float>v;
	float*data = mat.getData();
	for (int i = 0; i < 16; i++)
	{
		v.push_back(data[i]);
	}
	return asFieldList(v);
}

JEntity& JEntity::operator =(std::vector<JEntity> list)
{

	int index = 0;
	for (int i = 0; i < list.size(); i++)
	{
		JEntity& je = exporter->request(list[i].id);
		i == list.size() - 1 ? je.inner_index = 0 : ++index;
		i == list.size() - 1 ? je.has_parent = 1 : je.has_parent = 1;
	}

	close();
	return *this;
}
JEntity& JEntity::include(std::vector<JEntity> list)
{
	int index = 0;
	for (int i = 0; i<list.size(); i++)
	{
		JEntity& je = exporter->request(list[i].id);
		if (i < list.size() - 1)
		{
			je.inner_index = ++index;
		}
		else
		{
			je.inner_index = 0;
		}
		
		je.has_parent = 1;

	}

	close();
	return *this;
}
JEntity& JEntity::include(...)
{
	close();
	return *this;
}

void JEntity::Open(int depth)
{
	switch (type)
	{
	case ENTITY:      {  exporter->outln(label() + clause(1), depth); } break;
	case ENTITY_LIST: {  exporter->outln(label() + clause(1), depth); } break;
	case JLIST:       {  exporter->outln(label() + clause(1), depth); } break;

	case FIELD:       { exporter->outln(label() + clause(1) + " " + data() + " " + clause(0) + (inner_index ? "," : ""), depth); } break;
	case FIELD_LIST:  { exporter->outln(label() + clause(1) + " " + data() + " " + clause(0) + (inner_index ? "," : ""), depth); } break;

	case NULL_JTYPE:  { exporter->outln(); } break;
	}

}
void JEntity::Close(int depth)
{
	switch (type)
	{
	case ENTITY:      {  exporter->outln(clause(0) + (inner_index ? "," : ""), depth); } break;
	case ENTITY_LIST: {  exporter->outln(clause(0) + (inner_index ? "," : ""), depth); } break;
	case JLIST:       {  exporter->outln(clause(0) + (inner_index ? "," : ""), depth); } break;

	case FIELD:      {  } break;
	case FIELD_LIST: {  } break;
	//case NULL_JTYPE: { exporter->outln(); } break;
	}

}
void JEntity::Export(bool o, int ddepth)
{
	if (!o)
		Open(ddepth);
	else
		Close(ddepth);
}
JEntity& JEntity::Spacing()
{
	return exporter->Spacing();
}