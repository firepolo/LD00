#version 450 core

layout(location = 0) out vec4 oColor;

in vec2 vCoord;

layout(location = 3) uniform sampler2D uSampler;

void main()
{
	oColor = texture(uSampler, vCoord);
}