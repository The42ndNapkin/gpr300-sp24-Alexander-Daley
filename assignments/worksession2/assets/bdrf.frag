#version 450 core
struct PointLight{
	vec3 position;
    float radius;
	vec4 color;
};

in Surface{
	vec3 world_position;
	vec3 world_normal;
	vec2 TexCoord;
} surface;

out vec4 frag_color;

const float PI = 3.14159265359;

//uniforms
uniform vec3 cameraPos;
uniform float roughness;
uniform float metallic;
uniform PointLight light;

vec3 viewDir;
vec3 lightDir;
vec3 toEye;
float VdotN = 0.0;
float LdotN = 0.0;
float NdotH = 0.0;

float D(float mRoughness)
{
	//alpha = roughness squared
	float alpha2 = pow(mRoughness, 4.0);
	float denom = PI * pow(pow(NdotH, 2.0) * (alpha2 - 1.0) + 1.0,2.0);
	return alpha2 / denom;
}

vec3 F()
{
	return vec3(1.0);
}

vec3 G()
{
	return vec3(1.0);
}

//describe specular lighting
vec3 cookTorrence(vec3 fresnel, float mRoughness)
{
	vec3 numerator = D(mRoughness) * fresnel * G();
	float denominator = 4.0 * VdotN * LdotN;
	return numerator / denominator;
}

vec3 BDRF(float cosTheta)
{
	vec3 lambert = vec3(1.0,0.0,0.0) / PI;
	vec3 fresnel = lambert + (1.0 - lambert) * pow(1.0 - cosTheta, 5);
	vec3 ks = fresnel;
	vec3 kd = (1.0- ks) * (1.0-metallic);

	vec3 diffuse = kd*lambert;
	vec3 specular = ks * cookTorrence(fresnel,roughness);
	return (diffuse + specular);
}

vec3 outgoingLight(vec3 outdir, vec3 fragpos)
{
	vec3 emitted = vec3(0);
	vec3 radiance = vec3(0);

	vec3 incoming = vec3(light.color.r,light.color.g,light.color.b);
	vec3 inDir = normalize(viewDir - light.position);
	vec3 bdrf = BDRF(dot(toEye, lightDir));

	radiance += bdrf * incoming * dot(surface.world_normal, light.position);
	
	return radiance + emitted;
}

void main()
{
//calculate dot products
	VdotN;
	LdotN;
	NdotH;

	lightDir = (normalize(light.position - surface.world_position));
	toEye = normalize(cameraPos - surface.world_position);
	vec3 h = normalize(lightDir + toEye);

	viewDir = normalize(cameraPos - surface.world_position);
	frag_color = vec4(outgoingLight(toEye,surface.world_position), 1.0);
}