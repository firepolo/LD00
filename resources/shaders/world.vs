#version 450 core

layout(location = 0) in vec3 iVertex;
layout(location = 1) in vec2 iCoord;

out vec2 vCoord;

layout(location = 0) uniform mat4 uModel;
layout(location = 1) uniform mat4 uView;
layout(location = 2) uniform mat4 uProjection;

layout(location = 3) uniform int uAnimationIndex;
layout(location = 4) uniform int uAnimationFrame;


void main()
{
	gl_Position = (uProjection * uView * uModel) * vec4(iVertex, 1);
	vCoord = vec2(iCoord.x + uAnimationFrame * 0.25, iCoord.y + uAnimationIndex * 0.25);
}