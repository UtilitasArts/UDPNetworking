#pragma once
#include "NetClientStateMachine.h"
#include "NetClientStates.h"

//BaseState
BaseNetClientState::BaseNetClientState(NetClientStateMachine* sm) :
StateMachine(sm)
{}
void BaseNetClientState::InitState(){

	StateEnum = ENetClientStates::Unconnected;
}
void BaseNetClientState::OnEnter() {
	InitState();
	std::cout << "State=" << ENetClientStateString(StateEnum) << "\n";
}
void BaseNetClientState::OnExit(){
	//std::cout << "Exit State " << ENetClientStateString(StateEnum) << "\n";
}


//Unconnected
void Unconnected_NetClientState::InitState(){
	BaseNetClientState::InitState();
	StateEnum = ENetClientStates::Unconnected;
}
void Unconnected_NetClientState::OnEnter() {
	BaseNetClientState::OnEnter();
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
			if (ReceiveAdress.HostIP() == StateMachine->Server.HostIP()) {

				std::cout << " - Received message from server\n" << std::endl;

				// Fill Receive Byte Container here and append name to ReceiverAdress
				// Check if we are allowed or not to the server.
				break; 
			}       		
       	} 
     }

	StateMachine->SetState(ENetClientStates::ConnectedToServer);
}
void Unconnected_NetClientState::OnExit(){
	BaseNetClientState::OnExit();
}


//ConnectedToServer
void ConnectedToServer_NetClientState::InitState(){
	BaseNetClientState::InitState();
	StateEnum = ENetClientStates::ConnectedToServer;
}
void ConnectedToServer_NetClientState::OnEnter() {
	BaseNetClientState::OnEnter();
	StateMachine->SetState(ENetClientStates::ConnectedToPlayers);
}
void ConnectedToServer_NetClientState::OnExit() {
	BaseNetClientState::OnExit();
}



//ConnectedToPlayers
void ConnectedToPlayers_NetClientState::InitState(){
	BaseNetClientState::InitState();
	StateEnum = ENetClientStates::ConnectedToPlayers;
}
void ConnectedToPlayers_NetClientState::OnEnter(){
	BaseNetClientState::OnEnter();
}
void ConnectedToPlayers_NetClientState::OnExit(){
	BaseNetClientState::OnExit();
}
