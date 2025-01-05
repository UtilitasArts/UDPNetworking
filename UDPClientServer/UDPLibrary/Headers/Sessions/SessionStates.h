#pragma once
#include <iostream>
#include "SessionStateEnums.h"
#include "UDPLibrary.h"

//Forward Declaration
class SessionStateMachine;

class BaseSessionState {
public:

	BaseSessionState(SessionStateMachine* sm);
	virtual void InitState();
	virtual void OnEnter();
	virtual void OnExit();

protected:
	SessionStateMachine* StateMachine;
	ESessionStates StateEnum;
};


class Initialize_SessionState : public BaseSessionState {
public:
	using BaseSessionState::BaseSessionState;
	virtual void InitState() override;
	virtual void OnEnter() override;
	virtual void OnExit()  override;
};


class WaitingForPlayers_SessionState : public BaseSessionState {
public:
	using BaseSessionState::BaseSessionState;
	virtual void InitState() override;
	virtual void OnEnter() override;
	virtual void OnExit()  override;
};

class SessionInProgress_SessionState : public BaseSessionState {
public:
	using BaseSessionState::BaseSessionState;
	virtual void InitState() override;
	virtual void OnEnter() override;
	virtual void OnExit()  override;
};

class DestroyingSession_SessionState : public BaseSessionState {
public:
	using BaseSessionState::BaseSessionState;
	virtual void InitState() override;
	virtual void OnEnter() override;
	virtual void OnExit()  override;
};


