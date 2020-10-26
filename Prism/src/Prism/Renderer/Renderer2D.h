#pragma once
#include "OrthographicCamera.h"

namespace Prism
{
	class Renderer2D
	{
	public:
		static void Initialize2DRenderer();
		static void Shutdown2DRenderer();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		//Primitives
		static void DrawQuad(const glm::vec2& quadPosition, const glm::vec2& quadSize, const glm::vec4& quadColor);
		static void DrawQuad(const glm::vec3& quadPosition, const glm::vec2& quadSize, const glm::vec4& quadColor); //We use Vec3 here because we can possibly use the last Z value for sorting, such as rendering certain things at the back of the scene - almost layering of sorts.
	};
}