# version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

out vec2 textureCoords;
out vec3 LightVector;
out vec3 toCameraVector;
out float moveFactor;
out vec2 vTexCoord2D;
out mat4 o2v_projection_reflection;
out vec3 interpolatedVertexObject;


uniform mat4 MVP, M;
uniform vec3 cameraPosition;
uniform float waveTime;
uniform vec3 lightPosition;

const float waveWidth = 0.6;
const float waveHeight = 0.2;
const float tiling = 2.0;

const float speed = 0.1;

const vec4 plane = vec4(0, 1, 0, 0.1);


void main() {

	//gl_ClipDistance[0] = dot(MVP*vec4(Position, 1.0), plane);

	//passing the vertex coords and the MVP-matrix to the fragment shader
	interpolatedVertexObject = Position;
	o2v_projection_reflection = MVP;

	//texture coords for the reflection texture
	vTexCoord2D = TexCoord * 8.0;

	//texture coords for the sea plane
	textureCoords = vec2(Position.x / 2.0 + 0.5, Position.y / 2.0 + 0.5)*tiling;

	//Preparing Fresnel Effect and NormalMap
	vec4 worldPosition = M*vec4(Position.x, 0.0, Position.y, 1.0);

	//this is my view vector
	toCameraVector = cameraPosition - worldPosition.xyz;// +vec3(0, -9, 0);

	LightVector = worldPosition.xyz - lightPosition;

	moveFactor = waveTime;

	gl_Position = MVP* vec4(Position.xy, 0.0, 1.0);


}

