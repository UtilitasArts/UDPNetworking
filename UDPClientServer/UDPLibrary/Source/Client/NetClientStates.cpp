#pragma once
#include <filesystem>
#include "NetClientStateMachine.h"
#include "NetClientStates.h"
#include "SessionStateEnums.h"

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
	uint32_t MyNetIp; uint16_t MyNetPort; bool bCnctApproved;uint8_t AmountOfSessions;

	UDPPacks::RecvBytePack.ReturnBytes(bCnctApproved, 1);

	if (bCnctApproved)
	{
		UDPPacks::RecvBytePack.ReturnBytes(MyNetIp, 2);
		UDPPacks::RecvBytePack.ReturnBytes(MyNetPort, 3);
		UDPPacks::RecvBytePack.ReturnBytes(AmountOfSessions, 4);

		std::cout << "- Connection Approved \n";
		UDPPacks::PublicAdress.SetAdress(MyNetIp, MyNetPort, UDPSetup::MyName, true);

		if (AmountOfSessions > 0) {
			std::cout << "\n- Session Count = " << (int)AmountOfSessions << "\n";
			for (size_t i = 0; i < AmountOfSessions; i++) {

				std::string    SessionName;
				uint8_t		   SessionSize;
				uint8_t		   JoinedCount;
				ESessionStates SessionState;

				size_t count = i * 4;
				UDPPacks::RecvBytePack.ReturnBytes(SessionName,  5 + count);
				UDPPacks::RecvBytePack.ReturnBytes(SessionSize,  6 + count);
				UDPPacks::RecvBytePack.ReturnBytes(JoinedCount,  7 + count);
				UDPPacks::RecvBytePack.ReturnBytes(SessionState, 8 + count);

				printf("|Room#%02zd[ID:%-10s][%d/%d][State:%s]\n", i, SessionName.c_str(), JoinedCount, SessionSize,ESessionStateString(SessionState).c_str());
			}
			std::cout << "\n- Type -J to Join a session \n";
		}	std::cout << "- Type -C to Create a session \n";

		SendCnctApprovalResp(AmountOfSessions);
	}
	else
	{
		StateMachine->SetState(ENetClientStates::Unconnected);
	}
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
		if (SendReqJoinSession(Response, JoinPattern, AmountOfSessions))	{
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

		std::string RestartPath  = CMD::SetPath(UDPSetup::RestartFolder);
		std::string BatchFile    = CMD::Command("Push.bat ", CMD::SetString(CommitLog));
		std::string FinalCommand = CMD::MultiCMD(RestartPath, BatchFile);

		system(FinalCommand.c_str());

		UDPPacks::SendBytePack.Clear(20, 3);
		UDPPacks::SendBytePack.AddBytes(MessageType::UpdateRequest);
		UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
		
		return true;
	}	
	return false;	
}

bool Unconnected_NetClientState::SendReqCreateSession(std::string Response, std::regex& Pattern) {
	if (std::regex_search(Response, Pattern)) {

		std::cout << "- Request to create session: \n";
		std::string SessionID;
		std::cout << "- Please enter session ID with a maximum amount of 10 characters : \n";
		getline(std::cin, SessionID);

		std::regex RoomID{ R"(^[a-zA-Z\d]{1,10})" };std::smatch IDMatch;
		if (std::regex_search(SessionID, IDMatch, RoomID)) {

			std::string SessionIDString = IDMatch.str();
			std::string AmountOfPlayers;
			std::cout << "- Please enter the amount of players 2-4 : \n";
			getline(std::cin, AmountOfPlayers);

			std::regex RoomNumber{ R"(^[2-4]$)" };	std::smatch Match;
			if (std::regex_search(AmountOfPlayers, Match, RoomNumber)) {
				uint8_t AOP = std::stoi(Match.str());
				std::cout << "- Request to create Room with ID:" << SessionID << " That has space for " << (int)AOP << " Players.\n";
				UDPPacks::SendBytePack.Clear(20, 3);
				UDPPacks::SendBytePack.AddBytes(MessageType::CreateRequest);
				UDPPacks::SendBytePack.AddBytes(UDPSetup::MyName);
				UDPPacks::SendBytePack.AddBytes(SessionIDString);
				UDPPacks::SendBytePack.AddBytes(AOP);
				UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
				return true;
			}		
		}
	}
	return false;
}

bool Unconnected_NetClientState::SendReqJoinSession(std::string Response, std::regex& Pattern, uint8_t& AmountOfSessions) {
	if (std::regex_search(Response, Pattern) && (AmountOfSessions > 0)) {

		std::cout << "- Request to join session: \n";
		std::string RoomNr;
		std::cout << "- Please enter room number : \n";
		getline(std::cin, RoomNr);
		
		std::regex RoomNumber{ R"(^[1-10]$)" };	std::smatch Match;
		if (std::regex_search(RoomNr, Match, RoomNumber)) {
			uint8_t RoomNr = min(std::stoi(Match.str()), AmountOfSessions);
			std::cout << "- Request to join Room with number:" << RoomNr;
			UDPPacks::SendBytePack.Clear(20,3);
			UDPPacks::SendBytePack.AddBytes(MessageType::JoinRequest);
			UDPPacks::SendBytePack.AddBytes(UDPSetup::MyName);
			UDPPacks::SendBytePack.AddBytes(RoomNr);
			UDPPacks::SendBytes(UDPPacks::ServerAdress, true);

			return true;
		}
	}
	return false;
}

void Unconnected_NetClientState::RecvJoinSessionApproval() {
	bool bApproved;
	UDPPacks::RecvBytePack.ReturnBytes(bApproved, 1);
	if (bApproved){
		std::cout << "- Joining of room was approved\n";
		StateMachine->SetState(ENetClientStates::ConnectedToSession);
	}
	else {
		std::cout << "- Joining of room was denied, Room was probably full";
		StateMachine->SetState(ENetClientStates::Unconnected);
	}	
}
void Unconnected_NetClientState::RecvCreateSessionApproval() {
	bool bApproved;
	UDPPacks::RecvBytePack.ReturnBytes(bApproved,1);
	if (bApproved){
		std::cout << "- Creation of room was approved\n";
	}
	else{
		std::cout << "- Creation of room was denied, Too many rooms where probably created";
		StateMachine->SetState(ENetClientStates::Unconnected);
	}
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



//ConnectedToSession
void ConnectedToSession_NetClientState::InitState(){
	BaseNetClientState::InitState();
	StateEnum = ENetClientStates::ConnectedToSession;
}
void ConnectedToSession_NetClientState::OnEnter() {
	BaseNetClientState::OnEnter();

	std::cout << "- Waiting For Players..";


}
void ConnectedToSession_NetClientState::OnExit() {
	BaseNetClientState::OnExit();
}



//ConnectedToPlayers
void ConnectedToPlayers_NetClientState::InitState(){
	BaseNetClientState::InitState();
	StateEnum = ENetClientStates::ConnectedToSession;
}
void ConnectedToPlayers_NetClientState::OnEnter(){
	BaseNetClientState::OnEnter();
}
void ConnectedToPlayers_NetClientState::OnExit(){
	BaseNetClientState::OnExit();
}
