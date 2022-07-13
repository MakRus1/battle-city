#include "ResourceManager.h"
#include "../Renderer/ShaderProgram.h"
#include "../Renderer/Texture2D.h"
#include "../Renderer/Sprite.h"
#include "../Renderer/AnimatedSprite.h"

#include <sstream>
#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

ResourceManager::ShaderProgramsMap ResourceManager::m_shaderPrograms;
ResourceManager::TexturesMap ResourceManager::m_textures;
ResourceManager::SpritesMap ResourceManager::m_sprites;
ResourceManager::AnimatedSpritesMap ResourceManager::m_animatedSprites;

std::string ResourceManager::m_path;

void ResourceManager::unloadResources() {
	m_shaderPrograms.clear();
	m_textures.clear();
	m_sprites.clear();
	m_animatedSprites.clear();
}

void ResourceManager::setExecutablePath(const std::string& executablePath) {
	size_t found = executablePath.find_last_of("/\\");
	m_path = executablePath.substr(0, found);
}

std::string ResourceManager::getFileString(const std::string& relativeFilePath) {
	std::ifstream f;
	f.open(m_path + "/" + relativeFilePath.c_str(), std::ios::in | std::ios::binary);
	if (!f.is_open()) {
		std::cerr << "Failed to open file: " << relativeFilePath << std::endl;
		return std::string();
	}

	std::stringstream buffer;
	buffer << f.rdbuf();
	return buffer.str();
}

std::shared_ptr <RenderEngine::ShaderProgram> ResourceManager::loadShaders(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath) {
	std::string vertexString = getFileString(vertexPath);
	if (vertexString.empty()) {
		std::cerr << "No vertex shader!" << std::endl;
		return nullptr;
	}

	std::string fragmentString = getFileString(fragmentPath);
	if (fragmentString.empty()) {
		std::cerr << "No fragment shader!" << std::endl;
		return nullptr;
	}

	std::shared_ptr<RenderEngine::ShaderProgram>& newShader = m_shaderPrograms.emplace(shaderName, std::make_shared<RenderEngine::ShaderProgram>(vertexString, fragmentString)).first->second;
	if (newShader->isCompiled()) {
		return newShader;
	}

	std::cerr << "Can't load shader program:\n"
		<< "Vertex: " << vertexPath << "\n"
		<< "Fragment: " << fragmentPath << std::endl;

	return nullptr;
}

std::shared_ptr <RenderEngine::ShaderProgram> ResourceManager::getShaderProgram(const std::string& shaderName) {
	ShaderProgramsMap::const_iterator it = m_shaderPrograms.find(shaderName);
	if (it != m_shaderPrograms.end()) {
		return it->second;
	}
	std::cerr << "Can't find shader program: " << shaderName << std::endl;
	return nullptr;
}

std::shared_ptr <RenderEngine::Texture2D> ResourceManager::loadTexture(const std::string& textureName, const std::string& texturePath) {
	int channels = 0;
	int width = 0;
	int height = 0;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* pixels = stbi_load(std::string(m_path + "/" + texturePath).c_str(), &width, &height, &channels, 0);

	if (!pixels) {
		std::cerr << "Can't load image: " << texturePath << std::endl;
		return nullptr;
	}

	std::shared_ptr <RenderEngine::Texture2D> newTexture = m_textures.emplace(textureName, std::make_shared<RenderEngine::Texture2D>(width, 
																															 height, 
																															 pixels, 
																															 channels, 
																															 GL_NEAREST, 
																															 GL_CLAMP_TO_EDGE)).first->second;

	stbi_image_free(pixels);
	return newTexture;
}

std::shared_ptr <RenderEngine::Texture2D> ResourceManager::getTexture(const std::string& textureName) {
	TexturesMap::const_iterator it = m_textures.find(textureName);
	if (it != m_textures.end()) {
		return it->second;
	}
	std::cerr << "Can't find texture: " << textureName << std::endl;
	return nullptr;
}

