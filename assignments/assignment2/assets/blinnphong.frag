#version 450

out vec4 FragColor;

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light
{
	vec3 color;
	vec3 position;
};

in vec3 normal;
in vec3 lightPosition;
in vec3 worldPosition;
in vec2 texcoords;

//uniforms
uniform Material material;
uniform vec3 cameraPosition;
uniform Light light;

float shadowCalculation(vec4 fragPositionLightspace)
{
	//perspective divide
	//vec3 projCoords = fragPositionLightspace.xyz / 
	projCoords = (projCoords * 0.5) + 0.5;

	//float lightDepth = texture(shadowMap, projCoords.xy).r;
	float cameraDepth = projCoords.z;

	float shadow = (cameraDepth > lightDepth) ? 1.0 : 0.0;

	return shadow;
}

vec3 blinnphong(vec3 normal, vec3 fragPos)
{
	//normalize inputs
	vec3 viewDir = normalize(cameraPosition - fragPos);
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	//dot products
	float ndotl = max(dot(normal,lightDir),0.0);
	float ndoth = max(dot(normal,halfwayDir),0.0);

	//light components
	vec3 diffuse = ndotl * material.diffuse;
	vec3 specular = pow(ndoth,material.shininess * 128.0) * material.specular;

	return(diffuse + specular);
}

void main()
{
	vec3 normal = normalize(vsNormal);
	float shadow = shadowCalculation(lightPosition);

	vec3 lighting = blinnphong(normal,vsPosition);
	lighting *= (1.0-shadow);
	lighting *= light.color;
	lighting *= vec3(1.0) * material.ambient;

	vec3 objectColor = normal * 0.5 + 0.5;

	FragColor = vec4(objectColor * lighting,1.0);
}