#version 450

out vec4 FragColor;

in vec2 texcoords;  //output texcoords

uniform sampler2D gAlbedo;
uniform sampler2D gPosition;
uniform sampler2D gNormal;

vec3 blinnphong(vec3 position, vec3 normal)
{
	return vec3(1.0, 0.0, 1.0);
}

void main()
{
	vec3 albedo = texture(gAlbedo, texcoords).rgb;
	vec3 position = texture(gPosition, texcoords).rgb;
	vec3 normal = texture(gNormal, texcoords).rgb;

	vec3 lighting = blinnphong(position, normal);

	FragColor = vec4(albedo * lighting, 1.0);
}