#version 330 core
#extension GL_ARB_explicit_uniform_location: enable

layout(location = 0) out vec4 oColor;

in vec2 vCoord;

layout(location = 2) uniform sampler2D uColorSampler;
layout(location = 3) uniform sampler2D uDepthSampler;

const float near = 0.1;
const float far = 8.0;

void main()
{
	float depth = (2.2 * near) / (far + near - texture2D(uDepthSampler, vCoord).x * (far - near));
	oColor = texture(uColorSampler, vCoord) * (1.0 - depth * depth);
}