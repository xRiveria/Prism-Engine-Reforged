#include "PrismPrecompiledHeader.h"
#include "Renderer2D.h"
#include "Shader.h"
#include "VertexArray.h"
#include "RenderCommand.h"
#include "glm/gtc/matrix_transform.hpp"

namespace Prism
{
	//Information per Vertex. To add TexID, Masking etc.
	struct QuadVertex 
	{
		glm::vec3 m_QuadPosition;
		glm::vec4 m_QuadColor;
		glm::vec2 m_TexCoord;
		float m_TexIndex; //If it's 0, its white which means it will sample from the white texture and not draw anything.
		float m_TilingFactor;
	};

	//Information per Draw Call. (Batching)
	struct Renderer2DData
	{
		const uint32_t m_MaxQuadsToDraw = 10000;
		const uint32_t m_MaxVertices = m_MaxQuadsToDraw * 4;
		const uint32_t m_MaxIndices = m_MaxQuadsToDraw * 6;
		static const uint32_t m_MaxTextureSlots = 32; //To Do: RenderCapabilities.

		Reference<Prism::VertexArray> m_QuadVertexArray;
		Reference<Prism::VertexBuffer> m_QuadVertexBuffer;
		Reference<Prism::Shader> m_TextureShader;
		Reference<Prism::Texture2D> m_WhiteTexture;

		uint32_t m_QuadIndexCount = 0; //Increments everytime a Quad is drawed.
		QuadVertex* m_QuadVertexBufferBase = nullptr; //To keep track of our max base amount of vertices.
		QuadVertex* m_QuadVertexBufferPointer = nullptr; //We need to create and push into the Vertex Buffer for new vertices.
	
		std::array<Reference<Texture2D>, m_MaxTextureSlots> m_TextureSlots; //Identifier for bound textures. In the future, all our assets will have a asset handle (UUID) that will point to the actual asset.
		uint32_t m_CurrentTextureSlotIndex = 1; //0 is our white texture. //m_CurrentTextureSlotIndex will always be the next free texture slot we can assign to. 
	};

	static Renderer2DData s_Data; 

