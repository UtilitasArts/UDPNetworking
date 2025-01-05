
#include <filesystem>
#include "CMDCommands.h"
#include "SessionStateEnums.h"
#include "NetClientStates.h"
#include "NetClientStateMachine.h"

namespace fs = std::filesystem;


//ConnectedToPlayers
void ConnectedToPlayers_NetClientState::InitState(){
	BaseNetState_Client::InitState();
	StateEnum = ENetClientStates::ConnectedToPlayers;
}
void ConnectedToPlayers_NetClientState::OnEnter(){
	BaseNetState_Client::OnEnter();
}
void ConnectedToPlayers_NetClientState::OnRecv()
{
}


