#include "Sandbox2D.h"
#include "imgui/imgui.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Prism/Renderer/Renderer.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{

}

void Sandbox2D::OnAttach()
{
	
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Prism::Timestep timeStep)
{
	//Update
	m_CameraController.OnUpdate(timeStep);

	//Render
	Prism::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	Prism::RenderCommand::Clear();

	Prism::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Prism::Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, {m_SquareColor});

	Prism::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Prism::Event& event)
{
	m_CameraController.OnEvent(event);
}