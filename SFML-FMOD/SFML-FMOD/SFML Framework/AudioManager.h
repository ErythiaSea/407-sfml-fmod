#pragma once
#include "SFML/Audio.hpp"
#include "SoundObject.h"
#include <map>
#include <string>

// NOTE FOR CMP407 MARKERS:
// This exists to be a generic interface with SFML's audio system.
// It is NOT used in this project! All audio processing is done through FMOD.
// See the FMODManager class.

class AudioManager
{
public:
	AudioManager() {};
	~AudioManager() {};

	// Create a music object with a key string. Returns a pointer to the created object, or nullptr if it could not be created.
	MusicObject* addMusic(std::string filename, std::string key);
	// Create a sound object with a key string. Returns a pointer to the created object, or nullptr if it could not be created.
	SoundObject* addSound(std::string filename, std::string key);

	// Get pointer to music stream
	sf::Music* getMusicStream();
	// Get pointer to music object
	MusicObject* getMusicObject(std::string key);
	// Get pointer to sound object from key string if it exists, or nullptr if it does not
	SoundObject* getSoundObject(std::string key);

	// Plays music from the string key if it exists, and configures the sf::Music stream with loop information.
	void playMusic(std::string key);
	// Plays a sound from the string key if it exists
	void playSound(std::string key);

	// Stops playing music.
	void stopMusic();
	// Stops playing all sounds.
	void stopAllSounds();

	// Set the volume of the music stream.
	void setMusicVolume(float vol);
	// Get the volume of the music stream.
	float getMusicVolume() const { return music_volume; }
private:
	// value is string because music is loaded from filename into sf::Music object
	std::map<std::string, MusicObject> music_objects;
	// map of sound keynames to sound objects
	std::map<std::string, SoundObject> sound_objects;

	// the music stream
	sf::Music music;

	// the last played song's key
	std::string last_song_key = "";
	// the volume of the music stream
	float music_volume = 100.0f;
};

