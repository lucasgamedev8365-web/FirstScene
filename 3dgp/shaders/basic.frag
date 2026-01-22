// FRAGMENT SHADER

#version 330

in vec4 color;
in vec4 position;
in vec3 normal;

out vec4 outColor;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// View Matrix
uniform mat4 matrixView;

//textures
uniform sampler2D texture0;
in vec2 texCoord0;

struct POINT
{
		vec3 position;
		vec3 diffuse;
		vec3 specular;
};
uniform POINT lightPoint1, lightPoint2;

vec4 PointLight(POINT light)
{
	// Calculate Point Light
	vec4 color = vec4(0, 0, 0, 0);
	//In Directional Light: vec3 L = normalize(mat3(matrixView) * light.direction);
	vec3 L = (normalize(matrixView * vec4(light.position, 1) - position)).xyz;
	//differences with directional light end here
	float NdotL = dot(normal, L);
	color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0); //diffuse light calc
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	color += vec4(materialSpecular * light.specular * pow(max(RdotV, 0), shininess), 1); // specular light calc
	return color;
}

void main(void) 
{
	outColor = color;
	outColor += PointLight(lightPoint1);
	outColor += PointLight(lightPoint2);
	outColor *= texture(texture0, texCoord0);	
}
