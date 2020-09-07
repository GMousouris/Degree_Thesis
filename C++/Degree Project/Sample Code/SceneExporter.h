#pragma once



#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

#include <filesystem>
#include "Transformations.h"

#include <direct.h>
#include <stack>
#include <vector>

#include "JEntity.h"


#define _quote_ "\""
#define OFFSET_JUMP "  "
#define INIT_DEPTH -1

struct JNode
{
	int depth = INIT_DEPTH;
	JEntity JE;
	struct JNode* parent = nullptr;
	std::vector<JNode*> childs;
	bool IS_ROOT = false;

	JNode();
	JNode(JEntity JE, int depth);
	void push(JNode* child);
	void clear();

	bool visited = false;
};
class JTree
{


private:

	JNode * ROOT = 0;


public:

	JTree() { ROOT = new JNode(); ROOT->IS_ROOT = true; }
	~JTree() // delete all nodes
	{
		//std::cout << "\n - deleting JTree!" << std::endl;
		std::vector<JNode*> nodes_to_delete;
		std::stack<JNode*> nodes;
		for (JNode* node : ROOT->childs) nodes.push(node);
		while (nodes.size() > 0)
		{
			JNode* node = nodes.top(); nodes.pop();
			nodes_to_delete.push_back(node);
			for (JNode* child : node->childs)
				nodes.push(child);
		}

		for (JNode* node : nodes_to_delete)
		{
			delete node;
		}

		delete ROOT;
		ROOT = 0;
		//std::cout << " - bye - " << std::endl;

	}
	JNode* root() { return ROOT; }


};

class SceneExporter
{
	

private:
	int JID = 0;

	std::string filename;
	std::string offset = "";

	std::ofstream output;
	bool initialized = false;
	bool is_stream_open   = false;
	bool is_stream_closed = false;

	
	int offset_index = -1;
	std::vector<JEntity> JStack; // ~ Init
	int stack_index = -1;


	std::stack<JNode*> JNodes;
	JTree * Jtree = 0;
	JNode * activeNode = 0;

	

public:

	SceneExporter();
	SceneExporter(std::string filename);
	~SceneExporter();



public:

	void Init();
	SceneExporter& Start( ... );
	SceneExporter& Start(std::vector<JEntity> list);
	SceneExporter& End(std::string data = "");
	void Reset();
	void Clear();

	void Export();

private: 

	void open_Stream(std::string msg="");
	void close_Stream(std::string msg ="");
	void clear_Stream(std::ios::_Iostate state = std::ios::goodbit);
	
public:

	//
	void print(std::string data ="");
	void println(std::string data ="");
	void emit(std::string data = "", int offset = 0);
	void emitln(std::string data = "", int offset = 0);

	void out(std::string data = "", int offset = 0);
	void outln(std::string data = "", int offset = 0);

	JEntity& open(std::string label  = "");
	JEntity& entry(std::string label = "");
	SceneExporter& close(std::string label = "");

	JEntity& Spacing();

	JEntity& request(int JID);

	

	const JEntity& activeEntity();
	void clearEntities();

	void setActiveNode(int jid);
	void passActivityToParent();

	//

	//

	//
	std::ofstream& GetStream();
	bool HasStarted();
	bool isGood();

	
};

