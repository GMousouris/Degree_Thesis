#pragma once

#include "Data_Get_Interface.h"
#include "Transformations.h"
#include "Utilities.h"
#include "Mediator.h"


namespace Data_Request
{

#ifdef Object_Transformation_Get_Functions


	namespace Object
	{

		namespace Transformation
		{

			namespace Matrix
			{
				optix::Matrix4x4 Translation(SceneObject* object, CoordSystem CRT, SceneObject* relativeObject)
				{
					return optix::Matrix4x4::translate(Data_Request::Object::Transformation::Attribute::Translation(object, CRT, relativeObject));
				}
				optix::Matrix4x4 Rotation(SceneObject* object, CoordSystem CRT, SceneObject* relativeObject)
				{

					optix::Matrix4x4 coordSystem = Data_Request::Object::Transformation::General::CoordSystem_Base(CRT, TRS_ROTATION, object, relativeObject);
					return (coordSystem.inverse() * object->getRotationMatrix_chain());

				}
				optix::Matrix4x4 Scale(SceneObject* object, CoordSystem CRT, SceneObject* relativeObject)
				{
					return object->getScaleMatrix_value();
				}
				optix::Matrix4x4 Transformation(SceneObject* object, CoordSystem CRT, SceneObject* relativeObject)
				{
					optix::Matrix4x4 T, R, S;
					T = Data_Request::Object::Transformation::Matrix::Translation(object, CRT, relativeObject);
					R = Data_Request::Object::Transformation::Matrix::Rotation(object, CRT, relativeObject);
					S = Data_Request::Object::Transformation::Matrix::Scale(object, CRT, relativeObject);
					return T * R*S;
				}

			}

			namespace Attribute
			{
				optix::float3    Translation(SceneObject* object, CoordSystem CRT, SceneObject* relativeObject)
				{

					optix::Matrix4x4 coordSystem = Data_Request::Object::Transformation::General::CoordSystem_Base(CRT, TRS_TRANSLATION, object, relativeObject);
					return optix::make_float3(coordSystem.inverse() * optix::make_float4(object->getTranslationAttributes(), 1.0f));
				}
				optix::float3    Rotation(SceneObject* object, CoordSystem CRT, SceneObject* relativeObject)
				{
					return object->getRotationAttributes();
				}
				optix::float3    Scale(SceneObject* object, CoordSystem CRT, SceneObject* relativeObject)
				{
					return object->getScaleAttributes();
				}

			}

			namespace Package
			{

				Object_Transformation_Data Transformation_Pack(SceneObject* object, CoordSystem CRT, SceneObject* relativeObject)
				{


					optix::Matrix4x4 coordSystem_T = Data_Request::Object::Transformation::General::CoordSystem_Base(CRT, TRS_TRANSLATION, object, relativeObject);
					optix::Matrix4x4 coordSystem_R = Data_Request::Object::Transformation::General::CoordSystem_Base(CRT, TRS_ROTATION, object, relativeObject);
					optix::Matrix4x4 coordSystem_S = IDENTITY_4x4;

					Object_Transformation_Data data;

					// fill World-Coord data //
					data.translation_world = Data_Request::Object::Transformation::Attribute::Translation(object);
					data.rotation = Data_Request::Object::Transformation::Attribute::Rotation(object);
					data.scale = Data_Request::Object::Transformation::Attribute::Scale(object);

					data.translationMatrix_world = Data_Request::Object::Transformation::Matrix::Translation(object);
					data.rotationMatrix_world = Data_Request::Object::Transformation::Matrix::Rotation(object);
					data.scaleMatrix_world = Data_Request::Object::Transformation::Matrix::Scale(object);


					// fill relative-Coord data //
					data.translation_local = Data_Request::Object::Transformation::Attribute::Translation(object, CRT, relativeObject);
					data.transformationMatrix_local = Data_Request::Object::Transformation::Matrix::Translation(object, CRT, relativeObject);
					data.rotationMatrix_local = Data_Request::Object::Transformation::Matrix::Rotation(object, CRT, relativeObject);

					return data;
				}

			}

