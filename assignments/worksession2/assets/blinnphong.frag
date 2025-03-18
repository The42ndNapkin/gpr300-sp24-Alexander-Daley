#version 450

out vec4 FragColor;

in vec2 texcoords;  //output texcoords

// geometric info.
uniform sampler2D gAlbedo;
uniform sampler2D gPosition;
uniform sampler2D gNormal;

//light info
uniform vec3 cameraPosition;
uniform vec3 lightPos;
uniform vec3 lightColor;

//material info
uniform float mDiffuse;
uniform float mShininess;
uniform float mSpecular;

vec3 blinnphong(vec3 position, vec3 normal)
{
	//normalize inputs
	vec3 viewDir = normalize(cameraPosition - position);
	vec3 lightDir = normalize(lightPos - position);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	//dot products
	float ndotl = max(dot(normal,lightDir),0.0);
	float ndoth = max(dot(normal,halfwayDir),0.0);

	//light components
	vec3 diffuse = vec3(ndotl * mDiffuse);
	vec3 specular =  vec3(pow(ndoth, mShininess) * mSpecular);

	return (diffuse + specular);
}

void main()
{
	vec3 albedo = texture(gAlbedo, texcoords).rgb;
	vec3 position = texture(gPosition, texcoords).rgb;
	vec3 normal = texture(gNormal, texcoords).rgb;

	vec3 lighting = blinnphong(position, normal);

	FragColor = vec4(lighting, 1.0);
}