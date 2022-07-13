#include "Sprite.h"

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Renderer.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RenderEngine {

	Sprite::Sprite(const std::shared_ptr<Texture2D> texture,
		const std::string initialSubTexture,
		const std::shared_ptr<ShaderProgram> shaderProgram,
		const glm::vec2& position,
		const glm::vec2& size,
		const float rotation) 
		: m_texture(std::move(texture))
		, m_shaderProgram(std::move(shaderProgram))
		, m_position(position)
		, m_size(size)
		, m_rotation(rotation)
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

	void Sprite::render() const {
		m_shaderProgram->use();

		glm::mat4 model(1.f);

		model = glm::translate(model, glm::vec3(m_position, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * m_size.x, 0.5f * m_size.y, 0.f));
		model = glm::rotate(model, glm::radians(m_rotation), glm::vec3(0.f, 0.f, 1.f));
		model = glm::translate(model, glm::vec3(-0.5f * m_size.x, -0.5f * m_size.y, 0.f));
		model = glm::scale(model, glm::vec3(m_size, 1.f));

		m_shaderProgram->setMatrix4("modelMat", model);

		glActiveTexture(GL_TEXTURE0);
		m_texture->bind();

		Renderer::draw(m_vertexArray, m_indexBuffer, *m_shaderProgram);
	}

	void Sprite::setPosition(const glm::vec2& position) {
		m_position = position;
	}

	void Sprite::setSize(const glm::vec2& size) {
		m_size = size;
	}

	void Sprite::setRotation(const float rotation) {
		m_rotation = rotation;
	}

}