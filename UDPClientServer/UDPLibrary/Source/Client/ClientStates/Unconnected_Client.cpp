#include <filesystem>
#include "CMDCommands.h"
#include "SessionStateEnums.h"
#include "NetClientStates.h"
#include "NetClientStateMachine.h"

namespace fs = std::filesystem;

void Unconnected_Client::InitState(){
	BaseNetState_Client::InitState();
	StateEnum = ENetClientStates::Unconnected;
}
void Unconnected_Client::OnEnter() {
	BaseNetState_Client::OnEnter();

    UDPSetup::UDPInit();
	SendConnectionRequest();
}

void Unconnected_Client::OnRecv() {
	if (UDPPacks::ReceiveAdress == UDPPacks::ServerAdress) {
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

void Unconnected_Client::SendConnectionRequest() {
	UDPPacks::CreateEchoMessage(MessageType::ConnectRequest, MessageType::EchoRequest, UDPPacks::SendID,
							    UDPSetup::MyName,
								UDPSetup::LocalAddress.HostIP());
	UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
}

void Unconnected_Client::RecvCnctApproval() {
	uint32_t MyNetIp; uint16_t MyNetPort; bool bCnctApproved; uint8_t AmountOfSessions;
	UDPPacks::RecvBytePack.ReturnBytes(bCnctApproved, 3);

	if (bCnctApproved)
	{
		UDPPacks::RecvBytePack.ReturnBytes(MyNetIp, 4);
		UDPPacks::RecvBytePack.ReturnBytes(MyNetPort, 5);
		UDPPacks::RecvBytePack.ReturnBytes(AmountOfSessions, 6);

		std::cout << "- Connection Approved \n";
		UDPPacks::MyPublicAdress.SetAdress(MyNetIp, MyNetPort, UDPSetup::MyName, true);

		if (AmountOfSessions > 0) {
			std::cout << "\n- Session Count = " << (int)AmountOfSessions << "\n";
			for (size_t i = 0; i < AmountOfSessions; i++) {

				std::string    SessionName;
				uint8_t		   SessionSize;
				uint8_t		   JoinedCount;
				ESessionStates SessionState;

				size_t count = i * 4;
				UDPPacks::RecvBytePack.ReturnBytes(SessionName, 7  + count);
				UDPPacks::RecvBytePack.ReturnBytes(SessionSize, 8  + count);
				UDPPacks::RecvBytePack.ReturnBytes(JoinedCount, 9  + count);
				UDPPacks::RecvBytePack.ReturnBytes(SessionState,10 + count);

				printf("|Room#%02zd[ID:%-10s][%d/%d][State:%s]\n", i, SessionName.c_str(), JoinedCount, SessionSize, ESessionStateString(SessionState).c_str());
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
//
void Unconnected_Client::SendCnctApprovalResp(uint8_t& AmountOfSessions) {
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

bool Unconnected_Client::SendReqUpdate(std::string Response, std::regex& Pattern) {
	if (std::regex_search(Response, Pattern)) {

		std::cout << "- Request to update server: \n";
		std::string CommitLog;
		std::cout << "- Please enter commit log: \n";
		getline(std::cin, CommitLog);

		std::string RestartPath  = CMD::SetPath(UDPSetup::RestartFolder);
		std::string BatchFile    = CMD::Command("Push.bat ", CMD::SetString(CommitLog));
		std::string FinalCommand = CMD::MultiCMD(RestartPath, BatchFile);

		system(FinalCommand.c_str());

		UDPPacks::CreateEchoMessage(MessageType::UpdateRequest, MessageType::EchoRequest, UDPPacks::SendID);
		UDPPacks::SendBytes(UDPPacks::ServerAdress, true);

		
		return true;
	}	

	return false;	
}

 bool Unconnected_Client::SendReqCreateSession(std::string Response, std::regex& Pattern) {
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
 				

				UDPPacks::CreateEchoMessage(MessageType::CreateRequest, MessageType::EchoRequest, UDPPacks::SendID,
											UDPSetup::MyName,
											SessionIDString,
											AOP);

 				UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
 				return true;
 			}		
 		}
 	}
 	return false;
 }
 
 bool Unconnected_Client::SendReqJoinSession(std::string Response, std::regex& Pattern, uint8_t& AmountOfSessions) {
 	if (std::regex_search(Response, Pattern) && (AmountOfSessions > 0)) {
 		std::cout << "- Request to join session: \n";
 		std::string RoomNr;
 		std::cout << "- Please enter room number : \n";
 		getline(std::cin, RoomNr);
 		
 		std::regex RoomNumber{ R"([0-10])" };	std::smatch Match;
 		if (std::regex_search(RoomNr, Match, RoomNumber)) {
 			uint8_t RoomNr = min(std::stoi(Match.str()), AmountOfSessions);
 			std::cout << "- Request to join Room with number:" << (int)RoomNr << "\n";

			UDPPacks::CreateEchoMessage(MessageType::JoinRequest, MessageType::EchoRequest, UDPPacks::SendID,
										UDPSetup::MyName,
										RoomNr);

			UDPPacks::SendBytes(UDPPacks::ServerAdress, true); 
 			return true;
 		}
 	}
 	return false;
 }
 
 void Unconnected_Client::RecvJoinSessionApproval() {
 	bool bApproved;
 	UDPPacks::RecvBytePack.ReturnBytes(bApproved, 3);
 	if (bApproved){
 		std::cout << "- Joining of room was approved\n";
 		StateMachine->SetState(ENetClientStates::ConnectedToSession);
 	}
 	else {
 		std::cout << "- Joining of room was denied, Room was probably full";
 		StateMachine->SetState(ENetClientStates::Unconnected);
 	}	
 }
// 
 void Unconnected_Client::RecvCreateSessionApproval() {
 	bool bApproved;
 	UDPPacks::RecvBytePack.ReturnBytes(bApproved,3);
 	if (bApproved){
 		std::cout << "- Creation of room was approved\n";
 	}
 	else{
 		std::cout << "- Creation of room was denied, Too many rooms where probably created";
 		StateMachine->SetState(ENetClientStates::Unconnected);
 	}
 }
// 
void Unconnected_Client::RecvUpdateApproval() {
	std::cout << "- Update of server was approved, Restarting now";
	std::string RestartPath = CMD::SetPath(UDPSetup::RestartFolder);
	std::string BatchFile = CMD::Command("Restart.bat ", CMD::SetString("UDPClient.exe"));
	std::string FinalCommand = CMD::MultiCMD(RestartPath, BatchFile);
	system(FinalCommand.c_str());
	exit(0);
}
// 
// 
