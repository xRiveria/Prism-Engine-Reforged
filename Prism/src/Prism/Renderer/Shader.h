#pragma once
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"

namespace Prism
{
	class Shader
	{
	public:
		~Shader() = default;

		virtual void BindShader() const = 0;
		virtual void UnbindShader() const = 0;

		virtual void SetShaderInteger(const std::string& name, int value) = 0;
		virtual void SetShaderIntegerArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetShaderFloat(const std::string& name, int value) = 0;
		virtual void SetShaderFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetShaderFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetShaderMat4(const std::string& name, const glm::mat4& value) = 0;
		
		virtual const std::string& GetShaderName() const = 0;

		static Reference<Shader> CreateShader(const std::string& filePath);
		static Reference<Shader> CreateShader(const std::string& shaderName, std::string& vertexSourceCode, const std::string& fragmentSourceCode);
	};

	class ShaderLibrary //Stores a reference to each created Shader in our application. When we do have a standard 2D Shader, Blur Shader, Postprocessing Shader, PBR Shader etc, we will use a shader library to load these shaders in for use by the entire codebase.
	{
	public:
		void AddShader(const std::string& shaderName, const Reference<Shader>& shader);
		void AddShader(const Reference<Shader>& shader);
		Reference<Shader> LoadShader(const std::string& filePath); //Loads in a shader file from a filePath. Texture.glsl will be stored as Texture.
		Reference<Shader> LoadShader(const std::string& shaderName, const std::string& filePath); //Custom name.

		Reference<Shader> GetShader(const std::string& shaderName);
		bool DoesShaderExist(const std::string& shaderName);

	private:
		std::unordered_map<std::string, Reference<Shader>> m_Shaders;
	};
}
