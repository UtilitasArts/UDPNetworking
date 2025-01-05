#pragma once

enum class ENetClientStates : uint8_t {
	Unconnected,
	ConnectedToSession,
	ConnectedToPlayers,
};

inline std::string ENetClientStateString(ENetClientStates ENetStateEnum) {
	switch (ENetStateEnum)
	{
		case ENetClientStates::Unconnected:			return "Unconnected";
		case ENetClientStates::ConnectedToSession:	return "ConnectedToSession";
		case ENetClientStates::ConnectedToPlayers:	return "ConnectedToPlayers";
		default:									return "None";
	}
}


