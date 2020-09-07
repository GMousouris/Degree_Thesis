#pragma once

#include "SceneObject.h"
//#include "Utilities.h"
//#include "Mediator.h"
#include <stack>
#include <vector>
#include "Defines.h"
#include <unordered_map>
#include <algorithm>


struct ObjectStruct
{
	SceneObject* object;
	SceneObject* parent;
	SceneObject* group;
	std::vector<SceneObject*> childs;
	std::vector<SceneObject*> facevectors;
	std::vector<SceneObject*> samplers;
	

	ObjectStruct()
		: object(0), parent(0), group(0)
	{}

	ObjectStruct(SceneObject* obj, std::vector<SceneObject*> chs)
	{
		object = obj;
		parent = 0;
		group  = 0;
		for (SceneObject* objj : chs)
			childs.push_back(objj);
	}
	ObjectStruct(SceneObject* obj, SceneObject* prnt, std::vector<SceneObject*> chs)
	{
		object = obj;
		parent = prnt;
		group  = 0;
		for (SceneObject* objj : chs)
			childs.push_back(objj);
	}
	ObjectStruct(SceneObject* obj, SceneObject* prnt,SceneObject* grp, std::vector<SceneObject*> chs)
	{
		object = obj;
		parent = prnt;
		group  = grp;
		for (SceneObject* objj : chs)
			childs.push_back(objj);
	}
	ObjectStruct(SceneObject* obj, SceneObject* prnt, SceneObject* grp, std::vector<SceneObject*> chs, std::vector<SceneObject*> fvs, std::vector<SceneObject*> smplrs)
	{
		object = obj;
		parent = prnt;
		group = grp;
		for (SceneObject* objj : chs) childs.push_back(objj);
		for (SceneObject* fv : fvs) facevectors.push_back(fv);
		for (SceneObject* smplr : smplrs) samplers.push_back(smplr);
	}
};


// Hierarchy Tree structure //
struct TreeNode
{
	bool is_root;
	SceneObject* object;
	TreeNode* parent;
	TreeNode* group = 0;
	std::vector<TreeNode*> childs;
	std::vector<TreeNode*> childs_depth_ordered;

	TreeNode* operator[](int index)
	{
		return childs[index];
	}
	TreeNode* getChild(int index)
	{
		return childs[index];
	}
	std::vector<SceneObject*> Childs_asObjects()
	{
		std::vector<SceneObject*> childs_asObjects;
		for (TreeNode* child : childs) childs_asObjects.push_back(child->object);
		return childs_asObjects;
	}
	int       getChilds_size()
	{
		return childs.size();
	}
	int getActiveChilds_size(bool inc_objs = true, bool inc_grps = true, bool inc_lights = true)
	{
		int num = 0;
		for (TreeNode * node : childs)
		{
			SceneObject* obj = node->object;
			if (obj->isActive())
			{
				Type obj_type = obj->getType();
				if (   (obj_type == LIGHT_OBJECT       && !inc_lights)
					|| (obj_type == IMPORTED_OBJECT    && !inc_objs)
					|| (obj_type == GROUP_PARENT_DUMMY && !inc_grps)
					)
					continue;

				num++;
			}	
		}

		return num;
	}
	

