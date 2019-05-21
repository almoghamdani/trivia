#pragma once
#include <vector>

#include "logged_user.h"
#include "room_data.h"

class Room
{
public:
    Room(RoomData& metadata);

    std::vector<LoggedUser> getAllUsers();
    bool addUser(LoggedUser &user);
    bool removeUser(LoggedUser &user);

private:
    std::vector<LoggedUser> m_users;
    RoomData m_metadata;
};

