#include "ResourceManager.h"
#include "../Renderer/ShaderProgram.h"
#include "../Renderer/Texture2D.h"
#include "../Renderer/Sprite.h"

#include <sstream>
#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

ResourceManager::ShaderProgramsMap ResourceManager::m_shaderPrograms;
ResourceManager::TexturesMap ResourceManager::m_textures;
ResourceManager::SpritesMap ResourceManager::m_sprites;

std::string ResourceManager::m_path;
std::vector<std::vector<std::string>> ResourceManager::m_levels;
std::vector<std::string> ResourceManager::m_startScreen;

void ResourceManager::unloadResources() {
	m_shaderPrograms.clear();
	m_textures.clear();
	m_sprites.clear();
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
																													shader)).first->second;
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
			glm::vec2 leftBottomUV(static_cast<float>(currentTextureOffsetX + 0.01f) / textureWidth,				 static_cast<float>(currentTextureOffsetY - subTextureHeight + 0.01f) / textureHeight);
			glm::vec2 rightTopUV(static_cast<float>(currentTextureOffsetX + SubTextureWidth - 0.01f) / textureWidth, static_cast<float>(currentTextureOffsetY - 0.01f) / textureHeight);
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

bool ResourceManager::loadJSONResources(const std::string& JSONPath) {
	const std::string JSONString = getFileString(JSONPath);
	if (JSONString.empty()) {
		std::cerr << "No JSON resources file!" << std::endl;
		return false;
	}

	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse(JSONString.c_str());
	if (!parseResult) {
		std::cerr << "JSON parse error: " << rapidjson::GetParseError_En(parseResult.Code()) << "(" << parseResult.Offset() << ")" << std::endl;
		std::cerr << "In JSON file: " << JSONPath << std::endl;
		return false;
	}

	auto shadersIt = document.FindMember("shaders");
	if (shadersIt != document.MemberEnd()) {
		for (const auto& currentShader : shadersIt->value.GetArray()) {
			const std::string name = currentShader["name"].GetString();
			const std::string filePath_v = currentShader["filePath_v"].GetString();
			const std::string filePath_f = currentShader["filePath_f"].GetString();
			loadShaders(name, filePath_v, filePath_f);
		}
	}

	auto textureAtlasesIt = document.FindMember("textureAtlases");
	if (textureAtlasesIt != document.MemberEnd()) {
		for (const auto& currentTextureAtlas : textureAtlasesIt->value.GetArray()) {
			const std::string name = currentTextureAtlas["name"].GetString();
			const std::string filePath = currentTextureAtlas["filePath"].GetString();
			const unsigned int subTextureWidth = currentTextureAtlas["subTextureWidth"].GetUint();
			const unsigned int subTextureHeight = currentTextureAtlas["subTextureHeight"].GetUint();

			const auto subTexturesArray = currentTextureAtlas["subTextures"].GetArray();
			std::vector<std::string> subTextures;
			subTextures.reserve(subTexturesArray.Size());
			for (const auto& currentSubTexture : subTexturesArray) {
				subTextures.emplace_back(currentSubTexture.GetString());
			}
			loadTextureAtlas(name, filePath, std::move(subTextures), subTextureWidth, subTextureHeight);
		}
	}

	auto spritesIt = document.FindMember("sprites");
	if (spritesIt != document.MemberEnd()) {
		for (const auto& currentSprite : spritesIt->value.GetArray()) {
			const std::string name = currentSprite["name"].GetString();
			const std::string textureAtlas = currentSprite["textureAtlas"].GetString();
			const std::string shader = currentSprite["shader"].GetString();
			const std::string subTextureName = currentSprite["initialSubTexture"].GetString();

			auto sprite = loadSprite(name, textureAtlas, shader, subTextureName);
			if (!sprite) {
				continue;
			}

			auto framesIt = currentSprite.FindMember("frames");
			if (framesIt != currentSprite.MemberEnd()) {
				const auto framesArray = framesIt->value.GetArray();
				std::vector<RenderEngine::Sprite::FrameDescription> framesDescriptions;
				framesDescriptions.reserve(framesArray.Size());
				for (const auto& currentFrame : framesArray) {
					const std::string subTextureStr = currentFrame["subTexture"].GetString();
					const double duration = currentFrame["duration"].GetDouble();
					const auto pTextureAtlas = getTexture(textureAtlas);
					const auto pSubTexture = pTextureAtlas->getSubTexture(subTextureStr);
					framesDescriptions.emplace_back(pSubTexture.leftBottomUV, pSubTexture.rightTopUV, duration);
				}
				sprite->insertFrames(std::move(framesDescriptions));
			}
		}
	}

	auto startScreenIt = document.FindMember("start_screen");
	if (startScreenIt != document.MemberEnd()) {
		const auto descriptionArray = startScreenIt->value.GetArray();
		m_startScreen.reserve(descriptionArray.Size());
		size_t maxLength = 0;
		for (const auto& currentRow : descriptionArray) {
			m_startScreen.emplace_back(currentRow.GetString());
			if (maxLength < m_startScreen.back().length()) {
				maxLength = m_startScreen.back().length();
			}
		}

		for (auto& currentRow : m_startScreen) {
			while (currentRow.length() < maxLength) {
				currentRow.append("F");
			}
		}
	}

	auto levelsIt = document.FindMember("levels");
	if (levelsIt != document.MemberEnd()) {
		for (const auto& currentLevel : levelsIt->value.GetArray()) {
			const auto description = currentLevel["description"].GetArray();
			std::vector<std::string> levelRows;
			levelRows.reserve(description.Size());
			size_t maxLength = 0;
			for (const auto& currentRow : description) {
				levelRows.emplace_back(currentRow.GetString());
				if (maxLength < levelRows.back().length()) {
					maxLength = levelRows.back().length();
				}
			}

			for (auto& currentRow : levelRows) {
				while (currentRow.length() < maxLength) {
					currentRow.append("D");
				}
			}
			m_levels.emplace_back(std::move(levelRows));
		}
	}
	return true;
}