std::shared_ptr <RenderEngine::Sprite> ResourceManager::loadSprite(const std::string& spriteName,
															   const std::string& textureName,
															   const std::string& shaderName,
															   const unsigned int spriteWidth,
															   const unsigned int spriteHeight,
															   const std::string& subTextureName) {
	auto texture = getTexture(textureName);
	if (!texture) {
		std::cerr << "Can't find texture: " << textureName << " for the sprite: " << spriteName << std::endl;
		return nullptr;
	}

	auto shader = getShaderProgram(shaderName);
	if (!shader) {
		std::cerr << "Can't find shader: " << shaderName << " for the sprite: " << spriteName << std::endl;
		return nullptr;
	}

	std::shared_ptr <RenderEngine::Sprite> newSprite = m_sprites.emplace(spriteName, std::make_shared<RenderEngine::Sprite>(texture,
																													subTextureName,
																													shader,
																													glm::vec2(0.f, 0.f),
																													glm::vec2(spriteWidth, spriteHeight))).first->second;
	return newSprite;
}

std::shared_ptr <RenderEngine::AnimatedSprite> ResourceManager::loadAnimatedSprite(const std::string& spriteName,
	const std::string& textureName,
	const std::string& shaderName,
	const unsigned int spriteWidth,
	const unsigned int spriteHeight,
	const std::string& subTextureName) {
	auto texture = getTexture(textureName);
	if (!texture) {
		std::cerr << "Can't find texture: " << textureName << " for the sprite: " << spriteName << std::endl;
		return nullptr;
	}

	auto shader = getShaderProgram(shaderName);
	if (!shader) {
		std::cerr << "Can't find shader: " << shaderName << " for the sprite: " << spriteName << std::endl;
		return nullptr;
	}

	std::shared_ptr <RenderEngine::AnimatedSprite> newSprite = m_animatedSprites.emplace(spriteName, std::make_shared<RenderEngine::AnimatedSprite>(texture,
		subTextureName,
		shader,
		glm::vec2(0.f, 0.f),
		glm::vec2(spriteWidth, spriteHeight))).first->second;
	return newSprite;
}

std::shared_ptr <RenderEngine::Sprite> ResourceManager::getSprite(const std::string& spriteName) {
	SpritesMap::const_iterator it = m_sprites.find(spriteName);
	if (it != m_sprites.end()) {
		return it->second;
	}
	std::cerr << "Can't find the sprite: " << spriteName << std::endl;
	return nullptr;
}

std::shared_ptr <RenderEngine::AnimatedSprite> ResourceManager::getAnimatedSprite(const std::string& spriteName) {
	AnimatedSpritesMap::const_iterator it = m_animatedSprites.find(spriteName);
	if (it != m_animatedSprites.end()) {
		return it->second;
	}
	std::cerr << "Can't find animated sprite: " << spriteName << std::endl;
	return nullptr;
}

std::shared_ptr <RenderEngine::Texture2D> ResourceManager::loadTextureAtlas(const std::string textureName,
																		const std::string texturePath,
																		const std::vector <std::string> subTextures,
																		const unsigned int SubTextureWidth,
																		const unsigned int subTextureHeight) {

	auto texture = loadTexture(std::move(textureName), std::move(texturePath));
	if (texture) {
		const unsigned int textureWidth = texture->width();
		const unsigned int textureHeight = texture->height();
		unsigned int currentTextureOffsetX = 0;
		unsigned int currentTextureOffsetY = textureHeight;
		for (const auto& currentSubTextureName : subTextures) {
			glm::vec2 leftBottomUV(static_cast<float>(currentTextureOffsetX) / textureWidth,				 static_cast<float>(currentTextureOffsetY - subTextureHeight) / textureHeight);
			glm::vec2 rightTopUV(static_cast<float>(currentTextureOffsetX + SubTextureWidth) / textureWidth, static_cast<float>(currentTextureOffsetY) / textureHeight);
			texture->addSubTexture(std::move(currentSubTextureName), leftBottomUV, rightTopUV);

			currentTextureOffsetX += SubTextureWidth;
			if (currentTextureOffsetX >= textureWidth) {
				currentTextureOffsetX = 0;
				currentTextureOffsetY -= subTextureHeight;
			}
		}
	}
	return texture;
}