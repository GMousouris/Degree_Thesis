#pragma once

#include <string>
#include <unordered_map>
#include "OBJLoader.h"
#include "GeometryNode.h"
#include "TextureManager.h"


class AssetManager
{

public:

	
	AssetManager();
	~AssetManager();

	static void clearData();

	static bool loadAssets();
	static bool loadTextures();
	class GeometryNode * operator[](const std::string key);
	static class GeometryNode * GetIndex(const std::string key);
	static GLuint * getTexture(std::string key);
	static class GeometryNode * * GetArray(const std::string * keys, int length);

private:

	static std::unordered_map<std::string, class GeometryNode *> Assets;
	static std::unordered_map<std::string, GLuint > Textures;

};