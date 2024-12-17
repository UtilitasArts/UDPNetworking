#pragma once
#include <iostream>

enum class ESessionStates : uint8_t {
	InitializeSession,
	WaitingForPlayers,
	SessionInProgress,
	DestroyingSession,
};

inline std::string ESessionStateString(ESessionStates ENetStateEnum) {
	switch (ENetStateEnum)
	{
		case ESessionStates::InitializeSession:	return "InitializeSession";
		case ESessionStates::WaitingForPlayers:	return "WaitingForPlayers";
		case ESessionStates::SessionInProgress:	return "SessionInProgress";
		case ESessionStates::DestroyingSession:	return "DestroyingSession";
		default:								return "None";
	}
}

