#pragma once
#include <iostream>
#include "NetClientStateEnums.h"
#include "UDPLibrary.h"

//Forward Declaration
class NetClientStateMachine;

class BaseNetClientState {
public:

	BaseNetClientState(NetClientStateMachine* sm);
	virtual void InitState();
	virtual void OnEnter();
	virtual void OnExit();


protected:
	NetClientStateMachine* StateMachine;
	ENetClientStates StateEnum;
};

class Unconnected_NetClientState : public BaseNetClientState {
public:
	using BaseNetClientState::BaseNetClientState;
	virtual void InitState() override;
	virtual void OnEnter() override;
	virtual void OnExit()  override;
};


class ConnectedToServer_NetClientState : public BaseNetClientState {
public:
	using BaseNetClientState::BaseNetClientState;
	virtual void InitState() override;
	virtual void OnEnter() override;
	virtual void OnExit()  override;
};

class ConnectedToPlayers_NetClientState : public BaseNetClientState {
public:
	using BaseNetClientState::BaseNetClientState;
	virtual void InitState() override;
	virtual void OnEnter() override;
	virtual void OnExit()  override;
};

