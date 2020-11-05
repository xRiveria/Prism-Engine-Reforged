#pragma once
#include "glm/glm.hpp"

namespace Prism
{
	struct TransformComponent
	{
		glm::mat4 m_Transform = glm::mat4(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& transform) : m_Transform(transform) {}

		operator glm::mat4&() { return m_Transform; }
		operator const glm::mat4&() const { return m_Transform; }
	};

	struct SpriteRendererComponent
	{
		glm::vec4 m_Color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color) : m_Color(color) {}
	};
}