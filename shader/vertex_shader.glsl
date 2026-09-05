#version 150 core
#define PI 3.1415926538

in vec4 position;
in vec3 vertex_normal;
in vec2 tex_coord;
in vec3 face_normal;
in vec3 tangent;

out vec3 fs_vn;
out vec2 fs_text_coord;
out vec3 fs_fn;
out vec3 fs_tangt;
out vec4 fs_pos;

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
	gl_Position = view * proj * camera * model * vec4(position[0], position[1], position[2], 1.0);
	fs_vn = vertex_normal;
	fs_text_coord = vec2(tex_coord.x, tex_coord.y);

	fs_fn = face_normal;
	fs_tangt = tangent;
	fs_pos = position;
}