	void Renderer2D::Initialize2DRenderer()
	{
		PRISM_PROFILE_FUNCTION();

		s_Data.m_QuadVertexArray = VertexArray::CreateVertexArray();
		s_Data.m_QuadVertexBuffer = VertexBuffer::CreateVertexBuffer(s_Data.m_MaxVertices * sizeof(QuadVertex));
		
		s_Data.m_QuadVertexBuffer->SetBufferLayout(
			{
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color" },
			    { ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Float,  "a_TexIndex" },
				{ ShaderDataType::Float,  "a_TilingFactor" }
			});
		s_Data.m_QuadVertexArray->AddVertexBuffer(s_Data.m_QuadVertexBuffer);

		s_Data.m_QuadVertexBufferBase = new QuadVertex[s_Data.m_MaxVertices];
		//======== Index Buffer (Indices) ========

		uint32_t* quadIndices = new uint32_t[s_Data.m_MaxIndices];
		
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.m_MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Reference<IndexBuffer> quadIndexBuffer = IndexBuffer::CreateIndexBuffer(quadIndices, s_Data.m_MaxIndices);
		s_Data.m_QuadVertexArray->SetIndexBuffer(quadIndexBuffer);
		delete[] quadIndices;
		
		s_Data.m_WhiteTexture = Texture2D::CreateTexture(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.m_WhiteTexture->SetTextureData(&whiteTextureData, sizeof(whiteTextureData));

		int32_t samplers[s_Data.m_MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.m_MaxTextureSlots; i++)
		{
			samplers[i] = i; //Initialize sampler data to texture slot number.
		}

		s_Data.m_TextureShader = Shader::CreateShader("assets/shaders/Texture.glsl");
		s_Data.m_TextureShader->BindShader();
		s_Data.m_TextureShader->SetShaderIntegerArray("u_Textures", samplers, s_Data.m_MaxTextureSlots);

		//Set white texture to index 0 for our texture slots.
		s_Data.m_TextureSlots[0] = s_Data.m_WhiteTexture; //Sets Index 0 of our avaliable texture slots to the White Texture. 
	}

	void Renderer2D::Shutdown2DRenderer()
	{
		PRISM_PROFILE_FUNCTION();
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		PRISM_PROFILE_FUNCTION();

		s_Data.m_TextureShader->BindShader();
		s_Data.m_TextureShader->SetShaderMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	
		s_Data.m_QuadIndexCount = 0; //Reset amount of indexes currently drawn.
		s_Data.m_CurrentTextureSlotIndex = 1;

		s_Data.m_QuadVertexBufferPointer = s_Data.m_QuadVertexBufferBase; //Set the pointer to the base. Now, we can freely increment the pointer without losing track of the base. 
	}

	void Renderer2D::EndScene()
	{
		PRISM_PROFILE_FUNCTION();

		//We flush whenever we have drawn the max amount of vertices possible for a frame, or when the scene has ended.
		uint32_t dataSize = (uint8_t*)s_Data.m_QuadVertexBufferPointer - (uint8_t*)s_Data.m_QuadVertexBufferBase; //uint8_t is 1 Byte. Thus, when we subtract, we get the amount of elements it takes up in terms of bytes. 
		s_Data.m_QuadVertexBuffer->SetData(s_Data.m_QuadVertexBufferBase, dataSize);
		FlushRenderer();
	}

	void Renderer2D::FlushRenderer()
	{
		//Bind Texture 
		for (uint32_t i = 0; i < s_Data.m_CurrentTextureSlotIndex; i++) //Essentially all texture slots up to the currently bound one. (1 - 24, for example)
		{
			//Bind the texture in each of the slots to corresponding texture slots.
			s_Data.m_TextureSlots[i]->BindTexture(i); //BindTexture() takes in a texture slot. We pass in i because that is the index of our texture slot. 
		}
		RenderCommand::DrawIndexed(s_Data.m_QuadVertexArray, s_Data.m_QuadIndexCount);
	}

	void Renderer2D::DrawQuad(const glm::vec2& quadPosition, const glm::vec2& quadSize, const glm::vec4& quadColor)
	{
		DrawQuad({quadPosition.x, quadPosition.y, 0.0f}, quadSize, quadColor);
	}
	
	void Renderer2D::DrawQuad(const glm::vec3& quadPosition, const glm::vec2& quadSize, const glm::vec4& quadColor)
	{
		PRISM_PROFILE_FUNCTION();

		const float textureIndex = 0.0f; //White Texture
		const float tilingFactor = 1.0f;

		s_Data.m_QuadVertexBufferPointer->m_QuadPosition = quadPosition;
		s_Data.m_QuadVertexBufferPointer->m_QuadColor = quadColor;
		s_Data.m_QuadVertexBufferPointer->m_TexCoord = { 0.0f, 0.0f, };
		s_Data.m_QuadVertexBufferPointer->m_TexIndex = textureIndex;
		s_Data.m_QuadVertexBufferPointer->m_TilingFactor = tilingFactor;

		s_Data.m_QuadVertexBufferPointer++;

		s_Data.m_QuadVertexBufferPointer->m_QuadPosition = { quadPosition.x + quadSize.x, quadPosition.y, 0.0f };
		s_Data.m_QuadVertexBufferPointer->m_QuadColor = quadColor;
		s_Data.m_QuadVertexBufferPointer->m_TexCoord = { 1.0f, 0.0f, };
		s_Data.m_QuadVertexBufferPointer->m_TexIndex = textureIndex;
		s_Data.m_QuadVertexBufferPointer->m_TilingFactor = tilingFactor;

		s_Data.m_QuadVertexBufferPointer++;

		s_Data.m_QuadVertexBufferPointer->m_QuadPosition = { quadPosition.x + quadSize.x, quadPosition.y + quadSize.y, 0.0f };
		s_Data.m_QuadVertexBufferPointer->m_QuadColor = quadColor;
		s_Data.m_QuadVertexBufferPointer->m_TexCoord = { 1.0f, 0.0f, };
		s_Data.m_QuadVertexBufferPointer->m_TexIndex = textureIndex;
		s_Data.m_QuadVertexBufferPointer->m_TilingFactor = tilingFactor;

		s_Data.m_QuadVertexBufferPointer++;

		s_Data.m_QuadVertexBufferPointer->m_QuadPosition = { quadPosition.x, quadPosition.y + quadSize.y, 0.0f };
		s_Data.m_QuadVertexBufferPointer->m_QuadColor = quadColor;
		s_Data.m_QuadVertexBufferPointer->m_TexCoord = { 0.0f, 1.0f, };
		s_Data.m_QuadVertexBufferPointer->m_TexIndex = textureIndex;
		s_Data.m_QuadVertexBufferPointer->m_TilingFactor = tilingFactor;

		s_Data.m_QuadVertexBufferPointer++;

		s_Data.m_QuadIndexCount += 6;
		/*s_Data.m_TextureShader->SetShaderFloat4("u_Color", quadColor);
		s_Data.m_TextureShader->SetShaderFloat("u_TilingFactor", 1.0f);
		s_Data.m_WhiteTexture->BindTexture();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), quadPosition) * glm::scale(glm::mat4(1.0f), {quadSize.x, quadSize.y, 1.0f});
		s_Data.m_TextureShader->SetShaderMat4("u_Transform", transform);

		s_Data.m_QuadVertexArray->BindVertexArray();
		RenderCommand::DrawIndexed(s_Data.m_QuadVertexArray);*/
	}

	//======== Texture ========

	void Renderer2D::DrawQuad(const glm::vec2& quadPosition, const glm::vec2& quadSize, const Reference<Texture2D>& quadTexture, float quadTilingFactor, const glm::vec4& quadTintColor)
	{
		DrawQuad({ quadPosition.x, quadPosition.y, 0.0f }, quadSize, quadTexture, quadTilingFactor, quadTintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& quadPosition, const glm::vec2& quadSize, const Reference<Texture2D>& quadTexture, float quadTilingFactor, const glm::vec4& quadTintColor)
	{
		PRISM_PROFILE_FUNCTION();

		constexpr glm::vec4 quadColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data.m_CurrentTextureSlotIndex; i++) //Check if texture already exists in our existing texture slots.
		{
			if (*s_Data.m_TextureSlots[i].get() == *quadTexture.get()) //.get() returns the stored pointer. By dereferencing it, we are comparing the data within the pointers.
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f) //If texture isn't bound to anything, we manually assign it a new slot. 
		{
			textureIndex = (float)s_Data.m_CurrentTextureSlotIndex; //m_CurrentTextureSlotIndex will always be the next free texture slot we can assign to. 
			s_Data.m_TextureSlots[s_Data.m_CurrentTextureSlotIndex] = quadTexture;
			s_Data.m_CurrentTextureSlotIndex++;
		}

		s_Data.m_QuadVertexBufferPointer->m_QuadPosition = quadPosition;
		s_Data.m_QuadVertexBufferPointer->m_QuadColor = quadColor;
		s_Data.m_QuadVertexBufferPointer->m_TexCoord = { 0.0f, 0.0f, };
		s_Data.m_QuadVertexBufferPointer->m_TexIndex = textureIndex;
		s_Data.m_QuadVertexBufferPointer->m_TilingFactor = quadTilingFactor;
		s_Data.m_QuadVertexBufferPointer++;

		s_Data.m_QuadVertexBufferPointer->m_QuadPosition = { quadPosition.x + quadSize.x, quadPosition.y, 0.0f };
		s_Data.m_QuadVertexBufferPointer->m_QuadColor = quadColor;
		s_Data.m_QuadVertexBufferPointer->m_TexCoord = { 1.0f, 0.0f, };
		s_Data.m_QuadVertexBufferPointer->m_TexIndex = textureIndex;
		s_Data.m_QuadVertexBufferPointer->m_TilingFactor = quadTilingFactor;
		s_Data.m_QuadVertexBufferPointer++;

		s_Data.m_QuadVertexBufferPointer->m_QuadPosition = { quadPosition.x + quadSize.x, quadPosition.y + quadSize.y, 0.0f };
		s_Data.m_QuadVertexBufferPointer->m_QuadColor = quadColor;
		s_Data.m_QuadVertexBufferPointer->m_TexCoord = { 1.0f, 1.0f, };
		s_Data.m_QuadVertexBufferPointer->m_TexIndex = textureIndex;
		s_Data.m_QuadVertexBufferPointer->m_TilingFactor = quadTilingFactor;
		s_Data.m_QuadVertexBufferPointer++;

		s_Data.m_QuadVertexBufferPointer->m_QuadPosition = { quadPosition.x, quadPosition.y + quadSize.y, 0.0f };
		s_Data.m_QuadVertexBufferPointer->m_QuadColor = quadColor;
		s_Data.m_QuadVertexBufferPointer->m_TexCoord = { 0.0f, 1.0f, };
		s_Data.m_QuadVertexBufferPointer->m_TexIndex = textureIndex;
		s_Data.m_QuadVertexBufferPointer->m_TilingFactor = quadTilingFactor;

		s_Data.m_QuadVertexBufferPointer++;

		s_Data.m_QuadIndexCount += 6;

#if OLD_PATH
		s_Data.m_TextureShader->SetShaderFloat("u_TilingFactor", quadTilingFactor);
		quadTexture->BindTexture();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), quadPosition) * glm::scale(glm::mat4(1.0f), { quadSize.x, quadSize.y, 1.0f });
		s_Data.m_TextureShader->SetShaderMat4("u_Transform", transform);

