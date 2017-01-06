#version 330

in vec2 textureCoords;
in vec3 LightVector;
in vec3 toCameraVector;
in float moveFactor;
in vec2 vTexCoord2D;
in mat4 o2v_projection_reflection;
in vec3 interpolatedVertexObject;

out vec4 outputColor;

uniform sampler2D normalMap;
uniform sampler2D dudvMap;
uniform sampler2D seaTexture;
uniform sampler2D refractionTexture;
uniform sampler2D renderedTexture;


const float waveStrength = 0.04;
const float shineDamper = 20.0;
const float reflectivity = 0.5;


void main()
{
	
	//creating rippling effect with distortion coords
	vec2 distortedTexCoords = texture(dudvMap, vec2(textureCoords.x + moveFactor*0.1, textureCoords.y)).rg*0.1;
	distortedTexCoords = textureCoords + vec2(distortedTexCoords.x, distortedTexCoords.y + moveFactor*0.1);
	vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength ;

	
	//normal vector generation
	vec4 normalMapColour = texture(normalMap, distortedTexCoords);
	vec3 normal = vec3(normalMapColour.r * 2.0 - 1.0, normalMapColour.b * 2.0, normalMapColour.g * 2.0 - 1.0);
	normal = normalize(normal);

	
	vec3 viewVector = normalize(toCameraVector);
	float refractiveFactor = dot(viewVector, normal);
	refractiveFactor = pow(refractiveFactor, 0.5);

	vec3 lightColour = vec3(1.0, 1.0, 1.0);

	//Specular light calculation
	vec3 reflectedLight = reflect(normalize(LightVector), normal);
	float specular = max(dot(reflectedLight, viewVector), 0.0);		 	
	specular = pow(specular, shineDamper);
	vec3 specularHighlights = lightColour * specular * reflectivity ; 

	// converting reflection coordinates to Norma Device Space to UV-coordinates 
	vec4 vClipReflection = o2v_projection_reflection * vec4(interpolatedVertexObject.xy, 0.0, 1.0);
	vec2 vDeviceReflection = vClipReflection.st / vClipReflection.q;
	vec2 vTextureReflection = vec2(0.5, 0.5) + 0.5 * vDeviceReflection ;
	// adding distortion
	vTextureReflection += totalDistortion ;

	vTextureReflection = clamp(vTextureReflection, 0.001, 0.999);
	vec4 reflectionTextureColor = texture(renderedTexture, vTextureReflection);

	// Framebuffer reflection can have alpha > 1
	reflectionTextureColor.a = 1.0;

	outputColor = mix(reflectionTextureColor, vec4(0.0, 0.3, 0.5, 1), 0.2) + vec4(specularHighlights, 1);

}

