#include "AssetManager.h"
#include <algorithm>
#include <iostream>

std::unordered_map<std::string, class GeometryNode *> AssetManager::Assets;
std::unordered_map<std::string, GLuint > AssetManager::Textures;


AssetManager::AssetManager()
{

}

AssetManager::~AssetManager()
{


}


void AssetManager::clearData()
{
	Assets.erase(Assets.begin() , Assets.end());

	std::unordered_map<std::string, GLuint>::iterator it = Textures.begin();
	while (it != Textures.end())
	{
		glDeleteTextures(1, &it->second);
		it++;
	}
}

bool AssetManager::loadAssets()
{

	OBJLoader loader;
	bool initialized = true;
	// load geometric object 1
	auto mesh = loader.load("../Data/Assets/Sphere/sphere.obj");
	if (mesh != nullptr)
	{
		//GeometryNode * node = new GeometryNode();
		//node->Init(mesh);
		//Assets["sphere"] = node;
		Assets["sphere"] = new GeometryNode();
		Assets["sphere"]->Init(mesh);
	}
	
	mesh = loader.load("../Data/Assets/Terrain/terrain.obj");
	if (mesh != nullptr)
	{
		Assets["terrain"] = new GeometryNode();
		Assets["terrain"]->Init(mesh);
	}
	else
		initialized = false;
	
	mesh = loader.load("../Data/Assets/Terrain/road.obj");
	if (mesh != nullptr)
	{
		Assets["road"] = new GeometryNode();
		Assets["road"]->Init(mesh);
	}
	else
		initialized = false;
	
	mesh = loader.load("../Data/Assets/MedievalTower/tower.obj");
	if (mesh != nullptr)
	{
		Assets["tower"] = new GeometryNode();
		Assets["tower"]->Init(mesh);
	}
	else
		initialized = false;
	
	mesh = loader.load("../Data/Assets/Treasure/treasure_chest.obj");
	if (mesh != nullptr)
	{
		Assets["chest"] = new GeometryNode();
		Assets["chest"]->Init(mesh);
	}
	else
		initialized = false;
	
	mesh = loader.load("../Data/Assets/Pirate/pirate_body.obj");
	if (mesh != nullptr)
	{
		Assets["pirate_body"] = new GeometryNode();
		Assets["pirate_body"]->Init(mesh);
	}
	else
		initialized = false;

	mesh = loader.load("../Data/Assets/Pirate/pirate_arm.obj");
	if (mesh != nullptr)
	{
		Assets["pirate_arm"] = new GeometryNode();
		Assets["pirate_arm"]->Init(mesh);
	}
	else
		initialized = false;

	mesh = loader.load("../Data/Assets/Pirate/pirate_left_foot.obj");
	if (mesh != nullptr)
	{
		Assets["pirate_left_foot"] = new GeometryNode();
		Assets["pirate_left_foot"]->Init(mesh);
	}
	else
		initialized = false;

	mesh = loader.load("../Data/Assets/Pirate/pirate_right_foot.obj");
	if (mesh != nullptr)
	{
		Assets["pirate_right_foot"] = new GeometryNode();
		Assets["pirate_right_foot"]->Init(mesh);
	}
	else
		initialized = false;

	mesh = loader.load("../Data/Assets/Various/cannonball.obj");
	if (mesh != nullptr)
	{
		Assets["cannon_ball"] = new GeometryNode();
		Assets["cannon_ball"]->Init(mesh);
	}
	else
		initialized = false;

	mesh = loader.load("../Data/Assets/Various/plane_green.obj");
	if (mesh != nullptr)
	{
		Assets["green_plane"] = new GeometryNode();
		Assets["green_plane"]->Init(mesh);
	}
	else
		initialized = false;

	mesh = loader.load("../Data/Assets/Various/plane_red.obj");
	if (mesh != nullptr)
	{
		Assets["red_plane"] = new GeometryNode();
		Assets["red_plane"]->Init(mesh);
	}
	else
		initialized = false;


	mesh = loader.load("../Data/Assets/Meteorite/Asteroid01.obj");
	if (mesh != nullptr)
	{
		Assets["asteroid01"] = new GeometryNode();
		Assets["asteroid01"]->Init(mesh);
	}
	else
		initialized = false;


	mesh = loader.load("../Data/Assets/Meteorite/Asteroid02.obj");
	if (mesh != nullptr)
	{
		Assets["asteroid02"] = new GeometryNode();
		Assets["asteroid02"]->Init(mesh);
	}
	else
		initialized = false;


	mesh = loader.load("../Data/Assets/Meteorite/Asteroid03.obj");
	if (mesh != nullptr)
	{
		Assets["asteroid03"] = new GeometryNode();
		Assets["asteroid03"]->Init(mesh);
	}
	else
		initialized = false;

	mesh = loader.load("../Data/Assets/Various/20facestar.obj");
	if (mesh != nullptr)
	{
		Assets["star"] = new GeometryNode();
		Assets["star"]->Init(mesh);
	}
	else
		initialized = false;

	
	mesh = loader.load("../Data/Assets/Various/cassini.obj");
	if (mesh != nullptr)
	{
		Assets["cassini"] = new GeometryNode();
		Assets["cassini"]->Init(mesh);
	}
	else
		initialized = false;
	
	mesh = loader.load("../Data/Assets/Various/Moon.obj");
	if (mesh != nullptr)
	{
		Assets["moon"] = new GeometryNode();
		Assets["moon"]->Init(mesh);
	}
	else
		initialized = false;

	mesh = loader.load("../Data/Assets/Various/coin.obj");
	if (mesh != nullptr)
	{
		Assets["coin"] = new GeometryNode();
		Assets["coin"]->Init(mesh);
	}
	else
		initialized = false;


	return initialized;
}

