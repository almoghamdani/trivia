#pragma once
#include <vector>
#include <memory>
#include <mutex>

#include "database_interface.h"
#include "game.h"
#include "room.h"
#include "question.h"
#include "logged_user.h"
#include "player_results.h"

class GameManager
{
public:
	GameManager(std::shared_ptr<IDatabase> database);

	unsigned int createGame(RoomData& room, std::vector<LoggedUser> players);
	bool deleteGame(unsigned int gameId);

	Question getQuestionForUser(unsigned int gameId, const LoggedUser& user);
	void submitAnswer(unsigned int gameId, const LoggedUser& player, unsigned int answerId, unsigned int timeToAnswer);
	void removePlayer(unsigned int gameId, const LoggedUser& player);
	std::vector<PlayerResults> getPlayersResults(unsigned int gameId);

private:
	std::shared_ptr<IDatabase> m_database;
	std::vector<Game> m_games;
	std::mutex gamesMutex;

	std::vector<Question> createQuestions(unsigned int amount);
	std::string decodeURLEncodedString(std::string encoded);

	std::vector<Game>::iterator findGame(unsigned int gameId);
};

