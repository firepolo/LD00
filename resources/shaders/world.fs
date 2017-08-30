#version 330 core
#extension GL_ARB_explicit_uniform_location: enable

layout(location = 0) out vec4 oColor;

in vec2 vCoord;

layout(location = 5) uniform sampler2D uSampler;

void main()
{
	vec4 color = texture(uSampler, vCoord);
	if (color.rgb == vec3(1, 140.0/255.0, 0)) discard;
	oColor = color;
}