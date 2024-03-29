#pragma once

#include "login_request_handler.h"
#include "menu_request_handler.h"
#include "room_admin_request_handler.h"
#include "room_member_request_handler.h"
#include "game_request_handler.h"

#include "database_interface.h"

#include "highscore_table.h"
#include "room_manager.h"
#include "login_manager.h"
#include "game_manager.h"

#include <memory>

class RequestHandlerFactory : public std::enable_shared_from_this<RequestHandlerFactory>
{
public:
    RequestHandlerFactory(std::shared_ptr<IDatabase> database);

    std::shared_ptr<LoginRequestHandler> createLoginRequestHandler();
    std::shared_ptr<MenuRequestHandler> createMenuRequestHandler(const LoggedUser& user);
	std::shared_ptr<RoomAdminRequestHandler> createRoomAdminRequestHandler(const LoggedUser& user, const int roomId);
	std::shared_ptr<RoomMemberRequestHandler> createRoomMemberRequestHandler(const LoggedUser& user, const int roomId);
	std::shared_ptr<GameRequestHandler> createGameRequestHandler(const LoggedUser& user, const unsigned int gameId);
    
    std::shared_ptr<LoginManager> getLoginManager();
	std::shared_ptr<GameManager> getGameManager();
	std::shared_ptr<RoomManager> getRoomManager();

    std::shared_ptr<RequestHandlerFactory> getPtr();
private:
	std::shared_ptr<HighscoreTable> m_highscoreTable;
    std::shared_ptr<LoginManager> m_loginManager;
    std::shared_ptr<RoomManager> m_roomManager;
	std::shared_ptr<GameManager> m_gameManager;
};

