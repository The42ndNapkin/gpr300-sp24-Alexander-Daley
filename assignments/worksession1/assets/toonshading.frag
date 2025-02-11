#version 450

out vec4 frag_color;
in Surface{
	vec3 world_position;
	vec3 world_normal;
	vec2 TexCoord;
} vs_surface;

uniform sampler2D albedo;
uniform sampler2D zatoon;
//Light source
uniform vec3 _lightDirection = vec3(0.0,-1.0,0.0);

vec3 toon_lighting(vec3 normal, vec3 light_direction)
{
	float diff = (dot(normal, light_direction) + 1.0) * 0.5;
	vec3 light_color = vec3(1.0) * diff;
	float step = texture(zatoon, vec2(diff)).r;
	return light_color * step;
}

void main()
{
	vec3 normal = normalize(vs_surface.world_normal);
	vec3 toLight = -_lightDirection;
	vec3 lightColor = toon_lighting(normal, toLight);
	vec3 objectColor = texture(albedo, vs_surface.TexCoord).rgb;

	frag_color = vec4(objectColor*lightColor, 1.0);
}