	std::vector<TreeNode*> childs_by_depth_order()
	{
		std::unordered_map<int, int> childs_depth;
		for (TreeNode* child : childs)
		{
			int depth = 0;

			std::stack<TreeNode*> stack;
			for (TreeNode* node : child->childs) stack.push(node);
			while (stack.size() > 0)
			{
				TreeNode* current = stack.top();
				stack.pop();

				for (TreeNode* child : current->childs)
					stack.push(child);

				depth++;
			}

			childs_depth[child->object->getId()] = depth;
		}

		std::vector<TreeNode*> childs_sorted; for (TreeNode* child : childs) childs_sorted.push_back(child);
		
		// sorting //
		for (int i = 0; i < childs_sorted.size(); i++)
		{
			
			for (int j = i; j > 0; j--)
			{
				
				TreeNode* child_j = childs_sorted[j - 1];
				int id_j          = child_j->object->getId();
				int depth_j       = childs_depth[id_j];

				TreeNode* child_i = childs_sorted[j];
				int id_i          = child_i->object->getId();
				int depth_i       = childs_depth[id_i];

				if ( depth_j > depth_i )
				{
					TreeNode* temp     = child_j;
					childs_sorted[j-1] = child_i;
					childs_sorted[ j ] = temp;
				}

				if (depth_j == depth_i)
				{
					int id_j = child_j->object->getId();
					int id_i = child_i->object->getId();

					if (id_j > id_i)
					{
						TreeNode* temp = child_j;
						childs_sorted[j - 1] = child_i;
						childs_sorted[j] = temp;
					}
				}
			}
		}
		
		return childs_sorted;
	}
	std::vector<TreeNode*> childs_by_depth_order(int inc_fv, int inc_smplr)
	{
		std::unordered_map<int, int> childs_depth;
		for (TreeNode* child : childs)
		{
			//SceneObject* child_obj = child->object;
			//Type child_obj_type = child_obj->getType();
			//if      (child_obj_type == FACE_VECTOR && !inc_fv) continue;
			//else if (child_obj_type == SAMPLER_OBJECT && !inc_smplr) continue;

			int depth = 0;

			std::stack<TreeNode*> stack;
			for (TreeNode* node : child->childs) stack.push(node);
			while (stack.size() > 0)
			{
				TreeNode* current = stack.top();
				stack.pop();

				SceneObject* curr_obj = current->object;
				Type curr_type = curr_obj->getType();
				bool is_fv    = curr_type == FACE_VECTOR;
				bool is_smplr = curr_type == SAMPLER_OBJECT;  

				for (TreeNode* child : current->childs)
				{

					//SceneObject* child_obj = child->object;
					//Type child_obj_type = child_obj->getType();
					//if      (child_obj_type == FACE_VECTOR && !inc_fv) continue;
					//else if (child_obj_type == SAMPLER_OBJECT && !inc_smplr) continue;

					stack.push(child);
				}

				if ((is_fv && !inc_fv) || (is_smplr && !inc_smplr))
				{

				}
				else
					depth++;
			}

			childs_depth[child->object->getId()] = depth;
		}

		std::vector<TreeNode*> childs_sorted; for (TreeNode* child : childs) childs_sorted.push_back(child);

		// sorting //
		for (int i = 0; i < childs_sorted.size(); i++)
		{

			for (int j = i; j > 0; j--)
			{

				TreeNode* child_j = childs_sorted[j - 1];
				int id_j = child_j->object->getId();
				int depth_j = childs_depth[id_j];

				TreeNode* child_i = childs_sorted[j];
				int id_i = child_i->object->getId();
				int depth_i = childs_depth[id_i];

				if (depth_j > depth_i)
				{
					TreeNode* temp = child_j;
					childs_sorted[j - 1] = child_i;
					childs_sorted[j] = temp;
				}

				if (depth_j == depth_i)
				{
					int id_j = child_j->object->getId();
					int id_i = child_i->object->getId();

					if (id_j > id_i)
					{
						TreeNode* temp = child_j;
						childs_sorted[j - 1] = child_i;
						childs_sorted[j] = temp;
					}
				}
			}
		}

		//childs_depth_ordered = childs_sorted;
		return childs_sorted;
	}
	void update_childs_by_depth_order(int inc_fv, int inc_smplr)
	{
		std::unordered_map<int, int> childs_depth;
		for (TreeNode* child : childs)
		{
			SceneObject* child_obj = child->object;
			Type child_obj_type = child_obj->getType();
			if (child_obj_type == FACE_VECTOR && !inc_fv) continue;
			if (child_obj_type == SAMPLER_OBJECT && !inc_smplr) continue;

			int depth = 0;

			std::stack<TreeNode*> stack;
			for (TreeNode* node : child->childs) stack.push(node);
			while (stack.size() > 0)
			{
				TreeNode* current = stack.top();
				stack.pop();

				for (TreeNode* child : current->childs)
					stack.push(child);

				depth++;
			}

			childs_depth[child->object->getId()] = depth;
		}

		std::vector<TreeNode*> childs_sorted; for (TreeNode* child : childs) childs_sorted.push_back(child);

		// sorting //
		for (int i = 0; i < childs_sorted.size(); i++)
		{

			for (int j = i; j > 0; j--)
			{

				TreeNode* child_j = childs_sorted[j - 1];
				int id_j = child_j->object->getId();
				int depth_j = childs_depth[id_j];

				TreeNode* child_i = childs_sorted[j];
				int id_i = child_i->object->getId();
				int depth_i = childs_depth[id_i];

				if (depth_j > depth_i)
				{
					TreeNode* temp = child_j;
					childs_sorted[j - 1] = child_i;
					childs_sorted[j] = temp;
				}

				if (depth_j == depth_i)
				{
					int id_j = child_j->object->getId();
					int id_i = child_i->object->getId();

					if (id_j > id_i)
					{
						TreeNode* temp = child_j;
						childs_sorted[j - 1] = child_i;
						childs_sorted[j] = temp;
					}
				}
			}
		}

		childs_depth_ordered.clear();
		for (TreeNode* node : childs_sorted)
			childs_depth_ordered.push_back(node);
	}

