#pragma once

#include <unordered_map>
#include <memory>

#include "SDL.h"
#include "SDL_gamecontroller.h"

#include "util/KeyCode.h"
#include "v8datamodel/InputObject.h"
#include "v8datamodel/HapticService.h"

namespace RBX
{
	class DataModel;
	class UserInputService;

	class GamepadService;

	typedef std::unordered_map<RBX::KeyCode, std::shared_ptr<RBX::InputObject> > Gamepad;
}

struct HapticData
{
	int hapticEffectId;
	float currentLeftMotorValue;
	float currentRightMotorValue;
	SDL_Haptic* hapticDevice;
};

class SDLGameController
{
private:
	std::weak_ptr<RBX::DataModel> dataModel;
	std::unordered_map<int, std::pair<int,SDL_GameController*> > gamepadIdToGameController;
	std::unordered_map<int, HapticData> hapticsFromGamepadId;
	std::unordered_map<int, int> joystickIdToGamepadId;

	rbx::signals::scoped_connection renderSteppedConnection;
	rbx::signals::scoped_connection getSupportedGamepadKeyCodesConnection;

	rbx::signals::scoped_connection setEnabledVibrationMotorsConnection;
	rbx::signals::scoped_connection setVibrationMotorConnection;

	void initSDL();

	RBX::UserInputService* getUserInputService();
	RBX::HapticService* getHapticService();

	RBX::GamepadService* getGamepadService();
	RBX::Gamepad getRbxGamepadFromJoystickId(int joystickId);

	void setupControllerId(int joystickId, int gamepadId, SDL_GameController *pad);
	SDL_GameController* removeControllerMapping(int joystickId);

	int getGamepadIntForEnum(RBX::InputObject::UserInputType gamepadType);

	void findAvailableGamepadKeyCodesAndSet(RBX::InputObject::UserInputType gamepadType);
	std::shared_ptr<const RBX::Reflection::ValueArray> getAvailableGamepadKeyCodes(RBX::InputObject::UserInputType gamepadType);

	void bindToDataModel();

	// Haptic Functions
	void refreshHapticEffects();
	bool setupHapticsForDevice(int id);

	void setVibrationMotorsEnabled(RBX::InputObject::UserInputType gamepadType);
	void setVibrationMotor(RBX::InputObject::UserInputType gamepadType, RBX::HapticService::VibrationMotor vibrationMotor, shared_ptr<const RBX::Reflection::Tuple> args);

public:
	SDLGameController(std::shared_ptr<RBX::DataModel> newDM);
	~SDLGameController();

	void updateControllers();

	void onControllerAxis( const SDL_ControllerAxisEvent sdlEvent );
	void onControllerButton( const SDL_ControllerButtonEvent sdlEvent );
	void removeController(int joystickId);
	void addController(int gamepadId);
};
