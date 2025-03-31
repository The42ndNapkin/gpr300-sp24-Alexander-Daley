#version 450

//vertex attributes
layout(location = 0) in vec3 in_position;  //Take in the position
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord; //Take in the texture coordinates'

out vec2 texcoords;  //output texcoords
out vec3 normal;
out vec3 position;

uniform mat4 transform_model;
uniform mat4 camera_viewproj;

void main()
{
    vec4 wold_posiion = transform_model * vec4(in_position, 1.0);
	texcoords = in_texcoord;
	normal = in_normal;
	position = wold_posiion.xyz;
	gl_Position = camera_viewproj * wold_posiion;
}
