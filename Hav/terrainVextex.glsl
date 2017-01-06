# version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

out vec2 st;
uniform mat4 MVP;

const vec4 plane = vec4(0, 1, 0, 0.1);

void main() {

	//gl_ClipDistance[0] = dot(MVP*vec4(Position, 1.0), plane);

	st = TexCoord;
	gl_Position = MVP*vec4(Position, 1.0);
}
