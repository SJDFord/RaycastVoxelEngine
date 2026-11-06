#version 450


layout(binding = 1) uniform sampler2D texSampler;
layout (binding = 2) uniform samplerCube samplerCubeMap;

layout (location = 0) in vec3 fragTexCoord;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = texture(samplerCubeMap, fragTexCoord);

  	//vec2 texCoord2d = vec2(fragTexCoord.x, fragTexCoord.y);
  	//outColor = texture(texSampler, texCoord2d);
}