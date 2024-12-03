#pragma once
#include "NetClientStates.h"
#include "NetClientStateMachine.h"

NetClientStateMachine::NetClientStateMachine() :
StateArray
(
	{
		new Unconnected_NetClientState(this),
		new ConnectedToServer_NetClientState(this),
		new ConnectedToPlayers_NetClientState(this)
	}
),
	SendBytePack(200, 10),
	RecvBytePack(200, 10),
	Server(80, 61, 175, 45, 7000, "Server", false)
{ }

NetClientStateMachine::~NetClientStateMachine() {
	for (size_t i = 0; i < StateArray.size(); i++) {
		delete StateArray[i];		
	}
}

void NetClientStateMachine::SetState(ENetClientStates NextState) {
	if (CurrentState) {	CurrentState->OnExit();	}

	CurrentState = StateArray.at((size_t)NextState);

	if (CurrentState) {	CurrentState->OnEnter();}
}

