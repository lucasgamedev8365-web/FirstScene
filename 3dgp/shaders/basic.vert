
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

	// calculate light
	
	color = vec4(0, 0, 0, 1);
	color += AmbientLight(lightAmbient);
	color += DirectionalLight(lightDir);
}