			namespace General
			{
				optix::Matrix4x4 CoordSystem_Base(CoordSystem CRT, TRS_REQUEST_TYPE TRS_Type, SceneObject* object, SceneObject* relativeObject)
				{

					SceneObject* rel_object = 0;
					if (CRT == LOCAL_COORDINATES)    rel_object = object->getParent();
					else if (CRT == RELATIVE_COORDINATES) rel_object = relativeObject;

					bool has_rel = rel_object != 0;

					optix::Matrix4x4 relObject_TRS = has_rel ? rel_object->getTransformationMatrix() : IDENTITY_4x4;
					optix::Matrix4x4 relObject_R = has_rel ? rel_object->getRotationMatrix_chain() : IDENTITY_4x4;
					optix::Matrix4x4 relObject_S = has_rel ? rel_object->getScaleMatrix_value() : IDENTITY_4x4;

					optix::Matrix4x4 coordSystem;
					switch (CRT)
					{

					case WORLD_COORDINATES:
					{
						switch (TRS_Type)
						{
						case TRS_TRANSLATION: coordSystem = IDENTITY_4x4; break;
						case TRS_ROTATION:    coordSystem = IDENTITY_4x4; break;
						case TRS_SCALE:       coordSystem = IDENTITY_4x4;break;
						}
					}break;
					case LOCAL_COORDINATES:
					{
						switch (TRS_Type)
						{
						case TRS_TRANSLATION: coordSystem = relObject_TRS; break;
						case TRS_ROTATION:    coordSystem = relObject_R * relObject_S; break;
						case TRS_SCALE:       coordSystem = IDENTITY_4x4; break;
						}
					}break;
					case RELATIVE_COORDINATES:
					{
						switch (TRS_Type)
						{
						case TRS_TRANSLATION: coordSystem = relObject_TRS; break;
						case TRS_ROTATION:    coordSystem = relObject_R * relObject_S; break;
						case TRS_SCALE:       coordSystem = IDENTITY_4x4; break;
						}
					}break;

					}

					return coordSystem;
				}

			}

		}

	}

	


#endif



#ifdef Object_AttachedEntities_Get_Functions

	namespace Object
	{
		namespace AttachedEntities
		{
			std::vector<SceneObject*> FaceVectors(SceneObject* object)
			{
				std::vector<SceneObject*> fvs;
				for (SceneObject* fv : object->get_Face_Vectors())
				{
					bool is_ok = fv->isActive();

					if (is_ok)
						fvs.push_back(fv);
				}

				return fvs;
			}
			std::vector<SceneObject*> Samplers(SceneObject* object, inc_SamplerType inc_type)
			{
				std::vector<SceneObject*> smplrs;
				for (SceneObject* smplr : object->get_Samplers())
				{
					bool is_ok = smplr->isActive();
					if (inc_type != inc_Samplers_ALL)
					{
						is_ok &= smplr->get_SamplerParameters().type == (Sampler_Type)inc_type;
					}

					if (is_ok)
						smplrs.push_back(smplr);
				}

				return smplrs;
			}
		}
	}

#endif



#ifdef Object_Hierarchy_Get_Functions

	namespace Object
	{
		namespace Hierarchy
		{

