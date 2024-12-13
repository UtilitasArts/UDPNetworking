#pragma once
#include <filesystem>
#include <iostream>
#include <regex>
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

// -----------------------|
// Initialize UDP Systems |
// =======================|
    UDPSetup::UDPInit();

// --------------------------|
// Send a Request to connect |
// ==========================|

 	UDPPacks::SendBytePack.Clear(20,3);
 	UDPPacks::SendBytePack.AddBytes(MessageType::ConnectRequest);
 	UDPPacks::SendBytePack.AddBytes(UDPSetup::MyName);
	UDPPacks::SendBytes(UDPPacks::ServerAdress, true);

 	while (true){
	// --------------------------------------|
	// This is where the messages comes from |
	// ======================================|
 		UDPPacks::RecvBytes(true);	

	// --------------------------------------------------------------------|
	// for now only accept messages from the server ignore everything else |
	// ====================================================================|
		if (UDPPacks::ReceiveAdress.HostIP() == UDPPacks::ServerAdress.HostIP()){

		// --------------------------|
		// Extract Approval Messages |
		// ==========================|
			if (UDPPacks::RecvMT == MessageType::ConnectApproval)
			{
				uint32_t MyNetIp;
				uint16_t MyNetPort;
				bool     bConnectionApproved;
				uint8_t  AmountOfSessions;

				UDPPacks::RecvBytePack.ReturnBytes(MyNetIp,1);
				UDPPacks::RecvBytePack.ReturnBytes(MyNetPort, 2);
				UDPPacks::RecvBytePack.ReturnBytes(bConnectionApproved, 3);
				UDPPacks::RecvBytePack.ReturnBytes(AmountOfSessions, 4);
		
				std::cout <<"- Connection Approved \n";
				UDPPacks::PublicAdress.SetAdress(MyNetIp,MyNetPort,UDPSetup::MyName, true);


				if (AmountOfSessions > 0){
					std::cout << "- Found Session Id's - \n";
					for (size_t i = 0; i < AmountOfSessions; i++)
					{
						std::string SessionName;
						UDPPacks::RecvBytePack.ReturnBytes(SessionName, 5 + i, true);
						std::cout << "- " << i << " " << SessionName;
					}
					std::cout << "- Join a session with -J [RoomNumber] \n";
				}	std::cout << "- Or Create a session with -C [SessionID] with a maximum amount of 6 characters \n";

			// -----------------------------------------------------|
			// If we got an approval message we send an answer back	|
			// We can Create or Join a room, Or we update server	|
			// =====================================================|
				while (true) {
					std::string Response;
					getline(std::cin, Response);
					std::regex JoinPattern  { R"(^-J)", std::regex::icase };
					std::regex CreatePattern{ R"(^-C)", std::regex::icase };
					std::regex UpdatePattern{ R"(^-U)", std::regex::icase };

				// ----------------|
				// Updating Server |
				//=================|
					if (std::regex_search(Response, UpdatePattern)) {							
						std::cout << "- Request to update server: \n";
						std::string CommitLog;
						std::cout << "- Please enter commit log: \n";
						getline(std::cin, CommitLog);

						fs::path Repos = fs::current_path().parent_path().parent_path();
						fs::path Location = fs::current_path().parent_path() / "x64" / "Release" / "UDPClient.exe";
						std::string Command = "cd \"" + Repos.string() + "\" && git add . && git commit -m \"" + CommitLog.c_str() + "\" && push -u origin main && \"" + Location.string() + "\"";

						UDPPacks::SendBytePack.Clear(20, 3);
						UDPPacks::SendBytePack.AddBytes(MessageType::UpdateRequest);
						UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
						system(Command.c_str()); exit(0);
						break;					
					}
				// ----------------|
				// Create Session  |
				// ================|
					if (std::regex_search(Response, CreatePattern))	{
						Response = std::regex_replace(Response, CreatePattern, "");
						std::regex RoomIDPattern{ R"([a-zA-Z0-9]{1,6})" };
						std::smatch Match;
						if (std::regex_search(Response, Match, RoomIDPattern))	{
							std::string RoomID = Match.str();
							std::cout << "- Request to create Room with ID:" << RoomID;
							UDPPacks::SendBytePack.Clear(20, 3);
							UDPPacks::SendBytePack.AddBytes(MessageType::CreateRequest);
							UDPPacks::SendBytePack.AddBytes(RoomID);
							UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
							break;
						}
					}
				// ----------------|
				// Join Session	   |
				// ================|
					if (std::regex_search(Response, JoinPattern) && (AmountOfSessions > 0))	{
						Response = std::regex_replace(Response, JoinPattern, "");
						std::regex RoomNumber{ R"(\d{1,2})" };
						std::smatch Match;
						if (std::regex_search(Response, Match, RoomNumber))	{
							uint8_t RoomNr = min(std::stoi(Match.str()), AmountOfSessions);
							std::cout << "- Request to join Room with number:" << RoomNr;
							UDPPacks::SendBytePack.AddBytes(MessageType::JoinRequest);
							UDPPacks::SendBytePack.AddBytes(RoomNr);
							UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
							break;
						}
					}



					std::cout << "Wrong Input \n";
				}
			}
			// -----------------------|
			// Creating Room Approved |
			// =======================|
			if (UDPPacks::RecvMT == MessageType::CreateApproval){
				std::cout << "- Creation of room was approved";
			}

			// ----------------------|
			// Joining Room Approved |
			// ======================|
			if (UDPPacks::RecvMT == MessageType::JoinApproval){

			}



		}
 	}	
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
