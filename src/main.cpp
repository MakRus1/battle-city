#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include <iostream>
#include <chrono>

#include "Game/Game.h"
#include "Resources/ResourceManager.h"
#include "Renderer/Renderer.h"

glm::ivec2 g_windowSize(640, 480);
Game g_game(g_windowSize);

void glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {
    g_windowSize.x = width;
    g_windowSize.y = height;
    RenderEngine::Renderer::setViewport(g_windowSize.x, g_windowSize.y);
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    g_game.setKey(key, action);
}

int main(int argc, char** argv)
{
    /* Initialize the library */
    if (!glfwInit()) {
        std::cout << "glfwInit() failed!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(g_windowSize.x, g_windowSize.y, "Battle City", nullptr, nullptr);
    if (!window)
    {
        std::cout << "glfwCreateWindow failed!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetWindowSizeCallback(window, glfwWindowSizeCallback);
    glfwSetKeyCallback(window, glfwKeyCallback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (!gladLoadGL()){
        std::cout << "Can't load GLAD" << std::endl;
        return -1;
    }

    std::cout << "Renderer " << RenderEngine::Renderer::getRendererStr() << std::endl;
    std::cout << "OpenGL Version " << RenderEngine::Renderer::getVersionStr() << std::endl;

    RenderEngine::Renderer::setClearColor(0, 0, 0, 1);

    {
        ResourceManager::setExecutablePath(argv[0]);
        g_game.init();

        auto lastTime = std::chrono::high_resolution_clock::now();

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            uint64_t duration = std::chrono::duration_cast <std::chrono::nanoseconds> (currentTime - lastTime).count();
            lastTime = currentTime;
            g_game.update(duration);

            /* Render here */
            RenderEngine::Renderer::clear();

            g_game.render();

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
        ResourceManager::unloadResources();
    }

    glfwTerminate();
    return 0;
}