			SceneObject* Parent(SceneObject* object)
			{
				return object->getParent();
			}
			SceneObject* GrandParent(SceneObject* object)
			{
				return object->hasParent() ? object->getParent()->getParent() : nullptr;
			}
			std::vector<SceneObject*> Childs(SceneObject* object, inc_ObjectType inc_type)
			{
				std::vector<SceneObject*> childs;
				for (SceneObject* obj : object->getChilds())
				{
					bool is_ok = true;
					Type type = obj->getType();
					if (inc_type == 1 && type != IMPORTED_OBJECT) is_ok = false;
					else if (inc_type == 2 && type != GROUP_PARENT_DUMMY) is_ok = false;

					if (is_ok)
						childs.push_back(obj);
				}
				return childs;
			}
			std::vector<SceneObject*> Childs_OnlyObjects(SceneObject* object)
			{
				return Data_Request::Object::Hierarchy::Childs(object, inc_ObjectType::inc_Objects_Object);
			}
			std::vector<SceneObject*> Childs_OnlyGroups(SceneObject* object)
			{
				return Data_Request::Object::Hierarchy::Childs(object, inc_ObjectType::inc_Objects_Group);
			}
			std::vector<SceneObject*> GrandChilds(SceneObject* object, inc_ObjectType inc_type)
			{
				std::vector<SceneObject*> grandchilds;
				std::vector<SceneObject*> childs = Data_Request::Object::Hierarchy::Childs(object);
				for (SceneObject* child : childs)
				{
					std::vector<SceneObject*> gchilds = Data_Request::Object::Hierarchy::Childs(child, inc_type);
					for (SceneObject* gchild : gchilds) grandchilds.push_back(gchild);
				}

				return grandchilds;
			}
			std::vector<SceneObject*> GrandChilds_OnlyObjects(SceneObject* object)
			{
				std::vector<SceneObject*> grandchilds;
				std::vector<SceneObject*> childs = Data_Request::Object::Hierarchy::Childs(object);
				for (SceneObject* child : childs)
				{
					std::vector<SceneObject*> gchilds = Data_Request::Object::Hierarchy::Childs(child, inc_Objects_Object);
					for (SceneObject* gchild : gchilds) grandchilds.push_back(gchild);
				}

				return grandchilds;
			}
			std::vector<SceneObject*> GrandChilds_OnlyGroups(SceneObject* object)
			{
				std::vector<SceneObject*> grandchilds;
				std::vector<SceneObject*> childs = Data_Request::Object::Hierarchy::Childs(object);
				for (SceneObject* child : childs)
				{
					std::vector<SceneObject*> gchilds = Data_Request::Object::Hierarchy::Childs(child, inc_Objects_Group);
					for (SceneObject* gchild : gchilds) grandchilds.push_back(gchild);
				}

				return grandchilds;
			}
			std::vector<SceneObject*> Upper_FamilyTree(SceneObject* object, inc_ObjectType inc_type, unsigned int max_level, bool include_this)
			{
				return Utilities::get_UpperHierarchy(object, max_level, (int)inc_type);
			}
			std::vector<SceneObject*> Lower_FamilyTree(SceneObject* object, inc_ObjectType inc_type, unsigned int max_level, bool include_this)
			{
				return Utilities::get_LowerHierarchy(object, max_level, (int)inc_type);
			}

			//
			std::vector<int> Childs_ID(SceneObject* object, inc_ObjectType inc_type)
			{
				std::vector<int> childs;
				for (SceneObject* obj : object->getChilds())
				{
					bool is_ok = true;
					Type type = obj->getType();
					if (inc_type == 1 && type != IMPORTED_OBJECT) is_ok = false;
					else if (inc_type == 2 && type != GROUP_PARENT_DUMMY) is_ok = false;

					if (is_ok)
						childs.push_back(obj->getId());
				}
				return childs;
			}
			std::vector<int> Childs_OnlyObjects_ID(SceneObject* object)
			{
				return Data_Request::Object::Hierarchy::Childs_ID(object, inc_ObjectType::inc_Objects_Object);
			}
			std::vector<int> Childs_OnlyGroups_ID(SceneObject* object)
			{
				return Data_Request::Object::Hierarchy::Childs_ID(object, inc_ObjectType::inc_Objects_Group);
			}
			std::vector<int> GrandChilds_ID(SceneObject* object, inc_ObjectType inc_type)
			{
				std::vector<int> grandchilds;
				std::vector<SceneObject*> childs = Data_Request::Object::Hierarchy::Childs(object);
				for (SceneObject* child : childs)
				{
					std::vector<SceneObject*> gchilds = Data_Request::Object::Hierarchy::Childs(child, inc_type);
					for (SceneObject* gchild : gchilds) grandchilds.push_back(gchild->getId());
				}

				return grandchilds;
			}
			std::vector<int> GrandChilds_OnlyObjects_ID(SceneObject* object)
			{
				std::vector<int> grandchilds;
				std::vector<SceneObject*> childs = Data_Request::Object::Hierarchy::Childs(object);
				for (SceneObject* child : childs)
				{
					std::vector<SceneObject*> gchilds = Data_Request::Object::Hierarchy::Childs(child, inc_Objects_Object);
					for (SceneObject* gchild : gchilds) grandchilds.push_back(gchild->getId());
				}

				return grandchilds;
			}
			std::vector<int> GrandChilds_OnlyGroups_ID(SceneObject* object)
			{
				std::vector<int> grandchilds;
				std::vector<SceneObject*> childs = Data_Request::Object::Hierarchy::Childs(object);
				for (SceneObject* child : childs)
				{
					std::vector<SceneObject*> gchilds = Data_Request::Object::Hierarchy::Childs(child, inc_Objects_Group);
					for (SceneObject* gchild : gchilds) grandchilds.push_back(gchild->getId());
				}

				return grandchilds;
			}
			

		}
	}

#endif




#ifdef Scene_Get_Functions


	
	namespace Scene
	{

