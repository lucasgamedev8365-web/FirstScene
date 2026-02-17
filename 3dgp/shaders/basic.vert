
// VERTEX SHADER
#version 330

// Light declarations
struct AMBIENT
{	
	vec3 color;
};
uniform AMBIENT lightAmbient;

struct DIRECTIONAL
{	
	vec3 direction;
	vec3 diffuse;
};

struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1, lightPoint2;

struct SPOT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	float cutoff;
	float attenuation;
	mat4 matrix;
};
uniform SPOT spotlight1;

uniform DIRECTIONAL lightDir;

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
 
in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;

out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;
out vec3 texCoordCubeMap; //cube map

vec4 AmbientLight(AMBIENT light)
{
// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);
	return color;
}



void main(void) 
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal
	normal = normalize(mat3(matrixModelView) * aNormal);

	// calculate texture coordinate
	texCoord0 = aTexCoord;

	//calculate cube map reflection vector
	texCoordCubeMap = -inverse(mat3(matrixView)) * reflect(position.xyz, normal);

	// calculate light
	
	color = vec4(0, 0, 0, 1);
	color += AmbientLight(lightAmbient);
	color += DirectionalLight(lightDir);
}

