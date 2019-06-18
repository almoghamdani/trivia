#include "room_admin_request_handler.h"

#include "json_response_packet_serializer.hpp"

#include "close_room_response.h"
#include "get_room_state_response.h"
#include "response_status.h"

#include "request_handler_factory.h"

RoomAdminRequestHandler::RoomAdminRequestHandler(const Room & room, const LoggedUser & user, std::shared_ptr<RoomManager> roomManager, std::shared_ptr<RequestHandlerFactory> handlerFactory)
	: m_room(room), m_user(user), m_roomManager(roomManager), m_handlerFactory(handlerFactory)
{
}

bool RoomAdminRequestHandler::isRequestRelevant(const Request & req) const
{
	return req.id == CLOSE_ROOM_REQUEST || req.id == START_GAME_REQUEST || req.id == GET_ROOM_STATE_REQUEST;
}

RequestResult RoomAdminRequestHandler::handleRequest(const Request & req) const
{
	RequestResult res;

	switch (req.id)
	{
	case CLOSE_ROOM_REQUEST:
		res = closeRoom(req);
		break;

	case GET_ROOM_STATE_REQUEST:
		res = getRoomState(req);
		break;
	}

	return res;
}

void RoomAdminRequestHandler::disconnect() const
{
	try {
		// Try to delete the room
		m_roomManager->deleteRoom(m_room.getMetadata().id);
	}
	catch (...) {}
}

RequestResult RoomAdminRequestHandler::closeRoom(const Request & req) const
{
	RequestResult res;

	CloseRoomResponse closeRoomResponse;

	try {
		// Try to delete the room
		m_roomManager->deleteRoom(m_room.getMetadata().id);
		closeRoomResponse.status = SUCCESS;
	}
	catch (...) {
		closeRoomResponse.status = ERROR;
	}

	// Set the new handler as the menu request handler and serialize the response
	res.newHandler = m_handlerFactory->createMenuRequestHandler(m_user);
	res.response = JsonResponsePacketSerializer::SerializePacket(closeRoomResponse);

	return res;
}

RequestResult RoomAdminRequestHandler::getRoomState(const Request & req) const
{
	RequestResult res;

	GetRoomStateResponse getRoomStateResponse;

	// Set the details of the room
	getRoomStateResponse.answerTimeout = m_room.getMetadata().timePerQuestion;
	getRoomStateResponse.questionCount = m_room.getMetadata().questionCount;

	try {
		// Get the players of the room
		getRoomStateResponse.players = m_roomManager->getPlayersInRoom(m_room.getMetadata().id);

		// Check if the game in the room has begun
		getRoomStateResponse.hasGameBegun = m_roomManager->getRoomState(m_room.getMetadata().id);
		getRoomStateResponse.status = SUCCESS;
	}
	catch (...) {
		getRoomStateResponse.status = ERROR;
	}

	// Set the new handler as the menu request handler and serialize the response
	res.newHandler = nullptr;
	res.response = JsonResponsePacketSerializer::SerializePacket(getRoomStateResponse);

	return res;
}
