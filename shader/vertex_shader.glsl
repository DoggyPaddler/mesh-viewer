#version 150 core
#define PI 3.1415926538

in vec4 position;
in vec3 vertex_normal;
in vec2 tex_coord;
in vec3 face_normal;
in vec3 tangent;
in float vertex_ao;

out vec3 fs_vn;
out vec2 fs_text_coord;
out vec3 fs_fn;
out vec3 fs_tangt;
out vec4 fs_pos;
out float fs_ao;

// out vec3 f_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 camera;
uniform mat4 proj;
uniform vec3 diffuse;
uniform vec3 specular;
uniform vec3 ambient;
uniform vec3 light_d;
uniform float p;


void main() {
	vec4 world_position = model * vec4(position[0], position[1], position[2], 1.0);
	gl_Position = view * proj * camera * world_position;
	fs_vn = mat3(model) * vertex_normal;
	fs_text_coord = vec2(tex_coord.x, tex_coord.y);

	fs_fn = face_normal;
	fs_tangt = mat3(model) * tangent;
	fs_pos = world_position;
	fs_ao = vertex_ao;
}