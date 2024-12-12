#pragma once
#include <array>
#include <iostream>
#include "SessionStateEnums.h"
#include "UDPLibrary.h"

//Forward Declaration
class BaseSessionState;

class SessionStateMachine {

public:
	SessionStateMachine(uint8_t session_size, std::string session_name);
	~SessionStateMachine();

	void SetState(ESessionStates NextState);

	AdressCtr* AdressArray;
	std::string SessionName;

private:
	BaseSessionState* CurrentState;
	std::array<BaseSessionState*, 3> StateArray;
};