	void remove(SceneObject* child_obj, bool del = false);
	void remove(TreeNode* child_node, bool del = false);
	void insertChild(TreeNode* child);
	void insertChild(SceneObject* child_obj);
	
	TreeNode() :parent(nullptr), object(nullptr), is_root(false) {}
	TreeNode(TreeNode* parent) :parent(parent), object(nullptr), is_root(false) {}
	TreeNode(SceneObject* object, bool is_root) :parent(nullptr), object(nullptr), is_root(is_root) {}
	TreeNode(bool is_root) :parent(nullptr), object(nullptr), is_root(is_root) {}

	~TreeNode()
	{
		for (TreeNode* child_node : childs)
			delete child_node;
	}

};

struct Hierarchy_Tree
{
private:

	bool is_dirty = false;
	TreeNode* rootNode;

	void      removeRecursively(TreeNode* node)
	{

	}

	std::vector<SceneObject*> temp_added_ut_childs;

public:

	Hierarchy_Tree() :rootNode(nullptr)
	{
		rootNode = new TreeNode(nullptr, true);
	}

	~Hierarchy_Tree()
	{
		delete rootNode;
		/*
		std::stack<TreeNode*> tree;
		tree.push(rootNode);
		while (!tree.empty)
		{
			TreeNode* current = tree.top();
			tree.pop();

			std::vector<TreeNode*> childs = current->childs;
			for (TreeNode* child : childs)
			{

			}
		}
		*/
	}

	TreeNode* getRoot()
	{
		return rootNode;
	}


	// BFS tree till find Node with key == object
	// otherwise return nullptr
	void print()
	{
		std::stack<TreeNode*> tree;
		tree.push(rootNode);
		while (tree.size() > 0)
		{
			TreeNode* current = tree.top();
			tree.pop();
			if (current->object != nullptr)
				std::cout << " -Node[" << current->object->getId() << "]:" << std::endl;
			else
				std::cout << " -RootNode:" << std::endl;

			for (TreeNode* node : current->childs)
			{
				//std::cout << "      -child[" << node->object->getId() << "]" << std::endl;
				tree.push(node);
			}
		}
	}

	TreeNode* find(SceneObject * object)
	{
		
		std::stack<TreeNode*> tree;
		tree.push(rootNode);

		while (tree.size() > 0)
		{

			TreeNode* current = tree.top();
			tree.pop();

			//if (current->object == object)
			//	return current;
			
			for (int i = 0; i < current->childs.size(); i++)
			{
				
				if (current->getChild(i)->object == object)
					return current->getChild(i);

				tree.push(current->getChild(i));
			}
		}

		return nullptr;

	}

	void insert(SceneObject* object)
	{
		// insert Node with key = object at 1st level of the tree
		//TreeNode* node = new TreeNode();
		//node->object = object;
		rootNode->insertChild(object);
		markDirty();
	}
	void insert(TreeNode* node)
	{
		// insert Node with key = object at 1st level of the tree
		rootNode->insertChild(node);
		markDirty();
	}
	void insertChild(SceneObject* object, SceneObject* parent_obj)
	{
		// traverse tree till find Node with key == parent
		// parent.insertChild( object )
		TreeNode* parent = find(parent_obj);
		parent->insertChild(object);
		markDirty();
	}
	void insertChild(TreeNode* child, SceneObject* parent_obj)
	{
		// traverse tree till find Node with key == parent
		// parent.insertChild( object )
		TreeNode* parent = find(parent_obj);
		parent->insertChild(child);
		markDirty();
	}
	void move(TreeNode* child_node, TreeNode* parent_node)
	{
		if( child_node->parent != nullptr )
			child_node->parent->remove(child_node);
		parent_node->insertChild(child_node);
		markDirty();
	}
	void move(SceneObject* object, TreeNode* parent_node)
	{
		TreeNode* child = find(object);
		if (child->parent != nullptr)
			child->parent->remove(child);
		parent_node->insertChild(child);
		markDirty();
		
	}
	void move(SceneObject* object, SceneObject* parent_obj)
	{
		TreeNode* child = find(object);
		TreeNode* parent = find(parent_obj);

		if(child->parent != nullptr)
			child->parent->remove(child);

		parent->insertChild(object);
		markDirty();
	}

	void order_Node_Childs_by_depth_Order(int inc_fv = 0, int inc_smplr = 0);
	void insert_Utility_Childs(int fv, int smplr, int lights);
	void clear_Utility_Childs();
	
