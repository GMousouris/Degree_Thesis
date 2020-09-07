#include "HierarchyTree.h"
#include "Mediator.h"
#include "Utilities.h"



void TreeNode::remove(SceneObject* child_obj, bool del )
{
#ifdef CANCER_DEBUG

	if (this == nullptr)
	{
		//std::cout << "\n TreeNode::remove( SceneObject : " << child_obj->getName() << " )" << std::endl;
		//std::cout << "     - this == nullptr? : " << (this == nullptr) << std::endl;
		//std::cout << "     - childs.size() = " << childs.size() << std::endl;
	}

#endif

	int index = -1;
	for (int i = 0; i < childs.size(); i++)
	{
		if (childs[i]->object->getId() == child_obj->getId())
		{
			index = i;
			break;
		}
	}

	//if(del)
	//	delete childs[index];
	if (index == -1)
	{

		//std::cout << " treeNode[" << object->getId() << "]::remove( TreeNode[" << child_obj->getId() << "] )" << std::endl;
		//std::cout << "     - requested Child index is invalid!" << std::endl;
		system("pause");
		return;
	}

	childs[index]->parent = 0;
	childs[index] = 0;
	childs.erase(childs.begin() + index);

	Mediator::request_HierarchyTreeHandle()->markDirty();
}
void TreeNode::remove(TreeNode* child_node, bool del )
{

#ifdef CANCER_DEBUG

	if (this == nullptr)
	{
		//std::cout << "\n TreeNode::remove( TreeNode : " << child_node->object->getId() << ")" << std::endl;
		//std::cout << "     - this == nullptr? : " << (this == nullptr) << std::endl;
		//std::cout << "     - childs.size() = " << childs.size() << std::endl;
	}

#endif
	int index = -1;
	for (int i = 0; i < childs.size(); i++)
	{
		if (childs[i]->object->getId() == child_node->object->getId())
		{
			index = i;
			break;
		}
	}

	if (index == -1)
	{
		if (object != nullptr)
		{
		//	std::cout << " treeNode[" << object->getId() << "]::remove( TreeNode[" << child_node->object->getId() << "] )" << std::endl;
		}
		//else
		//	std::cout << " treeNode[]::remove( TreeNode[" << child_node->object->getId() << "] ) " << std::endl;

		//std::cout << "     - requested Child index is invalid!" << std::endl;
		system("pause");
		return;
	}

	//if (del)
	//	delete childs[index];
	childs[index]->parent = 0;
	childs[index] = 0;
	childs.erase(childs.begin() + index);
	Mediator::request_HierarchyTreeHandle()->markDirty();
}
void TreeNode::insertChild(TreeNode* child)
{
	child->parent = this;
	childs.push_back(child);
	Mediator::request_HierarchyTreeHandle()->markDirty();
}
void TreeNode::insertChild(SceneObject* child_obj)
{
	TreeNode* child_node = new TreeNode();
	child_node->parent = this;
	child_node->object = child_obj;
	childs.push_back(child_node);
	Mediator::request_HierarchyTreeHandle()->markDirty();
}

void Hierarchy_Tree::order_Node_Childs_by_depth_Order(int inc_fv, int inc_smplr)
{

	std::stack<TreeNode*> stack;
	stack.push(rootNode);
	while (stack.size() > 0)
	{
		TreeNode* current = stack.top();
		stack.pop();

		//
		SceneObject* obj = current->object;

		bool is_root = false;
		bool is_valid = false;
		bool is_active = false;
		bool is_fv = false;
		bool is_smplr = false;

		is_root = current->is_root;

		if (!is_root)
		{
			is_active = obj->isActive();
			is_smplr  = obj->getType() == SAMPLER_OBJECT;
			is_fv     = obj->getType() == FACE_VECTOR;
		}

		
		if( (!is_fv && !is_smplr) )
		{
			current->childs_by_depth_order(0, 0);
		}


		for (TreeNode* child : current->childs)
			stack.push(child);

	}
}
void Hierarchy_Tree::insert_Utility_Childs(int fv, int smplr, int lights)
{
	////std::cout << "\n - insert_Utility_Childs():" << std::endl;
	std::stack<TreeNode*> stack;
	for (TreeNode* child : rootNode->childs)
		stack.push(child);

	while (stack.size() > 0)
	{
		TreeNode* current = stack.top();
		stack.pop();

		//
		SceneObject* obj = current->object;
		if (obj->isActive())
		{
			if (fv)
			{
				for (SceneObject* fv : obj->get_Face_Vectors())
				{
					current->insertChild(fv);
					temp_added_ut_childs.push_back(fv);
				}
			}
		
			if (smplr)
			{
				for (SceneObject* smplr : obj->get_Samplers())
				{
					current->insertChild(smplr);
					temp_added_ut_childs.push_back(smplr);
				}
			}

		}
		//

		for (TreeNode* child : current->childs)
			stack.push(child);


	}
}

void Hierarchy_Tree::clear_Utility_Childs()
{
	for (SceneObject* obj : temp_added_ut_childs)
		remove(obj);

	temp_added_ut_childs.clear();
}