		namespace Restrictions
		{

			std::vector<Restriction*> all(inc_RestrictionType inc_type)
			{
				std::vector<Restriction*> res_list;
				for (Restriction* R : Mediator::Request_Scene_Restrictions())
				{
					bool is_ok = true;
					is_ok = R->is_Active();
					if (inc_type == 1 && R->getId() != FOCUS_R) is_ok = false;
					if (inc_type == 2 && R->getId() != CONVERSATION_R) is_ok = false;
					if (inc_type == 3 && R->getId() != DISTANCE_R) is_ok = false;

					if (is_ok)
						res_list.push_back(R);
				}
				return res_list;
			}
			std::vector<Restriction*> of(SceneObject* object, inc_RestrictionType inc_type)
			{
				std::vector<Restriction*> res_list;

				for (Restriction* R : Mediator::Request_Scene_Restrictions())
				{
					bool is_ok = true;
					is_ok = R->is_Active();
					SceneObject* ownerA = R->get_Owner(0);
					SceneObject* ownerB = R->get_Owner(1);
					if (ownerA->getId() != object->getId() && ownerB->getId() != object->getId()) is_ok = false;
					if (inc_type == 1 && R->getId() != FOCUS_R) is_ok = false;
					if (inc_type == 2 && R->getId() != CONVERSATION_R) is_ok = false;
					if (inc_type == 3 && R->getId() != DISTANCE_R) is_ok = false;

					if (is_ok)
						res_list.push_back(R);
				}

				return res_list;
			}

		}

	}


#endif



#ifdef Object_Material_Data

	namespace Object
	{
		namespace Matieral
		{
			std::vector<Material_Data> all(SceneObject* object)
			{
				std::vector<Material_Data> mat_data;
				for (int i =0; i<object->getMaterialCount(); i++)
				{
					MaterialParameters mat_params = object->getMaterialParams(i);
					Material_Data mdata;
					mdata.diffuse  = optix::make_float3(mat_params.Kd[0], mat_params.Kd[1], mat_params.Kd[2]);
					mdata.specular = optix::make_float3(mat_params.Ks[0], mat_params.Ks[1], mat_params.Ks[2]);
					mdata.reflectivity = mat_params.Kr[0];
					mdata.refraction   = mat_params.Ka[0];
					mdata.roughness    = mat_params.phong_exp;
					//mdata.id = mat_params.name;
					mdata.id = object->getGeomProperties().mat_names[i];
					mat_data.push_back(mdata);
				}

				return mat_data;
			}
			Material_Data indexed(SceneObject* object, int index)
			{


				MaterialParameters mat_params = object->getMaterialParams(index);
				Material_Data mdata;
				mdata.diffuse = optix::make_float3(mat_params.Kd[0], mat_params.Kd[1], mat_params.Kd[2]);
				mdata.specular = optix::make_float3(mat_params.Ks[0], mat_params.Ks[1], mat_params.Ks[2]);
				mdata.reflectivity = mat_params.Kr[0];
				mdata.refraction = mat_params.Ka[0];
				mdata.roughness = mat_params.phong_exp;
				mdata.id = object->getGeomProperties().mat_names[index];

				return mdata;

			}
		}
	}

#endif

}