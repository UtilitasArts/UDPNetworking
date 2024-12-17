#pragma once

enum class MessageType : uint8_t {
	None,
	ConnectRequest,
	ConnectApproval,
	CreateRequest,
	CreateApproval,
	JoinRequest,
	JoinApproval,
	JoinNotify,
	SessionStart,
	UpdateRequest,
	UpdateApproval,
	ProfilePackage,
};

inline std::string MessageTypeToString(MessageType& MesType) {
	switch (MesType) {
		case MessageType::ConnectRequest:	return "ConnectRequest";
		case MessageType::ConnectApproval:	return "ConnectApproval";
		case MessageType::CreateRequest:	return "CreateRequest";
		case MessageType::CreateApproval:	return "CreateApproval";
		case MessageType::JoinRequest:		return "ConnectRequest";
		case MessageType::JoinApproval:		return "JoinApproval";
		case MessageType::JoinNotify:		return "JoinNotify";
		case MessageType::SessionStart:		return "SessionStart";
		case MessageType::UpdateRequest:	return "UpdateRequest";
		case MessageType::UpdateApproval:	return "UpdateApproval";
		case MessageType::ProfilePackage:	return "ProfilePackage";
		default:							return "UnknownMessage";
	}
}

