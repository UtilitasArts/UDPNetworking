#pragma once
#include <array>
#include <iostream>
#include "SessionStateEnums.h"
#include "UDPLibrary.h"

//Forward Declaration
class BaseSessionState;

class SessionStateMachine {

public:
	SessionStateMachine(std::string session_name, uint8_t session_size);
	~SessionStateMachine();

	void SetState(ESessionStates NextState);
	bool JoinSession();

	bool IsNewConnection();
	void SendSessionAdresses(MessageType message_type);

	std::vector<AddrCtr> SessionAdresses;
	std::string SessionName;
	uint8_t SessionSize;
	uint8_t JoinedCount;
	ESessionStates CurrentStateEnum;

private:
	BaseSessionState* CurrentState;
	std::array<BaseSessionState*, 3> StateArray;
};