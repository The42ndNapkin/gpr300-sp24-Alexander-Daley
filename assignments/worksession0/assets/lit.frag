#version 450

out vec4 frag_color;
in Surface{
	vec3 world_position;
	vec3 world_normal;
	vec2 TexCoord;
} vs_surface;

struct Material{
	float diffuseK; //diffuse light coeficcient (0-1)
	float specularK; //Specular light coeficcient (0-1)
	float ambientK; //Ambient light coeficcient (0-1)
	float shininess;  //Size of specular highlight
};

uniform Material _Material;
uniform sampler2D _MainTexture;
//Light source
uniform vec3 _eyePos;
uniform vec3 _lightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _lightColor = vec3(1.0,1.0,1.0);
uniform vec3 ambientColor = vec3(0.3,0.4,0.46);

void main()
{
	vec3 normal = normalize(vs_surface.world_normal);
	vec3 toLight = -_lightDirection;
	float diffuseFactor = 0.5 * max(dot(normal, toLight), 0.0);
	vec3 toEye = normalize(_eyePos - vs_surface.world_position);
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.shininess);

	vec3 lightColor = (_Material.diffuseK * diffuseFactor + specularFactor * _Material.specularK) * _lightColor;
	lightColor += ambientColor * _Material.ambientK;
	vec3 objectColor = texture(_MainTexture, vs_surface.TexCoord).rgb;

	frag_color = vec4(objectColor*lightColor, 1.0);
}
