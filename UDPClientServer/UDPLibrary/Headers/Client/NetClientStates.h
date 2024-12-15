#pragma once
#include <iostream>
#include <regex>
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
	void ReceiveConnectionApproval();
	void SendConnectionRequest();
	void RecvJoinSessionApproval();
	void RecvCreateSessionApproval();
	void RecvUpdateApproval();
	void RecvCnctApproval();
	void SendCnctApprovalResp(uint8_t& AmountOfSessions);
	void SendReqJoinSession(std::smatch& Match, uint8_t& AmountOfSessions);
	void SendReqCreateSession(std::smatch& Match);
	void SendReqUpdate();
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

