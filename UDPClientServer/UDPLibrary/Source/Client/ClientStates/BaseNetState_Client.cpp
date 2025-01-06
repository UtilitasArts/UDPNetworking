#include "SessionStateEnums.h"
#include "NetClientStates.h"
#include "NetClientStateMachine.h"

//BaseState
BaseNetState_Client::BaseNetState_Client(NetClientStateMachine* sm) :
StateMachine(sm),
bActive(false)
{}
void BaseNetState_Client::InitState(){
	StateEnum = ENetClientStates::Unconnected;
}
void BaseNetState_Client::OnEnter() {
	InitState();
	std::cout << "- Entering State = " << ENetClientStateString(StateEnum) << "\n";
	bActive = true;
}

void BaseNetState_Client::OnTick() {
	if (bActive)	{
 		while (UDPSetup::SocketHasNewBytes()) {			
 			UDPPacks::RecvBytes(true);
			OnRecv();

			if (!bActive){break;}
 		}

 		UDPPacks::SendEchoes(true);
	}
}

void BaseNetState_Client::OnRecv()
{

}

void BaseNetState_Client::OnExit(){
	bActive = false;
	std::cout << "Exit State " << ENetClientStateString(StateEnum) << "\n";
}

