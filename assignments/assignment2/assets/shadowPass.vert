#version 450

//vertex attributes
layout(location = 0) in vec3 in_position;  //Take in the position

uniform mat4 model;
uniform mat4 light_viewproj;

void main()
{
	vec4 world_position = model * vec4(in_position, 1.0);
	gl_Position = light_viewproj * world_position;
}
