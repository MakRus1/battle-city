#include "AnimatedSprite.h"
#include "Texture2D.h"

#include <iostream>

namespace RenderEngine {

	AnimatedSprite::AnimatedSprite(const std::shared_ptr<Texture2D> texture,
		const std::string initialSubTexture,
		const std::shared_ptr<ShaderProgram> shaderProgram,
		const glm::vec2& position,
		const glm::vec2& size,
		const float rotation)
		: Sprite(std::move(texture), std::move(initialSubTexture), std::move(shaderProgram), position, size, rotation)
	{
		m_currentAnimationDurations = m_statesMap.end();
	}

	void AnimatedSprite::insertState(std::string state, std::vector<std::pair <std::string, uint64_t>> subTextureDuration) {
		m_statesMap.emplace(std::move(state), std::move(subTextureDuration));
	}

	void AnimatedSprite::setState(const std::string newState) {
		auto it = m_statesMap.find(newState);
		if (it == m_statesMap.end()) {
			std::cout << "Can't find animation state: " << newState << std::endl;
			return;
		}

		if (it != m_currentAnimationDurations) {
			m_currentAnimationTime = 0;
			m_currentFrame = 0;
			m_currentAnimationDurations = it;
			m_dirty = true;
		}
	}

	void AnimatedSprite::update(const uint64_t delta) {
		if (m_currentAnimationDurations != m_statesMap.end()) {
			m_currentAnimationTime += delta;

			while (m_currentAnimationTime >= m_currentAnimationDurations->second[m_currentFrame].second) {
				m_currentAnimationTime -= m_currentAnimationDurations->second[m_currentFrame].second;
				++m_currentFrame;
				m_dirty = true;

				if (m_currentFrame == m_currentAnimationDurations->second.size()) {
					m_currentFrame = 0;
				}
			}
		}
	}

	void AnimatedSprite::render() const {
		if (m_dirty) {
			auto subTexture = m_texture->getSubTexture(m_currentAnimationDurations->second[m_currentFrame].first);

			const GLfloat textureCoords[] = {
				// U V
				subTexture.leftBottomUV.x, subTexture.leftBottomUV.y,
				subTexture.leftBottomUV.x, subTexture.rightTopUV.y,
				subTexture.rightTopUV.x, subTexture.rightTopUV.y,
				subTexture.rightTopUV.x, subTexture.leftBottomUV.y,
			};

			m_textureCoordsBuffer.update(textureCoords, 2 * 4 * sizeof(GLfloat));
			m_dirty = false;
		}

		Sprite::render();
	}
}