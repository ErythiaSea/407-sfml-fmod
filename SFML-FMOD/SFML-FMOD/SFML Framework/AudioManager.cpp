#include "AudioManager.h"

// NOTE FOR CMP407 MARKERS:
// This exists to be a generic interface with SFML's audio system.
// It is NOT used in this project! All audio processing is done through FMOD.
// See the FMODManager class.

MusicObject* AudioManager::addMusic(std::string filename, std::string key)
{
	// key exists in map so can't make the music object
	if (music_objects.count(key) > 0) return nullptr;

	// create object, store in map and return it
	music_objects[key] = MusicObject(filename);
	return &music_objects[key];
}

SoundObject* AudioManager::addSound(std::string filename, std::string key)
{
	// key exists in map so can't make the sound object
	if (sound_objects.count(key) > 0) return nullptr;

	// create object, store in map and return it;
	sound_objects[key] = SoundObject(filename);
	return &sound_objects[key];
}

sf::Music* AudioManager::getMusicStream()
{
	return &music;
}

MusicObject* AudioManager::getMusicObject(std::string key)
{
	if (music_objects.count(key) == 0) return nullptr;
	return &music_objects[key];
}

SoundObject* AudioManager::getSoundObject(std::string key)
{
	if (sound_objects.count(key) == 0) return nullptr;
	return &sound_objects[key];
}

void AudioManager::playMusic(std::string key)
{
	// can't play music that doesn't exist
	if (music_objects.count(key) == 0) return;

	// get the music object and load the file into the stream
	MusicObject musicObj = music_objects[key];
	if (music.openFromFile(musicObj.getFilename()) == false) return;

	// assign this key as the last song played
	last_song_key = key;

	// If the song loops, set music stream to loop
	music.setLooping(musicObj.getSongLooping());

	// If it has custom loop points, use them
	if (musicObj.getLoopPoints().length > sf::Time::Zero) {
		music.setLoopPoints(musicObj.getLoopPoints());
	}
	// Otherwise, reset the loop points
	else {
		music.setLoopPoints({ sf::Time::Zero, music.getDuration() });
	}

	// Set the volume of the music stream
	music.setVolume(music_volume * (musicObj.getSongVolume() / 100));

	music.play();
}

void AudioManager::playSound(std::string key)
{
	if (sound_objects.count(key) == 0) return;

	SoundObject soundObj = sound_objects[key];
	if (soundObj.getMaxConcurrent() == 1) {
		soundObj.getSound()->play();
	}
	else {
		sf::Sound* freeSound = soundObj.getFreeSound();
		if (freeSound == nullptr) { 
			soundObj.getSound()->play();
			return;
		}
		freeSound->play();
	}
}

void AudioManager::stopMusic()
{
	music.stop();
}

void AudioManager::stopAllSounds()
{
	for (auto& keyValPair : sound_objects) {
		SoundObject sObj = keyValPair.second;
		for (sf::Sound* snd : *sObj.getSounds()) {
			snd->stop();
		}
	}
}

void AudioManager::setMusicVolume(float vol)
{
	if (vol < 0) vol = 0;
	if (vol > 100) vol = 100;
	music_volume = vol;

	if (last_song_key == "") return;

	// adjust the volume of the music stream based on music and song volume
	music.setVolume(music_volume * (music_objects[last_song_key].getSongVolume() / 100));
}
