#version 450

out vec4 frag_color;
in vec2 vs_texcoord;

uniform sampler2D hdrBuffer;
uniform float exposure;

void main()
{
	const float gamma = 2.2;
	vec3 hdrColor = texture(hdrBuffer,vs_texcoord).rgb;

	//exposure tone mapping
	vec3 map = vec3(1.0) - exp(-hdrColor * exposure);
	//gamma correction
	map = pow(map,vec3(1.0/gamma));

	frag_color = vec4(map,1.0);
}
