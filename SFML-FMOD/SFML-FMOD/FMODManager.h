#pragma once
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include "utils.h"

class FMODManager {
	FMODManager();
	FMOD::Studio::System* fSystem = nullptr;

	FMOD::Studio::Bank* soundBank = nullptr;
	FMOD::Studio::Bank* stringBank = nullptr;

	std::unordered_map<std::string, FMOD::Studio::EventDescription*> eventDescriptions;
	std::unordered_map<std::string, FMOD::Studio::EventInstance*> eventInstances;

public:
	// Play an event
	FMOD_RESULT playEvent(std::string eventName);
	// Set a parameter of an event
	FMOD_RESULT setEventParameter(std::string eventName, std::string paramName, float paramValue);
	// Stop an event
	FMOD_RESULT stopEvent(std::string eventName, FMOD_STUDIO_STOP_MODE mode);

	// Play an event once
	void playOneshotEvent(std::string eventName);
	// Play an event once at a certain position
	void playOneshotSpatial(std::string eventName, sf::Vector2f pos);

	// Set the location of the listener in the game world
	void setListenerPosition(sf::Vector2f pos);

	// Load a bank into memory
	FMOD_RESULT loadBank(std::string bankPath, FMOD::Studio::Bank** bank);

	// Update the FMOD system so that queued events and commands are processed.
	void updateSystem();

	// singleton stuff
	FMODManager(const FMODManager&) = delete;
	FMODManager(FMODManager&&) = delete;
	FMODManager& operator=(const FMODManager&) = delete;
	FMODManager& operator=(FMODManager&&) = delete;

	static FMODManager& Instance()
	{
		static FMODManager mgr{};
		return mgr;
	}
};
