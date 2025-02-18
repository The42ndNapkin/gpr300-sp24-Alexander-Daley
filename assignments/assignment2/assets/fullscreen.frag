#version 450

out vec4 frag_color;
in vec2 vs_texcoord;

uniform sampler2D texture0;

void main()
{
	vec3 albedo = texture(texture0, vs_texcoord).rgb;
	frag_color = vec4(albedo,1.0);
}
