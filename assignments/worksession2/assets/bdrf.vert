#version 450

//vertex attributes
layout(location = 0) in vec3 in_position;  //Take in the position
layout(location = 1) in vec3 in_normal;  //Take in the normal
layout(location = 2) in vec2 in_texcoord; //Take in the texture coordinates

out Surface{
	vec3 world_position;
	vec3 world_normal;
	vec2 TexCoord;
} surface;

uniform mat4 transform_model;
uniform mat4 camera_viewproj;

void main()
{
	surface.world_normal = in_normal;
	surface.TexCoord = in_texcoord;
	surface.world_position = vec3(transform_model * vec4(in_position, 1.0));
	gl_Position = camera_viewproj * transform_model * vec4(in_position, 1.0);
}