	void remove(SceneObject* object, bool del = false)
	{

		// traverse tree till find Node with key == object
		// add Node childs to 1st level of the tree
		// remove Node

		TreeNode* node = find(object);

		if (node == nullptr) return;

#ifdef CANCER_DEBUG

		if (object == nullptr || node == nullptr )
		{
			//std::cout << "\n HierarchyTree::remove( SceneObject ) " << std::endl;
			if (object != nullptr)
				//std::cout << "    - SceneObject : " << object->getId() << std::endl;
			if (node != nullptr)
				//std::cout << "    - Node_object : " << node->object->getId() << std::endl;
			
			//std::cout << "    - SceneObject = " << !(object == nullptr) << std::endl;
			//std::cout << "    - Node        = " << !(node == nullptr) << std::endl;

			if (node != nullptr)
				if (node->parent == nullptr)
					//std::cout << "    - Node.parent = nullptr!" << std::endl;
			system("pause");
			return;
		}

#endif

		for (TreeNode* child : node->childs)
			insert(child);

		//if( node->parent != nullptr )
			node->parent->remove(node);
		
		markDirty();
		//if(del)
		//	delete node;
		
	}
	void remove(TreeNode* node, bool del = false)
	{

#ifdef CANCER_DEBUG

		if (node == nullptr)
		{
			//std::cout << "\n HierarchyTree::remove( TreeNode ) " << std::endl;
			//std::cout << "    - Node        = " << !(node == nullptr) << std::endl;
			system("pause");
		}
#endif
		for (TreeNode* child : node->childs)
			insert(child);

		node->parent->remove(node);
		
		//if (del)
		//	delete node;
		markDirty();
	}

	void markDirty();
	bool checkIntegrity();
	bool isDirty();


	static Hierarchy_Tree* clone(Hierarchy_Tree* src)
	{
		Hierarchy_Tree* cln = new Hierarchy_Tree();
		clone(cln, src);
		return cln;
	}
	static void clone(Hierarchy_Tree* cln, Hierarchy_Tree* src)
	{
		//std::cout << "\n - Hierarchy_Tree::clone( *clone, *src ):" << std::endl;

		std::stack<TreeNode*> tree_nodes;
		for (TreeNode* node : src->getRoot()->childs)
		{
			SceneObject* obj = node->object;
			bool is_valid = obj->isActive() && !obj->isTemporary();

			if(is_valid)
				tree_nodes.push(node);
		}

		while (tree_nodes.size() > 0)
		{

			TreeNode* node = tree_nodes.top(); tree_nodes.pop();
			TreeNode* node_parent = node->parent;
			SceneObject* object   = node->object;

			
			//std::string p_name = node_parent->is_root ? "root_node" : node_parent->object->getName();
			bool is_temporary = node->object->isTemporary();
			if (is_temporary) 
				continue;

			//std::cout << "\n   -> Inserting node : " << node->object->getName() << std::endl;
			//std::cout <<   "    - node_parent : " << p_name << std::endl;
			//std::cout <<   "    - type : " << node->object->getType() << std::endl;
			//std::cout <<   "    - temporary : " << node->object->isTemporary() << std::endl;
			//std::cout <<   "    - active : " << node->object->isActive() << std::endl;

			if (node_parent->is_root)
			{
				//std::cout << " - cln->insert( " << object->getName() << " )" << std::endl;
				cln->insert(object);
			}
			else
			{
				//std::cout << " - cln->insertChild( object = " << object->getName() << " , parent = " << p_name << " ) " << std::endl;
				cln->insertChild(object, node_parent->object);
			}

			for (TreeNode* node_child : node->childs)
				tree_nodes.push(node_child);

		}

		//std::cout << "\n - Hierarchy_Tree::clone( .. )_END_" << std::endl;
	}
	
	
	std::vector<ObjectStruct> as_List()
	{
		std::vector<ObjectStruct> list;

		std::stack<TreeNode*> tree;
		for (TreeNode* node : rootNode->childs) tree.push(node);

		while (tree.size() > 0)
		{
			TreeNode* node = tree.top(); tree.pop();
			TreeNode* parent_node = node->parent;

			list.push_back(
				ObjectStruct(node->object,
				             node->parent->object,
				             (node->group!=0? node->group->object: nullptr),
				             node->Childs_asObjects()) 
			);

			for (TreeNode* child : node->childs)
				tree.push(child);

		}

		return list;

	}


};



struct TreeNode_Struct
{
	TreeNode* node = 0;
	int depth = 0;
	TreeNode_Struct(TreeNode* node, int depth)
		: node(node), depth(depth) {}
};