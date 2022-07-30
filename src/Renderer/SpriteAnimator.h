#pragma once

#include <memory>

namespace RenderEngine {

	class Sprite;

	class SpriteAnimator {
	public:
		SpriteAnimator(std::shared_ptr<Sprite> sprite);
		size_t getCurrentFrame() const { return m_currentFrame; }
		void update(const double delta);
		double getTotalDuration() const { return m_totalDuration; }
		void reset();

	private:
		std::shared_ptr<Sprite> m_sprite;
		size_t m_currentFrame;
		double m_currentFrameDuration;
		double m_currentAnimationTime;
		double m_totalDuration;
	};
}