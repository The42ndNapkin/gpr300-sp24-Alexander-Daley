#version 450

//vertex attributes
layout(location = 0) in vec3 in_position;  //Take in the position
layout(location = 1) in vec2 in_texcoord; //Take in the texture coordinates

out vec2 texcoords;  //output texcoords

uniform mat4 transform_model;
uniform mat4 camera_viewproj;

void main()
{
	texcoords = in_texcoord;
	gl_Position = camera_viewproj * transform_model * vec4(in_position, 1.0);
}

