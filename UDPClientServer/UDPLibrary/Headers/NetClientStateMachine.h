#pragma once
#include <array>
#include <iostream>
#include "NetClientStateEnums.h"
#include "UDPLibrary.h"


//Forward Declaration
class BaseNetClientState;

class NetClientStateMachine {

public:
	NetClientStateMachine();
	~NetClientStateMachine();

	void SetState(ENetClientStates NextState);

	BytePack  SendBytePack;
	BytePack  RecvBytePack;
	AdressCtr Server;

	AdressCtr AdressArray[3];

private:
	BaseNetClientState* CurrentState;
	std::array<BaseNetClientState*, 3> StateArray;
};