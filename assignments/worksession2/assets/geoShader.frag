#version 450

out vec4 FragColor0;
out vec4 FragColor1;
out vec4 FragColor2;

in vec2 texcoords;  //output texcoords
in vec3 normal;
in vec3 position;

uniform sampler2D mainTex;

void main()
{
	vec3 objectColor = texture(mainTex, texcoords).rgb;
	FragColor0 = vec4(objectColor, 1.0);
	FragColor1 = vec4(position, 1.0);
	FragColor2 = vec4(normal, 1.0);
}
