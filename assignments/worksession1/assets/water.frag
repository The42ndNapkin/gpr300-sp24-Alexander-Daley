#version 450

in vec3 out_normal;
in vec3 toCamera;
in vec2 vs_texcoord;
out vec4 fragColor;

uniform vec3 camPos;
uniform vec3 watercolor;
uniform sampler2D texture0;
uniform float tiling;
uniform float time;
uniform float b1;
uniform float b2;

const vec3 reflectColor = vec3(1.0,0.0,0.0);

void main()
{
//	float fresnelFactor = dot(normalize(camPos), vec3(0.0,1.0,0.0));
//
//	vec2 dir = normalize(vec2(1.0));
	vec2 uv = (vs_texcoord * tiling - time);
//
//	vec4 smp1 = texture(texture0,uv);
//	vec4 smp2 = texture(texture0,uv+vec2(0.2));
//
	vec3 albedo = texture(texture0, vs_texcoord).rgb;
//	//vec3 color = mix(albedo, watercolor, fresnelFactor);
//	vec3 color = watercolor + vec3(smp1.a*b1 - smp2.a*b2);
	fragColor = vec4(albedo, 1.0);
}