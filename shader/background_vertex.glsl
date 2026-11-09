#version 150 core

out vec2 fs_ndc;

void main() {
	vec2 pos = vec2((gl_VertexID == 1) ? 3.0 : -1.0,
					(gl_VertexID == 2) ? 3.0 : -1.0);
	fs_ndc = pos;
	gl_Position = vec4(pos, 0.0, 1.0);
}
