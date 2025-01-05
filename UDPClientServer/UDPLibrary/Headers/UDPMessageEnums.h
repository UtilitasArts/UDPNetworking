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
	EchoRequest,
	EchoResponse,
};

inline std::string MessageTypeToString(MessageType& message_type) {
	switch (message_type) {
		case MessageType::ConnectRequest:	return "ConnectRequest";
		case MessageType::ConnectApproval:	return "ConnectApproval";
		case MessageType::CreateRequest:	return "CreateRequest";
		case MessageType::CreateApproval:	return "CreateApproval";
		case MessageType::JoinRequest:		return "JoinRequest";
		case MessageType::JoinApproval:		return "JoinApproval";
		case MessageType::JoinNotify:		return "JoinNotify";
		case MessageType::SessionStart:		return "SessionStart";
		case MessageType::UpdateRequest:	return "UpdateRequest";
		case MessageType::UpdateApproval:	return "UpdateApproval";
		case MessageType::EchoRequest:		return "EchoRequest";
		case MessageType::EchoResponse:		return "EchoResponse";
		default:							return "UnknownMessage";
	}
}

