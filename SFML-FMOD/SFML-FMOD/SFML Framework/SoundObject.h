#pragma once
#include "SFML/Audio.hpp"

// NOTE FOR CMP407 MARKERS:
// This exists for use with AudioManager, which is a generic interface with SFML's audio system.
// It is NOT used in this project! All audio processing is done through FMOD.
// See the FMODManager class.

class SoundObject
{
public:
	SoundObject(){};
	SoundObject(std::string filename);

	// Load a new sound.
	void loadSound(std::string filename);
	// Set the maximum number of this sound effect that can play at once. 
	// This creates copies of the original sound, so modify the original sound first and call this function last (to preserve e.g. volume changes.)
	void setMaxConcurrent(int maxConcurrent);

	// Get the sound. If setMaxConcurrent() was called, use getSounds() and iterate through the vector!
	sf::Sound* getSound();
	// Get the vector of all sounds.
	std::vector<sf::Sound*>* getSounds();

	// Get the earliest sound in the vector that isn't currently playing, or nullptr if all are playing.
	sf::Sound* getFreeSound();

	// Get how many sounds can play at once.
	int getMaxConcurrent() const { return max_playing; }


private:
	sf::SoundBuffer* buffer = nullptr;
	std::vector<sf::Sound*> sounds;
	int max_playing = 1;
};

class MusicObject
{
public:
	MusicObject(){};
	MusicObject(std::string filename);

	// Set looping points for this song. These will be used automatically when the song is played.
	void setSongLoopPoints(sf::Music::TimeSpan span);
	// Set whether the song should loop or not when played.
	void setSongLooping(bool loop);
	// Set the default volume for this song. Won't affect this song if it's currently playing!
	void setSongVolume(float vol);

	// Whether or not the song should loop when played.
	bool getSongLooping() const { return loop_song; }
	// The points between which the song will loop.
	sf::Music::TimeSpan getLoopPoints() const { return loop_points; }
	// The default volume for the song.
	float getSongVolume() const { return default_volume; }
	// The filename for this song.
	std::string getFilename() { return path; };

private:
	std::string path;
	sf::Music::TimeSpan loop_points;
	bool loop_song = true;
	float default_volume = 100.0f;
};

