#include "game.h"

#include <algorithm>

Game::Game(unsigned int id, std::vector<Question> questions, std::vector<LoggedUser> players)
	: m_id(id), m_questions(questions)
{
	GameData gameData = { m_questions[0], 0, 0, 0, false };

	// For each player set the default game data
	for (LoggedUser& player : players)
	{
		m_players[player] = gameData;
	}
}

Question Game::getQuestionForUser(LoggedUser & user)
{
	return m_players[user].currentQuestion;
}

void Game::submitAnswer(LoggedUser & player, unsigned int answerId, unsigned int timeToAnswer)
{
	GameData gameData = m_players[player];
	unsigned int amountOfAnswers = gameData.correctAnswerCount + gameData.wrongAnswerCount + 1;

	unsigned int currentQuestionIdx = find(m_questions.begin(), m_questions.end(), gameData.currentQuestion) - m_questions.begin();

	// Check if the user was correct
	if (answerId == gameData.currentQuestion.getCorrectAnswer()) {
		gameData.correctAnswerCount++;
	}
	else {
		gameData.wrongAnswerCount++;
	}

	// Re-calculate the average answer time
	gameData.averageAnswerTime = (gameData.averageAnswerTime * (amountOfAnswers - 1) + timeToAnswer) / amountOfAnswers;

	// Set the next question if not reached the end
	gameData.currentQuestion = (currentQuestionIdx == m_questions.size() - 1) ? Question() : m_questions[currentQuestionIdx + 1];
}

void Game::removePlayer(LoggedUser & player)
{
	m_players[player].playerLeft = true;
}

unsigned int Game::getGameId()
{
	return m_id;
}
 