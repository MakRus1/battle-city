#pragma once

#include <glm/vec2.hpp>
#include <array>

class IGameState;

namespace RenderEngine {
	class ShaderProgram;
}

class Game {
public:
	enum class EGameMode {
		OnePlayer,
		TwoPlayers
	};

	Game(const glm::uvec2& m_windowSize);
	~Game();

	void render();
	void update(const double delta);
	void setKey(const int key, const int action);
	bool init();
	unsigned int getCurrentWidth() const;
	unsigned int getCurrentHeight() const;
	void startNewLevel(const size_t level, const EGameMode eGameMode);
	void nextLevel(const EGameMode eGameMode);
	void updateViewport();
	void setWindowSize(const glm::uvec2& windowSize);

private:
	std::array<bool, 349> m_keys;

	enum class EGameState {
		StartScreen,
		LevelStart,
		Level,
		Pause,
		Scores,
		GameOver
	};

	glm::ivec2 m_windowSize;
	EGameState m_eCurrentGameState;

	std::shared_ptr<IGameState> m_currentGameState;
	std::shared_ptr<RenderEngine::ShaderProgram> m_spriteShaderProgram;
	size_t m_currentLevelIndex;
};