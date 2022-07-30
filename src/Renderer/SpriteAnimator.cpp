#include "SpriteAnimator.h"

#include "Sprite.h"

namespace RenderEngine {
	SpriteAnimator::SpriteAnimator(std::shared_ptr<Sprite> sprite)
		: m_sprite(std::move(sprite))
		, m_currentFrame(0)
		, m_currentFrameDuration(m_sprite->getFrameDuration(0))
		, m_currentAnimationTime(0)
		, m_totalDuration(0)
	{
		for (size_t currentFrameID = 0; currentFrameID < m_sprite->getFramesCount(); ++currentFrameID) {
			m_totalDuration += m_sprite->getFrameDuration(currentFrameID);
		}
	}

	void SpriteAnimator::update(const double delta) {
		m_currentAnimationTime += delta;

		while (m_currentAnimationTime >= m_currentFrameDuration) {
			m_currentAnimationTime -= m_currentFrameDuration;
			++m_currentFrame;

			if (m_currentFrame == m_sprite->getFramesCount()) {
				m_currentFrame = 0;
			}
			m_currentFrameDuration = m_sprite->getFrameDuration(m_currentFrame);
		}
	}

	void SpriteAnimator::reset() {
		m_currentFrame = 0;
		m_currentFrameDuration = m_sprite->getFrameDuration(0);
		m_currentAnimationTime = 0;
	}
}