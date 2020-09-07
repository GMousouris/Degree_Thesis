#include "SoundEngine.h"
#include <iostream>


irrklang::ISoundEngine * SoundEngine::soundEngine = irrklang::createIrrKlangDevice();
std::unordered_map<std::string, irrklang::ISoundSource* > SoundEngine::sounds;

SoundEngine::SoundEngine()
{

}

SoundEngine::~SoundEngine()
{
	soundEngine->drop();
}

bool SoundEngine::InitEngine()
{
	std::string file = "dizzy";
	std::string filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.3);

	file = "coin";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "coin_cluster1";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "coin_cluster2";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "coin_cluster3";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "coin_cluster4";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "coin_cluster5";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "coin_cluster6";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);


	file = "expl";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "expl_double";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "footsteps";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.3);

	file = "jump1";
	filepath = "../Data/Assets/Sounds/"+file+".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "jump2";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "jump3";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "jump4";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.2);

	file = "pain1";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "pain2";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "pain3";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "pain4";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "portal_3";
	filepath = "../Data/Assets/Sounds/"+ file +".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.1);

	file = "portal_2";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "portal_1";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "pushback";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "Select";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "falling1";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "falling2";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "interaction1";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);

	file = "interaction2";
	filepath = "../Data/Assets/Sounds/" + file + ".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.5);


	file = "theme";
	filepath = "../Data/Assets/Sounds/"+file+".wav";
	sounds[file] = soundEngine->addSoundSourceFromFile(filepath.c_str());
	sounds[file]->setDefaultVolume(0.28);

	return true;
}

void SoundEngine::setDefaultVolume(const char * file ,double volume)
{
	sounds[file]->setDefaultVolume(volume);
}

void SoundEngine::startTheme(const char * file)
{

}


void SoundEngine::PlaySound(const char * file,  bool loop)
{
#ifdef SOUNDS
	soundEngine->play2D(sounds[file], loop);
#endif
}


void SoundEngine::PlaySound(const char * file, int index, bool loop)
{

#ifdef  SOUNDS
	std::string sound = file + std::to_string(index);
	PlaySound(sound.c_str(), false);
#endif //  SOUNDS

}


void SoundEngine::PlayRandomSoundFrom(const char * file, int a, int b ,bool loop)
{

#ifdef SOUNDS

	std::mt19937 rng;
	rng.seed(std::random_device()());
	std::uniform_int_distribution<> rand(a,b);
	int random = rand(rng);

	std::string sound = std::to_string(random);
	sound = file + sound;

	PlaySound(sound.c_str(), false);
#endif
	
}


void SoundEngine::RemoveSound(const char * file)
{
	soundEngine->removeSoundSource(sounds[file]);
}
