#pragma once
#include <array>
#include <iostream>
#include "NetClientStateEnums.h"
#include "UDPLibrary.h"

//---------------------|
// Forward Declaration |
//=====================|
class BaseNetState_Client;

//---------------|
// State Machine |
//===============|
class NetClientStateMachine {

public:
	NetClientStateMachine();
	~NetClientStateMachine();

	void SetState(ENetClientStates NextState);
	void OnTick();

private:
	BaseNetState_Client* CurrentState;
	std::array<BaseNetState_Client*, 3> StateArray;
};