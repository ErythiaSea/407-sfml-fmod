#include "SoundObject.h"

// NOTE FOR CMP407 MARKERS:
// This exists for use with AudioManager, which is a generic interface with SFML's audio system.
// It is NOT used in this project! All audio processing is done through FMOD.

SoundObject::SoundObject(std::string filename)
{
	loadSound(filename);
}

void SoundObject::loadSound(std::string filename)
{
	buffer = new sf::SoundBuffer();
	if (buffer->loadFromFile(filename) == false) return;

	sounds.clear();
	sounds.push_back(new sf::Sound(*buffer));
}

void SoundObject::setMaxConcurrent(int maxConcurrent)
{
	if (maxConcurrent < 1) maxConcurrent = 1;
	max_playing = maxConcurrent;

	sf::Sound originalSound(*sounds[0]);
	sounds.clear();
	for (int i = 0; i < maxConcurrent; i++) {
		sounds.push_back(new sf::Sound(originalSound));
	}
}

sf::Sound* SoundObject::getSound()
{
	return sounds[0];
}

std::vector<sf::Sound*>* SoundObject::getSounds()
{
	return &sounds;
}

sf::Sound* SoundObject::getFreeSound()
{
	for (sf::Sound* snd : sounds) {
		if (snd->getStatus() != sf::SoundSource::Status::Playing) return snd;
	}
	return nullptr;
}


/////////////////////////////////////////////////////////////////////////


MusicObject::MusicObject(std::string filename)
{
	path = filename;
}

void MusicObject::setSongLoopPoints(sf::Music::TimeSpan span)
{
	loop_points = span;
}

void MusicObject::setSongLooping(bool loop)
{
	loop_song = loop;
}

void MusicObject::setSongVolume(float vol)
{
	if (vol < 0) vol = 0;
	if (vol > 100) vol = 100;

	default_volume = vol;
}
