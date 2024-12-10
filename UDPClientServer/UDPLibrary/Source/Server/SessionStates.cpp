#pragma once
#include "SessionStateMachine.h"
#include "SessionStates.h"

//BaseState
BaseSessionState::BaseSessionState(SessionStateMachine* sm) :
StateMachine(sm)
{}
void BaseSessionState::InitState(){
	StateMachine->SendBytePack.Clear(200,20);
	StateMachine->RecvBytePack.Clear(200,20);
	StateEnum = ESessionStates::InitializeSession;
}
void BaseSessionState::OnEnter() {
	InitState();
	std::cout << "State=" << ESessionStateString(StateEnum) << "\n";
}
void BaseSessionState::OnExit(){
	//std::cout << "Exit State " << ENetServerStateString(StateEnum) << "\n";
}


//Initialize_Session
void Initialize_SessionState::InitState(){
	BaseSessionState::InitState();
	StateEnum = ESessionStates::InitializeSession;
}
void Initialize_SessionState::OnEnter() {
	BaseSessionState::OnEnter();
    UDPSetup::InitConnect();
	StateMachine->SendBytePack.AddBytes(MessageType::ConnectRequest);
	StateMachine->SendBytePack.AddBytes(UDPSetup::MyName);
	StateMachine->Server.SendPack(StateMachine->SendBytePack, true);

	AdressCtr ReceiveAdress;
	unsigned char buffer[1024];
	std::cout << "\n - Waiting for server...\n";

     while (true) {  

       	int bytesReceived = recvfrom(UDPSetup::UDPSocket, (char*)buffer, sizeof(buffer), 0, ReceiveAdress.GetSockAddr(), ReceiveAdress.GetAddrSize()); // halts the loop because the socket is prolly set to blocking
       	if (bytesReceived != SOCKET_ERROR) {       	

			ReceiveAdress.FillFromSockAddr();
			if (ReceiveAdress.HostIP() == StateMachine->Server.HostIP())
			{
				std::cout << " - Received message from server\n" << std::endl;

				// Fill Receive Byte Container here and append name to ReceiverAdress
				// Check if we are allowed or not to the server.
				break; 
			}       		
       	} 
     }

}
void Initialize_SessionState::OnExit(){
	BaseSessionState::OnExit();
}


//SessionInProgress
void SessionInProgress_SessionState::InitState(){
	BaseSessionState::InitState();
	StateEnum = ESessionStates::SessionInProgress;
}
void SessionInProgress_SessionState::OnEnter(){
	BaseSessionState::OnEnter();
}
void SessionInProgress_SessionState::OnExit(){
	BaseSessionState::OnExit();
}


//SessionInProgress
void DestroyingSession_SessionState::InitState(){
	BaseSessionState::InitState();
	StateEnum = ESessionStates::DestroyingSession;
}

void DestroyingSession_SessionState::OnEnter()
{
}

void DestroyingSession_SessionState::OnExit()
{
}
