#version 450

in vec3 normal;
out vec4 frag_color;
in Surface{
	vec3 world_position;
	vec3 world_normal;
	vec2 TexCoord;
} vs_surface;

uniform sampler2D _MainTexture;
//Light source
uniform vec3 _eyePos;
uniform vec3 _lightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _lightColor = vec3(1.0,1.0,1.0);

void main()
{
	vec3 normal = normalize(vs_surface.world_normal);
	vec3 toLight = -_lightDirection;
	float diffuseFactor = max(dot(normal, toLight), 0.0);
	vec3 toEye = normalize(_eyePos - vs_surface.world_position);
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),128);

	vec3 lightColor = _lightColor * (diffuseFactor + specularFactor);
	vec3 objectColor = texture(_MainTexture, vs_surface.TexCoord).rgb;

	//frag_color = vec4(normal * 0.5 + 0.5 , 1.0);
	//frag_color = texture(_MainTexture, vs_surface.TexCoord);
	frag_color = vec4(objectColor*lightColor, 1.0);
}
