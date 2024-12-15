#pragma once
#include <filesystem>
#include "NetClientStateMachine.h"
#include "NetClientStates.h"

namespace fs = std::filesystem;

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
    UDPSetup::UDPInit();
	SendConnectionRequest();
	ReceiveConnectionApproval();
}
void Unconnected_NetClientState::SendConnectionRequest() {
	UDPPacks::SendBytePack.Clear(20, 3);
	UDPPacks::SendBytePack.AddBytes(MessageType::ConnectRequest);
	UDPPacks::SendBytePack.AddBytes(UDPSetup::MyName);
	UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
}

void Unconnected_NetClientState::ReceiveConnectionApproval() {
	while (true) {
		UDPPacks::RecvBytes(true);
		if (UDPPacks::ReceiveAdress.HostIP() == UDPPacks::ServerAdress.HostIP()) {
			switch (UDPPacks::RecvMT) {
			case MessageType::ConnectApproval:
				RecvCnctApproval();
				break;
			case MessageType::UpdateApproval:
				RecvUpdateApproval();
				break;
			case MessageType::CreateApproval:
				RecvCreateSessionApproval();
				break;
			case MessageType::JoinApproval:
				RecvJoinSessionApproval();
				break;
			}
		}
	}
}

void Unconnected_NetClientState::RecvCnctApproval(){
	uint32_t MyNetIp; uint16_t MyNetPort; bool bConnectionApproved;uint8_t AmountOfSessions;

	UDPPacks::RecvBytePack.ReturnBytes(MyNetIp, 1);
	UDPPacks::RecvBytePack.ReturnBytes(MyNetPort, 2);
	UDPPacks::RecvBytePack.ReturnBytes(bConnectionApproved, 3);
	UDPPacks::RecvBytePack.ReturnBytes(AmountOfSessions, 4);

	std::cout << "- Connection Approved \n";
	UDPPacks::PublicAdress.SetAdress(MyNetIp, MyNetPort, UDPSetup::MyName, true);

	if (AmountOfSessions > 0) {
		std::cout << "- Found Session Id's - \n";
		for (size_t i = 0; i < AmountOfSessions; i++)
		{
			std::string SessionName;
			UDPPacks::RecvBytePack.ReturnBytes(SessionName, 5 + i, true);
			std::cout << "- " << i << " " << SessionName;
		}
		std::cout << "- Join a session with -J [RoomNumber] \n";
	}	std::cout << "- Create a session with -C [SessionID] with a maximum amount of 6 characters \n";

	SendCnctApprovalResp(AmountOfSessions);
}

void Unconnected_NetClientState::SendCnctApprovalResp(uint8_t& AmountOfSessions) {
	while (true) {

		std::string Response; getline(std::cin, Response);
		std::regex JoinPattern{   R"(^-J)", std::regex::icase };
		std::regex CreatePattern{ R"(^-C)", std::regex::icase };
		std::regex UpdatePattern{ R"(^-U)", std::regex::icase };

	
		if (SendReqUpdate(Response, UpdatePattern)) {
			break;
		}

		if (SendReqCreateSession(Response, CreatePattern)) {
			break;
		}
	
		if (SendReqJoinSession(Response, CreatePattern, AmountOfSessions))	{
			break;
		}

		std::cout << "Wrong Input \n";
	}
}

bool Unconnected_NetClientState::SendReqUpdate(std::string Response, std::regex& Pattern) {
	if (std::regex_search(Response, Pattern)) {

		std::cout << "- Request to update server: \n";
		std::string CommitLog;
		std::cout << "- Please enter commit log: \n";
		getline(std::cin, CommitLog);

		std::string RestartPath = CMD::SetPath(UDPSetup::RestartFolder);
		std::string BatchFile   = CMD::Command("Push.bat ", CMD::SetString(CommitLog));
		std::string FinalCommand = CMD::MultiCMD(RestartPath, BatchFile);

		system(FinalCommand.c_str());

		UDPPacks::SendBytePack.Clear(20, 3);
		UDPPacks::SendBytePack.AddBytes(MessageType::UpdateRequest);
		UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
		
		return true;
	}	
	return false;	
}

bool Unconnected_NetClientState::SendReqCreateSession(std::string& Response, std::regex& Pattern) {
	if (std::regex_search(Response, Pattern)) {
		std::cout << "- Request to create session: \n";
		Response = std::regex_replace(Response, Pattern, "");
		std::regex RoomIDPattern{ R"([a-zA-Z0-9]{1,6})" }; std::smatch Match;
		if (std::regex_search(Response, Match, RoomIDPattern)) {
			
			std::string RoomID = Match.str();
			std::cout << "- Request to create Room with ID:" << RoomID;
			UDPPacks::SendBytePack.Clear(20, 3);
			UDPPacks::SendBytePack.AddBytes(MessageType::CreateRequest);
			UDPPacks::SendBytePack.AddBytes(RoomID);
			UDPPacks::SendBytePack.AddBytes(UDPSetup::MyName);
			UDPPacks::SendBytes(UDPPacks::ServerAdress, true);

			return true;
		}
	}
	return false;
}

bool Unconnected_NetClientState::SendReqJoinSession(std::string Response, std::regex& Pattern, uint8_t& AmountOfSessions) {
	if (std::regex_search(Response, Pattern) && (AmountOfSessions > 0)) {

		Response = std::regex_replace(Response, Pattern, "");
		std::regex RoomNumber{ R"(\d{1,2})" };	std::smatch Match;

		if (std::regex_search(Response, Match, RoomNumber)) {

			uint8_t RoomNr = min(std::stoi(Match.str()), AmountOfSessions);
			std::cout << "- Request to join Room with number:" << RoomNr;

			UDPPacks::SendBytePack.AddBytes(MessageType::JoinRequest);
			UDPPacks::SendBytePack.AddBytes(RoomNr);
			UDPPacks::SendBytes(UDPPacks::ServerAdress, true);

			return true;
		}
	}
	return false;
}




void Unconnected_NetClientState::RecvJoinSessionApproval() {
	std::cout << "- Joining of room was approved";
}

void Unconnected_NetClientState::RecvCreateSessionApproval() {
	std::cout << "- Creation of room was approved";
}

void Unconnected_NetClientState::RecvUpdateApproval() {
	std::cout << "- Update of server was approved, Restarting now";

	std::string RestartPath = CMD::SetPath(UDPSetup::RestartFolder);
	std::string BatchFile = CMD::Command("Restart.bat ", CMD::SetString("UDPClient.exe"));
	std::string FinalCommand = CMD::MultiCMD(RestartPath, BatchFile);
	system(FinalCommand.c_str());
	exit(0);
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
