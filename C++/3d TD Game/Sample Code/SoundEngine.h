#pragma once
#include "Irrklang/irrKlang.h"
#include <string>
#include <unordered_map>
#include <random>
#include "Defines.h"




class SoundEngine
{


	

private:

	double volume;
	const char * theme_song;
	static std::unordered_map<std::string, irrklang::ISoundSource* > sounds;
	static irrklang::ISoundEngine * soundEngine;

	
public:

	SoundEngine();
	~SoundEngine();

	static bool InitEngine();

	static void setDefaultVolume(const char * file , double volume);
	static void startTheme(const char * file);
	static void PlaySound(const char * file,  bool loop);
	static void PlaySound(const char * file, int index , bool loop);
	static void PlayRandomSoundFrom(const char * file, int a , int b , bool loop);
	static void RemoveSound(const char * file);

};