bool AssetManager::loadTextures()
{
	
	char * filepath = "../Data/Assets/Terrain/maps/dwestonefloor02.jpg";
	Textures["floor02"] = TextureManager::GetInstance().RequestTexture(filepath);

	filepath = "../Data/Assets/Terrain/maps/engw_t.jpg";
	Textures["engw_t"] = TextureManager::GetInstance().RequestTexture(filepath);


	filepath = "../Data/Assets/Terrain/maps/spnza_bricks_a_diff.jpg";
	Textures["bricks"] = TextureManager::GetInstance().RequestTexture(filepath);


	filepath = "../Data/Assets/Terrain/maps/stoneblocks01n.jpg";
	Textures["stones01"] = TextureManager::GetInstance().RequestTexture(filepath);


	filepath = "../Data/Assets/Terrain/maps/stoneblocks02.jpg";
	Textures["stones02"] = TextureManager::GetInstance().RequestTexture(filepath);


	filepath = "../Data/Assets/Menu/bones.png";
	Textures["bones"] = TextureManager::GetInstance().RequestTexture(filepath);


	filepath = "../Data/Assets/Menu/coins.png";
	Textures["coins"] = TextureManager::GetInstance().RequestTexture(filepath);

	filepath = "../Data/Assets/Menu/Tower.png";
	Textures["Tower1"] = TextureManager::GetInstance().RequestTexture(filepath);


	filepath = "../Data/Assets/Menu/Tower2.png";
	Textures["Tower2"] = TextureManager::GetInstance().RequestTexture(filepath);

	
	filepath = "../Data/Assets/Menu/info.png";
	Textures["info"] = TextureManager::GetInstance().RequestTexture(filepath);


	filepath = "../Data/Assets/Menu/superpower_meteorite1.png";
	Textures["sp_meteorite1"] = TextureManager::GetInstance().RequestTexture(filepath);

	filepath = "../Data/Assets/Menu/superpower_meteorite2.png";
	Textures["sp_meteorite2"] = TextureManager::GetInstance().RequestTexture(filepath);

	const char * numbers[] = {
		"../Data/Assets/Menu/0.png",
		"../Data/Assets/Menu/1.png",
		"../Data/Assets/Menu/2.png",
		"../Data/Assets/Menu/3.png",
		"../Data/Assets/Menu/4.png",
		"../Data/Assets/Menu/5.png",
		"../Data/Assets/Menu/6.png",
		"../Data/Assets/Menu/7.png",
		"../Data/Assets/Menu/8.png",
		"../Data/Assets/Menu/9.png"
	};

	const char * numbers2[] = {
		"../Data/Assets/Various/DamageDigits/0.png",
		"../Data/Assets/Various/DamageDigits/1.png",
		"../Data/Assets/Various/DamageDigits/2.png",
		"../Data/Assets/Various/DamageDigits/3.png",
		"../Data/Assets/Various/DamageDigits/4.png",
		"../Data/Assets/Various/DamageDigits/5.png",
		"../Data/Assets/Various/DamageDigits/6.png",
		"../Data/Assets/Various/DamageDigits/7.png",
		"../Data/Assets/Various/DamageDigits/8.png",
		"../Data/Assets/Various/DamageDigits/9.png"
	};

	for (int i = 0; i < 10; i++)
	{
		std::string numb = "numb_"+std::to_string(i);
		std::string dmg_numb = "dmg_numb_" + std::to_string(i);

		Textures[numb.c_str()] = TextureManager::GetInstance().RequestTexture(numbers[i]);
		Textures[dmg_numb.c_str()] = TextureManager::GetInstance().RequestTexture(numbers2[i]);
	}

	return true;
}

class GeometryNode * AssetManager::operator[](const std::string key)
{
	//std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	auto it = Assets.find(key);
	return (it != Assets.end()) ? it->second : nullptr;
}

class GeometryNode * AssetManager::GetIndex(const std::string key)
{
	//std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	auto it = Assets.find(key);
	return (it != Assets.end()) ? it->second : nullptr;
}

class GeometryNode * * AssetManager::GetArray(const std::string * keys, int length)
{
	GeometryNode ** MeshArray = new GeometryNode * [length];

	for (int i = 0; i < length; i++)
	{
		MeshArray[i] = Assets[keys[i]];
	}

	return MeshArray;
}

GLuint * AssetManager::getTexture(std::string key)
{
	auto it = Textures.find(key);
	return (it != Textures.end()) ? &it->second : nullptr;
}

