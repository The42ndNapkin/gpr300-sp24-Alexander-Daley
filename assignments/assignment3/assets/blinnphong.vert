#version 450

//vertex attributes
layout(location = 0) in vec3 in_position;  //Take in the position
layout(location = 1) in vec2 in_texcoord; //Take in the texture coordinates

out vec2 texcoords;  //output texcoords

void main()
{
	texcoords = in_texcoord;
	gl_Position = vec4(in_position.xy, 0.0, 1.0);
}