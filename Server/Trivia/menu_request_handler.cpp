#include "menu_request_handler.h"

#include "logout_response.h"
#include "get_rooms_response.h"
#include "get_players_in_room_request.h"
#include "get_players_in_room_response.h"

#include "response_status.h"
#include "json_request_packet_deserializer.hpp"
#include "json_response_packet_serializer.hpp"
#include "request_handler_factory.h"

MenuRequestHandler::MenuRequestHandler(LoggedUser& user, std::shared_ptr<RoomManager> roomManager, std::shared_ptr<RequestHandlerFactory> handlerFactory)
    : m_user(user), m_roomManager(roomManager), m_handlerFactory(handlerFactory)
{
}

bool MenuRequestHandler::isRequestRelevant(const Request & req) const
{
    return req.id == LOGOUT_REQUEST || req.id == GET_ROOMS_REQUEST || req.id == GET_PLAYERS_IN_ROOM_REQUEST || req.id == JOIN_ROOM_REQUEST || req.id == CREATE_ROOM_REQUEST;
}

RequestResult MenuRequestHandler::handleRequest(const Request & req) const
{
    RequestResult res;

    switch (req.id)
    {
    case LOGOUT_REQUEST:
		res = logout(req);

	case GET_ROOMS_REQUEST:
		res = getRooms(req);
    }

    return res;
}

RequestResult MenuRequestHandler::logout(const Request & req) const
{
    LogoutResponse logoutRes;
    RequestResult res;

    // Try to logout the user
    if (m_handlerFactory->getLoginManager()->logout(m_user))
    {
		logoutRes.status = SUCCESS;
    }
    else {
		logoutRes.status = ERROR;
    }

    // Serialize the new packet and set the next handler
    res.newHandler = nullptr;
    res.response = JsonResponsePacketSerializer::SerializePacket(logoutRes);

    return res;
}

RequestResult MenuRequestHandler::getRooms(const Request & req) const
{
	GetRoomsResponse roomsRes;
	RequestResult res;

	// Get the available rooms
	roomsRes.rooms = m_roomManager->getRooms();
	roomsRes.status = SUCCESS;

	// Serialize the new packet and set the next handler
	res.newHandler = nullptr;
	res.response = JsonResponsePacketSerializer::SerializePacket(roomsRes);

	return res;
}

RequestResult MenuRequestHandler::getPlayersInRoom(const Request & req) const
{
	GetPlayersInRoomRequest playersReq = JsonRequestPacketDeserializer::DeserializePacket<GetPlayersInRoomRequest>(req.buffer);
	GetPlayersInRoomResponse playersRes;

	RequestResult res;

	try {
		// Get the players in room
		playersRes.players = m_roomManager->getPlayersInRoom(playersReq.roomId);
		playersRes.status = SUCCESS;
	}
	catch (...) {
		playersRes.status = ERROR;
	}

	// Serialize the new packet and set the next handler
	res.newHandler = nullptr;
	res.response = JsonResponsePacketSerializer::SerializePacket(playersRes);

	return res;
}


