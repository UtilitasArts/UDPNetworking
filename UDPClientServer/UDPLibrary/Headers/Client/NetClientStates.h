#pragma once
#include <iostream>
#include <regex>
#include "NetClientStateEnums.h"
#include "UDPLibrary.h"

//--------------------|
//Forward Declaration |
//====================|
class NetClientStateMachine;

//-----------|
//Base Class |
//===========|
class BaseNetState_Client {
public:
	BaseNetState_Client(NetClientStateMachine* sm);
	virtual void InitState();
	virtual void OnEnter();
	virtual void OnTick();
	virtual void OnRecv();
	virtual void OnExit();

protected:
	bool bActive; 
	NetClientStateMachine* StateMachine;
	ENetClientStates StateEnum;
};
//-------------------|
// Unconnected_class |
//===================|
class Unconnected_Client : public BaseNetState_Client {
public:
	using BaseNetState_Client::BaseNetState_Client;
	virtual void InitState() override;
	virtual void OnEnter()   override;
	virtual void OnRecv() override;

// 	void ReceiveConnectionApproval();
 	void RecvJoinSessionApproval();
 	void RecvCreateSessionApproval();
 	void RecvUpdateApproval();
	void RecvCnctApproval();

	void SendConnectionRequest();
 	void SendCnctApprovalResp(uint8_t& AmountOfSessions);
 	bool SendReqUpdate(std::string Response, std::regex& Pattern);
 	bool SendReqCreateSession(std::string Response, std::regex& Pattern);
 	bool SendReqJoinSession(std::string Response, std::regex& Pattern, uint8_t& AmountOfSessions);
};
//----------------------------|
// Connected To Session class |
//============================|
class ConnectedToSession_NetClientState : public BaseNetState_Client {
public:
	using BaseNetState_Client::BaseNetState_Client;
	virtual void InitState() override;
	virtual void OnEnter()   override;
	virtual void OnRecv() override;

	void ReturnAddresses(MessageType message_type);
	void SessionStart();
	void RecvPlayerConnectRequest();

	bool bSessionStarted;
};
//----------------------------|
// Connected To Players class |
//============================|
class ConnectedToPlayers_NetClientState : public BaseNetState_Client {
public:
	using BaseNetState_Client::BaseNetState_Client;
	virtual void InitState() override;
	virtual void OnEnter()   override;
	virtual void OnRecv() override;
};

