#version 450

struct PointLight{
	vec3 position;
    float radius;
	vec4 color;
};
#define MAX_POINT_LIGHTS 64
uniform PointLight _PointLights[MAX_POINT_LIGHTS];

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

//Linear falloff
float attenuateLinear(float distance, float radius){
	return clamp((radius-distance)/radius,0.0,1.0);
}
//Exponential falloff
float attenuateExponential(float distance, float radius){
	float i = clamp(1.0 - pow(distance/radius,4.0),0.0,1.0);
	return i * i;
	
}

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

vec3 calcPointLight(PointLight light,vec3 normal,vec3 pos)
{	
	vec3 diff = light.position - pos;
	//Direction toward light position
	vec3 toLight = normalize(diff);
	vec3 lightColor = blinnphong(pos, normal)* vec3(light.color);
	float d = length(diff); //Distance to light
	//Attenuation
	lightColor *= attenuateLinear(d,light.radius);
	return lightColor;
	//return vec3(0.0,1.0,0.0);
}

void main()
{
	vec3 position = texture(gPosition, texcoords).rgb;
	vec3 normal = texture(gNormal, texcoords).rgb;
	vec3 albedo = texture(gAlbedo, texcoords).rgb;
	vec3 totalLight = vec3(0);

	PointLight mainLight;
	mainLight.position = lightPos;
	mainLight.radius = 1.0f;
	mainLight.color = vec4(lightColor,1.0f);

	totalLight += blinnphong(position, normal);
	for(int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		totalLight += calcPointLight(_PointLights[i], normal, position);
	}
	
	FragColor = vec4(albedo*totalLight, 1.0);
	//FragColor = vec4(vec3(1.0), 1.0);
}