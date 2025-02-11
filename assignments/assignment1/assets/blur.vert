#version 450

//vertex attributes
layout(location = 0) in vec2 in_position;  //Take in the position
layout(location = 1) in vec2 in_texcoord; //Take in the texture coordinates

vec4 vertices[3] = {
	vec4(-1,-1,0,0),
	vec4(3,-1,2,0),
	vec4(-1,3,0,2)
};

out vec2 vs_texcoord;

void main()
{
	vs_texcoord = in_texcoord;
	gl_Position = vec4(in_position.xy, 0.0,1.0);
	//gl_Position = vec4(vertices[gl_VertexID].xy,0,1);
}
