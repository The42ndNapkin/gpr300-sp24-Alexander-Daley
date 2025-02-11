#version 450

in vec3 out_normal;
in vec3 toCamera;
in vec2 vs_texcoord;
out vec4 fragColor;

uniform vec3 camPos;
uniform vec3 watercolor;
uniform float tiling;
uniform float time;

uniform float warpScale;
uniform float warpStrength;
uniform sampler2D waterTex;
uniform sampler2D waterWarp;
uniform float specScale;
uniform float brightnessLowerCut;
uniform float brightnessUpperCut;

const vec3 reflectColor = vec3(1.0,0.0,0.0);

void main()
{
	vec2 uv = vs_texcoord;

	//Warp
	vec2 warpUV = vs_texcoord * warpScale;
	vec2 warpScroll = vec2(0.5,0.5)*time;
	vec2 warp = texture(waterWarp,warpUV + warpScroll).xy * warpStrength;
	warp = warp*2.0 - 1.0;
	//albedo
	vec2 albedoUV = vs_texcoord * tiling;
	vec2 albedoScroll = vec2(-0.5,0.5)*time;
	vec4 albedo = texture(waterTex, albedoUV + albedoScroll);
	//specular
	vec2 specUV = vs_texcoord * specScale;
	vec3 smp1 = texture(waterTex, specUV + vec2(1.0,0.0)).rgb;
	vec3 smp2 = texture(waterTex, specUV + vec2(1.0,1.0)).rgb;
	vec3 spec = smp1 + smp2;

	float brightness = dot(spec,vec3(0.299,0.587,0.144));
	if(brightness <= brightnessLowerCut || brightness >= brightnessUpperCut)
	{
		//not within range
	}
	else{
		discard;
	}

	fragColor = vec4(watercolor + vec3(albedo.a) + spec, 1.0);
}