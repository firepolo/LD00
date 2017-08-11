#version 450 core

layout(location = 0) in vec3 iVertex;
layout(location = 1) in vec2 iCoord;

out vec2 vCoord;

layout(location = 0) uniform mat4 uModel;
layout(location = 1) uniform mat4 uView;
layout(location = 2) uniform mat4 uProjection;

void main()
{
	gl_Position = (uProjection * uView * uModel) * vec4(iVertex, 1);
	vCoord = iCoord;
}