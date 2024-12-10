#pragma once
#include <array>
#include <iostream>
#include "SessionStateEnums.h"
#include "UDPLibrary.h"

//Forward Declaration
class BaseSessionState;

class SessionStateMachine {

public:
	SessionStateMachine(uint8_t session_size);
	~SessionStateMachine();

	void SetState(ESessionStates NextState);

	BytePack  SendBytePack;
	BytePack  RecvBytePack;
	AdressCtr Server;

	AdressCtr* AdressArray;

private:
	BaseSessionState* CurrentState;
	std::array<BaseSessionState*, 3> StateArray;
};