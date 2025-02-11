#version 450

//vertex attributes
layout(location = 0) in vec3 in_position;  //Take in the position
layout(location = 1) in vec3 in_normal;  //Take in the normal
layout(location = 2) in vec2 in_texcoord; //Take in the texture coordinates

out vec3 normal;  //Output normal
out Surface{
	vec3 world_position;
	vec3 world_normal;
	vec2 TexCoord;
} vs_surface;

uniform mat4 transform_model;
uniform mat4 camera_viewproj;

void main()
{
	vs_surface.world_normal = in_normal;
	vs_surface.TexCoord = in_texcoord;
	gl_Position = camera_viewproj * transform_model * vec4(in_position, 1.0);
}
