#version 330 core
#extension GL_ARB_explicit_uniform_location: enable

layout(location = 0) in vec3 iVertex;
layout(location = 1) in vec2 iCoord;

out vec2 vCoord;


void main()
{
	gl_Position = vec4(iVertex, 1);
	vCoord = iCoord;
}