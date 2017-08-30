#version 450 core

layout(location = 0) in vec3 iVertex;
layout(location = 1) in vec2 iCoord;

out vec2 vCoord;


void main()
{
	gl_Position = vec4(iVertex, 1);
	vCoord = iCoord;
}