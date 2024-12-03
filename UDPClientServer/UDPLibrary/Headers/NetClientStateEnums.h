#pragma once

enum class ENetClientStates : uint8_t {
	Unconnected,
	ConnectedToServer,
	ConnectedToPlayers,
};

inline std::string ENetClientStateString(ENetClientStates ENetStateEnum)
{
	switch (ENetStateEnum)
	{
	case ENetClientStates::Unconnected:
		return "Unconnected";
		break;
	case ENetClientStates::ConnectedToServer:
		return "ConnectedToServer";
		break;
	case ENetClientStates::ConnectedToPlayers:
		return "ConnectedToPlayers";
		break;
	default:
		return "None";
	}
}

