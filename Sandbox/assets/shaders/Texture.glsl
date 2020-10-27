#type VertexShader
#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
}

#type FragmentShader
#version 330 core

uniform sampler2D u_Texture;

in vec2 v_TexCoord;
out vec4 outputColor;

void main()
{
	outputColor = texture(u_Texture, v_TexCoord * 10.0f) * vec4(1.0f, 0.8f, 1.0f, 1.0f);
}