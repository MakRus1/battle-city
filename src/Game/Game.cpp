#include "Game.h"

#include "../Resources/ResourceManager.h"
#include "../Renderer/ShaderProgram.h"
#include "../Renderer/Texture2D.h"
#include "../Renderer/Sprite.h"

#include "GameObjects/Tank.h"
#include "GameObjects/Bullet.h"

#include "GameStates/Level.h"
#include "GameStates/StartScreen.h"
#include "../Physics/PhysicsEngine.h"
#include "../Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Game::Game(const glm::uvec2& windowSize) 
    : m_eCurrentGameState(EGameState::StartScreen)
    , m_windowSize(windowSize)
    , m_currentLevelIndex(0)
{
	m_keys.fill(false);
}

void Game::setWindowSize(const glm::uvec2& windowSize) {
    m_windowSize = windowSize;
    updateViewport();
}

Game::~Game() {

}

void Game::render() {
    m_currentGameState->render();
}

void Game::updateViewport() {
    const float map_aspect_ratio = static_cast<float>(getCurrentWidth()) / getCurrentHeight();
    unsigned int viewPortWidth = m_windowSize.x;
    unsigned int viewPortHeight = m_windowSize.y;
    unsigned int viewPortLeftOffset = 0;
    unsigned int viewPortBottomOffset = 0;

    if (static_cast<float>(m_windowSize.x) / m_windowSize.y > map_aspect_ratio) {
        viewPortWidth = static_cast<unsigned int>(m_windowSize.y * map_aspect_ratio);
        viewPortLeftOffset = (m_windowSize.x - viewPortWidth) / 2;
    }
    else {
        viewPortHeight = static_cast<unsigned int>(m_windowSize.x / map_aspect_ratio);
        viewPortBottomOffset = (m_windowSize.y - viewPortHeight) / 2;
    }

    RenderEngine::Renderer::setViewport(viewPortWidth, viewPortHeight, viewPortLeftOffset, viewPortBottomOffset);

    glm::mat4 projectionMatrix = glm::ortho(0.f, static_cast<float>(getCurrentWidth()), 0.f, static_cast<float>(getCurrentHeight()), -100.f, 100.f);

    m_spriteShaderProgram->setMatrix4("projectionMat", projectionMatrix);
}

void Game::startNewLevel(const size_t level, const EGameMode eGameMode) {
    m_currentLevelIndex = level;
    auto pLevel = std::make_shared<Level>(ResourceManager::getLevels()[m_currentLevelIndex], eGameMode);
    m_currentGameState = pLevel;
    Physics::PhysicsEngine::setCurrentLevel(pLevel);
    updateViewport();
}

void Game::nextLevel(const EGameMode eGameMode) {
    startNewLevel(++m_currentLevelIndex, eGameMode);
}

void Game::update(const double delta) {
    m_currentGameState->processInput(m_keys);
    m_currentGameState->update(delta);
}

void Game::setKey(const int key, const int action) {
	m_keys[key] = action;
}

bool Game::init() {
    ResourceManager::loadJSONResources("res/resources.json");

    m_spriteShaderProgram = ResourceManager::getShaderProgram("spriteShader");
    if (!m_spriteShaderProgram) {
        std::cerr << "Can't find shader program: " << "spriteShader" << std::endl;
        return false;
    }

    m_spriteShaderProgram->use();
    m_spriteShaderProgram->setInt("tex", 0);

    m_currentGameState = std::make_shared<StartScreen>(ResourceManager::getStartScreen(), this);

    setWindowSize(m_windowSize);

    return true;
}

unsigned int Game::getCurrentWidth() const {
    return m_currentGameState->getStateWidth();
}

unsigned int Game::getCurrentHeight() const {
    return m_currentGameState->getStateHeight();
}