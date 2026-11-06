#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uvw;
//layout (location = 0) in vec3 inPos;

/*
layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
} ubo;
*/

// TODO: Validate
struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
};

// TODO: Validate
layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

// TODO: Validate
layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

layout (location = 0) out vec3 fragTexCoord;

void main() 
{
	fragTexCoord = position;
	// Convert cubemap coordinates into Vulkan coordinate space
	fragTexCoord.xy *= -1.0;
	// Remove translation from view matrix
	mat4 viewMat = mat4(mat3(ubo.view));
	//gl_Position = ubo.projection * viewMat * vec4(position.xyz, 1.0);

	gl_Position = ubo.projection * viewMat * vec4(position.xyz, 1.0);
}
