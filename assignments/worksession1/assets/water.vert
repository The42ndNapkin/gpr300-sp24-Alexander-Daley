#version 450

layout (location = 0) in vec3 v_inPos;
layout (location = 1) in vec3 v_toNormal;
layout (location = 2) in vec2 in_texcoord;

out vec2 vs_texcoord;
out vec3 toCamera;

uniform vec3 cameraPosition;
uniform mat4 view_proj;
uniform mat4 model;
uniform float time;

float calculateSurface(float x, float z)
{
	float y = 0.0f;
	y += sin(x+time);
	y -= sin(z+time);
	return y;
}

void main()
{
	vec3 pos = v_inPos;
	pos += calculateSurface(pos.x,pos.y);

	vec4 worldPos = model * vec4(v_inPos, 1.0);
	toCamera = cameraPosition - worldPos.xyz;
	vs_texcoord = in_texcoord;
	gl_Position = view_proj * model * vec4(v_inPos, 1.0);
}
