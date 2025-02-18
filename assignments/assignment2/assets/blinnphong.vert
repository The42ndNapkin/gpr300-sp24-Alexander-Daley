#version 450

//vertex attributes
layout(location = 0) in vec3 in_position;  //Take in the position
layout(location = 1) in vec3 in_normal;  //Take in the normal
layout(location = 2) in vec2 in_texcoord; //Take in the texture coordinates

out vec3 normal;  //Output normal
out vec3 lightPosition;
out vec3 worldPosition;
out vec2 texcoords;

uniform mat4 transform_model;
uniform mat4 camera_viewproj;
uniform mat4 light_viewproj;

void main()
{
	vec4 worldPos = transform_model * vec4(in_position,1.0);

	worldPosition = worldPos.xyz;
	lightPosition = light_viewproj * worldPos.xyz;
	normal = transpose(inverse(mat3(model))) * in_normal;
	texcoords = in_texcoord;
	gl_Position = camera_viewproj * worldPos;
}
