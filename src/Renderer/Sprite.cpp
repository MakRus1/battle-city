#include "Sprite.h"

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Renderer.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RenderEngine {

	Sprite::Sprite(const std::shared_ptr<Texture2D> texture,
		const std::string initialSubTexture,
		const std::shared_ptr<ShaderProgram> shaderProgram) 
		: m_texture(std::move(texture))
		, m_shaderProgram(std::move(shaderProgram))
		, m_lastFrameID(0)
	{
		const GLfloat vertexCoords[] = {
			// X Y
			0.f, 0.f,
			0.f, 1.f,
			1.f, 1.f,
			1.f, 0.f,
		};

		auto subTexture = m_texture->getSubTexture(std::move(initialSubTexture));

		const GLfloat textureCoords[] = {
			// U V
			subTexture.leftBottomUV.x, subTexture.leftBottomUV.y,	
			subTexture.leftBottomUV.x, subTexture.rightTopUV.y,
			subTexture.rightTopUV.x, subTexture.rightTopUV.y,
			subTexture.rightTopUV.x, subTexture.leftBottomUV.y,
		};

		const GLuint indexes[] = {
			0, 1, 2,
			2, 3, 0
		};

		m_vertexCoordsBuffer.init(vertexCoords, 2 * 4 * sizeof(GL_FLOAT));
		VertexBufferLayout vertexCoordsLayout;
		vertexCoordsLayout.addElementLayoutFloat(2, false);
		m_vertexArray.addBuffer(m_vertexCoordsBuffer, vertexCoordsLayout);

		m_textureCoordsBuffer.init(textureCoords, 2 * 4 * sizeof(GL_FLOAT));
		VertexBufferLayout textureCoordsLayout;
		textureCoordsLayout.addElementLayoutFloat(2, false);
		m_vertexArray.addBuffer(m_textureCoordsBuffer, textureCoordsLayout);

		m_indexBuffer.init(indexes, 6);
		
		m_vertexArray.unbind();
		m_indexBuffer.unbind();
	}

	Sprite::~Sprite() {
	}

	void Sprite::render(const glm::vec2& position, const glm::vec2& size, const float rotation, const float layer, const size_t frameID) const {
		if (m_lastFrameID != frameID) {
			m_lastFrameID = frameID;

			const FrameDescription& currentFrameDescription = m_framesDescriptions[frameID];

			const GLfloat textureCoords[] = {
				// U V
				currentFrameDescription.leftBottomUV.x, currentFrameDescription.leftBottomUV.y,
				currentFrameDescription.leftBottomUV.x, currentFrameDescription.rightTopUV.y,
				currentFrameDescription.rightTopUV.x, currentFrameDescription.rightTopUV.y,
				currentFrameDescription.rightTopUV.x, currentFrameDescription.leftBottomUV.y,
			};

			m_textureCoordsBuffer.update(textureCoords, 2 * 4 * sizeof(GLfloat));
		}

		m_shaderProgram->use();

		glm::mat4 model(1.f);

		model = glm::translate(model, glm::vec3(position, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.f));
		model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.f, 0.f, 1.f));
		model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.f));
		model = glm::scale(model, glm::vec3(size, 1.f));

		m_shaderProgram->setMatrix4("modelMat", model);
		m_shaderProgram->setFloat("layer", layer);

		glActiveTexture(GL_TEXTURE0);
		m_texture->bind();

		Renderer::draw(m_vertexArray, m_indexBuffer, *m_shaderProgram);
	}

	void Sprite::insertFrames(std::vector<FrameDescription> framesDescriptions) {
		m_framesDescriptions = std::move(framesDescriptions);
	}

	double Sprite::getFrameDuration(const size_t frameID) const {
		return m_framesDescriptions[frameID].duration;
	}

	size_t Sprite::getFramesCount() const {
		return m_framesDescriptions.size();
	}
}