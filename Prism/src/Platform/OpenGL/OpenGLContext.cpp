#include "PrismPrecompiledHeader.h"
#include "OpenGLContext.h"
#include "glad/glad.h"

namespace Prism
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
	{
		PRISM_ENGINE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Initialize()
	{
		glfwMakeContextCurrent(m_WindowHandle);

		int gladInitializationStatus = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); //Returns the current OpenGL context.
		PRISM_ENGINE_ASSERT(gladInitializationStatus, "Failed to Initialize Glad.");
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}
