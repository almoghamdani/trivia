#pragma once
#include "request_handler_interface.h"

#include "logged_user.h"
#include "room_manager.h"
#include "login_manager.h"
#include "highscore_table.h"

class RequestHandlerFactory;

class MenuRequestHandler :
    public IRequestHandler
{
public:
	MenuRequestHandler(const LoggedUser &user, std::shared_ptr<RoomManager> roomManager, std::shared_ptr<HighscoreTable> highscoreTable, std::shared_ptr<RequestHandlerFactory> handlerFactory);
    
    virtual bool isRequestRelevant(const Request& req) const;
    virtual RequestResult handleRequest(const Request& req) const;

	virtual void disconnect() const;

private:
    RequestResult logout(const Request& req) const;
	RequestResult getRooms(const Request& req) const;
	RequestResult getPlayersInRoom(const Request& req) const;
	RequestResult joinRoom(const Request& req) const;
	RequestResult createRoom(const Request& req) const;
	RequestResult getHighscores(const Request& req) const;

    LoggedUser m_user;
	std::shared_ptr<HighscoreTable> m_highscoreTable;
    std::shared_ptr<RoomManager> m_roomManager;
    std::shared_ptr<RequestHandlerFactory> m_handlerFactory;
};