		s_Data.m_QuadVertexArray->BindVertexArray();
		RenderCommand::DrawIndexed(s_Data.m_QuadVertexArray);
#endif
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& quadPosition, const glm::vec2& quadSize, float quadRotation, const glm::vec4& quadColor)
	{
		DrawRotatedQuad({ quadPosition.x, quadPosition.y, 0.0f }, quadSize, quadRotation, quadColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& quadPosition, const glm::vec2& quadSize, float quadRotation, const glm::vec4& quadColor)
	{
		PRISM_PROFILE_FUNCTION();

		s_Data.m_TextureShader->SetShaderFloat4("u_Color", quadColor);
		s_Data.m_TextureShader->SetShaderFloat("u_TilingFactor", 1.0f);
		s_Data.m_WhiteTexture->BindTexture();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), quadPosition) * glm::rotate(glm::mat4(1.0f), quadRotation, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), { quadSize.x, quadSize.y, 1.0f });
		s_Data.m_TextureShader->SetShaderMat4("u_Transform", transform);

		s_Data.m_QuadVertexArray->BindVertexArray();
		RenderCommand::DrawIndexed(s_Data.m_QuadVertexArray);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& quadPosition, const glm::vec2& quadSize, float quadRotation, const Reference<Texture2D>& quadTexture, float quadTilingFactor, const glm::vec4& quadTintColor)
	{
		DrawRotatedQuad({ quadPosition.x, quadPosition.y, 0.0f }, quadSize, quadRotation, quadTexture, quadTilingFactor, quadTintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& quadPosition, const glm::vec2& quadSize, float quadRotation, const Reference<Texture2D>& quadTexture, float quadTilingFactor, const glm::vec4& quadTintColor)
	{
		PRISM_PROFILE_FUNCTION();

		s_Data.m_TextureShader->SetShaderFloat4("u_Color", quadTintColor);
		s_Data.m_TextureShader->SetShaderFloat("u_TilingFactor", quadTilingFactor);
		quadTexture->BindTexture();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), quadPosition) * glm::rotate(glm::mat4(1.0f), quadRotation, { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), { quadSize.x, quadSize.y, 1.0f });
		s_Data.m_TextureShader->SetShaderMat4("u_Transform", transform);

		s_Data.m_QuadVertexArray->BindVertexArray();
		RenderCommand::DrawIndexed(s_Data.m_QuadVertexArray);
	}
}