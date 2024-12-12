#pragma once

enum class ESessionStates : uint8_t {
	InitializeSession,
	WaitingForPlayers,
	SessionInProgress,
	DestroyingSession,
};

inline std::string ESessionStateString(ESessionStates ENetStateEnum)
{
	switch (ENetStateEnum)
	{
	case ESessionStates::InitializeSession:
		return "InitializeSession";
		break;
	case ESessionStates::WaitingForPlayers:
		return "WaitingForPlayers";
		break;
	case ESessionStates::SessionInProgress:
		return "SessionInProgress";
		break;
	case ESessionStates::DestroyingSession:
		return "DestroyingSession";
		break;
	default:
		return "None";
	}
}

