# version 330 core

out vec4 finalcolor;
in vec2 st;

uniform sampler2D Texture;


void main() {

	

	vec4 texColour = texture(Texture, st);
	finalcolor = texColour;

}