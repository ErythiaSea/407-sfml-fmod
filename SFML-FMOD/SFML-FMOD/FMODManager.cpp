#include "FMODManager.h"

FMODManager::FMODManager()
{
	// create and initialise studio system
	FMOD::Studio::System::create(&fSystem);
	fSystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);

	// load bank files
	FMOD_RESULT res;
	res = loadBank("sfx/Master.bank", &soundBank);
	res = loadBank("sfx/Master.strings.bank", &stringBank);

	// load event descriptions from bank
	std::vector<FMOD::Studio::EventDescription*> eventDescs;
	int eventCount = 0;
	soundBank->getEventCount(&eventCount);
	eventDescs.resize(eventCount);
	soundBank->getEventList(eventDescs.data(), eventCount, &eventCount);

	// get event names so we can call them nicely
	for (auto& desc : eventDescs) {
		char path[128];
		int retrieved;
		desc->getPath(path, 128, &retrieved);

		// format of path is "event:/EventName", so we can ignore chars 0-6
		// Utils::printMsg(std::format("path: {}", path));

		std::string subPath = path;
		subPath = subPath.substr(7, 128);
		Utils::printMsg(std::format("subPath: {}", subPath));

		// add description to map with string name as key,
		// then make an instance of it with the same key
		eventDescriptions[subPath] = desc;
		eventInstances[subPath] = nullptr;
		desc->createInstance(&eventInstances.at(subPath));
	}
}

FMOD_RESULT FMODManager::playEvent(std::string eventName)
{
	if (eventInstances.count(eventName) == 0) {
		Utils::printMsg(std::format("Event with name {} wasn't found!", eventName), error);
		return FMOD_RESULT_FORCEINT;
	}
	return eventInstances.at(eventName)->start();
}

FMOD_RESULT FMODManager::setEventParameter(std::string eventName, std::string paramName, float paramValue)
{
	if (eventInstances.count(eventName) == 0) {
		Utils::printMsg(std::format("Event with name {} wasn't found!", eventName), error);
		return FMOD_RESULT_FORCEINT;
	}

	FMOD_STUDIO_PARAMETER_DESCRIPTION paramDesc;
	FMOD_RESULT res;
	res = eventDescriptions.at(eventName)->getParameterDescriptionByIndex(0, &paramDesc); 
	if (res != FMOD_OK)
	{
		Utils::printMsg(std::format("Parameter with name {} wasn't found!", paramName), error);
		return res;
	}
	return eventInstances.at(eventName)->setParameterByName(paramName.c_str(), paramValue);
}

FMOD_RESULT FMODManager::stopEvent(std::string eventName, FMOD_STUDIO_STOP_MODE mode)
{
	if (eventInstances.count(eventName) == 0) {
		Utils::printMsg(std::format("Event with name {} wasn't found!", eventName), error);
		return FMOD_RESULT_FORCEINT;
	}
	return eventInstances.at(eventName)->stop(mode);
}

void FMODManager::playOneshotEvent(std::string eventName)
{
	if (eventDescriptions.count(eventName) == 0) {
		Utils::printMsg(std::format("Event with name {} wasn't found!", eventName), error);
		return;
	}

	// make a new instance, start it, and queue it for release once it finishes
	FMOD::Studio::EventInstance* instance = nullptr;
	eventDescriptions.at(eventName)->createInstance(&instance);
	FMOD_RESULT res = instance->start();
	instance->release();
}

void FMODManager::playOneshotSpatial(std::string eventName, sf::Vector2f pos)
{
	if (eventDescriptions.count(eventName) == 0) {
		Utils::printMsg(std::format("Event with name {} wasn't found!", eventName), error);
		return;
	}

	// make a new instance, start it, and queue it for release once it finishes
	FMOD::Studio::EventInstance* instance = nullptr;
	eventDescriptions.at(eventName)->createInstance(&instance);

	//FMOD::Studio::EventDescription* desc = nullptr;
	//instance->getDescription(&desc);
	//char path[128];
	//int retrieved;
	//desc->getPath(path, 128, &retrieved);
	//Utils::printMsg(std::format("path: {}", path));

	FMOD_3D_ATTRIBUTES attrib;
	attrib.forward = FMOD_VECTOR{ 0.0f, 0.0f, -1.0f };
	attrib.up = FMOD_VECTOR{ 0.0f, -1.0f, 0.0f };
	attrib.position = FMOD_VECTOR{ pos.x, pos.y, 0 };
	instance->set3DAttributes(&attrib);

	FMOD_RESULT res = instance->start();
	instance->release();
}

void FMODManager::setListenerPosition(sf::Vector2f pos)
{
	FMOD_3D_ATTRIBUTES attrib;
	attrib.forward = FMOD_VECTOR{ 0.0f, 0.0f, -1.0f };
	attrib.up = FMOD_VECTOR{ 0.0f, -1.0f, 0.0f };
	attrib.position = FMOD_VECTOR{ pos.x, pos.y, 0 };
	fSystem->setListenerAttributes(0, &attrib);
}

FMOD_RESULT FMODManager::loadBank(std::string bankPath, FMOD::Studio::Bank** bank)
{
	return fSystem->loadBankFile(bankPath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, bank);
}

void FMODManager::updateSystem()
{
	fSystem->update();
}