bool Hierarchy_Tree::checkIntegrity()
{
	return true;

	//return;
	bool is_ok = true;
	bool tree_scene_ok = true;
	std::vector<SceneObject*> scene_objects = Mediator::RequestSceneObjects();
	int scene_objects_count = 0;
	for (SceneObject* obj : scene_objects)
	{
		if (
			   obj->getType() == APP_OBJECT 
			|| obj->getType() == FACE_VECTOR 
			|| obj->getType() == SAMPLER_OBJECT)
			continue;

		TreeNode* obj_node = find(obj);
		if (obj_node == nullptr || obj_node == 0)
		{
			
			//std::cout << " - HierarchyTree()::checkIntegrity():" << std::endl;
			//std::cout << "       - tree_scene_ok : " << tree_scene_ok << std::endl;
			//std::cout << "       - find(" << obj->getId() << ") returned : nullptr!" << std::endl;
			system("pause");
			
			is_ok = false;
			tree_scene_ok = false;
			break;
		}

		scene_objects_count++;
	}
	

	std::stack<TreeNode*> stack;
	for (TreeNode* child : rootNode->childs)
		stack.push(child);

	int treeNode_count = 0;
	while (stack.size() > 0)
	{
		TreeNode* current = stack.top();
		stack.pop();
		treeNode_count++;

		bool treeNode_ok = !(current == nullptr || current == 0);
		bool treeNode_object_ok = !(current->object == nullptr || current->object == 0);
		bool treeNode_object_id_ok = (current->object->getId() >= 0 && Mediator::requestObjectsArrayIndex(current->object->getId()) != -1);
		bool treeNode_context_object_parent_ok = true;
		
		
		if (
			  current->parent == nullptr && current->object->getParent() != nullptr
			)
		{
			if (current->object->getParent() != nullptr)
			{
				if(!current->object->getParent()->isTemporary())
					treeNode_context_object_parent_ok = false;
			}
			else
				treeNode_context_object_parent_ok = false;
		}
		


		bool check_childs = true;
		if (Mediator::is_Multiple_Selection_Active())
			check_childs = false;


		bool treeNode_context_object_childs_ok = true;
		if (check_childs)
		{
			for (TreeNode* child : current->childs)
			{
				if (!Utilities::is_object_in_List(child->object, current->object->getChilds()))
				{
					treeNode_context_object_childs_ok = false;
					break;
				}
			}
		}

		for (TreeNode* child : current->childs)
			stack.push(child);

		if (
			   !treeNode_ok
			|| !treeNode_object_ok
			|| !treeNode_object_id_ok
			|| !treeNode_context_object_parent_ok
			|| !treeNode_context_object_childs_ok
			)
		{
			
			is_ok = false;
			//std::cout << " \n\n";
			//std::cout << " - HierarchyTree()::checkIntegrity():" << std::endl;
			//std::cout << "       - current                                : " << current->object->getId() << std::endl;
			

			
			//std::cout << "       - current_Object_Parent   ( != nullptr ) : " << !(current->object->getParent() == nullptr || current->object->getParent() == 0) << std::endl;
			//std::cout << "       - current_TreeNode_Parent ( != nullptr ) : " << !(current->parent->object == nullptr || current->parent->object == 0) << std::endl;
			if (current->object->getParent() != nullptr && current->object->getParent() != 0)
				//std::cout << "       - current_Object_Parent_ID : " << current->object->getParent()->getId() << std::endl;
			
			if (current->parent != nullptr && current->parent != 0)
				if( current->parent->object != nullptr && current->parent->object != 0)
					//std::cout << "       - current_TreeNode_Parent_ID         : " << current->parent->object->getId() << std::endl;

			//std::cout << "       - current_Object_Childs():" << std::endl;
			for (SceneObject* obj : current->object->getChilds())
				//std::cout << "              - current[" << current->object->getId() << "]_child : " << obj->getId() << std::endl;
			
			//std::cout << "       - current_TreeNode_Childs():" << std::endl;
			for (TreeNode* obj : current->childs)
				//std::cout << "              - current[" << current->object->getId() << "]_treeNode_child : " << obj->object->getId() << std::endl;

			//std::cout << "       - treeNode_ok                           : " << treeNode_ok << std::endl;
			//std::cout << "       - treeNode_object_ok                    : " << treeNode_object_ok << std::endl;
			//std::cout << "       - treeNode_object_id_ok                 : " << treeNode_object_id_ok << std::endl;
			//std::cout << "       - treeNode_context_object_parent_ok     : " << treeNode_context_object_parent_ok << std::endl;
			//std::cout << "       - treeNode_context_object_childs_ok     : " << treeNode_context_object_childs_ok << std::endl;
			system("pause");
		}
	}

	bool objects_count_ok = ( treeNode_count == scene_objects_count );
	if (!objects_count_ok)
	{
		is_ok = false;
		//std::cout << " - HierarchyTree()::checkIntegrity():" << std::endl;
		//std::cout << "       - objects_count_ok : " << objects_count_ok << std::endl;
		system("pause");
	}

	return is_ok;
}

void Hierarchy_Tree::markDirty()
{
	is_dirty = true;
}

bool Hierarchy_Tree::isDirty()
{
	bool dirty = is_dirty;
	is_dirty = false;
	return dirty;
}

